#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define ICM20602_FIFO_SIZE 1008
#define ICM20602_FIFO_SAMPLES 72

typedef struct {
    double accel_x[ICM20602_FIFO_SAMPLES];
    double accel_y[ICM20602_FIFO_SAMPLES];
    double accel_z[ICM20602_FIFO_SAMPLES];
    double gyro_x[ICM20602_FIFO_SAMPLES];
    double gyro_y[ICM20602_FIFO_SAMPLES];
    double gyro_z[ICM20602_FIFO_SAMPLES];
    double temp;
    uint32_t samples;
    uint32_t dt;
} icm20602_fifo_t;

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm20602_param_t;

typedef struct {
    spi_cfg_t *spi;
    gpio_cfg_t *cs;
    exti_cfg_t *drdy;
    icm20602_param_t param;
} icm20602_cfg_t;

enum icm20602_state_t {
    ICM20602_RESET,
    ICM20602_RESET_WAIT,
    ICM20602_CONF,
    ICM20602_FIFO_READ,
    ICM20602_FAIL
};

int ICM20602_Init(spi_cfg_t *spi, gpio_cfg_t *cs, exti_cfg_t *drdy);
int ICM20602_Operation(void);
void ICM20602_Run(void);
void ICM20602_DataReadyHandler(void);

double ICM20602_Get_ax(void);
double ICM20602_Get_ay(void);
double ICM20602_Get_az(void);
double ICM20602_Get_wx(void);
double ICM20602_Get_wy(void);
double ICM20602_Get_wz(void);
int ICM20602_MeasReady(void);
