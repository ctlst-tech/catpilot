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
static uint8_t ICM20948_ReadReg(uint8_t bank, uint8_t reg);
static void ICM20948_WriteReg(uint8_t bank, uint8_t reg, uint8_t value);
static void ICM20948_SetClearReg(uint8_t bank, uint8_t reg, uint8_t setbits, uint8_t clearbits);
static int ICM20948_Configure(void);
static void ICM20948_AccelConfigure(void);
static void ICM20948_GyroConfigure(void);
static int ICM20948_Probe(void);
static void ICM20948_Statistics(void);
static int ICM20948_SampleRead(void);
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
int ICM20948_Init(spi_cfg_t *spi, 
                  gpio_cfg_t *cs, 
                  exti_cfg_t *drdy, 
                  i2c_cfg_t *i2c,
                  int enable_mag,
                  int enable_drdy) {
    if(spi == NULL || cs == NULL) return -1;

    icm20948_cfg.spi = spi;
    icm20948_cfg.cs = cs;
    icm20948_cfg.enable_drdy = enable_drdy;
    icm20948_cfg.enable_mag = enable_mag;

    if(enable_drdy) {
        if(drdy == NULL) return -1;
        icm20948_cfg.drdy = drdy;
        if(drdy_semaphore == NULL) drdy_semaphore = xSemaphoreCreateBinary();
    }
    if(enable_mag) {
        if(i2c == NULL) return -1;
        icm20948_cfg.i2c = i2c;
    }

    if(measrdy_semaphore == NULL) measrdy_semaphore = xSemaphoreCreateBinary();

    timer_id = Timer_Create("ICM20948_Timer");

    xSemaphoreTake(measrdy_semaphore, 0);
    icm20948_state = ICM20948_RESET;

    return 0;
}

static int ind = 0;
static void _debug(void) {
    static int all = 0;
    static int error = 0;
    all += icm20948_fifo.samples;
    // if(xTaskGetTickCount() > 60000) {
    //         vTaskDelay(100);
    // }
}

void ICM20948_Run(void) {
    switch(icm20948_state) {

    case ICM20948_RESET:
        timer_status = Timer_Start(timer_id, 2000);

        if(timer_status == TIMER_STARTED) {
            ICM20948_WriteReg(BANK_0, PWR_MGMT_1, DEVICE_RESET);
        } else if(timer_status == TIMER_COUNTING) {
        } else if(timer_status == TIMER_FINISHED) {
            icm20948_state = ICM20948_RESET_WAIT;
        }

        break;

    case ICM20948_RESET_WAIT:
        timer_status = Timer_Start(timer_id, 100);
    
        if(timer_status == TIMER_STARTED) {
            if((ICM20948_ReadReg(BANK_0, WHO_AM_I) == WHOAMI) && 
               (ICM20948_ReadReg(BANK_0, PWR_MGMT_1) == 0x41)) {
                ICM20948_WriteReg(BANK_0, PWR_MGMT_1, CLKSEL_0);
                ICM20948_WriteReg(BANK_0, USER_CTRL, I2C_IF_DIS | SRAM_RST);
            } else {
                Timer_Stop(timer_id);
                LOG_ERROR(device, "Wrong default registers values after reset");
                icm20948_state = ICM20948_RESET;
                attempt++;
                if(attempt > 5) {
                    icm20948_state = ICM20948_FAIL;
                    LOG_ERROR(device, "Fatal error");
                    attempt = 0;
                }
            }
        } else if(timer_status == TIMER_COUNTING) {
        } else if(timer_status == TIMER_FINISHED) {
            icm20948_state = ICM20948_CONF;
            LOG_DEBUG(device, "Device available");
        }

        break;

    case ICM20948_CONF:
        if(ICM20948_Configure()) {
            ICM20948_FIFOReset();
            icm20948_last_sample = xTaskGetTickCount();
            // ICM20948_FIFOCount();
            // ICM20948_FIFORead();
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
        if(icm20948_cfg.enable_drdy) {
            xSemaphoreTake(drdy_semaphore, portMAX_DELAY);
        }
        // ICM20948_FIFOCount();
        // ICM20948_FIFORead();
        ICM20948_SampleRead();
        icm20948_fifo.imu_dt = xTaskGetTickCount() - icm20948_last_sample;
        icm20948_fifo.samples = icm20948_FIFOParam.samples;
        icm20948_last_sample = xTaskGetTickCount();
        xSemaphoreGive(measrdy_semaphore);
        _debug();
        break;

    case ICM20948_FAIL:
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
    SPI_ChipSelect(icm20948_cfg.spi, icm20948_cfg.cs);
}

static void ICM20948_ChipDeselection(void) {
    SPI_ChipDeselect(icm20948_cfg.spi, icm20948_cfg.cs);
}

static void ICM20948_SetBank(uint8_t bank) {
    static uint8_t prev_bank = 0xFF;
    uint8_t data[2];
    data[0] = REG_BANK_SEL;
    data[1] = bank;
    if(bank != prev_bank) {
        SPI_TransmitReceive(icm20948_cfg.spi, data, data, sizeof(data));
    }
    prev_bank = bank;
}

static uint8_t ICM20948_ReadReg(uint8_t bank, uint8_t reg) {
    uint8_t data[2];
    data[0] = reg | READ;
    ICM20948_ChipSelection();
    ICM20948_SetBank(bank);
    SPI_TransmitReceive(icm20948_cfg.spi, data, data, sizeof(data));
    ICM20948_ChipDeselection();
    return data[1];
}

static void ICM20948_WriteReg(uint8_t bank, uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;
    ICM20948_ChipSelection();
    ICM20948_SetBank(bank);
    SPI_Transmit(icm20948_cfg.spi, data, sizeof(data));
    ICM20948_ChipDeselection();
}

static void ICM20948_SetClearReg(uint8_t bank, uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = ICM20948_ReadReg(bank, reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        ICM20948_WriteReg(bank, reg, val);
    }
}

static int ICM20948_Configure(void) {
    uint8_t orig_val;
    int rv = 1;

    // Set configure BANK_0
    for(int i = 0; i < BANK_0_SIZE_REG_CFG; i++) {
        ICM20948_SetClearReg(BANK_0, 
                             bank_0_reg_cfg[i].reg, 
                             bank_0_reg_cfg[i].setbits, 
                             bank_0_reg_cfg[i].clearbits);
    }

    // Check BANK_0
    for(int i = 0; i < BANK_0_SIZE_REG_CFG; i++) {
        orig_val = ICM20948_ReadReg(BANK_0, bank_0_reg_cfg[i].reg);

        if((orig_val & bank_0_reg_cfg[i].setbits) != bank_0_reg_cfg[i].setbits) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not set)",
            (uint8_t)bank_0_reg_cfg[i].reg, orig_val, bank_0_reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & bank_0_reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not cleared)",
            (uint8_t)bank_0_reg_cfg[i].reg, orig_val, bank_0_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set configure BANK_2
    for(int i = 0; i < BANK_2_SIZE_REG_CFG; i++) {
        ICM20948_SetClearReg(BANK_2, 
                             bank_2_reg_cfg[i].reg, 
                             bank_2_reg_cfg[i].setbits, 
                             bank_2_reg_cfg[i].clearbits);
    }

    // Check BANK_2
    for(int i = 0; i < BANK_2_SIZE_REG_CFG; i++) {
        orig_val = ICM20948_ReadReg(BANK_2, bank_2_reg_cfg[i].reg);

        if((orig_val & bank_2_reg_cfg[i].setbits) != bank_2_reg_cfg[i].setbits) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not set)",
            (uint8_t)bank_2_reg_cfg[i].reg, orig_val, bank_2_reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & bank_2_reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not cleared)",
            (uint8_t)bank_2_reg_cfg[i].reg, orig_val, bank_2_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set configure BANK_3
    reg_cfg_t bank_3_reg_cfg[BANK_3_SIZE_REG_CFG];
    if(icm20948_cfg.enable_mag) {
        memcpy(bank_3_reg_cfg, 
               bank_3_reg_cfg_w_mag, 
               sizeof(reg_cfg_t) * BANK_3_SIZE_REG_CFG);
    } else {
        memcpy(bank_3_reg_cfg, 
               bank_3_reg_cfg_wo_mag, 
               sizeof(reg_cfg_t) * BANK_3_SIZE_REG_CFG);
    }

    for(int i = 0; i < BANK_3_SIZE_REG_CFG; i++) {
        ICM20948_SetClearReg(BANK_3, 
                             bank_3_reg_cfg[i].reg, 
                             bank_3_reg_cfg[i].setbits, 
                             bank_3_reg_cfg[i].clearbits);
    }

    // Check BANK_3
    for(int i = 0; i < BANK_3_SIZE_REG_CFG; i++) {
        orig_val = ICM20948_ReadReg(BANK_3, bank_3_reg_cfg[i].reg);

        if((orig_val & bank_3_reg_cfg[i].setbits) != bank_3_reg_cfg[i].setbits) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not set)",
            (uint8_t)bank_3_reg_cfg[i].reg, orig_val, bank_3_reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & bank_3_reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(device, "0x%02x: 0x%02x (0x%02x not cleared)",
            (uint8_t)bank_3_reg_cfg[i].reg, orig_val, bank_3_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    vTaskDelay(1);

    // Set scale and range for processing
    ICM20948_AccelConfigure();
    ICM20948_GyroConfigure();

    // Enable EXTI IRQ for DataReady pin
    if(icm20948_cfg.enable_drdy) {
        EXTI_EnableIRQ(icm20948_cfg.drdy);
    }

    return rv;
}

static void ICM20948_AccelConfigure(void) {
    uint8_t ACCEL_FS_SEL = ICM20948_ReadReg(BANK_2, ACCEL_CONFIG) & (BIT1 | BIT2);
    ACCEL_FS_SEL = ICM20948_ReadReg(BANK_2, ACCEL_CONFIG) & (BIT1 | BIT2);


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
    uint8_t FS_SEL = ICM20948_ReadReg(BANK_2, GYRO_CONFIG_1) & (BIT1 | BIT2);
    FS_SEL = ICM20948_ReadReg(BANK_2, GYRO_CONFIG_1) & (BIT1 | BIT2);

    if(FS_SEL == GYRO_FS_SEL_250_DPS) {
        icm20948_cfg.param.gyro_range = 250.f;
    } else if(FS_SEL == GYRO_FS_SEL_500_DPS) {
        icm20948_cfg.param.gyro_range = 500.f;
    } else if(FS_SEL == GYRO_FS_SEL_1000_DPS) {
        icm20948_cfg.param.gyro_range = 1000.f;
    } else if(FS_SEL == GYRO_FS_SEL_2000_DPS) {
        icm20948_cfg.param.gyro_range = 2000.f;
    }

    icm20948_cfg.param.gyro_scale = (icm20948_cfg.param.gyro_range / 32768.f);
}

static int ICM20948_SampleRead(void) {
    icm20948_FIFOBuffer.COUNTL = ACCEL_XOUT_H | READ;
    icm20948_FIFOParam.bytes = 13;
    icm20948_FIFOParam.samples = 1;

    ICM20948_ChipSelection();
    ICM20948_SetBank(BANK_0);
    SPI_TransmitReceive(icm20948_cfg.spi, 
                (uint8_t *)&icm20948_FIFOBuffer.COUNTL, 
                (uint8_t *)&icm20948_FIFOBuffer.COUNTL,
                icm20948_FIFOParam.bytes);
    ICM20948_ChipDeselection();

    ICM20948_TempProcess();
    ICM20948_AccelProcess();
    ICM20948_GyroProcess();

    if(icm20948_cfg.enable_drdy) {
        EXTI_EnableIRQ(icm20948_cfg.drdy);
    }

    return 0;
}

static void ICM20948_FIFOCount(void) {
    icm20948_FIFOBuffer.CMD = FIFO_COUNTH | READ;

    ICM20948_ChipSelection();
    ICM20948_SetBank(BANK_0);
    SPI_TransmitReceive(icm20948_cfg.spi, 
                        (uint8_t *)&icm20948_FIFOBuffer, 
                        (uint8_t *)&icm20948_FIFOBuffer,
                        3);
    ICM20948_ChipDeselection();
    icm20948_FIFOParam.bytes = msblsb16(icm20948_FIFOBuffer.COUNTH, 
                                        icm20948_FIFOBuffer.COUNTL);
    icm20948_FIFOParam.samples = icm20948_FIFOParam.bytes / sizeof(FIFO_t);
}

static int ICM20948_FIFORead(void) {
    icm20948_FIFOBuffer.CMD = FIFO_COUNTH | READ;

    ICM20948_ChipSelection();
    ICM20948_SetBank(BANK_0);
    SPI_TransmitReceive(icm20948_cfg.spi, 
                        (uint8_t *)&icm20948_FIFOBuffer, 
                        (uint8_t *)&icm20948_FIFOBuffer,
                        icm20948_FIFOParam.bytes + 3);
    ICM20948_ChipDeselection();

    ICM20948_TempProcess();
    ICM20948_AccelProcess();
    ICM20948_GyroProcess();

    if(icm20948_cfg.enable_drdy) {
        EXTI_EnableIRQ(icm20948_cfg.drdy);
    }

    return 0;
}

static void ICM20948_FIFOReset(void) {
    ICM20948_WriteReg(BANK_0, FIFO_RST, FIFO_RESET);
    ICM20948_SetClearReg(BANK_0, FIFO_RST, 0, FIFO_RESET);
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

        ind += (fabs(icm20948_fifo.accel_x[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20948_fifo.accel_y[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20948_fifo.accel_z[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20948_fifo.accel_z[i]) < 9 ? 1 : 0);
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
        ind += (fabs(icm20948_fifo.gyro_x[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20948_fifo.gyro_y[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20948_fifo.gyro_z[i]) > 10 ? 1 : 0);
    }
}

static void ICM20948_TempProcess(void) {
    uint8_t data[3];
    data[0] = TEMP_OUT_H | READ;

    ICM20948_ChipSelection();
    ICM20948_SetBank(BANK_0);
    SPI_TransmitReceive(icm20948_cfg.spi, &data[0], &data[1], sizeof(data));
    ICM20948_ChipDeselection();

    int16_t temp_raw = msblsb16(data[1], data[2]);
    icm20948_fifo.temp = temp_raw / TEMP_SENS + TEMP_OFFSET;
}

static int ICM20948_Probe(void) {
    uint8_t whoami;
    whoami = ICM20948_ReadReg(BANK_1, WHO_AM_I);
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
    LOG_DEBUG(device, "mag_x   = %.3f [G]", icm20948_fifo.mag_x);
    LOG_DEBUG(device, "mag_y   = %.3f [G]", icm20948_fifo.mag_y);
    LOG_DEBUG(device, "mag_z   = %.3f [G]", icm20948_fifo.mag_z);
    LOG_DEBUG(device, "temp    = %.3f [C]", icm20948_fifo.temp);
    LOG_DEBUG(device, "N       = %lu [samples]", icm20948_fifo.samples);
    LOG_DEBUG(device, "IMU dt  = %lu [ms]", icm20948_fifo.imu_dt);
    LOG_DEBUG(device, "MAG dt  = %lu [ms]", icm20948_fifo.mag_dt);
    LOG_DEBUG(device, "N = %lu", icm20948_fifo.samples);
}
