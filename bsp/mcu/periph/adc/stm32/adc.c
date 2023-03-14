#include "adc.h"

static int adc_id_init(adc_t *cfg);
static int adc_clock_init(adc_t *cfg);
void adc_handler(void *area);
void adc_dma_handler(void *area);

int adc_init(adc_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return EINVAL;
    }
    if ((rv = adc_id_init(cfg))) {
        return rv;
    }
    if ((rv = adc_clock_init(cfg))) {
        return rv;
    }
    if ((rv = irq_init(cfg->p.id, cfg->irq_priority, adc_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.id))) {
        return rv;
    }
    if ((rv = dma_init(&cfg->dma, adc_dma_handler, cfg))) {
        return rv;
    }
    if ((rv = HAL_ADC_Init(&cfg->init))) {
        return rv;
    }

    cfg->dma.init.Parent = &cfg->init;
    cfg->init.DMA_Handle = &cfg->dma.init;

    uint32_t length = 0;
    for (int i = 0; i < ADC_MAX_CHANNELS; i++) {
        if (cfg->channel[i].status == ENABLE) {
            rv = HAL_ADC_ConfigChannel(&cfg->init, &cfg->channel[i].cfg);
            length++;
            if (rv) {
                return rv;
            }
        }
    }

    cfg->p.mutex = xSemaphoreCreateMutex();
    if (cfg->p.mutex == NULL) {
        return -1;
    }
    cfg->p.sem = xSemaphoreCreateBinary();
    if (cfg->p.sem == NULL) {
        return -1;
    }
    xSemaphoreTake(cfg->p.sem, 0);

    rv = HAL_ADC_Start_DMA(&cfg->init, (uint32_t *)cfg->p.raw, length);
    xSemaphoreTake(cfg->p.sem, portMAX_DELAY);
    adc_reset_stat(cfg);

    return rv;
}

float adc_get_volt(adc_t *cfg, uint8_t channel) {
    float rv = -1.0;
    xSemaphoreTake(cfg->p.mutex, portMAX_DELAY);
    if (channel < ADC_MAX_CHANNELS) {
        rv = cfg->p.meas[channel];
    }
    xSemaphoreGive(cfg->p.mutex);
    return rv;
}

float adc_get_volt_max(adc_t *cfg, uint8_t channel) {
    float rv = -1.0;
    xSemaphoreTake(cfg->p.mutex, portMAX_DELAY);
    if (channel < ADC_MAX_CHANNELS) {
        rv = cfg->p.max[channel];
    }
    xSemaphoreGive(cfg->p.mutex);
    return rv;
}

float adc_get_volt_min(adc_t *cfg, uint8_t channel) {
    float rv = -1.0;
    xSemaphoreTake(cfg->p.mutex, portMAX_DELAY);
    if (channel < ADC_MAX_CHANNELS) {
        rv = cfg->p.min[channel];
    }
    xSemaphoreGive(cfg->p.mutex);
    return rv;
}

int adc_reset_stat_channel(adc_t *cfg, uint8_t channel) {
    xSemaphoreTake(cfg->p.mutex, portMAX_DELAY);
    if (channel < ADC_MAX_CHANNELS) {
        cfg->p.max[channel] = cfg->p.meas[channel];
        cfg->p.min[channel] = cfg->p.meas[channel];
    }
    xSemaphoreGive(cfg->p.mutex);
    return 0;
}

int adc_reset_stat(adc_t *cfg) {
    for (int i = 0; i < ADC_MAX_CHANNELS; i++) {
        adc_reset_stat_channel(cfg, i);
    }
    return 0;
}

static int adc_id_init(adc_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case ADC1_BASE:
            cfg->p.id = ADC_IRQn;
            break;
        case ADC2_BASE:
            cfg->p.id = ADC_IRQn;
            break;
        case ADC3_BASE:
            cfg->p.id = ADC3_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static int adc_clock_init(adc_t *cfg) {
    switch (cfg->p.id) {
        case ADC_IRQn:
            __HAL_RCC_ADC12_CLK_ENABLE();
            break;
        case ADC3_IRQn:
            __HAL_RCC_ADC3_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}

void adc_handler(void *area) {
    adc_t *cfg = (adc_t *)area;
    HAL_ADC_IRQHandler(&cfg->init);
}

void adc_dma_handler(void *area) {
    adc_t *cfg = (adc_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_DMA_IRQHandler(&cfg->dma.init);

    if (xSemaphoreTakeFromISR(cfg->p.mutex, &xHigherPriorityTaskWoken)) {
        for (int i = 0; i < ADC_MAX_CHANNELS; i++) {
            cfg->p.meas[i] = (float)cfg->p.raw[i] / 0xFFFF * 3.3;
            if (cfg->p.meas[i] > cfg->p.max[i]) {
                cfg->p.max[i] = cfg->p.meas[i];
            }
            if (cfg->p.meas[i] < cfg->p.min[i]) {
                cfg->p.min[i] = cfg->p.meas[i];
            }
        }
        xSemaphoreGiveFromISR(cfg->p.mutex, &xHigherPriorityTaskWoken);
    }
    xSemaphoreGiveFromISR(cfg->p.sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
