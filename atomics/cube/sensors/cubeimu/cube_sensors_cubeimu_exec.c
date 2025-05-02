#include <math.h>
#include "cube_sensors_cubeimu.h"
#include "board.h"

#define deg2rad(d) ((d) * (M_PI / 180.0))

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} cubeimu_meas_t;

// Define function pointer type
typedef void (*imu_get_meas_block_fn_t)(void *imu_dev, cubeimu_meas_t *meas);

// Function pointer and selected device
static imu_get_meas_block_fn_t get_meas_block = NULL;
static void *selected_imu = NULL;

fspec_rv_t cube_io_rc_pre_exec_init() 
{
    if (icm20649)
    {
        if (icm20649->state!=ICM20649_FAIL)
        {
            get_meas_block = (imu_get_meas_block_fn_t)icm20649_get_meas_block;
            selected_imu = icm20649;
            return fspec_rv_ok;
        }
    }
    else if (icm45686)
    {
        if (icm45686->state!=ICM45686_FAIL)
        {
            get_meas_block = (imu_get_meas_block_fn_t)icm45686_get_meas_block;
            selected_imu = icm45686;
            return fspec_rv_ok;
        }
    }
    return fspec_rv_initerr;
}

void cube_sensors_cubeimu_exec(cube_sensors_cubeimu_outputs_t *o)
{
    cubeimu_meas_t meas_imu;
    gpio_set(&gpio_fmu_pwm[0]);
    if (get_meas_block && selected_imu)
    {
        get_meas_block(selected_imu, &meas_imu);
    }
    o->wx = deg2rad(meas_imu.gyro_x);
    o->wy = deg2rad(meas_imu.gyro_y);
    o->wz = deg2rad(meas_imu.gyro_z);

    o->ax = -meas_imu.accel_x;
    o->ay = -meas_imu.accel_y;
    o->az = -meas_imu.accel_z;
    gpio_reset(&gpio_fmu_pwm[0]);
}
