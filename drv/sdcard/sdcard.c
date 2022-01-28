#include "sdcard.h"

static char *device = "SDMMC1";

static gpio_cfg_t sdcard_ck =  GPIO_SDMMC1_CK;
static gpio_cfg_t sdcard_cmd = GPIO_SDMMC1_CMD;
static gpio_cfg_t sdcard_d0 =  GPIO_SDMMC1_D0;
static gpio_cfg_t sdcard_d1 =  GPIO_SDMMC1_D1;
static gpio_cfg_t sdcard_d2 =  GPIO_SDMMC1_D2;
static gpio_cfg_t sdcard_d3 =  GPIO_SDMMC1_D3;
static gpio_cfg_t sdcard_cd =  GPIO_SDMMC1_CD;

static dma_cfg_t dma_sdcard_tx;
static dma_cfg_t dma_sdcard_rx;

static sdcard_cfg_t sdcard_cfg;

int SDCARD_Init() {
    int rv = 0;

    sdcard_cfg.sdio.ck_cfg  =  &sdcard_ck;
    sdcard_cfg.sdio.cmd_cfg =  &sdcard_cmd;
    sdcard_cfg.sdio.d0_cfg  =  &sdcard_d0;
    sdcard_cfg.sdio.d1_cfg  =  &sdcard_d1;
    sdcard_cfg.sdio.d2_cfg  =  &sdcard_d2;
    sdcard_cfg.sdio.d3_cfg  =  &sdcard_d3;
    sdcard_cfg.sdio.cd_cfg  =  &sdcard_cd;

    sdcard_cfg.sdio.dma_tx_cfg = &dma_sdcard_tx;
    sdcard_cfg.sdio.dma_rx_cfg = &dma_sdcard_rx;

    dma_sdcard_tx.DMA_InitStruct.Instance = DMA2_Stream6;
    dma_sdcard_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_4;
    dma_sdcard_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_sdcard_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_sdcard_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_sdcard_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_sdcard_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_sdcard_tx.DMA_InitStruct.Init.Mode = DMA_PFCTRL;
    dma_sdcard_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_sdcard_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    dma_sdcard_tx.DMA_InitStruct.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    dma_sdcard_tx.DMA_InitStruct.Init.MemBurst = DMA_MBURST_INC4;
    dma_sdcard_tx.DMA_InitStruct.Init.PeriphBurst = DMA_PBURST_INC4;
    dma_sdcard_tx.priority = sdcard_cfg.sdio.priority;

    dma_sdcard_rx.DMA_InitStruct.Instance = DMA2_Stream4;
    dma_sdcard_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_4;
    dma_sdcard_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_sdcard_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_sdcard_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_sdcard_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_sdcard_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_sdcard_rx.DMA_InitStruct.Init.Mode = DMA_PFCTRL;
    dma_sdcard_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_sdcard_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    dma_sdcard_rx.DMA_InitStruct.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    dma_sdcard_rx.DMA_InitStruct.Init.MemBurst = DMA_MBURST_INC4;
    dma_sdcard_rx.DMA_InitStruct.Init.PeriphBurst = DMA_PBURST_INC4;
    dma_sdcard_rx.priority = sdcard_cfg.sdio.priority;

    rv = SDIO_Init(&sdcard_cfg.sdio);

    return rv;
}

int SDCARD_Read(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = SDIO_ReadBlocks(&sdcard_cfg.sdio, pdata, address, num);
    return rv;
}

int SDCARD_Write(uint8_t *pdata, uint32_t address, uint32_t num) {
    int rv;
    rv = SDIO_WriteBlocks(&sdcard_cfg.sdio, pdata, address, num);
    return rv;
}

int SDCARD_SendCommand(uint32_t cmd) {
    int rv;
    (void)cmd;
    rv = SDIO_Check(&sdcard_cfg.sdio);
    return rv;
}

int SDCARD_Status() {
    int rv;
    rv = SDIO_GetStatus(&sdcard_cfg.sdio);
    return rv;
}
