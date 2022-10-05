#include "cube_io_pwm.h"
#include "cubeio.h"
#include "gpio.h"
#include "init.h"

void cube_io_pwm_exec(const cube_io_pwm_inputs_t *i, const cube_io_pwm_params_t *p, cube_io_pwm_state_t *state){
    double pwm[16];

    GPIO_Set(&gpio_fmu_pwm[1]);

    if(i->arm && !state->arm_passed) {
        CubeIO_ForceSafetyOff();
        state->arm_passed = TRUE;
        state->disarm_passed = FALSE;
    } else if (!i->arm && !state->disarm_passed){
        CubeIO_ForceSafetyOn();
        state->arm_passed = FALSE;
        state->disarm_passed = TRUE;
    }

    if(!state->inited) {
//        CubeIO_SetDefaultFreq(200);
        CubeIO_SetFreq(0xFFFF, 400);
        CubeIO_SetRange(CubeIO_PWM, 0, p->ch1_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch1_min, p->ch1_max);
        CubeIO_SetRange(CubeIO_PWM, 1, p->ch2_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch2_min, p->ch2_max);
        CubeIO_SetRange(CubeIO_PWM, 2, p->ch3_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch3_min, p->ch3_max);
        CubeIO_SetRange(CubeIO_PWM, 3, p->ch4_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch4_min, p->ch4_max);
        CubeIO_SetRange(CubeIO_PWM, 4, p->ch5_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch5_min, p->ch5_max);
        CubeIO_SetRange(CubeIO_PWM, 5, p->ch6_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch6_min, p->ch6_max);
        CubeIO_SetRange(CubeIO_PWM, 6, p->ch7_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch7_min, p->ch7_max);
        CubeIO_SetRange(CubeIO_PWM, 7, p->ch8_bipolar ? CubeIO_ChannelBipolar : CubeIO_ChannelUnipolar, p->ch8_min, p->ch8_max);
        state->inited = TRUE;
    }

    pwm[0] = i->ch1;
    pwm[1] = i->ch2;
    pwm[2] = i->ch3;
    pwm[3] = i->ch4;
    pwm[4] = i->ch5;
    pwm[5] = i->ch6;
    pwm[6] = i->ch7;
    pwm[7] = i->ch8;

    CubeIO_SetPWM(8, pwm);

    // For debug reset pin in CubeIO thread
    // GPIO_Reset(&gpio_fmu_pwm[1]);
}
