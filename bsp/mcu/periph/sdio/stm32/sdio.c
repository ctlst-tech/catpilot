#include "sdio.h"

int sdio_init(sdio_t *cfg) {
    int rv = 0;

    if ((rv = sdio_clock_enable(cfg)) != 0) {
        return rv;
    }

    if ((rv = gpio_init(cfg->ck)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->cmd)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->d0)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->d1)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->d2)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->d3)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->cd)) != 0) {
        return rv;
    }

    cfg->init.Init.ClockEdge = SDMMC_CLOCK_EDGE_FALLING;
    cfg->init.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    cfg->init.Init.BusWide = SDMMC_BUS_WIDE_1B;
    cfg->init.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    cfg->init.Init.ClockDiv = 2;

    if (HAL_SD_Init(&cfg->init)) {
        return EINVAL;
    }

    if (HAL_SD_ConfigWideBusOperation(&cfg->init, SDMMC_BUS_WIDE_4B)) {
        return EINVAL;
    }

    sdio_enable_irq(cfg);

    if (cfg->mutex == NULL) {
        cfg->mutex = xSemaphoreCreateMutex();
    }
    if (cfg->semaphore == NULL) {
        cfg->semaphore = xSemaphoreCreateBinary();
    }

    return rv;
}

int sdio_read_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                     uint32_t num) {
    int rv = 0;

    if (num == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->semaphore, 0);

    cfg->state = SDIO_READ;

    rv = sdio_check_status_with_timeout(cfg, cfg->timeout);
    if (rv != SUCCESS) {
        goto free;
    }

    rv = HAL_SD_ReadBlocks_DMA(&cfg->init, pdata, address, num);
    if (rv != HAL_OK) {
        goto free;
    }

    if (!xSemaphoreTake(cfg->semaphore, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    }

free:
    cfg->state = SDIO_FREE;
    xSemaphoreGive(cfg->mutex);

    return rv;
}

int sdio_write_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                      uint32_t num) {
    int rv = 0;

    if (num == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->mutex, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->semaphore, 0);

    cfg->state = SDIO_WRITE;

    rv = sdio_check_status_with_timeout(cfg, cfg->timeout);
    if (rv != SUCCESS) {
        goto free;
    }

    rv = HAL_SD_WriteBlocks_DMA(&cfg->init, pdata, address, num);
    if (rv != HAL_OK) {
        goto free;
    }

    if (!xSemaphoreTake(cfg->semaphore, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    }

free:
    cfg->state = SDIO_FREE;
    xSemaphoreGive(cfg->mutex);

    return rv;
}

int sdio_check_status_with_timeout(sdio_t *cfg, uint32_t timeout) {
    uint32_t status;
    uint32_t start = xTaskGetTickCount();
    uint32_t dt;

    dt = xTaskGetTickCount() - start;

    if (dt > 100) {
        vTaskDelay(0);
    }

    while (dt < timeout) {
        status = HAL_SD_GetCardState(&cfg->init);
        if (status == HAL_SD_CARD_TRANSFER) {
            return 0;
        } else {
            vTaskDelay(0);
        }
    }

    return ETIMEDOUT;
}

int sdio_detect(sdio_t *cfg) {
    if (GPIO_Read(cfg->cd) == GPIO_PIN_RESET) {
        cfg->connected = SDIO_NOT_CONNECTED;
        return ENXIO;
    } else {
        cfg->connected = SDIO_CONNECTED;
        return 0;
    }
}

int sdio_get_info(sdio_t *cfg, HAL_SD_CardInfoTypeDef *info) {
    int rv;
    rv = HAL_SD_GetCardInfo(&cfg->init, info);
    return rv;
}

int sdio_get_status(sdio_t *cfg) {
    int rv;
    rv = HAL_SD_GetCardState(&cfg->init);
    if ((rv == HAL_SD_CARD_ERROR) || (rv == HAL_SD_CARD_DISCONNECTED)) {
        return rv;
    } else {
        return 0;
    }
}

int sdio_it_handler(sdio_t *cfg) {
    HAL_SD_IRQHandler(&cfg->init);
    return 0;
}

int sdio_tx_complete(sdio_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return 0;
}

int sdio_rx_complete(sdio_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    return 0;
}

int sdio_enable_irq(sdio_t *cfg) {
    HAL_NVIC_SetPriority(cfg->IRQ, cfg->irq_priority, 0);
    HAL_NVIC_EnableIRQ(cfg->IRQ);
    return 0;
}

int sdio_disable_irq(sdio_t *cfg) {
    HAL_NVIC_DisableIRQ(cfg->IRQ);
    return 0;
}

int sdio_clock_enable(sdio_t *cfg) {
    switch ((uint32_t)(cfg->sdio)) {
#ifdef SDMMC1
        case SDMMC1_BASE:
            __HAL_RCC_SDMMC1_CLK_ENABLE();
            cfg->IRQ = SDMMC1_IRQn;
            break;
#endif

#ifdef SDMMC2
        case SDMMC2_BASE:
            __HAL_RCC_SDMMC2_CLK_ENABLE();
            cfg->IRQ = SDMMC2_IRQn;
            break;
#endif

        default:
            return EINVAL;
    }

    return 0;
}
