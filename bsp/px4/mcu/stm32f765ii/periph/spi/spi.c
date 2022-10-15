#include "spi.h"

int SPI_Init(spi_cfg_t *cfg) {

    portENTER_CRITICAL();

    if(cfg == NULL) return -1;

    int rv = 0;
    if((rv = SPI_ClockEnable(cfg)) != 0) return rv;

    if((rv = GPIO_Init(cfg->mosi_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->miso_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->sck_cfg)) != 0) return rv;

    if(cfg->dma_mosi_cfg != NULL) {
        cfg->dma_mosi_cfg->DMA_InitStruct.Parent = &cfg->SPI_InitStruct;
        if((rv = DMA_Init(cfg->dma_mosi_cfg)) != 0) return rv;
    }

    if(cfg->dma_miso_cfg != NULL) {
        cfg->dma_miso_cfg->DMA_InitStruct.Parent = &cfg->SPI_InitStruct;
        if((rv = DMA_Init(cfg->dma_miso_cfg)) != 0) return rv;
    }

    cfg->SPI_InitStruct.Instance = cfg->SPI;

    cfg->SPI_InitStruct.hdmatx = &cfg->dma_mosi_cfg->DMA_InitStruct;
    cfg->SPI_InitStruct.hdmarx = &cfg->dma_miso_cfg->DMA_InitStruct;

    if(HAL_SPI_Init(&cfg->SPI_InitStruct) != HAL_OK) return EINVAL;
    SPI_EnableIRQ(cfg);

    if(cfg->inst.mutex == NULL) cfg->inst.mutex = xSemaphoreCreateMutex();
    if(cfg->inst.cs_mutex == NULL) cfg->inst.cs_mutex = xSemaphoreCreateMutex();
    if(cfg->inst.semaphore == NULL) cfg->inst.semaphore = xSemaphoreCreateBinary();

    portEXIT_CRITICAL();

    return rv;
}

int SPI_ReInit(spi_cfg_t *cfg) {
    if(HAL_SPI_DeInit(&cfg->SPI_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_SPI_Init(&cfg->SPI_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int SPI_ChipSelect(spi_cfg_t *cfg, gpio_cfg_t *cs) {
    xSemaphoreTake(cfg->inst.cs_mutex, portMAX_DELAY);
    GPIO_Reset(cs);
    return 0;
}

int SPI_ChipDeselect(spi_cfg_t *cfg, gpio_cfg_t *cs) {
    GPIO_Set(cs);
    xSemaphoreGive(cfg->inst.cs_mutex);
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

    HAL_SPI_Transmit_DMA(&cfg->SPI_InitStruct, pdata, length);

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

    HAL_SPI_Receive_DMA(&cfg->SPI_InitStruct, pdata, length);

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->inst.state = SPI_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int SPI_TransmitReceive(spi_cfg_t *cfg, 
                        uint8_t *tdata, 
                        uint8_t *rdata, 
                        uint16_t length) {
    int rv = 0;

    if(length == 0) return EINVAL;
    if(tdata ==  NULL) return EINVAL;
    if(rdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = SPI_TRANSMIT_RECEIVE;

    HAL_SPI_TransmitReceive_DMA(&cfg->SPI_InitStruct, tdata, rdata, length);

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

    HAL_SPI_IRQHandler(&cfg->SPI_InitStruct);

    if(cfg->SPI_InitStruct.State == HAL_SPI_STATE_READY) {
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

    if(cfg->SPI_InitStruct.State == HAL_SPI_STATE_READY) {
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

    if(cfg->SPI_InitStruct.State == HAL_SPI_STATE_READY) {
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
