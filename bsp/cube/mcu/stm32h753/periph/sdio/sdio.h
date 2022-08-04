#pragma once
#include "stm32_base.h"
#include "dma.h"

enum sdio_ex_state_t {
    SDIO_FREE,
    SDIO_WRITE,
    SDIO_READ
};

enum sdio_cd_state_t {
    SDIO_CONNECTED,
    SDIO_NOT_CONNECTED
};

struct sdio_inst_t {
    SD_HandleTypeDef SD_InitStruct;
    HAL_SD_CardInfoTypeDef SD_CardInfo;
    SemaphoreHandle_t semaphore;
    SemaphoreHandle_t mutex;
    IRQn_Type IRQ;
    enum sdio_ex_state_t state;
    enum sdio_cd_state_t connected;
};

typedef struct {
    SD_TypeDef *SDIO;
    gpio_cfg_t *ck_cfg;
    gpio_cfg_t *cmd_cfg;
    gpio_cfg_t *d0_cfg;
    gpio_cfg_t *d1_cfg;
    gpio_cfg_t *d2_cfg;
    gpio_cfg_t *d3_cfg;
    gpio_cfg_t *cd_cfg;
    dma_cfg_t *dma_cfg;
    int timeout;
    int priority;
    struct sdio_inst_t inst;
} sdio_cfg_t;

int SDIO_Init(sdio_cfg_t *cfg);
int SDIO_ClockEnable(sdio_cfg_t *cfg);
int SDIO_EnableIRQ(sdio_cfg_t *cfg);
int SDIO_DisableIRQ(sdio_cfg_t *cfg);

int SDIO_ReadBlocks(sdio_cfg_t *cfg, uint8_t *pdata, uint32_t address, uint32_t num);
int SDIO_WriteBlocks(sdio_cfg_t *cfg, uint8_t *pdata, uint32_t address, uint32_t num);
int SDIO_CheckStatusWithTimeout(sdio_cfg_t *cfg, uint32_t timeout);
int SDIO_Detect(sdio_cfg_t *cfg);
int SDIO_GetInfo(sdio_cfg_t *cfg, HAL_SD_CardInfoTypeDef *info);
int SDIO_GetStatus(sdio_cfg_t *cfg);

int SDIO_DMA_Handler(sdio_cfg_t *cfg);
int SDIO_IT_Handler(sdio_cfg_t *cfg);
int SDIO_TX_Complete(sdio_cfg_t *cfg);
int SDIO_RX_Complete(sdio_cfg_t *cfg);
