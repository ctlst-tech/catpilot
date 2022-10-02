#include "cube_io_rc.h"
#include "cubeio.h"
#include "gpio.h"
#include "init.h"

void cube_io_rc_exec(cube_io_rc_outputs_t *o)
{
    double rc[16];

    GPIO_Set(&gpio_fmu_pwm[2]);

    // TODO use state, replace scale to spec
    static int init = 0;
    if(!init) {
        CubeIO_SetRange(CubeIO_RC, 0, CubeIO_ChannelBipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 1, CubeIO_ChannelBipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 2, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 3, CubeIO_ChannelBipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 4, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 5, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 6, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 7, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 8, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 9, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 9, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 10, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 11, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 12, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 13, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 14, CubeIO_ChannelUnipolar, 982, 2006);
        CubeIO_SetRange(CubeIO_RC, 15, CubeIO_ChannelUnipolar, 982, 2006);
        init = 1;
    }

    CubeIO_GetRC(rc);
    o->rc_channel1 = rc[0];
    o->rc_channel2 = rc[1];
    o->rc_channel3 = rc[2];
    o->rc_channel4 = rc[3];
    o->rc_channel5 = rc[4];
    o->rc_channel6 = rc[5];
    o->rc_channel7 = rc[6];
    o->rc_channel8 = rc[7];
    o->rc_channel9 = rc[8];
    o->rc_channel10 = rc[9];
    o->rc_channel11 = rc[10];
    o->rc_channel12 = rc[11];
    o->rc_channel13 = rc[12];
    o->rc_channel14 = rc[13];
    o->rc_channel15 = rc[14];
    o->rc_channel16 = rc[15];

    // For debug reset pin in CubeIO thread
    // GPIO_Reset(&gpio_fmu_pwm[2]);
}

