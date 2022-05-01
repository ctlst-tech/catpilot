#include "bmi055.h"
#include "bmi055_reg.h"
#include "init.h"
#include "cfg.h"

static char *device = "BMI055";

bmi055_fifo_t bmi055_fifo;

enum bmi055_state_t {
    BMI055_RESET,
    BMI055_RESET_WAIT,
    BMI055_CONF,
    BMI055_FIFO_READ
};
enum bmi055_state_t bmi055_state;

uint8_t BMI055_ReadReg(uint8_t reg);
void BMI055_WriteReg(uint8_t reg, uint8_t value);
void BMI055_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
int BMI055_Configure();
void BMI055_AccelConfigure();
void BMI055_GyroConfigure();
int BMI055_Probe();
void BMI055_Statistics();
void BMI055_FIFOCount();
int BMI055_FIFORead();
void BMI055_FIFOReset();
void BMI055_AccelProcess();
void BMI055_GyroProcess();
void BMI055_TempProcess();

static gpio_cfg_t bmi055_gyro_cs = GPIO_SPI1_CS3;
static gpio_cfg_t bmi055_accel_cs = GPIO_SPI1_CS4;
static exti_cfg_t bmi055_gyro_drdy = EXTI_SPI1_DRDY3;
static exti_cfg_t bmi055_accel_drdy = EXTI_SPI1_DRDY4;
static SemaphoreHandle_t gyro_drdy_semaphore;
static SemaphoreHandle_t accel_drdy_semaphore;

typedef struct {
    spi_cfg_t *spi;
    bmi055_param_t param;
} bmi055_cfg_t;
static bmi055_cfg_t bmi055_cfg;

static TickType_t bmi055_accel_last_sample = 0;
static TickType_t bmi055_gyro_last_sample = 0;

int BMI055_Init() {
    int rv = 0;

    bmi055_cfg.spi = &spi1;

    if(gyro_drdy_semaphore == NULL) gyro_drdy_semaphore = xSemaphoreCreateBinary();
    if(accel_drdy_semaphore == NULL) accel_drdy_semaphore = xSemaphoreCreateBinary();

    bmi055_state = BMI055_RESET;

    return rv;
}

void BMI055_AccelChipSelection() {
    GPIO_Reset(&bmi055_accel_cs);
}

void BMI055_GyroChipSelection() {
    GPIO_Reset(&bmi055_gyro_cs);
}

void BMI055_AccelChipDeselection() {
    GPIO_Set(&bmi055_accel_cs);
}

void BMI055_GyroChipDeselection() {
    GPIO_Set(&bmi055_accel_cs);
}

void BMI055_Run() {
    switch(bmi055_state) {

    case BMI055_RESET:
    case BMI055_RESET_WAIT:
    case BMI055_CONF:
    case BMI055_FIFO_READ:
        break;

}
