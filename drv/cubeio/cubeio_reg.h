#pragma once
#include "stm32_base.h"
#include "bit.h"

#define PKT_MAX_REGS                        22
#define MAX_CHANNELS                        16

#pragma pack(push, 1)
typedef struct {
    uint8_t count_code;
    uint8_t crc;
    uint8_t page;
    uint8_t offset;
    uint16_t regs[PKT_MAX_REGS];
} cubeio_packet_t;
#pragma pack(pop)

#define PAGE_CONFIG                         0
#define PAGE_STATUS                         1
#define PAGE_ACTUATORS                      2
#define PAGE_SERVOS                         3
#define PAGE_RAW_RCIN                       4
#define PAGE_RCIN                           5
#define PAGE_RAW_ADC                        6
#define PAGE_PWM_INFO                       7
#define PAGE_SETUP                          50
#define PAGE_DIRECT_PWM                     54
#define PAGE_FAILSAFE_PWM                   55
#define PAGE_MIXING                         200
#define PAGE_GPIO                           201

#define PROTOCOL_VERSION                    4
#define PROTOCOL_VERSION2                   10
#define PAGE_CONFIG_PROTOCOL_VERSION        0
#define PAGE_CONFIG_PROTOCOL_VERSION2       1

#define PAGE_REG_SETUP_FEATURES             0
#define P_SETUP_FEATURES_SBUS1_OUT          1
#define P_SETUP_FEATURES_SBUS2_OUT          2
#define P_SETUP_FEATURES_PWM_RSSI           4
#define P_SETUP_FEATURES_ADC_RSSI           8
#define P_SETUP_FEATURES_ONESHOT            16
#define P_SETUP_FEATURES_BRUSHED            32

#define PAGE_REG_SETUP_ARMING               1
#define P_SETUP_ARMING_IO_ARM_OK            (1<<0)
#define P_SETUP_ARMING_FMU_ARMED            (1<<1)
#define P_SETUP_ARMING_RC_HANDLING_DISABLED (1<<6)
#define P_SETUP_ARMING_SAFETY_DISABLE_ON    (1 << 11)
#define P_SETUP_ARMING_SAFETY_DISABLE_OFF   (1 << 12)

#define PAGE_REG_SETUP_PWM_RATE_MASK        2
#define PAGE_REG_SETUP_DEFAULTRATE          3
#define PAGE_REG_SETUP_ALTRATE              4
#define PAGE_REG_SETUP_REBOOT_BL            10
#define PAGE_REG_SETUP_CRC                  11
#define PAGE_REG_SETUP_SBUS_RATE            19
#define PAGE_REG_SETUP_IGNORE_SAFETY        20
#define PAGE_REG_SETUP_HEATER_DUTY_CYCLE    21
#define PAGE_REG_SETUP_DSM_BIND             22
#define PAGE_REG_SETUP_RC_PROTOCOLS         23

#define REBOOT_BL_MAGIC                     14662

#define PAGE_REG_SETUP_FORCE_SAFETY_OFF     12
#define PAGE_REG_SETUP_FORCE_SAFETY_ON      14
#define FORCE_SAFETY_MAGIC                  22027

#define PKT_CODE_READ                       0
#define PKT_CODE_WRITE                      1

#define PKT_CODE_SUCCESS                    0
#define PKT_CODE_CORRUPT                    1
#define PKT_CODE_ERROR                      2

struct page_config {
    uint16_t protocol_version;
    uint16_t protocol_version2;
};

struct page_reg_status {
    uint16_t freemem;
    uint32_t timestamp_ms;
    uint16_t vservo;
    uint16_t vrssi;
    uint32_t num_errors;
    uint32_t total_pkts;
    uint8_t flag_safety_off;
    uint8_t err_crc;
    uint8_t err_bad_opcode;
    uint8_t err_read;
    uint8_t err_write;
    uint8_t err_uart;
};

struct page_rc_input {
    uint8_t count;
    uint8_t flags_failsafe:1;
    uint8_t flags_rc_ok:1;
    uint8_t rc_protocol;
    uint16_t pwm[MAX_CHANNELS];
    int16_t rssi;
};

/*
  data for mixing on FMU failsafe
 */
struct page_mixing {
    uint16_t servo_min[MAX_CHANNELS];
    uint16_t servo_max[MAX_CHANNELS];
    uint16_t servo_trim[MAX_CHANNELS];
    uint8_t servo_function[MAX_CHANNELS];
    uint8_t servo_reversed[MAX_CHANNELS];

    // RC input arrays are in AETR order
    uint16_t rc_min[4];
    uint16_t rc_max[4];
    uint16_t rc_trim[4];
    uint8_t rc_reversed[MAX_CHANNELS];
    uint8_t rc_channel[4];

    // gain for elevon and vtail mixing, x1000
    uint16_t mixing_gain;

    // channel which when high forces mixer
    int8_t rc_chan_override;

    // is the throttle an angle input?
    uint8_t throttle_is_angle;

    // mask of channels which are pure manual in override
    uint16_t manual_rc_mask;

    // enabled needs to be 1 to enable mixing
    uint8_t enabled;

    uint8_t pad;
};

struct __attribute__((packed, aligned(2))) page_GPIO {
    uint8_t channel_mask;
    uint8_t output_mask;
};

#pragma pack(push, 1)
typedef struct {
    uint16_t general_status;
    uint16_t vservo_status;
    uint16_t vrssi_status;
    uint16_t alarms_status;
    uint32_t arm_status;
    uint16_t rc[MAX_CHANNELS];
    uint16_t outputs[MAX_CHANNELS];
    uint32_t protocol_version;
    uint32_t hardware_version;
    uint32_t max_actuators;
    uint32_t max_controls;
    uint32_t max_transfer;
    uint32_t max_rc_input;
    uint16_t arm;
    uint16_t max_pwm;
    uint16_t min_pwm;
} cubeio_reg_t;
#pragma pack(pop)
