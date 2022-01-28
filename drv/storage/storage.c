#include "storage.h"

static char *device = "SDMMC1";

static gpio_cfg_t storage_ck =  GPIO_SDMMC1_CK;
static gpio_cfg_t storage_cmd = GPIO_SDMMC1_CMD;
static gpio_cfg_t storage_d0 =  GPIO_SDMMC1_D0;
static gpio_cfg_t storage_d1 =  GPIO_SDMMC1_D1;
static gpio_cfg_t storage_d2 =  GPIO_SDMMC1_D2;
static gpio_cfg_t storage_d3 =  GPIO_SDMMC1_D3;
static gpio_cfg_t storage_cd =  GPIO_SDMMC1_CD;

static dma_cfg_t dma_storage_tx;
static dma_cfg_t dma_storage_rx;

static storage_cfg_t storage_cfg;

int Storage_Init() {
    int rv = 0;

    storage_cfg.sdio.ck_cfg  =  &storage_ck;
    storage_cfg.sdio.cmd_cfg =  &storage_cmd;
    storage_cfg.sdio.d0_cfg  =  &storage_d0;
    storage_cfg.sdio.d1_cfg  =  &storage_d1;
    storage_cfg.sdio.d2_cfg  =  &storage_d2;
    storage_cfg.sdio.d3_cfg  =  &storage_d3;
    storage_cfg.sdio.cd_cfg  =  &storage_cd;

    storage_cfg.sdio.dma_tx_cfg = &dma_storage_tx;
    storage_cfg.sdio.dma_rx_cfg = &dma_storage_rx;

    dma_storage_tx.DMA_InitStruct.Instance = DMA2_Stream6;
    dma_storage_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_4;
    dma_storage_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_storage_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_storage_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_storage_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_storage_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_storage_tx.DMA_InitStruct.Init.Mode = DMA_PFCTRL;
    dma_storage_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_storage_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    dma_storage_tx.DMA_InitStruct.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    dma_storage_tx.DMA_InitStruct.Init.MemBurst = DMA_MBURST_INC4;
    dma_storage_tx.DMA_InitStruct.Init.PeriphBurst = DMA_PBURST_INC4;
    dma_storage_tx.priority = storage_cfg.sdio.priority;

    dma_storage_rx.DMA_InitStruct.Instance = DMA2_Stream4;
    dma_storage_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_4;
    dma_storage_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_storage_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_storage_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_storage_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_storage_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_storage_rx.DMA_InitStruct.Init.Mode = DMA_PFCTRL;
    dma_storage_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_storage_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    dma_storage_rx.DMA_InitStruct.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    dma_storage_rx.DMA_InitStruct.Init.MemBurst = DMA_MBURST_INC4;
    dma_storage_rx.DMA_InitStruct.Init.PeriphBurst = DMA_PBURST_INC4;
    dma_storage_rx.priority = storage_cfg.sdio.priority;

    rv = SDIO_Init(&storage_cfg.sdio);

    return rv;

}
