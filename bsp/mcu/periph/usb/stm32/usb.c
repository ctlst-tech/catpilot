#include "usb.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

static usb_t *usb = NULL;

int usb_init(usb_t *cfg) {
    if (usb != NULL) {
        return EBUSY;
    }

    usb = cfg;

    cfg->p.tx_mutex = xSemaphoreCreateMutex();
    if (cfg->p.tx_mutex == NULL) {
        return -1;
    }
    cfg->p.rx_mutex = xSemaphoreCreateMutex();
    if (cfg->p.rx_mutex == NULL) {
        return -1;
    }
    cfg->p.tx_sem = xSemaphoreCreateBinary();
    if (cfg->p.tx_sem == NULL) {
        return -1;
    }
    cfg->p.rx_sem = xSemaphoreCreateBinary();
    if (cfg->p.rx_sem == NULL) {
        return -1;
    }

    struct file_operations fops = {.open = usb_open,
                                   .write = usb_write,
                                   .read = usb_read,
                                   .close = usb_close,
                                   .ioctl = usb_ioctl,
                                   .dev = cfg};

    char path[32];
    sprintf(path, "/dev/%s", cfg->name);
    if (node_mount(path, &fops) == NULL) {
        return -1;
    }

    cfg->fops = fops;

    cfg->p.read_buf = ring_buf_init(cfg->buf_size);
    if (cfg->p.read_buf == NULL) {
        return -1;
    }
    cfg->p.write_buf = ring_buf_init(cfg->buf_size);
    if (cfg->p.write_buf == NULL) {
        return -1;
    }
    cfg->p.rx_buf = calloc(cfg->buf_size, sizeof(uint8_t));
    if (cfg->p.rx_buf == NULL) {
        return -1;
    }
    cfg->p.tx_buf = calloc(cfg->buf_size, sizeof(uint8_t));
    if (cfg->p.tx_buf == NULL) {
        return -1;
    }

    cfg->p.rx_count = 0;
    cfg->p.tx_count = 0;
    cfg->p.host_port_state = false;

    MX_USB_DEVICE_Init();

    cfg->p.periph_init = true;
    return 0;
}

int usb_transmit_compl(uint8_t *buf, uint32_t *len, uint8_t epnum) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(usb->p.tx_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    usb->p.tx_count = *len;
    return 0;
}

int usb_receive_compl(uint8_t *buf, uint32_t *len) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(usb->p.rx_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    usb->p.rx_count = *len;
    return 0;
}

int usb_transmit(usb_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;
    if (xSemaphoreTake(cfg->p.tx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }
    xSemaphoreTake(cfg->p.tx_sem, 0);
    rv = CDC_Transmit_FS(pdata, length);
    if (!rv) {
        xSemaphoreTake(cfg->p.tx_sem, portMAX_DELAY);
    }
    xSemaphoreGive(cfg->p.tx_mutex);
    return rv;
}

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
int usb_receive(usb_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;
    if (xSemaphoreTake(cfg->p.rx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }
    xSemaphoreTake(cfg->p.rx_sem, 0);
    xSemaphoreTake(cfg->p.rx_sem, portMAX_DELAY);
    memcpy(pdata, UserRxBufferFS, cfg->p.rx_count);
    xSemaphoreGive(cfg->p.rx_mutex);
    return 0;
}

void usb_read_task(void *cfg_ptr) {
    usb_t *cfg = (usb_t *)cfg_ptr;
    uint8_t *buf = cfg->p.rx_buf;
    vTaskDelay(2000);
    while (1) {
        if (usb_receive(cfg, buf, cfg->buf_size)) {
            cfg->p.error = ERROR;
        } else {
            cfg->p.error = SUCCESS;
        }
        ring_buf_write(cfg->p.read_buf, buf,
                       MIN(cfg->p.rx_count, cfg->buf_size));
    }
}

void usb_write_task(void *cfg_ptr) {
    usb_t *cfg = (usb_t *)cfg_ptr;
    uint8_t *buf = cfg->p.tx_buf;
    uint16_t length;
    vTaskDelay(2000);
    while (1) {
        if (CDC_Busy_FS() || !cfg->p.host_port_state) {
            vTaskDelay(10);
            continue;
        }
        length = ring_buf_read(cfg->p.write_buf, buf, cfg->buf_size);
        if (usb_transmit(cfg, buf, length)) {
            cfg->p.error = ERROR;
        } else {
            cfg->p.error = SUCCESS;
        }
    }
}

int usb_open(FILE *file, const char *path) {
    usb_t *cfg = (usb_t *)file->node->f_op.dev;

    errno = 0;

    if (cfg->p.port_open && !cfg->p.stdio) {
        errno = EACCES;
        return -1;
    }

    if (cfg->p.tasks_init) {
        return 0;
    }

    if (cfg->buf_size <= 0) {
        errno = EINVAL;
        return -1;
    }

    if (cfg->p.read_buf == NULL || cfg->p.write_buf == NULL) {
        errno = ENOMEM;
        return -1;
    }

    cfg->p.tasks_init = true;
    cfg->p.port_open = true;

    char name[32];
    snprintf(name, MAX_NAME_LEN, "%s_read_thread", cfg->name);
    xTaskCreate(usb_read_task, name, 512, cfg, cfg->task_priority, NULL);
    snprintf(name, MAX_NAME_LEN, "%s_write_thread", cfg->name);
    xTaskCreate(usb_write_task, name, 512, cfg, cfg->task_priority, NULL);

    return 0;
}

ssize_t usb_write(FILE *file, const char *buf, size_t count) {
    ssize_t rv;
    errno = 0;
    usb_t *cfg = (usb_t *)file->node->f_op.dev;

    rv = ring_buf_write(cfg->p.write_buf, (uint8_t *)buf, count);
    if (cfg->p.error) {
        errno = EPROTO;
        return -1;
    }
    return rv;
}

ssize_t usb_read(FILE *file, char *buf, size_t count) {
    ssize_t rv;
    errno = 0;
    usb_t *cfg = (usb_t *)file->node->f_op.dev;

    rv = ring_buf_read_timeout(cfg->p.read_buf, (uint8_t *)buf, count,
                               cfg->read_timeout);
    if (rv < 0) {
        errno = ETIMEDOUT;
        return -1;
    }
    if (cfg->p.error) {
        errno = EPROTO;
        return -1;
    }
    return rv;
}

int usb_close(FILE *file) {
    usb_t *cfg = file->node->f_op.dev;
    errno = 0;
    if (cfg->p.port_open) {
        cfg->p.port_open = false;
    } else {
        errno = ENOENT;
    }
    return 0;
}

int usb_ioctl(FILE *file, int request, va_list args) {
    return 0;
}

void usb_error_handler() {
    return;
}

void usb_set_host_com_port() {
    usb->p.host_port_state = true;
}

void usb_reset_host_com_port() {
    usb->p.host_port_state = false;
}

void OTG_FS_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void OTG_FS_EP1_OUT_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void OTG_FS_EP1_IN_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
