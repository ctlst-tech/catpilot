#include "icm20602.h"
#include "icm20602_reg.h"
#include "init.h"
#include "cfg.h"
#include "timer.h"

static char *device = "ICM20602";

// Data structures
static icm20602_cfg_t icm20602_cfg;
static icm20602_fifo_t icm20602_fifo;
static enum icm20602_state_t icm20602_state;
static FIFOBuffer_t icm20602_FIFOBuffer;
static FIFOParam_t icm20602_FIFOParam;

// Private functions
static void ICM20602_ChipSelection(void);
static void ICM20602_ChipDeselection(void);
static uint8_t ICM20602_ReadReg(uint8_t reg);
static void ICM20602_WriteReg(uint8_t reg, uint8_t value);
static void ICM20602_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
static int ICM20602_Configure(void);
static void ICM20602_AccelConfigure(void);
static void ICM20602_GyroConfigure(void);
static int ICM20602_Probe(void);
static void ICM20602_Statistics(void);
static void ICM20602_FIFOCount(void);
static int ICM20602_FIFORead(void);
static void ICM20602_FIFOReset(void);
static void ICM20602_AccelProcess(void);
static void ICM20602_GyroProcess(void);
static void ICM20602_TempProcess(void);

// Sync
static int timer_id;
static int timer_status;
static SemaphoreHandle_t drdy_semaphore;
static SemaphoreHandle_t measrdy_semaphore;
static uint32_t attempt = 0;
static TickType_t icm20602_last_sample = 0;

// Public functions
int ICM20602_Init(spi_cfg_t *spi, gpio_cfg_t *cs, exti_cfg_t *drdy) {
    if(spi == NULL || cs == NULL) return -1;

    icm20602_cfg.spi = spi;
    icm20602_cfg.cs = cs;

    if(drdy != NULL) {
        icm20602_cfg.drdy = drdy;
        if(drdy_semaphore == NULL) drdy_semaphore = xSemaphoreCreateBinary();
    }

    if(measrdy_semaphore == NULL) measrdy_semaphore = xSemaphoreCreateBinary();

    timer_id = Timer_Create("ICM20602_Timer");

    xSemaphoreTake(measrdy_semaphore, 0);
    icm20602_state = ICM20602_RESET;

    return 0;
}

static int ind = 0;
static void _debug(void) {
    static int all = 0;
    static int error = 0;
    all += icm20602_fifo.samples;
    // if(xTaskGetTickCount() > 60000) {
    //         vTaskDelay(100);
    // }
}

void ICM20602_Run(void) {
    switch(icm20602_state) {

    case ICM20602_RESET:
        timer_status = Timer_Start(timer_id, 2000);

        if(timer_status == TIMER_STARTED) {
            ICM20602_WriteReg(PWR_MGMT_1, DEVICE_RESET);
        } else if(timer_status == TIMER_COUNTING) {
        } else if(timer_status == TIMER_FINISHED) {
            icm20602_state = ICM20602_RESET_WAIT;
        }

        break;

    case ICM20602_RESET_WAIT:
        timer_status = Timer_Start(timer_id, 100);
    
        if(timer_status == TIMER_STARTED) {
            if((ICM20602_ReadReg(WHO_AM_I) == WHOAMI) && 
               (ICM20602_ReadReg(PWR_MGMT_1) == 0x41) && 
               (ICM20602_ReadReg(CONFIG) == 0x80)) {
                ICM20602_WriteReg(I2C_IF, I2C_IF_DIS);
                ICM20602_WriteReg(PWR_MGMT_1, CLKSEL_0);
                ICM20602_WriteReg(SIGNAL_PATH_RESET, ACCEL_RST | TEMP_RST);
                ICM20602_SetClearReg(USER_CTRL, SIG_COND_RST, 0);
            } else {
                Timer_Stop(timer_id);
                LOG_ERROR(device, "Wrong default registers values after reset");
                icm20602_state = ICM20602_RESET;
                attempt++;
                if(attempt > 5) {
                    icm20602_state = ICM20602_FAIL;
                    LOG_ERROR(device, "Fatal error");
                    attempt = 0;
                }
            }
        } else if(timer_status == TIMER_COUNTING) {
        } else if(timer_status == TIMER_FINISHED) {
            icm20602_state = ICM20602_CONF;
            LOG_DEBUG(device, "Device available");
        }

        break;

    case ICM20602_CONF:
        if(ICM20602_Configure()) {
            ICM20602_FIFOReset();
            icm20602_last_sample = xTaskGetTickCount();
            ICM20602_FIFOCount();
            ICM20602_FIFORead();
            LOG_DEBUG(device, "Device configured");
            icm20602_state = ICM20602_FIFO_READ;
        } else {
            LOG_ERROR(device, "Failed configuration, retrying...");
            attempt++;
            if(attempt > 5) {
                icm20602_state = ICM20602_RESET;
                LOG_ERROR(device, "Failed configuration, reset...");
                attempt = 0;
            }
        }

        break;

    case ICM20602_FIFO_READ:
        if(drdy_semaphore != NULL) {
            xSemaphoreTake(drdy_semaphore, portMAX_DELAY);
        }
        ICM20602_FIFOCount();
        ICM20602_FIFORead();
        icm20602_fifo.dt = xTaskGetTickCount() - icm20602_last_sample;
        icm20602_fifo.samples = icm20602_FIFOParam.samples;
        icm20602_last_sample = xTaskGetTickCount();
        xSemaphoreGive(measrdy_semaphore);
        _debug();
        break;

    case ICM20602_FAIL:
        break;
    }
}

int ICM20602_Operation(void) {
    if(icm20602_state == ICM20602_FIFO_READ) {
        return 0;
    } else {
        return -1;
    }
}

void ICM20602_DataReadyHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&icm20602_cfg.drdy->EXTI_Handle,
                            EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(icm20602_cfg.drdy);

    if(xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

double ICM20602_Get_ax(void) {
    return (icm20602_fifo.accel_x[0]);
}

double ICM20602_Get_ay(void) {
    return (icm20602_fifo.accel_y[0]);
}

double ICM20602_Get_az(void) {
    return (icm20602_fifo.accel_z[0]);
}

double ICM20602_Get_wx(void) {
    return (icm20602_fifo.gyro_x[0]);
}

double ICM20602_Get_wy(void) {
    return (icm20602_fifo.gyro_y[0]);
}

double ICM20602_Get_wz(void) {
    return (icm20602_fifo.gyro_z[0]);
}

int ICM20602_MeasReady(void) {
    xSemaphoreTake(measrdy_semaphore, portMAX_DELAY);
    return 1;
}

// Private functions
static void ICM20602_ChipSelection(void) {
    SPI_ChipSelect(icm20602_cfg.spi, icm20602_cfg.cs);
}

static void ICM20602_ChipDeselection(void) {
    SPI_ChipDeselect(icm20602_cfg.spi, icm20602_cfg.cs);
}

static uint8_t ICM20602_ReadReg(uint8_t reg) {
    uint8_t cmd = reg | READ;
    uint8_t data;

    ICM20602_ChipSelection();
    SPI_Transmit(icm20602_cfg.spi, &cmd, 1);
    SPI_Receive(icm20602_cfg.spi, &data, 1);
    ICM20602_ChipDeselection();

    return data;
}

static void ICM20602_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;

    ICM20602_ChipSelection();
    SPI_Transmit(icm20602_cfg.spi, data, 2);
    ICM20602_ChipDeselection();
}

static void ICM20602_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = ICM20602_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        ICM20602_WriteReg(reg, val);
    }
}

static int ICM20602_Configure(void) {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        ICM20602_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
    }

    // Check
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = ICM20602_ReadReg(reg_cfg[i].reg);

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
    ICM20602_AccelConfigure();
    ICM20602_GyroConfigure();

    // Enable EXTI IRQ for DataReady pin
    if(drdy_semaphore != NULL) {
        EXTI_EnableIRQ(icm20602_cfg.drdy);
    }

    return rv;
}

static void ICM20602_AccelConfigure(void) {
    const uint8_t ACCEL_FS_SEL = ICM20602_ReadReg(ACCEL_CONFIG) & (BIT4 | BIT3);

    if(ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 16384.f);
        icm20602_cfg.param.accel_range = (2.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 8192.f);
        icm20602_cfg.param.accel_range = (4.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 4096.f);
        icm20602_cfg.param.accel_range = (8.f * CONST_G);
    } else if(ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        icm20602_cfg.param.accel_scale = (CONST_G / 2048.f);
        icm20602_cfg.param.accel_range = (16.f * CONST_G);
    }
}

static void ICM20602_GyroConfigure(void) {
    const uint8_t FS_SEL = ICM20602_ReadReg(GYRO_CONFIG) & (BIT4 | BIT3);

    if(FS_SEL == FS_SEL_250_DPS) {
        icm20602_cfg.param.gyro_range = 250.f;
    } else if(FS_SEL == FS_SEL_500_DPS) {
        icm20602_cfg.param.gyro_range = 500.f;
    } else if(FS_SEL == FS_SEL_1000_DPS) {
        icm20602_cfg.param.gyro_range = 1000.f;
    } else if(FS_SEL == FS_SEL_2000_DPS) {
        icm20602_cfg.param.gyro_range = 2000.f;
    }

    icm20602_cfg.param.gyro_scale = (icm20602_cfg.param.gyro_range / 32768.f);
}

static void ICM20602_FIFOCount(void) {
    icm20602_FIFOBuffer.CMD = FIFO_COUNTH | READ;

    ICM20602_ChipSelection();
    SPI_TransmitReceive(icm20602_cfg.spi, 
                        (uint8_t *)&icm20602_FIFOBuffer, 
                        (uint8_t *)&icm20602_FIFOBuffer,
                        3);
    ICM20602_ChipDeselection();
    icm20602_FIFOParam.bytes = msblsb16(icm20602_FIFOBuffer.COUNTH, 
                                        icm20602_FIFOBuffer.COUNTL);
    icm20602_FIFOParam.samples = icm20602_FIFOParam.bytes / sizeof(FIFO_t);
}

static int ICM20602_FIFORead(void) {
    icm20602_FIFOBuffer.CMD = FIFO_COUNTH | READ;

    ICM20602_ChipSelection();
    SPI_TransmitReceive(icm20602_cfg.spi, 
                        (uint8_t *)&icm20602_FIFOBuffer, 
                        (uint8_t *)&icm20602_FIFOBuffer,
                        icm20602_FIFOParam.bytes + 3);
    ICM20602_ChipDeselection();

    ICM20602_TempProcess();
    ICM20602_AccelProcess();
    ICM20602_GyroProcess();

    if(drdy_semaphore != NULL) {
        EXTI_EnableIRQ(icm20602_cfg.drdy);
    }

    return 0;
}

static void ICM20602_FIFOReset(void) {
    ICM20602_WriteReg(FIFO_EN, 0);
    ICM20602_SetClearReg(USER_CTRL, USR_CTRL_FIFO_RST, USR_CTRL_FIFO_EN);

    for(int i = 0; i < SIZE_REG_CFG; i++) {
        if(reg_cfg[i].reg == FIFO_EN || reg_cfg[i].reg == USER_CTRL) {
            ICM20602_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
        }
    }
}

static void ICM20602_AccelProcess(void) {
    for (int i = 0; i < icm20602_FIFOParam.samples; i++) {
        int16_t accel_x = msblsb16(icm20602_FIFOBuffer.buf[i].ACCEL_XOUT_H,
                                    icm20602_FIFOBuffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msblsb16(icm20602_FIFOBuffer.buf[i].ACCEL_YOUT_H,
                                    icm20602_FIFOBuffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msblsb16(icm20602_FIFOBuffer.buf[i].ACCEL_ZOUT_H,
                                    icm20602_FIFOBuffer.buf[i].ACCEL_ZOUT_L);

        icm20602_fifo.accel_x[i] = accel_x * icm20602_cfg.param.accel_scale;
        icm20602_fifo.accel_y[i] = ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
                                        icm20602_cfg.param.accel_scale;
        icm20602_fifo.accel_z[i] = ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
                                        icm20602_cfg.param.accel_scale;
        ind += (fabs(icm20602_fifo.accel_x[i]) > 12 ? 1 : 0);
        ind += (fabs(icm20602_fifo.accel_y[i]) > 12 ? 1 : 0);
        ind += (fabs(icm20602_fifo.accel_z[i]) > 12 ? 1 : 0);
    }

}

static void ICM20602_GyroProcess(void) {
    for (int i = 0; i < icm20602_FIFOParam.samples; i++) {
        int16_t gyro_x = msblsb16(icm20602_FIFOBuffer.buf[i].GYRO_XOUT_H,
                                    icm20602_FIFOBuffer.buf[i].GYRO_XOUT_L);
        int16_t gyro_y = msblsb16(icm20602_FIFOBuffer.buf[i].GYRO_YOUT_H,
                                    icm20602_FIFOBuffer.buf[i].GYRO_YOUT_L);
        int16_t gyro_z = msblsb16(icm20602_FIFOBuffer.buf[i].GYRO_ZOUT_H,
                                    icm20602_FIFOBuffer.buf[i].GYRO_ZOUT_L);

        icm20602_fifo.gyro_x[i] = gyro_x * icm20602_cfg.param.gyro_scale;
        icm20602_fifo.gyro_y[i] = ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
                                    icm20602_cfg.param.gyro_scale;
        icm20602_fifo.gyro_z[i] = ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
                                    icm20602_cfg.param.gyro_scale;
        ind += (fabs(icm20602_fifo.gyro_x[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20602_fifo.gyro_y[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20602_fifo.gyro_z[i]) > 10 ? 1 : 0);
    }
}

static void ICM20602_TempProcess(void) {
    float temperature_sum = 0;

    for (int i = 0; i < icm20602_FIFOParam.samples; i++) {
        const int16_t t = msblsb16(icm20602_FIFOBuffer.buf[i].TEMP_H,
                                    icm20602_FIFOBuffer.buf[i].TEMP_L);
        temperature_sum += t;
    }

    const float temperature_avg = temperature_sum / icm20602_FIFOParam.samples;
    const float temperature_C = (temperature_avg / TEMP_SENS) + TEMP_OFFSET;

    icm20602_fifo.temp = temperature_C;
}

static int ICM20602_Probe(void) {
    uint8_t whoami;
    whoami = ICM20602_ReadReg(WHO_AM_I);
    if(whoami != WHOAMI) {
        LOG_ERROR(device, "unexpected WHO_AM_I reg 0x%02x\n", whoami);
        return ENODEV;
    }
    return 0;
}

static void ICM20602_Statistics(void) {
    LOG_DEBUG(device, "Statistics:");
    LOG_DEBUG(device, "accel_x = %.3f [m/s2]", icm20602_fifo.accel_x[0]);
    LOG_DEBUG(device, "accel_y = %.3f [m/s2]", icm20602_fifo.accel_y[0]);
    LOG_DEBUG(device, "accel_z = %.3f [m/s2]", icm20602_fifo.accel_z[0]);
    LOG_DEBUG(device, "gyro_x  = %.3f [deg/s]", icm20602_fifo.gyro_x[0]);
    LOG_DEBUG(device, "gyro_y  = %.3f [deg/s]", icm20602_fifo.gyro_y[0]);
    LOG_DEBUG(device, "gyro_z  = %.3f [deg/s]", icm20602_fifo.gyro_z[0]);
    LOG_DEBUG(device, "temp    = %.3f [C]", icm20602_fifo.temp);
    LOG_DEBUG(device, "N       = %lu [samples]", icm20602_fifo.samples);
    LOG_DEBUG(device, "dt      = %lu [ms]", icm20602_fifo.dt);
    LOG_DEBUG(device, "N = %lu", icm20602_fifo.samples);
}
