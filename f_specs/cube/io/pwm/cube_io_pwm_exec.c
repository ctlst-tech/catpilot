#include "cube_io_pwm.h"
#include "cubeio.h"
#include "gpio.h"
#include "init.h"

void cube_io_pwm_exec(const cube_io_pwm_inputs_t *i, cube_io_pwm_outputs_t *o)
{
    double pwm[16];

    GPIO_Set(&gpio_fmu_pwm_2);

    if(i->arm) {
        CubeIO_ForceSafetyOff();
    } else {
        CubeIO_ForceSafetyOn();
    }

    static int init = 0;
    if(!init) {
        for(int i = 0; i < 8; i++) {
            CubeIO_SetRange(CubeIO_PWM, i, 1000, 1000, 3000);
        }
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
    // GPIO_Reset(&gpio_fmu_pwm_2);

    o->stub = i->arm;
}

