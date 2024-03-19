#include "ctlst_io_phase_in.h"
#include "gpio.h"

fspec_rv_t ctlst_io_phase_in_pre_exec_init(
    const ctlst_io_phase_in_params_t *p) {
    if (gpio_init(p->channel)) {
        return fspec_rv_invarg;
    }
    if (gpio_set_phase_mode_in(p->channel, p->total_teeth, p->missing_tooth,
                               p->tooth_step)) {
        return fspec_rv_invarg;
    }
    return fspec_rv_ok;
}

void ctlst_io_phase_in_exec(ctlst_io_phase_in_outputs_t *o,
                            const ctlst_io_phase_in_params_t *p) {
    uint32_t period;
    gpio_get_period(p->channel, &period);
    gpio_get_phase_step(p->channel, &o->step);
    gpio_get_tooth(p->channel, &o->tooth);
    o->period = (double)period;
}
