#include "stm32_base.h"
#include "stm32_periph.h"


int SDCARD_Init();
int SDCARD_Read(uint8_t *pdata, uint32_t address, uint32_t num);
int SDCARD_Write(uint8_t *pdata, uint32_t address, uint32_t num);
int SDCARD_GetInfo(HAL_SD_CardInfoTypeDef *info);
int SDCARD_GetStatus();