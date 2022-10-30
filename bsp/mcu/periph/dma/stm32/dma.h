#pragma once
#include "stm32_base.h"
#include "gpio.h"

struct dma_inst_t {
    IRQn_Type IRQ;
};

typedef struct {
    DMA_HandleTypeDef DMA_InitStruct;
    int priority;
    struct dma_inst_t inst;
} dma_cfg_t;

int DMA_Init(dma_cfg_t *cfg);
int DMA_ReInit(dma_cfg_t *cfg);
int DMA_ClockEnable(dma_cfg_t *cfg);
int DMA_EnableIRQ(dma_cfg_t *cfg);
int DMA_DisableIRQ(dma_cfg_t *cfg);
int DMA_IRQHandler(dma_cfg_t *cfg);
