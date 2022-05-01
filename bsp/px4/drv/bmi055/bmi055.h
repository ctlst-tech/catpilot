#include "stm32_base.h"
#include "stm32_periph.h"
#include "const.h"

#define BMI055_DEBUG
#define FIFO_SIZE 1008
#define FIFO_SAMPLES 72

typedef struct {
    float gyro_scale;
    float gyro_range;
    float accel_scale;
    float accel_range;
} bmi055_param_t;

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
} bmi055_fifo_t;

extern bmi055_fifo_t bmi055_fifo;

int BMI055_Init();
void BMI055_Run();
