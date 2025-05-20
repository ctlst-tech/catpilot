#include <math.h>
#include "cube_sensors_ak09916.h"
#include "board.h"

void cube_sensors_ak09916_exec(cube_sensors_ak09916_outputs_t* o)
{
    ak09916_meas_t meas;
    ak09916_get_meas_block(ak09916, &meas);
    o->mx = meas.mag_x;
    o->my = meas.mag_y;
    o->mz = meas.mag_z;
} 