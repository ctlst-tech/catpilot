#include "ctlst_io_phase_out.h"
#include "gpio.h"

fspec_rv_t ctlst_io_phase_out_pre_exec_init(
    const ctlst_io_phase_out_params_t *p) {
    if (gpio_init(p->channel)) {
        return fspec_rv_invarg;
    }
    if (gpio_set_phase_mode_out(p->channel, p->invert, p->time_mode,
                                p->sync_channel, p->tooth, p->phase_on,
                                p->phase_off)) {
        return fspec_rv_invarg;
    }
    return fspec_rv_ok;
}

void ctlst_io_phase_out_exec(const ctlst_io_phase_out_params_t *p) {
    return;
}
