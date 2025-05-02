#include <math.h>
#include "cube_sensors_ak09918.h"
#include "board.h"

void cube_sensors_ak09918_exec(cube_sensors_ak09918_outputs_t* o)
{
    ak09918_meas_t meas;
    ak09918_get_meas_non_block(ak09918, &meas);
    o->mx = meas.mag_x;
    o->my = meas.mag_y;
    o->mz = meas.mag_z;
} 