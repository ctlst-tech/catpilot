
#include "spi.h"
static int spi_id_init(spi_t *cfg);
static int spi_clock_init(spi_t *cfg);
void spi_dma_tx_handler(void *area);
void spi_dma_rx_handler(void *area);
void spi_it_handler(void *area);

int spi_init(spi_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return -1;
    }
    if ((rv = spi_id_init(cfg))) {
        return rv;
    }
    if ((rv = spi_clock_init(cfg))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->mosi))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->miso))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->sck))) {
        return rv;
    }
    if (cfg->dma_tx.init.Instance != NULL &&
        cfg->dma_rx.init.Instance != NULL) {
        cfg->dma_tx.init.Parent = &cfg->init;
        cfg->dma_rx.init.Parent = &cfg->init;
        if ((rv = dma_init(&cfg->dma_tx, spi_dma_tx_handler, cfg)) != 0) {
            return rv;
        }
        if ((rv = dma_init(&cfg->dma_rx, spi_dma_rx_handler, cfg)) != 0) {
            return rv;
        }
        cfg->init.hdmatx = &cfg->dma_tx.init;
        cfg->init.hdmarx = &cfg->dma_rx.init;
    }
    if ((rv = HAL_SPI_Init(&cfg->init))) {
        return rv;
    }
    if ((rv = irq_init(cfg->p.id, cfg->irq_priority, spi_it_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.id))) {
        return rv;
    }

    if (cfg->p.mutex == NULL) {
        cfg->p.mutex = xSemaphoreCreateMutex();
    }
    if (cfg->p.cs_mutex == NULL) {
        cfg->p.cs_mutex = xSemaphoreCreateMutex();
    }
    if (cfg->p.sem == NULL) {
        cfg->p.sem = xSemaphoreCreateBinary();
    }
    return rv;
}

int spi_chip_select(spi_t *cfg, gpio_t *cs) {
    xSemaphoreTake(cfg->p.cs_mutex, portMAX_DELAY);
    gpio_reset(cs);
    return 0;
}

int spi_chip_deselect(spi_t *cfg, gpio_t *cs) {
    gpio_set(cs);
    xSemaphoreGive(cfg->p.cs_mutex);
    return 0;
}

int spi_transmit(spi_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = SPI_TRANSMIT;

    HAL_SPI_Transmit_DMA(&cfg->init, pdata, length);

    if (!xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.state = SPI_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

int spi_receive(spi_t *cfg, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = SPI_RECEIVE;

    HAL_SPI_Receive_DMA(&cfg->init, pdata, length);

    if (!xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.state = SPI_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

int spi_transmit_receive(spi_t *cfg, uint8_t *tdata, uint8_t *rdata,
                         uint16_t length) {
    int rv = 0;

    if (length == 0 || tdata == NULL || rdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = SPI_TRANSMIT_RECEIVE;

    HAL_SPI_TransmitReceive_DMA(&cfg->init, tdata, rdata, length);

    if (!xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.state = SPI_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

void spi_it_handler(void *area) {
    spi_t *cfg = (spi_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_SPI_IRQHandler(&cfg->init);

    if (cfg->init.State == HAL_SPI_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->p.sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void spi_dma_tx_handler(void *area) {
    spi_t *cfg = (spi_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_tx.init);
}

void spi_dma_rx_handler(void *area) {
    spi_t *cfg = (spi_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_rx.init);
}

static int spi_id_init(spi_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case SPI1_BASE:
            cfg->p.id = SPI1_IRQn;
            break;
        case SPI2_BASE:
            cfg->p.id = SPI2_IRQn;
            break;
        case SPI3_BASE:
            cfg->p.id = SPI3_IRQn;
            break;
        case SPI4_BASE:
            cfg->p.id = SPI4_IRQn;
            break;
        case SPI5_BASE:
            cfg->p.id = SPI5_IRQn;
            break;
        case SPI6_BASE:
            cfg->p.id = SPI6_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static int spi_clock_init(spi_t *cfg) {
    switch ((uint32_t)(cfg->p.id)) {
        case SPI1_IRQn:
            __HAL_RCC_SPI1_CLK_ENABLE();
            break;
        case SPI2_IRQn:
            __HAL_RCC_SPI2_CLK_ENABLE();
            break;
        case SPI3_IRQn:
            __HAL_RCC_SPI3_CLK_ENABLE();
            break;
        case SPI4_IRQn:
            __HAL_RCC_SPI4_CLK_ENABLE();
            break;
        case SPI5_IRQn:
            __HAL_RCC_SPI5_CLK_ENABLE();
            break;
        case SPI6_IRQn:
            __HAL_RCC_SPI6_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}
