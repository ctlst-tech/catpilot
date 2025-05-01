#ifndef AK09918_REG_H
#define AK09918_REG_H

#include "ak09918.h"

#define type_t static const uint8_t

// Default settings
type_t ADDRESS      = 0x0C;  // I2C address
type_t DEVICE_ID    = 0x09;  // Device ID for AK09918

// Register addresses
type_t WIA1         = 0x00;  // Device ID 1
type_t WIA2         = 0x01;  // Device ID 2
type_t RSV1         = 0x02;  // Reserved 1
type_t RSV2         = 0x03;  // Reserved 2
type_t ST1          = 0x10;  // Status 1
type_t HXL          = 0x11;  // X-axis data lower byte
type_t HXH          = 0x12;  // X-axis data higher byte
type_t HYL          = 0x13;  // Y-axis data lower byte
type_t HYH          = 0x14;  // Y-axis data higher byte
type_t HZL          = 0x15;  // Z-axis data lower byte
type_t HZH          = 0x16;  // Z-axis data higher byte
type_t TMPS         = 0x17;  // Temperature sensor data
type_t ST2          = 0x18;  // Status 2
type_t CNTL1        = 0x30;  // Control 1
type_t CNTL2        = 0x31;  // Control 2
type_t CNTL3        = 0x32;  // Control 3

// Register read/write flag
type_t READ         = 0x80;
type_t WRITE        = 0x00;

// Status 1 register bits
type_t DOR          = BIT1;  // Data overrun
type_t DRDY         = BIT0;  // Data ready

// Status 2 register bits
type_t HOFL         = BIT3;  // Magnetic sensor overflow

// Control 1 register modes
type_t MODE_POWER_DOWN     = 0x00;  // Power-down mode
type_t MODE_SINGLE_MEAS    = 0x01;  // Single measurement mode
type_t MODE_CONT_MEAS_1    = 0x02;  // Continuous measurement mode 1 (10Hz)
type_t MODE_CONT_MEAS_2    = 0x04;  // Continuous measurement mode 2 (20Hz)
type_t MODE_CONT_MEAS_3    = 0x06;  // Continuous measurement mode 3 (50Hz)
type_t MODE_CONT_MEAS_4    = 0x08;  // Continuous measurement mode 4 (100Hz)
type_t MODE_SELF_TEST      = 0x10;  // Self-test mode

// Control 2 register bits
type_t SRST         = BIT0;  // Soft reset

// Control 3 register bits
type_t LOW_POWER    = BIT0;  // Low power mode

#define SIZE_REG_CFG 3

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

// Default configuration
static const reg_cfg_t reg_cfg[SIZE_REG_CFG] = {
    {CNTL2, 0, SRST},                    // Clear soft reset
    {CNTL1, MODE_CONT_MEAS_4, 0},        // Set continuous measurement mode 4 (100Hz)
    {CNTL3, 0, LOW_POWER}                // Disable low power mode
};

// Magnetometer sensitivity (μT/LSB)
static const float MAG_SENS = 0.15f;  // AK09918 sensitivity is 0.15 μT/LSB

#endif  // AK09918_REG_H 