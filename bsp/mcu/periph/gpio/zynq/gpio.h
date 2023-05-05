#ifndef GPIO_MAP
#define GPIO_MAP

#include <stdint.h>

//global adresses
#define IO_BASE_ADDR            0x43C00000
#define SDPU_BASE_ADDR          0x43C01000
#define DMA_BASE_ADDR           0x43C01800

//addresses of IO
#define PIN_SETUP_BASE_ADDR     0x43C00000
#define OUT_SETUP_BASE_ADDR     0x43C00200
#define OUT_VALUE_BASE_ADDR     0x43C00400
#define GUARD_SETUP_BASE_ADDR   0x43C00600
#define GUARD_VALUE_BASE_ADDR   0x43C00800
#define INPUT_SETUP_BASE_ADDR   0x43C00A00
#define INPUT_VALUE_BASE_ADDR   0x43C00C00

#define PIN_SETUP_OFF     0x000
#define OUT_SETUP_OFF     0x200
#define OUT_VALUE_OFF     0x400
#define GUARD_SETUP_OFF   0x600
#define GUARD_VALUE_OFF   0x800
#define INPUT_SETUP_OFF   0xA00
#define INPUT_VALUE_OFF   0xC00

#define IO_REG_MAP_SIZE         3584

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

typedef struct {
    uintptr_t base;
    uintptr_t phys_base;
} gpio_instance_t;

int gpio_init(gpio_instance_t *i, uintptr_t phys_base);
int gpio_set_discrete_mode(gpio_instance_t *i, uint32_t channel);
int gpio_set_output_value(gpio_instance_t *i, uint32_t channel, uint32_t value);
int gpio_get_output_value(gpio_instance_t *i, uint32_t channel, uint32_t *value);

#endif //  GPIO_MAP