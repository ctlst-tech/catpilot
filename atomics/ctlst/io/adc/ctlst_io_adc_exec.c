#include <malloc.h>

#include "ad7606b.h"
#include "ctlst_io_adc.h"

ad7606b_instance_t *adc = NULL;

fspec_rv_t ctlst_io_adc_pre_exec_init(const ctlst_io_adc_params_t *p) {
    if (adc != NULL) {
        return fspec_rv_exists;
    }
    adc = calloc(1, sizeof(ad7606b_instance_t));
    if (adc == NULL) {
        return fspec_rv_no_memory;
    }
    if (adc_ad7606b_init(adc, AD7606B_BASE_ADDR)) {
        free(adc);
        return fspec_rv_system_err;
    };
    // adc_ad7606b_set_disc_period(i, p->period, );
    adc_ad7606b_start(adc);
    return fspec_rv_ok;
}

void ctlst_io_adc_exec(ctlst_io_adc_outputs_t *o,
                       const ctlst_io_adc_params_t *p) {
    adc_ad7606b_print_raw_adc_values(adc);
    uint16_t value = 0;
    adc_ad7606b_get_adc_value(adc, p->adc, p->channel, value);
    o->output = value * p->scale + p->bias;
}
