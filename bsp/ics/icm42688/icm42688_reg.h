#ifndef ICM42688_REG_H
#define ICM42688_REG_H

#include "bit.h"

#define type_t static const uint8_t

// Bank 0 Register Map
type_t DEVICE_CONFIG         = 0x11;
type_t DRIVE_CONFIG         = 0x13;
type_t INT_CONFIG           = 0x14;
type_t FIFO_CONFIG         = 0x16;
type_t TEMP_DATA1          = 0x1D;
type_t TEMP_DATA0          = 0x1E;
type_t ACCEL_DATA_X1       = 0x1F;
type_t ACCEL_DATA_X0       = 0x20;
type_t ACCEL_DATA_Y1       = 0x21;
type_t ACCEL_DATA_Y0       = 0x22;
type_t ACCEL_DATA_Z1       = 0x23;
type_t ACCEL_DATA_Z0       = 0x24;
type_t GYRO_DATA_X1        = 0x25;
type_t GYRO_DATA_X0        = 0x26;
type_t GYRO_DATA_Y1        = 0x27;
type_t GYRO_DATA_Y0        = 0x28;
type_t GYRO_DATA_Z1        = 0x29;
type_t GYRO_DATA_Z0        = 0x2A;
type_t TMST_FSYNCH        = 0x2B;
type_t TMST_FSYNCL        = 0x2C;
type_t INT_STATUS         = 0x2D;
type_t FIFO_COUNTH        = 0x2E;
type_t FIFO_COUNTL        = 0x2F;
type_t FIFO_DATA          = 0x30;
type_t APEX_DATA0         = 0x31;
type_t APEX_DATA1         = 0x32;
type_t APEX_DATA2         = 0x33;
type_t APEX_DATA3         = 0x34;
type_t APEX_DATA4         = 0x35;
type_t APEX_DATA5         = 0x36;
type_t INT_STATUS2        = 0x37;
type_t INT_STATUS3        = 0x38;
type_t SIGNAL_PATH_RESET  = 0x4B;
type_t INTF_CONFIG0       = 0x4C;
type_t INTF_CONFIG1       = 0x4D;
type_t PWR_MGMT0          = 0x4E;
type_t GYRO_CONFIG0       = 0x4F;
type_t ACCEL_CONFIG0      = 0x50;
type_t GYRO_CONFIG1       = 0x51;
type_t GYRO_ACCEL_CONFIG0 = 0x52;
type_t ACCEL_CONFIG1      = 0x53;
type_t TMST_CONFIG       = 0x54;
type_t APEX_CONFIG0       = 0x56;
type_t SMD_CONFIG        = 0x57;
type_t FIFO_CONFIG1      = 0x5F;
type_t FIFO_CONFIG2      = 0x60;
type_t FIFO_CONFIG3      = 0x61;
type_t FSYNC_CONFIG      = 0x62;
type_t INT_CONFIG0       = 0x63;
type_t INT_CONFIG1       = 0x64;
type_t INT_SOURCE0       = 0x65;
type_t INT_SOURCE1       = 0x66;
type_t INT_SOURCE3       = 0x68;
type_t INT_SOURCE4       = 0x69;
type_t FIFO_LOST_PKT0    = 0x6C;
type_t FIFO_LOST_PKT1    = 0x6D;
type_t SELF_TEST_CONFIG  = 0x70;
type_t WHO_AM_I          = 0x75;
type_t REG_BANK_SEL      = 0x76;

// Register values
type_t WHOAMI            = 0x47;

// Power management
type_t PWR_MGMT_RESET    = BIT0;
type_t GYRO_MODE_OFF     = 0x00;
type_t GYRO_MODE_STANDBY = 0x01;
type_t GYRO_MODE_LN      = 0x03;
type_t ACCEL_MODE_OFF    = 0x00;
type_t ACCEL_MODE_LP     = 0x02;
type_t ACCEL_MODE_LN     = 0x03;

// GYRO_CONFIG0 bits
type_t GYRO_FS_SEL_2000DPS  = (0x00 << 5);
type_t GYRO_FS_SEL_1000DPS  = (0x01 << 5);
type_t GYRO_FS_SEL_500DPS   = (0x02 << 5);
type_t GYRO_FS_SEL_250DPS   = (0x03 << 5);
type_t GYRO_ODR_1KHZ        = (0x06);
type_t GYRO_ODR_500HZ       = (0x07);

// ACCEL_CONFIG0 bits
type_t ACCEL_FS_SEL_16G     = (0x00 << 5);
type_t ACCEL_FS_SEL_8G      = (0x01 << 5);
type_t ACCEL_FS_SEL_4G      = (0x02 << 5);
type_t ACCEL_FS_SEL_2G      = (0x03 << 5);
type_t ACCEL_ODR_1KHZ       = (0x06);
type_t ACCEL_ODR_500HZ      = (0x07);

// Temperature settings
static const float TEMP_SENS     = 132.48f;
static const float TEMP_OFFSET   = 25.0f;
static const float TEMP_SENS_MIN = -40.0f;
static const float TEMP_SENS_MAX = 85.0f;

// Register read/write flag
type_t READ = 0x80;
type_t WRITE = 0x00;

#define SIZE_REG_CFG 12

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

// Default configuration
static const reg_cfg_t reg_cfg[SIZE_REG_CFG] = {
    {DEVICE_CONFIG,    0x00, 0x01},                          // Disable soft reset
    {DRIVE_CONFIG,     0x00, 0x00},                         // Default drive settings
    {INT_CONFIG,       0x00, 0x00},                         // Default interrupt settings
    {FIFO_CONFIG,      0x01, 0x00},                         // Stream-to-FIFO mode
    {SIGNAL_PATH_RESET, 0x00, 0x00},                        // No resets
    {PWR_MGMT0,        GYRO_MODE_LN | ACCEL_MODE_LN, 0x00}, // Both sensors in low-noise mode
    {GYRO_CONFIG0,     GYRO_FS_SEL_2000DPS | GYRO_ODR_1KHZ, 0x00},
    {ACCEL_CONFIG0,    ACCEL_FS_SEL_16G | ACCEL_ODR_1KHZ, 0x00},
    {FIFO_CONFIG1,     0x03, 0x00},                         // Enable FIFO for both sensors
    {INT_SOURCE0,      0x08, 0x00},                         // Enable data ready interrupt
    {INT_CONFIG0,      0x00, 0x00},                         // Active-high, push-pull interrupts
    {INTF_CONFIG0,     0x00, 0x00},                         // Disable I2C interface
};

#endif  // ICM42688_REG_H
