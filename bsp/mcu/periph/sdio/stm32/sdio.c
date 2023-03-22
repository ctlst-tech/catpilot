#include "sdio.h"

static int sdio_id_init(sdio_t *cfg);
static int sdio_clock_init(sdio_t *cfg);
void sdio_handler(void *area);

int sdio_init(sdio_t *cfg) {
    int rv = 0;

    if ((rv = sdio_id_init(cfg)) != 0) {
        return rv;
    }
    if ((rv = sdio_clock_init(cfg)) != 0) {
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
    if ((rv = HAL_SD_Init(&cfg->init))) {
        return rv;
    }
    if ((rv = HAL_SD_ConfigWideBusOperation(&cfg->init, SDMMC_BUS_WIDE_4B))) {
        return rv;
    }
    if ((rv = irq_init(cfg->p.id, cfg->irq_priority, sdio_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.id))) {
        return rv;
    }
    cfg->p.mutex = xSemaphoreCreateMutex();
    if (cfg->p.mutex == NULL) {
        return -1;
    }
    cfg->p.sem = xSemaphoreCreateBinary();
    if (cfg->p.sem == NULL) {
        return -1;
    }

    return rv;
}

int sdio_read_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                     uint32_t num) {
    int rv = 0;

    if (num == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = SDIO_READ;

    rv = sdio_check_status_with_timeout(cfg, cfg->timeout);
    if (rv != SUCCESS) {
        goto free;
    }

    rv = HAL_SD_ReadBlocks_DMA(&cfg->init, pdata, address, num);
    if (rv != HAL_OK) {
        goto free;
    }

    if (!xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    }

free:
    cfg->p.state = SDIO_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

int sdio_write_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                      uint32_t num) {
    int rv = 0;

    if (num == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
        goto free;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = SDIO_WRITE;

    rv = sdio_check_status_with_timeout(cfg, cfg->timeout);
    if (rv != SUCCESS) {
        goto free;
    }

    rv = HAL_SD_WriteBlocks_DMA(&cfg->init, pdata, address, num);
    if (rv != HAL_OK) {
        goto free;
    }

    if (!xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    }

free:
    cfg->p.state = SDIO_FREE;
    xSemaphoreGive(cfg->p.mutex);

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
    if (gpio_read(cfg->cd) == GPIO_PIN_RESET) {
        cfg->p.connected = SDIO_NOT_CONNECTED;
        return ENXIO;
    } else {
        cfg->p.connected = SDIO_CONNECTED;
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

void sdio_handler(void *area) {
    sdio_t *cfg = (sdio_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_SD_IRQHandler(&cfg->init);
    if (cfg->init.State == HAL_SD_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->p.sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd) {
    // TODO: add handler
}

static int sdio_id_init(sdio_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case SDMMC1_BASE:
            cfg->p.id = SDMMC1_IRQn;
            break;
        case SDMMC2_BASE:
            cfg->p.id = SDMMC2_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static int sdio_clock_init(sdio_t *cfg) {
    switch (cfg->p.id) {
        case SDMMC1_IRQn:
            __HAL_RCC_SDMMC1_CLK_ENABLE();
            break;
        case SDMMC2_IRQn:
            __HAL_RCC_SDMMC2_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}
