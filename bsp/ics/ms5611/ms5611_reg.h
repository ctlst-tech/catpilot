#ifndef MS5611_REG_H
#define MS5611_REG_H

#include "ms5611.h"

#define type_t static const uint8_t

type_t CMD_MS5611_RESET                 = 0x1E;
type_t CMD_MS5611_CONVERT_D1_OSR256     = 0x40;
type_t CMD_MS5611_CONVERT_D1_OSR512     = 0x42;
type_t CMD_MS5611_CONVERT_D1_OSR1024    = 0x44;
type_t CMD_MS5611_CONVERT_D1_OSR2048    = 0x46;
type_t CMD_MS5611_CONVERT_D1_OSR4096    = 0x48;
type_t CMD_MS5611_CONVERT_D2_OSR256     = 0x50;
type_t CMD_MS5611_CONVERT_D2_OSR512     = 0x52;
type_t CMD_MS5611_CONVERT_D2_OSR1024    = 0x54;
type_t CMD_MS5611_CONVERT_D2_OSR2048    = 0x56;
type_t CMD_MS5611_CONVERT_D2_OSR4096    = 0x58;
type_t CMD_MS5611_READ_ADC              = 0x00;
type_t CMD_MS5611_PROM_ADDR             = 0xA0;

uint16_t crc4(const uint16_t *data) {
    uint16_t n_rem = 0;
    uint8_t n_bit;

    for (uint8_t cnt = 0; cnt < 16; cnt++) {
        /* uneven bytes */
        if (cnt & 1) {
            n_rem ^= (uint8_t)((data[cnt >> 1]) & 0x00FF);
        } else {
            n_rem ^= (uint8_t)(data[cnt >> 1] >> 8);
        }

        for (n_bit = 8; n_bit > 0; n_bit--) {
            if (n_rem & 0x8000) {
                n_rem = (n_rem << 1) ^ 0x3000;
            } else {
                n_rem = (n_rem << 1);
            }
        }
    }

    return (n_rem >> 12) & 0xF;
}

#endif  // MS5611_REG_H
