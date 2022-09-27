#include "cube_io_rc.h"
#include "cubeio.h"

void cube_io_rc_exec(cube_io_rc_outputs_t *o)
{
    uint16_t rc[16];
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
}

