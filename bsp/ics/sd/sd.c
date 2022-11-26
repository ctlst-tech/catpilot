#include "sd.h"

sdcard_t *sdcard_init(char *name, sdio_t *sdio) {
    sdcard_t *dev = calloc(sizeof(sdcard_t), 1);

    if(dev != NULL) {
        dev->sdio = sdio;
        strncpy(dev->name, name, MAX_NAME_LEN);
    }

    return dev;
}

int sdcard_read(sdcard_t *dev, uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = sdio_read_blocks(dev->sdio, pdata, address, num);
    return rv;
}

int sdcard_write(sdcard_t *dev, uint8_t *pdata, uint32_t address,
                 uint32_t num) {
    int rv;
    rv = sdio_write_blocks(dev->sdio, pdata, address, num);
    return rv;
}

int sdcard_get_info(sdcard_t *dev, HAL_SD_CardInfoTypeDef *info) {
    int rv;
    rv = sdio_get_info(dev->sdio, info);
    return rv;
}

int sdcard_get_status(sdcard_t *dev) {
    int rv;
    rv = sdio_get_status(dev->sdio);
    return rv;
}
