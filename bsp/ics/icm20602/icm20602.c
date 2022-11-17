#include "icm20602.h"

#include "icm20602_reg.h"

// Private functions
static void icm20602_thread(void *dev_ptr);
static void icm20602_fsm(icm20602_t *dev);
static int icm20602_configure(icm20602_t *dev);
static int icm20602_reset_fifo(icm20602_t *dev);
static void icm20602_chip_select(icm20602_t *dev);
static void icm20602_chip_deselect(icm20602_t *dev);
static uint8_t icm20602_read_reg(icm20602_t *dev, uint8_t reg);
static void icm20602_write_reg(icm20602_t *dev, uint8_t reg, uint8_t value);
static void icm20602_set_clear_reg(icm20602_t *dev, uint8_t reg,
                                   uint8_t setbits, uint8_t clearbits);
static int icm20602_fifo_read(icm20602_t *dev, uint16_t samples);
static void icm20602_accel_configure(icm20602_t *dev);
static void icm20602_gyro_configure(icm20602_t *dev);
static void icm20602_fifo_reset(icm20602_t *dev);
static void icm20602_temp_process(icm20602_t *dev);
static void icm20602_gyro_process(icm20602_t *dev);
static void icm20602_accel_process(icm20602_t *dev);
static void icm20602_stat(icm20602_t *dev);

void icm20602_drdy_handler(void *area);

// Public functions
int icm20602_start(spi_t *spi, gpio_t *cs, exti_t *drdy, uint32_t period,
                   uint32_t thread_priority) {
    if (spi == NULL || cs == NULL) {
        return -1;
    }

    icm20602_t *dev = calloc(1, sizeof(icm20602_t));

    if (dev == NULL) {
        return -1;
    }

    strcpy(dev->name, "ICM20602");
    dev->interface.spi = spi;
    dev->interface.cs = cs;

    if (drdy != NULL) {
        dev->interface.drdy = drdy;
        if (exti_init(dev->interface.drdy, icm20602_drdy_handler, dev)) {
            return -1;
        }
        dev->sync.drdy_sem = xSemaphoreCreateBinary();
        if (dev->sync.drdy_sem == NULL) {
            return -1;
        }
    }

    if (period < 2) {
        LOG_ERROR(dev->name, "Too high frequency");
        return -1;
    }

    dev->os.period = period / portTICK_PERIOD_MS;

    dev->sync.measrdy_sem = xSemaphoreCreateBinary();
    if (dev->sync.measrdy_sem == NULL) {
        return -1;
    }

    if (xTaskCreate(icm20602_thread, dev->name, 512, dev, dev->os.priority,
                    NULL) != pdTRUE) {
        LOG_ERROR(dev->name, "Thread start error");
        return -1;
    }

    LOG_DEBUG(dev->name, "Start service, period = %u ms", period);

    xSemaphoreTake(dev->sync.measrdy_sem, 0);

    return 0;
}

void icm20602_thread(void *dev_ptr) {
    icm20602_t *dev = dev_ptr;
    TickType_t last_wake_time;

    last_wake_time = xTaskGetTickCount();
    dev->state = icm20602_RESET;

    while (1) {
        icm20602_fsm(dev);
        xTaskDelayUntil(&last_wake_time, dev->os.period);
    }
}

void icm20602_fsm(icm20602_t *dev) {
    switch (dev->state) {
        case icm20602_RESET:
            vTaskDelay(100);
            icm20602_write_reg(dev, PWR_MGMT_1, DEVICE_RESET);
            dev->state = icm20602_RESET_WAIT;
            vTaskDelay(100);
            break;

        case icm20602_RESET_WAIT:
            if ((icm20602_read_reg(dev, WHO_AM_I) == WHOAMI) &&
                (icm20602_read_reg(dev, PWR_MGMT_1) == 0x41) &&
                (icm20602_read_reg(dev, CONFIG) == 0x80)) {
                icm20602_write_reg(dev, PWR_MGMT_1, CLKSEL_0);
                icm20602_write_reg(dev, I2C_IF, I2C_IF_DIS);
                icm20602_write_reg(dev, SIGNAL_PATH_RESET,
                                   ACCEL_RST | TEMP_RST);
                icm20602_write_reg(dev, SIG_COND_RST, 0);
                dev->state = icm20602_CONF;
            } else {
                dev->state = icm20602_RESET;
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = icm20602_FAIL;
                    LOG_ERROR(dev->name,
                              "Wrong default registers values after reset");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case icm20602_CONF:
            if (icm20602_configure(dev)) {
                icm20602_fifo_reset(dev);
                LOG_INFO(dev->name, "Initialization successful");
                dev->state = icm20602_FIFO_READ;
            } else {
                LOG_ERROR(dev->name, "Failed configuration, retrying...");
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = icm20602_RESET;
                    LOG_ERROR(dev->name, "Failed configuration, reset...");
                    dev->attempt = 0;
                }
            }
            break;

        case icm20602_FIFO_READ:
            if (dev->interface.drdy != NULL) {
                xSemaphoreTake(dev->sync.drdy_sem, portMAX_DELAY);
            }
            icm20602_fifo_read(dev, 1);
            icm20602_temp_process(dev);
            icm20602_accel_process(dev);
            icm20602_gyro_process(dev);
            static int count = 0;
            count++;
            if (count > 2000) {
                icm20602_stat(dev);
                count = 0;
            }
            if (dev->interface.drdy != NULL) {
                irq_enable(dev->interface.drdy->p.id);
            }
            xSemaphoreGive(dev->sync.measrdy_sem);
            break;

        case icm20602_FAIL:
            vTaskDelete(NULL);
            return;

        default:
            break;
    }
}

void icm20602_drdy_handler(void *area) {
    icm20602_t *dev = (icm20602_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(dev->sync.drdy_sem, &xHigherPriorityTaskWoken);

    HAL_EXTI_ClearPending((EXTI_HandleTypeDef *)&dev->interface.drdy->handle,
                          EXTI_TRIGGER_RISING);
    irq_disable(dev->interface.drdy->p.id);

    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void icm20602_get_meas_non_block(icm20602_t *dev, void *ptr) {
    memcpy(ptr, (void *)&dev, sizeof(icm20602_meas_t));
}

void icm20602_get_meas_block(icm20602_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    icm20602_get_meas_non_block(dev, ptr);
}

// Private functions
static void icm20602_chip_select(icm20602_t *dev) {
    spi_chip_select(dev->interface.spi, dev->interface.cs);
}

static void icm20602_chip_deselect(icm20602_t *dev) {
    spi_chip_deselect(dev->interface.spi, dev->interface.cs);
}

static void icm20602_exchange(icm20602_t *dev, uint8_t *tx_buf, uint8_t *rx_buf,
                              uint16_t length) {
    icm20602_chip_select(dev);
    spi_transmit_receive(dev->interface.spi, tx_buf, rx_buf, length);
    icm20602_chip_deselect(dev);
}

static uint8_t icm20602_read_reg(icm20602_t *dev, uint8_t reg) {
    uint8_t data[2];
    data[0] = reg | READ;
    data[1] = 0;
    icm20602_exchange(dev, data, data, sizeof(data));
    return data[1];
}

static void icm20602_write_reg(icm20602_t *dev, uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg | WRITE;
    data[1] = value;
    icm20602_exchange(dev, data, data, sizeof(data));
}

static void icm20602_set_clear_reg(icm20602_t *dev, uint8_t reg,
                                   uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = icm20602_read_reg(dev, reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        icm20602_write_reg(dev, reg, val);
    }
}

static int icm20602_configure(icm20602_t *dev) {
    uint8_t orig_val;
    int rv = 1;

    for (int i = 0; i < SIZE_REG_CFG; i++) {
        icm20602_set_clear_reg(dev, reg_cfg[i].reg, reg_cfg[i].setbits,
                               reg_cfg[i].clearbits);
    }

    // Check BANK_0
    for (int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = icm20602_read_reg(dev, reg_cfg[i].reg);

        if ((orig_val & reg_cfg[i].setbits) != reg_cfg[i].setbits) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not set)",
                      (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if ((orig_val & reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not cleared)",
                      (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    // Set scale and range for processing
    icm20602_accel_configure(dev);
    icm20602_gyro_configure(dev);

    // Enable EXTI IRQ for DataReady pin
    if (dev->interface.drdy != NULL) {
        irq_enable(dev->interface.drdy->p.id);
    }

    return rv;
}

static void icm20602_accel_configure(icm20602_t *dev) {
    const uint8_t ACCEL_FS_SEL =
        icm20602_read_reg(dev, ACCEL_CONFIG) & (BIT4 | BIT3);

    if (ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        dev->meas_param.accel_scale = (CONST_G / 16384.f);
        dev->meas_param.accel_range = (2.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        dev->meas_param.accel_scale = (CONST_G / 8192.f);
        dev->meas_param.accel_range = (4.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        dev->meas_param.accel_scale = (CONST_G / 4096.f);
        dev->meas_param.accel_range = (8.f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        dev->meas_param.accel_scale = (CONST_G / 2048.f);
        dev->meas_param.accel_range = (16.f * CONST_G);
    }
}

static void icm20602_gyro_configure(icm20602_t *dev) {
    const uint8_t FS_SEL =
        icm20602_read_reg(dev, GYRO_CONFIG) & (BIT4 | BIT3);

    if (FS_SEL == FS_SEL_250_DPS) {
        dev->meas_param.gyro_range = 250.f;
    } else if (FS_SEL == FS_SEL_500_DPS) {
        dev->meas_param.gyro_range = 500.f;
    } else if (FS_SEL == FS_SEL_1000_DPS) {
        dev->meas_param.gyro_range = 1000.f;
    } else if (FS_SEL == FS_SEL_2000_DPS) {
        dev->meas_param.gyro_range = 2000.f;
    }

    dev->meas_param.gyro_scale = (dev->meas_param.gyro_range / 32768.f);
}

static int icm20602_fifo_read(icm20602_t *dev, uint16_t samples) {
    if (samples > 1) {
        dev->fifo_buffer.CMD = FIFO_COUNTH | READ;
        icm20602_exchange(dev, (uint8_t *)&dev->fifo_buffer,
                          (uint8_t *)&dev->fifo_buffer, 3);

        dev->fifo_param.bytes =
            MIN(msblsb16(dev->fifo_buffer.COUNTH, dev->fifo_buffer.COUNTL),
                samples);
        dev->fifo_param.samples =
            dev->fifo_param.bytes / sizeof(icm20602_fifo_t);

        dev->fifo_buffer.CMD = FIFO_COUNTH | READ;
        icm20602_exchange(dev, (uint8_t *)&dev->fifo_buffer,
                          (uint8_t *)&dev->fifo_buffer,
                          dev->fifo_param.bytes + 3);
    } else {
        dev->fifo_buffer.COUNTL = ACCEL_XOUT_H | READ;
        dev->fifo_param.samples = 1;
        dev->fifo_param.bytes = sizeof(icm20602_fifo_t);
        icm20602_exchange(dev, (uint8_t *)&dev->fifo_buffer.COUNTL,
                          (uint8_t *)&dev->fifo_buffer.COUNTL,
                          dev->fifo_param.bytes + 1);
    }

    return 0;
}

static void icm20602_fifo_reset(icm20602_t *dev) {
    icm20602_write_reg(dev, FIFO_EN, 0);
    icm20602_set_clear_reg(dev, USER_CTRL, USR_CTRL_FIFO_RST, USR_CTRL_FIFO_EN);
}

static void icm20602_accel_process(icm20602_t *dev) {
    for (int i = 0; i < dev->fifo_param.samples; i++) {
        int16_t accel_x = msblsb16(dev->fifo_buffer.buf[i].ACCEL_XOUT_H,
                                   dev->fifo_buffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msblsb16(dev->fifo_buffer.buf[i].ACCEL_YOUT_H,
                                   dev->fifo_buffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msblsb16(dev->fifo_buffer.buf[i].ACCEL_ZOUT_H,
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

static void icm20602_gyro_process(icm20602_t *dev) {
    for (int i = 0; i < dev->fifo_param.samples; i++) {
        int16_t gyro_x = msblsb16(dev->fifo_buffer.buf[i].GYRO_XOUT_H,
                                  dev->fifo_buffer.buf[i].GYRO_XOUT_L);
        int16_t gyro_y = msblsb16(dev->fifo_buffer.buf[i].GYRO_YOUT_H,
                                  dev->fifo_buffer.buf[i].GYRO_YOUT_L);
        int16_t gyro_z = msblsb16(dev->fifo_buffer.buf[i].GYRO_ZOUT_H,
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

static void icm20602_temp_process(icm20602_t *dev) {
    uint8_t data[3];
    data[0] = TEMP_OUT_H | READ;

    icm20602_exchange(dev, data, data, sizeof(data));

    int16_t temp_raw = msblsb16(data[1], data[2]);
    dev->meas_buffer.temp =
        (temp_raw - TEMP_SENS * TEMP_OFFSET) / TEMP_SENS + TEMP_OFFSET;
}

static void icm20602_stat(icm20602_t *dev) {
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
