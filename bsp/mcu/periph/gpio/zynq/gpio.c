#include "gpio.h"

#include <hw/inout.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/neutrino.h>

#define WRITE_REG(__r, __d) out32((__r), (__d))
#define READ_REG(r) in32(r)

int gpio_init(gpio_instance_t *i, uintptr_t phys_base) {
    if (ThreadCtl(_NTO_TCTL_IO, 0) == -1) {
        perror("ThreadCtl failed");
        return NULL;
    }
    i->base = mmap_device_io(IO_REG_MAP_SIZE, phys_base);
    if (i->base == NULL) {
        perror("mmap_device_io");
        return -1;
    }
    i->phys_base = phys_base;
    return 0;
}

int gpio_set_discrete_mode(gpio_instance_t *i, uint32_t channel) {
    WRITE_REG(i->base + OUT_SETUP_OFF + channel * sizeof(uint32_t),
              OUT_MUX_DISCRETE_MODE);
    return 0;
}

int gpio_set_output_value(gpio_instance_t *i, uint32_t channel,
                          uint32_t value) {
    WRITE_REG(i->base + OUT_VALUE_OFF + channel * sizeof(uint32_t), value);
    return 0;
}

int gpio_get_output_value(gpio_instance_t *i, uint32_t channel,
                          uint32_t *value) {
    *value = READ_REG(i->base + OUT_VALUE_OFF + channel * 4);
    return 0;
}
