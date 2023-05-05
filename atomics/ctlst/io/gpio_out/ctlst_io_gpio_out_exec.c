#include "ctlst_io_gpio_out.h"
#include "gpio.h"

fspec_rv_t ctlst_io_gpio_out_pre_exec_init(
    const ctlst_io_gpio_out_optional_inputs_flags_t *input_flags,
    const ctlst_io_gpio_out_params_t *p) {
    if (gpio_init(p->channel)) {
        return fspec_rv_system_err;
    }
    if (gpio_set_discrete_mode_out(p->channel)) {
        return fspec_rv_system_err;
    }
    return fspec_rv_ok;
}

void ctlst_io_gpio_out_exec(const ctlst_io_gpio_out_inputs_t *i,
                            const ctlst_io_gpio_out_params_t *p) {
    if (i->optional_inputs_flags.input_bool) {
        gpio_set_output_value(p->channel, !i->input_bool);
    }
    if (i->optional_inputs_flags.input_float) {
        gpio_set_output_value(p->channel, i->input_float > 0.5 ? 0 : 1);
    } else {
        uint32_t value[1];
        gpio_get_output_value(p->channel, value);
        gpio_set_output_value(p->channel, ~value[0] & 0x01);
    }
}
