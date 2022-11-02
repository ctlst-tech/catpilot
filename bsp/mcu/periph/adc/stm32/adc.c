#include "adc.h"

int adc_init(adc_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return -1;
    }

    if ((rv = adc_clock_enable(cfg)) != 0) {
        return rv;
    }
    if ((rv = adc_enable_irq(cfg)) != 0) {
        return rv
    }

    if ((rv = HAL_ADC_Init(&cfg->init)) != 0) {
        return rv;
    }

    if (cfg->dma != NULL) {
        cfg->dma->init.Parent = &cfg->init;
        cfg->init.DMA_Handle = &cfg->dma->init;
        if ((rv = dma_init(cfg->dma)) != 0) {
            return rv;
        }
    }

    for (int i = 0; i < ADC_MAX_CHANNELS; i++) {
        if (cfg->channel[i].status == ENABLE) {
            rv = HAL_ADC_ConfigChannel(&cfg->init, &cfg->ch[i].cfg);
            if (rv) {
                return rv;
            }
        }
    }

    rv = HAL_ADC_Start_DMA(&cfg->init, (uint32_t *)cfg->buf, cfg->ch_num);

    return rv;
}

uint16_t adc_get_raw(adc_t *cfg, int ch) {
    if (ch > ADC_MAX_CHANNELS) {
        return (__UINT16_MAX__);
    }
    return (cfg->buf[ch]);
}

double adc_get_volt(adc_t *cfg, int ch) {
    if (ch > ADC_MAX_CHANNELS) {
        return (__UINT16_MAX__);
    }
    return ((double)cfg->buf[ch] / 0xFFFF * 3.3);
}

int adc_enable_irq(adc_t *cfg) {
    HAL_NVIC_SetPriority(cfg->IRQ, cfg->irq_priority, 0);
    HAL_NVIC_EnableIRQ(cfg->IRQ);
    return 0;
}

int adc_disable_irq(adc_t *cfg) {
    HAL_NVIC_DisableIRQ(cfg->IRQ);
    return 0;
}

int adc_handler(adc_t *cfg) {
    HAL_ADC_IRQHandler(&cfg->init);
    return 0;
}

int adc_dma_handler(adc_t *cfg) {
    HAL_DMA_IRQHandler(&cfg->dma->init);
    return 0;
}

int adc_clock_enable(adc_t *cfg) {
    switch ((uint32_t)(cfg->ADC)) {
#ifdef ADC1
        case ADC1_BASE:
            __HAL_RCC_ADC12_CLK_ENABLE();
            cfg->IRQ = ADC_IRQn;
            break;
#endif

#ifdef ADC2
        case ADC2_BASE:
            __HAL_RCC_ADC12_CLK_ENABLE();
            cfg->IRQ = ADC_IRQn;
            break;
#endif

#ifdef ADC3
        case ADC3_BASE:
            __HAL_RCC_ADC3_CLK_ENABLE();
            cfg->IRQ = ADC3_IRQn;
            break;
#endif

        default:
            return EINVAL;
    }

    return 0;
}
