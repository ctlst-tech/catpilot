#include <math.h>
#include "cube_sensors_icm45686.h"
#include "board.h"

#define deg2rad(d) ((d) * (M_PI / 180.0))

void cube_sensors_icm45686_exec(cube_sensors_icm45686_outputs_t* o)
{
    icm45686_meas_t meas;
    gpio_set(&gpio_fmu_pwm[0]);
    icm45686_get_meas_non_block(icm45686, &meas);
    
    o->wx = deg2rad(meas.gyro_x);
    o->wy = deg2rad(meas.gyro_y);
    o->wz = deg2rad(meas.gyro_z);
        
    o->ax = -meas.accel_x;
    o->ay = -meas.accel_y;
    o->az = -meas.accel_z;
    gpio_reset(&gpio_fmu_pwm[0]);
} 