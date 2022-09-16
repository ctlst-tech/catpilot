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

#define PKT_CODE_READ                       0x00
#define PKT_CODE_WRITE                      0x40

#define PKT_CODE_SUCCESS                    0x00
#define PKT_CODE_CORRUPT                    0x01
#define PKT_CODE_ERROR                      0x02
#define PKT_CODE_MASK                       0xC0
#define PKT_COUNT_MASK                      0x3F

#define PKT_COUNT(_str)   ((_str).count_code & PKT_COUNT_MASK)
#define PKT_CODE(_str)    ((_str).count_code & PKT_CODE_MASK)
#define PKT_SIZE(_str)    ((size_t)((uint8_t *)&((_str).regs[PKT_COUNT(_str)]) - ((uint8_t *)&(_str))))

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

static uint16_t get_pkt_count(cubeio_packet_t *pkt) {
    return (pkt->count_code & PKT_COUNT_MASK);
}

static uint16_t get_pkt_code(cubeio_packet_t *pkt) {
    return (pkt->count_code & PKT_CODE_MASK);
}

static uint32_t get_pkt_size(cubeio_packet_t *pkt) {
    return ((size_t)((uint8_t *)(pkt->regs[get_pkt_count(pkt)]) - ((uint8_t *)(pkt))));
}

static const uint8_t crc8_tab[256] __attribute__((unused)) = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

static uint8_t crc_packet(cubeio_packet_t *pkt) __attribute__((unused));
static uint8_t crc_packet(cubeio_packet_t *pkt) {
    uint8_t *end = (uint8_t *)(&pkt->regs[PKT_COUNT(*pkt)]);
    uint8_t *p = (uint8_t *)pkt;
    uint8_t c = 0;

    while (p < end) {
        c = crc8_tab[c ^ * (p++)];
    }

    return c;
}
