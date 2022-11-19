#include "cubeio.h"

#include "cubeio_reg.h"

// Data structures
static cubeio_cfg_t cubeio_cfg;
static enum cubeio_state_t cubeio_state;
static cubeio_packet_t cubeio_tx_packet;
static cubeio_packet_t cubeio_rx_packet;
static cubeio_eventmask_t cubeio_eventmask;
static cubeio_page_config_t cubeio_config;
static cubeio_pwm_out_t cubeio_pwm_out;
static cubeio_pwm_in_t cubeio_pwm_in;
static cubeio_rate_t cubeio_rate;
static cubeio_page_reg_status_t cubeio_page_reg_status;
static cubeio_page_rc_input_t cubeio_page_rc_input;

static cubeio_range_cfg_t cubeio_rc_range_cfg[CUBEIO_MAX_CHANNELS];
static cubeio_range_cfg_t cubeio_pwm_range_cfg[CUBEIO_MAX_CHANNELS];

static double cubeio_pwm[CUBEIO_MAX_CHANNELS];
static double cubeio_failsafe_pwm;
static double cubeio_rc[CUBEIO_MAX_CHANNELS];

// Private functions
void cuibeio_thread(void *dev_ptr);
static void cubeio_fsm(void);
static int cubeio_write_regs(uint8_t page, uint8_t offset, uint8_t count,
                             uint16_t *ptr);

static int cubeio_read_regs(uint8_t page, uint8_t offset, uint8_t count,
                            uint16_t *ptr);

static int cubeio_write_reg(uint8_t page, uint8_t offset, uint16_t data);

static int cubeio_set_clear_reg(uint8_t page, uint8_t offset, uint16_t setbits,
                                uint16_t clearbits);

static int cubeio_pop_event(cubeio_eventmask_t *eventmask,
                            enum cubeio_event_t event);

static void cubeio_push_event(cubeio_eventmask_t *eventmask,
                              enum cubeio_event_t event);

void cubeio_update_safety_options(void);
uint16_t cubeio_scale_output(double out, cubeio_range_cfg_t *cfg);
double cubeio_scale_input(uint16_t in, cubeio_range_cfg_t *cfg);

// Sync
static SemaphoreHandle_t iordy_semaphore;
static uint32_t attempt;
static TickType_t now;
static TickType_t last_rc_read_ms;
static TickType_t last_status_read_ms;
static TickType_t last_servo_read_ms;
static TickType_t last_safety_ms;

// Public functions
int cubeio_start(usart_t *usart, uint32_t period, uint32_t thread_priority) {
    if (usart == NULL) {
        return -1;
    }

    strncpy(cubeio_cfg.name, "CUBEIO", 32);
    cubeio_cfg.usart = usart;

    if (period < 2) {
        LOG_ERROR(cubeio_cfg.name, "Too high frequency");
        return -1;
    }

    cubeio_cfg.os.period = period / portTICK_PERIOD_MS;
    cubeio_cfg.os.priority = thread_priority;

    iordy_semaphore = xSemaphoreCreateBinary();
    if (iordy_semaphore == NULL) {
        return -1;
    }

    if (xTaskCreate(cuibeio_thread, cubeio_cfg.name, 512, NULL,
                    cubeio_cfg.os.priority, NULL) != pdTRUE) {
        LOG_ERROR(cubeio_cfg.name, "Thread start error");
        return -1;
    }

    LOG_DEBUG(cubeio_cfg.name, "Start service, period = %u ms, priority = %u",
              cubeio_cfg.os.period, cubeio_cfg.os.priority);

    xSemaphoreGive(iordy_semaphore);

    return 0;
}

void cuibeio_thread(void *dev_ptr) {
    TickType_t last_wake_time;
    last_wake_time = xTaskGetTickCount();
    cubeio_state = CUBEIO_RESET;
    while (1) {
        cubeio_fsm();
        xTaskDelayUntil(&last_wake_time, cubeio_cfg.os.period);
    }
}

static void cubeio_fsm(void) {
    switch (cubeio_state) {
        case CUBEIO_RESET:
            vTaskDelay(100);
            cubeio_state = CUBEIO_CONF;

            // Check protocol version
            cubeio_read_regs(PAGE_CONFIG, 0, sizeof(cubeio_config) / 2,
                             (uint16_t *)&cubeio_config);

            if (cubeio_config.protocol_version != PROTOCOL_VERSION ||
                cubeio_config.protocol_version2 != PROTOCOL_VERSION2) {
                LOG_ERROR(cubeio_cfg.name, "Wrong protocols versions: %lu, %lu",
                          cubeio_config.protocol_version,
                          cubeio_config.protocol_version2);
                attempt++;
                if (attempt > 5) {
                    LOG_ERROR(cubeio_cfg.name, "Fatal error");
                    cubeio_state = CUBEIO_FAIL;
                }
            }
            attempt = 0;

            // Set ARM
            if (cubeio_set_clear_reg(PAGE_SETUP, PAGE_REG_SETUP_ARMING,
                                     P_SETUP_ARMING_IO_ARM_OK |
                                         P_SETUP_ARMING_FMU_ARMED |
                                         P_SETUP_ARMING_RC_HANDLING_DISABLED,
                                     0)) {
                LOG_ERROR(cubeio_cfg.name, "Arming setup error");
                cubeio_state = CUBEIO_FAIL;
            }

            break;

        case CUBEIO_CONF:
            // TODO: There will be a default configuration
            LOG_INFO(cubeio_cfg.name, "Initialization successful");
            cubeio_state = CUBEIO_OPERATION;
            break;

        case CUBEIO_OPERATION:
            now = xTaskGetTickCount() * portTICK_PERIOD_MS;

            // Event handling
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_SET_PWM)) {
                for (int i = 0; i < cubeio_pwm_out.num_channels; i++) {
                    cubeio_pwm_out.pwm[i] = cubeio_scale_output(
                        cubeio_pwm[i], &cubeio_pwm_range_cfg[i]);
                }
                cubeio_write_regs(PAGE_DIRECT_PWM, 0,
                                  cubeio_pwm_out.num_channels,
                                  cubeio_pwm_out.pwm);
                // DEBUG
                // gpio_reset(&gpio_fmu_pwm[1]);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_SET_FAILSAFE_PWM)) {
                for (int i = 0; i < CUBEIO_MAX_CHANNELS; i++) {
                    cubeio_pwm_out.failsafe_pwm[i] = cubeio_scale_output(
                        cubeio_failsafe_pwm, &cubeio_pwm_range_cfg[i]);
                }
                cubeio_write_regs(PAGE_FAILSAFE_PWM, 0, CUBEIO_MAX_CHANNELS,
                                  cubeio_pwm_out.failsafe_pwm);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_FORCE_SAFETY_OFF)) {
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_FORCE_SAFETY_OFF,
                                 FORCE_SAFETY_MAGIC);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_FORCE_SAFETY_ON)) {
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_FORCE_SAFETY_ON,
                                 FORCE_SAFETY_MAGIC);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_ENABLE_SBUS_OUT)) {
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_SBUS_RATE,
                                 cubeio_rate.sbus_rate_hz);
                cubeio_set_clear_reg(PAGE_SETUP, PAGE_REG_SETUP_FEATURES,
                                     P_SETUP_FEATURES_SBUS1_OUT, 0);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_SET_RATES)) {
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_ALTRATE,
                                 cubeio_rate.freq);
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_PWM_RATE_MASK,
                                 cubeio_rate.chmask);
            }
            if (cubeio_pop_event(&cubeio_eventmask,
                                 CUBEIO_SET_IMU_HEATER_DUTY)) {
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_HEATER_DUTY_CYCLE,
                                 cubeio_pwm_out.heater_duty);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_SET_DEFAULT_RATE)) {
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_DEFAULTRATE,
                                 cubeio_rate.default_freq);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_BRUSHED_ON)) {
                cubeio_set_clear_reg(PAGE_SETUP, PAGE_REG_SETUP_FEATURES,
                                     P_SETUP_FEATURES_BRUSHED, 0);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_ONESHOT_ON)) {
                cubeio_set_clear_reg(PAGE_SETUP, PAGE_REG_SETUP_FEATURES,
                                     P_SETUP_FEATURES_ONESHOT, 0);
            }
            if (cubeio_pop_event(&cubeio_eventmask, CUBEIO_SET_SAFETY_MASK)) {
                cubeio_write_reg(PAGE_SETUP, PAGE_REG_SETUP_IGNORE_SAFETY,
                                 cubeio_pwm_out.safety_mask);
            }
            // TODO: Add MIXING event for FMU dying case

            // Routine
            if (now - last_rc_read_ms > 20) {
                cubeio_read_regs(PAGE_RAW_RCIN, 0,
                                 sizeof(cubeio_page_rc_input) / 2,
                                 (uint16_t *)&cubeio_page_rc_input);
                for (int i = 0; i < CUBEIO_MAX_CHANNELS; i++) {
                    cubeio_rc[i] =
                        cubeio_scale_input(cubeio_page_rc_input.channel[i],
                                           &cubeio_rc_range_cfg[i]);
                }
                last_rc_read_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
                // DEBUG
                // GPIO_Reset(&gpio_fmu_pwm[2]);
            }
            if (now - last_status_read_ms > 50) {
                cubeio_read_regs(PAGE_STATUS, 0,
                                 sizeof(cubeio_page_reg_status) / 2,
                                 (uint16_t *)&cubeio_page_reg_status);
                last_status_read_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }
            if (now - last_servo_read_ms > 50) {
                cubeio_read_regs(PAGE_SERVOS, 0, cubeio_pwm_out.num_channels,
                                 (uint16_t *)&cubeio_pwm_in);
                last_servo_read_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }
            if (now - last_safety_ms > 1000) {
                cubeio_update_safety_options();
                last_safety_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            }

            uint32_t rc_protocol = 1;
            cubeio_write_regs(PAGE_SETUP, PAGE_REG_SETUP_RC_PROTOCOLS, 2,
                              (uint16_t *)&rc_protocol);

            xSemaphoreGive(iordy_semaphore);
            break;

        case CUBEIO_FAIL:
            vTaskDelay(1000);
            break;
    }
}

int CubeIO_Operation(void) {
    if (cubeio_state == CUBEIO_OPERATION) {
        return 0;
    } else {
        return -1;
    }
}

int CubeIO_Ready(void) {
    xSemaphoreTake(iordy_semaphore, portMAX_DELAY);
    return 1;
}

void cubeio_set_range(int type, uint8_t channel, uint16_t channel_type,
                      uint16_t min, uint16_t max) {
    cubeio_range_cfg_t *range_cfg;

    if (type == CUBEIO_RC) {
        range_cfg = cubeio_rc_range_cfg;
    } else if (type == CUBEIO_PWM) {
        range_cfg = cubeio_pwm_range_cfg;
    } else {
        return;
    }

    if (channel > CUBEIO_MAX_CHANNELS) return;

    range_cfg[channel].type = channel_type;
    range_cfg[channel].min = min;
    range_cfg[channel].max = max;
}

void cubeio_set_pwm(uint8_t channels, double *pwm) {
    if (channels < 1) return;
    cubeio_pwm_out.num_channels = MIN(channels, CUBEIO_MAX_CHANNELS);
    memcpy(cubeio_pwm, pwm, sizeof(cubeio_pwm));
    cubeio_push_event(&cubeio_eventmask, CUBEIO_SET_PWM);
}

void cubeio_set_failsafe_pwm(double pwm) {
    cubeio_failsafe_pwm = pwm;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_SET_FAILSAFE_PWM);
}

void cubeio_get_rc(double *ptr) {
    memcpy(ptr, cubeio_rc, sizeof(cubeio_rc));
}

uint16_t cubeio_get_pwm_channel(uint8_t channel) {
    if (channel >= CUBEIO_MAX_CHANNELS) return 0xFFFF;
    return cubeio_pwm_in.pwm[channel];
}

void cubeio_set_safety_mask(uint16_t safety_mask) {
    if (cubeio_pwm_out.safety_mask != safety_mask) {
        cubeio_pwm_out.safety_mask = safety_mask;
    }
    cubeio_push_event(&cubeio_eventmask, CUBEIO_SET_SAFETY_MASK);
}

void cubeio_set_freq(uint16_t chmask, uint16_t freq) {
    const uint8_t ch_masks[3] = {0x03, 0x0C, 0xF0};
    for (size_t i = 0; i < (sizeof(ch_masks) / sizeof(ch_masks[0])); i++) {
        if (chmask & ch_masks[i]) {
            chmask |= ch_masks[i];
        }
    }
    cubeio_rate.freq = freq;
    cubeio_rate.chmask = chmask;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_SET_RATES);
}

uint16_t cubeio_get_freq(uint16_t channel) {
    if (1 << channel & cubeio_rate.chmask) {
        return cubeio_rate.freq;
    }
    return cubeio_rate.default_freq;
}

void cubeio_set_default_freq(uint16_t freq) {
    if (cubeio_rate.default_freq != freq) {
        cubeio_rate.default_freq = freq;
        cubeio_push_event(&cubeio_eventmask, CUBEIO_SET_DEFAULT_RATE);
    }
}

void cubeio_set_oneshot_mode(void) {
    cubeio_rate.oneshot_enabled = 1;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_ONESHOT_ON);
}

void cubeio_set_brushed_mode(void) {
    cubeio_rate.brushed_enabled = 1;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_BRUSHED_ON);
}

int cubeio_get_safety_switch_state(void) {
    return (cubeio_page_reg_status.flag_safety_off ? ARMED : DISARMED);
}

void cubeio_force_safety_on(void) {
    cubeio_page_reg_status.safety_forced_off = SAFETY_ON;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_FORCE_SAFETY_ON);
}

void cubeio_force_safety_off(void) {
    cubeio_page_reg_status.safety_forced_off = SAFETY_OFF;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_FORCE_SAFETY_OFF);
}

void cubeio_set_imu_heater_duty(uint8_t duty) {
    cubeio_pwm_out.heater_duty = duty;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_SET_IMU_HEATER_DUTY);
}

int16_t cubeio_get_rssi(void) {
    return cubeio_page_rc_input.rssi;
}

void cubeio_enable_sbus_out(uint16_t freq) {
    cubeio_rate.sbus_rate_hz = freq;
    cubeio_push_event(&cubeio_eventmask, CUBEIO_ENABLE_SBUS_OUT);
}

// Private functions
static int cubeio_write_regs(uint8_t page, uint8_t offset, uint8_t count,
                             uint16_t *regs) {
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
        rv = usart_transmit_receive(cubeio_cfg.usart, (uint8_t *)&tx_pkt,
                                    (uint8_t *)&rx_pkt, sizeof(tx_pkt),
                                    sizeof(rx_pkt));

        if (rv != SUCCESS) {
            continue;
        }
        if (get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(cubeio_cfg.name, "Bad code, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EINVAL;
            continue;
        }
        uint8_t got_crc = rx_pkt.crc;
        rx_pkt.crc = 0;
        if (crc_packet(&rx_pkt) != got_crc) {
            LOG_ERROR(cubeio_cfg.name, "Bad crc, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EPROTO;
            continue;
        }
        break;
    }

    return rv;
}

static int cubeio_read_regs(uint8_t page, uint8_t offset, uint8_t count,
                            uint16_t *regs) {
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
        rv = usart_transmit_receive(cubeio_cfg.usart, (uint8_t *)&tx_pkt,
                                    (uint8_t *)&rx_pkt, sizeof(tx_pkt),
                                    sizeof(rx_pkt));

        if (rv != SUCCESS) {
            continue;
        }
        if (get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(cubeio_cfg.name, "Bad code, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EINVAL;
            continue;
        }
        if (get_pkt_count(&rx_pkt) != count) {
            LOG_ERROR(cubeio_cfg.name, "Bad count, %d, %d", count,
                      get_pkt_size(&rx_pkt));
            rv = EINVAL;
            continue;
        }
        uint8_t got_crc = rx_pkt.crc;
        rx_pkt.crc = 0;
        if (crc_packet(&rx_pkt) != got_crc) {
            LOG_ERROR(cubeio_cfg.name, "Bad crc, 0x%X, 0x%X, %d", page, offset,
                      count);
            rv = EPROTO;
            continue;
        }
        memcpy(regs, rx_pkt.regs, 2 * count);
        break;
    }

    return rv;
}

static int cubeio_write_reg(uint8_t page, uint8_t offset, uint16_t reg) {
    return cubeio_write_regs(page, offset, 1, &reg);
}

static int cubeio_set_clear_reg(uint8_t page, uint8_t offset, uint16_t setbits,
                                uint16_t clearbits) {
    uint16_t reg = 0;
    if (cubeio_read_regs(page, offset, 1, &reg)) return -1;
    reg = (reg & ~clearbits) | setbits;
    return cubeio_write_reg(page, offset, reg);
}

static int cubeio_pop_event(cubeio_eventmask_t *eventmask,
                            enum cubeio_event_t event) {
    if (*(eventmask) & (1 << event)) {
        *(eventmask) &= ~(1 << event);
        return 1;
    } else {
        return 0;
    }
}

static void cubeio_push_event(cubeio_eventmask_t *eventmask,
                              enum cubeio_event_t event) {
    *(eventmask) |= (1 << event);
}

void cubeio_update_safety_options(void) {
    uint16_t reg = 0;
    // TODO: there will check the safety board button
    reg = P_SETUP_ARMING_SAFETY_DISABLE_OFF;
    uint16_t mask =
        (P_SETUP_ARMING_SAFETY_DISABLE_OFF | P_SETUP_ARMING_SAFETY_DISABLE_ON);
    cubeio_set_clear_reg(PAGE_SETUP, PAGE_REG_SETUP_ARMING, reg & mask,
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
    if ((cfg->max - cfg->min) == 0) return 0;
    if (cfg->type == CUBEIO_CHANNEL_UNIPOLAR) {
        return ((in - cfg->min) / (double)(cfg->max - cfg->min));
    } else if (cfg->type == CUBEIO_CHANNEL_BIPOLAR) {
        return (2 * (in - cfg->min) / (double)(cfg->max - cfg->min) - 1.0);
    }
    return 0;
}
