#include "can.h"

static int can_id_init(can_t *cfg);
static int can_clock_init(can_t *cfg);
static uint32_t can_length_to_hal(uint32_t length);
static uint32_t can_length_from_hal(uint32_t dlc);
static uint32_t can_id_type_to_hal(uint32_t id);
static uint32_t can_frame_type_to_hal(uint32_t frame_type);
static uint32_t can_frame_type_from_hal(uint32_t frame_type);
static int can_start_service(can_t *cfg);
void can_it_handler(void *area);

int can_init(can_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return -1;
    }
    if ((rv = can_id_init(cfg))) {
        return rv;
    }
    if ((rv = can_clock_init(cfg))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->tx))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->rx))) {
        return rv;
    }
    if ((rv = HAL_FDCAN_Init(&cfg->init))) {
        return rv;
    }
    if ((rv =
             irq_init(cfg->p.it0_id, cfg->irq_priority, can_it_handler, cfg))) {
        return rv;
    }
    if ((rv =
             irq_init(cfg->p.it1_id, cfg->irq_priority, can_it_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.it0_id))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.it1_id))) {
        return rv;
    }

    cfg->p.tx_mutex = xSemaphoreCreateMutex();
    if (cfg->p.tx_mutex == NULL) {
        return -1;
    }
    cfg->p.tx_sem = xSemaphoreCreateBinary();
    if (cfg->p.tx_sem == NULL) {
        return -1;
    }
    cfg->p.rx_mutex = xSemaphoreCreateMutex();
    if (cfg->p.rx_mutex == NULL) {
        return -1;
    }
    cfg->p.rx_sem = xSemaphoreCreateBinary();
    if (cfg->p.rx_sem == NULL) {
        return -1;
    }
    cfg->p.channel_mutex = xSemaphoreCreateMutex();
    if (cfg->p.channel_mutex == NULL) {
        return -1;
    }
    cfg->p.tx_queue = xQueueCreate(cfg->queue_size, sizeof(can_frame_t));
    if (cfg->p.tx_queue == NULL) {
        return -1;
    }

    if ((rv = HAL_FDCAN_Start(&cfg->init))) {
        return rv;
    }
    if ((rv = HAL_FDCAN_ActivateNotification(&cfg->init,
                                             FDCAN_IT_TX_COMPLETE |
                                                 FDCAN_IT_RX_FIFO0_NEW_MESSAGE |
                                                 FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
                                             0xFFFFFFFF))) {
        return rv;
    }

    xSemaphoreTake(cfg->p.rx_sem, 0);

    if (can_start_service(cfg)) {
        errno = ENXIO;
        return -1;
    }

    struct file_operations f_op = {.open = can_open,
                                   .write = can_write,
                                   .read = can_read,
                                   .close = can_close,
                                   .ioctl = can_ioctl,
                                   .dev = cfg};

    char path[32];
    sprintf(path, "/dev/%s", cfg->name);
    if (node_mount(path, &f_op) == NULL) {
        return -1;
    }

    cfg->p.periph_init = true;

    return rv;
}

int can_transmit(can_t *cfg, can_header_t *header, uint8_t *pdata) {
    int rv = 0;

    if (cfg == NULL || pdata == NULL || header == NULL || header->size > 8) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.tx_mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->p.tx_sem, 0);

    cfg->p.state = CAN_TRANSMIT;

    FDCAN_TxHeaderTypeDef hal_header;
    hal_header.Identifier = header->id;
    hal_header.IdType = can_id_type_to_hal(header->id);
    hal_header.TxFrameType = can_frame_type_to_hal(header->frame_type);
    hal_header.DataLength = can_length_to_hal(header->size);
    hal_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    hal_header.BitRateSwitch = FDCAN_BRS_ON;
    hal_header.FDFormat = FDCAN_CLASSIC_CAN;
    hal_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    hal_header.MessageMarker = 0x00;

    rv = HAL_FDCAN_AddMessageToTxFifoQ(&cfg->init, &hal_header, pdata);

    if (rv == HAL_OK) {
        if (cfg->verbosity == CAN_VERBOSITY_HIGH) {
            can_print_info(cfg, header, pdata);
        }
    } else {
        if (cfg->verbosity > CAN_VERBOSITY_OFF) {
            printf("Interface = %s\n", cfg->name);
            printf("Tx Error = %d\n\n", cfg->init.ErrorCode);
        }
    }

    if (rv == HAL_OK &&
        !xSemaphoreTake(cfg->p.tx_sem, pdMS_TO_TICKS(cfg->timeout))) {
        if (cfg->verbosity > CAN_VERBOSITY_OFF) {
            printf("Interface = %s\n", cfg->name);
            printf("Timeout = %d\n\n", cfg->timeout);
        }
        rv = ETIMEDOUT;
    }

    cfg->p.state = CAN_FREE;
    xSemaphoreGive(cfg->p.tx_mutex);

    return rv;
}

int can_receive(can_t *cfg, can_header_t *header, uint8_t *pdata) {
    int rv = 0;

    if (cfg == NULL || pdata == NULL || header == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.rx_mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    cfg->p.state = CAN_RECEIVE;

    if (!xSemaphoreTake(cfg->p.rx_sem, pdMS_TO_TICKS(cfg->timeout))) {
        if (cfg->verbosity > CAN_VERBOSITY_OFF) {
            printf("Interface = %s\n", cfg->name);
            printf("Timeout = %d\n\n", cfg->timeout);
        }
        rv = ETIMEDOUT;
    } else {
        FDCAN_RxHeaderTypeDef hal_header;
        rv = HAL_FDCAN_GetRxMessage(&cfg->init, FDCAN_RX_FIFO0, &hal_header,
                                    pdata);

        if (rv == HAL_OK) {
            header->id = hal_header.Identifier;
            header->frame_type =
                can_frame_type_from_hal(hal_header.RxFrameType);
            header->size = can_length_from_hal(hal_header.DataLength);
            if (cfg->verbosity == CAN_VERBOSITY_HIGH) {
                can_print_info(cfg, header, pdata);
            }
        } else {
            if (cfg->verbosity > CAN_VERBOSITY_OFF) {
                printf("Interface = %s\n", cfg->name);
                printf("Rx Error = %d\n\n", cfg->init.ErrorCode);
            }
        }
    }

    cfg->p.state = CAN_FREE;
    xSemaphoreGive(cfg->p.rx_mutex);

    return rv;
}

void can_write_thread(void *dev) {
    can_t *cfg = (can_t *)dev;
    while (1) {
        can_frame_t frame = {0};
        xQueueReceive(cfg->p.tx_queue, &frame, portMAX_DELAY);
        can_transmit(cfg, &frame.header, frame.data);
    }
}

void can_read_thread(void *dev) {
    can_t *cfg = (can_t *)dev;
    while (1) {
        can_frame_t frame = {0};
        if (!can_receive(cfg, &frame.header, frame.data)) {
            for (int i = 0; i < CAN_MAX_CHANNELS && cfg->p.channel[i] != NULL;
                 i++) {
                if (cfg->p.channel[i]->id_filter & frame.header.id) {
                    xQueueSend(cfg->p.channel[i]->rx_queue, &frame,
                               portMAX_DELAY);
                }
            }
        }
    }
}

int can_open(FILE *file, const char *path) {
    int rv = 0;
    can_t *cfg = (can_t *)file->node->f_op.dev;

    errno = 0;

    if (!cfg->p.periph_init) {
        errno = EPERM;
        return -1;
    }

    can_channel_t *channel = calloc(sizeof(can_channel_t), sizeof(uint8_t));
    if (channel == NULL) {
        errno = ENOMEM;
        return -1;
    }

    channel->can = cfg;
    channel->tx_queue = cfg->p.tx_queue;
    channel->rx_queue = xQueueCreate(cfg->queue_size, sizeof(can_frame_t));
    if (cfg->p.tx_queue == NULL) {
        errno = ENOMEM;
        return -1;
    }
    file->private_data = channel;

    static uint32_t id = CAN_DEFAULT_START_ID;
    xSemaphoreTake(cfg->p.channel_mutex, portMAX_DELAY);
    int i = 0;
    while (cfg->p.channel[i] != NULL) {
        if (i > CAN_MAX_CHANNELS) {
            break;
        }
        i++;
    }
    if (i < CAN_MAX_CHANNELS) {
        channel->id = id++;
        channel->id_filter = CAN_DEFAULT_ID_FILTER_MAST;
        cfg->p.channel[i] = channel;
    } else {
        errno = ENOMEM;
        rv = -1;
    }
    xSemaphoreGive(cfg->p.channel_mutex);

    return rv;
}

ssize_t can_write(FILE *file, const char *buf, size_t count) {
    ssize_t rv;
    can_channel_t *dev = (can_channel_t *)file->private_data;

    errno = 0;

    if (count > 8) {
        errno = EMSGSIZE;
        return -1;
    }

    can_frame_t frame = {
        .channel = dev,
        .header = {.frame_type = CAN_DATA_FRAME, .id = dev->id, .size = count},
        .data = {0}};

    memcpy(frame.data, buf, count);
    xQueueSend(dev->tx_queue, &frame, portMAX_DELAY);
    rv = count;

    return rv;
}

ssize_t can_read(FILE *file, char *buf, size_t count) {
    can_channel_t *dev = (can_channel_t *)file->private_data;

    errno = 0;

    can_frame_t frame = {0};

    xQueueReceive(dev->rx_queue, &frame, portMAX_DELAY);

    memcpy(buf, &frame.header, sizeof(frame.header));
    memcpy(buf + sizeof(frame.header), frame.data, frame.header.size);

    return sizeof(frame.header) + (int)frame.header.size;
}

int can_close(FILE *file) {
    errno = ENOSYS;
    return -1;
}

static int can_ioctl_set_rx_filter_id(FILE *file, va_list args) {
    uint32_t id_filter = va_arg(args, uint32_t);
    can_channel_t *channel = (can_channel_t *)file->private_data;
    channel->id_filter = id_filter;
    return 0;
}

static int can_ioctl_set_tx_msg_id(FILE *file, va_list args) {
    uint32_t id = va_arg(args, uint32_t);
    can_channel_t *channel = (can_channel_t *)file->private_data;
    channel->id = id;
    return 0;
}

int can_ioctl(FILE *file, int request, va_list args) {
    int rv = 0;
    can_channel_t *dev = (can_channel_t *)file->private_data;

    errno = 0;

    switch (request) {
        case CAN_IOCTL_SET_TX_MSG_ID:
            can_ioctl_set_tx_msg_id(file, args);
            break;
        case CAN_IOCTL_SET_RX_FILTER_ID:
            can_ioctl_set_rx_filter_id(file, args);
            break;
        default:
            errno = ENOENT;
            rv = -1;
    }

    return rv;
}

void can_print_info(can_t *cfg, can_header_t *header, uint8_t *pdata) {
    printf("State = %d\n", cfg->p.state);
    printf("Interface = %s\n", cfg->name);
    printf("ID = 0x%X\n", header->id);
    printf("Frame type = 0x%X\n", header->frame_type);
    printf("Msg size = 0x%X\n", header->size);
    printf("Data = ", header->size);
    for (uint32_t i = 0; i < header->size; i++) {
        printf("0x%X ", pdata[i]);
    }
    printf("\n\n");
}

void can_it_handler(void *area) {
    can_t *cfg = (can_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint32_t ir = cfg->init.Instance->IR;

    if (ir & FDCAN_IR_TC) {
        cfg->init.Instance->IR = FDCAN_IR_TC & FDCAN_IR_MASK;
        xSemaphoreGiveFromISR(cfg->p.tx_sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else if (ir & (FDCAN_IR_RF0N | FDCAN_IR_RF1N)) {
        cfg->init.Instance->IR =
            (FDCAN_IR_RF0N | FDCAN_IR_RF1N) & FDCAN_IR_MASK;
        xSemaphoreGiveFromISR(cfg->p.rx_sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

static int can_start_service(can_t *cfg) {
    char name[32];
    snprintf(name, MAX_NAME_LEN, "%s_read_thread", cfg->name);
    if (!xTaskCreate(can_read_thread, name, configMINIMAL_STACK_SIZE, cfg,
                     cfg->task_priority, NULL)) {
        return -1;
    }
    snprintf(name, MAX_NAME_LEN, "%s_write_thread", cfg->name);
    if (!xTaskCreate(can_write_thread, name, configMINIMAL_STACK_SIZE, cfg,
                     cfg->task_priority, NULL)) {
        return -1;
    }
    return 0;
}

static int can_id_init(can_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case FDCAN1_BASE:
            cfg->p.it0_id = FDCAN1_IT0_IRQn;
            cfg->p.it1_id = FDCAN1_IT1_IRQn;
            break;
        case FDCAN2_BASE:
            cfg->p.it0_id = FDCAN2_IT0_IRQn;
            cfg->p.it1_id = FDCAN2_IT1_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static uint32_t can_length_to_hal(uint32_t length) {
    uint32_t rv = 0;
    switch (length) {
        case 0:
            rv = FDCAN_DLC_BYTES_0;
            break;
        case 1:
            rv = FDCAN_DLC_BYTES_1;
            break;
        case 2:
            rv = FDCAN_DLC_BYTES_2;
            break;
        case 3:
            rv = FDCAN_DLC_BYTES_3;
            break;
        case 4:
            rv = FDCAN_DLC_BYTES_4;
            break;
        case 5:
            rv = FDCAN_DLC_BYTES_5;
            break;
        case 6:
            rv = FDCAN_DLC_BYTES_6;
            break;
        case 7:
            rv = FDCAN_DLC_BYTES_7;
            break;
        case 8:
            rv = FDCAN_DLC_BYTES_8;
            break;
        default:
            rv = 0;
    }
    return rv;
}

static uint32_t can_length_from_hal(uint32_t dlc) {
    uint32_t rv = 0;
    switch (dlc) {
        case FDCAN_DLC_BYTES_0:
            rv = 0;
            break;
        case FDCAN_DLC_BYTES_1:
            rv = 1;
            break;
        case FDCAN_DLC_BYTES_2:
            rv = 2;
            break;
        case FDCAN_DLC_BYTES_3:
            rv = 3;
            break;
        case FDCAN_DLC_BYTES_4:
            rv = 4;
            break;
        case FDCAN_DLC_BYTES_5:
            rv = 5;
            break;
        case FDCAN_DLC_BYTES_6:
            rv = 6;
            break;
        case FDCAN_DLC_BYTES_7:
            rv = 7;
            break;
        case FDCAN_DLC_BYTES_8:
            rv = 8;
            break;
        default:
            rv = 0;
    }
    return rv;
}

static uint32_t can_id_type_to_hal(uint32_t id) {
    if (id > 0x7FF && id < (0x1 << 29)) {
        return FDCAN_EXTENDED_ID;
    }
    return FDCAN_STANDARD_ID;
}

static uint32_t can_frame_type_to_hal(uint32_t frame_type) {
    if (frame_type == CAN_REMOTE_FRAME) {
        return FDCAN_REMOTE_FRAME;
    }
    return FDCAN_DATA_FRAME;
}

static uint32_t can_frame_type_from_hal(uint32_t frame_type) {
    if (frame_type == FDCAN_REMOTE_FRAME) {
        return CAN_REMOTE_FRAME;
    }
    return CAN_DATA_FRAME;
}

static int can_clock_init(can_t *cfg) {
    __HAL_RCC_FDCAN_CLK_ENABLE();
    return 0;
}
