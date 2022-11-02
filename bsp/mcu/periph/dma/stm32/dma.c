#include "dma.h"

int dma_init(dma_t *cfg) {
    int rv = 0;

    if ((rv = dma_clock_enable(cfg)) != 0) {
        return rv;
    }
    if ((rv = dma_enable_irq(cfg)) != 0) {
        return rv;
    }

    rv = HAL_DMA_Init(&cfg->init);

    return rv;
}

int dma_reinit(dma_t *cfg) {
    if (HAL_DMA_DeInit(&cfg->init) != HAL_OK) {
        return EINVAL;
    }
    if (HAL_DMA_Init(&cfg->init) != HAL_OK) {
        return EINVAL;
    }
    return 0;
}

int dma_enable_irq(dma_t *cfg) {
    HAL_NVIC_SetPriority(cfg->irq, cfg->irq_priority, 0);
    HAL_NVIC_EnableIRQ(cfg->irq);
    return 0;
}

int dma_disable_irq(dma_t *cfg) {
    HAL_NVIC_DisableIRQ(cfg->irq);
    return 0;
}

int dma_irq_handler(dma_t *cfg) {
    HAL_DMA_IRQHandler(&cfg->init);
    return 0;
}

int dma_clock_enable(dma_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
#ifdef DMA1_Stream0
        case DMA1_Stream0_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream0_IRQn;
            break;
#endif

#ifdef DMA1_Stream1
        case DMA1_Stream1_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream1_IRQn;
            break;
#endif

#ifdef DMA1_Stream2
        case DMA1_Stream2_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream2_IRQn;
            break;
#endif

#ifdef DMA1_Stream3
        case DMA1_Stream3_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream3_IRQn;
            break;
#endif

#ifdef DMA1_Stream4
        case DMA1_Stream4_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream4_IRQn;
            break;
#endif

#ifdef DMA1_Stream5
        case DMA1_Stream5_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream5_IRQn;
            break;
#endif

#ifdef DMA1_Stream6
        case DMA1_Stream6_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream6_IRQn;
            break;
#endif

#ifdef DMA1_Stream7
        case DMA1_Stream7_BASE:
            __HAL_RCC_DMA1_CLK_ENABLE();
            cfg->irq = DMA1_Stream7_IRQn;
            break;
#endif

#ifdef DMA2_Stream0
        case DMA2_Stream0_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream0_IRQn;
            break;
#endif

#ifdef DMA2_Stream1
        case DMA2_Stream1_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream1_IRQn;
            break;
#endif

#ifdef DMA2_Stream2
        case DMA2_Stream2_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream2_IRQn;
            break;
#endif

#ifdef DMA2_Stream3
        case DMA2_Stream3_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream3_IRQn;
            break;
#endif

#ifdef DMA2_Stream4
        case DMA2_Stream4_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream4_IRQn;
            break;
#endif

#ifdef DMA2_Stream5
        case DMA2_Stream5_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream5_IRQn;
            break;
#endif

#ifdef DMA2_Stream6
        case DMA2_Stream6_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream6_IRQn;
            break;
#endif

#ifdef DMA2_Stream7
        case DMA2_Stream7_BASE:
            __HAL_RCC_DMA2_CLK_ENABLE();
            cfg->irq = DMA2_Stream7_IRQn;
            break;
#endif

        default:
            return EINVAL;
    }

    return 0;
}
