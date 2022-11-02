#include "spi.h"

int spi_init(spi_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return -1;
    }

    if ((rv = spi_clock_enable(cfg)) != 0) {
        return rv;
    }

    if ((rv = gpio_init(cfg->mosi)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->miso)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->sck)) != 0) {
        return rv;
    }

    if (cfg->dma_mosi != NULL) {
        cfg->dma_mosi->init.Parent = &cfg->init;
        if ((rv = dma_init(cfg->dma_mosi)) != 0) return rv;
    }

    if (cfg->dma_miso != NULL) {
        cfg->dma_miso->init.Parent = &cfg->init;
        if ((rv = dma_init(cfg->dma_miso)) != 0) return rv;
    }

    cfg->init.hdmatx = &cfg->dma_mosi->init;
    cfg->init.hdmarx = &cfg->dma_miso->init;

    if (HAL_SPI_Init(&cfg->init) != HAL_OK) {
        return EINVAL;
    }

    spi_enable_irq(cfg);

    if (cfg->mutex == NULL) {
        cfg->mutex = xSemaphoreCreateMutex();
    }
    if (cfg->cs_mutex == NULL) {
        cfg->cs_mutex = xSemaphoreCreateMutex();
    }
    if (cfg->semaphore == NULL) {
        cfg->semaphore = xSemaphoreCreateBinary();
    }

    return rv;
}

int spi_reinit(spi_t *cfg) {
    if (HAL_SPI_DeInit(&cfg->init) != HAL_OK) {
        return EINVAL;
    }
    if (HAL_SPI_Init(&cfg->init) != HAL_OK) {
        return EINVAL;
    }
    return 0;
}

int spi_chip_select(spi_t *cfg, gpio_t *cs) {
    xSemaphoreTake(cfg->cs_mutex, portMAX_DELAY);
    gpio_reset(cs);
    return 0;
}

int spi_chip_deselect(spi_t *cfg, gpio_t *cs) {
    gpio_set(cs);
    xSemaphoreGive(cfg->cs_mutex);
    return 0;
}

int spi_transmit(spi_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->semaphore, 0);

    cfg->state = SPI_TRANSMIT;

    HAL_SPI_Transmit_DMA(&cfg->init, pdata, length);

    if (!xSemaphoreTake(cfg->semaphore, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->state = SPI_FREE;
    xSemaphoreGive(cfg->mutex);

    return rv;
}

int spi_receive(spi_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->semaphore, 0);

    cfg->state = SPI_RECEIVE;

    HAL_SPI_Receive_DMA(&cfg->init, pdata, length);

    if (!xSemaphoreTake(cfg->semaphore, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->state = SPI_FREE;
    xSemaphoreGive(cfg->mutex);

    return rv;
}

int spi_transmit_receive(spi_t *cfg, uint8_t *tdata, uint8_t *rdata,
                         uint16_t length) {
    int rv = 0;

    if (length == 0 || tdata == NULL || rdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->semaphore, 0);

    cfg->state = SPI_TRANSMIT_RECEIVE;

    HAL_SPI_TransmitReceive_DMA(&cfg->init, tdata, rdata, length);

    if (!xSemaphoreTake(cfg->semaphore, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->state = SPI_FREE;
    xSemaphoreGive(cfg->mutex);

    return rv;
}

int spi_it_handler(spi_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_SPI_IRQHandler(&cfg->init);

    if (cfg->init.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int spi_dma_mosi_handler(spi_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_mosi->init);

    if (cfg->init.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int spi_dma_miso_handler(spi_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_miso->init);

    if (cfg->init.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int spi_enable_irq(spi_t *cfg) {
    HAL_NVIC_SetPriority(cfg->IRQ, cfg->irq_priority, 0);
    HAL_NVIC_EnableIRQ(cfg->IRQ);
    return 0;
}

int spi_disable_irq(spi_t *cfg) {
    HAL_NVIC_DisableIRQ(cfg->IRQ);
    return 0;
}

int SPI_ClockEnable(spi_t *cfg) {
    switch ((uint32_t)(cfg->SPI)) {
#ifdef SPI1
        case SPI1_BASE:
            __HAL_RCC_SPI1_CLK_ENABLE();
            cfg->IRQ = SPI1_IRQn;
            break;
#endif

#ifdef SPI2
        case SPI2_BASE:
            __HAL_RCC_SPI2_CLK_ENABLE();
            cfg->IRQ = SPI2_IRQn;
            break;
#endif

#ifdef SPI3
        case SPI3_BASE:
            __HAL_RCC_SPI3_CLK_ENABLE();
            cfg->IRQ = SPI3_IRQn;
            break;
#endif

#ifdef SPI4
        case SPI4_BASE:
            __HAL_RCC_SPI4_CLK_ENABLE();
            cfg->IRQ = SPI4_IRQn;
            break;
#endif

#ifdef SPI5
        case SPI5_BASE:
            __HAL_RCC_SPI5_CLK_ENABLE();
            cfg->IRQ = SPI5_IRQn;
            break;
#endif

#ifdef SPI6
        case SPI6_BASE:
            __HAL_RCC_SPI6_CLK_ENABLE();
            cfg->IRQ = SPI6_IRQn;
            break;
#endif

        default:
            return EINVAL;
    }

    return 0;
}
