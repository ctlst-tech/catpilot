#include "ph_periph_io_pwm.h"

extern int PX4IO_Ready();
extern void PX4IO_SetOutput(int channel, double out);
extern void PX4IO_SetArm(bool arm);

void ph_periph_io_pwm_exec(const ph_periph_io_pwm_inputs_t *i, ph_periph_io_pwm_outputs_t *o)
{
    if(PX4IO_Ready()) {
        o->stub = 0;
        PX4IO_SetOutput(1, i->pwm_channel1);
        PX4IO_SetOutput(2, i->pwm_channel2);
        PX4IO_SetOutput(3, i->pwm_channel3);
        PX4IO_SetOutput(4, i->pwm_channel4);
        PX4IO_SetOutput(5, i->pwm_channel5);
        PX4IO_SetOutput(6, i->pwm_channel6);
        PX4IO_SetOutput(7, i->pwm_channel7);
        PX4IO_SetOutput(8, i->pwm_channel8);
        PX4IO_SetArm(i->arm);
    }
}

