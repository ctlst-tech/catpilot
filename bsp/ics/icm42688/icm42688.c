#include "icm42688.h"

#include "icm42688_reg.h"

// Private functions
static void icm42688_fsm(void *area);
static int icm42688_configure(icm42688_t *dev);
static int icm42688_reset_fifo(icm42688_t *dev);
static void icm42688_chip_select(icm42688_t *dev);
static void icm42688_chip_deselect(icm42688_t *dev);
static uint8_t icm42688_read_reg(icm42688_t *dev, uint8_t reg);
static void icm42688_write_reg(icm42688_t *dev, uint8_t reg, uint8_t value);
static void icm42688_set_clear_reg(icm42688_t *dev, uint8_t reg,
                                   uint8_t setbits, uint8_t clearbits);
static int icm42688_fifo_read(icm42688_t *dev, uint16_t samples);
static void icm42688_accel_configure(icm42688_t *dev);
static void icm42688_gyro_configure(icm42688_t *dev);
static void icm42688_fifo_reset(icm42688_t *dev);
static void icm42688_temp_process(icm42688_t *dev);
static void icm42688_gyro_process(icm42688_t *dev);
static void icm42688_accel_process(icm42688_t *dev);
static void icm42688_update_meas(icm42688_t *dev);

void icm42688_drdy_handler(void *area);

// Public functions
icm42688_t *icm42688_start(char *name, uint32_t period, uint32_t priority,
                           spi_t *spi, gpio_t *cs, exti_t *drdy) {
    if (spi == NULL || cs == NULL || name == NULL || period <= 0 ||
        priority <= 0) {
        return NULL;
    }

    icm42688_t *dev = calloc(1, sizeof(icm42688_t));

    if (dev == NULL) {
        return NULL;
    }

    strncpy(dev->name, name, MAX_NAME_LEN - 1);

    dev->interface.spi = spi;
    dev->interface.cs = cs;

    if (drdy != NULL) {
        dev->interface.drdy = drdy;
        if (exti_init(dev->interface.drdy, icm42688_drdy_handler, dev)) {
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
    if (dev->sync.mutex == NULL) {
        return NULL;
    }

    dev->state = ICM42688_RESET;

    if ((dev->service = service_start(dev->name, dev, icm42688_fsm, period,
                                      priority)) == NULL) {
        LOG_ERROR(dev->name, "Fatal error");
        return NULL;
    }

    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);

    return dev;
}

void icm42688_fsm(void *area) {
    icm42688_t *dev = (icm42688_t *)area;
    switch (dev->state) {
        case ICM42688_RESET:
            icm42688_write_reg(dev, DEVICE_CONFIG, PWR_MGMT_RESET);
            dev->state = ICM42688_RESET_WAIT;
            vTaskDelay(100);
            break;

        case ICM42688_RESET_WAIT:
            if (icm42688_read_reg(dev, WHO_AM_I) == WHOAMI) {
                dev->state = ICM42688_CONF;
            } else {
                dev->state = ICM42688_RESET;
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = ICM42688_FAIL;
                    LOG_ERROR(dev->name, "Wrong WHO_AM_I value after reset");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case ICM42688_CONF:
            if (icm42688_configure(dev)) {
                icm42688_fifo_reset(dev);
                LOG_INFO(dev->name, "Initialization successful");
                dev->state = ICM42688_FIFO_READ;
            } else {
                LOG_WARN(dev->name, "Failed configuration, retrying");
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = ICM42688_RESET;
                    LOG_ERROR(dev->name, "Failed configuration");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case ICM42688_FIFO_READ:
            if (dev->interface.drdy != NULL) {
                xSemaphoreTake(dev->sync.drdy_sem, portMAX_DELAY);
            }
            
            icm42688_fifo_read(dev, 1);
            icm42688_accel_process(dev);
            icm42688_gyro_process(dev);
            icm42688_temp_process(dev);
            icm42688_update_meas(dev);
            xSemaphoreGive(dev->sync.measrdy_sem);
            break;

        case ICM42688_FAIL:
            vTaskDelay(1000);
            break;

        default:
            dev->state = ICM42688_RESET;
            break;
    }
}

void icm42688_drdy_handler(void *area) {
    icm42688_t *dev = (icm42688_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(dev->sync.drdy_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void icm42688_get_meas_block(icm42688_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    memcpy(ptr, &dev->meas, sizeof(icm42688_meas_t));
    xSemaphoreGive(dev->sync.mutex);
}

void icm42688_get_meas_non_block(icm42688_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    memcpy(ptr, &dev->meas, sizeof(icm42688_meas_t));
    xSemaphoreGive(dev->sync.mutex);
}

void icm42688_stat(icm42688_t *dev) {
    LOG_INFO(dev->name, "State: %d", dev->state);
    LOG_INFO(dev->name, "Accel range: %.2f", dev->meas_param.accel_range);
    LOG_INFO(dev->name, "Gyro range: %.2f", dev->meas_param.gyro_range);
}

// Private functions
static void icm42688_chip_select(icm42688_t *dev) {
    gpio_reset(dev->interface.cs);
}

static void icm42688_chip_deselect(icm42688_t *dev) {
    gpio_set(dev->interface.cs);
}

static uint8_t icm42688_read_reg(icm42688_t *dev, uint8_t reg) {
    uint8_t data[2];
    data[0] = reg | READ;
    data[1] = 0;

    icm42688_chip_select(dev);
    spi_transmit_receive(dev->interface.spi, data, data, sizeof(data));
    icm42688_chip_deselect(dev);

    return data[1];
}

static void icm42688_write_reg(icm42688_t *dev, uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg & ~READ;
    data[1] = value;

    icm42688_chip_select(dev);
    spi_transmit_receive(dev->interface.spi, data, data, sizeof(data));
    icm42688_chip_deselect(dev);
}

static void icm42688_set_clear_reg(icm42688_t *dev, uint8_t reg,
                                   uint8_t setbits, uint8_t clearbits) {
    uint8_t value = icm42688_read_reg(dev, reg);
    value &= ~clearbits;
    value |= setbits;
    icm42688_write_reg(dev, reg, value);
}

static int icm42688_configure(icm42688_t *dev) {
    for (int i = 0; i < SIZE_REG_CFG; i++) {
        icm42688_set_clear_reg(dev, reg_cfg[i].reg, reg_cfg[i].setbits,
                               reg_cfg[i].clearbits);
    }

    icm42688_accel_configure(dev);
    icm42688_gyro_configure(dev);

    return 1;
}

static void icm42688_accel_configure(icm42688_t *dev) {
    const uint8_t ACCEL_FS_SEL =
        icm42688_read_reg(dev, ACCEL_CONFIG0) & (BIT5 | BIT6);

    if (ACCEL_FS_SEL == ACCEL_FS_SEL_2G) {
        dev->meas_param.accel_scale = (CONST_G / 16384.0f);
        dev->meas_param.accel_range = (2.0f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_4G) {
        dev->meas_param.accel_scale = (CONST_G / 8192.0f);
        dev->meas_param.accel_range = (4.0f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_8G) {
        dev->meas_param.accel_scale = (CONST_G / 4096.0f);
        dev->meas_param.accel_range = (8.0f * CONST_G);
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_16G) {
        dev->meas_param.accel_scale = (CONST_G / 2048.0f);
        dev->meas_param.accel_range = (16.0f * CONST_G);
    }
}

static void icm42688_gyro_configure(icm42688_t *dev) {
    const uint8_t GYRO_FS_SEL =
        icm42688_read_reg(dev, GYRO_CONFIG0) & (BIT5 | BIT6);

    if (GYRO_FS_SEL == GYRO_FS_SEL_250DPS) {
        dev->meas_param.gyro_range = 250.0f;
    } else if (GYRO_FS_SEL == GYRO_FS_SEL_500DPS) {
        dev->meas_param.gyro_range = 500.0f;
    } else if (GYRO_FS_SEL == GYRO_FS_SEL_1000DPS) {
        dev->meas_param.gyro_range = 1000.0f;
    } else if (GYRO_FS_SEL == GYRO_FS_SEL_2000DPS) {
        dev->meas_param.gyro_range = 2000.0f;
    }

    dev->meas_param.gyro_scale = (dev->meas_param.gyro_range / 32768.0f);
}

static int icm42688_fifo_read(icm42688_t *dev, uint16_t samples) {
    if (samples > 1) {
        dev->fifo_buffer.CMD = FIFO_COUNTH | READ;
        icm42688_chip_select(dev);
        spi_transmit_receive(dev->interface.spi, (uint8_t *)&dev->fifo_buffer,
                    (uint8_t *)&dev->fifo_buffer, 3);
        icm42688_chip_deselect(dev);

        dev->fifo_param.bytes =
            MIN(msb_lsb_16(dev->fifo_buffer.COUNTH, dev->fifo_buffer.COUNTL),
                samples * sizeof(icm42688_fifo_t));
        dev->fifo_param.samples =
            dev->fifo_param.bytes / sizeof(icm42688_fifo_t);

        dev->fifo_buffer.CMD = FIFO_DATA | READ;
        icm42688_chip_select(dev);
        spi_transmit_receive(dev->interface.spi, (uint8_t *)&dev->fifo_buffer,
                    (uint8_t *)&dev->fifo_buffer,
                    dev->fifo_param.bytes + 3);
        icm42688_chip_deselect(dev);
    } else {
        dev->fifo_buffer.COUNTL = ACCEL_DATA_X1 | READ;
        dev->fifo_param.samples = 1;
        dev->fifo_param.bytes = sizeof(icm42688_fifo_t);
        icm42688_chip_select(dev);
        spi_transmit_receive(dev->interface.spi, (uint8_t *)&dev->fifo_buffer.COUNTL,
                    (uint8_t *)&dev->fifo_buffer.COUNTL,
                    dev->fifo_param.bytes + 1);
        icm42688_chip_deselect(dev);
    }

    return 0;
}

static void icm42688_fifo_reset(icm42688_t *dev) {
    icm42688_write_reg(dev, FIFO_CONFIG, 0x80); // Reset FIFO
    icm42688_write_reg(dev, FIFO_CONFIG, 0x00); // Clear reset bit
}

static void icm42688_accel_process(icm42688_t *dev) {
    for (int i = 0; i < dev->fifo_param.samples; i++) {
        int16_t accel_x = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_XOUT_H,
                                    dev->fifo_buffer.buf[i].ACCEL_XOUT_L);
        int16_t accel_y = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_YOUT_H,
                                    dev->fifo_buffer.buf[i].ACCEL_YOUT_L);
        int16_t accel_z = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_ZOUT_H,
                                    dev->fifo_buffer.buf[i].ACCEL_ZOUT_L);

        dev->meas_buffer.meas[i].accel_x = accel_x * dev->meas_param.accel_scale;
        dev->meas_buffer.meas[i].accel_y =
            ((accel_y == INT16_MIN) ? INT16_MAX : -accel_y) *
            dev->meas_param.accel_scale;
        dev->meas_buffer.meas[i].accel_z =
            ((accel_z == INT16_MIN) ? INT16_MAX : -accel_z) *
            dev->meas_param.accel_scale;
    }
}

static void icm42688_gyro_process(icm42688_t *dev) {
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

static void icm42688_temp_process(icm42688_t *dev) {
    uint8_t data[3];
    data[0] = TEMP_DATA1 | READ;

    icm42688_chip_select(dev);
    spi_transmit_receive(dev->interface.spi, data, data, sizeof(data));
    icm42688_chip_deselect(dev);

    int16_t temp_raw = msb_lsb_16(data[1], data[2]);
    dev->meas_buffer.temp =
        (temp_raw - TEMP_SENS * TEMP_OFFSET) / TEMP_SENS + TEMP_OFFSET;
}

static void icm42688_update_meas(icm42688_t *dev) {
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    dev->meas.accel_x = dev->meas_buffer.meas[0].accel_x;
    dev->meas.accel_y = dev->meas_buffer.meas[0].accel_y;
    dev->meas.accel_z = dev->meas_buffer.meas[0].accel_z;
    dev->meas.gyro_x = dev->meas_buffer.meas[0].gyro_x;
    dev->meas.gyro_y = dev->meas_buffer.meas[0].gyro_y;
    dev->meas.gyro_z = dev->meas_buffer.meas[0].gyro_z;
    xSemaphoreGive(dev->sync.mutex);
}
