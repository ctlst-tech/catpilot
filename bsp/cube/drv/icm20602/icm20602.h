#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define ICM20602_DEBUG
#define FIFO_SIZE 1008
#define FIFO_SAMPLES 72

typedef struct {
    double gyro_scale;
    double gyro_range;
    double accel_scale;
    double accel_range;
} icm20602_param_t;

typedef struct {
    double accel_x[FIFO_SAMPLES];
    double accel_y[FIFO_SAMPLES];
    double accel_z[FIFO_SAMPLES];
    double gyro_x[FIFO_SAMPLES];
    double gyro_y[FIFO_SAMPLES];
    double gyro_z[FIFO_SAMPLES];
    double temp;
    uint32_t samples;
    uint32_t dt;
} icm20602_fifo_t;

enum icm20602_state_t {
    ICM20602_RESET,
    ICM20602_RESET_WAIT,
    ICM20602_CONF,
    ICM20602_FIFO_READ
};

extern icm20602_fifo_t icm20602_fifo;
extern enum icm20602_state_t icm20602_state;

int ICM20602_Init();
void ICM20602_Run();

// TODO replace to another src
double ICM20602_Get_ax();
double ICM20602_Get_ay();
double ICM20602_Get_az();
double ICM20602_Get_wx();
double ICM20602_Get_wy();
double ICM20602_Get_wz();
int ICM20602_MeasReady();
