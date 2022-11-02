#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define ICM20948_FIFO_SIZE 4096
#define ICM20948_FIFO_SAMPLES 342

typedef struct {
    double accel_x[ICM20948_FIFO_SAMPLES];
    double accel_y[ICM20948_FIFO_SAMPLES];
    double accel_z[ICM20948_FIFO_SAMPLES];
    double gyro_x[ICM20948_FIFO_SAMPLES];
    double gyro_y[ICM20948_FIFO_SAMPLES];
    double gyro_z[ICM20948_FIFO_SAMPLES];
    double mag_x;
    double mag_y;
    double mag_z;
    double temp;
    uint32_t samples;
    uint32_t imu_dt;
    uint32_t mag_dt;
} icm20948_data_t;

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} icm20948_imu_meas_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
    double mag_scale;
    double mag_range;
} icm20948_param_t;

typedef struct {
    spi_t *spi;
    gpio_t *cs;
    exti_t *drdy;
    i2c_t *i2c;
    icm20948_param_t param;
    int enable_mag;
    int enable_drdy;
} icm20948_cfg_t;

enum icm20948_state_t {
    ICM20948_RESET,
    ICM20948_RESET_WAIT,
    ICM20948_CONF,
    ICM20948_FIFO_READ,
    ICM20948_FAIL
};

int ICM20948_Init(spi_t *spi, 
                  gpio_t *cs, 
                  exti_t *drdy, 
                  i2c_t *i2c,
                  int enable_mag,
                  int enable_drdy);
int ICM20948_Operation(void);
void ICM20948_Run(void);
void ICM20948_DataReadyHandler(void);

void ICM20948_GetMeasBlock(void *ptr);
void ICM20948_GetMeasNonBlock(void *ptr);
