#include <math.h>
#include "cube_sensors_ist8310.h"
#include "board.h"

void cube_sensors_ist8310_exec(cube_sensors_ist8310_outputs_t *o)
{
    ist8310_meas_t meas;
    
    if (ist8310)
    {
        ist8310_get_meas_block(ist8310, &meas);
        
        o->magx = meas.mag_y;
        o->magy = meas.mag_x;
        o->magz = -meas.mag_z;
    }
    else
    {
        o->magx = 0;
        o->magy = 0;
        o->magz = 0;
    }
}
