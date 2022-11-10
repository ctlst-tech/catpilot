#include "icm20649.h"

#include "icm20649_reg.h"

// Private functions
static void icm20649_thread(void *dev_ptr);
static void icm20649_fsm(void *dev_ptr);

// Public functions
int icm20649_start(spi_t *spi, gpio_t *cs, exti_t *drdy, uint32_t period,
                   uint32_t thread_priority) {
    if (spi == NULL || cs == NULL) {
        return -1;
    }

    icm20649_t *dev = calloc(1, sizeof(icm20649_t));

    if (dev == NULL) {
        return -1;
    }

    dev->interface.spi = spi;
    dev->interface.cs = cs;

    if (drdy != NULL) {
        dev->interface.drdy = drdy;
        if (dev->sync.drdy_semaphore == NULL) {
            dev->sync.drdy_semaphore = xSemaphoreCreateBinary();
        }
        LOG_DEBUG(dev->name, "DRDY mode");
    } else {
        LOG_DEBUG(dev->name, "Not DRDY mode");
    }

    if (period < 2) {
        LOG_ERROR(dev->name, "Too high frequency");
        return -1;
    }

    dev->os.period = period;

    if (dev->sync.measrdy_semaphore == NULL) {
        dev->sync.measrdy_semaphore = xSemaphoreCreateBinary();
    }

    if (xTaskCreate(icm20649_thread, dev->name, 512, dev, dev->os.priority,
                    NULL) != pdTRUE) {
        LOG_ERROR(dev->name, "Thread start error");
        return -1;
    }

    xSemaphoreTake(dev->sync.measrdy_semaphore, 0);
    dev->state = ICM20649_RESET;

    return 0;
}

void icm20649_thread(void *dev_ptr) {
    icm20649_t *dev = dev_ptr;
    TickType_t last_wake_time;

    last_wake_time = xTaskGetTickCount();

    while (1) {
        icm20649_fsm(dev);
        xTaskDelayUntil(&last_wake_time, dev->os.period / portTICK_PERIOD_MS);
    }
}

void icm20649_fsm(void *dev_ptr) {
    icm20649_t *dev = dev_ptr;

    switch (dev->state) {
        case ICM20649_RESET:
            icm20649_write_reg(BANK_0, PWR_MGMT_1, DEVICE_RESET);
            vTaskDelay(100);
            dev->state = ICM20649_RESET_WAIT;
            break;

        case ICM20649_RESET_WAIT:
            if ((icm20649_read_reg(BANK_0, WHO_AM_I) == WHOAMI) &&
                (icm20649_read_reg(BANK_0, PWR_MGMT_1) == 0x41)) {
                icm20649_write_reg(BANK_0, PWR_MGMT_1, CLKSEL_0);
                icm20649_write_reg(BANK_0, USER_CTRL, I2C_IF_DIS | SRAM_RST);
                vTaskDelay(100);
                dev->state = ICM20649_CONF;
                LOG_DEBUG(dev->name, "Device available");
            } else {
                LOG_ERROR(dev->name,
                          "Wrong default registers values after reset");
                dev->state = ICM20649_RESET;
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = ICM20649_FAIL;
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case ICM20649_CONF:
            if (icm20649_configure(dev)) {
                icm20649_fifo_reset(dev);
                // icm20649_fifo_count();
                // ICM20649_fifo_read();
                LOG_DEBUG(dev->name, "Device configured");
                dev->state = ICM20649_FIFO_READ;
            } else {
                LOG_ERROR(dev->name, "Failed configuration, retrying...");
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = ICM20649_RESET;
                    LOG_ERROR(dev->name, "Failed configuration, reset...");
                    dev->attempt = 0;
                }
            }
            break;

        case ICM20649_FIFO_READ:
            if (dev->interface.drdy != NULL) {
                xSemaphoreTake(dev->sync.drdy_semaphore, portMAX_DELAY);
            }
            icm20649_sample_read(dev);
            xSemaphoreGive(dev->sync.measrdy_semaphore);
            break;

        case ICM20649_FAIL:
            vTaskDelay(0);
            break;

        default:
            break;
    }
}

int ICM20649_Operation(void) {
    if (dev->state == ICM20649_FIFO_READ) {
        return 0;
    } else {
        return -1;
    }
}

void ICM20649_DataReadyHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(drdy_semaphore, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&icm20649_cfg.drdy->EXTI_Handle,
                          EXTI_TRIGGER_RISING);
    EXTI_DisableIRQ(icm20649_cfg.drdy);

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void ICM20649_GetMeasBlock(void *ptr) {
    xSemaphoreTake(measrdy_semaphore, portMAX_DELAY);
    memcpy(ptr, (void *)&icm20649_imu_meas, sizeof(icm20649_imu_meas_t));
}

void ICM20649_GetMeasNonBlock(void *ptr) {
    memcpy(ptr, (void *)&icm20649_imu_meas, sizeof(icm20649_imu_meas_t));
}

// Private functions
static void ICM20649_ChipSelection(void) {
    spi_chip_select(icm20649_cfg.spi, icm20649_cfg.cs);
}

static void ICM20649_ChipDeselection(void) {
    spi_chip_deselect(icm20649_cfg.spi, icm20649_cfg.cs);
}

static void ICM20649_SetBank(uint8_t bank) {
    static uint8_t prev_bank = 0xFF;
    uint8_t data[2];
    data[0] = REG_BANK_SEL;
    data[1] = bank;
    if (bank != prev_bank) {
        spi_transmit(icm20649_cfg.spi, data, sizeof(data));
    }
    prev_bank = bank;
}

static uint8_t icm20649_read_reg(uint8_t bank, uint8_t reg) {
    uint8_t data[2];
    data[0] = reg | READ;
    ICM20649_ChipSelection();
    ICM20649_SetBank(bank);
    spi_transmit_receive(icm20649_cfg.spi, data, data, sizeof(data));
    ICM20649_ChipDeselection();
    return data[1];
}

static void icm20649_write_reg(uint8_t bank, uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;
    ICM20649_ChipSelection();
    ICM20649_SetBank(bank);
    spi_transmit(icm20649_cfg.spi, data, sizeof(data));
    ICM20649_ChipDeselection();
}

static void ICM20649_SetClearReg(uint8_t bank, uint8_t reg, uint8_t setbits,
                                 uint8_t clearbits) {
    uint8_t orig_val = icm20649_read_reg(bank, reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        icm20649_write_reg(bank, reg, val);
    }
}

static int icm20649_configure(void) {
    uint8_t orig_val;
    int rv = 1;

    // Set configure BANK_0
    for (int i = 0; i < BANK_0_SIZE_REG_CFG; i++) {
        ICM20649_SetClearReg(BANK_0, bank_0_reg_cfg[i].reg,
                             bank_0_reg_cfg[i].setbits,
                             bank_0_reg_cfg[i].clearbits);
    }

    // Check BANK_0
    for (int i = 0; i < BANK_0_SIZE_REG_CFG; i++) {
        orig_val = icm20649_read_reg(BANK_0, bank_0_reg_cfg[i].reg);

        if ((orig_val & bank_0_reg_cfg[i].setbits) !=
            bank_0_reg_cfg[i].setbits) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not set)",
                      (uint8_t)bank_0_reg_cfg[i].reg, orig_val,
                      bank_0_reg_cfg[i].setbits);
            rv = 0;
        }

        if ((orig_val & bank_0_reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not cleared)",
                      (uint8_t)bank_0_reg_cfg[i].reg, orig_val,
                      bank_0_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set configure BANK_2
    for (int i = 0; i < BANK_2_SIZE_REG_CFG; i++) {
        ICM20649_SetClearReg(BANK_2, bank_2_reg_cfg[i].reg,
                             bank_2_reg_cfg[i].setbits,
                             bank_2_reg_cfg[i].clearbits);
    }

    // Check BANK_2
    for (int i = 0; i < BANK_2_SIZE_REG_CFG; i++) {
        orig_val = icm20649_read_reg(BANK_2, bank_2_reg_cfg[i].reg);

        if ((orig_val & bank_2_reg_cfg[i].setbits) !=
            bank_2_reg_cfg[i].setbits) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not set)",
                      (uint8_t)bank_2_reg_cfg[i].reg, orig_val,
                      bank_2_reg_cfg[i].setbits);
            rv = 0;
        }

        if ((orig_val & bank_2_reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not cleared)",
                      (uint8_t)bank_2_reg_cfg[i].reg, orig_val,
                      bank_2_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set scale and range for processing
    ICM20649_AccelConfigure();
    ICM20649_GyroConfigure();

    // Enable EXTI IRQ for DataReady pin
    if (icm20649_cfg.enable_drdy) {
        EXTI_EnableIRQ(icm20649_cfg.drdy);
    }

    return rv;
}

static void ICM20649_AccelConfigure(void) {
    const uint8_t ACCEL_FS_SEL =
        icm20649_read_reg(BANK_2, ACCEL_CONFIG) & (BIT1 | BIT2);

    if (ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        icm20649_cfg.param.accel_scale = (CONST_G / 8192.f);
        icm20649_cfg.param.accel_range = (4.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        icm20649_cfg.param.accel_scale = (CONST_G / 4096.f);
        icm20649_cfg.param.accel_range = (8.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        icm20649_cfg.param.accel_scale = (CONST_G / 2048.f);
        icm20649_cfg.param.accel_range = (16.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_30G) {
        icm20649_cfg.param.accel_scale = (CONST_G / 1024.f);
        icm20649_cfg.param.accel_range = (30.f * CONST_G);
    }
}

static void ICM20649_GyroConfigure(void) {
    const uint8_t FS_SEL =
        icm20649_read_reg(BANK_2, GYRO_CONFIG_1) & (BIT1 | BIT2);

    if (FS_SEL == GYRO_FS_SEL_500_DPS) {
        icm20649_cfg.param.gyro_range = 500.f;
    } else if (FS_SEL == GYRO_FS_SEL_1000_DPS) {
        icm20649_cfg.param.gyro_range = 1000.f;
    } else if (FS_SEL == GYRO_FS_SEL_2000_DPS) {
        icm20649_cfg.param.gyro_range = 2000.f;
    } else if (FS_SEL == GYRO_FS_SEL_4000_DPS) {
        icm20649_cfg.param.gyro_range = 4000.f;
    }

    icm20649_cfg.param.gyro_scale = (icm20649_cfg.param.gyro_range / 32768.f);
}

static void ICM20649_FIFOCount(void) {
    icm20649_FIFOBuffer.CMD = FIFO_COUNTH | READ;

    ICM20649_ChipSelection();
    ICM20649_SetBank(BANK_0);
    spi_transmit_receive(icm20649_cfg.spi, (uint8_t *)&icm20649_FIFOBuffer,
                         (uint8_t *)&icm20649_FIFOBuffer, 3);
    ICM20649_ChipDeselection();

    icm20649_FIFOParam.bytes =
        msblsb16(icm20649_FIFOBuffer.COUNTH, icm20649_FIFOBuffer.COUNTL);
    icm20649_FIFOParam.samples = icm20649_FIFOParam.bytes / sizeof(FIFO_t);
}

static int ICM20649_FIFORead(void) {
    icm20649_FIFOBuffer.CMD = FIFO_COUNTH | READ;

    ICM20649_ChipSelection();
    ICM20649_SetBank(BANK_0);
    spi_transmit_receive(icm20649_cfg.spi, (uint8_t *)&icm20649_FIFOBuffer,
                         (uint8_t *)&icm20649_FIFOBuffer,
                         icm20649_FIFOParam.bytes + 3);
    ICM20649_ChipDeselection();

    ICM20649_TempProcess();
    ICM20649_AccelProcess();
    ICM20649_GyroProcess();

    if (icm20649_cfg.enable_drdy) {
        EXTI_EnableIRQ(icm20649_cfg.drdy);
    }

    return 0;
}

static int ICM20649_SampleRead(void) {
    icm20649_FIFOBuffer.COUNTL = ACCEL_XOUT_H | READ;
    icm20649_FIFOParam.bytes = 13;
    icm20649_FIFOParam.samples = 1;

    ICM20649_ChipSelection();
    ICM20649_SetBank(BANK_0);
    spi_transmit_receive(
        icm20649_cfg.spi, (uint8_t *)&icm20649_FIFOBuffer.COUNTL,
        (uint8_t *)&icm20649_FIFOBuffer.COUNTL, icm20649_FIFOParam.bytes);
    ICM20649_ChipDeselection();

    ICM20649_TempProcess();
    ICM20649_AccelProcess();
    ICM20649_GyroProcess();

    icm20649_imu_meas.accel_x = icm20649_fifo.accel_x[0];
    icm20649_imu_meas.accel_y = icm20649_fifo.accel_y[0];
    icm20649_imu_meas.accel_z = icm20649_fifo.accel_z[0];
    icm20649_imu_meas.gyro_x = icm20649_fifo.gyro_x[0];
    icm20649_imu_meas.gyro_y = icm20649_fifo.gyro_y[0];
    icm20649_imu_meas.gyro_z = icm20649_fifo.gyro_z[0];
    icm20649_imu_meas.imu_dt = icm20649_fifo.imu_dt;

    if (icm20649_cfg.enable_drdy) {
        EXTI_EnableIRQ(icm20649_cfg.drdy);
    }

    return 0;
}

static void icm20649_fifo_reset(void) {
    icm20649_write_reg(BANK_0, FIFO_RST, FIFO_RESET);
    ICM20649_SetClearReg(BANK_0, FIFO_RST, 0, FIFO_RESET);
}

static void ICM20649_AccelProcess(void) {
    for (int i = 0; i < icm20649_FIFOParam.samples; i++) {
        int16_t accel_x = msblsb16(icm20649_FIFOBuffer.buf[i].ACCEL_XOUT_H,
                                   icm20649_FIFOBuffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msblsb16(icm20649_FIFOBuffer.buf[i].ACCEL_YOUT_H,
                                   icm20649_FIFOBuffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msblsb16(icm20649_FIFOBuffer.buf[i].ACCEL_ZOUT_H,
                                   icm20649_FIFOBuffer.buf[i].ACCEL_ZOUT_L);

        icm20649_fifo.accel_x[i] = accel_x * icm20649_cfg.param.accel_scale;
        icm20649_fifo.accel_y[i] =
            ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
            icm20649_cfg.param.accel_scale;
        icm20649_fifo.accel_z[i] =
            ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
            icm20649_cfg.param.accel_scale;
        ind += (fabs(icm20649_fifo.accel_x[i]) > 12 ? 1 : 0);
        ind += (fabs(icm20649_fifo.accel_y[i]) > 12 ? 1 : 0);
        ind += (fabs(icm20649_fifo.accel_z[i]) > 12 ? 1 : 0);
        if ((fabs(icm20649_fifo.accel_x[i]) > 10) ||
            (fabs(icm20649_fifo.accel_y[i]) > 10) ||
            (fabs(icm20649_fifo.accel_z[i]) > 10)) {
            ind += 0;
        }
    }
}

static void ICM20649_GyroProcess(void) {
    for (int i = 0; i < icm20649_FIFOParam.samples; i++) {
        int16_t gyro_x = msblsb16(icm20649_FIFOBuffer.buf[i].GYRO_XOUT_H,
                                  icm20649_FIFOBuffer.buf[i].GYRO_XOUT_L);
        int16_t gyro_y = msblsb16(icm20649_FIFOBuffer.buf[i].GYRO_YOUT_H,
                                  icm20649_FIFOBuffer.buf[i].GYRO_YOUT_L);
        int16_t gyro_z = msblsb16(icm20649_FIFOBuffer.buf[i].GYRO_ZOUT_H,
                                  icm20649_FIFOBuffer.buf[i].GYRO_ZOUT_L);

        icm20649_fifo.gyro_x[i] = gyro_x * icm20649_cfg.param.gyro_scale;
        icm20649_fifo.gyro_y[i] =
            ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
            icm20649_cfg.param.gyro_scale;
        icm20649_fifo.gyro_z[i] =
            ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
            icm20649_cfg.param.gyro_scale;
        ind += (fabs(icm20649_fifo.gyro_x[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20649_fifo.gyro_y[i]) > 10 ? 1 : 0);
        ind += (fabs(icm20649_fifo.gyro_z[i]) > 10 ? 1 : 0);
        if ((fabs(icm20649_fifo.gyro_x[i]) > 10) ||
            (fabs(icm20649_fifo.gyro_y[i]) > 10) ||
            (fabs(icm20649_fifo.gyro_z[i]) > 10)) {
            ind += 0;
        }
    }
}

static void ICM20649_TempProcess(void) {
    uint8_t data[3];
    data[0] = TEMP_OUT_H | READ;

    ICM20649_ChipSelection();
    ICM20649_SetBank(BANK_0);
    spi_transmit_receive(icm20649_cfg.spi, &data[0], &data[1], sizeof(data));
    ICM20649_ChipDeselection();

    int16_t temp_raw = msblsb16(data[1], data[2]);
    icm20649_fifo.temp = temp_raw / TEMP_SENS + TEMP_OFFSET;
}

static int ICM20649_Probe(void) {
    uint8_t whoami;
    whoami = icm20649_read_reg(BANK_1, WHO_AM_I);
    if (whoami != WHOAMI) {
        LOG_ERROR(dev->name, "unexpected WHO_AM_I reg 0x%02x\n", whoami);
        return ENODEV;
    }
    return 0;
}

static void ICM20649_Statistics(void) {
    LOG_DEBUG(dev->name, "Statistics:");
    LOG_DEBUG(dev->name, "accel_x = %.3f [m/s2]", icm20649_fifo.accel_x[0]);
    LOG_DEBUG(dev->name, "accel_y = %.3f [m/s2]", icm20649_fifo.accel_y[0]);
    LOG_DEBUG(dev->name, "accel_z = %.3f [m/s2]", icm20649_fifo.accel_z[0]);
    LOG_DEBUG(dev->name, "gyro_x  = %.3f [deg/s]", icm20649_fifo.gyro_x[0]);
    LOG_DEBUG(dev->name, "gyro_y  = %.3f [deg/s]", icm20649_fifo.gyro_y[0]);
    LOG_DEBUG(dev->name, "gyro_z  = %.3f [deg/s]", icm20649_fifo.gyro_z[0]);
    LOG_DEBUG(dev->name, "temp    = %.3f [C]", icm20649_fifo.temp);
    LOG_DEBUG(dev->name, "N       = %lu [samples]", icm20649_fifo.samples);
    LOG_DEBUG(dev->name, "IMU dt  = %lu [ms]", icm20649_fifo.imu_dt);
    LOG_DEBUG(dev->name, "MAG dt  = %lu [ms]", icm20649_fifo.mag_dt);
    LOG_DEBUG(dev->name, "N = %lu", icm20649_fifo.samples);
}
