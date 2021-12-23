#include "usart.h"

int USART_Init(usart_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = USART_ClockEnable(cfg)) != 0) return rv;
    
    if((rv = GPIO_Init(cfg->rx_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->tx_cfg)) != 0) return rv;

    cfg->inst.USART_InitStruct.Instance = cfg->USART;
    cfg->inst.USART_InitStruct.Init.BaudRate = cfg->speed;
    cfg->inst.USART_InitStruct.Init.Mode = UART_MODE_TX_RX;
    cfg->inst.USART_InitStruct.Init.OverSampling = UART_OVERSAMPLING_16;
    cfg->inst.USART_InitStruct.Init.Parity = UART_PARITY_NONE;
    cfg->inst.USART_InitStruct.Init.StopBits = UART_STOPBITS_1;
    cfg->inst.USART_InitStruct.Init.WordLength = UART_WORDLENGTH_8B;

    if(HAL_UART_Init(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    
    if(cfg->inst.mutex == NULL) cfg->inst.mutex = xSemaphoreCreateMutex();
    if(cfg->inst.semaphore == NULL) cfg->inst.semaphore = xSemaphoreCreateBinary();

    portEXIT_CRITICAL();
    
    return rv;
}

int USART_ReInit(usart_cfg_t *cfg) {
    if(HAL_UART_DeInit(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_UART_Init(&cfg->inst.USART_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int USART_Transmit(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdTRUE) {

        cfg->inst.state = USART_TRANSMIT;

        USART_ReInit(cfg);
        USART_EnableIRQ(cfg);

        HAL_UART_Transmit_IT(&cfg->inst.USART_InitStruct, pdata, length);

        if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE &&
        (__HAL_UART_GET_FLAG(&cfg->inst.USART_InitStruct, USART_ISR_TXE))) {
            xSemaphoreGive(cfg->inst.mutex);
            HAL_UART_AbortTransmit_IT(&cfg->inst.USART_InitStruct);
            cfg->inst.state = USART_FREE;
            return ETIME;
        } else {
            xSemaphoreGive(cfg->inst.mutex);
            cfg->inst.state = USART_FREE;
            return 0;
        }
    } else {
        cfg->inst.state = USART_FREE;
        return ETIME;
    }
}

int USART_Receive(usart_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdTRUE) {

        cfg->inst.state = USART_RECEIVE;

        HAL_UART_Init(&cfg->inst.USART_InitStruct);
        USART_EnableIRQ(cfg);
        
        HAL_UART_Receive_IT(&cfg->inst.USART_InitStruct, pdata, length);

        if(xSemaphoreTake(cfg->inst.semaphore, portMAX_DELAY) == pdFALSE &&
        !(__HAL_UART_GET_FLAG(&cfg->inst.USART_InitStruct, USART_ISR_RXNE))) {
            xSemaphoreGive(cfg->inst.mutex);
            HAL_UART_AbortReceive_IT(&cfg->inst.USART_InitStruct);
            HAL_UART_DeInit(&cfg->inst.USART_InitStruct);
            cfg->inst.state = USART_FREE;
            return ETIME;
        } else {
            xSemaphoreGive(cfg->inst.mutex);
            HAL_UART_DeInit(&cfg->inst.USART_InitStruct);
            cfg->inst.state = USART_FREE;
            return 0;
        }
    } else {
        cfg->inst.state = USART_FREE;
        return ETIME;
    }
}

int USART_Handler(usart_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_UART_IRQHandler(&cfg->inst.USART_InitStruct);

    if(cfg->inst.USART_InitStruct.gState == HAL_UART_STATE_READY &&
    cfg->inst.state == USART_TRANSMIT) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        USART_DisableIRQ(cfg);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    if(cfg->inst.USART_InitStruct.RxState == HAL_UART_STATE_READY &&
    cfg->inst.state == USART_RECEIVE) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        USART_DisableIRQ(cfg);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
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
