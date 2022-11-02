#ifndef DMA_H
#define DMA_H

#include "core.h"
#include "hal.h"
#include "os.h"

typedef struct {
    DMA_HandleTypeDef init;
    IRQn_Type irq;
    int irq_priority;
} dma_t;

int dma_init(dma_t *cfg);
int dma_reinit(dma_t *cfg);
int dma_clock_enable(dma_t *cfg);
int dma_enable_irq(dma_t *cfg);
int dma_disable_irq(dma_t *cfg);
int dma_irq_handler(dma_t *cfg);

#endif  // DMA_H
