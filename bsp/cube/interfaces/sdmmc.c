#include "init.h"
#include "cfg.h"

sdio_cfg_t sdmmc1;
dma_cfg_t sdmmc1_dma_txrx;
gpio_cfg_t sdmmc1_ck  = GPIO_SDMMC1_CK;
gpio_cfg_t sdmmc1_cmd = GPIO_SDMMC1_CMD;
gpio_cfg_t sdmmc1_d0  = GPIO_SDMMC1_D0;
gpio_cfg_t sdmmc1_d1  = GPIO_SDMMC1_D1;
gpio_cfg_t sdmmc1_d2  = GPIO_SDMMC1_D2;
gpio_cfg_t sdmmc1_d3  = GPIO_SDMMC1_D3;
gpio_cfg_t sdmmc1_cd  = GPIO_SDMMC1_CD;
const int sdmmc1_timeout = 1000;
const int sdmmc1_priority = 7;

int SDMMC1_Init() {
    int rv = 0;

    sdmmc1.SDIO     = SDMMC1;
    sdmmc1.ck_cfg   = &sdmmc1_ck;
    sdmmc1.cmd_cfg  = &sdmmc1_cmd;
    sdmmc1.d0_cfg   = &sdmmc1_d0;
    sdmmc1.d1_cfg   = &sdmmc1_d1;
    sdmmc1.d2_cfg   = &sdmmc1_d2;
    sdmmc1.d3_cfg   = &sdmmc1_d3;
    sdmmc1.cd_cfg   = &sdmmc1_cd;
    sdmmc1.timeout  = sdmmc1_timeout;
    sdmmc1.priority = sdmmc1_priority;

    sdmmc1.dma_cfg = &sdmmc1_dma_txrx;

    sdmmc1_dma_txrx.DMA_InitStruct.Instance = DMA2_Stream6;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.Request = DMA_REQUEST_USART6_RX;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.Mode = DMA_PFCTRL;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.MemBurst = DMA_MBURST_INC4;
    sdmmc1_dma_txrx.DMA_InitStruct.Init.PeriphBurst = DMA_PBURST_INC4;
    sdmmc1_dma_txrx.priority = sdmmc1_priority;

    rv = SDIO_Init(&sdmmc1);

    return rv;
}
