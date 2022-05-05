#include "stm32_base.h"
#include "bit.h"
#pragma once

#define type_t static const uint8_t

type_t ACCEL_MODE = 0;
type_t GYRO_MODE = 1;
type_t BGW_RESET_VALUE = 0xB6;

type_t READ = 0x80;

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

// BMI055 Accelerometer

type_t ACCEL_CHIP_ID_VALUE   = 0xFA;

type_t ACCEL_BGW_CHIP_ID   = 0x00;
type_t ACCEL_ACCD_TEMP     = 0x08;
type_t ACCEL_INT_STATUS_1  = 0x0A;
type_t ACCEL_FIFO_STATUS   = 0x0E;
type_t ACCEL_PMU_RANGE     = 0x0F;
type_t ACCEL_ACCD_HBW      = 0x13;
type_t ACCEL_BGW_SOFTRESET = 0x14;
type_t ACCEL_INT_EN_1      = 0x17;
type_t ACCEL_INT_MAP_1     = 0x1A;
type_t ACCEL_INT_OUT_CTRL  = 0x20;
type_t ACCEL_FIFO_CONFIG_0 = 0x30;
type_t ACCEL_FIFO_CONFIG_1 = 0x3E;
type_t ACCEL_FIFO_DATA     = 0x3F;

// INT_STATUS_1
type_t accel_data_int      = BIT7;
type_t accel_fifo_wm_int   = BIT6;
type_t accel_fifo_full_int = BIT5;

// FIFO_STATUS
type_t accel_fifo_overrun       = BIT7;
type_t accel_fifo_frame_counter = BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0;

// ACCD_HBW
type_t accel_data_high_bw = BIT7;

// PMU_RANGE
    // range<3:0>
type_t accel_range_16g_set   = BIT3 | BIT2;
type_t accel_range_16g_clear = BIT1 | BIT0;
type_t accel_range_8g_set    = BIT3;
type_t accel_range_8g_clear  = BIT2 | BIT1 | BIT0;
type_t accel_range_4g_set    = BIT2 | BIT0;
type_t accel_range_4g_clear  = BIT3 | BIT1;
type_t accel_range_2g_set    = BIT1 | BIT0;
type_t accel_range_2g_clear  = BIT3 | BIT2;

// INT_EN_1
type_t accel_int_fwm_en   = BIT6;
type_t accel_int_ffull_en = BIT5;
type_t accel_data_en      = BIT4;

// INT_MAP_1
type_t accel_int2_data  = BIT7;
type_t accel_int2_fwm   = BIT6;
type_t accel_int2_ffull = BIT5;

type_t accel_int1_ffull = BIT2;
type_t accel_int1_fwm   = BIT1;
type_t accel_int1_data  = BIT0;

// INT_OUT_CTRL
type_t accel_int1_od  = BIT1;
type_t accel_int1_lvl = BIT0;

// FIFO_CONFIG_1
type_t accel_fifo_mode = BIT6;

typedef struct {
    uint8_t ACCD_X_LSB;
    uint8_t ACCD_X_MSB;
    uint8_t ACCD_Y_LSB;
    uint8_t ACCD_Y_MSB;
    uint8_t ACCD_Z_LSB;
    uint8_t ACCD_Z_MSB;
} accel_fifo_data_t;

type_t ACCEL_FIFO_FRAMES = 32;
size_t ACCEL_FIFO_BYTES = sizeof(accel_fifo_data_t) * ACCEL_FIFO_FRAMES;

#define ACCEL_SIZE_REG_CFG 7

static const reg_cfg_t accel_reg_cfg[ACCEL_SIZE_REG_CFG] = {
    {ACCEL_PMU_RANGE,     accel_range_16g_set, accel_range_16g_clear},
    {ACCEL_ACCD_HBW,      accel_data_high_bw, 0},
    {ACCEL_INT_EN_1,      accel_int_fwm_en, 0},
    {ACCEL_INT_MAP_1,     accel_int1_fwm, 0},
    {ACCEL_INT_OUT_CTRL,  accel_int1_od | accel_int1_lvl, 0},
    {ACCEL_FIFO_CONFIG_0, 0, 0},
    {ACCEL_FIFO_CONFIG_1, accel_fifo_mode, 0},
};

// BMI055 Gyroscope

type_t GYRO_CHIP_ID_VALUE  = 0x0F;

type_t GYRO_CHIP_ID        = 0x00;
type_t GYRO_FIFO_STATUS    = 0x0E;
type_t GYRO_RANGE          = 0x0F;
type_t GYRO_RATE_HBW       = 0x13;
type_t GYRO_BGW_SOFTRESET  = 0x14;
type_t GYRO_INT_EN_0       = 0x15;
type_t GYRO_INT_EN_1       = 0x16;
type_t GYRO_INT_MAP_1      = 0x18;
type_t GYRO_FIFO_WM_ENABLE = 0x1E;
type_t GYRO_FIFO_CONFIG_0  = 0x3D;
type_t GYRO_FIFO_CONFIG_1  = 0x3E;
type_t GYRO_FIFO_DATA      = 0x3F;

// FIFO_STATUS
type_t gyro_fifo_overrun       = BIT7;
type_t gyro_fifo_frame_counter = BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0;

// RANGE
type_t gyro_range_2000_dps = 0x00;
type_t gyro_range_1000_dps = 0x01;
type_t gyro_range_500_dps  = 0x02;
type_t gyro_range_250_dps  = 0x04;
type_t gyro_range_125_dps  = 0x05;

// RATE_HBW
type_t gyro_data_high_bw = BIT7;

// INT_EN_0
type_t gyro_data_en = BIT7;
type_t gyro_fifo_en = BIT6;

// INT_EN_1
type_t gyro_int1_od  = BIT1;
type_t gyro_int1_lvl = BIT0;

// INT_MAP_1
type_t gyro_int1_fifo = BIT2;
type_t gyro_int1_data = BIT0;

// FIFO_WM_ENABLE
type_t gyro_fifo_wm_enable  = BIT7;

// FIFO_CONFIG_0
type_t gyro_tag = BIT7;

// FIFO_CONFIG_1
type_t gyro_fifo_mode = BIT6;

typedef struct {
    uint8_t RATE_X_LSB;
    uint8_t RATE_X_MSB;
    uint8_t RATE_Y_LSB;
    uint8_t RATE_Y_MSB;
    uint8_t RATE_Z_LSB;
    uint8_t RATE_Z_MSB;
} gyro_fifo_data_t;

type_t GYRO_FIFO_FRAMES = 100;
size_t GYRO_SIZE = sizeof(gyro_fifo_data_t) * GYRO_FIFO_FRAMES;

#define GYRO_SIZE_REG_CFG 8

static const reg_cfg_t gyro_reg_cfg[GYRO_SIZE_REG_CFG] = {
    {GYRO_RANGE,          gyro_range_2000_dps, 0},
    {GYRO_RATE_HBW,       gyro_data_high_bw, 0},
    {GYRO_INT_EN_0,       gyro_fifo_en, 0},
    {GYRO_INT_EN_1,       0, gyro_int1_od | gyro_int1_lvl},
    {GYRO_INT_MAP_1,      gyro_int1_fifo, 0},
    {GYRO_FIFO_WM_ENABLE, gyro_fifo_wm_enable, 0},
    {GYRO_FIFO_CONFIG_0,  0, gyro_tag},
    {GYRO_FIFO_CONFIG_1,  gyro_fifo_mode, 0},
};
