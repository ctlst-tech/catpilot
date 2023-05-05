#include <malloc.h>

#include "ctlst_io_gpio.h"
#include "gpio.h"

gpio_instance_t *gpio = NULL;

fspec_rv_t ctlst_io_gpio_pre_exec_init(
    const ctlst_io_gpio_optional_inputs_flags_t *input_flags,
    const ctlst_io_gpio_params_t *p) {
    if (gpio == NULL) {
        gpio = calloc(1, sizeof(gpio_instance_t));
        if (gpio == NULL) {
            return fspec_rv_no_memory;
        }
        if (gpio_init(gpio, IO_BASE_ADDR)) {
            free(gpio);
            return fspec_rv_system_err;
        };
    }
    if (gpio_set_discrete_mode(gpio, p->channel)) {
        return fspec_rv_system_err;
    }
    return fspec_rv_ok;
}

void ctlst_io_gpio_exec(const ctlst_io_gpio_inputs_t *i,
                        const ctlst_io_gpio_params_t *p) {
    // if (p->channel < 1 || p->channel > 6) {
    //     return;
    // }
    if (i->optional_inputs_flags.input_bool) {
        gpio_set_output_value(gpio, p->channel, i->input_bool);
    }
    if (i->optional_inputs_flags.input_float) {
        gpio_set_output_value(gpio, p->channel, i->input_float > 0.5 ? 1 : 0);
    } else {
        uint32_t value[1];
        gpio_get_output_value(gpio, p->channel, value);
        gpio_set_output_value(gpio, p->channel, ~value[0] & 0x01 );
    }
}
