#include "board.h"
#include "core.h"
#include "hal.h"

adc_t adc1 = {
    .init =
        {
            .Instance = ADC1,
            .Init.ScanConvMode = ADC_SCAN_ENABLE,
            .Init.ContinuousConvMode = ENABLE,
            .Init.NbrOfConversion = 6,
            .Init.ExternalTrigConv = ADC_SOFTWARE_START,
            .Init.Resolution = ADC_RESOLUTION_16B,
            .Init.Overrun = ADC_OVR_DATA_OVERWRITTEN,
            .Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2,
            .Init.EOCSelection = ADC_EOC_SEQ_CONV,
            .Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR,
        },
    .dma =
        {
            .init =
                {
                    .Instance = DMA2_Stream6,
                    .Init.Request = DMA_REQUEST_ADC1,
                    .Init.Direction = DMA_PERIPH_TO_MEMORY,
                    .Init.PeriphInc = DMA_PINC_DISABLE,
                    .Init.MemInc = DMA_MINC_ENABLE,
                    .Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD,
                    .Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD,
                    .Init.Mode = DMA_CIRCULAR,
                    .Init.Priority = DMA_PRIORITY_LOW,
                    .Init.FIFOMode = DMA_FIFOMODE_DISABLE,
                },
            .irq_priority = 5,
        },
    .irq_priority = 5,
    .channel = {
        {
            .status = ENABLE,
            .cfg.Channel = ADC_CHANNEL_14,
            .cfg.Rank = ADC_REGULAR_RANK_1,
            .cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
            .cfg.SingleDiff = ADC_SINGLE_ENDED,
        },
        {
            .status = ENABLE,
            .cfg.Channel = ADC_CHANNEL_15,
            .cfg.Rank = ADC_REGULAR_RANK_2,
            .cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
            .cfg.SingleDiff = ADC_SINGLE_ENDED,
        },
        {
            .status = ENABLE,
            .cfg.Channel = ADC_CHANNEL_18,
            .cfg.Rank = ADC_REGULAR_RANK_3,
            .cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
            .cfg.SingleDiff = ADC_SINGLE_ENDED,
        },
        {
            .status = ENABLE,
            .cfg.Channel = ADC_CHANNEL_13,
            .cfg.Rank = ADC_REGULAR_RANK_4,
            .cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
            .cfg.SingleDiff = ADC_SINGLE_ENDED,
        },
        {
            .status = ENABLE,
            .cfg.Channel = ADC_CHANNEL_4,
            .cfg.Rank = ADC_REGULAR_RANK_5,
            .cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
            .cfg.SingleDiff = ADC_SINGLE_ENDED,
        },
        {
            .status = ENABLE,
            .cfg.Channel = ADC_CHANNEL_8,
            .cfg.Rank = ADC_REGULAR_RANK_6,
            .cfg.SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
            .cfg.SingleDiff = ADC_SINGLE_ENDED,
        },
    }};
