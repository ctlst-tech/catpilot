#ifndef SD_H
#define SD_H

#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "hal.h"
#include "sdio.h"

typedef struct {
    char name[MAX_NAME_LEN];
    sdio_t *sdio;
    HAL_SD_CardInfoTypeDef info;
    SemaphoreHandle_t mutex;
} sdcard_t;

sdcard_t *sdcard_start(char *name, sdio_t *sdio);
int sdcard_init(void);
int sdcard_read(uint8_t *pdata, uint32_t address, uint32_t num);
int sdcard_write(uint8_t *pdata, uint32_t address, uint32_t num);
int sdcard_get_status(void);
uint32_t sdcard_get_sector_count(void);
uint16_t sdcard_get_sector_size(void);
uint32_t sdcard_get_block_size(void);

#endif  // SD_H
