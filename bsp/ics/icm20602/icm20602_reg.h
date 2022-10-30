#include "stm32_base.h"
#include "bit.h"
#include "icm20602.h"
#pragma once

#define type_t static const uint8_t

// Register addresses
type_t XG_OFFS_TC_H         = 0x04;
type_t XG_OFFS_TC_L         = 0x05;
type_t YG_OFFS_TC_H         = 0x07;
type_t YG_OFFS_TC_L         = 0x08;
type_t ZG_OFFS_TC_H         = 0x0A;
type_t ZG_OFFS_TC_L         = 0x0B;
type_t CONFIG               = 0x1A;
type_t GYRO_CONFIG          = 0x1B;
type_t ACCEL_CONFIG         = 0x1C;
type_t ACCEL_CONFIG2        = 0x1D;
type_t FIFO_EN              = 0x23;
type_t INT_PIN_CFG          = 0x37;
type_t INT_ENABLE           = 0x38;
type_t TEMP_OUT_H           = 0x41;
type_t TEMP_OUT_L           = 0x42;
type_t FIFO_WM_TH1          = 0x60;
type_t FIFO_WM_TH2          = 0x61;
type_t SIGNAL_PATH_RESET    = 0x68;
type_t USER_CTRL            = 0x6A;
type_t PWR_MGMT_1           = 0x6B;
type_t I2C_IF               = 0x70;
type_t FIFO_COUNTH          = 0x72;
type_t FIFO_COUNTL          = 0x73;
type_t FIFO_R_W             = 0x74;
type_t WHO_AM_I             = 0x75;
type_t XA_OFFSET_H          = 0x77;
type_t XA_OFFSET_L          = 0x78;
type_t YA_OFFSET_H          = 0x7A;
type_t YA_OFFSET_L          = 0x7B;
type_t ZA_OFFSET_H          = 0x7D;
type_t ZA_OFFSET_L          = 0x7E;

// CONFIG
type_t FIFO_MODE            = BIT6;
type_t DLPF_CFG_8KHZ        = BIT1 | BIT2;

// GYRO_CONFIG
type_t FS_SEL_250_DPS       = 0x00;
type_t FS_SEL_500_DPS       = BIT3;
type_t FS_SEL_1000_DPS      = BIT4;
type_t FS_SEL_2000_DPS      = BIT4 | BIT3;
type_t FCHOICE_B_8KHZ       = BIT1 | BIT0;

// ACCEL_CONFIG
type_t ACCEL_FS_SEL_2G      = 0x00;
type_t ACCEL_FS_SEL_4G      = BIT3;
type_t ACCEL_FS_SEL_8G      = BIT4;
type_t ACCEL_FS_SEL_16G     = BIT4 | BIT3;

// ACCEL_CONFIG2
type_t ACCEL_FCHOICE_B      = BIT3;
type_t A_DLPF_CFG_1KHZ      = 7;

// FIFO_EN
type_t GYRO_FIFO_EN         = BIT4;
type_t ACCEL_FIFO_EN        = BIT3;

// INT_PIN_CFG
type_t INT_LEVEL            = BIT7;
type_t INT_OPEN             = BIT6;
type_t LATCH_INT_EN         = BIT5;
type_t INT_RD_CLEAR         = BIT4;

// INT_ENABLE
type_t DATA_RDY_INT_EN      = BIT0;

// SIGNAL_PATH_RESET
type_t ACCEL_RST            = BIT1;
type_t TEMP_RST             = BIT0;

// USR_CTRL
type_t USR_CTRL_FIFO_EN     = BIT6;
type_t USR_CTRL_FIFO_RST    = BIT2;
type_t SIG_COND_RST         = BIT0;

// PWR_MGMT_1
type_t DEVICE_RESET         = BIT7;
type_t SLEEP                = BIT6;
type_t CLKSEL_0             = BIT0;

// I2C_IF_BIT
type_t I2C_IF_DIS           = BIT6;

// Register values
type_t WHOAMI               = 0x12;

// Temp settings
static const float TEMP_SENS     = 326.8f;
static const float TEMP_OFFSET   = 25.f;
static const float TEMP_SENS_MIN = -40.f;
static const float TEMP_SENS_MAX = 85.f;

// Register read/write flag
type_t READ = 0x80;
type_t WRITE = 0x00;

// FIFO layout
typedef struct {
    uint8_t ACCEL_XOUT_H;
    uint8_t ACCEL_XOUT_L;
    uint8_t ACCEL_YOUT_H;
    uint8_t ACCEL_YOUT_L;
    uint8_t ACCEL_ZOUT_H;
    uint8_t ACCEL_ZOUT_L;
    uint8_t TEMP_H;
    uint8_t TEMP_L;
    uint8_t GYRO_XOUT_H;
    uint8_t GYRO_XOUT_L;
    uint8_t GYRO_YOUT_H;
    uint8_t GYRO_YOUT_L;
    uint8_t GYRO_ZOUT_H;
    uint8_t GYRO_ZOUT_L;
} FIFO_t;

#define SIZE_REG_CFG 24

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

// ACCEL 420Hz -3db BW, 1kHz ODR rate
// GYRO 250Hz -3db BW, 1kHz ODR rate
static const reg_cfg_t reg_cfg[SIZE_REG_CFG] = {
    {CONFIG,        FIFO_MODE | BIT0, DLPF_CFG_8KHZ},
    {GYRO_CONFIG,   FS_SEL_2000_DPS, FCHOICE_B_8KHZ},
    {ACCEL_CONFIG,  ACCEL_FS_SEL_16G, 0},
    {ACCEL_CONFIG2, A_DLPF_CFG_1KHZ, ACCEL_FCHOICE_B},
    {FIFO_EN,       GYRO_FIFO_EN | ACCEL_FIFO_EN, 0},
    {INT_PIN_CFG,   LATCH_INT_EN | INT_RD_CLEAR, 0},
    {INT_ENABLE,    DATA_RDY_INT_EN, 0},
    {FIFO_WM_TH1,   0, 0},
    {FIFO_WM_TH2,   0, 0},
    {USER_CTRL,     USR_CTRL_FIFO_EN, 0},
    {PWR_MGMT_1,    CLKSEL_0, SLEEP},
    {I2C_IF,        I2C_IF_DIS, 0},
    {XG_OFFS_TC_H,  0, 0},
    {XG_OFFS_TC_L,  0, 0},
    {YG_OFFS_TC_H,  0, 0},
    {YG_OFFS_TC_L,  0, 0},
    {ZG_OFFS_TC_H,  0, 0},
    {ZG_OFFS_TC_L,  0, 0},
    {XA_OFFSET_H,   0, 0},
    {XA_OFFSET_L,   0, 0},
    {YA_OFFSET_H,   0, 0},
    {YA_OFFSET_L,   0, 0},
    {ZA_OFFSET_H,   0, 0},
    {ZA_OFFSET_L,   0, 0},
};

typedef struct {
    uint8_t CMD;
    uint8_t COUNTH;
    uint8_t COUNTL;
    FIFO_t buf[ICM20602_FIFO_SIZE / sizeof(FIFO_t) + 1];
} FIFOBuffer_t;

typedef struct {
    uint16_t bytes;
    uint16_t samples;
} FIFOParam_t;
