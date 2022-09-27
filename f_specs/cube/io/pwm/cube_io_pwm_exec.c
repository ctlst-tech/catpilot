#include "cube_io_pwm.h"
#include "cubeio.h"

void cube_io_pwm_exec(const cube_io_pwm_inputs_t *i, cube_io_pwm_outputs_t *o)
{
    uint16_t pwm[16];

    if(i->arm) {
        CubeIO_ForceSafetyOff();
    } else {
        CubeIO_ForceSafetyOn();
    }

    pwm[0] = i->pwm_channel1;
    pwm[1] = i->pwm_channel2;
    pwm[2] = i->pwm_channel3;
    pwm[3] = i->pwm_channel4;
    pwm[4] = i->pwm_channel5;
    pwm[5] = i->pwm_channel6;
    pwm[6] = i->pwm_channel7;
    pwm[7] = i->pwm_channel8;

    CubeIO_SetPWM(8, pwm);

    o->stub = 0;
}

