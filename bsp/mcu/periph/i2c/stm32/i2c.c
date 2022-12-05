#include "i2c.h"

int i2c_init(i2c_t *cfg) {
    int rv = 0;

    if ((rv = i2c_clock_enable(cfg)) != 0) {
        return rv;
    }

    if ((rv = gpio_init(cfg->sda)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(cfg->scl)) != 0) {
        return rv;
    }

    gpio_set(cfg->scl);
    gpio_set(cfg->sda);

    if (cfg->dma_tx != NULL) {
        cfg->dma_tx->init.Parent = &cfg->init;
        if ((rv = dma_init(cfg->dma_tx)) != 0) {
            return rv;
        }
    }

    if (cfg->dma_rx != NULL) {
        cfg->dma_rx->init.Parent = &cfg->init;
        if ((rv = dma_init(cfg->dma_rx)) != 0) {
            return rv;
        }
    }

    cfg->init.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    cfg->init.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    cfg->init.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    cfg->init.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    // From CubeMX configuration tool for FAST MODE
    cfg->init.Init.Timing = 0x6000030D;

    cfg->init.hdmatx = &cfg->dma_tx->init;
    cfg->init.hdmarx = &cfg->dma_rx->init;

    if (HAL_I2C_Init(&cfg->init) != HAL_OK) {
        return EINVAL;
    }
    i2c_enable_irq(cfg);

    if (cfg->mutex == NULL) {
        cfg->mutex = xSemaphoreCreateMutex();
    }
    if (cfg->semaphore == NULL) {
        cfg->semaphore = xSemaphoreCreateBinary();
    }

    return rv;
}

int i2c_reinit(i2c_t *cfg) {
    if (HAL_I2C_DeInit(&cfg->init) != HAL_OK) {
        return EINVAL;
    }
    if (HAL_I2C_Init(&cfg->init) != HAL_OK) {
        return EINVAL;
    }
    return 0;
}

int i2c_transmit(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->semaphore, 0);

    cfg->state = I2C_TRANSMIT;

    HAL_I2C_Master_Transmit_DMA(&cfg->init, address, pdata, length);

    if (!xSemaphoreTake(cfg->semaphore, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->state = I2C_FREE;
    xSemaphoreGive(cfg->mutex);

    return rv;
}

int i2c_receive(i2c_t *cfg, uint8_t address, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (xSemaphoreTake(cfg->mutex, pdMS_TO_TICKS(cfg->timeout)) == pdFALSE) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->semaphore, 0);

    cfg->state = I2C_RECEIVE;

    HAL_I2C_Master_Receive_DMA(&cfg->init, address, pdata, length);

    if (!xSemaphoreTake(cfg->semaphore, pdMS_TO_TICKS(cfg->timeout))) {
        rv = ETIMEDOUT;
    } else {
        rv = 0;
    }

    cfg->state = I2C_FREE;
    xSemaphoreGive(cfg->mutex);

    return rv;
}

int i2c_ev_handler(i2c_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_I2C_EV_IRQHandler(&cfg->init);

    if (cfg->init.State == HAL_I2C_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int i2c_er_handler(i2c_t *cfg) {
    HAL_I2C_ER_IRQHandler(&cfg->init);
    return 0;
}

int i2c_dma_tx_handler(i2c_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_tx->init);

    if (cfg->dma_tx->init.State == HAL_DMA_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int i2c_dma_rx_handler(i2c_t *cfg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma_rx->init);

    if (cfg->dma_rx->init.State == HAL_DMA_STATE_READY) {
        xSemaphoreGiveFromISR(cfg->semaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

    return 0;
}

int i2c_enable_irq(i2c_t *cfg) {
    HAL_NVIC_SetPriority(cfg->ev_irq, cfg->irq_priority, 0);
    HAL_NVIC_SetPriority(cfg->er_irq, cfg->irq_priority, 0);
    HAL_NVIC_EnableIRQ(cfg->ev_irq);
    HAL_NVIC_EnableIRQ(cfg->er_irq);
    return 0;
}

int i2c_disable_irq(i2c_t *cfg) {
    HAL_NVIC_DisableIRQ(cfg->ev_irq);
    HAL_NVIC_DisableIRQ(cfg->er_irq);
    return 0;
}

int i2c_clock_enable(i2c_t *cfg) {
    switch ((uint32_t)(cfg->i2c)) {
#ifdef I2C1
        case I2C1_BASE:
            __HAL_RCC_I2C1_CLK_ENABLE();
            cfg->ev_irq = I2C1_EV_IRQn;
            cfg->er_irq = I2C1_ER_IRQn;
            break;
#endif

#ifdef I2C2
        case I2C2_BASE:
            __HAL_RCC_I2C2_CLK_ENABLE();
            cfg->ev_irq = I2C2_EV_IRQn;
            cfg->er_irq = I2C2_ER_IRQn;
            break;
#endif

#ifdef I2C3
        case I2C3_BASE:
            __HAL_RCC_I2C3_CLK_ENABLE();
            cfg->ev_irq = I2C3_EV_IRQn;
            cfg->er_irq = I2C3_ER_IRQn;
            break;
#endif

#ifdef I2C4
        case I2C4_BASE:
            __HAL_RCC_I2C4_CLK_ENABLE();
            cfg->ev_irq = I2C4_EV_IRQn;
            cfg->er_irq = I2C4_ER_IRQn;
            break;
#endif

        default:
            return EINVAL;
    }

    return 0;
}
