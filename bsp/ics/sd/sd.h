#ifndef SD_H
#define SD_H

#include "core.h"
#include "hal.h"
#include "sdio.h"

int SDCARD_Init();
int SDCARD_Read(uint8_t *pdata, uint32_t address, uint32_t num);
int SDCARD_Write(uint8_t *pdata, uint32_t address, uint32_t num);
int SDCARD_GetInfo(HAL_SD_CardInfoTypeDef *info);
int SDCARD_GetStatus();

#endif  // SD_H
