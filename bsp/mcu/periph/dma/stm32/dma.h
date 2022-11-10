#ifndef DMA_H
#define DMA_H

#include <errno.h>

#include "core.h"
#include "dma.h"
#include "hal.h"
#include "irq.h"
#include "os.h"

typedef struct {
    IRQn_Type id;
} dma_private_t;

typedef struct {
    DMA_HandleTypeDef init;
    int irq_priority;
    dma_private_t p;
} dma_t;

int dma_init(dma_t *cfg, void (*dma_handler)(void *area), void *area);

#endif  // DMA_H
