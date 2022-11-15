#ifndef SDIO_H
#define SDIO_H

#include <errno.h>

#include "core.h"
#include "gpio.h"
#include "hal.h"
#include "irq.h"
#include "os.h"

enum sdio_ex_state_t { SDIO_FREE = 0, SDIO_WRITE = 1, SDIO_READ = 2 };
enum sdio_cd_state_t { SDIO_CONNECTED = 0, SDIO_NOT_CONNECTED = 1 };

typedef struct {
    IRQn_Type id;
    HAL_SD_CardInfoTypeDef info;
    enum sdio_ex_state_t state;
    enum sdio_cd_state_t connected;
    SemaphoreHandle_t sem;
    SemaphoreHandle_t mutex;
} sdio_private_t;

typedef struct {
    SD_HandleTypeDef init;
    gpio_t *ck;
    gpio_t *cmd;
    gpio_t *d0;
    gpio_t *d1;
    gpio_t *d2;
    gpio_t *d3;
    gpio_t *cd;
    int timeout;
    int irq_priority;
    sdio_private_t p;
} sdio_t;

int sdio_init(sdio_t *cfg);
int sdio_read_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                     uint32_t num);
int sdio_write_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                      uint32_t num);
int sdio_check_status_with_timeout(sdio_t *cfg, uint32_t timeout);
int sdio_detect(sdio_t *cfg);
int sdio_get_info(sdio_t *cfg, HAL_SD_CardInfoTypeDef *info);
int sdio_get_status(sdio_t *cfg);

#endif  // SDIO_H
