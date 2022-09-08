#include "init.h"
#include "cfg.h"

adc_cfg_t adc12;
dma_cfg_t adc12_dma;
gpio_cfg_t adc12_inp14 = GPIO_ADC12_INP14;
gpio_cfg_t adc12_inp15 = GPIO_ADC12_INP15;
gpio_cfg_t adc12_inp18 = GPIO_ADC12_INP18;
gpio_cfg_t adc12_inp13 = GPIO_ADC12_INP13;
gpio_cfg_t adc12_inp4 = GPIO_ADC12_INP4;
gpio_cfg_t adc12_inp8 = GPIO_ADC12_INP8;

int ADC12_Init() {
    int rv = 0;

    adc12.ADC = ADC1;
    adc12.ADC_InitStruct.Init.ScanConvMode = ADC_SCAN_ENABLE;
    adc12.ADC_InitStruct.Init.ContinuousConvMode = ENABLE;
    adc12.ADC_InitStruct.Init.NbrOfConversion = 6;
    adc12.ADC_InitStruct.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc12.ADC_InitStruct.Init.Resolution = ADC_RESOLUTION_16B;
    adc12.ADC_InitStruct.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    adc12.ADC_InitStruct.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    adc12.ADC_InitStruct.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    adc12.ADC_InitStruct.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
    adc12.ch_num = 6;
    adc12.dma_cfg = &adc12_dma;
    adc12.priority = 5;

    adc12_dma.DMA_InitStruct.Instance = DMA2_Stream2;
    adc12_dma.DMA_InitStruct.Init.Request = DMA_REQUEST_ADC1;
    adc12_dma.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    adc12_dma.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    adc12_dma.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    adc12_dma.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    adc12_dma.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    adc12_dma.DMA_InitStruct.Init.Mode = DMA_CIRCULAR;
    adc12_dma.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    adc12_dma.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    adc12.ch[0].status = ENABLE;
    adc12.ch[0].cfg.Channel = ADC_CHANNEL_14;
    adc12.ch[0].cfg.Rank = ADC_REGULAR_RANK_1;
    adc12.ch[0].cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    adc12.ch[0].cfg.SingleDiff = ADC_SINGLE_ENDED;

    adc12.ch[1].status = ENABLE;
    adc12.ch[1].cfg.Channel = ADC_CHANNEL_15;
    adc12.ch[1].cfg.Rank = ADC_REGULAR_RANK_2;
    adc12.ch[1].cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    adc12.ch[1].cfg.SingleDiff = ADC_SINGLE_ENDED;

    adc12.ch[2].status = ENABLE;
    adc12.ch[2].cfg.Channel = ADC_CHANNEL_18;
    adc12.ch[2].cfg.Rank = ADC_REGULAR_RANK_3;
    adc12.ch[2].cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    adc12.ch[2].cfg.SingleDiff = ADC_SINGLE_ENDED;

    adc12.ch[3].status = ENABLE;
    adc12.ch[3].cfg.Channel = ADC_CHANNEL_13;
    adc12.ch[3].cfg.Rank = ADC_REGULAR_RANK_4;
    adc12.ch[3].cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    adc12.ch[3].cfg.SingleDiff = ADC_SINGLE_ENDED;

    adc12.ch[4].status = ENABLE;
    adc12.ch[4].cfg.Channel = ADC_CHANNEL_4;
    adc12.ch[4].cfg.Rank = ADC_REGULAR_RANK_5;
    adc12.ch[4].cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    adc12.ch[4].cfg.SingleDiff = ADC_SINGLE_ENDED;

    adc12.ch[5].status = ENABLE;
    adc12.ch[5].cfg.Channel = ADC_CHANNEL_8;
    adc12.ch[5].cfg.Rank = ADC_REGULAR_RANK_6;
    adc12.ch[5].cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    adc12.ch[5].cfg.SingleDiff = ADC_SINGLE_ENDED;

    GPIO_Init(&adc12_inp14);
    GPIO_Init(&adc12_inp15);
    GPIO_Init(&adc12_inp18);
    GPIO_Init(&adc12_inp13);
    GPIO_Init(&adc12_inp4);
    GPIO_Init(&adc12_inp8);

    rv = ADC_Init(&adc12);

    return rv;
}
