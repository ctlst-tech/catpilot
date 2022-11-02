#ifndef SDIO_H
#define SDIO_H

#include "core.h"
#include "dma.h"
#include "gpio.h"
#include "hal.h"

enum sdio_ex_state_t { SDIO_FREE, SDIO_WRITE, SDIO_READ };
enum sdio_cd_state_t { SDIO_CONNECTED, SDIO_NOT_CONNECTED };

typedef struct {
    gpio_t *ck;
    gpio_t *cmd;
    gpio_t *d0;
    gpio_t *d1;
    gpio_t *d2;
    gpio_t *d3;
    gpio_t *cd;
    SD_HandleTypeDef init;
    HAL_SD_CardInfoTypeDef info;
    SemaphoreHandle_t semaphore;
    SemaphoreHandle_t mutex;
    enum sdio_ex_state_t state;
    enum sdio_cd_state_t connected;
    int timeout;
    IRQn_Type irq;
    int irq_priority;
} sdio_t;

int sdio_init(sdio_t *cfg);
int sdio_clock_enable(sdio_t *cfg);
int sdio_enable_irq(sdio_t *cfg);
int sdio_disable_irq(sdio_t *cfg);

int sdio_read_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                     uint32_t num);
int sdio_write_blocks(sdio_t *cfg, uint8_t *pdata, uint32_t address,
                      uint32_t num);
int sdio_check_status_with_timeout(sdio_t *cfg, uint32_t timeout);
int sdio_detect(sdio_t *cfg);
int sdio_get_info(sdio_t *cfg, HAL_SD_CardInfoTypeDef *info);
int sdio_get_status(sdio_t *cfg);

// int SDIO_DMA_Handler(sdio_t *cfg);
int sdio_it_handler(sdio_t *cfg);
int sdio_tx_complete(sdio_t *cfg);
int sdio_rx_complete(sdio_t *cfg);

#endif  // SDIO_H
