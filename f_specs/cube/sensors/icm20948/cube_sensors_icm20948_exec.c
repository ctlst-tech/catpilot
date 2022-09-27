#include "cube_sensors_icm20948.h"
#include "icm20948.h"
#include "gpio.h"
#include "init.h"

void cube_sensors_icm20948_exec(cube_sensors_icm20948_outputs_t *o)
{
    icm20948_imu_meas_t meas;

    GPIO_Set(&gpio_fmu_pwm_1);

    ICM20948_GetMeasBlock(&meas);
    o->ax = meas.accel_x;
    o->ay = meas.accel_y;
    o->az = meas.accel_z;
    o->wx = meas.gyro_x;
    o->wy = meas.gyro_y;
    o->wz = meas.gyro_z;

    GPIO_Reset(&gpio_fmu_pwm_1);
}
