#include "core_periph_px4io_rc.h"
#include "px4io.h"

extern void PX4IO_Run();
extern float PX4IO_GetRC(int channel);

extern enum px4io_state_t px4io_state;

void core_periph_px4io_rc_exec(core_periph_px4io_rc_outputs_t *o)
{
    PX4IO_Run();
    o->rc_channel1 = 0;
    o->rc_channel2 = 0;
    o->rc_channel3 = 0;
    o->rc_channel4 = 0;
    o->rc_channel5 = 0;
    o->rc_channel6 = 0;
    o->rc_channel7 = 0;
    o->rc_channel8 = 0;
    o->rc_channel9 = 0;
    o->rc_channel10 = 0;
    o->rc_channel11 = 0;
    o->rc_channel12 = 0;
    o->rc_channel13 = 0;
    o->rc_channel14 = 0;
    o->rc_channel15 = 0;
    o->rc_channel16 = 0;
    if(px4io_state != PX4IO_OPERATION) {
        return;
    }
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

