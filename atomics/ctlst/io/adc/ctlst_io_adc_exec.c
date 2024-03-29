#include <malloc.h>

#include "ad7606b.h"
#include "ctlst_io_adc.h"

ad7606b_instance_t *adc = NULL;

fspec_rv_t ctlst_io_adc_pre_exec_init(const ctlst_io_adc_params_t *p) {
    if (adc == NULL) {
        adc = calloc(1, sizeof(ad7606b_instance_t));
        if (adc == NULL) {
            return fspec_rv_no_memory;
        }
        if (adc_ad7606b_init(adc, AD7606B_BASE_ADDR)) {
            free(adc);
            return fspec_rv_system_err;
        };
        adc_ad7606b_set_disc_period(adc, 1000, 100000000);
    }
    adc_ad7606b_stop(adc);
    if (p->range == 5) {
        adc_ad7606b_set_range(adc, p->channel, RANGE_5V);
    } else if (p->range == 10) {
        adc_ad7606b_set_range(adc, p->channel, RANGE_10V);
    } else {
        adc_ad7606b_set_range(adc, p->channel, RANGE_2V5);
    }
    adc_ad7606b_start(adc);
    return fspec_rv_ok;
}

void ctlst_io_adc_exec(ctlst_io_adc_outputs_t *o,
                       const ctlst_io_adc_params_t *p) {
    int16_t value = 0;
    if (p->channel < 8) {
        adc_ad7606b_get_adc_value(adc, 0, p->channel, p->mux, &value);
    } else if (p->channel >= 8 && p->channel < 16) {
        adc_ad7606b_get_adc_value(adc, 1, p->channel - 8, p->mux, &value);
    }
    if (p->range == 5) {
        o->output = adc_convert_value(value, RANGE_5V) * p->scale + p->bias;
    } else if (p->range == 10) {
        o->output = adc_convert_value(value, RANGE_10V) * p->scale + p->bias;
    } else {
        o->output = adc_convert_value(value, RANGE_2V5) * p->scale + p->bias;
    }
    // printf("ch =%d, value = %d, output = %lf\n", p->channel, value, o->output);
}
