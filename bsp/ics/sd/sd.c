#include "sd.h"

static char *device = "SDMMC1";

typedef struct {
    sdio_t *sdio;
} sdcard_cfg_t;

static sdcard_cfg_t *sdcard_cfg;

int SDCARD_Init(sdcard_cfg_t *sdcard) {
    int rv = 0;

    sdcard_cfg = sdcard;

    return rv;
}

int SDCARD_Read(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = sdio_read_blocks(sdcard_cfg->sdio, pdata, address, num);
    return rv;
}

int SDCARD_Write(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = sdio_write_blocks(sdcard_cfg->sdio, pdata, address, num);
    return rv;
}

int SDCARD_GetInfo(HAL_SD_CardInfoTypeDef *info) {
    int rv;
    rv = sdio_get_info(sdcard_cfg->sdio, info);
    return rv;
}

int SDCARD_GetStatus() {
    int rv;
    rv = sdio_get_status(sdcard_cfg->sdio);
    return rv;
}
