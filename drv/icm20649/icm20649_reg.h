#include "stm32_base.h"
#include "bit.h"
#include "icm20649.h"
#pragma once

#define type_t static const uint8_t

// Register addresses
// BANK 0
type_t WHO_AM_I             = 0x00;
type_t USER_CTRL            = 0x03;
type_t LP_CONFIG            = 0x05;
type_t PWR_MGMT_1           = 0x06;
type_t INT_PIN_CFG          = 0x0F;
type_t INT_ENABLE_1         = 0x11;
type_t I2C_MST_STATUS       = 0x17;
type_t ACCEL_XOUT_H         = 0x2D;
type_t TEMP_OUT_H           = 0x39;
type_t TEMP_OUT_L           = 0x3A;
type_t FIFO_EN_2            = 0x67;
type_t FIFO_RST             = 0x68;
type_t FIFO_MODE            = 0x69;
type_t FIFO_COUNTH          = 0x70;
type_t FIFO_COUNTL          = 0x71;
type_t FIFO_R_W             = 0x72;
type_t FIFO_CFG             = 0x76;
type_t REG_BANK_SEL         = 0x7F;

// BANK 2
type_t GYRO_CONFIG_1        = 0x01;
type_t ACCEL_CONFIG         = 0x14;
type_t ACCEL_SMPLRT_DIV_1   = 0x10;
type_t ACCEL_SMPLRT_DIV_2   = 0x11;
type_t GYRO_SMPLRT_DIV      = 0x00;

// BANK 0 Registers
// LP_CONFIG
type_t GYRO_CYCLE           = BIT4;
type_t ACCEL_CYCLE          = BIT5;

// USER_CTRL
type_t DMP_EN               = BIT7;
type_t FIFO_EN              = BIT6;
type_t I2C_MST_EN           = BIT5;
type_t I2C_IF_DIS           = BIT4;
type_t SRAM_RST             = BIT2;

// PWR_MGMT_1
type_t DEVICE_RESET         = BIT7;
type_t SLEEP                = BIT6;
type_t LP_EN                = BIT5;
type_t CLKSEL_2             = BIT2;
type_t CLKSEL_1             = BIT1;
type_t CLKSEL_0             = BIT0;

// INT_PIN_CFG
type_t INT1_ACTL            = BIT7;

// INT ENABLE_1
type_t RAW_DATA_0_RDY_EN    = BIT0;

// FIFO_EN_2
type_t ACCEL_FIFO_EN        = BIT4;
type_t GYRO_Z_FIFO_EN       = BIT3;
type_t GYRO_Y_FIFO_EN       = BIT2;
type_t GYRO_X_FIFO_EN       = BIT1;
type_t TEMP_FIFO_EN         = BIT0;

// FIFO_RST
type_t FIFO_RESET           = BIT4 | BIT3 | BIT2 | BIT1 | BIT0;

// FIFO_MODE
type_t SNAPSHOT             = BIT0;

// FIFO_CFG
type_t FIFO_CFG_BIT         = BIT0;

type_t BANK_0               = 0;         
type_t BANK_1               = BIT4;      
type_t BANK_2               = BIT5;      
type_t BANK_3               = BIT5 | BIT4;

// BANK 2 Registers
// 5:3 GYRO_DLPFCFG[2:0]
type_t GYRO_DLPFCFG         = BIT5 | BIT4;

// 5:3 ACCEL_DLPFCFG[2:0]
type_t ACCEL_DLPFCFG        = BIT5 | BIT4 | BIT3;

// 2:1 GYRO_FS_SEL[1:0]
type_t GYRO_FS_SEL_500_DPS  = 0;           // ±250 dps
type_t GYRO_FS_SEL_1000_DPS = BIT1;        // ±500 dps
type_t GYRO_FS_SEL_2000_DPS = BIT2;        // ±1000 dps
type_t GYRO_FS_SEL_4000_DPS = BIT2 | BIT1; // ±2000 dps
type_t GYRO_FCHOICE         = BIT0;        // 0 – Bypass gyro DLPF

// 2:1 ACCEL_FS_SEL[1:0]
type_t ACCEL_FS_SEL_4G      = 0;           // 0b00: ±2g
type_t ACCEL_FS_SEL_8G      = BIT1;        // 0b01: ±4g
type_t ACCEL_FS_SEL_16G     = BIT2;        // 0b10: ±8g
type_t ACCEL_FS_SEL_30G     = BIT2 | BIT1; // 0b11: ±16g
type_t ACCEL_FCHOICE        = BIT0;        // 0: Bypass accel DLPF

// Data and parameters
type_t WHOAMI               = 0xE1; 

// Register read/write flag
type_t READ = 0x80;
type_t WRITE = 0x00;

// Temp settings
static const float TEMP_SENS     = 333.87f;
static const float TEMP_OFFSET   = 21.f;

// FIFO layout
#pragma pack(push,1)
typedef struct {
    uint8_t ACCEL_XOUT_H;
    uint8_t ACCEL_XOUT_L;
    uint8_t ACCEL_YOUT_H;
    uint8_t ACCEL_YOUT_L;
    uint8_t ACCEL_ZOUT_H;
    uint8_t ACCEL_ZOUT_L;
    uint8_t GYRO_XOUT_H;
    uint8_t GYRO_XOUT_L;
    uint8_t GYRO_YOUT_H;
    uint8_t GYRO_YOUT_L;
    uint8_t GYRO_ZOUT_H;
    uint8_t GYRO_ZOUT_L;
} FIFO_t;
#pragma pack(pop)

#define BANK_0_SIZE_REG_CFG 7
#define BANK_2_SIZE_REG_CFG 5

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

static const reg_cfg_t bank_0_reg_cfg[BANK_0_SIZE_REG_CFG] = {
    {USER_CTRL,     I2C_IF_DIS, DMP_EN | I2C_MST_EN | FIFO_EN},
    {PWR_MGMT_1,   CLKSEL_0, DEVICE_RESET | SLEEP | LP_EN},
    {INT_PIN_CFG,  INT1_ACTL, 0},
    {INT_ENABLE_1, RAW_DATA_0_RDY_EN, 0},
    {FIFO_EN_2,    0, ACCEL_FIFO_EN | GYRO_Z_FIFO_EN | GYRO_Y_FIFO_EN | GYRO_X_FIFO_EN | TEMP_FIFO_EN},
    {FIFO_MODE,    SNAPSHOT, 0},
};

// ACCEL 111.4 Hz -3dB BW, 1.125 kHz ODR rate
// GYRO 119.5 Hz -3dB BW, 1.125 kHz ODR rate
static const reg_cfg_t bank_2_reg_cfg[BANK_2_SIZE_REG_CFG] = {
    {GYRO_CONFIG_1,      GYRO_DLPFCFG | GYRO_FCHOICE, BIT3 | BIT1 | GYRO_FS_SEL_4000_DPS},
    {ACCEL_CONFIG,       ACCEL_FS_SEL_16G | ACCEL_DLPFCFG | ACCEL_FCHOICE, BIT1},
    {ACCEL_SMPLRT_DIV_1, 0, 0xFF},
    {ACCEL_SMPLRT_DIV_2, 0, 0xFF},
    {GYRO_SMPLRT_DIV,    0, 0xFF},
};

typedef struct {
    uint8_t CMD;
    uint8_t COUNTH;
    uint8_t COUNTL;
    FIFO_t buf[ICM20649_FIFO_SIZE / sizeof(FIFO_t) + 1];
} FIFOBuffer_t;

typedef struct {
    uint16_t bytes;
    uint16_t samples;
} FIFOParam_t;
