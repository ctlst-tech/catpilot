#include "sdcard.h"
#include "sdcard_conf.h"

static char *device = "SDMMC1";

static gpio_cfg_t sdcard_ck  = GPIO_SDMMC1_CK;
static gpio_cfg_t sdcard_cmd = GPIO_SDMMC1_CMD;
static gpio_cfg_t sdcard_d0  = GPIO_SDMMC1_D0;
static gpio_cfg_t sdcard_d1  = GPIO_SDMMC1_D1;
static gpio_cfg_t sdcard_d2  = GPIO_SDMMC1_D2;
static gpio_cfg_t sdcard_d3  = GPIO_SDMMC1_D3;
static gpio_cfg_t sdcard_cd  = GPIO_SDMMC1_CD;

static gpio_cfg_t sdcard_3v3 = GPIO_VDD_3V3_SD_CARD_EN;

static dma_cfg_t dma_sdcard;

static sdcard_cfg_t sdcard_cfg;

int SDCARD_Init() {
    int rv = 0;

    GPIO_Init(&sdcard_3v3);
    GPIO_Set(&sdcard_3v3);

    vTaskDelay(2);

    sdcard_cfg.sdio.SDIO     = SDMMC1;
    sdcard_cfg.sdio.ck_cfg   = &sdcard_ck;
    sdcard_cfg.sdio.cmd_cfg  = &sdcard_cmd;
    sdcard_cfg.sdio.d0_cfg   = &sdcard_d0;
    sdcard_cfg.sdio.d1_cfg   = &sdcard_d1;
    sdcard_cfg.sdio.d2_cfg   = &sdcard_d2;
    sdcard_cfg.sdio.d3_cfg   = &sdcard_d3;
    sdcard_cfg.sdio.cd_cfg   = &sdcard_cd;
    sdcard_cfg.sdio.timeout  = SDCARD_TIMEOUT;
    sdcard_cfg.sdio.priority = SDCARD_IRQ_PRIORITY;

    sdcard_cfg.sdio.dma_cfg = &dma_sdcard;

    dma_sdcard.DMA_InitStruct.Instance = DMA2_Stream6;
    dma_sdcard.DMA_InitStruct.Init.Channel = DMA_CHANNEL_4;
    dma_sdcard.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_sdcard.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_sdcard.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_sdcard.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_sdcard.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    dma_sdcard.DMA_InitStruct.Init.Mode = DMA_PFCTRL;
    dma_sdcard.DMA_InitStruct.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    dma_sdcard.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    dma_sdcard.DMA_InitStruct.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    dma_sdcard.DMA_InitStruct.Init.MemBurst = DMA_MBURST_INC4;
    dma_sdcard.DMA_InitStruct.Init.PeriphBurst = DMA_PBURST_INC4;
    dma_sdcard.priority = SDCARD_IRQ_PRIORITY;

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

int SDCARD_GetInfo(HAL_SD_CardInfoTypeDef *info) {
    int rv;
    rv = SDIO_GetInfo(&sdcard_cfg.sdio, info);
    return rv;
}

int SDCARD_GetStatus() {
    int rv;
    rv = SDIO_GetStatus(&sdcard_cfg.sdio);
    return rv;
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
    SDIO_RX_Complete(&sdcard_cfg.sdio);
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
    SDIO_TX_Complete(&sdcard_cfg.sdio);
}

void SDMMC1_IRQHandler(void) {
    SDIO_IT_Handler(&sdcard_cfg.sdio);
}

void DMA2_Stream6_IRQHandler(void) {
    SDIO_DMA_Handler(&sdcard_cfg.sdio);
}
