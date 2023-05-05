#ifndef GPIO_MAP
#define GPIO_MAP

#include <stdint.h>

//global adresses
#define IO_BASE_ADDR            0x43C90000
#define SDPU_BASE_ADDR          0x43C91000
#define DMA_BASE_ADDR           0x43C91800

//addresses of IO
#define PIN_SETUP_BASE_ADDR     0x43C90000
#define OUT_SETUP_BASE_ADDR     0x43C90200
#define OUT_VALUE_BASE_ADDR     0x43C90400
#define GUARD_SETUP_BASE_ADDR   0x43C90600
#define GUARD_VALUE_BASE_ADDR   0x43C90800
#define INPUT_SETUP_BASE_ADDR   0x43C90A00
#define INPUT_VALUE_BASE_ADDR   0x43C90C00

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

//regs of OUTPUT SETUP REG
#define OUTPUT_MUX_REG_OFF          0x0
#define DISCRETE_OUT_STATUS_REG_OFF 0x4
#define PWM_CTRL_REG_OFF            0x8
#define PWM_STATUS_REG_OFF          0xC
#define PHASE_CTRL_REG_OFF          0x10
#define PHASE_STATUS_REG_OFF        0x14

#define OUT_MUX_PIN_OFF         0x0
#define OUT_MUX_DISCRETE_MODE   0x1
#define OUT_MUX_PWM_MODE        0x2
#define OUT_MUX_PHASE_MODE      0x3

#define PWM_ENABLE_POS  1

#define GPIO_PINS   16

int gpio_init(uint32_t channel);
int gpio_set_discrete_mode(uint32_t channel);
int gpio_set_pwm_mode(uint32_t channel);
int gpio_set_output_value(uint32_t channel, uint32_t value);
int gpio_get_output_value(uint32_t channel, uint32_t *value);
int gpio_set_period(uint32_t channel, uint32_t period);
int gpio_set_width(uint32_t channel, uint32_t width);

#endif //  GPIO_MAP
