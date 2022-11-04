#include "usart.h"

int usart_id_init(usart_t *cfg);
int usart_clock_init(usart_t *cfg);
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
    if ((rv = dma_init(&cfg->dma_tx, usart_dma_tx_handler, cfg))) {
        return rv;
    }
    if ((rv = dma_init(&cfg->dma_rx, usart_dma_tx_handler, cfg))) {
        return rv;
    }
    cfg->init.hdmatx = &cfg->dma_tx.init;
    cfg->init.hdmarx = &cfg->dma_rx.init;

    if ((rv = HAL_UART_Init(&cfg->init)) != HAL_OK) {
        return rv;
    }

    if (cfg->p.tx_mutex == NULL) {
        cfg->p.tx_mutex = xSemaphoreCreateMutex();
    }
    if (cfg->p.rx_mutex == NULL) {
        cfg->p.rx_mutex = xSemaphoreCreateMutex();
    }
    if (cfg->p.tx_sem == NULL) {
        cfg->p.tx_sem = xSemaphoreCreateBinary();
    }
    if (cfg->p.rx_sem == NULL) {
        cfg->p.rx_sem = xSemaphoreCreateBinary();
    }

    // irq_enable(cfg->p.id, );

    cfg->p.periph_init = 1;
    return rv;
}

int USART_Transmit(usart_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;
    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->p.tx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->p.tx_sem, 0);

    cfg->p.tx_state = USART_TRANSMIT;

    HAL_UART_Transmit_DMA(&cfg->init, pdata, length);

    if (xSemaphoreTake(cfg->p.tx_sem, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.tx_state = USART_FREE;
    xSemaphoreGive(cfg->p.tx_mutex);

    return rv;
}

int USART_Receive(usart_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->p.rx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->p.rx_sem, 0);

    cfg->p.rx_state = USART_RECEIVE;

    if (cfg->mode == USART_IDLE) {
        SET_BIT(cfg->init.Instance->ICR, USART_ICR_IDLECF);
        SET_BIT(cfg->init.Instance->CR1, USART_CR1_IDLEIE);
    }

    HAL_UART_Receive_DMA(&cfg->init, pdata, length);

    if (xSemaphoreTake(cfg->p.rx_sem, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.rx_state = USART_FREE;
    xSemaphoreGive(cfg->p.rx_mutex);

    return rv;
}

int USART_TransmitReceive(usart_t *cfg, uint8_t *tx_pdata, uint8_t *rx_pdata,
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

    SET_BIT(cfg->init.Instance->ICR, USART_ICR_IDLECF);
    SET_BIT(cfg->init.Instance->CR1, USART_CR1_IDLEIE);
    HAL_UART_Receive_DMA(&cfg->init, rx_pdata, rx_length);
    HAL_UART_Transmit_DMA(&cfg->init, tx_pdata, tx_length);

    if (xSemaphoreTake(cfg->p.rx_sem, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.tx_state = USART_FREE;
    cfg->p.rx_state = USART_FREE;
    xSemaphoreGive(cfg->p.tx_mutex);
    xSemaphoreGive(cfg->p.rx_mutex);

    return rv;
}

int USART_SetSpeed(usart_t *cfg, uint32_t speed) {
    cfg->init.Init.BaudRate = speed;
    cfg->init.Instance->BRR = (uint16_t)(UART_DIV_SAMPLING16(
        HAL_RCC_GetPCLK1Freq(), cfg->init.Init.BaudRate,
        cfg->init.Init.ClockPrescaler));
    return 0;
}

uint32_t USART_GetSpeed(usart_t *cfg) { return (cfg->init.Init.BaudRate); }

void usart_handler(void *area) {
    usart_t *cfg = (usart_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    DMA_Stream_TypeDef *dma =
        (DMA_Stream_TypeDef *)((cfg->dma_rx.init.Instance));

    HAL_UART_IRQHandler(&cfg->init);

    if (cfg->init.gState == HAL_UART_STATE_READY &&
        cfg->p.tx_state == USART_TRANSMIT) {
        xSemaphoreGiveFromISR(cfg->p.tx_sem, &xHigherPriorityTaskWoken);
        cfg->p.tx_count = dma->NDTR;
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (cfg->init.RxState == HAL_UART_STATE_READY &&
        cfg->p.rx_state == USART_RECEIVE && cfg->mode == USART_TIMEOUT) {
        xSemaphoreGiveFromISR(cfg->p.rx_sem, &xHigherPriorityTaskWoken);
        cfg->p.rx_count = dma->NDTR;
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (cfg->init.Instance->ISR & USART_ISR_IDLE &&
        cfg->p.rx_state == USART_RECEIVE && cfg->mode == USART_IDLE) {
        SET_BIT(cfg->init.Instance->ICR, USART_ICR_IDLECF);
        cfg->p.rx_count = cfg->init.RxXferSize - dma->NDTR;
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
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_DMA_IRQHandler(&cfg->dma_tx.init);
    if (cfg->dma_rx.init.State == HAL_DMA_STATE_READY &&
        cfg->mode == USART_TIMEOUT) {
        xSemaphoreGiveFromISR(cfg->p.rx_sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

int usart_id_init(usart_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case USART1_BASE:
            __HAL_RCC_USART1_CLK_ENABLE();
            cfg->p.id = USART1_IRQn;
            break;
        case USART2_BASE:
            __HAL_RCC_USART2_CLK_ENABLE();
            cfg->p.id = USART2_IRQn;
            break;
        case USART3_BASE:
            __HAL_RCC_USART3_CLK_ENABLE();
            cfg->p.id = USART3_IRQn;
            break;
        case UART4_BASE:
            __HAL_RCC_UART4_CLK_ENABLE();
            cfg->p.id = UART4_IRQn;
            break;
        case UART5_BASE:
            __HAL_RCC_UART5_CLK_ENABLE();
            cfg->p.id = UART5_IRQn;
            break;
        case USART6_BASE:
            __HAL_RCC_USART6_CLK_ENABLE();
            cfg->p.id = USART6_IRQn;
            break;
        case UART7_BASE:
            __HAL_RCC_UART7_CLK_ENABLE();
            cfg->p.id = UART7_IRQn;
            break;
        case UART8_BASE:
            __HAL_RCC_UART8_CLK_ENABLE();
            cfg->p.id = UART8_IRQn;
            break;
        default:
            return EINVAL;
    }

    return 0;
}

int usart_clock_init(usart_t *cfg) {
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

#ifdef USART_POSIX_OSA

void USART_ReadTask(void *cfg_ptr) {
    usart_t *cfg = (usart_t *)cfg_ptr;
    uint8_t *buf = calloc(cfg->buf_size, sizeof(uint8_t));
    while (1) {
        if (USART_Receive(cfg, buf, cfg->buf_size)) {
            cfg->error = ERROR;
        } else {
            cfg->error = SUCCESS;
        }
        RingBuf_Write(cfg->read_buf, buf, cfg->p.rx_count);
        xSemaphoreGive(cfg->read_sem);
    }
}

void USART_WriteTask(void *cfg_ptr) {
    usart_t *cfg = (usart_t *)cfg_ptr;
    uint8_t *buf = calloc(cfg->buf_size, sizeof(uint8_t));
    uint16_t length;
    while (1) {
        xSemaphoreTake(cfg->write_sem, portMAX_DELAY);
        length = RingBuf_GetDataSize(cfg->write_buf);
        length = RingBuf_Read(cfg->write_buf, buf, length);
        if (USART_Transmit(cfg, buf, length)) {
            cfg->error = ERROR;
        } else {
            cfg->error = SUCCESS;
        }
    }
}

int usart_posix_open(void *devcfg, void *file, const char *pathname,
                     int flags) {
    (void)flags;
    (void)file;
    usart_t *cfg = (usart_t *)devcfg;

    errno = 0;

    if (cfg->tasks_init) return 0;

    if (cfg->buf_size <= 0) {
        errno = ENXIO;
        return -1;
    }

    cfg->read_buf = RingBuf_Init(cfg->buf_size);
    cfg->write_buf = RingBuf_Init(cfg->buf_size);

    if (cfg->read_buf == NULL || cfg->write_buf == NULL) {
        errno = ENXIO;
        return -1;
    }

    cfg->read_sem = xSemaphoreCreateBinary();
    cfg->write_sem = xSemaphoreCreateBinary();
    if (cfg->read_sem == NULL) return -1;
    if (cfg->write_sem == NULL) return -1;

    static int usartnum = 0;
    char read_task_name[configMAX_TASK_NAME_LEN];
    char write_task_name[configMAX_TASK_NAME_LEN];
    sprintf(read_task_name, "ttyS%d_ReadTask", usartnum);
    sprintf(write_task_name, "ttyS%d_WriteTask", usartnum);
    usartnum++;

    xTaskCreate(USART_ReadTask, read_task_name, 512, cfg, cfg->task_priority,
                NULL);
    xTaskCreate(USART_WriteTask, write_task_name, 512, cfg, cfg->task_priority,
                NULL);

    cfg->tasks_init = true;
    return 0;
}

ssize_t usart_posix_write(void *devcfg, void *file, const void *buf,
                          size_t count) {
    ssize_t rv;
    (void)file;
    usart_t *cfg = (usart_t *)devcfg;

    errno = 0;
    rv = RingBuf_Write(cfg->write_buf, (uint8_t *)buf, count);
    xSemaphoreGive(cfg->write_sem);
    if (cfg->error) {
        errno = EPROTO;
        return -1;
    }
    return rv;
}

ssize_t usart_posix_read(void *devcfg, void *file, void *buf, size_t count) {
    ssize_t rv;
    errno = 0;
    (void)file;
    usart_t *cfg = (usart_t *)devcfg;

    xSemaphoreTake(cfg->read_sem, portMAX_DELAY);
    rv = RingBuf_Read(cfg->read_buf, (uint8_t *)buf, count);
    if (cfg->error) {
        errno = EPROTO;
        return -1;
    }
    return rv;
}

int usart_posix_close(void *devcfg, void *file) {
    errno = 0;
    (void)file;
    usart_t *cfg = (usart_t *)devcfg;
    (void)cfg;
    return 0;
}

#endif
