#include "dma.h"

int dma_id_init(dma_t *cfg);
int dma_clock_init(dma_t *cfg);

int dma_init(dma_t *cfg, void(*dma_handler)(void *area), void *area) {
    int rv = 0;

    if (cfg == NULL) {
        return EINVAL;
    }
    if ((rv = dma_id_init(cfg))) {
        return rv;
    }
    if ((rv = dma_clock_init(cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.id, cfg->irq_priority, dma_handler, area))) {
        return rv;
    }

    rv = HAL_DMA_Init(&cfg->init);

    return rv;
}

int dma_id_init(dma_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case DMA1_Stream0_BASE:
            cfg->p.id = DMA1_Stream0_IRQn;
            break;
        case DMA1_Stream1_BASE:
            cfg->p.id = DMA1_Stream1_IRQn;
            break;
        case DMA1_Stream2_BASE:
            cfg->p.id = DMA1_Stream2_IRQn;
            break;
        case DMA1_Stream3_BASE:
            cfg->p.id = DMA1_Stream3_IRQn;
            break;
        case DMA1_Stream4_BASE:
            cfg->p.id = DMA1_Stream4_IRQn;
            break;
        case DMA1_Stream5_BASE:
            cfg->p.id = DMA1_Stream5_IRQn;
            break;
        case DMA1_Stream6_BASE:
            cfg->p.id = DMA1_Stream6_IRQn;
            break;
        case DMA1_Stream7_BASE:
            cfg->p.id = DMA1_Stream7_IRQn;
            break;
        case DMA2_Stream0_BASE:
            cfg->p.id = DMA2_Stream0_IRQn;
            break;
        case DMA2_Stream1_BASE:
            cfg->p.id = DMA2_Stream1_IRQn;
            break;
        case DMA2_Stream2_BASE:
            cfg->p.id = DMA2_Stream2_IRQn;
            break;
        case DMA2_Stream3_BASE:
            cfg->p.id = DMA2_Stream3_IRQn;
            break;
        case DMA2_Stream4_BASE:
            cfg->p.id = DMA2_Stream4_IRQn;
            break;
        case DMA2_Stream5_BASE:
            cfg->p.id = DMA2_Stream5_IRQn;
            break;
        case DMA2_Stream6_BASE:
            cfg->p.id = DMA2_Stream6_IRQn;
            break;
        case DMA2_Stream7_BASE:
            cfg->p.id = DMA2_Stream7_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

int dma_clock_init(dma_t *cfg) {
    switch (cfg->p.id) {
        case DMA1_Stream0_IRQn:
        case DMA1_Stream1_IRQn:
        case DMA1_Stream2_IRQn:
        case DMA1_Stream3_IRQn:
        case DMA1_Stream4_IRQn:
        case DMA1_Stream5_IRQn:
        case DMA1_Stream6_IRQn:
        case DMA1_Stream7_IRQn:
            __HAL_RCC_DMA1_CLK_ENABLE();
            break;
        case DMA2_Stream0_IRQn:
        case DMA2_Stream1_IRQn:
        case DMA2_Stream2_IRQn:
        case DMA2_Stream3_IRQn:
        case DMA2_Stream4_IRQn:
        case DMA2_Stream5_IRQn:
        case DMA2_Stream6_IRQn:
        case DMA2_Stream7_IRQn:
            __HAL_RCC_DMA2_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}
