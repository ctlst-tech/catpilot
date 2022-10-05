#include "cube_sensors_icm20649.h"
#include "icm20649.h"
#include "gpio.h"
#include "init.h"

#define deg2rad(d) ((d) * (M_PI / 180.0))

void cube_sensors_icm20649_exec(cube_sensors_icm20649_outputs_t *o)
{
    icm20649_imu_meas_t meas;

    GPIO_Set(&gpio_fmu_pwm[0]);

    ICM20649_GetMeasBlock(&meas);
    o->wx = deg2rad(meas.gyro_x);
    o->wy = deg2rad(meas.gyro_y);
    o->wz = deg2rad(meas.gyro_z);

    o->ax = -meas.accel_x;
    o->ay = -meas.accel_y;
    o->az = -meas.accel_z;

    GPIO_Reset(&gpio_fmu_pwm[0]);
}
