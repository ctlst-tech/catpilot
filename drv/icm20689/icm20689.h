#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define FIFO_SIZE 1008
#define FIFO_SAMPLES 72

typedef struct {
    float gyro_scale;
    float gyro_range;
    float accel_scale;
    float accel_range;
} icm20689_param_t;

typedef struct {
    float accel_x[FIFO_SAMPLES];
    float accel_y[FIFO_SAMPLES];
    float accel_z[FIFO_SAMPLES];
    float gyro_x[FIFO_SAMPLES];
    float gyro_y[FIFO_SAMPLES];
    float gyro_z[FIFO_SAMPLES];
    float temp;
    uint32_t samples;
    uint32_t dt;
} icm20689_fifo_t;

typedef struct {
    spi_cfg_t *spi;
    gpio_cfg_t *cs;
    exti_cfg_t *drdy;
    icm20689_param_t param;
} icm20689_cfg_t;

enum icm20689_state_t {
    ICM20689_RESET,
    ICM20689_RESET_WAIT,
    ICM20689_CONF,
    ICM20689_FIFO_READ
};

int ICM20689_Init(spi_cfg_t *spi, gpio_cfg_t *cs, exti_cfg_t *drdy);
int ICM20689_Operation(void);
void ICM20689_Run(void);
void ICM20689_DataReadyHandler(void);

double ICM20689_Get_ax(void);
double ICM20689_Get_ay(void);
double ICM20689_Get_az(void);
double ICM20689_Get_wx(void);
double ICM20689_Get_wy(void);
double ICM20689_Get_wz(void);
int ICM20689_MeasReady(void);
