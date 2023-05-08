#include "gpio.h"

#include <hw/inout.h>
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/neutrino.h>

#include "core.h"

#define WRITE_REG(__off, __d) out32(gpio->base + (__off), (__d))
#define READ_REG(__off) in32(gpio->base + (__off))
#define CH_OFF(ch) ((ch * PIN_REG_AMOUNT * BYTES_IN_REG))

static int gpio_check_output(uint32_t channel);
static int gpio_check_input(uint32_t channel);
static int gpio_add_output(uint32_t channel);
static int gpio_add_input(uint32_t channel);

typedef struct {
    uintptr_t base;
    uintptr_t phys_base;
} gpio_instance_t;

static gpio_instance_t *gpio = NULL;
static uint32_t channels = 0;

int gpio_init(uint32_t channel) {
    if (channel >= GPIO_PINS) {
        printf("channel out of range\n");
        return -1;
    }
    if (gpio == NULL) {
        gpio = calloc(1, sizeof(gpio_instance_t));
        if (gpio == NULL) {
            perror("calloc failed");
            return -1;
        }
        gpio->phys_base = IO_BASE_ADDR;
        gpio->base = mmap_device_io(IO_REG_MAP_SIZE, gpio->phys_base);
        if (gpio->base == NULL) {
            perror("mmap_device_io");
            return -1;
        }
        if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
            perror("ThreadCtl failed");
            free(gpio);
            return -1;
        }
    }
    return 0;
}

static int gpio_check_common(uint32_t channel) {
    if (gpio == NULL || channel >= GPIO_PINS) {
        printf("gpio_check_common failed\n");
        return -1;
    }
    return 0;
}

static int gpio_check_output(uint32_t channel) {
    if (gpio_check_common(channel)) {
        return -1;
    }
    if (channels & (1 << channel)) {
        return -1;
    }
    if (channel >= GPIO_INPUT_OUTPUT_OFF &&
        (channels & (1 << channel) << GPIO_PINS)) {
        return -1;
    }
    return 0;
}

static int gpio_check_input(uint32_t channel) {
    if (gpio_check_common(channel)) {
        return -1;
    }
    if (channels & (1 << channel) << GPIO_PINS) {
        return -1;
    }
    if (channel >= GPIO_INPUT_OUTPUT_OFF && (channels & (1 << channel))) {
        return -1;
    }
    return 0;
}

static int gpio_add_output(uint32_t channel) {
    if (gpio_check_output(channel)) {
        printf("channel is already reserved\n");
        return -1;
    }
    if (channel >= GPIO_INPUT_OUTPUT_OFF) {
        uint32_t reg = 0;
        reg = READ_REG(IO_OFF + FDIO_SETUP_OFF);
        WRITE_REG(IO_OFF + FDIO_SETUP_OFF,
                  ~(1 << (channel - GPIO_INPUT_OUTPUT_OFF)) & reg);
    }
    channels |= 1 << channel;
    return 0;
}

static int gpio_add_input(uint32_t channel) {
    if (gpio_check_input(channel)) {
        printf("channel is already reserved\n");
        return -1;
    }
    if (channel >= GPIO_INPUT_OUTPUT_OFF) {
        uint32_t reg = 0;
        reg = READ_REG(IO_OFF + FDIO_SETUP_OFF);
        WRITE_REG(IO_OFF + FDIO_SETUP_OFF,
                  (1 << (channel - GPIO_INPUT_OUTPUT_OFF)) | 0xF);
    }
    channels |= (1 << channel) << GPIO_PINS;
    return 0;
}

int gpio_set_discrete_mode_out(uint32_t channel) {
    if (gpio_add_output(channel)) {
        return -1;
    }
    WRITE_REG(OUT_SETUP_OFF + CH_OFF(channel), IN_OUT_MUX_DISCRETE_MODE);
    return 0;
}

int gpio_set_pwm_mode_out(uint32_t channel) {
    if (gpio_add_output(channel)) {
        return -1;
    }
    WRITE_REG(OUT_SETUP_OFF + CH_OFF(channel), IN_OUT_MUX_PWM_MODE);
    WRITE_REG(OUT_SETUP_OFF + CH_OFF(channel) + PWM_CTRL_REG_OFF, 1);
    return 0;
}

int gpio_set_phase_mode_out(uint32_t channel, uint32_t invert,
                            uint32_t time_mode, uint32_t sync_channel,
                            uint32_t tooth, uint32_t phase_on,
                            uint32_t phase_off) {
    if (gpio_add_output(channel)) {
        return -1;
    }
    WRITE_REG(OUT_SETUP_OFF + CH_OFF(channel), IN_OUT_MUX_PHASE_MODE);
    WRITE_REG(OUT_SETUP_OFF + CH_OFF(channel) + PHASE_CTRL_REG_OFF,
              (1 << PHASE_GEN_ENABLE_POS) |
                  (invert << PHASE_GEN_INV_OUTPUT_POS) |
                  (time_mode << PHASE_GEN_TIME_MODE_POS) |
                  (sync_channel << PHASE_GEN_CHN_NUM_START_POS));
    WRITE_REG(OUT_VALUE_OFF + CH_OFF(channel), tooth);
    WRITE_REG(OUT_VALUE_OFF + CH_OFF(channel) + BYTES_IN_REG, phase_on);
    WRITE_REG(OUT_VALUE_OFF + CH_OFF(channel) + 2 * BYTES_IN_REG, phase_off);
    return 0;
}

int gpio_set_discrete_mode_in(uint32_t channel) {
    if (gpio_add_input(channel)) {
        return -1;
    }
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel), IN_OUT_MUX_DISCRETE_MODE);
    return 0;
}

int gpio_set_pwm_mode_in(uint32_t channel) {
    if (gpio_add_input(channel)) {
        return -1;
    }
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel), IN_OUT_MUX_PWM_MODE);
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel) + PWM_ESTIMATOR_CTRL_OFF, 1);
    return 0;
}

int gpio_set_phase_mode_in(uint32_t channel, uint32_t total, uint32_t missing,
                           uint32_t step) {
    if (gpio_add_input(channel)) {
        return -1;
    }
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel), IN_OUT_MUX_PHASE_MODE);
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel) + PH_EST_ALL_TOOTH_NUM_OFF,
              total);
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel) + PH_EST_ABS_TOOTH_NUM_OFF,
              missing);
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel) +
                  PH_EST_LOG2_PHASE_STEP_TOOTH_NUM_OFF,
              step);
    WRITE_REG(INPUT_SETUP_OFF + CH_OFF(channel) + PHASE_ESTIMATOR_CTRL_OFF, 1);
    return 0;
}

int gpio_set_output_value(uint32_t channel, uint32_t value) {
    if (!gpio_check_output(channel)) {
        return -1;
    }
    WRITE_REG(OUT_VALUE_OFF + CH_OFF(channel), value);
    return 0;
}

int gpio_get_output_value(uint32_t channel, uint32_t *value) {
    if (!gpio_check_output(channel)) {
        return -1;
    }
    *value = READ_REG(OUT_VALUE_OFF + CH_OFF(channel));
    return 0;
}

int gpio_set_period(uint32_t channel, uint32_t period) {
    if (!gpio_check_output(channel)) {
        return -1;
    }
    WRITE_REG(OUT_VALUE_OFF + CH_OFF(channel), period * MCK_US_TO_TICKS);
    return 0;
}

int gpio_set_width(uint32_t channel, uint32_t width) {
    if (!gpio_check_output(channel)) {
        return -1;
    }
    WRITE_REG(OUT_VALUE_OFF + CH_OFF(channel) + sizeof(uint32_t),
              width * MCK_US_TO_TICKS);
    return 0;
}

int gpio_get_period(uint32_t channel, uint32_t *period) {
    if (!gpio_check_input(channel)) {
        return -1;
    }
    *period = READ_REG(INPUT_VALUE_OFF + CH_OFF(channel));
    *period /= MCK_US_TO_TICKS;
    return 0;
}

int gpio_get_width(uint32_t channel, uint32_t *width) {
    if (!gpio_check_input(channel)) {
        return -1;
    }
    *width = READ_REG(INPUT_VALUE_OFF + CH_OFF(channel) + sizeof(uint32_t));
    *width /= MCK_US_TO_TICKS;
    return 0;
}

int gpio_get_input_value(uint32_t channel, uint32_t *value) {
    if (!gpio_check_input(channel)) {
        return -1;
    }
    *value = READ_REG(INPUT_VALUE_OFF + CH_OFF(channel));
    return 0;
}

int gpio_get_tooth(uint32_t channel, uint32_t *tooth) {
    if (!gpio_check_input(channel)) {
        return -1;
    }
    *tooth = READ_REG(INPUT_VALUE_OFF + CH_OFF(channel) + sizeof(uint32_t));
    return 0;
}

int gpio_get_phase_step(uint32_t channel, uint32_t *step) {
    if (!gpio_check_input(channel)) {
        return -1;
    }
    *step = READ_REG(INPUT_VALUE_OFF + CH_OFF(channel) + sizeof(uint32_t) +
                     sizeof(uint32_t));
    return 0;
}
