#include "board.h"
#include "cube_sensors_adc.h"

void cube_sensors_adc_exec(cube_sensors_adc_outputs_t *o, const cube_sensors_adc_params_t *p) {
    o->output = p->scale * (adc_get_volt(&adc1, p->channel) - p->offset);
    o->max = p->scale * (adc_get_volt_max(&adc1, p->channel) - p->offset);
    o->min = p->scale * (adc_get_volt_min(&adc1, p->channel) - p->offset);
    adc_reset_stat_channel(&adc1, p->channel);
}
