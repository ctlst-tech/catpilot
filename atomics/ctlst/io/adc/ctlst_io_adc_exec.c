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
        adc_ad7606b_start(adc);
    }
    return fspec_rv_ok;
}

void ctlst_io_adc_exec(ctlst_io_adc_outputs_t *o,
                       const ctlst_io_adc_params_t *p) {
    int16_t value = 0;
    if (p->channel < 8) {
        adc_ad7606b_get_adc_value(adc, 0, p->channel, &value);
    } else if (p->channel >= 8 && p->channel < 16) {
        adc_ad7606b_get_adc_value(adc, 1, p->channel - 8, &value);
    }
    o->output = adc_convert_value(value, 0) * p->scale + p->bias;
    // printf("ch =%d, value = %d, output = %lf\n", p->channel, value, o->output);
}
