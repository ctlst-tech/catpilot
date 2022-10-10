#include "tim.h"

int TIM_Init(tim_cfg_t *cfg) {

    portENTER_CRITICAL();

    int rv = 0;
    if((rv = TIM_ClockEnable(cfg)) != 0) return rv;

    cfg->TIM_InitStruct.Instance = cfg->TIM;

    if(HAL_TIM_Base_Init(&cfg->TIM_InitStruct) != HAL_OK) return EINVAL;
    TIM_DisableIRQ(cfg);

    portEXIT_CRITICAL();

    return rv;
}

void TIM_Start(tim_cfg_t *cfg) {
    cfg->counter = 0;
    HAL_TIM_Base_Start_IT(&cfg->TIM_InitStruct);
}

void TIM_Stop(tim_cfg_t *cfg) {
    HAL_TIM_Base_Stop_IT(&cfg->TIM_InitStruct);
}

uint32_t TIM_GetTick(tim_cfg_t *cfg)  {
    return (cfg->counter) * (cfg->counter_scaler_us);
}

int TIM_EnableIRQ(tim_cfg_t *cfg) {
    HAL_NVIC_SetPriority(cfg->inst.IRQ, cfg->priority, 0);
    HAL_NVIC_EnableIRQ(cfg->inst.IRQ);
    return 0;
}

int TIM_DisableIRQ(tim_cfg_t *cfg)  {
    HAL_NVIC_DisableIRQ(cfg->inst.IRQ);
    return 0;
}

int TIM_Handler(tim_cfg_t *cfg)  {
    HAL_TIM_IRQHandler(&cfg->TIM_InitStruct);
    return 0;
}

int TIM_ClockEnable(tim_cfg_t *cfg) {
    switch((uint32_t)(cfg->TIM)) {

#ifdef TIM1
    case TIM1_BASE:
        __HAL_RCC_TIM1_CLK_ENABLE();
        cfg->inst.IRQ = TIM1_CC_IRQn;
        break;
#endif

#ifdef TIM2
    case TIM2_BASE:
        __HAL_RCC_TIM2_CLK_ENABLE();
        cfg->inst.IRQ = TIM2_IRQn;
        break;
#endif

#ifdef TIM3
    case TIM3_BASE:
        __HAL_RCC_TIM3_CLK_ENABLE();
        cfg->inst.IRQ = TIM3_IRQn;
        break;
#endif

#ifdef TIM4
    case TIM4_BASE:
        __HAL_RCC_TIM4_CLK_ENABLE();
        cfg->inst.IRQ = TIM4_IRQn;
        break;
#endif

#ifdef TIM5
    case TIM5_BASE:
        __HAL_RCC_TIM5_CLK_ENABLE();
        cfg->inst.IRQ = TIM5_IRQn;
        break;
#endif

#ifdef TIM6
    case TIM6_BASE:
        __HAL_RCC_TIM6_CLK_ENABLE();
        cfg->inst.IRQ = TIM6_DAC_IRQn;
        break;
#endif

#ifdef TIM7
    case TIM7_BASE:
        __HAL_RCC_TIM7_CLK_ENABLE();
        cfg->inst.IRQ = TIM7_IRQn;
        break;
#endif

#ifdef TIM8
    case TIM8_BASE:
        __HAL_RCC_TIM8_CLK_ENABLE();
        cfg->inst.IRQ = TIM8_CC_IRQn;
        break;
#endif

#ifdef TIM9
    case TIM9_BASE:
        __HAL_RCC_TIM9_CLK_ENABLE();
        cfg->inst.IRQ = TIM1_BRK_TIM9_IRQn;
        break;
#endif

#ifdef TIM10
    case TIM10_BASE:
        __HAL_RCC_TIM10_CLK_ENABLE();
        cfg->inst.IRQ = TIM1_UP_TIM10_IRQn;
        break;
#endif

#ifdef TIM11
    case TIM11_BASE:
        __HAL_RCC_TIM11_CLK_ENABLE();
        cfg->inst.IRQ = TIM1_TRG_COM_TIM11_IRQn;
        break;
#endif

#ifdef TIM12
    case TIM12_BASE:
        __HAL_RCC_TIM12_CLK_ENABLE();
        cfg->inst.IRQ = TIM8_BRK_TIM12_IRQn;
        break;
#endif

#ifdef TIM13
    case TIM13_BASE:
        __HAL_RCC_TIM13_CLK_ENABLE();
        cfg->inst.IRQ = TIM8_UP_TIM13_IRQn;
        break;
#endif

#ifdef TIM14
    case TIM14_BASE:
        __HAL_RCC_TIM14_CLK_ENABLE();
        cfg->inst.IRQ = TIM8_TRG_COM_TIM14_IRQn;
        break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}

