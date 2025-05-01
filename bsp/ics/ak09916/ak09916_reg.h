#ifndef AK09916_REG_H
#define AK09916_REG_H

#include "ak09916.h"

#define type_t static const uint8_t

// Default settings
type_t ADDRESS      = 0x0C;  // I2C address
type_t DEVICE_ID    = 0x09;  // Device ID for AK09916

// Register addresses
type_t WIA1         = 0x00;  // Company ID
type_t WIA2         = 0x01;  // Device ID
type_t ST1          = 0x10;  // Status 1
type_t HXL          = 0x11;  // X-axis data lower byte
type_t HXH          = 0x12;  // X-axis data higher byte
type_t HYL          = 0x13;  // Y-axis data lower byte
type_t HYH          = 0x14;  // Y-axis data higher byte
type_t HZL          = 0x15;  // Z-axis data lower byte
type_t HZH          = 0x16;  // Z-axis data higher byte
type_t TMPS         = 0x17;  // Temperature sensor data
type_t ST2          = 0x18;  // Status 2
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

// Control 2 register modes
type_t MODE_POWER_DOWN = 0x00;  // Power-down mode
type_t MODE1           = BIT1;   // Continuous measurement mode 1 (10Hz)
type_t MODE2           = BIT2;   // Continuous measurement mode 2 (20Hz)
type_t MODE3           = BIT2 | BIT1;  // Continuous measurement mode 3 (50Hz)
type_t MODE4           = BIT3;   // Continuous measurement mode 4 (100Hz)

// Control 3 register bits
type_t SRST         = BIT0;  // Soft reset

#define SIZE_REG_CFG 2

typedef struct {
    uint8_t reg;
    uint8_t setbits;
    uint8_t clearbits;
} reg_cfg_t;

// Default configuration
static const reg_cfg_t reg_cfg[SIZE_REG_CFG] = {
    {CNTL2, MODE4, 0},          // Set continuous measurement mode 4 (100Hz)
    {CNTL3, 0, SRST}           // Clear soft reset
};

// Magnetometer sensitivity (μT/LSB)
static const float MAG_SENS = 0.15f;  // AK09916 sensitivity is 0.15 μT/LSB

// Temperature sensor sensitivity
static const float TEMP_SCALE = 0.5f;     // 0.5°C per LSB
static const float TEMP_OFFSET = -8.0f;   // Temperature offset in °C

#endif  // AK09916_REG_H 