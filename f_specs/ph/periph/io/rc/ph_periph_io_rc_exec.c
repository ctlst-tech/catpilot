#include "ph_periph_io_rc.h"

extern int PX4IO_Ready();
extern float PX4IO_GetRC(int channel);

void ph_periph_io_rc_exec(ph_periph_io_rc_outputs_t *o)
{
    if(PX4IO_Ready()) {
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
}

