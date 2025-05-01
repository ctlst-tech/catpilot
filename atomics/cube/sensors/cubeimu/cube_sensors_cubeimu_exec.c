#include <math.h>
#include "cube_sensors_cubeimu.h"
#include "board.h"

#define deg2rad(d) ((d) * (M_PI / 180.0))

void cube_sensors_cubeimu_exec(cube_sensors_cubeimu_outputs_t *o)
{
    icm20649_meas_t meas_imu1;
    icm45686_meas_t meas_imu2;
    uint8_t use_icm20649 = 0;
    uint8_t use_icm45686 = 0;

    if (icm20649->state!=ICM20649_FAIL)
    {
        use_icm20649 = 1;
    }
    else if (icm45686->state!=ICM45686_FAIL)
    {
        use_icm45686 = 1;
    }

    if (use_icm20649)
    {
        gpio_set(&gpio_fmu_pwm[0]);
        icm20649_get_meas_block(icm20649, &meas_imu1);

        o->wx = deg2rad(meas_imu1.gyro_x);
        o->wy = deg2rad(meas_imu1.gyro_y);
        o->wz = deg2rad(meas_imu1.gyro_z);

        o->ax = -meas_imu1.accel_x;
        o->ay = -meas_imu1.accel_y;
        o->az = -meas_imu1.accel_z;
        gpio_reset(&gpio_fmu_pwm[0]);
    }
    else if (use_icm45686)
    {
        gpio_set(&gpio_fmu_pwm[0]);
        icm45686_get_meas_block(icm45686, &meas_imu2);

        o->wx = deg2rad(meas_imu2.gyro_x);
        o->wy = deg2rad(meas_imu2.gyro_y);
        o->wz = deg2rad(meas_imu2.gyro_z);

        o->ax = -meas_imu2.accel_x;
        o->ay = -meas_imu2.accel_y;
        o->az = -meas_imu2.accel_z;
        gpio_reset(&gpio_fmu_pwm[0]);
    }
    else
    {
        o->wx = 0;
        o->wy = 0;
        o->wz = 0;
        o->ax = 0;
        o->ay = 0;
        o->az = 0;
    }   
}
