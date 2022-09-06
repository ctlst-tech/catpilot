#include "icm20689.h"
#include "icm20689_reg.h"
#include "init.h"
#include "cfg.h"

static char *device = "ICM20689";

// Data sturctures
static icm20689_cfg_t icm20689_cfg;
static icm20689_fifo_t icm20689_fifo;
static enum icm20689_state_t icm20689_state;
static FIFOBuffer_t icm20689_FIFOBuffer;
static FIFOParam_t icm20689_FIFOParam;

// Private functions
static void ICM20689_ChipSelection(void);
static void ICM20689_ChipDeselection(void);
static uint8_t ICM20689_ReadReg(uint8_t reg);
static void ICM20689_WriteReg(uint8_t reg, uint8_t value);
static void ICM20689_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
static int ICM20689_Configure(void);
static void ICM20689_AccelConfigure(void);
static void ICM20689_GyroConfigure(void);
static int ICM20689_Probe(void);
static void ICM20689_Statistics(void);
static void ICM20689_FIFOCount(void);
static int ICM20689_FIFORead(void);
static void ICM20689_FIFOReset(void);
static void ICM20689_AccelProcess(void);
static void ICM20689_GyroProcess(void);
static void ICM20689_TempProcess(void);

// Sync
static SemaphoreHandle_t drdy_semaphore;
static SemaphoreHandle_t measrdy_semaphore;
static SemaphoreHandle_t timer_semaphore;
static uint32_t attempt = 0;
static uint32_t t0;
static TickType_t icm20689_last_sample = 0;

// Public functions
int ICM20689_Init(spi_cfg_t *spi, gpio_cfg_t *cs, exti_cfg_t *drdy) {
    if(spi == NULL || cs == NULL) return -1;

    icm20689_cfg.spi = spi;
    icm20689_cfg.cs = cs;
    icm20689_cfg.drdy = drdy;

    if(drdy_semaphore == NULL) drdy_semaphore = xSemaphoreCreateBinary();
    if(measrdy_semaphore == NULL) measrdy_semaphore = xSemaphoreCreateBinary();
    if(timer_semaphore == NULL) timer_semaphore = xSemaphoreCreateBinary();

    xSemaphoreTake(measrdy_semaphore, 0);
    xSemaphoreGive(timer_semaphore);
    icm20689_state = ICM20689_RESET;

    return 0;
}

void ICM20689_Run(void) {
    switch(icm20689_state) {

    case ICM20689_RESET:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            ICM20689_WriteReg(PWR_MGMT_1, DEVICE_RESET);
            t0 = xTaskGetTickCount();
        }
        if(xTaskGetTickCount() - t0 > 2.0) {
            icm20689_state = ICM20689_RESET_WAIT;
            xSemaphoreGive(timer_semaphore);
        }
        break;

    case ICM20689_RESET_WAIT:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            if((ICM20689_ReadReg(WHO_AM_I) == WHOAMI) && 
               (ICM20689_ReadReg(PWR_MGMT_1) == 0x40)) 
            {
                ICM20689_WriteReg(PWR_MGMT_1, CLKSEL_0);
                ICM20689_WriteReg(SIGNAL_PATH_RESET, ACCEL_RST | TEMP_RST);
                ICM20689_SetClearReg(USER_CTRL, SIG_COND_RST | I2C_IF_DIS, 0);
                t0 = xTaskGetTickCount();
            } else {
                LOG_ERROR(device, "Wrong default registers values after reset");
                icm20689_state = ICM20689_RESET;
                xSemaphoreGive(timer_semaphore);
                attempt++;
                if(attempt > 5) {
                    icm20689_state = ICM20689_FAIL;
                    LOG_ERROR(device, "Fatal error");
                    attempt = 0;
                }
            }
        }
        if(xTaskGetTickCount() - t0 > 100) {
            icm20689_state = ICM20689_CONF;
            LOG_DEBUG(device, "Device available");
            xSemaphoreGive(timer_semaphore);
        }
        break;

    case ICM20689_CONF:
        if(xTaskGetTickCount() - t0 > 100) {
            if(xSemaphoreTake(timer_semaphore, 0)) t0 = xTaskGetTickCount();
            if(ICM20689_Configure()) {
                icm20689_state = ICM20689_FIFO_READ;
                ICM20689_FIFOReset();
                icm20689_last_sample = xTaskGetTickCount();
                LOG_DEBUG(device, "Device configured");
                ICM20689_FIFOCount();
                ICM20689_FIFORead();
                xSemaphoreGive(timer_semaphore);
            } else {
                LOG_ERROR(device, "Failed configuration, retrying...");
                t0 = xTaskGetTickCount();
                attempt++;
                if(attempt > 5) {
                    icm20689_state = ICM20689_RESET;
                    LOG_ERROR(device, "Failed configuration, reset...");
                    attempt = 0;
                    xSemaphoreGive(timer_semaphore);
                }
            }
        }
        break;

    case ICM20689_FIFO_READ:
        if(icm20689_cfg.drdy != NULL) {
            xSemaphoreTake(drdy_semaphore, portMAX_DELAY);
        } else {
            vTaskDelay(1);
        }
        ICM20689_FIFOCount();
        ICM20689_FIFORead();
        icm20689_fifo.dt = xTaskGetTickCount() - icm20689_last_sample;
        icm20689_fifo.samples = icm20689_FIFOParam.samples;
        icm20689_last_sample = xTaskGetTickCount();
        xSemaphoreGive(measrdy_semaphore);
        break;

    case ICM20689_FAIL:
        vTaskDelay(1000);
        break;
    }
}

int ICM20689_Operation(void) {
    if(icm20689_state == ICM20689_FIFO_READ) {
        return 0;
    } else {
        return -1;
    }
}

void ICM20689_DataReadyHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&icm20689_cfg.drdy->EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(icm20689_cfg.drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

double ICM20689_Get_ax(void) {
    return (icm20689_fifo.accel_x[0]);
}

double ICM20689_Get_ay(void) {
    return (icm20689_fifo.accel_y[0]);
}

double ICM20689_Get_az(void) {
    return (icm20689_fifo.accel_z[0]);
}

double ICM20689_Get_wx(void) {
    return (icm20689_fifo.gyro_x[0]);
}

double ICM20689_Get_wy(void) {
    return (icm20689_fifo.gyro_y[0]);
}

double ICM20689_Get_wz(void) {
    return (icm20689_fifo.gyro_z[0]);
}

int ICM20689_MeasReady(void) {
    xSemaphoreTake(measrdy_semaphore, portMAX_DELAY);
    return 1;
}

// Private functions
static void ICM20689_ChipSelection(void) {
    GPIO_Reset(icm20689_cfg.cs);
}

static void ICM20689_ChipDeselection(void) {
    GPIO_Set(icm20689_cfg.cs);
}

static uint8_t ICM20689_ReadReg(uint8_t reg) {
    uint8_t cmd = reg | READ;
    uint8_t data;

    ICM20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, &cmd, 1);
    SPI_Receive(icm20689_cfg.spi, &data, 1);
    ICM20689_ChipDeselection();

    return data;
}

static void ICM20689_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    ICM20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, data, 2);
    ICM20689_ChipDeselection();
}

static void ICM20689_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = ICM20689_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        ICM20689_WriteReg(reg, val);
    }
}

static int ICM20689_Configure(void) {
    uint8_t orig_val;
    int rv = 1;

    // Enable EXTI IRQ for DataReady pin
    if(icm20689_cfg.drdy != NULL) {
        EXTI_EnableIRQ(icm20689_cfg.drdy);
    }

    // Set configure
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        ICM20689_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
    }

    // Check
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = ICM20689_ReadReg(reg_cfg[i].reg);

        if((orig_val & reg_cfg[i].setbits) != reg_cfg[i].setbits) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not set)",
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not cleared)",
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set scale and range for processing
    ICM20689_AccelConfigure();
    ICM20689_GyroConfigure();

    return rv;
}

static void ICM20689_AccelConfigure(void) {
    const uint8_t ACCEL_FS_SEL = ICM20689_ReadReg(ACCEL_CONFIG) & (BIT4 | BIT3);

    if(ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 16384.f);
        icm20689_cfg.param.accel_range = (2.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 8192.f);
        icm20689_cfg.param.accel_range = (4.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 4096.f);
        icm20689_cfg.param.accel_range = (8.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        icm20689_cfg.param.accel_scale = (CONST_G / 2048.f);
        icm20689_cfg.param.accel_range = (16.f * CONST_G);
    }
}

static void ICM20689_GyroConfigure(void) {
    const uint8_t FS_SEL = ICM20689_ReadReg(GYRO_CONFIG) & (BIT4 | BIT3);

    if(FS_SEL == FS_SEL_250_DPS) {
        icm20689_cfg.param.gyro_range = 250.f;
    } else if(FS_SEL == FS_SEL_500_DPS) {
        icm20689_cfg.param.gyro_range = 500.f;
    } else if(FS_SEL == FS_SEL_1000_DPS) {
        icm20689_cfg.param.gyro_range = 1000.f;
    } else if(FS_SEL == FS_SEL_2000_DPS) {
        icm20689_cfg.param.gyro_range = 2000.f;
    }

    icm20689_cfg.param.gyro_scale = (icm20689_cfg.param.gyro_range / 32768.f);
}

static void ICM20689_FIFOCount(void) {
    uint8_t cmd = FIFO_COUNTH | READ;
    uint8_t data[2];

    ICM20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, &cmd, 1);
    SPI_Receive(icm20689_cfg.spi, data, 2);
    ICM20689_ChipDeselection();

    icm20689_FIFOParam.bytes = msblsb16(data[0], data[1]);
    icm20689_FIFOParam.samples = icm20689_FIFOParam.bytes / sizeof(FIFO_t);
}

static int ICM20689_FIFORead(void) {
    uint8_t cmd = FIFO_COUNTH | READ;

    ICM20689_ChipSelection();
    SPI_Transmit(icm20689_cfg.spi, &cmd, 1);
    SPI_Receive(icm20689_cfg.spi, (uint8_t *)&icm20689_FIFOBuffer,
                icm20689_FIFOParam.bytes);
    ICM20689_ChipDeselection();

    ICM20689_TempProcess();
    ICM20689_AccelProcess();
    ICM20689_GyroProcess();

    if(icm20689_cfg.drdy != NULL) {
        EXTI_EnableIRQ(icm20689_cfg.drdy);
    }

    return 0;
}

static void ICM20689_FIFOReset(void) {
    ICM20689_WriteReg(FIFO_EN, 0);
    ICM20689_SetClearReg(USER_CTRL, USR_CTRL_FIFO_RST, USR_CTRL_FIFO_EN);

    for(int i = 0; i < SIZE_REG_CFG; i++) {
        if(reg_cfg[i].reg == FIFO_EN || reg_cfg[i].reg == USER_CTRL) {
            ICM20689_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
        }
    }
}

static void ICM20689_AccelProcess(void) {
    for (int i = 0; i < icm20689_FIFOParam.samples; i++) {
        int16_t accel_x = msblsb16(icm20689_FIFOBuffer.buf[i].ACCEL_XOUT_H,
                                    icm20689_FIFOBuffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msblsb16(icm20689_FIFOBuffer.buf[i].ACCEL_YOUT_H,
                                    icm20689_FIFOBuffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msblsb16(icm20689_FIFOBuffer.buf[i].ACCEL_ZOUT_H,
                                    icm20689_FIFOBuffer.buf[i].ACCEL_ZOUT_L);

        icm20689_fifo.accel_x[i] = accel_x * icm20689_cfg.param.accel_scale;
        icm20689_fifo.accel_y[i] = ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
                                        icm20689_cfg.param.accel_scale;
        icm20689_fifo.accel_z[i] = ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
                                        icm20689_cfg.param.accel_scale;
    }
}

static void ICM20689_GyroProcess(void) {
    for (int i = 0; i < icm20689_FIFOParam.samples; i++) {
        int16_t gyro_x = msblsb16(icm20689_FIFOBuffer.buf[i].GYRO_XOUT_H,
                                    icm20689_FIFOBuffer.buf[i].GYRO_XOUT_L);
        int16_t gyro_y = msblsb16(icm20689_FIFOBuffer.buf[i].GYRO_YOUT_H,
                                    icm20689_FIFOBuffer.buf[i].GYRO_YOUT_L);
        int16_t gyro_z = msblsb16(icm20689_FIFOBuffer.buf[i].GYRO_ZOUT_H,
                                    icm20689_FIFOBuffer.buf[i].GYRO_ZOUT_L);

        icm20689_fifo.gyro_x[i] = gyro_x * icm20689_cfg.param.gyro_scale;
        icm20689_fifo.gyro_y[i] = ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
                                    icm20689_cfg.param.gyro_scale;
        icm20689_fifo.gyro_z[i] = ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
                                    icm20689_cfg.param.gyro_scale;
    }
}

static void ICM20689_TempProcess(void) {
    float temperature_sum = 0;

    for (int i = 0; i < icm20689_FIFOParam.samples; i++) {
        const int16_t t = msblsb16(icm20689_FIFOBuffer.buf[i].TEMP_H,
                                    icm20689_FIFOBuffer.buf[i].TEMP_L);
        temperature_sum += t;
    }

    const float temperature_avg = temperature_sum / icm20689_FIFOParam.samples;
    const float temperature_C = (temperature_avg / TEMP_SENS) + TEMP_OFFSET;

    icm20689_fifo.temp = temperature_C;
}

static int ICM20689_Probe(void) {
    uint8_t whoami;
    whoami = ICM20689_ReadReg(WHO_AM_I);
    if(whoami != WHOAMI) {
        LOG_ERROR(device, "unexpected WHO_AM_I reg 0x%02x", whoami);
        return ENODEV;
    }
    return 0;
}

static void ICM20689_Statistics(void) {
    LOG_DEBUG(device, "Statistics:");
    LOG_DEBUG(device, "accel_x = %.3f [m/s2]", icm20689_fifo.accel_x[0]);
    LOG_DEBUG(device, "accel_y = %.3f [m/s2]", icm20689_fifo.accel_y[0]);
    LOG_DEBUG(device, "accel_z = %.3f [m/s2]", icm20689_fifo.accel_z[0]);
    LOG_DEBUG(device, "gyro_x  = %.3f [deg/s]", icm20689_fifo.gyro_x[0]);
    LOG_DEBUG(device, "gyro_y  = %.3f [deg/s]", icm20689_fifo.gyro_y[0]);
    LOG_DEBUG(device, "gyro_z  = %.3f [deg/s]", icm20689_fifo.gyro_z[0]);
    LOG_DEBUG(device, "temp    = %.3f [C]", icm20689_fifo.temp);
    LOG_DEBUG(device, "N       = %lu [samples]", icm20689_fifo.samples);
    LOG_DEBUG(device, "N = %lu", icm20689_fifo.samples);
    LOG_DEBUG(device, "dt      = %lu [ms]", icm20689_fifo.dt);
}
