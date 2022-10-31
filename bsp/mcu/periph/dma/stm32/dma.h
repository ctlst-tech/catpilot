#ifndef DMA_H
#define DMA_H

#include "core.h"
#include "hal.h"

typedef struct {
    DMA_HandleTypeDef DMA_InitStruct;
    int priority;
    IRQn_Type IRQ;
} dma_cfg_t;

int DMA_Init(dma_cfg_t *cfg);
int DMA_ReInit(dma_cfg_t *cfg);
int DMA_ClockEnable(dma_cfg_t *cfg);
int DMA_EnableIRQ(dma_cfg_t *cfg);
int DMA_DisableIRQ(dma_cfg_t *cfg);
int DMA_IRQHandler(dma_cfg_t *cfg);

#endif  // DMA_H
