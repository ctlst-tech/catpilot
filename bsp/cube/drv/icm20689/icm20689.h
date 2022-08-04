#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define ICM20689_DEBUG
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

enum icm20689_state_t {
    ICM20689_RESET,
    ICM20689_RESET_WAIT,
    ICM20689_CONF,
    ICM20689_FIFO_READ
};

extern icm20689_fifo_t icm20689_fifo;
extern enum icm20689_state_t icm20689_state;

int ICM20689_Init();
void ICM20689_Run();
