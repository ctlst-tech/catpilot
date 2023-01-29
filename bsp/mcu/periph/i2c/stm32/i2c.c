#include "i2c.h"

static int i2c_id_init(i2c_t *cfg);
static int i2c_clock_init(i2c_t *cfg);
void i2c_dma_tx_handler(void *area);
void i2c_dma_rx_handler(void *area);
void i2c_ev_handler(void *area);
void i2c_er_handler(void *area);

int i2c_init(i2c_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return -1;
    }
    if ((rv = i2c_id_init(cfg))) {
        return rv;
    }
    if ((rv = i2c_clock_init(cfg))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->sda))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->scl))) {
        return rv;
    }
    if (cfg->dma_tx.init.Instance != NULL &&
        cfg->dma_rx.init.Instance != NULL) {
        cfg->dma_tx.init.Parent = &cfg->init;
        cfg->dma_rx.init.Parent = &cfg->init;
        if ((rv = dma_init(&cfg->dma_tx, i2c_dma_tx_handler, cfg)) != 0) {
            return rv;
        }
        if ((rv = dma_init(&cfg->dma_rx, i2c_dma_rx_handler, cfg)) != 0) {
            return rv;
        }
        cfg->init.hdmatx = &cfg->dma_tx.init;
        cfg->init.hdmarx = &cfg->dma_rx.init;
    }
    if ((rv = HAL_I2C_Init(&cfg->init))) {
        return rv;
    }
    if ((rv = HAL_I2CEx_ConfigAnalogFilter(&cfg->init,
                                           I2C_ANALOGFILTER_ENABLE))) {
        return rv;
    }
    if ((rv = HAL_I2CEx_ConfigDigitalFilter(&cfg->init, 0))) {
        return rv;
    }
    if ((rv = irq_init(cfg->p.ev_id, cfg->irq_priority, i2c_ev_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_init(cfg->p.er_id, cfg->irq_priority, i2c_er_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.ev_id))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.er_id))) {
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

int i2c_transmit(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0) return EINVAL;
    if (pdata == NULL) return EINVAL;

    if (xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = I2C_TRANSMIT;

    HAL_I2C_Master_Transmit_DMA(&cfg->init, address, pdata, length);

    if (rv == HAL_OK && !xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.state = I2C_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

int i2c_receive(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0) return EINVAL;
    if (pdata == NULL) return EINVAL;

    if (xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = I2C_RECEIVE;

    HAL_I2C_Master_Receive_DMA(&cfg->init, address, pdata, length);

    if (rv == HAL_OK && !xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->p.state = I2C_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

void i2c_ev_handler(void *area) {
    i2c_t *cfg = (i2c_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    HAL_I2C_EV_IRQHandler(&cfg->init);
    if (cfg->init.State == HAL_I2C_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->p.sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void i2c_er_handler(void *area) {
    i2c_t *cfg = (i2c_t *)area;
    HAL_I2C_ER_IRQHandler(&cfg->init);
}

void i2c_dma_tx_handler(void *area) {
    i2c_t *cfg = (i2c_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_tx.init);
}

void i2c_dma_rx_handler(void *area) {
    i2c_t *cfg = (i2c_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_rx.init);

}

static int i2c_id_init(i2c_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case I2C1_BASE:
            cfg->p.ev_id = I2C1_EV_IRQn;
            cfg->p.er_id = I2C1_ER_IRQn;
            break;
        case I2C2_BASE:
            cfg->p.ev_id = I2C2_EV_IRQn;
            cfg->p.er_id = I2C2_ER_IRQn;
            break;
        case I2C3_BASE:
            cfg->p.ev_id = I2C3_EV_IRQn;
            cfg->p.er_id = I2C3_ER_IRQn;
            break;
        case I2C4_BASE:
            cfg->p.ev_id = I2C4_EV_IRQn;
            cfg->p.er_id = I2C4_ER_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static int i2c_clock_init(i2c_t *cfg) {
    switch ((uint32_t)(cfg->p.ev_id)) {
        case I2C1_EV_IRQn:
            __HAL_RCC_I2C1_CLK_ENABLE();
            break;
        case I2C2_EV_IRQn:
            __HAL_RCC_I2C2_CLK_ENABLE();
            break;
        case I2C3_EV_IRQn:
            __HAL_RCC_I2C3_CLK_ENABLE();
            break;
        case I2C4_EV_IRQn:
            __HAL_RCC_I2C4_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}
