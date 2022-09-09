#include "stm32_base.h"
#include "bit.h"
#pragma once

#define type_t static const uint8_t

// Register addresses
// BANK 1
type_t WHO_AM_I             = 0x00;
type_t USER_CTRL            = 0x03;
type_t PWR_MGMT_1           = 0x06;
type_t INT_PIN_CFG          = 0x0F;
type_t INT_ENABLE_1         = 0x11;
type_t I2C_MST_STATUS       = 0x17;
type_t TEMP_OUT_H           = 0x39;
type_t TEMP_OUT_L           = 0x3A;
type_t EXT_SLV_SENS_DATA_00 = 0x3B;
type_t EXT_SLV_SENS_DATA_23 = 0x52;
type_t FIFO_EN_2            = 0x67;
type_t FIFO_RST             = 0x68;
type_t FIFO_MODE            = 0x69;
type_t FIFO_COUNTH          = 0x70;
type_t FIFO_COUNTL          = 0x71;
type_t FIFO_R_W             = 0x72;
type_t FIFO_CFG             = 0x76;
type_t REG_BANK_SEL         = 0x7F;

// BANK 2
type_t GYRO_SMPLRT_DIV      = 0x00;
type_t GYRO_CONFIG_1        = 0x01;
type_t ACCEL_SMPLRT_DIV_2   = 0x11;
type_t ACCEL_CONFIG         = 0x14;

// BANK 3
type_t I2C_MST_CTRL         = 0x01;
type_t I2C_MST_DELAY_CTRL   = 0x02;
type_t I2C_SLV0_ADDR        = 0x03;
type_t I2C_SLV0_REG         = 0x04;
type_t I2C_SLV0_CTRL        = 0x05;
type_t I2C_SLV0_DO          = 0x06;
type_t I2C_SLV4_CTRL        = 0x15;

// Internal MAG AK09916
type_t WIA2                 = 0x01; // Device ID
type_t ST1                  = 0x10; // Status 1
type_t HXL                  = 0x11;
type_t HXH                  = 0x12;
type_t HYL                  = 0x13;
type_t HYH                  = 0x14;
type_t HZL                  = 0x15;
type_t HZH                  = 0x16;
type_t ST2                  = 0x18; // Status 2
type_t CNTL2                = 0x31; // Control 2
type_t CNTL3                = 0x32; // Control 3

// BANK 1 Registers
// USER_CTRL
type_t DMP_EN               = BIT7;
type_t FIFO_EN              = BIT6;
type_t I2C_MST_EN           = BIT5;
type_t I2C_IF_DIS           = BIT4;
type_t DMP_RST              = BIT3;
type_t SRAM_RST             = BIT2;
type_t I2C_MST_RST          = BIT1;

// PWR_MGMT_1
type_t DEVICE_RESET         = BIT7;
type_t SLEEP                = BIT6;
type_t CLKSEL_2             = BIT2;
type_t CLKSEL_1             = BIT1;
type_t CLKSEL_0             = BIT0;

// INT_PIN_CFG
type_t INT1_ACTL            = BIT7;
type_t BYPASS_EN            = BIT1;

// INT ENABLE
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

type_t USER_BANK_0          = 0;         
type_t USER_BANK_1          = BIT4;      
type_t USER_BANK_2          = BIT5;      
type_t USER_BANK_3          = BIT5 | BIT4;

// BANK 2 Registers
// 5:3 GYRO_DLPFCFG[2:0]
type_t GYRO_DLPFCFG         = BIT5 | BIT4 | BIT3;

// 2:1 GYRO_FS_SEL[1:0]
type_t GYRO_FS_SEL_250_DPS  = 0;           // ±250 dps
type_t GYRO_FS_SEL_500_DPS  = BIT1;        // ±500 dps
type_t GYRO_FS_SEL_1000_DPS = BIT2;        // ±1000 dps
type_t GYRO_FS_SEL_2000_DPS = BIT2 | BIT1; // ±2000 dps
type_t GYRO_FCHOICE         = BIT0;        // 0 – Bypass gyro DLPF

// 5:3 ACCEL_DLPFCFG[2:0]
type_t ACCEL_DLPFCFG        = BIT5 | BIT4 | BIT3;

// 2:1 ACCEL_FS_SEL[1:0]
type_t ACCEL_FS_SEL_2G      = 0;           // 0b00: ±2g
type_t ACCEL_FS_SEL_4G      = BIT1;        // 0b01: ±4g
type_t ACCEL_FS_SEL_8G      = BIT2;        // 0b10: ±8g
type_t ACCEL_FS_SEL_16G     = BIT2 | BIT1; // 0b11: ±16g
type_t ACCEL_FCHOICE        = BIT0;        // 0: Bypass accel DLPF

// BANK 3 Registers
// I2C_MST_CTRL
type_t I2C_MST_P_NSR        = BIT4;

// I2C_MST_DELAY_CTRL
type_t I2C_MST_CLK_400_kHz  = 7;

// I2C_MST_DELAY_CTRL
type_t I2C_SLVX_DLY_EN      = BIT4 | BIT3 | BIT2 | BIT1 | BIT0;

// I2C_SLV0_ADDR
type_t I2C_SLV0_RNW         = BIT7;
type_t I2C_ID_0             = BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0;

// I2C_SLV0_CTRL
type_t I2C_SLV0_EN          = BIT7;
type_t I2C_SLV0_BYTE_SW     = BIT6;
type_t I2C_SLV0_REG_DIS     = BIT5;
type_t I2C_SLV0_LENG        = BIT3 | BIT2 | BIT1 | BIT0;

// I2C_SLV4_CTRL
type_t I2C_MST_DLY          = BIT4 | BIT3 | BIT2 | BIT1 | BIT0;

// AK09916 Registers values
// ST1
type_t DOR                  = BIT1; // Data overrun
type_t DRDY                 = BIT0; // Data is ready

// ST2
type_t HOFL                 = BIT3; // Magnetic overflow

// CNTL2
type_t MODE1                = BIT1;        // “00010”: Continuous measurement mode 1 (10Hz)
type_t MODE2                = BIT2;        // “00100”: Continuous measurement mode 2 (20Hz)
type_t MODE3                = BIT2 | BIT1; // “00110”: Continuous measurement mode 3 (50Hz)
type_t MODE4                = BIT3;        // “01000”: Continuous measurement mode 4 (100Hz)

// CNTL3
type_t SRST                 = BIT0;

// Data and parameters
type_t WHOAMI               = 0xEA; 

#define FIFO_SIZE 512

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
    uint8_t GYRO_XOUT_H;
    uint8_t GYRO_XOUT_L;
    uint8_t GYRO_YOUT_H;
    uint8_t GYRO_YOUT_L;
    uint8_t GYRO_ZOUT_H;
    uint8_t GYRO_ZOUT_L;
} FIFO_t;

typedef struct {
    uint8_t ST1;
    uint8_t HXL;
    uint8_t HXH;
    uint8_t HYL;
    uint8_t HYH;
    uint8_t HZL;
    uint8_t HZH;
    uint8_t TMPS;
    uint8_t ST2;
} MAG_t;

#define BANK1_SIZE_REG_CFG 6
#define BANK2_SIZE_REG_CFG 2
#define BANK3_SIZE_REG_CFG 3

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

static const reg_cfg_t bank_1_reg_cfg[BANK1_SIZE_REG_CFG] = {
    {USER_CTRL,    FIFO_EN | I2C_MST_EN | I2C_IF_DIS, DMP_EN},
    {PWR_MGMT_1,   CLKSEL_0, DEVICE_RESET | SLEEP},
    {INT_PIN_CFG,  INT1_ACTL, 0},
    {INT_ENABLE_1, RAW_DATA_0_RDY_EN, 0},
    {FIFO_EN_2,    ACCEL_FIFO_EN | GYRO_Z_FIFO_EN | GYRO_Y_FIFO_EN | GYRO_X_FIFO_EN, TEMP_FIFO_EN},
    {FIFO_MODE,    SNAPSHOT, 0},
};

static const reg_cfg_t bank_2_reg_cfg[BANK2_SIZE_REG_CFG] = {
    {GYRO_CONFIG_1, GYRO_FS_SEL_2000_DPS, GYRO_FCHOICE},
    {ACCEL_CONFIG,  ACCEL_FS_SEL_16G, ACCEL_FCHOICE},
};

static const reg_cfg_t bank_3_reg_cfg[BANK3_SIZE_REG_CFG] = {
    {I2C_MST_CTRL,       0, 0},
    {I2C_MST_DELAY_CTRL, 0, 0},
    {I2C_SLV4_CTRL,      0, 0},
};

typedef struct {
    uint8_t COUNTH;
    uint8_t COUNTL;
    FIFO_t buf[FIFO_SIZE / sizeof(FIFO_t)];
} FIFOBuffer_t;

typedef struct {
    uint16_t bytes;
    uint8_t samples;
} FIFOParam_t;
