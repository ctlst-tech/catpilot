#include "ctlst_io_pwm_out.h"
#include "gpio.h"

fspec_rv_t ctlst_io_pwm_out_pre_exec_init(
    const ctlst_io_pwm_out_optional_inputs_flags_t *input_flags,
    const ctlst_io_pwm_out_params_t *p) {
    if (gpio_init(p->channel)) {
        return fspec_rv_invarg;
    }
    if (gpio_set_pwm_mode(p->channel)) {
        return fspec_rv_invarg;
    }
    if (gpio_set_period(p->channel, p->period)) {
        return fspec_rv_invarg;
    }
    return fspec_rv_ok;
}

void ctlst_io_pwm_out_exec(const ctlst_io_pwm_out_inputs_t *i,
                           const ctlst_io_pwm_out_params_t *p) {
    double in = 0;
    uint32_t out = 0;

    in = i->input > 1.0 ? 1.0 : i->input;

    if (p->bipolar) {
        in = in < -1.0 ? -1.0 : in;
        out = (uint32_t)((in + 1.0) * (p->max - p->min) / 2 + p->min);
    } else {
        in = in < 0.0 ? 0.0 : in;
        out = (uint32_t)(in * (p->max - p->min) + p->min);
    }
    gpio_set_width(p->channel, out);
}
