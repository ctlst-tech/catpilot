#pragma once
#include "stm32_base.h"
#include "dma.h"

#define ADC_MAX_CHANNELS 16

typedef struct {
    int status;
    ADC_ChannelConfTypeDef cfg;
} adc_channel_t;

typedef struct {
    ADC_TypeDef *ADC;
    ADC_HandleTypeDef ADC_InitStruct;
    dma_cfg_t *dma_cfg;
    adc_channel_t ch[ADC_MAX_CHANNELS];
    int priority;
    int ch_num;
    uint16_t buf[ADC_MAX_CHANNELS];
    IRQn_Type IRQ;
} adc_cfg_t;

int ADC_Init(adc_cfg_t *cfg);
int ADC_ClockEnable(adc_cfg_t *cfg);
int ADC_EnableIRQ(adc_cfg_t *cfg);
int ADC_DisableIRQ(adc_cfg_t *cfg);
int ADC_Handler(adc_cfg_t *cfg);
int ADC_DMA_Handler(adc_cfg_t *cfg);

uint16_t ADC_GetRAW(adc_cfg_t *cfg, int ch);
double ADC_GetVolt(adc_cfg_t *cfg, int ch);
