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
void BMI055_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits,
                                                            uint8_t mode);
int BMI055_AccelConfigure();
int BMI055_GyroConfigure();
int BMI055_Probe();
void BMI055_Statistics();
void BMI055_FIFOCount();
int BMI055_FIFORead();
void BMI055_FIFOReset();
void BMI055_AccelProcess();
void BMI055_GyroProcess();
void BMI055_TempProcess();

static gpio_cfg_t *bmi055_gyro_cs = &gpio_spi1_cs3;
static gpio_cfg_t *bmi055_accel_cs = &gpio_spi1_cs4;
static exti_cfg_t *bmi055_gyro_drdy = &exti_spi1_drdy3;
static exti_cfg_t *bmi055_accel_drdy = &exti_spi1_drdy4;
static SemaphoreHandle_t gyro_drdy_semaphore;
static SemaphoreHandle_t accel_drdy_semaphore;

static SemaphoreHandle_t timer_semaphore;
static uint32_t t0;

static uint32_t attempt = 0;

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

    if(gyro_drdy_semaphore == NULL) {
        gyro_drdy_semaphore = xSemaphoreCreateBinary();
    }
    if(accel_drdy_semaphore == NULL) {
        accel_drdy_semaphore = xSemaphoreCreateBinary();
    }

    if(timer_semaphore == NULL) timer_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(timer_semaphore);

    bmi055_state = BMI055_RESET;

    return rv;
}

void BMI055_ChipSelection(uint8_t mode) {
    if(mode == ACCEL_MODE) {
        GPIO_Reset(bmi055_accel_cs);
    } else if (mode == GYRO_MODE) {
        GPIO_Reset(bmi055_gyro_cs);
    }
}

void BMI055_ChipDeselection(uint8_t mode) {
    if(mode == ACCEL_MODE) {
        GPIO_Set(bmi055_accel_cs);
    } else if (mode == GYRO_MODE) {
        GPIO_Set(bmi055_gyro_cs);
    }
}
void BMI055_Run() {
    switch(bmi055_state) {

    case BMI055_RESET:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            BMI055_WriteReg(ACCEL_BGW_SOFTRESET, BGW_RESET_VALUE, ACCEL_MODE);
            BMI055_WriteReg(GYRO_BGW_SOFTRESET, BGW_RESET_VALUE, GYRO_MODE);
            t0 = xTaskGetTickCount();
        }
        if(xTaskGetTickCount() - t0 > 25.0) {
            bmi055_state = BMI055_RESET_WAIT;
            xSemaphoreGive(timer_semaphore);
        }
        break;

    case BMI055_RESET_WAIT:
        if(xTaskGetTickCount() - t0 > 10) {
            if(xSemaphoreTake(timer_semaphore, 0)) t0 = xTaskGetTickCount();
            uint8_t accel_id = BMI055_ReadReg(ACCEL_BGW_CHIP_ID, ACCEL_MODE);
            uint8_t gyro_id = BMI055_ReadReg(GYRO_CHIP_ID, GYRO_MODE);
            if(accel_id == ACCEL_CHIP_ID_VALUE && gyro_id == GYRO_CHIP_ID_VALUE) {
                bmi055_state = BMI055_CONF;
                LOG_DEBUG(device, "Device available");
                xSemaphoreGive(timer_semaphore);
            } else {
                LOG_ERROR(device, "Failed ID, retrying...");
                t0 = xTaskGetTickCount();
                attempt++;
                if(attempt > 5) {
                    bmi055_state = BMI055_RESET;
                    LOG_ERROR(device, "Failed ID, reset...");
                    attempt = 0;
                    xSemaphoreGive(timer_semaphore);
                }
            }
        }
        break;

    case BMI055_CONF:
        if(xTaskGetTickCount() - t0 > 100) {
            if(xSemaphoreTake(timer_semaphore, 0)) t0 = xTaskGetTickCount();
            uint8_t accel_conf = BMI055_GyroConfigure();
            uint8_t gyro_conf = BMI055_AccelConfigure();
            if(accel_conf && gyro_conf) {
                bmi055_state = BMI055_FIFO_READ;
                LOG_DEBUG(device, "Device configured");
                xSemaphoreGive(timer_semaphore);
            } else {
                LOG_ERROR(device, "Failed configuration, retrying...");
                t0 = xTaskGetTickCount();
                attempt++;
                if(attempt > 5) {
                    bmi055_state = BMI055_RESET;
                    LOG_ERROR(device, "Failed configuration, reset...");
                    attempt = 0;
                    xSemaphoreGive(timer_semaphore);
                }
            }
        }
        break;

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

void BMI055_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits,
                                                            uint8_t mode) {
    uint8_t orig_val = BMI055_ReadReg(reg, mode);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        BMI055_WriteReg(reg, val, mode);
    }
}

int BMI055_GyroConfigure() {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < GYRO_SIZE_REG_CFG; i++) {
        BMI055_SetClearReg(gyro_reg_cfg[i].reg, gyro_reg_cfg[i].setbits, gyro_reg_cfg[i].clearbits, GYRO_MODE);
    }

    // Check
    for(int i = 0; i < GYRO_SIZE_REG_CFG; i++) {
        orig_val = BMI055_ReadReg(gyro_reg_cfg[i].reg, GYRO_MODE);

        if((orig_val & gyro_reg_cfg[i].setbits) != gyro_reg_cfg[i].setbits) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not set)",
            (uint8_t)gyro_reg_cfg[i].reg, orig_val, gyro_reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & gyro_reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not cleared)",
            (uint8_t)gyro_reg_cfg[i].reg, orig_val, gyro_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    return rv;
}

int BMI055_AccelConfigure() {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < ACCEL_SIZE_REG_CFG; i++) {
        BMI055_SetClearReg(accel_reg_cfg[i].reg, accel_reg_cfg[i].setbits, accel_reg_cfg[i].clearbits, ACCEL_MODE);
    }

    // Check
    for(int i = 0; i < ACCEL_SIZE_REG_CFG; i++) {
        orig_val = BMI055_ReadReg(accel_reg_cfg[i].reg, ACCEL_MODE);

        if((orig_val & accel_reg_cfg[i].setbits) != accel_reg_cfg[i].setbits) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not set)",
            (uint8_t)accel_reg_cfg[i].reg, orig_val, accel_reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & accel_reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not cleared)",
            (uint8_t)accel_reg_cfg[i].reg, orig_val, accel_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    return rv;
}

void BMI055_UpdateTemperature() {
    float temp = (int8_t) BMI055_ReadReg(ACCEL_ACCD_TEMP, ACCEL_MODE)
                                                                * 0.5f + 23.f;
    bmi055_fifo.temp = temp;
}

int BMI055_Probe() {
    uint8_t bgwchipid;
    bgwchipid = BMI055_ReadReg(ACCEL_BGW_CHIP_ID, ACCEL_MODE);
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

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&bmi055_accel_drdy->EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(bmi055_accel_drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void BMI055_GyroDataReadyHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(gyro_drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&bmi055_gyro_drdy->EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(bmi055_gyro_drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void EXTI15_10_IRQHandler(void) {
    uint32_t accel_drdy_pin = bmi055_accel_drdy->gpio.GPIO_InitStruct.Pin;
    uint32_t gyro_drdy_pin = bmi055_gyro_drdy->gpio.GPIO_InitStruct.Pin;

    uint32_t accel_line = (EXTI->PR) & accel_drdy_pin;
    uint32_t gyro_line = (EXTI->PR) & gyro_drdy_pin;

    if(accel_line) {
        BMI055_AccelDataReadyHandler();
    }

    if(gyro_line) {
        BMI055_GyroDataReadyHandler();
    }
}
