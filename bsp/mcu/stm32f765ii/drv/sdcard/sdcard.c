#include "sdcard.h"
#include "board.h"
#include "board_cfg.h"

static char *device = "SDMMC1";

typedef struct {
    sdio_cfg_t *sdio;
} sdcard_cfg_t;

static sdcard_cfg_t sdcard_cfg;

int SDCARD_Init() {
    int rv = 0;

    sdcard_cfg.sdio = &sdmmc1;

    return rv;
}

int SDCARD_Read(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = SDIO_ReadBlocks(sdcard_cfg.sdio, pdata, address, num);
    return rv;
}

int SDCARD_Write(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = SDIO_WriteBlocks(sdcard_cfg.sdio, pdata, address, num);
    return rv;
}

int SDCARD_GetInfo(HAL_SD_CardInfoTypeDef *info) {
    int rv;
    rv = SDIO_GetInfo(sdcard_cfg.sdio, info);
    return rv;
}

int SDCARD_GetStatus() {
    int rv;
    rv = SDIO_GetStatus(sdcard_cfg.sdio);
    return rv;
}
