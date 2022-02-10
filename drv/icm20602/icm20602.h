#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"
#include "const.h"

#define ICM20602_DEBUG
#define FIFO_SIZE 1008
#define FIFO_SAMPLES 72

typedef struct {
    float gyro_scale;
    float gyro_range;
    float accel_scale;
    float accel_range;
} icm20602_dim_t;

typedef struct {
    spi_cfg_t spi;
    icm20602_dim_t dim;
} icm20602_cfg_t;

typedef struct {
    float accel_x[FIFO_SAMPLES];
    float accel_y[FIFO_SAMPLES];
    float accel_z[FIFO_SAMPLES];
    float gyro_x[FIFO_SAMPLES];
    float gyro_y[FIFO_SAMPLES];
    float gyro_z[FIFO_SAMPLES];
    float temp;
} icm20602_fifo_t;

extern icm20602_fifo_t icm20602_fifo;

int ICM20602_Init();
void ICM20602_Run();

uint8_t ICM20602_ReadReg(uint8_t reg);
void ICM20602_WriteReg(uint8_t reg, uint8_t value);
void ICM20602_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);

int ICM20602_Configure();
void ICM20602_AccelConfigure();
void ICM20602_GyroConfigure();

int ICM20602_Probe();
void ICM20602_Statistics();

void ICM20602_FIFOCount();
int ICM20602_FIFORead();
void ICM20602_FIFOReset();

void ICM20602_AccelProcess();
void ICM20602_GyroProcess();
void ICM20602_TempProcess();
