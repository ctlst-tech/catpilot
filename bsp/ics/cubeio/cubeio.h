#ifndef CUBEIO_H
#define CUBEIO_H

#include <errno.h>
#include <string.h>

#include "core.h"
#include "log.h"
#include "os.h"
#include "periph.h"
#include "service.h"

extern gpio_t gpio_fmu_pwm[6];

#define CUBEIO_PKT_MAX_REGS 22
#define CUBEIO_MAX_CHANNELS 16

#pragma pack(push, 1)
typedef struct {
    uint8_t count : 6;
    uint8_t code : 2;
    uint8_t crc;
    uint8_t page;
    uint8_t offset;
    uint16_t regs[CUBEIO_PKT_MAX_REGS];
} cubeio_packet_t;
#pragma pack(pop)

typedef struct {
    uint16_t protocol_version;
    uint16_t protocol_version2;
} cubeio_page_config_t;

typedef struct {
    uint8_t num_channels;
    uint16_t pwm[CUBEIO_MAX_CHANNELS];
    uint16_t failsafe_pwm[CUBEIO_MAX_CHANNELS];
    uint8_t heater_duty;
    uint16_t safety_mask;
} cubeio_pwm_out_t;

typedef struct {
    uint16_t pwm[CUBEIO_MAX_CHANNELS];
} cubeio_pwm_in_t;

typedef struct {
    uint16_t freq;
    uint16_t chmask;
    uint16_t default_freq;
    uint16_t sbus_rate_hz;
    uint8_t oneshot_enabled;
    uint8_t brushed_enabled;
} cubeio_rate_t;

typedef struct {
    uint16_t freemem;
    uint32_t timestamp_ms;
    uint16_t vservo;
    uint16_t vrssi;
    uint32_t num_errors;
    uint32_t total_pkts;
    uint8_t flag_safety_off;
    uint8_t safety_forced_off;
    uint8_t err_crc;
    uint8_t err_bad_opcode;
    uint8_t err_read;
    uint8_t err_write;
    uint8_t err_uart;
} cubeio_page_reg_status_t;

typedef struct {
    uint8_t count;
    uint8_t flags_failsafe : 1;
    uint8_t flags_rc_ok : 1;
    uint8_t rc_protocol;
    uint16_t channel[CUBEIO_MAX_CHANNELS];
    int16_t rssi;
} cubeio_page_rc_input_t;

/*
  data for mixing on FMU failsafe
 */
typedef struct {
    uint16_t servo_min[CUBEIO_MAX_CHANNELS];
    uint16_t servo_max[CUBEIO_MAX_CHANNELS];
    uint16_t servo_trim[CUBEIO_MAX_CHANNELS];
    uint8_t servo_function[CUBEIO_MAX_CHANNELS];
    uint8_t servo_reversed[CUBEIO_MAX_CHANNELS];

    // RC input arrays are in AETR order
    uint16_t rc_min[4];
    uint16_t rc_max[4];
    uint16_t rc_trim[4];
    uint8_t rc_reversed[CUBEIO_MAX_CHANNELS];
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
} cubeio_page_mixing_t;

typedef struct __attribute__((packed, aligned(2))) {
    uint8_t channel_mask;
    uint8_t output_mask;
} cubeio_page_gpio_t;

typedef struct {
    uint16_t type;
    uint16_t min;
    uint16_t max;
} cubeio_range_cfg_t;

// Sync
typedef struct {
    SemaphoreHandle_t iordy_semaphore;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t timeout_semaphore;
    uint32_t now;
    uint32_t last_rc_read_ms;
    uint32_t last_status_read_ms;
    uint32_t last_servo_read_ms;
    uint32_t last_safety_ms;
} cubeio_sync_t;

enum cubeio_state_t {
    CUBEIO_RESET = 0,
    CUBEIO_CONF = 1,
    CUBEIO_OPERATION = 2,
    CUBEIO_FAIL = 3,
};

enum cubeio_event_t {
    CUBEIO_SET_PWM = 1,
    CUBEIO_SET_FAILSAFE_PWM = 2,
    CUBEIO_FORCE_SAFETY_OFF = 3,
    CUBEIO_FORCE_SAFETY_ON = 4,
    CUBEIO_ENABLE_SBUS_OUT = 5,
    CUBEIO_ONESHOT_ON = 6,
    CUBEIO_BRUSHED_ON = 7,
    CUBEIO_SET_RATES = 8,
    CUBEIO_SET_IMU_HEATER_DUTY = 9,
    CUBEIO_SET_DEFAULT_RATE = 10,
    CUBEIO_SET_SAFETY_MASK = 11,
    CUBEIO_MIXING = 12,
    CUBEIO_GPIO = 13,
};

enum cubeio_type_t {
    CUBEIO_RC = 0,
    CUBEIO_PWM = 1,
};

enum cubeio_channel_type_t {
    CUBEIO_CHANNEL_UNIPOLAR = 0,
    CUBEIO_CHANNEL_BIPOLAR = 1,
};

typedef struct {
    usart_t *usart;
} cubeio_interface_t;


typedef uint32_t cubeio_eventmask_t;

typedef struct {
    char name[MAX_NAME_LEN];
    service_t *service;
    cubeio_interface_t interface;
    enum cubeio_state_t state;
    cubeio_sync_t sync;
    cubeio_packet_t tx_packet;
    cubeio_packet_t rx_packet;
    cubeio_eventmask_t eventmask;
    cubeio_page_config_t config;
    cubeio_pwm_out_t pwm_out;
    cubeio_pwm_in_t pwm_in;
    cubeio_rate_t rate;
    cubeio_page_reg_status_t page_reg_status;
    cubeio_page_rc_input_t page_rc_input;
    cubeio_range_cfg_t rc_range_cfg[CUBEIO_MAX_CHANNELS];
    cubeio_range_cfg_t pwm_range_cfg[CUBEIO_MAX_CHANNELS];
    double pwm[CUBEIO_MAX_CHANNELS];
    double failsafe_pwm;
    double rc[CUBEIO_MAX_CHANNELS];
    uint32_t attempt;
} cubeio_t;

cubeio_t *cubeio_start(char *name, uint32_t period, uint32_t priority,
                       usart_t *usart);
void cubeio_set_range(cubeio_t *dev, int type, uint8_t channel, uint16_t channel_type,
                      uint16_t min, uint16_t max);
void cubeio_set_pwm(cubeio_t *dev, uint8_t channels, double *pwm);
void cubeio_set_failsafe_pwm(cubeio_t *dev, double pwm);
void cubeio_get_rc(cubeio_t *dev, double *ptr);
uint16_t cubeio_get_pwm_channel(cubeio_t *dev, uint8_t channel);
void cubeio_set_safety_mask(cubeio_t *dev, uint16_t safety_mask);
void cubeio_set_freq(cubeio_t *dev, uint16_t chmask, uint16_t freq);
uint16_t cubeio_get_freq(cubeio_t *dev, uint16_t channel);
void cubeio_set_default_freq(cubeio_t *dev, uint16_t freq);
void cubeio_set_oneshot_mode(cubeio_t *dev);
void cubeio_set_brushed_mode(cubeio_t *dev);
int cubeio_get_safety_switch_state(cubeio_t *dev);
void cubeio_force_safety_on(cubeio_t *dev);
void cubeio_force_safety_off(cubeio_t *dev);
void cubeio_set_imu_heater_duty(cubeio_t *dev, uint8_t duty);
int16_t cubeio_get_rssi(cubeio_t *dev);
void cubeio_enable_sbus_out(cubeio_t *dev, uint16_t freq);
void cubeio_stat(cubeio_t *dev);

#endif  // CUBEIO_H
