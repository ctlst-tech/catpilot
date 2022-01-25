#include "i2c.h"

int I2C_Init(i2c_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = I2C_ClockEnable(cfg)) != 0) return rv;

    if((rv = GPIO_Init(cfg->sda_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->scl_cfg)) != 0) return rv;

    if(cfg->dma_tx_cfg != NULL) {
        cfg->dma_tx_cfg->DMA_InitStruct.Parent = &cfg->inst.I2C_InitStruct;
        if((rv = DMA_Init(cfg->dma_tx_cfg)) != 0) return rv;
    }

    if(cfg->dma_rx_cfg != NULL) {
        cfg->dma_rx_cfg->DMA_InitStruct.Parent = &cfg->inst.I2C_InitStruct;
        if((rv = DMA_Init(cfg->dma_rx_cfg)) != 0) return rv;
    }

    cfg->inst.I2C_InitStruct.Instance = cfg->I2C;
    cfg->inst.I2C_InitStruct.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    cfg->inst.I2C_InitStruct.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    cfg->inst.I2C_InitStruct.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    cfg->inst.I2C_InitStruct.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    cfg->inst.I2C_InitStruct.Init.Timing = 0x6000030D; // From CubeMX configuration tool for FAST MODE

    cfg->inst.I2C_InitStruct.hdmatx = &cfg->dma_tx_cfg->DMA_InitStruct;
    cfg->inst.I2C_InitStruct.hdmarx = &cfg->dma_rx_cfg->DMA_InitStruct;

    if(HAL_I2C_Init(&cfg->inst.I2C_InitStruct) != HAL_OK) return EINVAL;
    I2C_EnableIRQ(cfg);

    if(cfg->inst.mutex == NULL) cfg->inst.mutex = xSemaphoreCreateMutex();
    if(cfg->inst.semaphore == NULL) cfg->inst.semaphore = xSemaphoreCreateBinary();

    portEXIT_CRITICAL();

    return rv;
}

int I2C_ReInit(i2c_cfg_t *cfg) {
    if(HAL_I2C_DeInit(&cfg->inst.I2C_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_I2C_Init(&cfg->inst.I2C_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int I2C_Transmit(i2c_cfg_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = I2C_TRANSMIT;

    HAL_I2C_Master_Transmit_DMA(&cfg->inst.I2C_InitStruct, address, pdata, length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = I2C_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int I2C_Receive(i2c_cfg_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = I2C_RECEIVE;

    HAL_I2C_Master_Receive_DMA(&cfg->inst.I2C_InitStruct, address, pdata, length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = I2C_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int I2C_EV_Handler(i2c_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_I2C_EV_IRQHandler(&cfg->inst.I2C_InitStruct);

    if(cfg->inst.I2C_InitStruct.State == HAL_I2C_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int I2C_ER_Handler(i2c_cfg_t *cfg) {
    HAL_I2C_ER_IRQHandler(&cfg->inst.I2C_InitStruct);
    return 0;
}

int I2C_DMA_TX_Handler(i2c_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_tx_cfg->DMA_InitStruct);

    if(cfg->inst.I2C_InitStruct.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int I2C_DMA_RX_Handler(i2c_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_rx_cfg->DMA_InitStruct);

    if(cfg->inst.I2C_InitStruct.State == HAL_I2C_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int I2C_EnableIRQ(i2c_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.EV_IRQ, cfg->priority, 0);
    HAL_NVIC_SetPriority(cfg->inst.ER_IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.EV_IRQ);
    HAL_NVIC_EnableIRQ(cfg->inst.ER_IRQ);
    return 0;
}

int I2C_DisableIRQ(i2c_cfg_t *cfg)  {
    HAL_NVIC_DisableIRQ(cfg->inst.EV_IRQ);
    HAL_NVIC_DisableIRQ(cfg->inst.ER_IRQ);
    return 0;
}

int I2C_ClockEnable(i2c_cfg_t *cfg) {
    switch((uint32_t)(cfg->I2C)) {

#ifdef I2C1
        case I2C1_BASE:
            __HAL_RCC_I2C1_CLK_ENABLE();
            cfg->inst.EV_IRQ = I2C1_EV_IRQn;
            cfg->inst.ER_IRQ = I2C1_ER_IRQn;
            break;
#endif

#ifdef I2C2
        case I2C2_BASE:
            __HAL_RCC_I2C2_CLK_ENABLE();
            cfg->inst.EV_IRQ = I2C2_EV_IRQn;
            cfg->inst.ER_IRQ = I2C2_ER_IRQn;
            break;
#endif

#ifdef I2C3
        case I2C3_BASE:
            __HAL_RCC_I2C3_CLK_ENABLE();
            cfg->inst.EV_IRQ = I2C3_EV_IRQn;
            cfg->inst.ER_IRQ = I2C3_ER_IRQn;
            break;
#endif

#ifdef I2C4
        case I2C4_BASE:
            __HAL_RCC_I2C4_CLK_ENABLE();
            cfg->inst.EV_IRQ = I2C4_EV_IRQn;
            cfg->inst.ER_IRQ = I2C4_ER_IRQn;
            break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}
