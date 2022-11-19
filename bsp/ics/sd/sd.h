#ifndef SD_H
#define SD_H

#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "hal.h"
#include "sdio.h"

typedef struct {
    char name[32];
    sdio_t *sdio;
} sdcard_t;

sdcard_t *sdcard_init(sdio_t *sdio);
int sdcard_read(sdcard_t *dev, uint8_t *pdata, uint32_t address, uint32_t num);
int sdcard_write(sdcard_t *dev, uint8_t *pdata, uint32_t address, uint32_t num);
int sdcard_get_info(sdcard_t *dev, HAL_SD_CardInfoTypeDef *info);
int sdcard_get_status(sdcard_t *dev);

#endif  // SD_H
