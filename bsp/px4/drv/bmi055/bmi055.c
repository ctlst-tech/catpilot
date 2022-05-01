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

uint8_t BMI055_ReadReg(uint8_t reg, uint8_t mode);
void BMI055_WriteReg(uint8_t reg, uint8_t value, uint8_t mode);
void BMI055_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits, uint8_t mode);
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

void BMI055_ChipSelection(uint8_t mode) {
    if(mode == BMI055_ACCEL_MODE) {
        GPIO_Reset(&bmi055_accel_cs);
    } else if (mode == BMI055_GYRO_MODE) {
        GPIO_Reset(&bmi055_gyro_cs);
    }
}

void BMI055_ChipDeselection(uint8_t mode) {
    if(mode == BMI055_ACCEL_MODE) {
        GPIO_Set(&bmi055_accel_cs);
    } else if (mode == BMI055_GYRO_MODE) {
        GPIO_Set(&bmi055_gyro_cs);
    }
}

void BMI055_Run() {
    switch(bmi055_state) {

    case BMI055_RESET:
    case BMI055_RESET_WAIT:
    case BMI055_CONF:
    case BMI055_FIFO_READ:
        break;

    }
}

uint8_t BMI055_ReadReg(uint8_t reg, uint8_t mode) {
    uint8_t cmd = reg | READ;
    uint8_t data;

    BMI055_ChipSelection(mode);
    SPI_Transmit(bmi055_cfg.spi, &cmd, 1);
    SPI_Receive(bmi055_cfg.spi, &data, 1);
    BMI055_ChipDeselection(mode);

    return data;
}

void BMI055_WriteReg(uint8_t reg, uint8_t value, uint8_t mode) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    BMI055_ChipSelection(mode);
    SPI_Transmit(bmi055_cfg.spi, data, 2);
    BMI055_ChipDeselection(mode);
}

void BMI055_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits, uint8_t mode) {
    uint8_t orig_val = BMI055_ReadReg(reg, mode);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        BMI055_WriteReg(reg, val, mode);
    }
}

int BMI055_Probe() {
    uint8_t bgwchipid;
    bgwchipid = BMI055_ReadReg(ACCEL_BGW_CHIP_ID, BMI055_ACCEL_MODE);
    if(bgwchipid != ACCEL_CHIP_ID_VALUE) {
        printf("%s: unexpected BGW_CHIP_ID reg 0x%02x\n", device, bgwchipid);
        return ENODEV;
    }
    return 0;
}

void BMI055_Statistics() {
    // TODO add time between FIFO reading
    printf("%s: Statistics:\n", device);
    printf("accel_x = %.3f [m/s2]\n", bmi055_fifo.accel_x[0]);
    printf("accel_y = %.3f [m/s2]\n", bmi055_fifo.accel_y[0]);
    printf("accel_z = %.3f [m/s2]\n", bmi055_fifo.accel_z[0]);
    printf("gyro_x  = %.3f [deg/s]\n", bmi055_fifo.gyro_x[0]);
    printf("gyro_y  = %.3f [deg/s]\n", bmi055_fifo.gyro_y[0]);
    printf("gyro_z  = %.3f [deg/s]\n", bmi055_fifo.gyro_z[0]);
    printf("temp    = %.3f [C]\n", bmi055_fifo.temp);
    printf("N       = %lu [samples]\n", bmi055_fifo.samples);
    printf("dt      = %lu [ms]\n", bmi055_fifo.dt);
}

void BMI055_AccelDataReadyHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(accel_drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&bmi055_accel_drdy.EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(&bmi055_accel_drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void BMI055_GyroDataReadyHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(gyro_drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&bmi055_gyro_drdy.EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(&bmi055_gyro_drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void EXTI15_10_IRQHandler(void) {
    uint32_t accel_drdy_pin = bmi055_accel_drdy.gpio.GPIO_InitStruct.Pin;
    uint32_t gyro_drdy_pin = bmi055_gyro_drdy.gpio.GPIO_InitStruct.Pin;

    uint32_t accel_line = (EXTI->PR) & accel_drdy_pin;
    uint32_t gyro_line = (EXTI->PR) & gyro_drdy_pin;

    if(accel_line) {
        BMI055_AccelDataReadyHandler();
    }

    if(gyro_line) {
        BMI055_GyroDataReadyHandler();
    }
}
