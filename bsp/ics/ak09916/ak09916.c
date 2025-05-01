#include "ak09916.h"
#include "ak09916_reg.h"

// Private functions
static void ak09916_fsm(void *area);
static int ak09916_configure(ak09916_t *dev);
static uint8_t ak09916_read_reg(ak09916_t *dev, uint8_t reg);
static void ak09916_write_reg(ak09916_t *dev, uint8_t reg, uint8_t value);
static void ak09916_set_clear_reg(ak09916_t *dev, uint8_t reg, uint8_t setbits,
                                 uint8_t clearbits);
static void ak09916_read_meas(ak09916_t *dev);
static int ak09916_process_meas(ak09916_t *dev);
static void ak09916_update_meas(ak09916_t *dev);

// Public functions
ak09916_t *ak09916_start(char *name, uint32_t period, uint32_t priority,
                         i2c_t *i2c, int isolated, uint8_t rotation) {
    if (i2c == NULL || name == NULL || period <= 0 || priority <= 0) {
        return NULL;
    }

    ak09916_t *dev = calloc(1, sizeof(ak09916_t));
    if (dev == NULL) {
        return NULL;
    }

    strncpy(dev->name, name, MAX_NAME_LEN - 1);
    dev->interface.i2c = i2c;
    dev->interface.isolated = isolated;
    dev->interface.rotation = rotation;
    dev->state = AK09916_RESET;

    dev->sync.measrdy_sem = xSemaphoreCreateBinary();
    if (dev->sync.measrdy_sem == NULL) {
        return NULL;
    }

    dev->sync.mutex = xSemaphoreCreateMutex();
    if (dev->sync.mutex == NULL) {
        return NULL;
    }

    if ((dev->service = service_start(dev->name, dev, ak09916_fsm, period,
                                    priority)) == NULL) {
        LOG_ERROR(dev->name, "Fatal error");
        return NULL;
    }

    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    return dev;
}

static void ak09916_fsm(void *area) {
    ak09916_t *dev = (ak09916_t *)area;
    switch (dev->state) {
        case AK09916_RESET:
            ak09916_write_reg(dev, CNTL3, SRST);
            vTaskDelay(100);
            dev->state = AK09916_RESET_WAIT;
            break;

        case AK09916_RESET_WAIT:
            if ((ak09916_read_reg(dev, WIA2) == DEVICE_ID) &&
                ((ak09916_read_reg(dev, CNTL3) & SRST) == 0)) {
                dev->state = AK09916_CONF;
                LOG_DEBUG(dev->name, "Device available");
            } else {
                dev->state = AK09916_RESET;
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = AK09916_FAIL;
                    LOG_ERROR(dev->name, "Wrong device ID or reset failed");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case AK09916_CONF:
            if (ak09916_configure(dev)) {
                dev->state = AK09916_MEAS;
                LOG_INFO(dev->name, "Device configured");
            } else {
                LOG_ERROR(dev->name, "Wrong configuration, reset");
                dev->state = AK09916_RESET;
            }
            break;

        case AK09916_MEAS:
            ak09916_write_reg(dev, CNTL2, MODE4);  // Set to 100Hz continuous mode
            dev->state = AK09916_READ;
            break;

        case AK09916_READ:
            ak09916_read_meas(dev);
            ak09916_process_meas(dev);
            xSemaphoreGive(dev->sync.measrdy_sem);
            return;

        case AK09916_FAIL:
            xSemaphoreGive(dev->sync.measrdy_sem);
            vTaskDelete(NULL);
            return;

        default:
            break;
    }
}

static uint8_t ak09916_read_reg(ak09916_t *dev, uint8_t reg) {
    uint8_t buf[2];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = reg;

    i2c_transmit(dev->interface.i2c, buf[0], &buf[1], 1);
    i2c_receive(dev->interface.i2c, buf[0], &buf[1], 1);
    return buf[1];
}

static void ak09916_write_reg(ak09916_t *dev, uint8_t reg, uint8_t value) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | WRITE;
    buf[1] = reg;
    buf[2] = value;

    i2c_transmit(dev->interface.i2c, buf[0], &buf[1], 2);
}

static void ak09916_read_meas(ak09916_t *dev) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = ST1;

    i2c_transmit(dev->interface.i2c, buf[0], &buf[1], 1);
    i2c_receive(dev->interface.i2c, buf[0], (uint8_t *)&dev->raw,
                sizeof(ak09916_raw_t));
}

static int ak09916_process_meas(ak09916_t *dev) {
    if (dev->raw.ST1 & DRDY) {
        // Check for magnetic sensor overflow
        if (dev->raw.ST2 & HOFL) {
            LOG_WARN(dev->name, "Magnetic sensor overflow");
            return -1;
        }

        // Convert raw values to μT
        int16_t mag_x = (int16_t)((dev->raw.HXH << 8) | dev->raw.HXL);
        int16_t mag_y = (int16_t)((dev->raw.HYH << 8) | dev->raw.HYL);
        int16_t mag_z = (int16_t)((dev->raw.HZH << 8) | dev->raw.HZL);

        // Apply sensitivity scaling
        dev->meas.mag_x = mag_x * MAG_SENS;
        dev->meas.mag_y = mag_y * MAG_SENS;
        dev->meas.mag_z = mag_z * MAG_SENS;

        // Process temperature
        dev->meas.temperature = (dev->raw.TMPS * TEMP_SCALE) + TEMP_OFFSET;

        // Apply rotation if needed based on mounting orientation
        if (dev->interface.rotation != 0) {
            // TODO: Implement rotation matrix transformation
        }

        return 0;
    }
    return -1;
}

static void ak09916_set_clear_reg(ak09916_t *dev, uint8_t reg, uint8_t setbits,
                                 uint8_t clearbits) {
    uint8_t orig_val = ak09916_read_reg(dev, reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        ak09916_write_reg(dev, reg, val);
    }
}

static int ak09916_configure(ak09916_t *dev) {
    uint8_t orig_val;
    int rv = 1;

    // Set configuration
    for (int i = 0; i < SIZE_REG_CFG; i++) {
        ak09916_set_clear_reg(dev, reg_cfg[i].reg, reg_cfg[i].setbits,
                             reg_cfg[i].clearbits);
    }

    // Verify configuration
    for (int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = ak09916_read_reg(dev, reg_cfg[i].reg);

        if ((orig_val & reg_cfg[i].setbits) != reg_cfg[i].setbits) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not set)",
                     reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if ((orig_val & reg_cfg[i].clearbits) != 0) {
            LOG_ERROR(dev->name, "0x%02x: 0x%02x (0x%02x not cleared)",
                     reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    return rv;
}

void ak09916_get_meas_non_block(ak09916_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    memcpy(ptr, (void *)&dev->meas, sizeof(ak09916_meas_t));
    xSemaphoreGive(dev->sync.mutex);
}

void ak09916_get_meas_block(ak09916_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    ak09916_get_meas_non_block(dev, ptr);
}

void ak09916_stat(ak09916_t *dev) {
    if (dev == NULL || dev->state != AK09916_READ) {
        return;
    }
    LOG_INFO(dev->name, "Statistics:");
    LOG_INFO(dev->name, "mag_x = %.3f [μT]", dev->meas.mag_x);
    LOG_INFO(dev->name, "mag_y = %.3f [μT]", dev->meas.mag_y);
    LOG_INFO(dev->name, "mag_z = %.3f [μT]", dev->meas.mag_z);
    LOG_INFO(dev->name, "temperature = %.1f [°C]", dev->meas.temperature);
} 