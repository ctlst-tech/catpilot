#include "px4io.h"
#include "px4io_reg.h"
#include "init.h"
#include "cfg.h"
#include "func.h"
#include <string.h>
#include <math.h>

static char *device = "PX4IO";

// Data structures
static px4io_cfg_t px4io_cfg;
static px4io_reg_t px4io_reg;
static enum px4io_state_t px4io_state;
static px4io_packet_t px4io_tx_packet;
static px4io_packet_t px4io_rx_packet;

// Private functions
static int PX4IO_Write(uint16_t address, uint16_t *data, uint16_t length);
static int PX4IO_Read(uint16_t address, uint16_t length);
static int PX4IO_WriteRegs(uint8_t page, 
                           uint8_t offset, 
                           uint16_t *data, 
                           uint8_t num);
static int PX4IO_ReadRegs(uint8_t page, uint8_t offset, uint8_t num);
static int PX4IO_WriteReg(uint8_t page, uint8_t offset, uint16_t data);
static uint32_t PX4IO_ReadReg(uint8_t page, uint8_t offset);
static int PX4IO_SetClearReg(uint8_t page, 
                             uint8_t offset, 
                             uint16_t setbits, 
                             uint16_t clearbits);
static int PX4IO_SetArmingState(void);
static int PX4IO_GetIOStatus(void);
static int PX4IO_GetRCPacket(uint16_t *data);
static int PX4IO_SetPWM(uint16_t *outputs, uint32_t num);

// Sync
static SemaphoreHandle_t iordy_semaphore;
static SemaphoreHandle_t timer_semaphore;
static uint32_t t0;

// Public functions
int PX4IO_Init(usart_cfg_t *usart) {
    if(usart == NULL) return -1;

    px4io_cfg.usart = usart;

    if(timer_semaphore == NULL) timer_semaphore = xSemaphoreCreateBinary();
    if(iordy_semaphore == NULL) iordy_semaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(timer_semaphore);
    xSemaphoreGive(iordy_semaphore);
    px4io_state = PX4IO_RESET;

    return 0;
}

void PX4IO_Run(void) {
    int rv;
    uint16_t outputs[PX4IO_MAX_ACTUATORS];
    switch(px4io_state) {

    case PX4IO_RESET:
        if(xSemaphoreTake(timer_semaphore, 0)) {
            t0 = xTaskGetTickCount();
        }
        if(xTaskGetTickCount() - t0 < 2000.0) {
            return;
        } else {
            xSemaphoreGive(timer_semaphore);
        }

        px4io_reg.protocol_version = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION);

        if(px4io_reg.protocol_version == PX4IO_PROTOCOL_VERSION) {

            // Check PX4IO configuration
            px4io_reg.hardware_version = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_HARDWARE_VERSION);
            px4io_reg.max_actuators    = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_ACTUATOR_COUNT);
            px4io_reg.max_controls     = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_CONTROL_COUNT);
            px4io_reg.max_transfer     = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_MAX_TRANSFER) - 2;
            px4io_reg.max_rc_input     = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_RC_INPUT_COUNT);

            if ((px4io_reg.max_actuators < 1) || (px4io_reg.max_actuators > PX4IO_MAX_ACTUATORS) ||
                (px4io_reg.max_transfer < 16) || (px4io_reg.max_transfer > 255)  ||
                (px4io_reg.max_rc_input < 1)  || (px4io_reg.max_rc_input > 255)) {
                    LOG_ERROR(device, "Wrong configuration");
                    px4io_state = PX4IO_ERROR;
            } else {
                px4io_state = PX4IO_CONF;
            }

            // Get last IO state
            PX4IO_GetIOStatus();
            if(px4io_reg.arm_status == PX4IO_READREG_ERROR) px4io_state = PX4IO_ERROR;

            // Disarm IO
            rv = PX4IO_SetClearReg(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 0,
                PX4IO_P_SETUP_ARMING_FMU_ARMED | PX4IO_P_SETUP_ARMING_LOCKDOWN);

            if(rv) {
                LOG_ERROR(device, "Failed disarming proccess");
                px4io_state = PX4IO_ERROR;
            }
        } else {
            LOG_ERROR(device, "Wrong protocol version: %lu", px4io_reg.protocol_version);
        }
        break;

    case PX4IO_CONF:
        // Here we set number of rc channels, actuators, max/min rate, max/min pwm
        rv = PX4IO_SetClearReg(PX4IO_PAGE_DISARMED_PWM, 0, 1000, PX4IO_MAX_ACTUATORS);
        rv = PX4IO_SetClearReg(PX4IO_PAGE_FAILSAFE_PWM, 0, 0, PX4IO_MAX_ACTUATORS);
        // rv = PX4IO_SetClearReg(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_PWM_RATE_GROUP0, 0, PX4IO_MAX_ACTUATORS);

        rv = PX4IO_SetClearReg(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS,
                PX4IO_P_STATUS_FLAGS_SAFETY_OFF | PX4IO_P_STATUS_FLAGS_ARM_SYNC |
                PX4IO_P_STATUS_FLAGS_INIT_OK, 0);

        px4io_reg.arm = PX4IO_DISARM;
        px4io_state = PX4IO_OPERATION;

        if(rv) {
            LOG_ERROR(device, "Configuration proccess failed");
            px4io_state = PX4IO_ERROR;
        } else {
            LOG_DEBUG(device, "Device configured");
            LOG_INFO(device, "Disarming");
        }
        break;

    case PX4IO_OPERATION:
        xSemaphoreGive(iordy_semaphore);
        PX4IO_GetRCPacket((uint16_t *)&px4io_reg.rc);
        PX4IO_GetIOStatus();
        PX4IO_UpdateOutput();
        break;

    case PX4IO_ERROR:
        LOG_ERROR(device, "Fatal error");
        break;
    }
}

int PX4IO_Operation(void) {
    if(px4io_state == PX4IO_OPERATION) {
        return 0;
    } else {
        return -1;
    }
}

int PX4IO_Ready(void) {
    xSemaphoreTake(iordy_semaphore, portMAX_DELAY);
    return 1;
}

void PX4IO_SetArm(bool arm) {
    if(arm) {
        px4io_reg.arm = PX4IO_ARM;
    } else {
        px4io_reg.arm = PX4IO_DISARM;
    }
}

void PX4IO_SetMaxPWM(int pwm) {
    px4io_reg.max_pwm = pwm;
}

void PX4IO_SetMinPWM(int pwm) {
    px4io_reg.min_pwm = pwm;
}

void PX4IO_UpdateOutput(void) {
    PX4IO_SetArmingState();
    PX4IO_SetPWM(px4io_reg.outputs, PX4IO_MAX_ACTUATORS);
}

void PX4IO_SetOutput(int channel, double out) {
    px4io_reg.min_pwm = 1000;
    px4io_reg.max_pwm = 2000;
    uint16_t out_sat;
    out = SAT(out, 1, 0);
    out_sat = (uint16_t)roundl(out * (px4io_reg.max_pwm - px4io_reg.min_pwm) + px4io_reg.min_pwm);
    if(channel > PX4IO_MAX_ACTUATORS || channel < 1) {
        LOG_ERROR(device, "Wrong output channel number");
    } else {
        px4io_reg.outputs[channel - 1] = out_sat;
    }
}

double PX4IO_GetRC(int channel) {
    if(channel > PX4IO_RC_CHANNELS || channel < 1) {
        LOG_ERROR(device, "Wrong RC channel number");
        return 0;
    } else {
        return (px4io_reg.rc[channel - 1] / 1000.f - 1.0f);
    }
}

// Private functions
static int PX4IO_Write(uint16_t address, uint16_t *data, uint16_t length) {
    int rv = 0;
    uint8_t page = address >> 8;
    uint8_t offset = address & 0xFF;

    px4io_tx_packet.count_code = length | PKT_CODE_WRITE;
    px4io_tx_packet.page = page;
    px4io_tx_packet.offset = offset;

    memcpy((void *)&px4io_tx_packet.regs[0], (void *)data, (2 * length));

    for(uint16_t i = length; i < PKT_MAX_REGS; i++) {
        px4io_tx_packet.regs[i] = 0x55AA;
    }

    px4io_tx_packet.crc = 0;
    px4io_tx_packet.crc = crc_packet(&px4io_tx_packet);

    for (uint8_t att = 0; att < 3; att++) {
        rv = USART_TransmitReceive(px4io_cfg.usart,
                                    (uint8_t *)&px4io_tx_packet, (uint8_t *)&px4io_rx_packet,
                                    sizeof(px4io_tx_packet), sizeof(px4io_rx_packet));

        if(rv == SUCCESS) {
            if (get_pkt_code(&px4io_rx_packet) == PKT_CODE_ERROR) {
                rv = EINVAL;
            }
            break;
        } else {
            //TODO Add error proccessing
            return rv;
        }
    }

    return rv;
}

static int PX4IO_Read(uint16_t address, uint16_t length) {
    int rv = 0;
    uint8_t page = address >> 8;
    uint8_t offset = address & 0xFF;

    px4io_tx_packet.count_code = length | PKT_CODE_READ;
    px4io_tx_packet.page = page;
    px4io_tx_packet.offset = offset;

    px4io_tx_packet.crc = 0;
    px4io_tx_packet.crc = crc_packet(&px4io_tx_packet);

    for (uint8_t att = 0; att < 3; att++) {
        rv = USART_TransmitReceive(px4io_cfg.usart,
                                    (uint8_t *)&px4io_tx_packet,(uint8_t *)&px4io_rx_packet,
                                    sizeof(px4io_tx_packet), sizeof(px4io_rx_packet));

        if(rv == SUCCESS) {
            if (get_pkt_code(&px4io_rx_packet) == PKT_CODE_ERROR) {
                rv = EINVAL;
            } else if (get_pkt_count(&px4io_rx_packet) != length) {
                rv = EIO;
            } else {
                break;
            }
        } else {
            //TODO Add error proccessing
            return rv;
        }
    }

    return rv;
}

static int PX4IO_WriteRegs(uint8_t page, uint8_t offset, uint16_t *data, uint8_t num) {
    int rv = 0;
    rv = PX4IO_Write((page << 8) | offset, data, num);
    return rv;
}

static int PX4IO_ReadRegs(uint8_t page, uint8_t offset, uint8_t num) {
    int rv = 0;
    rv = PX4IO_Read((page << 8) | offset, num);
    return rv;
}

static int PX4IO_WriteReg(uint8_t page, uint8_t offset, uint16_t data) {
    int rv = 0;
    rv = PX4IO_WriteRegs(page, offset, &data, 1);
    if(rv != SUCCESS) return ERROR;
    return rv;
}

static uint32_t PX4IO_ReadReg(uint8_t page, uint8_t offset) {
    int rv = 0;
    rv = PX4IO_ReadRegs(page, offset, 1);
    if(rv != SUCCESS) return PX4IO_READREG_ERROR;
    return px4io_rx_packet.regs[0];
}

static int PX4IO_SetClearReg(uint8_t page, 
                             uint8_t offset, 
                             uint16_t setbits, 
                             uint16_t clearbits) {
    int value = 0;

    value = PX4IO_ReadReg(page, offset);
    if(value < 0) return ERROR;

    value &= ~clearbits;
    value |= setbits;

    return PX4IO_WriteReg(page, offset, value);
}

static int PX4IO_SetArmingState(void) {
    int rv = 0;
    uint16_t set = 0;
    uint16_t clear = 0;

    if(px4io_reg.arm == PX4IO_ARM) {
        set |= PX4IO_P_SETUP_ARMING_FMU_ARMED;
        set |= PX4IO_P_SETUP_ARMING_FMU_PREARMED;
        set |= PX4IO_P_SETUP_ARMING_IO_ARM_OK;
        clear |= PX4IO_P_SETUP_ARMING_LOCKDOWN;
        clear |= PX4IO_P_SETUP_ARMING_FORCE_FAILSAFE;
    } else {
        set |= PX4IO_P_SETUP_ARMING_IO_ARM_OK;
        set |= PX4IO_P_SETUP_ARMING_FMU_ARMED;
        set |= PX4IO_P_SETUP_ARMING_FMU_PREARMED;
        set |= PX4IO_P_SETUP_ARMING_LOCKDOWN;
        clear |= PX4IO_P_SETUP_ARMING_FORCE_FAILSAFE;
    }

    rv = PX4IO_SetClearReg(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, set, clear);

    return rv;
}

static int PX4IO_GetIOStatus(void) {
    int rv = 0;

    rv = PX4IO_ReadRegs(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS, 6);
    if(rv) return rv;

    px4io_reg.general_status = px4io_rx_packet.regs[0];
    px4io_reg.alarms_status = px4io_rx_packet.regs[1];
    px4io_reg.vservo_status = px4io_rx_packet.regs[4];
    px4io_reg.vrssi_status  = px4io_rx_packet.regs[5];

    px4io_reg.arm_status = PX4IO_ReadReg(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING);

    return rv;
}

static int PX4IO_GetRCPacket(uint16_t *data) {
    int rv = 0;
    const uint32_t prolog = (PX4IO_P_RAW_RC_BASE - PX4IO_P_RAW_RC_COUNT);
    rv = PX4IO_ReadRegs(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_COUNT, prolog + PX4IO_RC_CHANNELS);
    for(int i = 0; i < PX4IO_RC_CHANNELS; i++){
        data[i] = px4io_rx_packet.regs[i + prolog];
    }
    return rv;
}

static int PX4IO_SetPWM(uint16_t *outputs, uint32_t num) {
    int rv = 0;
    rv = PX4IO_WriteRegs(PX4IO_PAGE_DIRECT_PWM, 0, (uint16_t *)outputs, num);
    return rv;
}
