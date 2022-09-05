#include "dma.h"

int DMA_Init(dma_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = DMA_ClockEnable(cfg)) != 0) return rv;
    if((rv = DMA_EnableIRQ(cfg)) != 0) return rv;

    if((rv = HAL_DMA_Init(&cfg->DMA_InitStruct)) != 0) return rv;

    portEXIT_CRITICAL();

    return rv;
}

int DMA_ReInit(dma_cfg_t *cfg) {
    if(HAL_DMA_DeInit(&cfg->DMA_InitStruct) != HAL_OK) return EINVAL;
    if(HAL_DMA_Init(&cfg->DMA_InitStruct) != HAL_OK) return EINVAL;
    return 0;
}

int DMA_EnableIRQ(dma_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.IRQ);
    return 0;
}

int DMA_DisableIRQ(dma_cfg_t *cfg)  {
    HAL_NVIC_DisableIRQ(cfg->inst.IRQ);
    return 0;
}

int DMA_IRQHandler(dma_cfg_t *cfg) {
    HAL_DMA_IRQHandler(&cfg->DMA_InitStruct);
    return 0;
}

int DMA_ClockEnable(dma_cfg_t *cfg) {
    switch((uint32_t)(cfg->DMA_InitStruct.Instance)) {

#ifdef DMA1_Stream0
    case DMA1_Stream0_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream0_IRQn;
        break;
#endif

#ifdef DMA1_Stream1
    case DMA1_Stream1_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream1_IRQn;
        break;
#endif

#ifdef DMA1_Stream2
    case DMA1_Stream2_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream2_IRQn;
        break;
#endif

#ifdef DMA1_Stream3
    case DMA1_Stream3_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream3_IRQn;
        break;
#endif

#ifdef DMA1_Stream4
    case DMA1_Stream4_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream4_IRQn;
        break;
#endif

#ifdef DMA1_Stream5
    case DMA1_Stream5_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream5_IRQn;
        break;
#endif

#ifdef DMA1_Stream6
    case DMA1_Stream6_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream6_IRQn;
        break;
#endif

#ifdef DMA1_Stream7
    case DMA1_Stream7_BASE:
        __HAL_RCC_DMA1_CLK_ENABLE();
        cfg->inst.IRQ = DMA1_Stream7_IRQn;
        break;
#endif

#ifdef DMA2_Stream0
    case DMA2_Stream0_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream0_IRQn;
        break;
#endif

#ifdef DMA2_Stream1
    case DMA2_Stream1_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream1_IRQn;
        break;
#endif

#ifdef DMA2_Stream2
    case DMA2_Stream2_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream2_IRQn;
        break;
#endif

#ifdef DMA2_Stream3
    case DMA2_Stream3_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream3_IRQn;
        break;
#endif

#ifdef DMA2_Stream4
    case DMA2_Stream4_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream4_IRQn;
        break;
#endif

#ifdef DMA2_Stream5
    case DMA2_Stream5_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream5_IRQn;
        break;
#endif

#ifdef DMA2_Stream6
    case DMA2_Stream6_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream6_IRQn;
        break;
#endif

#ifdef DMA2_Stream7
    case DMA2_Stream7_BASE:
        __HAL_RCC_DMA2_CLK_ENABLE();
        cfg->inst.IRQ = DMA2_Stream7_IRQn;
        break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}
