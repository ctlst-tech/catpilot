#ifndef GPIO_MAP
#define GPIO_MAP

#include <stdint.h>

#define IO_BASE_ADDR            0x43C90000
#define SDPU_BASE_ADDR          0x43C91000
#define DMA_BASE_ADDR           0x43C91800

// IO_BASE
#define PIN_SETUP_BASE_ADDR     0x43C90000
#define OUT_SETUP_BASE_ADDR     0x43C90200
#define OUT_VALUE_BASE_ADDR     0x43C90400
#define GUARD_SETUP_BASE_ADDR   0x43C90600
#define GUARD_VALUE_BASE_ADDR   0x43C90800
#define INPUT_SETUP_BASE_ADDR   0x43C90A00
#define INPUT_VALUE_BASE_ADDR   0x43C90C00

// OFFSETS
#define PIN_SETUP_OFF     0x000
#define OUT_SETUP_OFF     0x200
#define OUT_VALUE_OFF     0x400
#define GUARD_SETUP_OFF   0x600
#define GUARD_VALUE_OFF   0x800
#define INPUT_SETUP_OFF   0xA00
#define INPUT_VALUE_OFF   0xC00

#define IO_REG_MAP_SIZE   14336

#define PIN_REG_AMOUNT 8
#define BYTES_IN_REG 4
#define PIN_BYTE_RANGE PIN_REG_AMOUNT * BYTES_IN_REG

// PIN SETUP REG
#define IO_OFF                      0x00
#define FDIO_SETUP_OFF              0x04
#define CPS_FI_SETUP_OFF            0x08

// OUTPUT SETUP REG
#define OUTPUT_MUX_REG_OFF          0x0
#define DISCRETE_OUT_STATUS_REG_OFF 0x4
#define PWM_CTRL_REG_OFF            0x8
#define PWM_STATUS_REG_OFF          0xC
#define PHASE_CTRL_REG_OFF          0x10
#define PHASE_STATUS_REG_OFF        0x14

// INPUT SETUP REG
#define PIN_SOURCE_REG_OFF                   0x0
#define AGLITCH_FILTER_SETUP_OFF             0x4
#define PWM_ESTIMATOR_CTRL_OFF               0x8
#define PHASE_ESTIMATOR_CTRL_OFF             0xC
#define PH_EST_ALL_TOOTH_NUM_OFF             0x10
#define PH_EST_ABS_TOOTH_NUM_OFF             0x14
#define PH_EST_LOG2_PHASE_STEP_TOOTH_NUM_OFF 0x18

// OUTPUT_MUX_REG / PIN_SOURCE_REG
#define IN_OUT_MUX_PIN_OFF         0x0
#define IN_OUT_MUX_DISCRETE_MODE   0x1
#define IN_OUT_MUX_PWM_MODE        0x2
#define IN_OUT_MUX_PHASE_MODE      0x3

#define PWM_ENABLE_POS  1

#define GPIO_PINS 16
#define GPIO_INPUT_OUTPUT_MAX 4
#define GPIO_INPUT_OUTPUT_OFF 12

int gpio_init(uint32_t channel);
int gpio_set_discrete_mode_out(uint32_t channel);
int gpio_set_pwm_mode_out(uint32_t channel);
int gpio_set_phase_mode_out(uint32_t channel);
int gpio_set_discrete_mode_in(uint32_t channel);
int gpio_set_pwm_mode_in(uint32_t channel);
int gpio_set_phase_mode_in(uint32_t channel, uint32_t total, uint32_t missing,
                           uint32_t step);
int gpio_set_output_value(uint32_t channel, uint32_t value);
int gpio_get_output_value(uint32_t channel, uint32_t *value);
int gpio_set_period(uint32_t channel, uint32_t period);
int gpio_set_width(uint32_t channel, uint32_t width);
int gpio_get_period(uint32_t channel, uint32_t *period);
int gpio_get_width(uint32_t channel, uint32_t *width);
int gpio_get_input_value(uint32_t channel, uint32_t *value);
#endif  // GPIO_MAP
