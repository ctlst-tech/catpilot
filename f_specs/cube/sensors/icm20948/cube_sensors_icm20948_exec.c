#include <math.h>

#include "cube_sensors_icm20948.h"
#include "icm20948.h"
#include "gpio.h"
#include "init.h"

#define deg2rad(d) ((d) * (M_PI / 180.0))

void cube_sensors_icm20948_exec(cube_sensors_icm20948_outputs_t *o)
{
    icm20948_imu_meas_t meas;

    GPIO_Set(&gpio_fmu_pwm_1);

    ICM20948_GetMeasBlock(&meas);
    o->ax = meas.accel_x;
    o->ay = meas.accel_y;
    o->az = meas.accel_z;
    o->wx = deg2rad(meas.gyro_x);
    o->wy = deg2rad(meas.gyro_y);
    o->wz = deg2rad(meas.gyro_z);

    GPIO_Reset(&gpio_fmu_pwm_1);
}
