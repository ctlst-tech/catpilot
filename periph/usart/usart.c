#include "usart.h"

int USART_Init(usart_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = USART_ClockEnable(cfg)) != 0) return rv;
    
    if((rv = GPIO_Init(cfg->rx_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->tx_cfg)) != 0) return rv;

    cfg->port.USART_InitStruct.Instance = cfg->USART;
    cfg->port.USART_InitStruct.Init.BaudRate = cfg->speed;
    cfg->port.USART_InitStruct.Init.Mode = UART_MODE_TX_RX;
    cfg->port.USART_InitStruct.Init.OverSampling = UART_OVERSAMPLING_16;
    cfg->port.USART_InitStruct.Init.Parity = UART_PARITY_NONE;
    cfg->port.USART_InitStruct.Init.StopBits = UART_STOPBITS_1;
    cfg->port.USART_InitStruct.Init.WordLength = UART_WORDLENGTH_8B;

    if(HAL_UART_Init(&cfg->port.USART_InitStruct) != HAL_OK) return EINVAL;
    
    if(cfg->port.mutex == NULL) cfg->port.mutex = xSemaphoreCreateMutex();
    if(cfg->port.semaphore == NULL) cfg->port.semaphore = xSemaphoreCreateBinary();

    portEXIT_CRITICAL();
    
    return rv;
}

int USART_ReInit(usart_cfg_t *cfg) {
    if(HAL_UART_DeInit(&cfg->port.USART_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_UART_Init(&cfg->port.USART_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {

    if(xSemaphoreTake(cfg->port.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdTRUE) {

        cfg->port.state = USART_TRANSMIT;

        USART_ReInit(cfg);
        USART_EnableIRQ(cfg);

        HAL_UART_Transmit_IT(&cfg->port.USART_InitStruct, pdata, length);

        if(xSemaphoreTake(cfg->port.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE &&
        (__HAL_UART_GET_FLAG(&cfg->port.USART_InitStruct, USART_ISR_TXE))) {
            xSemaphoreGive(cfg->port.mutex);
            HAL_UART_AbortTransmit_IT(&cfg->port.USART_InitStruct);
            cfg->port.state = USART_FREE;
            return ETIME;
        } else {
            xSemaphoreGive(cfg->port.mutex);
            cfg->port.state = USART_FREE;
            return 0;
        }
    } else {
        cfg->port.state = USART_FREE;
        return ETIME;
    }
}

int USART_Receive(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    if(xSemaphoreTake(cfg->port.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdTRUE) {

        cfg->port.state = USART_RECEIVE;

        HAL_UART_Init(&cfg->port.USART_InitStruct);
        USART_EnableIRQ(cfg);
        
        HAL_UART_Receive_IT(&cfg->port.USART_InitStruct, pdata, length);

        if(xSemaphoreTake(cfg->port.semaphore, portMAX_DELAY) == pdFALSE &&
        !(__HAL_UART_GET_FLAG(&cfg->port.USART_InitStruct, USART_ISR_RXNE))) {
            xSemaphoreGive(cfg->port.mutex);
            HAL_UART_AbortReceive_IT(&cfg->port.USART_InitStruct);
            HAL_UART_DeInit(&cfg->port.USART_InitStruct);
            cfg->port.state = USART_FREE;
            return ETIME;
        } else {
            xSemaphoreGive(cfg->port.mutex);
            HAL_UART_DeInit(&cfg->port.USART_InitStruct);
            cfg->port.state = USART_FREE;
            return 0;
        }
    } else {
        cfg->port.state = USART_FREE;
        return ETIME;
    }
}

int USART_Handler(usart_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_UART_IRQHandler(&cfg->port.USART_InitStruct);

    if(cfg->port.USART_InitStruct.gState == HAL_UART_STATE_READY &&
    cfg->port.state == USART_TRANSMIT) {
        xSemaphoreGiveFromISR(cfg->port.semaphore, &xHigherPriorityTaskWoken);
        USART_DisableIRQ(cfg);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if(cfg->port.USART_InitStruct.RxState == HAL_UART_STATE_READY &&
    cfg->port.state == USART_RECEIVE) {
        xSemaphoreGiveFromISR(cfg->port.semaphore, &xHigherPriorityTaskWoken);
        USART_DisableIRQ(cfg);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int USART_EnableIRQ(usart_cfg_t *cfg) {
    if(cfg->USART == USART1) {
        NVIC_EnableIRQ(USART1_IRQn);
        NVIC_SetPriority(USART1_IRQn, cfg->priority);
    } else if(cfg->USART == USART2) {
        HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
        HAL_NVIC_SetPriority(USART2_IRQn, cfg->priority, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        // NVIC_EnableIRQ(USART2_IRQn);
        // NVIC_SetPriority(USART2_IRQn, cfg->priority);
    } else if(cfg->USART == USART3) {
        NVIC_EnableIRQ(USART3_IRQn);
        NVIC_SetPriority(USART3_IRQn, cfg->priority);
    } else {
        return EINVAL;
    }
    return 0;
}

int USART_DisableIRQ(usart_cfg_t *cfg)  {
    if(cfg->USART == USART1) {
        NVIC_DisableIRQ(USART1_IRQn);
    } else if(cfg->USART == USART2) {
        NVIC_DisableIRQ(USART2_IRQn);
    } else if(cfg->USART == USART3) {
        NVIC_DisableIRQ(USART3_IRQn);
    } else {
        return EINVAL;
    }
    return 0;
}

int USART_ClockEnable(usart_cfg_t *cfg) {
    switch((uint32_t)(cfg->USART)) {

#ifdef USART1
        case USART1_BASE:
            __HAL_RCC_USART1_CLK_ENABLE();
            break;
#endif

#ifdef USART2
        case USART2_BASE:
            __HAL_RCC_USART2_CLK_ENABLE();
            break;
#endif

#ifdef USART3
        case USART3_BASE:
            __HAL_RCC_USART3_CLK_ENABLE();
            break;
#endif
    default:
        return EINVAL;
    }

    return 0;
}
