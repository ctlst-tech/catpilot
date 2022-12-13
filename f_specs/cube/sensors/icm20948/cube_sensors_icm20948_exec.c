#include <math.h>
#include "cube_sensors_icm20948.h"
#include "../../../../bsp/boards/cube/board.h"
#include "../../../../bsp/ics/icm20649/icm20649.h"
#include "../../../../bsp/mcu/periph/gpio/stm32/gpio.h"

#define deg2rad(d) ((d) * (M_PI / 180.0))

void cube_sensors_icm20948_exec(cube_sensors_icm20948_outputs_t *o)
{
    gpio_set(&gpio_fmu_pwm[0]);

    icm20948_meas_t meas;

    icm20948_get_meas_block(icm20948, &meas);

    o->wx = deg2rad(meas.gyro_x);
    o->wy = deg2rad(meas.gyro_y);
    o->wz = deg2rad(meas.gyro_z);

    o->ax = -meas.accel_x;
    o->ay = -meas.accel_y;
    o->az = -meas.accel_z;

    gpio_reset(&gpio_fmu_pwm[0]);
}
