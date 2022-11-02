#ifndef ADC_H
#define ADC_H

#include "core.h"
#include "dma.h"
#include "hal.h"
#include "os.h"

#define ADC_MAX_CHANNELS 16

typedef struct {
    int status;
    ADC_ChannelConfTypeDef cfg;
} adc_channel_t;

typedef struct {
    ADC_HandleTypeDef init;
    dma_t *dma;
    adc_channel_t channel[ADC_MAX_CHANNELS];
    uint16_t raw[ADC_MAX_CHANNELS];
    IRQn_Type irq;
    int irq_priority;
} adc_t;

int adc_init(adc_t *cfg);
int adc_clock_enable(adc_t *cfg);
int adc_enable_irq(adc_t *cfg);
int adc_disable_irq(adc_t *cfg);
int adc_handler(adc_t *cfg);
int adc_dma_handler(adc_t *cfg);

uint16_t adc_get_raw(adc_t *cfg, int ch);
double adc_get_volt(adc_t *cfg, int ch);

#endif  // ADC_H
