#include "core_periph_px4io.h"

extern void PX4IO_Run();
extern void PX4IO_SetOutput();
extern void PX4IO_SetArm();
extern uint16_t PX4IO_GetRC();

void ph_periph_px4io_exec(
    const core_periph_px4io_inputs_t *i,
    core_periph_px4io_outputs_t *o,
    const core_periph_px4io_params_t *p,
    core_periph_px4io_state_t *state,
    const core_periph_px4io_injection_t *injection
)
{
    PX4IO_SetOutput(1, i->pwm_channel1);
    PX4IO_SetOutput(1, i->pwm_channel2);
    PX4IO_SetOutput(1, i->pwm_channel3);
    PX4IO_SetOutput(1, i->pwm_channel4);
    PX4IO_SetOutput(1, i->pwm_channel5);
    PX4IO_SetOutput(1, i->pwm_channel6);
    PX4IO_SetOutput(1, i->pwm_channel7);
    PX4IO_SetOutput(1, i->pwm_channel8);
    PX4IO_SetArm(i->arm);
    PX4IO_Run();
    o->rc_channel1 = PX4IO_GetRC(1);
    o->rc_channel2 = PX4IO_GetRC(2);
    o->rc_channel3 = PX4IO_GetRC(3);
    o->rc_channel4 = PX4IO_GetRC(4);
    o->rc_channel5 = PX4IO_GetRC(5);
    o->rc_channel6 = PX4IO_GetRC(6);
    o->rc_channel7 = PX4IO_GetRC(7);
    o->rc_channel8 = PX4IO_GetRC(8);
    o->rc_channel9 = PX4IO_GetRC(9);
    o->rc_channel10 = PX4IO_GetRC(10);
    o->rc_channel11 = PX4IO_GetRC(11);
    o->rc_channel12 = PX4IO_GetRC(12);
    o->rc_channel13 = PX4IO_GetRC(13);
    o->rc_channel14 = PX4IO_GetRC(14);
    o->rc_channel15 = PX4IO_GetRC(15);
    o->rc_channel16 = PX4IO_GetRC(16);
}
