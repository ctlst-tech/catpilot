#include "ist8310.h"

#include "ist8310_reg.h"

// Private functions
static void ist8310_fsm(void *area);
static int ist8310_configure(ist8310_t *dev);
static uint8_t ist8310_read_reg(ist8310_t *dev, uint8_t reg);
static void ist8310_write_reg(ist8310_t *dev, uint8_t reg, uint8_t value);
static void ist8310_set_clear_reg(ist8310_t *dev, uint8_t reg, uint8_t setbits,
                                  uint8_t clearbits);
static void ist8310_read_meas(ist8310_t *dev);
static int ist8310_process_meas(ist8310_t *dev);
static void ist8310_update_meas(ist8310_t *dev);

// Public functions
ist8310_t *ist8310_start(char *name, uint32_t period, uint32_t priority,
                         i2c_t *i2c) {
    if (i2c == NULL || name == NULL || period <= 0 || priority <= 0) {
        return NULL;
    }

    ist8310_t *dev = calloc(1, sizeof(ist8310_t));

    if (dev == NULL) {
        return NULL;
    }

    strncpy(dev->name, name, MAX_NAME_LEN - 1);

    dev->interface.i2c = i2c;

    dev->state = IST8310_RESET;

    dev->sync.measrdy_sem = xSemaphoreCreateBinary();
    if (dev->sync.measrdy_sem == NULL) {
        return NULL;
    }

    dev->sync.mutex = xSemaphoreCreateMutex();
    if (dev->sync.mutex == NULL) {
        return NULL;
    }

    if ((dev->service = service_start(dev->name, dev, ist8310_fsm, period,
                                      priority)) == NULL) {
        LOG_ERROR(dev->name, "Fatal error");
        return NULL;
    }

    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);

    return dev;
}

static void ist8310_fsm(void *area) {
    ist8310_t *dev = (ist8310_t *)area;
    switch (dev->state) {
        case IST8310_RESET:
            ist8310_write_reg(dev, CNTL2, SRST);
            vTaskDelay(50);
            dev->state = IST8310_RESET_WAIT;
            break;

        case IST8310_RESET_WAIT:
            ist8310_read_reg(dev, CNTL2);
            if ((ist8310_read_reg(dev, WHO_AM_I) == DEVICE_ID) &&
                ((ist8310_read_reg(dev, CNTL2) & SRST) == 0)) {
                dev->state = IST8310_CONF;
                LOG_DEBUG(dev->name, "Device available");
            } else {
                dev->state = IST8310_RESET;
                dev->attempt++;
                if (dev->attempt > 5) {
                    dev->state = IST8310_FAIL;
                    LOG_ERROR(dev->name,
                              "Wrong default registers values after reset");
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->attempt = 0;
                }
            }
            break;

        case IST8310_CONF:
            if (ist8310_configure(dev)) {
                dev->state = IST8310_MEAS;
                LOG_INFO(dev->name, "Device configured");
            } else {
                LOG_ERROR(dev->name, "Wrong configuration, reset");
                dev->state = IST8310_RESET;
            }
            break;

        case IST8310_MEAS:
            ist8310_write_reg(dev, CNTL1, SINGLE_MEAS);
            dev->state = IST8310_READ;
            break;

        case IST8310_READ:
            ist8310_read_meas(dev);
            ist8310_process_meas(dev);
            ist8310_write_reg(dev, CNTL1, SINGLE_MEAS);
            xSemaphoreGive(dev->sync.measrdy_sem);
            return;

        case IST8310_FAIL:
            xSemaphoreGive(dev->sync.measrdy_sem);
            vTaskDelete(NULL);
            return;

        default:
            break;
    }
}

void ist8310_get_meas_non_block(ist8310_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.mutex, portMAX_DELAY);
    memcpy(ptr, (void *)&dev->meas, sizeof(ist8310_meas_t));
    xSemaphoreGive(dev->sync.mutex);
}

void ist8310_get_meas_block(ist8310_t *dev, void *ptr) {
    xSemaphoreTake(dev->sync.measrdy_sem, portMAX_DELAY);
    ist8310_get_meas_non_block(dev, ptr);
}

void ist8310_stat(ist8310_t *dev) {
    if (dev == NULL || dev->state != IST8310_READ) {
        return;
    }
    printf("\n");
    printf("Statistics:\n");
    printf("mag_x = %.3f [G]\n", dev->meas.mag_x);
    printf("mag_y = %.3f [G]\n", dev->meas.mag_y);
    printf("mag_z = %.3f [G]\n", dev->meas.mag_z);
}

// Private functions
static uint8_t ist8310_read_reg(ist8310_t *dev, uint8_t reg) {
    uint8_t buf[2];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = reg;

    i2c_transmit(dev->interface.i2c, buf[0], &buf[1], 1);
    i2c_receive(dev->interface.i2c, buf[0], &buf[1], 1);
    return buf[1];
}

static void ist8310_write_reg(ist8310_t *dev, uint8_t reg, uint8_t value) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | WRITE;
    buf[1] = reg;
    buf[2] = value;

    i2c_transmit(dev->interface.i2c, buf[0], &buf[1], 2);
}

static void ist8310_read_meas(ist8310_t *dev) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = STAT1;

    i2c_transmit(dev->interface.i2c, buf[0], &buf[1], 1);
    i2c_receive(dev->interface.i2c, buf[0], (uint8_t *)&dev->raw,
                sizeof(ist8310_raw_t));
}

static int ist8310_process_meas(ist8310_t *dev) {
    if (dev->raw.STAT1 & DRDY) {
        dev->meas.mag_x = s_msb_lsb_16(dev->raw.DATAXH, dev->raw.DATAXL) *
                          dev->param.mag_scale;
        dev->meas.mag_y = s_msb_lsb_16(dev->raw.DATAYH, dev->raw.DATAYL) *
                          dev->param.mag_scale;
        dev->meas.mag_z = s_msb_lsb_16(dev->raw.DATAZH, dev->raw.DATAZL) *
                          dev->param.mag_scale;
    } else {
        return -1;
    }
    return 0;
}

static void ist8310_set_clear_reg(ist8310_t *dev, uint8_t reg, uint8_t setbits,
                                  uint8_t clearbits) {
    uint8_t orig_val = ist8310_read_reg(dev, reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        ist8310_write_reg(dev, reg, val);
    }
}

static int ist8310_configure(ist8310_t *dev) {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for (int i = 0; i < SIZE_REG_CFG; i++) {
        ist8310_set_clear_reg(dev, reg_cfg[i].reg, reg_cfg[i].setbits,
                              reg_cfg[i].clearbits);
    }

    // Check
    for (int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = ist8310_read_reg(dev, reg_cfg[i].reg);

        if ((orig_val & reg_cfg[i].setbits) != reg_cfg[i].setbits) {
            printf("%s: 0x%02x: 0x%02x (0x%02x not set)\n", dev->name,
                   (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if ((orig_val & reg_cfg[i].clearbits) != 0) {
            printf("%s: 0x%02x: 0x%02x (0x%02x not cleared)\n", dev->name,
                   (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    dev->param.mag_scale = (1.f / 1320.f);  // 1320 LSB/Gauss

    return rv;
}
