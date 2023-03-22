#include "usart.h"

static int usart_id_init(usart_t *cfg);
static int usart_clock_init(usart_t *cfg);
void usart_dma_tx_handler(void *area);
void usart_dma_rx_handler(void *area);
void usart_handler(void *area);

int usart_init(usart_t *cfg) {
    int rv = 0;

    if (cfg->p.periph_init) {
        return 0;
    }

    if ((rv = usart_id_init(cfg))) {
        return rv;
    }
    if ((rv = usart_clock_init(cfg))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->gpio_rx))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->gpio_tx))) {
        return rv;
    }
    if (cfg->dma_rx.init.Instance != NULL &&
        cfg->dma_tx.init.Instance != NULL) {
        if ((rv = dma_init(&cfg->dma_tx, usart_dma_tx_handler, cfg))) {
            return rv;
        }
        if ((rv = dma_init(&cfg->dma_rx, usart_dma_rx_handler, cfg))) {
            return rv;
        }
        cfg->dma_tx.init.Parent = &cfg->init;
        cfg->dma_rx.init.Parent = &cfg->init;
        cfg->init.hdmatx = &cfg->dma_tx.init;
        cfg->init.hdmarx = &cfg->dma_rx.init;
        cfg->p.use_dma = true;
    } else {
        cfg->p.use_dma = false;
    }
    if ((rv = irq_init(cfg->p.id, cfg->irq_priority, usart_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.id))) {
        return rv;
    }
    if ((rv = HAL_UART_Init(&cfg->init)) != HAL_OK) {
        return rv;
    }

    struct file_operations f_op = {.open = usart_open,
                                   .write = usart_write,
                                   .read = usart_read,
                                   .close = usart_close,
                                   .dev = cfg};

    char path[32];
    sprintf(path, "/dev/%s", cfg->name);
    if (node_mount(path, &f_op) == NULL) {
        return -1;
    }

    cfg->p.rx_sem = xSemaphoreCreateBinary();
    if (cfg->p.rx_sem == NULL) {
        return -1;
    }
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

    cfg->p.read_buf = ring_buf_init(cfg->buf_size);
    if (cfg->p.read_buf == NULL) {
        return -1;
    }
    cfg->p.write_buf = ring_buf_init(cfg->buf_size);
    if (cfg->p.write_buf == NULL) {
        return -1;
    }
    cfg->p.dma_rx_buf = calloc(cfg->buf_size, sizeof(uint8_t));
    if (cfg->p.dma_rx_buf == NULL) {
        return -1;
    }
    cfg->p.dma_tx_buf = calloc(cfg->buf_size, sizeof(uint8_t));
    if (cfg->p.dma_tx_buf == NULL) {
        return -1;
    }

    cfg->p.periph_init = true;
    return rv;
}

int usart_transmit(usart_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;
    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->p.tx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->p.tx_sem, 0);

    cfg->p.tx_state = USART_TRANSMIT;
    cfg->p.tx_count = 0;

    if (cfg->p.use_dma) {
        rv = HAL_UART_Transmit_DMA(&cfg->init, pdata, length);
    } else {
        rv = HAL_UART_Transmit_IT(&cfg->init, pdata, length);
    }

    if (rv == HAL_OK &&
        !xSemaphoreTake(cfg->p.tx_sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    }

    cfg->p.tx_state = USART_FREE;
    xSemaphoreGive(cfg->p.tx_mutex);

    return rv;
}

int usart_receive(usart_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->p.rx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->p.rx_sem, 0);

    cfg->p.rx_state = USART_RECEIVE;
    cfg->p.rx_count = 0;

    if (cfg->mode == USART_IDLE) {
        SET_BIT(cfg->init.Instance->ICR, USART_ICR_IDLECF);
        SET_BIT(cfg->init.Instance->CR1, USART_CR1_IDLEIE);
    }
    if (cfg->p.use_dma) {
        rv = HAL_UART_Receive_DMA(&cfg->init, pdata, length);
    } else {
        rv = HAL_UART_Receive_IT(&cfg->init, pdata, length);
    }

    if (rv == HAL_OK &&
        !xSemaphoreTake(cfg->p.rx_sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    }

    cfg->p.rx_state = USART_FREE;
    xSemaphoreGive(cfg->p.rx_mutex);

    return rv;
}

int usart_transmit_receive(usart_t *cfg, uint8_t *tx_pdata, uint8_t *rx_pdata,
                           uint16_t tx_length, uint16_t rx_length) {
    int rv = 0;

    if (tx_length == 0 || rx_length == 0 || tx_pdata == NULL ||
        rx_pdata == NULL) {
        return EINVAL;
    }

    if ((xSemaphoreTake(cfg->p.tx_mutex, 0) == pdFALSE) ||
        (xSemaphoreTake(cfg->p.rx_mutex, 0) == pdFALSE)) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->p.rx_sem, 0);

    cfg->p.tx_state = USART_TRANSMIT;
    cfg->p.rx_state = USART_RECEIVE;
    cfg->p.tx_count = 0;
    cfg->p.rx_count = 0;

    SET_BIT(cfg->init.Instance->ICR, USART_ICR_IDLECF);
    SET_BIT(cfg->init.Instance->CR1, USART_CR1_IDLEIE);

    if (cfg->p.use_dma) {
        rv = HAL_UART_Receive_DMA(&cfg->init, rx_pdata, rx_length);
        rv |= HAL_UART_Transmit_DMA(&cfg->init, tx_pdata, tx_length);
    } else {
        rv = HAL_UART_Receive_IT(&cfg->init, rx_pdata, rx_length);
        rv |= HAL_UART_Transmit_IT(&cfg->init, tx_pdata, tx_length);
    }

    if (rv == HAL_OK &&
        !xSemaphoreTake(cfg->p.rx_sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    }

    cfg->p.tx_state = USART_FREE;
    cfg->p.rx_state = USART_FREE;
    xSemaphoreGive(cfg->p.tx_mutex);
    xSemaphoreGive(cfg->p.rx_mutex);

    return rv;
}

int usart_set_speed(void *dev, uint32_t speed) {
    usart_t *cfg = (usart_t *)dev;
    cfg->init.Init.BaudRate = speed;
    cfg->init.Instance->BRR = (uint16_t)(UART_DIV_SAMPLING16(
        HAL_RCC_GetPCLK1Freq(), cfg->init.Init.BaudRate,
        cfg->init.Init.ClockPrescaler));
    return 0;
}

uint32_t usart_get_speed(void *dev) {
    usart_t *cfg = (usart_t *)dev;
    return (cfg->init.Init.BaudRate);
}

void usart_read_task(void *cfg_ptr) {
    usart_t *cfg = (usart_t *)cfg_ptr;
    uint8_t *buf = cfg->p.dma_rx_buf;
    while (1) {
        if (usart_receive(cfg, buf, cfg->buf_size)) {
            cfg->p.error = ERROR;
        } else {
            cfg->p.error = SUCCESS;
        }
        ring_buf_write(cfg->p.read_buf, buf,
                       MIN(cfg->p.rx_count, cfg->buf_size));
    }
}

void usart_write_task(void *cfg_ptr) {
    usart_t *cfg = (usart_t *)cfg_ptr;
    uint8_t *buf = cfg->p.dma_tx_buf;
    uint16_t length;
    while (1) {
        length = ring_buf_read(cfg->p.write_buf, buf, cfg->buf_size);
        if (usart_transmit(cfg, buf, length)) {
            cfg->p.error = ERROR;
        } else {
            cfg->p.error = SUCCESS;
        }
    }
}

int usart_open(FILE *file, const char *path) {
    usart_t *cfg = (usart_t *)file->node->f_op.dev;

    errno = 0;

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

    char name[32];
    snprintf(name, MAX_NAME_LEN, "%s_read_thread", cfg->name);
    xTaskCreate(usart_read_task, name, 128, cfg, cfg->task_priority, NULL);
    snprintf(name, MAX_NAME_LEN, "%s_wirte_thread", cfg->name);
    xTaskCreate(usart_write_task, name, 128, cfg, cfg->task_priority, NULL);

    cfg->p.tasks_init = true;
    errno = 0;

    return 0;
}

ssize_t usart_write(FILE *file, const char *buf, size_t count) {
    ssize_t rv;
    errno = 0;
    usart_t *cfg = (usart_t *)file->node->f_op.dev;

    rv = ring_buf_write(cfg->p.write_buf, (uint8_t *)buf, count);
    if (cfg->p.error) {
        errno = EPROTO;
        return -1;
    }
    return rv;
}

ssize_t usart_read(FILE *file, char *buf, size_t count) {
    ssize_t rv;
    errno = 0;
    usart_t *cfg = (usart_t *)file->node->f_op.dev;

    rv = ring_buf_read(cfg->p.read_buf, (uint8_t *)buf, count);
    if (cfg->p.error) {
        errno = EPROTO;
        return -1;
    }
    return rv;
}

int usart_close(FILE *file) {
    (void)file;
    errno = 0;
    return 0;
}

void usart_handler(void *area) {
    usart_t *cfg = (usart_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    DMA_Stream_TypeDef *dma =
        (DMA_Stream_TypeDef *)((cfg->dma_rx.init.Instance));

    HAL_UART_IRQHandler(&cfg->init);

    if (cfg->init.gState == HAL_UART_STATE_READY &&
        cfg->p.tx_state == USART_TRANSMIT) {
        if (cfg->p.use_dma) {
            cfg->p.tx_count = dma->NDTR;
        } else {
            cfg->p.tx_count = cfg->init.TxXferSize;
        }
        xSemaphoreGiveFromISR(cfg->p.tx_sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (cfg->init.RxState == HAL_UART_STATE_READY &&
        cfg->p.rx_state == USART_RECEIVE && cfg->mode == USART_TIMEOUT) {
        if (cfg->p.use_dma) {
            cfg->p.rx_count = dma->NDTR;
        } else {
            cfg->p.rx_count = cfg->init.RxXferSize;
        }
        xSemaphoreGiveFromISR(cfg->p.rx_sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (cfg->init.Instance->ISR & USART_ISR_IDLE &&
        cfg->p.rx_state == USART_RECEIVE && cfg->mode == USART_IDLE) {
        SET_BIT(cfg->init.Instance->ICR, USART_ICR_IDLECF);
        if (cfg->p.use_dma) {
            cfg->p.rx_count = cfg->init.RxXferSize - dma->NDTR;
        } else {
            cfg->p.rx_count = cfg->init.RxXferSize - cfg->init.RxXferCount;
        }
        HAL_UART_AbortReceive(&cfg->init);
        CLEAR_BIT(cfg->init.Instance->ICR, USART_ICR_IDLECF);
        CLEAR_BIT(cfg->init.Instance->CR1, USART_CR1_IDLEIE);
        xSemaphoreGiveFromISR(cfg->p.rx_sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void usart_dma_tx_handler(void *area) {
    usart_t *cfg = (usart_t *)area;
    HAL_DMA_IRQHandler(&cfg->dma_tx.init);
}

void usart_dma_rx_handler(void *area) {
    usart_t *cfg = (usart_t *)area;
    HAL_DMA_IRQHandler(&cfg->dma_rx.init);
}

static int usart_id_init(usart_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case USART1_BASE:
            cfg->p.id = USART1_IRQn;
            break;
        case USART2_BASE:
            cfg->p.id = USART2_IRQn;
            break;
        case USART3_BASE:
            cfg->p.id = USART3_IRQn;
            break;
        case UART4_BASE:
            cfg->p.id = UART4_IRQn;
            break;
        case UART5_BASE:
            cfg->p.id = UART5_IRQn;
            break;
        case USART6_BASE:
            cfg->p.id = USART6_IRQn;
            break;
        case UART7_BASE:
            cfg->p.id = UART7_IRQn;
            break;
        case UART8_BASE:
            cfg->p.id = UART8_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static int usart_clock_init(usart_t *cfg) {
    switch (cfg->p.id) {
        case USART1_IRQn:
            __HAL_RCC_USART1_CLK_ENABLE();
            break;
        case USART2_IRQn:
            __HAL_RCC_USART2_CLK_ENABLE();
            break;
        case USART3_IRQn:
            __HAL_RCC_USART3_CLK_ENABLE();
            break;
        case UART4_IRQn:
            __HAL_RCC_UART4_CLK_ENABLE();
            break;
        case UART5_IRQn:
            __HAL_RCC_UART5_CLK_ENABLE();
            break;
        case USART6_IRQn:
            __HAL_RCC_USART6_CLK_ENABLE();
            break;
        case UART7_IRQn:
            __HAL_RCC_UART7_CLK_ENABLE();
            break;
        case UART8_IRQn:
            __HAL_RCC_UART8_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}
