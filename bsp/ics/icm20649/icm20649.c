#include "icm20649.h"

#include "icm20649_reg.h"

// Private functions
void icm20649_fsm(void *area);
static int icm20649_configure(icm20649_t *dev);
static int icm20649_reset_fifo(icm20649_t *dev);
static void icm20649_chip_select(icm20649_t *dev);
static void icm20649_chip_deselect(icm20649_t *dev);
static uint8_t icm20649_read_reg(icm20649_t *dev, uint8_t bank, uint8_t reg);
static void icm20649_write_reg(icm20649_t *dev, uint8_t bank, uint8_t reg,
                               uint8_t value);
static void icm20649_set_clear_reg(icm20649_t *dev, uint8_t bank, uint8_t reg,
                                   uint8_t setbits, uint8_t clearbits);
static int icm20649_fifo_read(icm20649_t *dev, uint16_t samples);
static void icm20649_accel_configure(icm20649_t *dev);
static void icm20649_gyro_configure(icm20649_t *dev);
static void icm20649_fifo_reset(icm20649_t *dev);
static void icm20649_temp_process(icm20649_t *dev);
static void icm20649_gyro_process(icm20649_t *dev);
static void icm20649_accel_process(icm20649_t *dev);
static void icm20649_update_meas(icm20649_t *dev);

void icm20649_drdy_handler(void *area);

// Public functions
icm20649_t *icm20649_start(char *name, uint32_t period, uint32_t priority,
                           spi_t *spi, gpio_t *cs, exti_t *drdy) {
    if (spi == NULL || cs == NULL || name == NULL || period <= 0 ||
        priority <= 0) {
        return NULL;
    }

    icm20649_t *dev = calloc(1, sizeof(icm20649_t));

    if (dev == NULL) {
        return NULL;
    }

    strncpy(dev->name, name, MAX_NAME_LEN - 1);
    dev->interface.spi = spi;
    dev->interface.cs = cs;

    if (drdy != NULL) {
        dev->interface.drdy = drdy;
        if (exti_init(dev->interface.drdy, icm20649_drdy_handler, dev)) {
            return NULL;
        }
        dev->sync.drdy_sem = xSemaphoreCreateBinary();
        if (dev->sync.drdy_sem == NULL) {
            return NULL;
        }
    }

    dev->sync.measrdy_sem = xSemaphoreCreateBinary();
    if (dev->sync.measrdy_sem == NULL) {
        return NULL;
    }

    dev->sync.mutex = xSemaphoreCreateMutex();
    if (dev->sync.measrdy_sem == NULL) {
        return NULL;
    }

    dev->state = ICM20649_RESET;

    if ((dev->service = service_start(dev->name, dev, icm20649_fsm, period,
                                      priority)) == NULL) {
        LOG_ERROR(dev->name, "Fatal error");
        return NULL;
    }

    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);

    return dev;
}

void icm20649_fsm(void *area) {
    icm20649_t *dev = (icm20649_t *)area;
    switch (dev->state) {
        case ICM20649_RESET:
            icm20649_write_reg(dev, BANK_0, PWR_MGMT_1, DEVICE_RESET);
            dev->state = ICM20649_RESET_WAIT;
            vTaskDelay(100);
            break;

        case ICM20649_RESET_WAIT:
            if ((icm20649_read_reg(dev, BANK_0, WHO_AM_I) == WHOAMI) &&
                (icm20649_read_reg(dev, BANK_0, PWR_MGMT_1) == 0x41)) {
                icm20649_write_reg(dev, BANK_0, PWR_MGMT_1, CLKSEL_0);
                icm20649_write_reg(dev, BANK_0, USER_CTRL,
                                   I2C_IF_DIS | SRAM_RST);
                dev->state = ICM20649_CONF;
            } else {
                dev->state = ICM20649_RESET;
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = ICM20649_FAIL;
                    LOG_ERROR(dev->name,
                              "Wrong default registers values after reset");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case ICM20649_CONF:
            if (icm20649_configure(dev)) {
                icm20649_fifo_reset(dev);
                LOG_INFO(dev->name, "Initialization successful");
                dev->state = ICM20649_FIFO_READ;
            } else {
                LOG_WARN(dev->name, "Failed configuration, retrying");
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = ICM20649_RESET;
                    LOG_ERROR(dev->name, "Failed configuration");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case ICM20649_FIFO_READ:
            if (dev->interface.drdy != NULL) {
                xSemaphoreTake(dev->sync.drdy_sem, portMAX_DELAY);
            }
            icm20649_fifo_read(dev, 1);
            icm20649_temp_process(dev);
            icm20649_accel_process(dev);
            icm20649_gyro_process(dev);
            icm20649_update_meas(dev);
            if (dev->interface.drdy != NULL) {
                irq_enable(dev->interface.drdy->p.id);
            }
            xSemaphoreGive(dev->sync.measrdy_sem);
            break;

        case ICM20649_FAIL:
            xSemaphoreGive(dev->sync.measrdy_sem);
            vTaskDelete(NULL);
            return;

        default:
            break;
    }
}

void icm20649_drdy_handler(void *area) {
    icm20649_t *dev = (icm20649_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(dev->sync.drdy_sem, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&dev->interface.drdy->handle,
                          EXTI_TRIGGER_RISING);
    irq_disable(dev->interface.drdy->p.id);

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void icm20649_get_meas_non_block(icm20649_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    memcpy(ptr, (void *)&dev->meas, sizeof(icm20649_meas_t));
    xSemaphoreGive(dev->sync.mutex);
}

void icm20649_get_meas_block(icm20649_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    icm20649_get_meas_non_block(dev, ptr);
}

void icm20649_stat(icm20649_t *dev) {
    if (dev == NULL || dev->state != ICM20649_FIFO_READ) {
        return;
    }
    printf("\n");
    LOG_DEBUG(dev->name, "Statistics:");
    LOG_DEBUG(dev->name, "accel_x = %.3f [m/s2]",
              dev->meas_buffer.meas[0].accel_x);
    LOG_DEBUG(dev->name, "accel_y = %.3f [m/s2]",
              dev->meas_buffer.meas[0].accel_y);
    LOG_DEBUG(dev->name, "accel_z = %.3f [m/s2]",
              dev->meas_buffer.meas[0].accel_z);
    LOG_DEBUG(dev->name, "gyro_x  = %.3f [deg/s]",
              dev->meas_buffer.meas[0].gyro_x);
    LOG_DEBUG(dev->name, "gyro_y  = %.3f [deg/s]",
              dev->meas_buffer.meas[0].gyro_y);
    LOG_DEBUG(dev->name, "gyro_z  = %.3f [deg/s]",
              dev->meas_buffer.meas[0].gyro_z);
    LOG_DEBUG(dev->name, "temp    = %.3f [C]", dev->meas_buffer.temp);
    LOG_DEBUG(dev->name, "N       = %lu [samples]", dev->fifo_param.samples);
}

// Private functions
static void icm20649_chip_select(icm20649_t *dev) {
    spi_chip_select(dev->interface.spi, dev->interface.cs);
}

static void icm20649_chip_deselect(icm20649_t *dev) {
    spi_chip_deselect(dev->interface.spi, dev->interface.cs);
}

static void icm20649_set_bank(icm20649_t *dev, uint8_t bank) {
    static uint8_t prev_bank = 0xFF;
    uint8_t data[2];
    data[0] = REG_BANK_SEL;
    data[1] = bank;
    if (bank != prev_bank) {
        spi_transmit_receive(dev->interface.spi, data, data, sizeof(data));
    }
    prev_bank = bank;
}

static void icm20649_exchange(icm20649_t *dev, uint8_t bank, uint8_t *tx_buf,
                              uint8_t *rx_buf, uint16_t length) {
    icm20649_chip_select(dev);
    icm20649_set_bank(dev, bank);
    spi_transmit_receive(dev->interface.spi, tx_buf, rx_buf, length);
    icm20649_chip_deselect(dev);
}

static uint8_t icm20649_read_reg(icm20649_t *dev, uint8_t bank, uint8_t reg) {
    uint8_t data[2];
    data[0] = reg | READ;
    data[1] = 0;
    icm20649_exchange(dev, bank, data, data, sizeof(data));
    return data[1];
}

static void icm20649_write_reg(icm20649_t *dev, uint8_t bank, uint8_t reg,
                               uint8_t value) {
    uint8_t data[2];
    data[0] = reg | WRITE;
    data[1] = value;
    icm20649_exchange(dev, bank, data, data, sizeof(data));
}

static void icm20649_set_clear_reg(icm20649_t *dev, uint8_t bank, uint8_t reg,
                                   uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = icm20649_read_reg(dev, bank, reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        icm20649_write_reg(dev, bank, reg, val);
    }
}

static int icm20649_configure(icm20649_t *dev) {
    uint8_t orig_val;
    int rv = 1;

    // Set configure BANK_0
    for (int i = 0; i < BANK_0_SIZE_REG_CFG; i++) {
        icm20649_set_clear_reg(dev, BANK_0, bank_0_reg_cfg[i].reg,
                               bank_0_reg_cfg[i].setbits,
                               bank_0_reg_cfg[i].clearbits);
    }

    // Check BANK_0
    for (int i = 0; i < BANK_0_SIZE_REG_CFG; i++) {
        orig_val = icm20649_read_reg(dev, BANK_0, bank_0_reg_cfg[i].reg);

        if ((orig_val & bank_0_reg_cfg[i].setbits) !=
            bank_0_reg_cfg[i].setbits) {
            LOG_WARN(dev->name, "0x%02x: 0x%02x (0x%02x not set)",
                     (uint8_t)bank_0_reg_cfg[i].reg, orig_val,
                     bank_0_reg_cfg[i].setbits);
            rv = 0;
        }

        if ((orig_val & bank_0_reg_cfg[i].clearbits) != 0) {
            LOG_WARN(dev->name, "0x%02x: 0x%02x (0x%02x not cleared)",
                     (uint8_t)bank_0_reg_cfg[i].reg, orig_val,
                     bank_0_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set configure BANK_2
    for (int i = 0; i < BANK_2_SIZE_REG_CFG; i++) {
        icm20649_set_clear_reg(dev, BANK_2, bank_2_reg_cfg[i].reg,
                               bank_2_reg_cfg[i].setbits,
                               bank_2_reg_cfg[i].clearbits);
    }

    // Check BANK_2
    for (int i = 0; i < BANK_2_SIZE_REG_CFG; i++) {
        orig_val = icm20649_read_reg(dev, BANK_2, bank_2_reg_cfg[i].reg);

        if ((orig_val & bank_2_reg_cfg[i].setbits) !=
            bank_2_reg_cfg[i].setbits) {
            LOG_WARN(dev->name, "0x%02x: 0x%02x (0x%02x not set)",
                     (uint8_t)bank_2_reg_cfg[i].reg, orig_val,
                     bank_2_reg_cfg[i].setbits);
            rv = 0;
        }

        if ((orig_val & bank_2_reg_cfg[i].clearbits) != 0) {
            LOG_WARN(dev->name, "0x%02x: 0x%02x (0x%02x not cleared)",
                     (uint8_t)bank_2_reg_cfg[i].reg, orig_val,
                     bank_2_reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set scale and range for processing
    icm20649_accel_configure(dev);
    icm20649_gyro_configure(dev);

    // Enable EXTI IRQ for DataReady pin
    if (dev->interface.drdy != NULL) {
        irq_enable(dev->interface.drdy->p.id);
    }

    return rv;
}

static void icm20649_accel_configure(icm20649_t *dev) {
    const uint8_t ACCEL_FS_SEL =
        icm20649_read_reg(dev, BANK_2, ACCEL_CONFIG) & (BIT1 | BIT2);

    if (ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        dev->meas_param.accel_scale = (CONST_G / 8192.f);
        dev->meas_param.accel_range = (4.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        dev->meas_param.accel_scale = (CONST_G / 4096.f);
        dev->meas_param.accel_range = (8.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        dev->meas_param.accel_scale = (CONST_G / 2048.f);
        dev->meas_param.accel_range = (16.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_30G) {
        dev->meas_param.accel_scale = (CONST_G / 1024.f);
        dev->meas_param.accel_range = (30.f * CONST_G);
    }
}

static void icm20649_gyro_configure(icm20649_t *dev) {
    const uint8_t FS_SEL =
        icm20649_read_reg(dev, BANK_2, GYRO_CONFIG_1) & (BIT1 | BIT2);

    if (FS_SEL == GYRO_FS_SEL_500_DPS) {
        dev->meas_param.gyro_range = 500.f;
    } else if (FS_SEL == GYRO_FS_SEL_1000_DPS) {
        dev->meas_param.gyro_range = 1000.f;
    } else if (FS_SEL == GYRO_FS_SEL_2000_DPS) {
        dev->meas_param.gyro_range = 2000.f;
    } else if (FS_SEL == GYRO_FS_SEL_4000_DPS) {
        dev->meas_param.gyro_range = 4000.f;
    }

    dev->meas_param.gyro_scale = (dev->meas_param.gyro_range / 32768.f);
}

static int icm20649_fifo_read(icm20649_t *dev, uint16_t samples) {
    if (samples > 1) {
        dev->fifo_buffer.CMD = FIFO_COUNTH | READ;
        icm20649_exchange(dev, BANK_0, (uint8_t *)&dev->fifo_buffer,
                          (uint8_t *)&dev->fifo_buffer, 3);

        dev->fifo_param.bytes =
            MIN(msb_lsb_16(dev->fifo_buffer.COUNTH, dev->fifo_buffer.COUNTL),
                samples);
        dev->fifo_param.samples =
            dev->fifo_param.bytes / sizeof(icm20649_fifo_t);

        dev->fifo_buffer.CMD = FIFO_COUNTH | READ;
        icm20649_exchange(dev, BANK_0, (uint8_t *)&dev->fifo_buffer,
                          (uint8_t *)&dev->fifo_buffer,
                          dev->fifo_param.bytes + 3);
    } else {
        dev->fifo_buffer.COUNTL = ACCEL_XOUT_H | READ;
        dev->fifo_param.samples = 1;
        dev->fifo_param.bytes = sizeof(icm20649_fifo_t);
        icm20649_exchange(dev, BANK_0, (uint8_t *)&dev->fifo_buffer.COUNTL,
                          (uint8_t *)&dev->fifo_buffer.COUNTL,
                          dev->fifo_param.bytes + 1);
    }

    return 0;
}

static void icm20649_fifo_reset(icm20649_t *dev) {
    icm20649_write_reg(dev, BANK_0, FIFO_RST, FIFO_RESET);
    icm20649_set_clear_reg(dev, BANK_0, FIFO_RST, 0, FIFO_RESET);
}

static void icm20649_accel_process(icm20649_t *dev) {
    for (int i = 0; i < dev->fifo_param.samples; i++) {
        int16_t accel_x = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_XOUT_H,
                                     dev->fifo_buffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_YOUT_H,
                                     dev->fifo_buffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_ZOUT_H,
                                     dev->fifo_buffer.buf[i].ACCEL_ZOUT_L);

        dev->meas_buffer.meas[i].accel_x =
            accel_x * dev->meas_param.accel_scale;
        dev->meas_buffer.meas[i].accel_y =
            ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
            dev->meas_param.accel_scale;
        dev->meas_buffer.meas[i].accel_z =
            ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
            dev->meas_param.accel_scale;
    }
}

static void icm20649_gyro_process(icm20649_t *dev) {
    for (int i = 0; i < dev->fifo_param.samples; i++) {
        int16_t gyro_x = msb_lsb_16(dev->fifo_buffer.buf[i].GYRO_XOUT_H,
                                    dev->fifo_buffer.buf[i].GYRO_XOUT_L);
        int16_t gyro_y = msb_lsb_16(dev->fifo_buffer.buf[i].GYRO_YOUT_H,
                                    dev->fifo_buffer.buf[i].GYRO_YOUT_L);
        int16_t gyro_z = msb_lsb_16(dev->fifo_buffer.buf[i].GYRO_ZOUT_H,
                                    dev->fifo_buffer.buf[i].GYRO_ZOUT_L);

        dev->meas_buffer.meas[i].gyro_x = gyro_x * dev->meas_param.gyro_scale;
        dev->meas_buffer.meas[i].gyro_y =
            ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
            dev->meas_param.gyro_scale;
        dev->meas_buffer.meas[i].gyro_z =
            ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
            dev->meas_param.gyro_scale;
    }
}

static void icm20649_temp_process(icm20649_t *dev) {
    uint8_t data[3];
    data[0] = TEMP_OUT_H | READ;

    icm20649_exchange(dev, BANK_0, data, data, sizeof(data));

    int16_t temp_raw = msb_lsb_16(data[1], data[2]);
    dev->meas_buffer.temp =
        (temp_raw - TEMP_SENS * TEMP_OFFSET) / TEMP_SENS + TEMP_OFFSET;
}

static void icm20649_update_meas(icm20649_t *dev) {
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    dev->meas.accel_x = dev->meas_buffer.meas[0].accel_x;
    dev->meas.accel_y = dev->meas_buffer.meas[0].accel_y;
    dev->meas.accel_z = dev->meas_buffer.meas[0].accel_z;
    dev->meas.gyro_x = dev->meas_buffer.meas[0].gyro_x;
    dev->meas.gyro_y = dev->meas_buffer.meas[0].gyro_y;
    dev->meas.gyro_z = dev->meas_buffer.meas[0].gyro_z;
    xSemaphoreGive(dev->sync.mutex);
}
