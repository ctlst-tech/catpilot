#include "stm32_base.h"
#include "stm32_lib.h"
#pragma once

#define type_t static const uint8_t

// Default settings
type_t ADDRESS      = 0x0E;
type_t DEVICE_ID    = 0x10;

// Register addresses
type_t WHO_AM_I     = 0x00;
type_t STAT1        = 0x02;
type_t DATAXL       = 0x03;
type_t DATAXH       = 0x04;
type_t DATAYL       = 0x05;
type_t DATAYH       = 0x06;
type_t DATAZL       = 0x07;
type_t DATAZH       = 0x08;
type_t STAT2        = 0x09;
type_t CNTL1        = 0x0A;
type_t CNTL2        = 0x0B;
type_t STR          = 0x0C;
type_t CNTL3        = 0x0D;
type_t TEMPL        = 0x1C;
type_t TEMPH        = 0x1D;
type_t TCCNTL       = 0x40;
type_t AVGCNTL      = 0x41;
type_t PDCNTL       = 0x42;
type_t XX_CROSS_L   = 0x9C;
type_t XX_CROSS_H   = 0x9D;
type_t XY_CROSS_L   = 0x9E;
type_t XY_CROSS_H   = 0x9F;
type_t XZ_CROSS_L   = 0xA0;
type_t XZ_CROSS_H   = 0xA1;
type_t YX_CROSS_L   = 0xA2;
type_t YX_CROSS_H   = 0xA3;
type_t YY_CROSS_L   = 0xA4;
type_t YY_CROSS_H   = 0xA5;
type_t YZ_CROSS_L   = 0xA6;
type_t YZ_CROSS_H   = 0xA7;
type_t ZX_CROSS_L   = 0xA8;
type_t ZX_CROSS_H   = 0xA9;
type_t ZY_CROSS_L   = 0xAA;
type_t ZY_CROSS_H   = 0xAB;
type_t ZZ_CROSS_L   = 0xAC;
type_t ZZ_CROSS_H   = 0xAD;

// Register read/write flag
type_t READ         = 0x01;
type_t WRITE        = 0x00;

// STAT
type_t DOR          = BIT1;
type_t DRDY         = BIT0;

// CTNL
type_t SINGLE_MEAS  = BIT0;

// CTNL2
type_t SRST         = BIT0;

// STR
type_t SELF_TEST    = BIT6;

// CTNL3
type_t Z_16BIT      = BIT6;
type_t Y_16BIT      = BIT5;
type_t X_16BIT      = BIT4;

// AVGCNTL
type_t Y_16TIMES_S  = BIT5;
type_t Y_16TIMES_C  = BIT4 | BIT3;
type_t XZ_16TIMES_S = BIT2;
type_t XZ_16TIMES_C = BIT1 | BIT0;

// PDCNTL
type_t PULSE_NORMAL = BIT7 | BIT6;

#define SIZE_REG_CFG 4

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

static const reg_cfg_t reg_cfg[SIZE_REG_CFG] = {
    {CNTL2,     0, 0},
    {CNTL3,     X_16BIT | Y_16BIT | Z_16BIT, 0},
    {AVGCNTL,   Y_16TIMES_S | XZ_16TIMES_S, Y_16TIMES_C | XZ_16TIMES_C},
    {PDCNTL,    PULSE_NORMAL, 0}
};
