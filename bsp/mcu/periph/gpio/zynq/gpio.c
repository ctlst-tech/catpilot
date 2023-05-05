#include "gpio.h"
#include "core.h"

#include <hw/inout.h>
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/neutrino.h>

#define WRITE_REG(__r, __d) out32((__r), (__d))
#define READ_REG(r) in32(r)

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
    if (channels[channel]) {
        return -1;
    }
    channels[channel] = 1;
    return 0;
}

int gpio_set_discrete_mode(uint32_t channel) {
    if (gpio == NULL || channel >= GPIO_PINS || channels[channel] == 0) {
        return -1;
    }
    WRITE_REG(gpio->base + OUT_SETUP_OFF + channel * sizeof(uint32_t),
              OUT_MUX_DISCRETE_MODE);
    return 0;
}

int gpio_set_pwm_mode(uint32_t channel) {
    if (gpio == NULL || channel >= GPIO_PINS || channels[channel] == 0) {
        return -1;
    }
    WRITE_REG(gpio->base + OUT_SETUP_OFF + channel * sizeof(uint32_t),
              OUT_MUX_PWM_MODE);
    return 0;
}

int gpio_set_output_value(uint32_t channel, uint32_t value) {
    if (gpio == NULL || channel >= GPIO_PINS || channels[channel] == 0) {
        return -1;
    }
    WRITE_REG(gpio->base + OUT_VALUE_OFF + channel * sizeof(uint32_t), value);
    return 0;
}

int gpio_get_output_value(uint32_t channel, uint32_t *value) {
    if (gpio == NULL || channel >= GPIO_PINS || channels[channel] == 0) {
        return -1;
    }
    *value = READ_REG(gpio->base + OUT_VALUE_OFF + channel * sizeof(uint32_t));
    return 0;
}

int gpio_set_period(uint32_t channel, uint32_t period) {
    if (gpio == NULL || channel >= GPIO_PINS || channels[channel] == 0) {
        return -1;
    }
    printf("period = %u\n", period * MCK_US_TO_TICKS);
    WRITE_REG(gpio->base + OUT_VALUE_OFF + channel * sizeof(uint32_t),
              period * MCK_US_TO_TICKS);
    return 0;
}

int gpio_set_width(uint32_t channel, uint32_t width) {
    if (gpio == NULL || channel >= GPIO_PINS || channels[channel] == 0) {
        return -1;
    }
    printf("width = %u\n", width * MCK_US_TO_TICKS);
    WRITE_REG(gpio->base + OUT_VALUE_OFF + channel * sizeof(uint32_t) +
                  sizeof(uint32_t),
              width * MCK_US_TO_TICKS);
    return 0;
}
