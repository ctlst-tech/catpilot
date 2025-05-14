#include "icm45686.h"
#include "icm45686_reg.h"

// Private functions
static void icm45686_fsm(void *area);
static int icm45686_configure(icm45686_t *dev);
static int icm45686_reset_fifo(icm45686_t *dev);
static void icm45686_chip_select(icm45686_t *dev);
static void icm45686_chip_deselect(icm45686_t *dev);
static uint8_t icm45686_read_reg(icm45686_t *dev, uint8_t reg);
static void icm45686_write_reg(icm45686_t *dev, uint8_t reg, uint8_t value);
static void icm45686_set_clear_reg(icm45686_t *dev, uint8_t reg,
                                  uint8_t setbits, uint8_t clearbits);
static int icm45686_fifo_read(icm45686_t *dev, uint16_t samples);
static void icm45686_accel_configure(icm45686_t *dev);
static void icm45686_gyro_configure(icm45686_t *dev);
static void icm45686_fifo_reset(icm45686_t *dev);
static void icm45686_temp_process(icm45686_t *dev);
static void icm45686_gyro_process(icm45686_t *dev);
static void icm45686_accel_process(icm45686_t *dev);
static void icm45686_update_meas(icm45686_t *dev);

static void icm45686_exchange(icm45686_t *dev, uint8_t *tx_buf,
                                uint8_t *rx_buf, uint16_t length) 
{
    icm45686_chip_select(dev);
    spi_transmit_receive(dev->interface.spi, tx_buf, rx_buf, length);
    icm45686_chip_deselect(dev);
}

// Public functions
icm45686_t *icm45686_start(char *name, uint32_t period, uint32_t priority,
                          spi_t *spi, gpio_t *cs) {
    // Validate input parameters
    if (spi == NULL || cs == NULL || name == NULL || period <= 0 ||
        priority <= 0) {
        LOG_ERROR("ICM45686", "Invalid input parameters");
        return NULL;
    }

    // Allocate device structure
    icm45686_t *dev = calloc(1, sizeof(icm45686_t));
    if (dev == NULL) {
        LOG_ERROR(name, "Failed to allocate memory");
        return NULL;
    }

    // Initialize device structure
    strncpy(dev->name, name, MAX_NAME_LEN - 1);
    dev->interface.spi = spi;
    dev->interface.cs = cs;

    // Create measurement ready semaphore
    dev->sync.measrdy_sem = xSemaphoreCreateBinary();
    if (dev->sync.measrdy_sem == NULL) {
        LOG_ERROR(dev->name, "Failed to create measurement ready semaphore");
        free(dev);
        return NULL;
    }

    // Create mutex for thread-safe access
    dev->sync.mutex = xSemaphoreCreateMutex();
    if (dev->sync.mutex == NULL) {
        LOG_ERROR(dev->name, "Failed to create mutex");
        vSemaphoreDelete(dev->sync.measrdy_sem);
        free(dev);
        return NULL;
    }

    // Initialize device state
    dev->state = ICM45686_RESET;
    dev->attempt = 0;

    // Start service task
    if ((dev->service = service_start(dev->name, dev, icm45686_fsm, period,
                                    priority)) == NULL) {
        LOG_ERROR(dev->name, "Failed to start service task");
        vSemaphoreDelete(dev->sync.measrdy_sem);
        vSemaphoreDelete(dev->sync.mutex);
        free(dev);
        return NULL;
    }

    // Wait for first measurement
    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    return dev;
}

static void icm45686_fsm(void *area) {
    icm45686_t *dev = (icm45686_t *)area;
    switch (dev->state) {
        case ICM45686_RESET:
            // Software reset configuration
            icm45686_write_reg(dev, REG_MISC2, SOFT_RST);
            dev->state = ICM45686_RESET_WAIT;
            vTaskDelay(pdMS_TO_TICKS(2));  // wait 2 ms for soft reset to be effective
            break;

        case ICM45686_RESET_WAIT:
            if ((icm45686_read_reg(dev, WHO_AM_I) == WHOAMI) && 
                ((icm45686_read_reg(dev, REG_MISC2) & BIT1) == 0x0)) {
                
                // Wakeup accel and gyro
                icm45686_write_reg(dev, PWR_MGMT0, 
                                 GYRO_MODE_LN | ACCEL_MODE_LN);  // Low noise mode for both
                dev->state = ICM45686_CONF;
                vTaskDelay(pdMS_TO_TICKS(30));  // 30 ms gyro startup, 10 ms accel

            } else {
                // RESET not complete
                LOG_DEBUG(dev->name, "Reset failed, retrying");
                dev->state = ICM45686_RESET;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            break;

        case ICM45686_CONF:
            if (icm45686_configure(dev)) {
                // if configure succeeded then reset the FIFO
                icm45686_fifo_reset(dev);
                LOG_INFO(dev->name, "Initialization successful");
                dev->state = ICM45686_FIFO_READ;
                vTaskDelay(pdMS_TO_TICKS(1));
            } else {
                LOG_DEBUG(dev->name, "Configure failed, resetting");
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = ICM45686_RESET;
                    LOG_ERROR(dev->name, "Failed configuration");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case ICM45686_FIFO_READ: {
            icm45686_fifo_read(dev, 1);
            icm45686_temp_process(dev);
            icm45686_accel_process(dev);
            icm45686_gyro_process(dev);
            icm45686_update_meas(dev);
            xSemaphoreGive(dev->sync.measrdy_sem);
            break;
        }

        default:
            dev->state = ICM45686_RESET;
            break;
    }
}

static void icm45686_chip_select(icm45686_t *dev) {
    gpio_reset(dev->interface.cs);
}

static void icm45686_chip_deselect(icm45686_t *dev) {
    gpio_set(dev->interface.cs);
}

static uint8_t icm45686_read_reg(icm45686_t *dev, uint8_t reg) {
    uint8_t data[2];
    data[0] = reg | READ;
    data[1] = 0;

    icm45686_chip_select(dev);
    spi_transmit_receive(dev->interface.spi, data, data, sizeof(data));
    icm45686_chip_deselect(dev);

    return data[1];
}

static void icm45686_write_reg(icm45686_t *dev, uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg & ~READ;
    data[1] = value;

    icm45686_chip_select(dev);
    spi_transmit_receive(dev->interface.spi, data, data, sizeof(data));
    icm45686_chip_deselect(dev);
}

static void icm45686_set_clear_reg(icm45686_t *dev, uint8_t reg,
                                  uint8_t setbits, uint8_t clearbits) {
    uint8_t value = icm45686_read_reg(dev, reg);
    value &= ~clearbits;
    value |= setbits;
    icm45686_write_reg(dev, reg, value);
}

static int icm45686_configure(icm45686_t *dev) {
    uint8_t reg_value;
    
    // Apply register configuration
    for (int i = 0; i < SIZE_REG_CFG; i++) {
        icm45686_set_clear_reg(dev, reg_cfg[i].reg, reg_cfg[i].setbits,
                              reg_cfg[i].clearbits);
        
        // Verify register write
        reg_value = icm45686_read_reg(dev, reg_cfg[i].reg);
        uint8_t expected = (reg_value & ~reg_cfg[i].clearbits) | reg_cfg[i].setbits;
        if (reg_value != expected) {
            LOG_ERROR(dev->name, "Register 0x%02X configuration failed", reg_cfg[i].reg);
            return 0;
        }
    }

    // Configure accelerometer and gyroscope
    icm45686_accel_configure(dev);
    icm45686_gyro_configure(dev);

    // Verify WHO_AM_I one more time
    if (icm45686_read_reg(dev, WHO_AM_I) != WHOAMI) {
        LOG_ERROR(dev->name, "WHO_AM_I verification failed after configuration");
        return 0;
    }

    return 1;
}

static void icm45686_accel_configure(icm45686_t *dev) {
    const uint8_t ACCEL_FS_SEL =
        icm45686_read_reg(dev, ACCEL_CONFIG0) & (BIT4 | BIT5 | BIT6);

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
    } else if (ACCEL_FS_SEL == ACCEL_FS_SEL_32G) {
        dev->meas_param.accel_scale = (CONST_G / 2048.0f);
        dev->meas_param.accel_range = (32.0f * CONST_G);
    }
}

static void icm45686_gyro_configure(icm45686_t *dev) {
    const uint8_t GYRO_FS_SEL =
        icm45686_read_reg(dev, GYRO_CONFIG0) & (BIT4 | BIT5 | BIT6);

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

static int icm45686_fifo_read(icm45686_t *dev, uint16_t samples) {
    if (samples > 1) {
        // dev->fifo_buffer.CMD = FIFO_COUNTH | READ;
        // icm45686_exchange(dev, (uint8_t *)&dev->fifo_buffer,
        //                   (uint8_t *)&dev->fifo_buffer, 3);

        // dev->fifo_param.bytes =
        //     MIN(msb_lsb_16(dev->fifo_buffer.COUNTH, dev->fifo_buffer.COUNTL),
        //         samples);
        // dev->fifo_param.samples =
        //     dev->fifo_param.bytes / sizeof(icm45686_fifo_t);

        // dev->fifo_buffer.CMD = FIFO_COUNTH | READ;
        // icm45686_exchange(dev, (uint8_t *)&dev->fifo_buffer,
        //                   (uint8_t *)&dev->fifo_buffer,
        //                   dev->fifo_param.bytes + 3);
        //TODO
    } 
    else
    {
        dev->fifo_buffer.COUNTL = ACCEL_DATA_X1_UI | READ;
        dev->fifo_param.samples = 1;
        dev->fifo_param.bytes = sizeof(icm45686_fifo_t);
        icm45686_exchange(dev, (uint8_t *)&dev->fifo_buffer.COUNTL,
                          (uint8_t *)&dev->fifo_buffer.COUNTL,
                          dev->fifo_param.bytes + 1);
    }
    return 0;
}

static void icm45686_fifo_reset(icm45686_t *dev) {
    icm45686_write_reg(dev, FIFO_CONFIG2, 0x80 | 0x20); // Reset FIFO. Datasheet p.17.31
    icm45686_write_reg(dev, FIFO_CONFIG2, 0x00 | 0x20); // Clear reset bit
}

static void icm45686_accel_process(icm45686_t *dev) {
    for (int i = 0; i < dev->fifo_param.samples; i++) {
        int16_t accel_x = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_XOUT_L,
                                     dev->fifo_buffer.buf[i].ACCEL_XOUT_H);
        int16_t accel_y = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_YOUT_L,
                                     dev->fifo_buffer.buf[i].ACCEL_YOUT_H);
        int16_t accel_z = msb_lsb_16(dev->fifo_buffer.buf[i].ACCEL_ZOUT_L,
                                     dev->fifo_buffer.buf[i].ACCEL_ZOUT_H);

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

static void icm45686_gyro_process(icm45686_t *dev) {
    for (int i = 0; i < dev->fifo_param.samples; i++) {
        int16_t gyro_x = msb_lsb_16(dev->fifo_buffer.buf[i].GYRO_XOUT_L,
                                    dev->fifo_buffer.buf[i].GYRO_XOUT_H);
        int16_t gyro_y = msb_lsb_16(dev->fifo_buffer.buf[i].GYRO_YOUT_L,
                                    dev->fifo_buffer.buf[i].GYRO_YOUT_H);
        int16_t gyro_z = msb_lsb_16(dev->fifo_buffer.buf[i].GYRO_ZOUT_L,
                                    dev->fifo_buffer.buf[i].GYRO_ZOUT_H);

        dev->meas_buffer.meas[i].gyro_x = gyro_x * dev->meas_param.gyro_scale;
        dev->meas_buffer.meas[i].gyro_y =
            ((gyro_y == INT16_MIN) ? INT16_MAX : -gyro_y) *
            dev->meas_param.gyro_scale;
        dev->meas_buffer.meas[i].gyro_z =
            ((gyro_z == INT16_MIN) ? INT16_MAX : -gyro_z) *
            dev->meas_param.gyro_scale;
    }
}

static void icm45686_temp_process(icm45686_t *dev) {
    uint8_t data[3];
    data[1] = icm45686_read_reg(dev, TEMP_DATA1_UI);
    data[2] = icm45686_read_reg(dev, TEMP_DATA0_UI);
    int16_t temp_raw = msb_lsb_16(data[2], data[1]);
    dev->meas_buffer.temp = (temp_raw) / TEMP_45686_SENS + TEMP_45686_OFFSET;
}

static void icm45686_update_meas(icm45686_t *dev) {
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    
    // Get raw values first
    accel_x = dev->meas_buffer.meas[0].accel_x;
    accel_y = dev->meas_buffer.meas[0].accel_y;
    accel_z = dev->meas_buffer.meas[0].accel_z;
    gyro_x = dev->meas_buffer.meas[0].gyro_x;
    gyro_y = dev->meas_buffer.meas[0].gyro_y;
    gyro_z = dev->meas_buffer.meas[0].gyro_z;

    // Apply rotations
    switch (dev->rotation) {
        case ROTATION_ROLL_180_YAW_90:
            xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
            dev->meas.accel_x = -accel_y;
            dev->meas.accel_y = -accel_x;
            dev->meas.accel_z = -accel_z;
            dev->meas.gyro_x = -gyro_y;
            dev->meas.gyro_y = -gyro_x;
            dev->meas.gyro_z = -gyro_z;
            xSemaphoreGive(dev->sync.mutex);
            break;

        case ROTATION_YAW_270:
            xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
            dev->meas.accel_x = accel_y;
            dev->meas.accel_y = -accel_x;
            dev->meas.accel_z = accel_z;
            dev->meas.gyro_x = gyro_y;
            dev->meas.gyro_y = -gyro_x;
            dev->meas.gyro_z = gyro_z;
            xSemaphoreGive(dev->sync.mutex);
            break;

        case ROTATION_NONE:
        default:
            xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
            dev->meas.accel_x = accel_x;
            dev->meas.accel_y = accel_y;
            dev->meas.accel_z = accel_z;
            dev->meas.gyro_x = gyro_x;
            dev->meas.gyro_y = gyro_y;
            dev->meas.gyro_z = gyro_z;
            xSemaphoreGive(dev->sync.mutex);
            break;
    }
}

// Add new function to set rotation
void icm45686_set_rotation(icm45686_t *dev, icm45686_rotation_t rotation) {
    if (dev != NULL) {
        xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
        dev->rotation = rotation;
        xSemaphoreGive(dev->sync.mutex);
    }
}

void icm45686_get_meas_block(icm45686_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    memcpy(ptr, &dev->meas, sizeof(icm45686_meas_t));
    xSemaphoreGive(dev->sync.mutex);
}

void icm45686_get_meas_non_block(icm45686_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    memcpy(ptr, &dev->meas, sizeof(icm45686_meas_t));
    xSemaphoreGive(dev->sync.mutex);
}

void icm45686_stat(icm45686_t *dev) {
    if (dev == NULL || dev->state != ICM45686_FIFO_READ) {
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
