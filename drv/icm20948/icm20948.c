#include "icm20948.h"
#include "icm20948_reg.h"
#include "init.h"
#include "cfg.h"
#include "timer.h"

static char *device = "ICM20948";

// Data structures
static icm20948_cfg_t icm20948_cfg;
static icm20948_data_t icm20948_fifo;
static enum icm20948_state_t icm20948_state;
static FIFOBuffer_t icm20948_FIFOBuffer;
static FIFOParam_t icm20948_FIFOParam;

// Private functions
static void ICM20948_ChipSelection(void);
static void ICM20948_ChipDeselection(void);
static uint8_t ICM20948_ReadReg(uint8_t reg);
static void ICM20948_WriteReg(uint8_t reg, uint8_t value);
static void ICM20948_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
static int ICM20948_Configure(void);
static void ICM20948_AccelConfigure(void);
static void ICM20948_GyroConfigure(void);
static int ICM20948_Probe(void);
static void ICM20948_Statistics(void);
static void ICM20948_FIFOCount(void);
static int ICM20948_FIFORead(void);
static void ICM20948_FIFOReset(void);
static void ICM20948_AccelProcess(void);
static void ICM20948_GyroProcess(void);
static void ICM20948_TempProcess(void);

// Sync
static int timer_id;
static int timer_status;
static SemaphoreHandle_t drdy_semaphore;
static SemaphoreHandle_t measrdy_semaphore;
static uint32_t attempt = 0;
static TickType_t icm20948_last_sample = 0;

// Public functions
int ICM20948_Init(spi_cfg_t *spi, gpio_cfg_t *cs, exti_cfg_t *drdy, i2c_cfg_t *i2c) {
    if(spi == NULL || cs == NULL) return -1;

    icm20948_cfg.spi = spi;
    icm20948_cfg.cs = cs;
    icm20948_cfg.i2c = i2c;

    if(drdy != NULL) {
        icm20948_cfg.drdy = drdy;
        if(drdy_semaphore == NULL) drdy_semaphore = xSemaphoreCreateBinary();
    }

    if(measrdy_semaphore == NULL) measrdy_semaphore = xSemaphoreCreateBinary();

    timer_id = Timer_Create("ICM20948_Timer");

    xSemaphoreTake(measrdy_semaphore, 0);
    icm20948_state = ICM20948_RESET;

    return 0;
}

void ICM20948_Run(void) {
    switch(icm20948_state) {

    case ICM20948_RESET:
        timer_status = Timer_Start(timer_id, 2000);

        if(timer_status == TIMER_START) {
            ICM20948_WriteReg(PWR_MGMT_1, DEVICE_RESET);
        } else if(timer_status == TIMER_WORK) {
        } else if(timer_status == TIMER_END) {
            icm20948_state = ICM20948_RESET_WAIT;
        }

        break;

    case ICM20948_RESET_WAIT:
        timer_status = Timer_Start(timer_id, 100);
    
        if(timer_status == TIMER_START) {
        } else if(timer_status == TIMER_WORK) {
            if((ICM20948_ReadReg(WHO_AM_I) == WHOAMI) && 
               (ICM20948_ReadReg(PWR_MGMT_1) == 0x41) && 
               (ICM20948_ReadReg(CONFIG) == 0x80)) {
                ICM20948_WriteReg(I2C_IF, I2C_IF_DIS);
                ICM20948_WriteReg(PWR_MGMT_1, CLKSEL_0);
                ICM20948_WriteReg(SIGNAL_PATH_RESET, ACCEL_RST | TEMP_RST);
                ICM20948_SetClearReg(USER_CTRL, SIG_COND_RST, 0);
            } else {
                LOG_ERROR(device, "Wrong default registers values after reset");
                icm20948_state = ICM20948_RESET;
                attempt++;
                if(attempt > 5) {
                    icm20948_state = ICM20948_FAIL;
                    LOG_ERROR(device, "Fatal error");
                    attempt = 0;
                }
            }
        } else if(timer_status == TIMER_END) {
            icm20948_state = ICM20948_CONF;
            LOG_DEBUG(device, "Device available");
        }

        break;

    case ICM20948_CONF:
        if(ICM20948_Configure()) {
            ICM20948_FIFOReset();
            icm20948_last_sample = xTaskGetTickCount();
            ICM20948_FIFOCount();
            ICM20948_FIFORead();
            LOG_DEBUG(device, "Device configured");
            icm20948_state = ICM20948_FIFO_READ;
        } else {
            LOG_ERROR(device, "Failed configuration, retrying...");
            attempt++;
            if(attempt > 5) {
                icm20948_state = ICM20948_RESET;
                LOG_ERROR(device, "Failed configuration, reset...");
                attempt = 0;
            }
        }

        break;

    case ICM20948_FIFO_READ:
        if(drdy_semaphore != NULL) {
            xSemaphoreTake(drdy_semaphore, portMAX_DELAY);
        } else {
            // TODO: Add hardware timer
            vTaskDelay(1);
        }
        ICM20948_FIFOCount();
        ICM20948_FIFORead();
        icm20948_fifo.dt = xTaskGetTickCount() - icm20948_last_sample;
        icm20948_fifo.samples = icm20948_FIFOParam.samples;
        icm20948_last_sample = xTaskGetTickCount();
        xSemaphoreGive(measrdy_semaphore);
        break;

    case ICM20948_FAIL:
        vTaskDelay(1000);
        break;
    }
}

int ICM20948_Operation(void) {
    if(icm20948_state == ICM20948_FIFO_READ) {
        return 0;
    } else {
        return -1;
    }
}

void ICM20948_DataReadyHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&icm20948_cfg.drdy->EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(icm20948_cfg.drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

double ICM20948_Get_ax(void) {
    return (icm20948_fifo.accel_x[0]);
}

double ICM20948_Get_ay(void) {
    return (icm20948_fifo.accel_y[0]);
}

double ICM20948_Get_az(void) {
    return (icm20948_fifo.accel_z[0]);
}

double ICM20948_Get_wx(void) {
    return (icm20948_fifo.gyro_x[0]);
}

double ICM20948_Get_wy(void) {
    return (icm20948_fifo.gyro_y[0]);
}

double ICM20948_Get_wz(void) {
    return (icm20948_fifo.gyro_z[0]);
}

int ICM20948_MeasReady(void) {
    xSemaphoreTake(measrdy_semaphore, portMAX_DELAY);
    return 1;
}

// Private functions
static void ICM20948_ChipSelection(void) {
    GPIO_Reset(icm20948_cfg.cs);
}

static void ICM20948_ChipDeselection(void) {
    GPIO_Set(icm20948_cfg.cs);
}

static uint8_t ICM20948_ReadReg(uint8_t reg) {
    uint8_t cmd = reg | READ;
    uint8_t data;

    ICM20948_ChipSelection();
    SPI_Transmit(icm20948_cfg.spi, &cmd, 1);
    SPI_Receive(icm20948_cfg.spi, &data, 1);
    ICM20948_ChipDeselection();

    return data;
}

static void ICM20948_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    ICM20948_ChipSelection();
    SPI_Transmit(icm20948_cfg.spi, data, 2);
    ICM20948_ChipDeselection();
}

static void ICM20948_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = ICM20948_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        ICM20948_WriteReg(reg, val);
    }
}

static int ICM20948_Configure(void) {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        ICM20948_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
    }

    // Check
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = ICM20948_ReadReg(reg_cfg[i].reg);

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
    ICM20948_AccelConfigure();
    ICM20948_GyroConfigure();

    // Enable EXTI IRQ for DataReady pin
    if(drdy_semaphore != NULL) {
        EXTI_EnableIRQ(icm20948_cfg.drdy);
    }

    return rv;
}

static void ICM20948_AccelConfigure(void) {
    const uint8_t ACCEL_FS_SEL = ICM20948_ReadReg(ACCEL_CONFIG) & (BIT4 | BIT3);

    if(ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        icm20948_cfg.param.accel_scale = (CONST_G / 16384.f);
        icm20948_cfg.param.accel_range = (2.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        icm20948_cfg.param.accel_scale = (CONST_G / 8192.f);
        icm20948_cfg.param.accel_range = (4.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        icm20948_cfg.param.accel_scale = (CONST_G / 4096.f);
        icm20948_cfg.param.accel_range = (8.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        icm20948_cfg.param.accel_scale = (CONST_G / 2048.f);
        icm20948_cfg.param.accel_range = (16.f * CONST_G);
    }
}

static void ICM20948_GyroConfigure(void) {
    const uint8_t FS_SEL = ICM20948_ReadReg(GYRO_CONFIG) & (BIT4 | BIT3);

    if(FS_SEL == FS_SEL_250_DPS) {
        icm20948_cfg.param.gyro_range = 250.f;
    } else if(FS_SEL == FS_SEL_500_DPS) {
        icm20948_cfg.param.gyro_range = 500.f;
    } else if(FS_SEL == FS_SEL_1000_DPS) {
        icm20948_cfg.param.gyro_range = 1000.f;
    } else if(FS_SEL == FS_SEL_2000_DPS) {
        icm20948_cfg.param.gyro_range = 2000.f;
    }

    icm20948_cfg.param.gyro_scale = (icm20948_cfg.param.gyro_range / 32768.f);
}

static void ICM20948_FIFOCount(void) {
    uint8_t cmd = FIFO_COUNTH | READ;
    uint8_t data[2];

    ICM20948_ChipSelection();
    SPI_Transmit(icm20948_cfg.spi, &cmd, 1);
    SPI_Receive(icm20948_cfg.spi, data, 2);
    ICM20948_ChipDeselection();

    icm20948_FIFOParam.samples = msblsb16(data[0], data[1]);
    icm20948_FIFOParam.bytes = icm20948_FIFOParam.samples * sizeof(FIFO_t);
}

static int ICM20948_FIFORead(void) {
    uint8_t cmd = FIFO_COUNTH | READ;

    ICM20948_ChipSelection();
    SPI_Transmit(icm20948_cfg.spi, &cmd, 1);
    SPI_Receive(icm20948_cfg.spi, (uint8_t *)&icm20948_FIFOBuffer,
                icm20948_FIFOParam.bytes);
    ICM20948_ChipDeselection();

    ICM20948_TempProcess();
    ICM20948_AccelProcess();
    ICM20948_GyroProcess();

    if(drdy_semaphore != NULL) {
        EXTI_EnableIRQ(icm20948_cfg.drdy);
    }

    return 0;
}

static void ICM20948_FIFOReset(void) {
    ICM20948_WriteReg(FIFO_EN, 0);
    ICM20948_SetClearReg(USER_CTRL, USR_CTRL_FIFO_RST, USR_CTRL_FIFO_EN);

    for(int i = 0; i < SIZE_REG_CFG; i++) {
        if(reg_cfg[i].reg == FIFO_EN || reg_cfg[i].reg == USER_CTRL) {
            ICM20948_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
        }
    }
}

static void ICM20948_AccelProcess(void) {
    for (int i = 0; i < icm20948_FIFOParam.samples; i++) {
        int16_t accel_x = msblsb16(icm20948_FIFOBuffer.buf[i].ACCEL_XOUT_H,
                                    icm20948_FIFOBuffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msblsb16(icm20948_FIFOBuffer.buf[i].ACCEL_YOUT_H,
                                    icm20948_FIFOBuffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msblsb16(icm20948_FIFOBuffer.buf[i].ACCEL_ZOUT_H,
                                    icm20948_FIFOBuffer.buf[i].ACCEL_ZOUT_L);

        icm20948_fifo.accel_x[i] = accel_x * icm20948_cfg.param.accel_scale;
        icm20948_fifo.accel_y[i] = ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
                                        icm20948_cfg.param.accel_scale;
        icm20948_fifo.accel_z[i] = ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
                                        icm20948_cfg.param.accel_scale;
    }
}

static void ICM20948_GyroProcess(void) {
    for (int i = 0; i < icm20948_FIFOParam.samples; i++) {
        int16_t gyro_x = msblsb16(icm20948_FIFOBuffer.buf[i].GYRO_XOUT_H,
                                    icm20948_FIFOBuffer.buf[i].GYRO_XOUT_L);
        int16_t gyro_y = msblsb16(icm20948_FIFOBuffer.buf[i].GYRO_YOUT_H,
                                    icm20948_FIFOBuffer.buf[i].GYRO_YOUT_L);
        int16_t gyro_z = msblsb16(icm20948_FIFOBuffer.buf[i].GYRO_ZOUT_H,
                                    icm20948_FIFOBuffer.buf[i].GYRO_ZOUT_L);

        icm20948_fifo.gyro_x[i] = gyro_x * icm20948_cfg.param.gyro_scale;
        icm20948_fifo.gyro_y[i] = ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
                                    icm20948_cfg.param.gyro_scale;
        icm20948_fifo.gyro_z[i] = ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
                                    icm20948_cfg.param.gyro_scale;
    }
}

static void ICM20948_TempProcess(void) {
    float temperature_sum = 0;

    for (int i = 0; i < icm20948_FIFOParam.samples; i++) {
        const int16_t t = msblsb16(icm20948_FIFOBuffer.buf[i].TEMP_H,
                                    icm20948_FIFOBuffer.buf[i].TEMP_L);
        temperature_sum += t;
    }

    const float temperature_avg = temperature_sum / icm20948_FIFOParam.samples;
    const float temperature_C = (temperature_avg / TEMP_SENS) + TEMP_OFFSET;

    icm20948_fifo.temp = temperature_C;
}

static int ICM20948_Probe(void) {
    uint8_t whoami;
    whoami = ICM20948_ReadReg(WHO_AM_I);
    if(whoami != WHOAMI) {
        LOG_ERROR(device, "unexpected WHO_AM_I reg 0x%02x\n", whoami);
        return ENODEV;
    }
    return 0;
}

static void ICM20948_Statistics(void) {
    LOG_DEBUG(device, "Statistics:");
    LOG_DEBUG(device, "accel_x = %.3f [m/s2]", icm20948_fifo.accel_x[0]);
    LOG_DEBUG(device, "accel_y = %.3f [m/s2]", icm20948_fifo.accel_y[0]);
    LOG_DEBUG(device, "accel_z = %.3f [m/s2]", icm20948_fifo.accel_z[0]);
    LOG_DEBUG(device, "gyro_x  = %.3f [deg/s]", icm20948_fifo.gyro_x[0]);
    LOG_DEBUG(device, "gyro_y  = %.3f [deg/s]", icm20948_fifo.gyro_y[0]);
    LOG_DEBUG(device, "gyro_z  = %.3f [deg/s]", icm20948_fifo.gyro_z[0]);
    LOG_DEBUG(device, "temp    = %.3f [C]", icm20948_fifo.temp);
    LOG_DEBUG(device, "N       = %lu [samples]", icm20948_fifo.samples);
    LOG_DEBUG(device, "dt      = %lu [ms]", icm20948_fifo.dt);
    LOG_DEBUG(device, "N = %lu", icm20948_fifo.samples);
}
