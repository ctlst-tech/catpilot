#include "spi.h"

int SPI_Init(spi_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = SPI_ClockEnable(cfg)) != 0) return rv;

    if((rv = GPIO_Init(cfg->mosi_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->miso_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->sck_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->cs_cfg)) != 0) return rv;

    if(cfg->dma_mosi_cfg != NULL) {
        cfg->dma_mosi_cfg->DMA_InitStruct.Parent = &cfg->inst.SPI_InitStruct;
        if((rv = DMA_Init(cfg->dma_mosi_cfg)) != 0) return rv;
    }

    if(cfg->dma_miso_cfg != NULL) {
        cfg->dma_miso_cfg->DMA_InitStruct.Parent = &cfg->inst.SPI_InitStruct;
        if((rv = DMA_Init(cfg->dma_miso_cfg)) != 0) return rv;
    }

    cfg->inst.SPI_InitStruct.Instance = cfg->SPI;
    cfg->inst.SPI_InitStruct.Init.Mode = SPI_MODE_MASTER;
    cfg->inst.SPI_InitStruct.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    cfg->inst.SPI_InitStruct.Init.CLKPhase = SPI_PHASE_2EDGE;
    cfg->inst.SPI_InitStruct.Init.CLKPolarity = SPI_POLARITY_HIGH;
    cfg->inst.SPI_InitStruct.Init.DataSize = SPI_DATASIZE_8BIT;
    cfg->inst.SPI_InitStruct.Init.Direction = SPI_DIRECTION_2LINES;
    cfg->inst.SPI_InitStruct.Init.FirstBit = SPI_FIRSTBIT_MSB;
    cfg->inst.SPI_InitStruct.Init.NSS = SPI_NSS_SOFT;
    cfg->inst.SPI_InitStruct.Init.TIMode = SPI_TIMODE_DISABLE;
    cfg->inst.SPI_InitStruct.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    cfg->inst.SPI_InitStruct.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    cfg->inst.SPI_InitStruct.hdmatx = &cfg->dma_mosi_cfg->DMA_InitStruct;
    cfg->inst.SPI_InitStruct.hdmarx = &cfg->dma_miso_cfg->DMA_InitStruct;

    if(HAL_SPI_Init(&cfg->inst.SPI_InitStruct) != HAL_OK) return EINVAL;
    SPI_EnableIRQ(cfg);

    if(cfg->inst.mutex == NULL) cfg->inst.mutex = xSemaphoreCreateMutex();
    if(cfg->inst.semaphore == NULL) cfg->inst.semaphore = xSemaphoreCreateBinary();

    portEXIT_CRITICAL();

    return rv;
}

int SPI_ReInit(spi_cfg_t *cfg) {
    if(HAL_SPI_DeInit(&cfg->inst.SPI_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_SPI_Init(&cfg->inst.SPI_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int SPI_Transmit(spi_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = SPI_TRANSMIT;

    HAL_SPI_Transmit_DMA(&cfg->inst.SPI_InitStruct, pdata, length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = SPI_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int SPI_Receive(spi_cfg_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = SPI_RECEIVE;

    HAL_SPI_Receive_DMA(&cfg->inst.SPI_InitStruct, pdata, length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = SPI_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int SPI_IT_Handler(spi_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_SPI_IRQHandler(&cfg->inst.SPI_InitStruct);

    if(cfg->inst.SPI_InitStruct.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int SPI_DMA_MOSI_Handler(spi_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_mosi_cfg->DMA_InitStruct);

    if(cfg->inst.SPI_InitStruct.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int SPI_DMA_MISO_Handler(spi_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_miso_cfg->DMA_InitStruct);

    if(cfg->inst.SPI_InitStruct.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int SPI_EnableIRQ(spi_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.IRQ);
    return 0;
}

int SPI_DisableIRQ(spi_cfg_t *cfg)  {
    HAL_NVIC_DisableIRQ(cfg->inst.IRQ);
    return 0;
}

int SPI_ClockEnable(spi_cfg_t *cfg) {
    switch((uint32_t)(cfg->SPI)) {

#ifdef SPI1
        case SPI1_BASE:
            __HAL_RCC_SPI1_CLK_ENABLE();
            cfg->inst.IRQ = SPI1_IRQn;
            break;
#endif

#ifdef SPI2
        case SPI2_BASE:
            __HAL_RCC_SPI2_CLK_ENABLE();
            cfg->inst.IRQ = SPI2_IRQn;
            break;
#endif

#ifdef SPI3
        case SPI3_BASE:
            __HAL_RCC_SPI3_CLK_ENABLE();
            cfg->inst.IRQ = SPI3_IRQn;
            break;
#endif

#ifdef SPI4
        case SPI4_BASE:
            __HAL_RCC_SPI4_CLK_ENABLE();
            cfg->inst.IRQ = SPI4_IRQn;
            break;
#endif

#ifdef SPI5
        case SPI5_BASE:
            __HAL_RCC_SPI5_CLK_ENABLE();
            cfg->inst.IRQ = SPI5_IRQn;
            break;
#endif

#ifdef SPI6
        case SPI6_BASE:
            __HAL_RCC_SPI6_CLK_ENABLE();
            cfg->inst.IRQ = SPI6_IRQn;
            break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}
