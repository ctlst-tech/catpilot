#include "ctlst_io_pwm_in.h"
#include "gpio.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SAT(in, max, min) MIN(MAX(in, min), max)

fspec_rv_t ctlst_io_pwm_in_pre_exec_init(const ctlst_io_pwm_in_params_t *p) {
    if (gpio_init(p->channel)) {
        return fspec_rv_invarg;
    }
    if (gpio_set_pwm_mode_in(p->channel)) {
        return fspec_rv_invarg;
    }
    return fspec_rv_ok;
}

void ctlst_io_pwm_in_exec(ctlst_io_pwm_in_outputs_t *o,
                          const ctlst_io_pwm_in_params_t *p) {
    uint32_t width = 0;
    double duty = 0;
    gpio_get_period(p->channel, &o->period);
    gpio_get_width(p->channel, &width);
    if (p->bipolar) {
        duty = (2 * (width - p->min) / (double)(p->max - p->min) - 1.0);
        duty = SAT(duty, 1.0, -1.0);
    } else {
        duty = (int)(width - p->min) / (double)(p->max - p->min);
        duty = SAT(duty, 1.0, 0.0);
    }
    o->duty = duty;
}
