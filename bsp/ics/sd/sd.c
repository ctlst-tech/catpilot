#include "sd.h"

static sdcard_t sdcard;
static sdcard_t *dev;

sdcard_t *sdcard_start(char *name, sdio_t *sdio) {
    if (sdio == NULL || name == NULL) {
        return NULL;
    }

    sdcard.sdio = sdio;
    dev = &sdcard;
    strncpy(sdcard.name, name, MAX_NAME_LEN - 1);

    return &sdcard;
}

int sdcard_init(void) {
    if (dev == NULL) {
        return -1;
    }
    return 0;
}

int sdcard_read(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = sdio_read_blocks(dev->sdio, pdata, address, num);
    return rv;
}

int sdcard_write(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = sdio_write_blocks(dev->sdio, pdata, address, num);
    return rv;
}

int sdcard_get_info(void) {
    return sdio_get_info(dev->sdio, &dev->info);
}

int sdcard_get_status(void) {
    return sdio_get_status(dev->sdio);
}

uint32_t sdcard_get_sector_count(void) {
    sdcard_get_info();
    return dev->info.LogBlockNbr;
}

uint16_t sdcard_get_sector_size(void) {
    sdcard_get_info();
    return dev->info.LogBlockSize;
}

uint32_t sdcard_get_block_size(void) {
    sdcard_get_info();
    return dev->info.LogBlockSize / 512;
}
