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
    
    rv = HAL_ADC_Start_DMA(&cfg->init, (uint32_t *)cfg->p.raw, length);

    return rv;
}

uint32_t adc_get_raw(adc_t *cfg, uint8_t channel) {
    if (channel > ADC_MAX_CHANNELS) {
        return (__UINT32_MAX__);
    }
    return (cfg->p.raw[channel]);
}

double adc_get_volt(adc_t *cfg, uint8_t channel) {
    if (channel > ADC_MAX_CHANNELS) {
        return (__UINT32_MAX__);
    }
    return ((double)cfg->p.raw[channel] / 0xFFFF * 3.3);
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
    HAL_DMA_IRQHandler(&cfg->dma.init);
}
