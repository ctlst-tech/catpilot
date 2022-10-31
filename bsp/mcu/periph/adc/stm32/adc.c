#include "adc.h"

int ADC_Init(adc_cfg_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return -1;
    }

    cfg->ADC_InitStruct.Instance = cfg->ADC;

    if ((rv = ADC_ClockEnable(cfg)) != 0) {
        return rv;
    }
    if ((rv = ADC_EnableIRQ(cfg)) != 0) {
        return rv
    }

    if ((rv = HAL_ADC_Init(&cfg->ADC_InitStruct)) != 0) {
        return rv;
    }

    if (cfg->dma_cfg != NULL) {
        cfg->dma_cfg->DMA_InitStruct.Parent = &cfg->ADC_InitStruct;
        cfg->ADC_InitStruct.DMA_Handle = &cfg->dma_cfg->DMA_InitStruct;
        if ((rv = DMA_Init(cfg->dma_cfg)) != 0) {
            return rv;
        }
    }

    for (int i = 0; i < ADC_MAX_CHANNELS; i++) {
        if (cfg->ch[i].status == ENABLE) {
            rv = HAL_ADC_ConfigChannel(&cfg->ADC_InitStruct, &cfg->ch[i].cfg);
            if (rv) {
                return rv;
            }
        }
    }

    rv = HAL_ADC_Start_DMA(&cfg->ADC_InitStruct, (uint32_t *)cfg->buf,
                           cfg->ch_num);

    return rv;
}

uint16_t ADC_GetRAW(adc_cfg_t *cfg, int ch) {
    if (ch > ADC_MAX_CHANNELS) {
        return (__UINT16_MAX__);
    }
    return (cfg->buf[ch]);
}

double ADC_GetVolt(adc_cfg_t *cfg, int ch) {
    if (ch > ADC_MAX_CHANNELS) {
        return (__UINT16_MAX__);
    }
    return ((double)cfg->buf[ch] / 0xFFFF * 3.3);
}

int ADC_EnableIRQ(adc_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->IRQ);
    return 0;
}

int ADC_DisableIRQ(adc_cfg_t *cfg) {
    HAL_NVIC_DisableIRQ(cfg->IRQ);
    return 0;
}

int ADC_Handler(adc_cfg_t *cfg) {
    HAL_ADC_IRQHandler(&cfg->ADC_InitStruct);
    return 0;
}

int ADC_DMA_Handler(adc_cfg_t *cfg) {
    HAL_DMA_IRQHandler(&cfg->dma_cfg->DMA_InitStruct);
    return 0;
}

int ADC_ClockEnable(adc_cfg_t *cfg) {
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
