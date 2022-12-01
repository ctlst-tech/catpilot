#include "cubeio.h"

#include "cubeio_reg.h"

// Private functions
static void cubeio_fsm(void *area);
static int cubeio_write_regs(cubeio_t *dev, uint8_t page, uint8_t offset,
                             uint8_t count, uint16_t *ptr);

static int cubeio_read_regs(cubeio_t *dev, uint8_t page, uint8_t offset,
                            uint8_t count, uint16_t *ptr);

static int cubeio_write_reg(cubeio_t *dev, uint8_t page, uint8_t offset,
                            uint16_t data);

static int cubeio_set_clear_reg(cubeio_t *dev, uint8_t page, uint8_t offset,
                                uint16_t setbits, uint16_t clearbits);

static int cubeio_pop_event(cubeio_t *dev, enum cubeio_event_t event);

static void cubeio_push_event(cubeio_t *dev, enum cubeio_event_t event);

void cubeio_update_safety_options(cubeio_t *dev);
uint16_t cubeio_scale_output(double out, cubeio_range_cfg_t *cfg);
double cubeio_scale_input(uint16_t in, cubeio_range_cfg_t *cfg);

// Public functions
cubeio_t *cubeio_start(char *name, uint32_t period, uint32_t priority,
                       usart_t *usart) {
    if (usart == NULL || name == NULL || period <= 0 || priority <= 0) {
        return NULL;
    }

    cubeio_t *dev = calloc(1, sizeof(cubeio_t));

    if (dev == NULL) {
        return NULL;
    }

    strncpy(dev->name, name, MAX_NAME_LEN);

    dev->interface.usart = usart;

    dev->sync.iordy_semaphore = xSemaphoreCreateBinary();
    if (dev->sync.iordy_semaphore == NULL) {
        return NULL;
    }

    dev->state = CUBEIO_RESET;

    if ((dev->service = service_start(dev->name, dev, cubeio_fsm, period,
                                      priority)) == NULL) {
        LOG_ERROR(dev->name, "Fatal error");
        return NULL;
    }

    xSemaphoreGive(dev->sync.iordy_semaphore);

    return dev;
}

static void cubeio_fsm(void *area) {
    cubeio_t *dev = (cubeio_t *)area;
    switch (dev->state) {
        case CUBEIO_RESET:
            vTaskDelay(100);
            dev->state = CUBEIO_CONF;

            // Check protocol version
            cubeio_read_regs(dev, PAGE_CONFIG, 0, sizeof(dev->config) / 2,
                             (uint16_t *)&dev->config);

            if (dev->config.protocol_version != PROTOCOL_VERSION ||
                dev->config.protocol_version2 != PROTOCOL_VERSION2) {
                LOG_ERROR(dev->name, "Wrong protocols versions: %lu, %lu",
                          dev->config.protocol_version,
                          dev->config.protocol_version2);
                dev->attempt++;
                if (dev->attempt > 5) {
                    LOG_ERROR(dev->name, "Fatal error");
                    dev->state = CUBEIO_FAIL;
                }
            }
            dev->attempt = 0;

            // Set ARM
            if (cubeio_set_clear_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_ARMING,
                                     P_SETUP_ARMING_IO_ARM_OK |
                                         P_SETUP_ARMING_FMU_ARMED |
                                         P_SETUP_ARMING_RC_HANDLING_DISABLED,
                                     0)) {
                LOG_ERROR(dev->name, "Arming setup error");
                dev->state = CUBEIO_FAIL;
            }

            break;

        case CUBEIO_CONF:
            // TODO: There will be a default configuration
            LOG_INFO(dev->name, "Initialization successful");
            dev->state = CUBEIO_OPERATION;
            break;

        case CUBEIO_OPERATION:
            dev->sync.now = xTaskGetTickCount() * portTICK_PERIOD_MS;

            // Event handling
            if (cubeio_pop_event(dev, CUBEIO_SET_PWM)) {
                for (int i = 0; i < dev->pwm_out.num_channels; i++) {
                    dev->pwm_out.pwm[i] = cubeio_scale_output(
                        dev->pwm[i], &dev->pwm_range_cfg[i]);
                }
                cubeio_write_regs(dev, PAGE_DIRECT_PWM, 0,
                                  dev->pwm_out.num_channels, dev->pwm_out.pwm);
                // DEBUG
                // gpio_reset(&gpio_fmu_pwm[1]);
            }
            if (cubeio_pop_event(dev, CUBEIO_SET_FAILSAFE_PWM)) {
                for (int i = 0; i < CUBEIO_MAX_CHANNELS; i++) {
                    dev->pwm_out.failsafe_pwm[i] = cubeio_scale_output(
                        dev->failsafe_pwm, &dev->pwm_range_cfg[i]);
                }
                cubeio_write_regs(dev, PAGE_FAILSAFE_PWM, 0,
                                  CUBEIO_MAX_CHANNELS,
                                  dev->pwm_out.failsafe_pwm);
            }
            if (cubeio_pop_event(dev, CUBEIO_FORCE_SAFETY_OFF)) {
                cubeio_write_reg(dev, PAGE_SETUP,
                                 PAGE_REG_SETUP_FORCE_SAFETY_OFF,
                                 FORCE_SAFETY_MAGIC);
            }
            if (cubeio_pop_event(dev, CUBEIO_FORCE_SAFETY_ON)) {
                cubeio_write_reg(dev, PAGE_SETUP,
                                 PAGE_REG_SETUP_FORCE_SAFETY_ON,
                                 FORCE_SAFETY_MAGIC);
            }
            if (cubeio_pop_event(dev, CUBEIO_ENABLE_SBUS_OUT)) {
                cubeio_write_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_SBUS_RATE,
                                 dev->rate.sbus_rate_hz);
                cubeio_set_clear_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_FEATURES,
                                     P_SETUP_FEATURES_SBUS1_OUT, 0);
            }
            if (cubeio_pop_event(dev, CUBEIO_SET_RATES)) {
                cubeio_write_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_ALTRATE,
                                 dev->rate.freq);
                cubeio_write_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_PWM_RATE_MASK,
                                 dev->rate.chmask);
            }
            if (cubeio_pop_event(dev, CUBEIO_SET_IMU_HEATER_DUTY)) {
                cubeio_write_reg(dev, PAGE_SETUP,
                                 PAGE_REG_SETUP_HEATER_DUTY_CYCLE,
                                 dev->pwm_out.heater_duty);
            }
            if (cubeio_pop_event(dev, CUBEIO_SET_DEFAULT_RATE)) {
                cubeio_write_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_DEFAULTRATE,
                                 dev->rate.default_freq);
            }
            if (cubeio_pop_event(dev, CUBEIO_BRUSHED_ON)) {
                cubeio_set_clear_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_FEATURES,
                                     P_SETUP_FEATURES_BRUSHED, 0);
            }
            if (cubeio_pop_event(dev, CUBEIO_ONESHOT_ON)) {
                cubeio_set_clear_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_FEATURES,
                                     P_SETUP_FEATURES_ONESHOT, 0);
            }
            if (cubeio_pop_event(dev, CUBEIO_SET_SAFETY_MASK)) {
                cubeio_write_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_IGNORE_SAFETY,
                                 dev->pwm_out.safety_mask);
            }
            // TODO: Add MIXING event for FMU dying case

            // Routine
            if (dev->sync.now - dev->sync.last_rc_read_ms > 20) {
                cubeio_read_regs(dev, PAGE_RAW_RCIN, 0,
                                 sizeof(dev->page_rc_input) / 2,
                                 (uint16_t *)&dev->page_rc_input);
                for (int i = 0; i < CUBEIO_MAX_CHANNELS; i++) {
                    dev->rc[i] = cubeio_scale_input(
                        dev->page_rc_input.channel[i], &dev->rc_range_cfg[i]);
                }
                dev->sync.last_rc_read_ms =
                    xTaskGetTickCount() * portTICK_PERIOD_MS;
                // DEBUG
                // gpio_reset(&gpio_fmu_pwm[2]);
            }
            if (dev->sync.now - dev->sync.last_status_read_ms > 50) {
                cubeio_read_regs(dev, PAGE_STATUS, 0,
                                 sizeof(dev->page_reg_status) / 2,
                                 (uint16_t *)&dev->page_reg_status);
                dev->sync.last_status_read_ms =
                    xTaskGetTickCount() * portTICK_PERIOD_MS;
            }
            if (dev->sync.now - dev->sync.last_servo_read_ms > 50) {
                cubeio_read_regs(dev, PAGE_SERVOS, 0, dev->pwm_out.num_channels,
                                 (uint16_t *)&dev->pwm_in);
                dev->sync.last_servo_read_ms =
                    xTaskGetTickCount() * portTICK_PERIOD_MS;
            }
            if (dev->sync.now - dev->sync.last_safety_ms > 1000) {
                cubeio_update_safety_options(dev);
                dev->sync.last_safety_ms =
                    xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            uint32_t rc_protocol = 1;
            cubeio_write_regs(dev, PAGE_SETUP, PAGE_REG_SETUP_RC_PROTOCOLS, 2,
                              (uint16_t *)&rc_protocol);

            xSemaphoreGive(dev->sync.iordy_semaphore);
            break;

        case CUBEIO_FAIL:
            vTaskDelay(1000);
            break;
    }
}

void cubeio_set_range(cubeio_t *dev, int type, uint8_t channel,
                      uint16_t channel_type, uint16_t min, uint16_t max) {
    cubeio_range_cfg_t *range_cfg;

    if (type == CUBEIO_RC) {
        range_cfg = dev->rc_range_cfg;
    } else if (type == CUBEIO_PWM) {
        range_cfg = dev->pwm_range_cfg;
    } else {
        return;
    }

    if (channel > CUBEIO_MAX_CHANNELS) return;

    range_cfg[channel].type = channel_type;
    range_cfg[channel].min = min;
    range_cfg[channel].max = max;
}

void cubeio_set_pwm(cubeio_t *dev, uint8_t channels, double *pwm) {
    if (channels < 1) return;
    dev->pwm_out.num_channels = MIN(channels, CUBEIO_MAX_CHANNELS);
    memcpy(dev->pwm, pwm, sizeof(dev->pwm));
    cubeio_push_event(dev, CUBEIO_SET_PWM);
}

void cubeio_set_failsafe_pwm(cubeio_t *dev, double pwm) {
    dev->failsafe_pwm = pwm;
    cubeio_push_event(dev, CUBEIO_SET_FAILSAFE_PWM);
}

void cubeio_get_rc(cubeio_t *dev, double *ptr) {
    memcpy(ptr, dev->rc, sizeof(dev->rc));
}

uint16_t cubeio_get_pwm_channel(cubeio_t *dev, uint8_t channel) {
    if (channel >= CUBEIO_MAX_CHANNELS) return 0xFFFF;
    return dev->pwm_in.pwm[channel];
}

void cubeio_set_safety_mask(cubeio_t *dev, uint16_t safety_mask) {
    if (dev->pwm_out.safety_mask != safety_mask) {
        dev->pwm_out.safety_mask = safety_mask;
    }
    cubeio_push_event(dev, CUBEIO_SET_SAFETY_MASK);
}

void cubeio_set_freq(cubeio_t *dev, uint16_t chmask, uint16_t freq) {
    const uint8_t ch_masks[3] = {0x03, 0x0C, 0xF0};
    for (size_t i = 0; i < (sizeof(ch_masks) / sizeof(ch_masks[0])); i++) {
        if (chmask & ch_masks[i]) {
            chmask |= ch_masks[i];
        }
    }
    dev->rate.freq = freq;
    dev->rate.chmask = chmask;
    cubeio_push_event(dev, CUBEIO_SET_RATES);
}

uint16_t cubeio_get_freq(cubeio_t *dev, uint16_t channel) {
    if (1 << channel & dev->rate.chmask) {
        return dev->rate.freq;
    }
    return dev->rate.default_freq;
}

void cubeio_set_default_freq(cubeio_t *dev, uint16_t freq) {
    if (dev->rate.default_freq != freq) {
        dev->rate.default_freq = freq;
        cubeio_push_event(dev, CUBEIO_SET_DEFAULT_RATE);
    }
}

void cubeio_set_oneshot_mode(cubeio_t *dev) {
    dev->rate.oneshot_enabled = 1;
    cubeio_push_event(dev, CUBEIO_ONESHOT_ON);
}

void cubeio_set_brushed_mode(cubeio_t *dev) {
    dev->rate.brushed_enabled = 1;
    cubeio_push_event(dev, CUBEIO_BRUSHED_ON);
}

int cubeio_get_safety_switch_state(cubeio_t *dev) {
    return (dev->page_reg_status.flag_safety_off ? ARMED : DISARMED);
}

void cubeio_force_safety_on(cubeio_t *dev) {
    dev->page_reg_status.safety_forced_off = SAFETY_ON;
    cubeio_push_event(dev, CUBEIO_FORCE_SAFETY_ON);
}

void cubeio_force_safety_off(cubeio_t *dev) {
    dev->page_reg_status.safety_forced_off = SAFETY_OFF;
    cubeio_push_event(dev, CUBEIO_FORCE_SAFETY_OFF);
}

void cubeio_set_imu_heater_duty(cubeio_t *dev, uint8_t duty) {
    dev->pwm_out.heater_duty = duty;
    cubeio_push_event(dev, CUBEIO_SET_IMU_HEATER_DUTY);
}

int16_t cubeio_get_rssi(cubeio_t *dev) {
    return dev->page_rc_input.rssi;
}

void cubeio_enable_sbus_out(cubeio_t *dev, uint16_t freq) {
    dev->rate.sbus_rate_hz = freq;
    cubeio_push_event(dev, CUBEIO_ENABLE_SBUS_OUT);
}

// Private functions
static int cubeio_write_regs(cubeio_t *dev, uint8_t page, uint8_t offset,
                             uint8_t count, uint16_t *regs) {
    int rv;
    cubeio_packet_t tx_pkt = {};
    cubeio_packet_t rx_pkt = {};

    tx_pkt.count = count;
    tx_pkt.code = PKT_CODE_WRITE;
    tx_pkt.page = page;
    tx_pkt.offset = offset;
    tx_pkt.crc = 0;
    memcpy(tx_pkt.regs, regs, 2 * count);
    tx_pkt.crc = crc_packet(&tx_pkt);

    for (uint8_t att = 0; att < 3; att++) {
        rv = usart_transmit_receive(dev->interface.usart, (uint8_t *)&tx_pkt,
                                    (uint8_t *)&rx_pkt, sizeof(tx_pkt),
                                    sizeof(rx_pkt));

        if (rv != SUCCESS) {
            continue;
        }
        if (get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(dev->name, "Bad code, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EINVAL;
            continue;
        }
        uint8_t got_crc = rx_pkt.crc;
        rx_pkt.crc = 0;
        if (crc_packet(&rx_pkt) != got_crc) {
            LOG_ERROR(dev->name, "Bad crc, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EPROTO;
            continue;
        }
        break;
    }

    return rv;
}

static int cubeio_read_regs(cubeio_t *dev, uint8_t page, uint8_t offset,
                            uint8_t count, uint16_t *regs) {
    int rv;
    cubeio_packet_t tx_pkt = {};
    cubeio_packet_t rx_pkt = {};

    tx_pkt.count = count;
    tx_pkt.code = PKT_CODE_READ;
    tx_pkt.page = page;
    tx_pkt.offset = offset;
    tx_pkt.crc = 0;
    memcpy(tx_pkt.regs, regs, 2 * count);
    tx_pkt.crc = crc_packet(&tx_pkt);

    for (uint8_t att = 0; att < 3; att++) {
        rv = usart_transmit_receive(dev->interface.usart, (uint8_t *)&tx_pkt,
                                    (uint8_t *)&rx_pkt, sizeof(tx_pkt),
                                    sizeof(rx_pkt));

        if (rv != SUCCESS) {
            continue;
        }
        if (get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(dev->name, "Bad code, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EINVAL;
            continue;
        }
        if (get_pkt_count(&rx_pkt) != count) {
            LOG_ERROR(dev->name, "Bad count, %d, %d", count,
                      get_pkt_size(&rx_pkt));
            rv = EINVAL;
            continue;
        }
        uint8_t got_crc = rx_pkt.crc;
        rx_pkt.crc = 0;
        if (crc_packet(&rx_pkt) != got_crc) {
            LOG_ERROR(dev->name, "Bad crc, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EPROTO;
            continue;
        }
        memcpy(regs, rx_pkt.regs, 2 * count);
        break;
    }

    return rv;
}

void cubeio_stat(cubeio_t *dev) {
    if (dev->state != CUBEIO_OPERATION) {
        return;
    }
    printf("\n");
    LOG_DEBUG(dev->name, "Statistics:");
    LOG_DEBUG(dev->name, "rc_ch1  = %u", dev->page_rc_input.channel[0]);
    LOG_DEBUG(dev->name, "rc_ch2  = %u", dev->page_rc_input.channel[1]);
    LOG_DEBUG(dev->name, "rc_ch3  = %u", dev->page_rc_input.channel[2]);
    LOG_DEBUG(dev->name, "rc_ch4  = %u", dev->page_rc_input.channel[3]);
    LOG_DEBUG(dev->name, "rc_ch5  = %u", dev->page_rc_input.channel[4]);
    LOG_DEBUG(dev->name, "rc_ch6  = %u", dev->page_rc_input.channel[5]);
    LOG_DEBUG(dev->name, "rc_ch7  = %u", dev->page_rc_input.channel[6]);
    LOG_DEBUG(dev->name, "rc_ch8  = %u", dev->page_rc_input.channel[7]);
    LOG_DEBUG(dev->name, "rc_ch9  = %u", dev->page_rc_input.channel[8]);
    LOG_DEBUG(dev->name, "rc_ch10 = %u", dev->page_rc_input.channel[9]);
    LOG_DEBUG(dev->name, "rc_ch11 = %u", dev->page_rc_input.channel[10]);
    LOG_DEBUG(dev->name, "rc_ch12 = %u", dev->page_rc_input.channel[11]);
    LOG_DEBUG(dev->name, "rc_ch13 = %u", dev->page_rc_input.channel[12]);
    LOG_DEBUG(dev->name, "rc_ch14 = %u", dev->page_rc_input.channel[13]);
    LOG_DEBUG(dev->name, "rc_ch15 = %u", dev->page_rc_input.channel[14]);
    LOG_DEBUG(dev->name, "rc_ch16 = %u", dev->page_rc_input.channel[15]);
}

static int cubeio_write_reg(cubeio_t *dev, uint8_t page, uint8_t offset,
                            uint16_t reg) {
    return cubeio_write_regs(dev, page, offset, 1, &reg);
}

static int cubeio_set_clear_reg(cubeio_t *dev, uint8_t page, uint8_t offset,
                                uint16_t setbits, uint16_t clearbits) {
    uint16_t reg = 0;
    if (cubeio_read_regs(dev, page, offset, 1, &reg)) return -1;
    reg = (reg & ~clearbits) | setbits;
    return cubeio_write_reg(dev, page, offset, reg);
}

static int cubeio_pop_event(cubeio_t *dev, enum cubeio_event_t event) {
    if (dev->eventmask & (1 << event)) {
        dev->eventmask &= ~(1 << event);
        return 1;
    } else {
        return 0;
    }
}

static void cubeio_push_event(cubeio_t *dev, enum cubeio_event_t event) {
    dev->eventmask |= (1 << event);
}

void cubeio_update_safety_options(cubeio_t *dev) {
    uint16_t reg = 0;
    // TODO: there will check the safety board button
    reg = P_SETUP_ARMING_SAFETY_DISABLE_OFF;
    uint16_t mask =
        (P_SETUP_ARMING_SAFETY_DISABLE_OFF | P_SETUP_ARMING_SAFETY_DISABLE_ON);
    cubeio_set_clear_reg(dev, PAGE_SETUP, PAGE_REG_SETUP_ARMING, reg & mask,
                         (~reg) & mask);
}

uint16_t cubeio_scale_output(double out, cubeio_range_cfg_t *cfg) {
    if (cfg->type == CUBEIO_CHANNEL_UNIPOLAR) {
        return (uint16_t)(out * (cfg->max - cfg->min) + cfg->min);
    } else if (cfg->type == CUBEIO_CHANNEL_BIPOLAR) {
        return (uint16_t)((out + 1.0) * (cfg->max - cfg->min) / 2 + cfg->min);
    }
    return 0;
}

double cubeio_scale_input(uint16_t in, cubeio_range_cfg_t *cfg) {
    double rv;
    if (in < cfg->min || in > cfg->max) {
        rv = 0;
    } else if (cfg->type == CUBEIO_CHANNEL_UNIPOLAR) {
        rv = (in - cfg->min) / (double)(cfg->max - cfg->min);
        rv = SAT(rv, 1, 0);
    } else if (cfg->type == CUBEIO_CHANNEL_BIPOLAR) {
        rv = (2 * (in - cfg->min) / (double)(cfg->max - cfg->min) - 1.0);
        rv = SAT(rv, 1, -1);
    }
    return rv;
}
