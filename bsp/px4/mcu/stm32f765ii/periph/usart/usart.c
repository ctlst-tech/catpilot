#include "usart.h"

static int usart_num = 0;
static usart_cfg_t *usart_fd[8];

int USART_Init(usart_cfg_t *cfg) {

    if(cfg->inst.periph_init) return 0;

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

    if(cfg->inst.tx_mutex == NULL) cfg->inst.tx_mutex = xSemaphoreCreateMutex();
    if(cfg->inst.rx_mutex == NULL) cfg->inst.rx_mutex = xSemaphoreCreateMutex();
    if(cfg->inst.tx_semaphore == NULL) cfg->inst.tx_semaphore = xSemaphoreCreateBinary();
    if(cfg->inst.rx_semaphore == NULL) cfg->inst.rx_semaphore = xSemaphoreCreateBinary();

    USART_EnableIRQ(cfg);

    cfg->inst.periph_init = true;

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
    xSemaphoreGive(cfg->inst.rx_semaphore);
    xSemaphoreGive(cfg->inst.tx_semaphore);
    if(HAL_UART_DeInit(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    HAL_UART_AbortReceive(&cfg->inst.USART_InitStruct);
    if(HAL_UART_Init(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    // if(USART_Init(cfg) != HAL_OK) return EINVAL;
    return 0;
}

int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.tx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->inst.tx_semaphore, 0);

    cfg->inst.tx_state = USART_TRANSMIT;

    HAL_UART_Transmit_DMA(&cfg->inst.USART_InitStruct, pdata, length);

    if(xSemaphoreTake(cfg->inst.tx_semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
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

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.rx_mutex, 0) == pdFALSE) {
        return EBUSY;
    }

    xSemaphoreTake(cfg->inst.rx_semaphore, 0);

    cfg->inst.rx_state = USART_RECEIVE;

    if(cfg->mode == USART_IDLE) {
        SET_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
        SET_BIT(cfg->USART->CR1, USART_CR1_IDLEIE);
    }
    HAL_UART_Receive_DMA(&cfg->inst.USART_InitStruct, pdata, length);

    if(xSemaphoreTake(cfg->inst.rx_semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
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

    if(tx_length == 0) return EINVAL;
    if(rx_length == 0) return EINVAL;
    if(tx_pdata ==  NULL) return EINVAL;
    if(rx_pdata ==  NULL) return EINVAL;

    if((xSemaphoreTake(cfg->inst.tx_mutex, 0) == pdFALSE) ||
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

    if(xSemaphoreTake(cfg->inst.rx_semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
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

int USART_Handler(usart_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_UART_IRQHandler(&cfg->inst.USART_InitStruct);

    if(cfg->inst.USART_InitStruct.gState == HAL_UART_STATE_READY &&
        cfg->inst.tx_state == USART_TRANSMIT) {
            xSemaphoreGiveFromISR(cfg->inst.tx_semaphore, &xHigherPriorityTaskWoken);
            cfg->inst.tx_count = cfg->dma_rx_cfg->DMA_InitStruct.Instance->NDTR;
            if(xHigherPriorityTaskWoken == pdTRUE) {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
    }

    if(cfg->inst.USART_InitStruct.RxState == HAL_UART_STATE_READY &&
        cfg->inst.rx_state == USART_RECEIVE &&
        cfg->mode == USART_TIMEOUT) {
            xSemaphoreGiveFromISR(cfg->inst.rx_semaphore, &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE) {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
    }

    if(cfg->USART->ISR & USART_ISR_IDLE &&
        cfg->inst.rx_state == USART_RECEIVE &&
        cfg->mode == USART_IDLE) {
            SET_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
            cfg->inst.rx_count = cfg->inst.USART_InitStruct.RxXferSize -
                                 cfg->dma_rx_cfg->DMA_InitStruct.Instance->NDTR;
            HAL_UART_AbortReceive(&cfg->inst.USART_InitStruct);
            CLEAR_BIT(cfg->USART->ICR, USART_ICR_IDLECF);
            CLEAR_BIT(cfg->USART->CR1, USART_CR1_IDLEIE);
            xSemaphoreGiveFromISR(cfg->inst.rx_semaphore, &xHigherPriorityTaskWoken);
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
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int rv = 0;
    rv = DMA_IRQHandler(cfg->dma_rx_cfg);
    if(cfg->dma_rx_cfg->DMA_InitStruct.State == HAL_DMA_STATE_READY &&
       cfg->mode == USART_TIMEOUT) {
            xSemaphoreGiveFromISR(cfg->inst.rx_semaphore, &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE) {
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

    int tcgetattr(int __fd, struct termios *__termios_p) {
        int dev_fd = __fd - 3;
        if(usart_fd[dev_fd] == NULL) {
            errno = EBADF;
            return -1;
        }
        __termios_p->c_ispeed = usart_fd[dev_fd]->speed;
        __termios_p->c_ospeed = usart_fd[dev_fd]->speed;
        return __fd;
    }

    int tcsetattr(int __fd, int __optional_actions,
                 const struct termios *__termios_p) {
        int rv = 0;
        (void)__optional_actions;
        int dev_fd = __fd - 3;
        if(usart_fd[dev_fd] == NULL) {
            errno = EBADF;
            return -1;
        }
         // FIXME
        usart_fd[dev_fd]->speed = __termios_p->c_ispeed;
        usart_fd[dev_fd]->speed = __termios_p->c_ospeed;
        usart_fd[dev_fd]->inst.USART_InitStruct.Init.BaudRate = usart_fd[dev_fd]->speed;
        usart_fd[dev_fd]->USART->BRR =
                                (uint16_t)(UART_DIV_SAMPLING16(HAL_RCC_GetPCLK1Freq(),
                                 usart_fd[dev_fd]->inst.USART_InitStruct.Init.BaudRate));
        // rv = USART_ReInit(usart_fd[dev_fd]);

        if(rv) {
            errno = EPROTO;
            return -1;
        }
        return 0;
    }

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

    void USART_ReadTask(void *cfg_ptr) {
        usart_cfg_t *cfg = (usart_cfg_t *)cfg_ptr;
        uint8_t *buf = calloc(cfg->buf_size, sizeof(uint8_t));
        while(1) {
            if(USART_Receive(cfg, buf, cfg->buf_size)) {
                cfg->inst.error = ERROR;
            } else {
                cfg->inst.error = SUCCESS;
            }
            RingBuf_Write(cfg->inst.read_buf, buf, cfg->inst.rx_count);
            xSemaphoreGive(cfg->inst.read_semaphore);
        }
    }

    void USART_WriteTask(void *cfg_ptr) {
        usart_cfg_t *cfg = (usart_cfg_t *)cfg_ptr;
        uint8_t *buf = calloc(cfg->buf_size, sizeof(uint8_t));
        uint16_t length;
        while(1) {
            xSemaphoreTake(cfg->inst.write_semaphore, portMAX_DELAY);
            length = RingBuf_GetDataSize(cfg->inst.write_buf);
            length = RingBuf_Read(cfg->inst.write_buf, buf, length);
            if(USART_Transmit(cfg, buf, length)) {
                cfg->inst.error = ERROR;
            } else {
                cfg->inst.error = SUCCESS;
            }
        }
    }

    int usart_posix_open(const char *pathname, int flags) {
        (void)pathname;
        (void)flags;
        errno = 0;
        for(int i = 0; i < usart_num; i++) {
            if(strcmp(pathname, __dev[i].path)) continue;
            if(usart_fd[i]->inst.tasks_init) return (i + 3);
            if(usart_fd[i]->buf_size <= 0) {
                errno = ENXIO;
                return -1;
            }
            usart_fd[i]->inst.read_buf = RingBuf_Init(usart_fd[i]->buf_size);
            usart_fd[i]->inst.write_buf = RingBuf_Init(usart_fd[i]->buf_size);
            if(usart_fd[i]->inst.read_buf == NULL ||
                usart_fd[i]->inst.write_buf == NULL) {
                    errno = ENXIO;
                    return -1;
                }
            char read_task_name[configMAX_TASK_NAME_LEN];
            char write_task_name[configMAX_TASK_NAME_LEN];
            usart_fd[i]->inst.read_semaphore = xSemaphoreCreateBinary();
            usart_fd[i]->inst.write_semaphore = xSemaphoreCreateBinary();
            if(usart_fd[i]->inst.read_semaphore == NULL) return -1;
            if(usart_fd[i]->inst.write_semaphore == NULL) return -1;
            sprintf(read_task_name, "ttyS%d_ReadTask\n", i);
            sprintf(write_task_name, "ttyS%d_WriteTask\n", i);
            xTaskCreate(USART_ReadTask,
                        read_task_name,
                        usart_fd[i]->buf_size + 256,
                        usart_fd[i],
                        usart_fd[i]->task_priority,
                        NULL);
            xTaskCreate(USART_WriteTask,
                        write_task_name,
                        usart_fd[i]->buf_size + 256,
                        usart_fd[i],
                        usart_fd[i]->task_priority,
                        NULL);
            usart_fd[i]->inst.tasks_init = true;
            return (i + 3);
        }
        errno = ENODEV;
        return -1;
    }

    ssize_t usart_posix_write(int fd, const void *buf, size_t count) {
        ssize_t rv;
        errno = 0;
        rv = RingBuf_Write(usart_fd[fd]->inst.write_buf,
                           (uint8_t *)buf,
                           count);
        xSemaphoreGive(usart_fd[fd]->inst.write_semaphore);
        if(usart_fd[fd]->inst.error) {
            errno = EPROTO;
            return -1;
        }
        return rv;
    }

    ssize_t usart_posix_read(int fd, const void *buf, size_t count) {
        ssize_t rv;
        errno = 0;
        xSemaphoreTake(usart_fd[fd]->inst.read_semaphore, portMAX_DELAY);
        rv = RingBuf_Read(usart_fd[fd]->inst.read_buf,
                           (uint8_t *)buf,
                           count);
        if(usart_fd[fd]->inst.error) {
            errno = EPROTO;
            return -1;
        }
        return rv;
    }

    int usart_posix_close(int fd) {
        errno = 0;
        (void)fd;
        return 0;
    }

#endif
