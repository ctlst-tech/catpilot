#ifndef ADC_H
#define ADC_H

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "hal.h"
#include "irq.h"
#include "os.h"

#define ADC_MAX_CHANNELS 16

typedef struct {
    int status;
    ADC_ChannelConfTypeDef cfg;
} adc_channel_t;

typedef struct {
    IRQn_Type id;
    uint32_t raw[ADC_MAX_CHANNELS];
} adc_private_t;

typedef struct {
    ADC_HandleTypeDef init;
    dma_t dma;
    adc_channel_t channel[ADC_MAX_CHANNELS];
    int irq_priority;
    adc_private_t p;
} adc_t;

int adc_init(adc_t *cfg);
uint32_t adc_get_raw(adc_t *cfg, uint8_t channel);
double adc_get_volt(adc_t *cfg, uint8_t channel);

#endif  // ADC_H
