#include "ctlst_io_gpio_in.h"
#include "gpio.h"

fspec_rv_t ctlst_io_gpio_in_pre_exec_init(const ctlst_io_gpio_in_params_t *p)
{
    if (gpio_init(p->channel)) {
        return fspec_rv_system_err;
    }
    if (gpio_set_discrete_mode_in(p->channel)) {
        return fspec_rv_system_err;
    }
    return fspec_rv_ok;
}

void ctlst_io_gpio_in_exec(ctlst_io_gpio_in_outputs_t *o, const ctlst_io_gpio_in_params_t *p)
{
    uint32_t value = 0;
    gpio_get_input_value(p->channel, &value);
    o->output = value;
}
