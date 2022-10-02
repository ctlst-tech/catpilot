#include "cube_io_pwm.h"
#include "cubeio.h"
#include "gpio.h"
#include "init.h"

void cube_io_pwm_exec(const cube_io_pwm_inputs_t *i, cube_io_pwm_outputs_t *o)
{
    double pwm[16];

    GPIO_Set(&gpio_fmu_pwm[1]);

    if(i->arm) {
        CubeIO_ForceSafetyOff();
    } else {
        CubeIO_ForceSafetyOn();
    }

    // TODO use state, replace scale to spec
    static int init = 0;
    if(!init) {
        CubeIO_SetRange(CubeIO_PWM, 0, CubeIO_ChannelUnipolar, 700, 2200);
        CubeIO_SetRange(CubeIO_PWM, 1, CubeIO_ChannelUnipolar, 700, 2200);
        CubeIO_SetRange(CubeIO_PWM, 2, CubeIO_ChannelUnipolar, 700, 2200);
        CubeIO_SetRange(CubeIO_PWM, 3, CubeIO_ChannelUnipolar, 700, 2200);
        CubeIO_SetRange(CubeIO_PWM, 4, CubeIO_ChannelUnipolar, 700, 2200);
        CubeIO_SetRange(CubeIO_PWM, 5, CubeIO_ChannelUnipolar, 700, 2200);
        CubeIO_SetRange(CubeIO_PWM, 6, CubeIO_ChannelUnipolar, 700, 2200);
        CubeIO_SetRange(CubeIO_PWM, 7, CubeIO_ChannelUnipolar, 700, 2200);
        init = 1;
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

    // For debug reset pin in CubeIO thread
    // GPIO_Reset(&gpio_fmu_pwm[1]);

    o->stub = i->arm;
}

