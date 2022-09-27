#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define ICM20649_FIFO_SIZE 4096
#define ICM20649_FIFO_SAMPLES 342

typedef struct {
    double accel_x[ICM20649_FIFO_SAMPLES];
    double accel_y[ICM20649_FIFO_SAMPLES];
    double accel_z[ICM20649_FIFO_SAMPLES];
    double gyro_x[ICM20649_FIFO_SAMPLES];
    double gyro_y[ICM20649_FIFO_SAMPLES];
    double gyro_z[ICM20649_FIFO_SAMPLES];
    double temp;
    uint32_t samples;
    uint32_t imu_dt;
    uint32_t mag_dt;
} icm20649_data_t;

typedef struct {
    double accel_x;
    double accel_y;
    double accel_z;
    double gyro_x;
    double gyro_y;
    double gyro_z;
    double imu_dt;
} icm20649_imu_meas_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm20649_param_t;

typedef struct {
    spi_cfg_t *spi;
    gpio_cfg_t *cs;
    exti_cfg_t *drdy;
    icm20649_param_t param;
    int enable_drdy;
} icm20649_cfg_t;

enum icm20649_state_t {
    ICM20649_RESET,
    ICM20649_RESET_WAIT,
    ICM20649_CONF,
    ICM20649_FIFO_READ,
    ICM20649_FAIL
};

int ICM20649_Init(spi_cfg_t *spi, 
                  gpio_cfg_t *cs, 
                  exti_cfg_t *drdy, 
                  int enable_drdy);
int ICM20649_Operation(void);
void ICM20649_Run(void);
void ICM20649_DataReadyHandler(void);

void ICM20649_GetMeasBlock(void *ptr);
void ICM20649_GetMeasNonBlock(void *ptr);
