#pragma once
#include <stdint.h>

#define BIT0 (1 << 0)
#define BIT1 (1 << 1)
#define BIT2 (1 << 2)
#define BIT3 (1 << 3)
#define BIT4 (1 << 4)
#define BIT5 (1 << 5)
#define BIT6 (1 << 6)
#define BIT7 (1 << 7)

uint16_t msblsb16(uint8_t msb, uint8_t lsb);
uint32_t GetNumFromMask(uint32_t reg);
uint32_t GetMaskFromNum(uint32_t num);
