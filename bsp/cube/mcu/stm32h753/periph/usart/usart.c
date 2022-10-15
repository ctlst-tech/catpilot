#include "usart.h"

int USART_Init(usart_cfg_t *cfg) {

    if (cfg->inst.periph_init) return 0;

    portENTER_CRITICAL();

    int rv = 0;
    if ((rv = USART_ClockEnable(cfg)) != 0) return rv;

    if ((rv = GPIO_Init(cfg->gpio_rx_cfg)) != 0) return rv;
    if ((rv = GPIO_Init(cfg->gpio_tx_cfg)) != 0) return rv;

    if (cfg->dma_tx_cfg != NULL) {
        cfg->dma_tx_cfg->DMA_InitStruct.Parent = &cfg->inst.USART_InitStruct;
        if ((rv = DMA_Init(cfg->dma_tx_cfg)) != 0) return rv;
    }

    if (cfg->dma_rx_cfg != NULL) {
        cfg->dma_rx_cfg->DMA_InitStruct.Parent = &cfg->inst.USART_InitStruct;
        if ((rv = DMA_Init(cfg->dma_rx_cfg)) != 0) return rv;
    }

    cfg->inst.USART_InitStruct.Instance = cfg->USART;
    cfg->inst.USART_InitStruct.Init.BaudRate = cfg->speed;
    cfg->inst.USART_InitStruct.Init.ClockPrescaler = 1;
    cfg->inst.USART_InitStruct.Init.Mode = UART_MODE_TX_RX;
    cfg->inst.USART_InitStruct.Init.OverSampling = UART_OVERSAMPLING_16;
    cfg->inst.USART_InitStruct.Init.Parity = UART_PARITY_NONE;
    cfg->inst.USART_InitStruct.Init.StopBits = UART_STOPBITS_1;
    cfg->inst.USART_InitStruct.Init.WordLength = UART_WORDLENGTH_8B;

    cfg->inst.USART_InitStruct.hdmatx = &cfg->dma_tx_cfg->DMA_InitStruct;
    cfg->inst.USART_InitStruct.hdmarx = &cfg->dma_rx_cfg->DMA_InitStruct;

    if (HAL_UART_Init(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;

    if (cfg->inst.tx_mutex == NULL) cfg->inst.tx_mutex = xSemaphoreCreateMutex();
    if (cfg->inst.rx_mutex == NULL) cfg->inst.rx_mutex = xSemaphoreCreateMutex();
    if (cfg->inst.tx_semaphore == NULL) cfg->inst.tx_semaphore = xSemaphoreCreateBinary();
    if (cfg->inst.rx_semaphore == NULL) cfg->inst.rx_semaphore = xSemaphoreCreateBinary();

    USART_EnableIRQ(cfg);

    cfg->inst.periph_init = true;

    portEXIT_CRITICAL();

    return rv;
}

int USART_ReInit(usart_cfg_t *cfg) {
    xSemaphoreGive(cfg->inst.rx_semaphore);
    xSemaphoreGive(cfg->inst.tx_semaphore);
    if (HAL_UART_DeInit(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    HAL_UART_AbortReceive(&cfg->inst.USART_InitStruct);
    if (HAL_UART_Init(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    // if(USART_Init(cfg) != HAL_OK) return EINVAL;
    return 0;
}

int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0) return EINVAL;
    if (pdata == NULL) return EINVAL;

    if (xSemaphoreTake(cfg->inst.tx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->inst.tx_semaphore, 0);

    cfg->inst.tx_state = USART_TRANSMIT;

    HAL_UART_Transmit_DMA(&cfg->inst.USART_InitStruct, pdata, length);

    if (xSemaphoreTake(cfg->inst.tx_semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.tx_state = USART_FREE;
    xSemaphoreGive(cfg->inst.tx_mutex);

    return rv;
}

int USART_Receive(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0) return EINVAL;
    if (pdata == NULL) return EINVAL;

    if (xSemaphoreTake(cfg->inst.rx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->inst.rx_semaphore, 0);

    cfg->inst.rx_state = USART_RECEIVE;

    if (cfg->mode == USART_IDLE) {
        SET_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
        SET_BIT(cfg->USART->CR1, USART_CR1_IDLEIE);
    }
    HAL_UART_Receive_DMA(&cfg->inst.USART_InitStruct, pdata, length);

    if (xSemaphoreTake(cfg->inst.rx_semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.rx_state = USART_FREE;
    xSemaphoreGive(cfg->inst.rx_mutex);

    return rv;
}

int USART_TransmitReceive(usart_cfg_t *cfg, uint8_t *tx_pdata, uint8_t *rx_pdata,
    uint16_t tx_length, uint16_t rx_length) {
    int rv = 0;

    if (tx_length == 0) return EINVAL;
    if (rx_length == 0) return EINVAL;
    if (tx_pdata == NULL) return EINVAL;
    if (rx_pdata == NULL) return EINVAL;

    if ((xSemaphoreTake(cfg->inst.tx_mutex, 0) == pdFALSE) ||
        (xSemaphoreTake(cfg->inst.rx_mutex, 0) == pdFALSE)) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->inst.rx_semaphore, 0);

    cfg->inst.tx_state = USART_TRANSMIT;
    cfg->inst.rx_state = USART_RECEIVE;

    SET_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
    SET_BIT(cfg->USART->CR1, USART_CR1_IDLEIE);
    HAL_UART_Receive_DMA(&cfg->inst.USART_InitStruct, rx_pdata, rx_length);
    HAL_UART_Transmit_DMA(&cfg->inst.USART_InitStruct, tx_pdata, tx_length);

    if (xSemaphoreTake(cfg->inst.rx_semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.tx_state = USART_FREE;
    cfg->inst.rx_state = USART_FREE;
    xSemaphoreGive(cfg->inst.tx_mutex);
    xSemaphoreGive(cfg->inst.rx_mutex);

    return rv;
}

int USART_SetSpeed(usart_cfg_t *cfg, uint32_t speed) {
    cfg->speed = speed;
    cfg->inst.USART_InitStruct.Init.BaudRate = cfg->speed;
    cfg->USART->BRR = (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetPCLK1Freq(),
        cfg->inst.USART_InitStruct.Init.BaudRate,
        cfg->inst.USART_InitStruct.Init.ClockPrescaler));
    return 0;
}

uint32_t USART_GetSpeed(usart_cfg_t *cfg) {
    return (cfg->speed);
}

int USART_Handler(usart_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    DMA_Stream_TypeDef *dma_inst =
        (DMA_Stream_TypeDef *)((cfg->dma_rx_cfg->DMA_InitStruct.Instance));

    HAL_UART_IRQHandler(&cfg->inst.USART_InitStruct);

    if (cfg->inst.USART_InitStruct.gState == HAL_UART_STATE_READY &&
        cfg->inst.tx_state == USART_TRANSMIT) {
        xSemaphoreGiveFromISR(cfg->inst.tx_semaphore, &xHigherPriorityTaskWoken);
        cfg->inst.tx_count = dma_inst->NDTR;
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (cfg->inst.USART_InitStruct.RxState == HAL_UART_STATE_READY &&
        cfg->inst.rx_state == USART_RECEIVE &&
        cfg->mode == USART_TIMEOUT) {
        xSemaphoreGiveFromISR(cfg->inst.rx_semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if (cfg->USART->ISR & USART_ISR_IDLE &&
        cfg->inst.rx_state == USART_RECEIVE &&
        cfg->mode == USART_IDLE) {
        SET_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
        cfg->inst.rx_count = cfg->inst.USART_InitStruct.RxXferSize -
            dma_inst->NDTR;
        HAL_UART_AbortReceive(&cfg->inst.USART_InitStruct);
        CLEAR_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
        CLEAR_BIT(cfg->USART->CR1, USART_CR1_IDLEIE);
        xSemaphoreGiveFromISR(cfg->inst.rx_semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int USART_DMA_TX_Handler(usart_cfg_t *cfg) {
    int rv = 0;
    rv = DMA_IRQHandler(cfg->dma_tx_cfg);
    return rv;
}

int USART_DMA_RX_Handler(usart_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int rv = 0;
    rv = DMA_IRQHandler(cfg->dma_rx_cfg);
    if (cfg->dma_rx_cfg->DMA_InitStruct.State == HAL_DMA_STATE_READY &&
        cfg->mode == USART_TIMEOUT) {
        xSemaphoreGiveFromISR(cfg->inst.rx_semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    return rv;
}

int USART_EnableIRQ(usart_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.IRQ);
    return 0;
}

int USART_DisableIRQ(usart_cfg_t *cfg) {
    HAL_NVIC_DisableIRQ(cfg->inst.IRQ);
    return 0;
}

int USART_ClockEnable(usart_cfg_t *cfg) {
    switch ((uint32_t)(cfg->USART)) {

#ifdef USART1
    case USART1_BASE:
        __HAL_RCC_USART1_CLK_ENABLE();
        cfg->inst.IRQ = USART1_IRQn;
        break;
#endif

#ifdef USART2
    case USART2_BASE:
        __HAL_RCC_USART2_CLK_ENABLE();
        cfg->inst.IRQ = USART2_IRQn;
        break;
#endif

#ifdef USART3
    case USART3_BASE:
        __HAL_RCC_USART3_CLK_ENABLE();
        cfg->inst.IRQ = USART3_IRQn;
        break;
#endif

#ifdef UART4
    case UART4_BASE:
        __HAL_RCC_UART4_CLK_ENABLE();
        cfg->inst.IRQ = UART4_IRQn;
        break;
#endif

#ifdef UART5
    case UART5_BASE:
        __HAL_RCC_UART5_CLK_ENABLE();
        cfg->inst.IRQ = UART5_IRQn;
        break;
#endif

#ifdef USART6
    case USART6_BASE:
        __HAL_RCC_USART6_CLK_ENABLE();
        cfg->inst.IRQ = USART6_IRQn;
        break;
#endif

#ifdef UART7
    case UART7_BASE:
        __HAL_RCC_UART7_CLK_ENABLE();
        cfg->inst.IRQ = UART7_IRQn;
        break;
#endif

#ifdef UART8
    case UART8_BASE:
        __HAL_RCC_UART8_CLK_ENABLE();
        cfg->inst.IRQ = UART8_IRQn;
        break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}

#ifdef USART_POSIX_OSA

void USART_ReadTask(void *cfg_ptr) {
    usart_cfg_t *cfg = (usart_cfg_t *)cfg_ptr;
    uint8_t *buf = calloc(cfg->buf_size, sizeof(uint8_t));
    while (1) {
        if (USART_Receive(cfg, buf, cfg->buf_size)) {
            cfg->inst.error = ERROR;
        } else {
            cfg->inst.error = SUCCESS;
        }
        fifo_write(cfg->inst.read_buf, buf, cfg->inst.rx_count);
    }
}

void USART_WriteTask(void *cfg_ptr) {
    usart_cfg_t *cfg = (usart_cfg_t *)cfg_ptr;
    uint8_t *buf = calloc(cfg->buf_size, sizeof(uint8_t));
    uint16_t length;
    while (1) {
        length = fifo_get_data_size(cfg->inst.write_buf);
        length = fifo_read(cfg->inst.write_buf, buf, length);
        if (USART_Transmit(cfg, buf, length)) {
            cfg->inst.error = ERROR;
        } else {
            cfg->inst.error = SUCCESS;
        }
    }
}

int usart_posix_open(void *devcfg, void *file, const char *pathname, int flags) {
    (void)flags;
    (void)file;
    usart_cfg_t *cfg = (usart_cfg_t *)devcfg;

    errno = 0;

    if (cfg->inst.tasks_init) return 0;

    if (cfg->buf_size <= 0) {
        errno = ENXIO;
        return -1;
    }

    cfg->inst.read_buf = fifo_init(cfg->buf_size);
    cfg->inst.write_buf = fifo_init(cfg->buf_size);

    if (cfg->inst.read_buf == NULL ||
        cfg->inst.write_buf == NULL) {
        errno = ENXIO;
        return -1;
    }

    char read_task_name[configMAX_TASK_NAME_LEN];
    char write_task_name[configMAX_TASK_NAME_LEN];
    strcpy(read_task_name, pathname);
    strcpy(write_task_name, pathname);
    strcat(read_task_name, "_read");
    strcat(write_task_name, "_write");

    xTaskCreate(USART_ReadTask,
        read_task_name,
        512,
        cfg,
        cfg->task_priority,
        NULL);
    xTaskCreate(USART_WriteTask,
        write_task_name,
        512,
        cfg,
        cfg->task_priority,
        NULL);

    cfg->inst.tasks_init = true;
    return 0;
}

ssize_t usart_posix_write(void *devcfg, void *file, const void *buf, size_t count) {
    ssize_t rv;
    (void)file;
    usart_cfg_t *cfg = (usart_cfg_t *)devcfg;

    errno = 0;
    rv = fifo_write(cfg->inst.write_buf,
        (uint8_t *)buf,
        count);

    if (cfg->inst.error) {
        errno = EPROTO;
        return -1;
    }

    return rv;
}

ssize_t usart_posix_read(void *devcfg, void *file, void *buf, size_t count) {
    ssize_t rv;
    errno = 0;
    (void)file;
    usart_cfg_t *cfg = (usart_cfg_t *)devcfg;

    rv = fifo_read(cfg->inst.read_buf,
        (uint8_t *)buf,
        count);

    if (cfg->inst.error) {
        errno = EPROTO;
        return -1;
    }

    return rv;
}

int usart_posix_close(void *devcfg, void *file) {
    errno = 0;
    (void)file;
    usart_cfg_t *cfg = (usart_cfg_t *)devcfg;
    (void)cfg;
    return 0;
}

#endif
