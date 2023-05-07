#include "gpio.h"

#include <hw/inout.h>
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/neutrino.h>

#include "core.h"

#define WRITE_REG(__r, __d) out32((__r), (__d))
#define READ_REG(r) in32(r)
#define CH_OFF(ch) ((ch * sizeof(uint32_t) * 8))

typedef struct {
    uintptr_t base;
    uintptr_t phys_base;
} gpio_instance_t;

static gpio_instance_t *gpio = NULL;
static uint8_t channels[GPIO_PINS] = {0};

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
    // if (channels[channel]) {
    //     return -1;
    // }
    channels[channel] = 1;
    return 0;
}

static int gpio_check(uint32_t channel) {
    if (gpio == NULL || channel >= GPIO_PINS) {
        return -1;
    }
    return 0;
}

int gpio_set_discrete_mode_out(uint32_t channel) {
    if (gpio_check(channel)) {
        return -1;
    }
    WRITE_REG(gpio->base + OUT_SETUP_OFF + CH_OFF(channel),
              IN_OUT_MUX_DISCRETE_MODE);
    return 0;
}

int gpio_set_pwm_mode_out(uint32_t channel) {
    if (gpio_check(channel)) {
        return -1;
    }
    WRITE_REG(gpio->base + OUT_SETUP_OFF + CH_OFF(channel),
              IN_OUT_MUX_PWM_MODE);
    WRITE_REG(gpio->base + OUT_SETUP_OFF + CH_OFF(channel) + PWM_CTRL_REG_OFF,
              1);
    return 0;
}

int gpio_set_phase_mode_out(uint32_t channel) {
    if (gpio_check(channel)) {
        return -1;
    }
    WRITE_REG(gpio->base + OUT_SETUP_OFF + CH_OFF(channel),
              IN_OUT_MUX_PHASE_MODE);
    return 0;
}

int gpio_set_discrete_mode_in(uint32_t channel) {
    if (gpio_check(channel)) {
        return -1;
    }
    WRITE_REG(gpio->base + INPUT_SETUP_OFF + CH_OFF(channel),
              IN_OUT_MUX_DISCRETE_MODE);
    return 0;
}

int gpio_set_pwm_mode_in(uint32_t channel) {
    if (gpio_check(channel)) {
        return -1;
    }
    WRITE_REG(gpio->base + INPUT_SETUP_OFF + CH_OFF(channel),
              IN_OUT_MUX_PWM_MODE);
    WRITE_REG(
        gpio->base + INPUT_SETUP_OFF + CH_OFF(channel) + PWM_ESTIMATOR_CTRL_OFF,
        1);
    return 0;
}

int gpio_set_phase_mode_in(uint32_t channel) {
    if (gpio_check(channel)) {
        return -1;
    }
    WRITE_REG(gpio->base + INPUT_SETUP_OFF + CH_OFF(channel),
              IN_OUT_MUX_PHASE_MODE);
    return 0;
}

int gpio_set_output_value(uint32_t channel, uint32_t value) {
    if (gpio_check(channel)) {
        return -1;
    }
    WRITE_REG(gpio->base + OUT_VALUE_OFF + CH_OFF(channel), value);
    return 0;
}

int gpio_get_output_value(uint32_t channel, uint32_t *value) {
    if (gpio_check(channel)) {
        return -1;
    }
    *value = READ_REG(gpio->base + OUT_VALUE_OFF + CH_OFF(channel));
    return 0;
}

int gpio_set_period(uint32_t channel, uint32_t period) {
    if (gpio_check(channel)) {
        return -1;
    }
    // printf("set period = %u\n", period * MCK_US_TO_TICKS);
    WRITE_REG(gpio->base + OUT_VALUE_OFF + CH_OFF(channel),
              period * MCK_US_TO_TICKS);
    return 0;
}

int gpio_set_width(uint32_t channel, uint32_t width) {
    if (gpio_check(channel)) {
        return -1;
    }
    // printf("set width = %u\n", width * MCK_US_TO_TICKS);
    WRITE_REG(gpio->base + OUT_VALUE_OFF + CH_OFF(channel) + sizeof(uint32_t),
              width * MCK_US_TO_TICKS);
    return 0;
}

int gpio_get_width(uint32_t channel, uint32_t *width) {
    if (gpio_check(channel)) {
        return -1;
    }
    *width = READ_REG(gpio->base + INPUT_VALUE_OFF + CH_OFF(channel) +
                      sizeof(uint32_t));
    // printf("get width = %u\n", *width / MCK_US_TO_TICKS);
    *width /= MCK_US_TO_TICKS;
    return 0;
}

int gpio_get_period(uint32_t channel, uint32_t *period) {
    if (gpio_check(channel)) {
        return -1;
    }
    *period = READ_REG(gpio->base + INPUT_VALUE_OFF + CH_OFF(channel));
    // printf("get period = %u\n", *period / MCK_US_TO_TICKS);
    *period /= MCK_US_TO_TICKS;
    return 0;
}

int gpio_get_input_value(uint32_t channel, uint32_t *value) {
    if (gpio_check(channel)) {
        return -1;
    }
    *value = READ_REG(gpio->base + INPUT_VALUE_OFF + CH_OFF(channel));
    return 0;
}
