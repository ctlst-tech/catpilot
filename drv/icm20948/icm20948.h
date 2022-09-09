#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define FIFO_SIZE 1008
#define FIFO_SAMPLES 72

typedef struct {
    double accel_x[FIFO_SAMPLES];
    double accel_y[FIFO_SAMPLES];
    double accel_z[FIFO_SAMPLES];
    double gyro_x[FIFO_SAMPLES];
    double gyro_y[FIFO_SAMPLES];
    double gyro_z[FIFO_SAMPLES];
    double mag_x[FIFO_SAMPLES];
    double mag_y[FIFO_SAMPLES];
    double mag_z[FIFO_SAMPLES];
    double temp;
    uint32_t samples;
    uint32_t imu_dt;
    uint32_t mag_dt;
} icm20948_data_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
    double mag_scale;
    double mag_range;
} icm20948_param_t;

typedef struct {
    spi_cfg_t *spi;
    gpio_cfg_t *cs;
    exti_cfg_t *drdy;
    i2c_cfg_t *i2c;
    icm20948_param_t param;
} icm20948_cfg_t;

enum icm20948_state_t {
    ICM20948_RESET,
    ICM20948_RESET_WAIT,
    ICM20948_CONF,
    ICM20948_FIFO_READ,
    ICM20948_FAIL
};

int ICM20948_Init(spi_cfg_t *spi, gpio_cfg_t *cs, exti_cfg_t *drdy, i2c_cfg_t *i2c);
int ICM20948_Operation(void);
void ICM20948_Run(void);
void ICM20948_DataReadyHandler(void);

double ICM20948_Get_ax(void);
double ICM20948_Get_ay(void);
double ICM20948_Get_az(void);
double ICM20948_Get_wx(void);
double ICM20948_Get_wy(void);
double ICM20948_Get_wz(void);
int ICM20948_MeasReady(void);
