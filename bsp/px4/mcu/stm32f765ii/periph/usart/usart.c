#include "usart.h"

static int usart_num = 0;
static usart_cfg_t *usart_fd[8];

int USART_Init(usart_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = USART_ClockEnable(cfg)) != 0) return rv;

    if((rv = GPIO_Init(cfg->gpio_rx_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->gpio_tx_cfg)) != 0) return rv;

    if(cfg->dma_tx_cfg != NULL) {
        cfg->dma_tx_cfg->DMA_InitStruct.Parent = &cfg->inst.USART_InitStruct;
        if((rv = DMA_Init(cfg->dma_tx_cfg)) != 0) return rv;
    }

    if(cfg->dma_rx_cfg != NULL) {
        cfg->dma_rx_cfg->DMA_InitStruct.Parent = &cfg->inst.USART_InitStruct;
        if((rv = DMA_Init(cfg->dma_rx_cfg)) != 0) return rv;
    }

    cfg->inst.USART_InitStruct.Instance = cfg->USART;
    cfg->inst.USART_InitStruct.Init.BaudRate = cfg->speed;
    cfg->inst.USART_InitStruct.Init.Mode = UART_MODE_TX_RX;
    cfg->inst.USART_InitStruct.Init.OverSampling = UART_OVERSAMPLING_16;
    cfg->inst.USART_InitStruct.Init.Parity = UART_PARITY_NONE;
    cfg->inst.USART_InitStruct.Init.StopBits = UART_STOPBITS_1;
    cfg->inst.USART_InitStruct.Init.WordLength = UART_WORDLENGTH_8B;

    cfg->inst.USART_InitStruct.hdmatx = &cfg->dma_tx_cfg->DMA_InitStruct;
    cfg->inst.USART_InitStruct.hdmarx = &cfg->dma_rx_cfg->DMA_InitStruct;

    if(HAL_UART_Init(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;

    if(cfg->inst.mutex == NULL) cfg->inst.mutex = xSemaphoreCreateMutex();
    if(cfg->inst.semaphore == NULL) cfg->inst.semaphore = xSemaphoreCreateBinary();

    USART_EnableIRQ(cfg);

    portEXIT_CRITICAL();

    #ifdef USART_POSIX_OSA
        usart_fd[usart_num] = cfg;
        sprintf(__dev[usart_num].path, "/dev/ttyS%d", usart_num);
        __dev[usart_num].open = usart_posix_open;
        __dev[usart_num].write = usart_posix_write;
        __dev[usart_num].read = usart_posix_read;
        __dev[usart_num].close = usart_posix_close;
        usart_num++;
    #endif

    return rv;
}

int USART_ReInit(usart_cfg_t *cfg) {
    if(HAL_UART_DeInit(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_UART_Init(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = USART_TRANSMIT;

    HAL_UART_Transmit_DMA(&cfg->inst.USART_InitStruct, pdata, length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = USART_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int USART_Receive(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = USART_RECEIVE;

    HAL_UART_Receive_DMA(&cfg->inst.USART_InitStruct, pdata, length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = USART_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int USART_TransmitReceive(usart_cfg_t *cfg, uint8_t *tx_pdata, uint8_t *rx_pdata, uint16_t tx_length, uint16_t rx_length) {
    int rv = 0;

    if(tx_length == 0) return EINVAL;
    if(rx_length == 0) return EINVAL;
    if(tx_pdata ==  NULL) return EINVAL;
    if(rx_pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = USART_RECEIVE;

    SET_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
    SET_BIT(cfg->USART->CR1, USART_CR1_IDLEIE);
    HAL_UART_Receive_DMA(&cfg->inst.USART_InitStruct, rx_pdata, rx_length);
    HAL_UART_Transmit_DMA(&cfg->inst.USART_InitStruct, tx_pdata, tx_length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = USART_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int USART_Handler(usart_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_UART_IRQHandler(&cfg->inst.USART_InitStruct);

    if(cfg->inst.USART_InitStruct.gState == HAL_UART_STATE_READY &&
        cfg->inst.state == USART_TRANSMIT) {
            xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE) {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
    }

    if(cfg->inst.USART_InitStruct.RxState == HAL_UART_STATE_READY &&
        cfg->inst.state == USART_RECEIVE &&
        cfg->mode == USART_TIMEOUT) {
            xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE) {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
    }

    if(cfg->USART->ISR & USART_ISR_IDLE &&
        cfg->inst.state == USART_RECEIVE &&
        cfg->mode == USART_IDLE) {
            SET_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
            HAL_UART_AbortReceive(&cfg->inst.USART_InitStruct);
            xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE) {
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
    int rv = 0;
    rv = DMA_IRQHandler(cfg->dma_rx_cfg);
    return rv;
}

int USART_EnableIRQ(usart_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.IRQ);
    return 0;
}

int USART_DisableIRQ(usart_cfg_t *cfg)  {
    HAL_NVIC_DisableIRQ(cfg->inst.IRQ);
    return 0;
}

int USART_ClockEnable(usart_cfg_t *cfg) {
    switch((uint32_t)(cfg->USART)) {

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

#ifdef USART_TERMIOS

    speed_t cfgetospeed(const struct termios *__termios_p) {
        return __termios_p->c_ospeed;
    }

    speed_t cfgetispeed(const struct termios *__termios_p) {
        return __termios_p->c_ispeed;
    }

    int cfsetospeed(struct termios *__termios_p, speed_t __speed) {
        __termios_p->c_ospeed = __speed;
        return 0;
    }

    int cfsetispeed(struct termios *__termios_p, speed_t __speed) {
        __termios_p->c_ispeed = __speed;
        return 0;
    }

    int tcflush(int __fd, int __queue_selector) {
        return 0;
    }

#endif

#ifdef USART_POSIX_OSA
    int usart_posix_open(const char *pathname, int flags) {
        (void)pathname;
        (void)flags;
        return 0;
    }

    ssize_t usart_posix_write(int fd, const void *buf, size_t count) {
        int rv;
        rv = USART_Transmit(usart_fd[fd], (uint8_t *)buf, (uint16_t)count);
        if(rv) return -1;
        return 0;
    }

    ssize_t usart_posix_read(int fd, const void *buf, size_t count) {
        int rv;
        rv = USART_Receive(usart_fd[fd], (uint8_t *)buf, (uint16_t)count);
        if(rv) return -1;
        return 0;
    }

    int usart_posix_close(int fd) {
        (void)fd;
        return 0;
    }
#endif
