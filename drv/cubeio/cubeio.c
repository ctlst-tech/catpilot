#include "cubeio.h"
#include "cubeio_reg.h"
#include "init.h"
#include "cfg.h"
#include "func.h"
#include <string.h>
#include "timer.h"

static char *device = "CubeIO";

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

static cubeio_range_cfg_t cubeio_rc_range_cfg[MAX_CHANNELS];
static cubeio_range_cfg_t cubeio_pwm_range_cfg[MAX_CHANNELS];

static double cubeio_pwm[MAX_CHANNELS];
static double cubeio_failsafe_pwm;
static double cubeio_rc[MAX_CHANNELS];

// Private functions
static int CubeIO_WriteRegs(uint8_t page, 
                            uint8_t offset, 
                            uint8_t count, 
                            uint16_t *ptr);

static int CubeIO_ReadRegs(uint8_t page, 
                           uint8_t offset, 
                           uint8_t count, 
                           uint16_t *ptr);

static int CubeIO_WriteReg(uint8_t page, 
                           uint8_t offset, 
                           uint16_t data);

static int CubeIO_SetClearReg(uint8_t page, 
                              uint8_t offset, 
                              uint16_t setbits, 
                              uint16_t clearbits);

static int CubeIO_PopEvent(cubeio_eventmask_t *eventmask, 
                           enum cubeio_event_t event);

static void CubeIO_PushEvent(cubeio_eventmask_t *eventmask, 
                             enum cubeio_event_t event);

void CubeIO_UpdateSafetyOptions(void);
uint16_t CubeIO_ScaleOutput(double out, cubeio_range_cfg_t *cfg);
double CubeIO_ScaleInput(uint16_t in, cubeio_range_cfg_t *cfg);

// Sync
static SemaphoreHandle_t iordy_semaphore;
static uint32_t attempt;
static TickType_t now;
static TickType_t last_rc_read_ms;
static TickType_t last_status_read_ms;
static TickType_t last_servo_read_ms;
static TickType_t last_safety_ms;

// Public functions
int CubeIO_Init(usart_cfg_t *usart) {
    if(usart == NULL) return -1;

    cubeio_cfg.usart = usart;

    if(iordy_semaphore == NULL) iordy_semaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(iordy_semaphore);
    cubeio_state = CubeIO_RESET;

    return 0;
}

void CubeIO_Run(void) {
    switch(cubeio_state) {

    case CubeIO_RESET:
        vTaskDelay(100);
        cubeio_state = CubeIO_CONF;

        // Check protocol version
        CubeIO_ReadRegs(PAGE_CONFIG, 
                        0, 
                        sizeof(cubeio_config) / 2, 
                        (uint16_t *)&cubeio_config);

        if(cubeio_config.protocol_version != PROTOCOL_VERSION ||
           cubeio_config.protocol_version2 != PROTOCOL_VERSION2) {
            LOG_ERROR(device, "Wrong protocols versions: %lu, %lu", 
                              cubeio_config.protocol_version, 
                              cubeio_config.protocol_version2);
            attempt++;
            if(attempt > 5) {
                LOG_ERROR(device, "Fatal error");
                cubeio_state = CubeIO_FAIL;
            }
        }
        attempt = 0;

        // Set ARM
        if(CubeIO_SetClearReg(PAGE_SETUP, PAGE_REG_SETUP_ARMING, 
                              P_SETUP_ARMING_IO_ARM_OK |
                              P_SETUP_ARMING_FMU_ARMED |
                              P_SETUP_ARMING_RC_HANDLING_DISABLED, 0)) {
            LOG_ERROR(device, "Arming setup error");
            cubeio_state = CubeIO_FAIL;
        }

        break;

    case CubeIO_CONF:
        // TODO: There will be a default configuration
        cubeio_state = CubeIO_OPERATION;
        break;

    case CubeIO_OPERATION:
        now = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // Event handling
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_SET_PWM)) {
            for(int i = 0; i < cubeio_pwm_out.num_channels; i++) {
                cubeio_pwm_out.pwm[i] = 
                    CubeIO_ScaleOutput(cubeio_pwm[i], 
                                        &cubeio_pwm_range_cfg[i]);
            }
            CubeIO_WriteRegs(PAGE_DIRECT_PWM, 0, cubeio_pwm_out.num_channels, 
                             cubeio_pwm_out.pwm);
            // DEBUG
            GPIO_Reset(&gpio_fmu_pwm_2);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_SET_FAILSAFE_PWM)) {
            for(int i = 0; i < MAX_CHANNELS; i++) {
                cubeio_pwm_out.failsafe_pwm[i] = 
                    CubeIO_ScaleOutput(cubeio_failsafe_pwm, 
                                        &cubeio_pwm_range_cfg[i]);
            }
            CubeIO_WriteRegs(PAGE_FAILSAFE_PWM, 0, MAX_CHANNELS, 
                             cubeio_pwm_out.failsafe_pwm);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_FORCE_SAFETY_OFF)) {
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_FORCE_SAFETY_OFF, 
                            FORCE_SAFETY_MAGIC);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_FORCE_SAFETY_ON)) {
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_FORCE_SAFETY_ON, 
                FORCE_SAFETY_MAGIC);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_ENABLE_SBUS_OUT)) {
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_SBUS_RATE, 
                            cubeio_rate.sbus_rate_hz);
            CubeIO_SetClearReg(PAGE_SETUP, PAGE_REG_SETUP_FEATURES, 
                               P_SETUP_FEATURES_SBUS1_OUT, 0);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_SET_RATES)) {
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_ALTRATE, 
                            cubeio_rate.freq);
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_PWM_RATE_MASK, 
                            cubeio_rate.chmask);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_SET_IMU_HEATER_DUTY)) {
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_HEATER_DUTY_CYCLE, 
                            cubeio_pwm_out.heater_duty);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_SET_DEFAULT_RATE)) {
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_DEFAULTRATE, 
                            cubeio_rate.default_freq);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_BRUSHED_ON)) {
            CubeIO_SetClearReg(PAGE_SETUP, PAGE_REG_SETUP_FEATURES, 
                               P_SETUP_FEATURES_BRUSHED, 0);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_ONESHOT_ON)) {
            CubeIO_SetClearReg(PAGE_SETUP, PAGE_REG_SETUP_FEATURES, 
                               P_SETUP_FEATURES_ONESHOT, 0);
        }
        if(CubeIO_PopEvent(&cubeio_eventmask, CubeIO_SET_SAFETY_MASK)) {
            CubeIO_WriteReg(PAGE_SETUP, PAGE_REG_SETUP_IGNORE_SAFETY, 
                            cubeio_pwm_out.safety_mask);
        }
        // TODO: Add MIXING event for FMU dying case

        // Routine
        if(now - last_rc_read_ms > 20) {
            CubeIO_ReadRegs(PAGE_RAW_RCIN, 0, sizeof(cubeio_page_rc_input) / 2,
                            (uint16_t *)&cubeio_page_rc_input);
            for(int i = 0; i < MAX_CHANNELS; i++) {
                cubeio_rc[i] = 
                    CubeIO_ScaleInput(cubeio_page_rc_input.channel[i], 
                                        &cubeio_rc_range_cfg[i]);
            }
            last_rc_read_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            // DEBUG
            GPIO_Reset(&gpio_fmu_pwm_3);
        }
        if(now - last_status_read_ms > 50) {
            CubeIO_ReadRegs(PAGE_STATUS, 0, sizeof(cubeio_page_reg_status) / 2, 
                                (uint16_t *)&cubeio_page_reg_status);
            last_status_read_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        }
        if(now - last_servo_read_ms > 50) {
            CubeIO_ReadRegs(PAGE_SERVOS, 0, cubeio_pwm_out.num_channels, 
                            (uint16_t *)&cubeio_pwm_in);
            last_servo_read_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        }
        if(now - last_safety_ms > 1000) {
            CubeIO_UpdateSafetyOptions();
            last_safety_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        }

        uint32_t rc_protocol = 1;
        CubeIO_WriteRegs(PAGE_SETUP, PAGE_REG_SETUP_RC_PROTOCOLS, 2, (uint16_t *)&rc_protocol);

        xSemaphoreGive(iordy_semaphore);
        break;

    case CubeIO_FAIL:
        vTaskDelay(1000);
        break;
    }
}

int CubeIO_Operation(void) {
    if(cubeio_state == CubeIO_OPERATION) {
        return 0;
    } else {
        return -1;
    }
}

int CubeIO_Ready(void) {
    xSemaphoreTake(iordy_semaphore, portMAX_DELAY);
    return 1;
}

void CubeIO_SetRange(int type, uint8_t channel,
                     uint16_t channel_type, uint16_t min, uint16_t max) {
    cubeio_range_cfg_t *range_cfg;

    if(type == CubeIO_RC) {
        range_cfg = cubeio_rc_range_cfg;
    } else if(type == CubeIO_PWM) {
        range_cfg = cubeio_pwm_range_cfg;
    } else {
        return;
    }

    if(channel > MAX_CHANNELS) return;

    range_cfg[channel].type = channel_type;
    range_cfg[channel].min = min;
    range_cfg[channel].max = max;
}

void CubeIO_SetPWM(uint8_t channels, double *pwm) {
    if(channels < 1) return;
    cubeio_pwm_out.num_channels = MIN(channels, MAX_CHANNELS);
    memcpy(cubeio_pwm, pwm,  sizeof(cubeio_pwm));
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_SET_PWM);
}

void CubeIO_SetFailsafePWM(double pwm) {
    cubeio_failsafe_pwm = pwm;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_SET_FAILSAFE_PWM);    
}

void CubeIO_GetRC(double *ptr) {
    memcpy(ptr, cubeio_rc, sizeof(cubeio_rc));
}

uint16_t CubeIO_GetPWMChannel(uint8_t channel) {
    if(channel >= MAX_CHANNELS) return 0xFFFF;
    return cubeio_pwm_in.pwm[channel];
}

void CubeIO_SetSafetyMask(uint16_t safety_mask) {
    if(cubeio_pwm_out.safety_mask != safety_mask) {
        cubeio_pwm_out.safety_mask = safety_mask;
    }
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_SET_SAFETY_MASK);
}

void CubeIO_SetFreq(uint16_t chmask, uint16_t freq) {
    const uint8_t ch_masks[3] = {0x03, 0x0C, 0xF0};
    for(size_t i = 0; i < (sizeof(ch_masks) / sizeof(ch_masks[0])); i++) {
        if(chmask & ch_masks[i]) {
            chmask |= ch_masks[i];
        }
    }
    cubeio_rate.freq = freq;
    cubeio_rate.chmask = chmask;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_SET_RATES);
}

uint16_t CubeIO_GetFreq(uint16_t channel) {
    if(1 << channel & cubeio_rate.chmask) {
        return cubeio_rate.freq;
    }
    return cubeio_rate.default_freq;
}

void CubeIO_SetDefaultFreq(uint16_t freq) {
    if(cubeio_rate.default_freq != freq) {
        cubeio_rate.default_freq = freq;
        CubeIO_PushEvent(&cubeio_eventmask, CubeIO_SET_DEFAULT_RATE);
    }
}

void CubeIO_SetOneshotMode(void) {
    cubeio_rate.oneshot_enabled = 1;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_ONESHOT_ON);
}

void CubeIO_SetBrushedMode(void) {
    cubeio_rate.brushed_enabled = 1;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_BRUSHED_ON);
}

int CubeIO_GetSafetySwitchState(void) {
    return (cubeio_page_reg_status.flag_safety_off ? 
            ARMED : 
            DISARMED);
}

void CubeIO_ForceSafetyOn(void) {
    cubeio_page_reg_status.safety_forced_off = SAFETY_ON;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_FORCE_SAFETY_ON);
}

void CubeIO_ForceSafetyOff(void) {
    cubeio_page_reg_status.safety_forced_off = SAFETY_OFF;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_FORCE_SAFETY_OFF);
}

void CubeIO_SetIMUHeaterDuty(uint8_t duty) {
    cubeio_pwm_out.heater_duty = duty;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_SET_IMU_HEATER_DUTY);
}

int16_t CubeIO_GetRSSI(void) {
    return cubeio_page_rc_input.rssi;
}

void CubeIO_EnableSBUSOut(uint16_t freq) {
    cubeio_rate.sbus_rate_hz = freq;
    CubeIO_PushEvent(&cubeio_eventmask, CubeIO_ENABLE_SBUS_OUT);
}

// Private functions
static int CubeIO_WriteRegs(uint8_t page, 
                            uint8_t offset, 
                            uint8_t count, 
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
        rv = USART_TransmitReceive(cubeio_cfg.usart,
                                    (uint8_t *)&tx_pkt, (uint8_t *)&rx_pkt,
                                    sizeof(tx_pkt), sizeof(rx_pkt));

        if(rv != SUCCESS) {
            continue;
        }
        if(get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(device, "Bad code, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EINVAL;
            continue;
        }
        uint8_t got_crc = rx_pkt.crc;
        rx_pkt.crc = 0;
        if(crc_packet(&rx_pkt) != got_crc) {
            LOG_ERROR(device, "Bad crc, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EPROTO;
            continue;
        }
        break;
    }

    return rv;
}

static int CubeIO_ReadRegs(uint8_t page, 
                           uint8_t offset, 
                           uint8_t count, 
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
        rv = USART_TransmitReceive(cubeio_cfg.usart,
                                    (uint8_t *)&tx_pkt, (uint8_t *)&rx_pkt,
                                    sizeof(tx_pkt), sizeof(rx_pkt));

        if(rv != SUCCESS) {
            continue;
        }
        if(get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(device, "Bad code, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EINVAL;
            continue;
        }
        if(get_pkt_count(&rx_pkt) != count) {
            LOG_ERROR(device, "Bad count, %d, %d",
                      count, get_pkt_size(&rx_pkt));
            rv = EINVAL;
            continue;
        }
        uint8_t got_crc = rx_pkt.crc;
        rx_pkt.crc = 0;
        if(crc_packet(&rx_pkt) != got_crc) {
            LOG_ERROR(device, "Bad crc, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EPROTO;
            continue;
        }
        memcpy(regs, rx_pkt.regs, 2 * count);
        break;
    }

    return rv;
}

static int CubeIO_WriteReg(uint8_t page, 
                           uint8_t offset, 
                           uint16_t reg) {
    return CubeIO_WriteRegs(page, offset, 1, &reg);
}

static int CubeIO_SetClearReg(uint8_t page, 
                              uint8_t offset, 
                              uint16_t setbits, 
                              uint16_t clearbits) {
    uint16_t reg = 0;
    if(CubeIO_ReadRegs(page, offset, 1, &reg)) return -1;
    reg = (reg & ~clearbits) | setbits;
    return CubeIO_WriteReg(page, offset, reg);
}

static int CubeIO_PopEvent(cubeio_eventmask_t *eventmask, 
                           enum cubeio_event_t event) {
    if(*(eventmask) & (1 << event)) {
        *(eventmask) &= ~(1 << event);
        return 1;
    } else {
        return 0;
    }
}

static void CubeIO_PushEvent(cubeio_eventmask_t *eventmask, 
                            enum cubeio_event_t event) { 
    *(eventmask) |= (1 << event);
}

void CubeIO_UpdateSafetyOptions(void) {
    uint16_t reg = 0;
    // TODO: there will check the safety board button
    reg = P_SETUP_ARMING_SAFETY_DISABLE_OFF;
    uint16_t mask = (P_SETUP_ARMING_SAFETY_DISABLE_OFF | 
                     P_SETUP_ARMING_SAFETY_DISABLE_ON);
    CubeIO_SetClearReg(PAGE_SETUP, PAGE_REG_SETUP_ARMING,
                       reg & mask, (~reg) & mask);
}

uint16_t CubeIO_ScaleOutput(double out, cubeio_range_cfg_t *cfg) {
    if(cfg->type == CubeIO_ChannelUnipolar) {
        return (uint16_t)(out * (cfg->max - cfg->min) + cfg->min);
    } else if(cfg->type == CubeIO_ChannelBipolar) {
        return (uint16_t)((out + 1.0) * (cfg->max - cfg->min) / 2 + cfg->min);
    }
    return 0;
}

double CubeIO_ScaleInput(uint16_t in, cubeio_range_cfg_t *cfg) {
    if((cfg->max - cfg->min) == 0) return 0;
    if(cfg->type == CubeIO_ChannelUnipolar) {
        return ((in - cfg->min) / (double)(cfg->max - cfg->min));
    } else if(cfg->type == CubeIO_ChannelBipolar) {
        return (2 * (in - cfg->min) / (double)(cfg->max - cfg->min) - 1.0);
    }
    return 0;
}
