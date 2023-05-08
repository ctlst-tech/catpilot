#include "ad7606b.h"

#include <hw/inout.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#define WRITE_REG(__r, __d) out32((__r), (__d))
#define READ_REG(r) in32(r)

int adc_ad7606b_init(ad7606b_instance_t *i, uintptr_t phys_base) {
    i->base = mmap_device_io(AD7606B_REG_MAP_SIZE, phys_base);
    if (i->base == NULL) {
        perror("mmap_device_io failed\n");
        return -1;
    }
    i->phys_base = phys_base;
    return 0;
}

int adc_ad7606b_set_disc_period(ad7606b_instance_t *i, uint32_t period_us,
                                uint32_t mck) {
    uint32_t per_reg = mck / (1000000 / period_us);
    WRITE_REG(i->base + AD7606B_DISC_PERIOD_OFF, per_reg);
    return 0;
}

int adc_ad7606b_start(ad7606b_instance_t *i) {
    WRITE_REG(i->base + AD7606B_CONTROL_OFF, AD7606B_CONTROL_ENABLE);
    return 0;
}

int adc_ad7606b_stop(ad7606b_instance_t *i) {
    WRITE_REG(i->base + AD7606B_CONTROL_OFF, AD7606B_CONTROL_DISABLE);
    return 0;
}
int adc_ad7606b_get_status(ad7606b_instance_t *i, uint32_t *status) {
    *status = READ_REG(i->base + AD7606B_STATUS_OFF);
    return 0;
}

int adc_ad7606b_get_raw_adc_value(ad7606b_instance_t *i, uint8_t adc,
                                  uint8_t channel, int16_t *value) {
    uint32_t offset =
        adc ? AD7606B_VIN_12_CHN_ADC_1_OFF : AD7606B_VIN_12_CHN_ADC_0_OFF;
    uint32_t reg_value;
    uint32_t full_offset;
    full_offset = offset + (channel / 2) * 4;
    reg_value = READ_REG(i->base + full_offset);
    // printf("Read reg addr 0x%x 0x%x\n",i->phys_base + full_offset,
    // reg_value);
    if (channel % 2) {
        *value = reg_value >> 16;
    } else {
        *value = reg_value & 0xFFFF;
    }
    return 0;
}
void adc_ad7606b_print_raw_adc_values(ad7606b_instance_t *i) {
    size_t j, k;
    for (j = 0; j < 2; j++) {
        printf("ADC %d:\n", j + 1);
        for (k = 0; k < 8; k++) {
            int16_t value;
            adc_ad7606b_get_raw_adc_value(i, j, k, &value);
            printf("channel #%d: %x %2.3f\t", (k + 1), value,
                   adc_convert_value(value, 0));
            if ((k == 3) || (k == 7)) {
                printf("\n");
            }
        }
    }
}
void adc_ad7606b_print_all_adc_values(ad7606b_instance_t *i) {
    size_t j, l, k;
    uint32_t reg_value;
    uint32_t chn_offset;
    int16_t value;
    int16_t value_0, value_1;
    for (j = 0; j < 2; j++) {
        printf("ADC %d:\n", j + 1);
        for (l = 0; l < 3; l++) {
            chn_offset = AD7606B_VIN_1_CHN_MUX_01_ADC_0_ADDR + 4 * 8 * l;
            printf("Adc Chn %d\n", l + 1);
            for (k = 0; k < 8; k++) {
                reg_value = READ_REG(i->base + chn_offset + k * 4);
                value_0 = reg_value & 0xFFFF;
                value_1 = reg_value >> 16;
                printf("mux %d v: %2.3f\t", 2 * k,
                       adc_convert_value(value_0, 0));
                printf("mux %d v: %2.3f\t", 2 * k + 1,
                       adc_convert_value(value_1, 0));
            }
            printf("\n");
        }
        for (l = 3; l < 8; l++) {
            adc_ad7606b_get_raw_adc_value(i, j, l, &value);
            printf("channel #%d: %x %2.3f\t", (l + 1), value,
                   adc_convert_value(value, 0));
        }
        printf("\n");
    }
}

int adc_ad7606b_get_adc_value(ad7606b_instance_t *i, uint8_t adc,
                              uint8_t channel, uint8_t mux, int16_t *value) {
    if (channel <= 2) {
        uint32_t chn_offset =
            AD7606B_VIN_1_CHN_MUX_01_ADC_0_ADDR + 4 * 8 * channel;
        uint32_t reg_value = READ_REG(i->base + chn_offset + mux * 4);
        *value = (mux % 2 == 0 ? reg_value & 0xFFFF : reg_value >> 16);
    } else {
        adc_ad7606b_get_raw_adc_value(i, adc, channel, value);
    }
    return 0;
}

int adc_get_error_counter(ad7606b_instance_t *i, uint32_t *err_counter) {
    *err_counter = READ_REG(i->base + AD7606B_BUSY_ERROR_COUNTER_OFF);

    return 0;
}

int adc_ad7606b_set_range(ad7606b_instance_t *i, uint32_t channel, uint32_t range) {
    uint16_t reg = READ_REG(i->base + AD7606B_CHN_SETUP_OFF);
    reg &= ~(3 << (channel * 2));
    if (range == RANGE_2V5) {
        reg |= RANGE_2V5 << (channel * 2);
        WRITE_REG(i->base + AD7606B_CHN_SETUP_OFF, reg);
    } else if (range == RANGE_5V) {
        reg |= RANGE_5V << (channel * 2);
        WRITE_REG(i->base + AD7606B_CHN_SETUP_OFF, reg);
    } else if (range == RANGE_10V) {
        reg |= RANGE_10V << (channel * 2);
        WRITE_REG(i->base + AD7606B_CHN_SETUP_OFF, reg);
    } else {
        return -1;
    }
    return 0;
}

float adc_convert_value(int16_t value, int8_t range) {
    float range_f;

    if (range == RANGE_2V5) {
        range_f = 2.5;
    } else if (range == RANGE_5V) {
        range_f = 5.0;
    } else if (range == RANGE_10V) {
        range_f = 10.0;
    } else {
        range_f = 100.0;
    }

    return (value * range_f) / 32768;
}
