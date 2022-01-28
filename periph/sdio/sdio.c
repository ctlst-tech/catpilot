#include "sdio.h"

int SDIO_Init(sdio_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;

    if((rv = SDIO_ClockEnable(cfg)) != 0) return rv;

    if((rv = GPIO_Init(cfg->ck_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->cmd_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->d0_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->d1_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->d2_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->d3_cfg)) != 0) return rv;
    if((rv = GPIO_Init(cfg->cd_cfg)) != 0) return rv;

    if(cfg->dma_tx_cfg != NULL) {
        cfg->dma_tx_cfg->DMA_InitStruct.Parent = &cfg->inst.SD_InitStruct;
        if((rv = DMA_Init(cfg->dma_tx_cfg)) != 0) return rv;
    }

    if(cfg->dma_rx_cfg != NULL) {
        cfg->dma_rx_cfg->DMA_InitStruct.Parent = &cfg->inst.SD_InitStruct;
        if((rv = DMA_Init(cfg->dma_rx_cfg)) != 0) return rv;
    }

    cfg->inst.SD_InitStruct.Instance = cfg->SDIO;
    cfg->inst.SD_InitStruct.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    cfg->inst.SD_InitStruct.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
    cfg->inst.SD_InitStruct.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    cfg->inst.SD_InitStruct.Init.BusWide = SDMMC_BUS_WIDE_1B;
    cfg->inst.SD_InitStruct.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    cfg->inst.SD_InitStruct.Init.ClockDiv = 2;

    cfg->inst.SD_InitStruct.hdmatx = &cfg->dma_tx_cfg->DMA_InitStruct;
    cfg->inst.SD_InitStruct.hdmarx = &cfg->dma_rx_cfg->DMA_InitStruct;

    if(HAL_SD_Init(&cfg->inst.SD_InitStruct) != HAL_OK) return EINVAL;
    SDIO_EnableIRQ(cfg);

    if(cfg->inst.mutex == NULL) cfg->inst.mutex = xSemaphoreCreateMutex();
    if(cfg->inst.semaphore == NULL) cfg->inst.semaphore = xSemaphoreCreateBinary();

    portEXIT_CRITICAL();

    return rv;
}

int SDIO_ReadBlocks(sdio_cfg_t *cfg, uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv = 0;

    if(num == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = SDIO_READ;

    rv = HAL_SD_ReadBlocks_DMA(&cfg->inst.SD_InitStruct, pdata, address, num);

    if(rv != HAL_OK) {
        goto free;
    }

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    }

    free:
    cfg->inst.state = SDIO_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int SDIO_WriteBlocks(sdio_cfg_t *cfg, uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv = 0;

    if(num == 0) return EINVAL;
    if(pdata ==  NULL) return EINVAL;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = SDIO_WRITE;

    rv = HAL_SD_WriteBlocks_DMA(&cfg->inst.SD_InitStruct, pdata, address, num);

    if(rv != HAL_OK) {
        goto free;
    }

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    }

    free:
    cfg->inst.state = SDIO_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int SDIO_Erase(sdio_cfg_t *cfg, uint32_t start_address, uint32_t end_address) {
    int rv = 0;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = SDIO_ERASE;

    rv = HAL_SD_Erase(&cfg->inst.SD_InitStruct, start_address, end_address);

    if(rv != HAL_OK) {
        goto free;
    }

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    }

    free:
    cfg->inst.state = SDIO_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int SDIO_Check(sdio_cfg_t *cfg) {
    int rv = 0;

    if(xSemaphoreTake(cfg->inst.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->inst.semaphore, 0);

    cfg->inst.state = SDIO_CHECK;

    rv = SDMMC_CmdGoIdleState(cfg->SDIO);

    if(rv != HAL_OK) {
        goto free;
    }

    if(xSemaphoreTake(cfg->inst.semaphore, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
    }

    free:
    cfg->inst.state = SDIO_FREE;
    xSemaphoreGive(cfg->inst.mutex);

    return rv;
}

int SDIO_GetCardInfo(sdio_cfg_t *cfg) {
    int rv = 0;
    rv = HAL_SD_GetCardInfo(&cfg->inst.SD_InitStruct, &cfg->inst.SD_CardInfo);
    return rv;
}

int SDIO_Detect(sdio_cfg_t *cfg) {
    if(GPIO_Read(cfg->cd_cfg) == GPIO_PIN_RESET) {
        cfg->inst.connected = SDIO_NOT_CONNECTED;
        return ENXIO;
    } else {
        cfg->inst.connected = SDIO_CONNECTED;
        return 0;
    }
}

int SDIO_GetStatus(sdio_cfg_t *cfg) {
    int rv;
    rv = cfg->inst.state;
    return rv;
}

int SDIO_IT_Handler(sdio_cfg_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_SD_IRQHandler(&cfg->inst.SD_InitStruct);

    if(cfg->inst.SD_InitStruct.State == HAL_SD_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->inst.semaphore, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    return 0;
}

int SDIO_DMA_TX_Handler(sdio_cfg_t *cfg) {
    HAL_DMA_IRQHandler(&cfg->dma_tx_cfg->DMA_InitStruct);
    return 0;
}

int SDIO_DMA_RX_Handler(sdio_cfg_t *cfg) {
    HAL_DMA_IRQHandler(&cfg->dma_rx_cfg->DMA_InitStruct);
    return 0;
}

int SDIO_EnableIRQ(sdio_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.IRQ);
    return 0;
}

int SDIO_DisableIRQ(sdio_cfg_t *cfg)  {
    HAL_NVIC_DisableIRQ(cfg->inst.IRQ);
    return 0;
}

int SDIO_ClockEnable(sdio_cfg_t *cfg) {
    switch((uint32_t)(cfg->SDIO)) {

#ifdef SDMMC1
    case SDMMC1_BASE:
        __HAL_RCC_SDMMC1_CLK_ENABLE();
        cfg->inst.IRQ = SDMMC1_IRQn;
        break;
#endif

#ifdef SDMMC2
    case SDMMC2_BASE:
        __HAL_RCC_SDMMC2_CLK_ENABLE();
        cfg->inst.IRQ = SDMMC2_IRQn;
        break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}
