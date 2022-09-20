#include "stm32_base.h"
#include "bit.h"
#include "ms5611.h"
#pragma once

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
type_t CMD_MS5611_PROM                  = 0xA0;

typedef struct {
    uint16_t C1;
    uint16_t C2;
    uint16_t C3;
    uint16_t C4;
    uint16_t C5;
    uint16_t C6;
} ms5611_calib_t;

typedef struct {
    uint32_t D1;
    uint32_t D2;
} ms5611_data_t;

typedef struct {
    double T;
    double P;
} ms5611_meas_t;
