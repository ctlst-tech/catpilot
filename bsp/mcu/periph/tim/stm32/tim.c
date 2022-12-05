#include "tim.h"

static int tim_id_init(tim_t *cfg);
static int tim_clock_init(tim_t *cfg);
void tim_handler(void *area);

int tim_init(tim_t *cfg) {
    int rv = 0;

    if ((rv = tim_id_init(cfg))) {
        return rv;
    }
    if ((rv = tim_clock_init(cfg))) {
        return rv;
    }
    if ((rv = HAL_TIM_Base_Init(&cfg->init))) {
        return rv;
    }
    if ((rv = irq_init(cfg->p.id, cfg->irq_priority, tim_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.id))) {
        return rv;
    }
    return rv;
}

void tim_start(tim_t *cfg) {
    cfg->p.counter = 0;
    HAL_TIM_Base_Start_IT(&cfg->init);
}

void tim_stop(tim_t *cfg) {
    HAL_TIM_Base_Stop_IT(&cfg->init);
}

uint32_t tim_get_tick(tim_t *cfg) {
    return (cfg->p.counter) * (cfg->counter_scaler_us);
}

void tim_handler(void *area) {
    tim_t *cfg = (tim_t *)area;
    HAL_TIM_IRQHandler(&cfg->init);
}

static int tim_id_init(tim_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case TIM1_BASE:
            cfg->p.id = TIM1_CC_IRQn;
            break;
        case TIM2_BASE:
            cfg->p.id = TIM2_IRQn;
            break;
        case TIM3_BASE:
            cfg->p.id = TIM3_IRQn;
            break;
        case TIM4_BASE:
            cfg->p.id = TIM4_IRQn;
            break;
        case TIM5_BASE:
            cfg->p.id = TIM5_IRQn;
            break;
        case TIM6_BASE:
            cfg->p.id = TIM6_DAC_IRQn;
            break;
        case TIM7_BASE:
            cfg->p.id = TIM7_IRQn;
            break;
        case TIM8_BASE:
            cfg->p.id = TIM8_CC_IRQn;
            break;
        case TIM12_BASE:
            cfg->p.id = TIM8_BRK_TIM12_IRQn;
            break;
        case TIM13_BASE:
            cfg->p.id = TIM8_UP_TIM13_IRQn;
            break;
        case TIM14_BASE:
            cfg->p.id = TIM8_TRG_COM_TIM14_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static int tim_clock_init(tim_t *cfg) {
    switch ((uint32_t)(cfg->p.id)) {
        case TIM1_CC_IRQn:
            __HAL_RCC_TIM1_CLK_ENABLE();
            break;
        case TIM2_IRQn:
            __HAL_RCC_TIM2_CLK_ENABLE();
            break;
        case TIM3_IRQn:
            __HAL_RCC_TIM3_CLK_ENABLE();
            break;
        case TIM4_IRQn:
            __HAL_RCC_TIM4_CLK_ENABLE();
            break;
        case TIM5_IRQn:
            __HAL_RCC_TIM5_CLK_ENABLE();
            break;
        case TIM6_DAC_IRQn:
            __HAL_RCC_TIM6_CLK_ENABLE();
            break;
        case TIM7_IRQn:
            __HAL_RCC_TIM7_CLK_ENABLE();
            break;
        case TIM8_CC_IRQn:
            __HAL_RCC_TIM8_CLK_ENABLE();
            break;
        case TIM8_BRK_TIM12_IRQn:
            __HAL_RCC_TIM12_CLK_ENABLE();
            break;
        case TIM8_UP_TIM13_IRQn:
            __HAL_RCC_TIM13_CLK_ENABLE();
            break;
        case TIM8_TRG_COM_TIM14_IRQn:
            __HAL_RCC_TIM14_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}
