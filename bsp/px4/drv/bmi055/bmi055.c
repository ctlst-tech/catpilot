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

void BMI055_Run() {

}
