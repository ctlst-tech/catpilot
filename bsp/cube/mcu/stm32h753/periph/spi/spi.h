#pragma once
#include "stm32_base.h"
#include "gpio.h"
#include "dma.h"

enum spi_state_t {
    SPI_FREE,
    SPI_TRANSMIT,
    SPI_RECEIVE
};

struct spi_inst_t {
    SemaphoreHandle_t semaphore;
    SemaphoreHandle_t mutex;
    IRQn_Type IRQ;
    enum spi_state_t state;
};

typedef struct {
    SPI_TypeDef *SPI;
    SPI_HandleTypeDef SPI_InitStruct;
    gpio_cfg_t *mosi_cfg;
    gpio_cfg_t *miso_cfg;
    gpio_cfg_t *sck_cfg;
    dma_cfg_t *dma_mosi_cfg;
    dma_cfg_t *dma_miso_cfg;
    int timeout;
    int priority;
    struct spi_inst_t inst;
} spi_cfg_t;

int SPI_Init(spi_cfg_t *cfg);
int SPI_ClockEnable(spi_cfg_t *cfg);
int SPI_Transmit(spi_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int SPI_Receive(spi_cfg_t *cfg, uint8_t *pdata, uint16_t length);
int SPI_EnableIRQ(spi_cfg_t *cfg);
int SPI_DisableIRQ(spi_cfg_t *cfg);
int SPI_IT_Handler(spi_cfg_t *cfg);
int SPI_DMA_MOSI_Handler(spi_cfg_t *cfg);
int SPI_DMA_MISO_Handler(spi_cfg_t *cfg);
