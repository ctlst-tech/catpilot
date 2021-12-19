#include "stm32_base.h"
#include "gpio.h"

int GPIO_Init(gpio_cfg_t *cfg) {
    int rv = 0;
    if(cfg == NULL) return EINVAL;
    if((rv = GPIO_ClockEnable(cfg) != 0)) return rv;
    HAL_GPIO_Init(cfg->GPIO, (GPIO_InitTypeDef *)&cfg->GPIO_InitStruct);
    return rv;
}

int GPIO_ClockEnable(gpio_cfg_t *cfg) {
    switch((uint32_t)(cfg->GPIO)) {

#ifdef GPIOA
        case GPIOA_BASE:
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;
#endif

#ifdef GPIOB
        case GPIOB_BASE:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;
#endif

#ifdef GPIOC
        case GPIOC_BASE:
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;
#endif

#ifdef GPIOD
        case GPIOD_BASE:
            __HAL_RCC_GPIOD_CLK_ENABLE();
            break;
#endif

#ifdef GPIOE
        case GPIOE_BASE:
            __HAL_RCC_GPIOE_CLK_ENABLE();
            break;
#endif

#ifdef GPIOF
        case GPIOF_BASE:
            __HAL_RCC_GPIOF_CLK_ENABLE();
            break;
#endif

#ifdef GPIOG
        case GPIOG_BASE:
            __HAL_RCC_GPIOG_CLK_ENABLE();
            break;
#endif

#ifdef GPIOH
        case GPIOH_BASE:
            __HAL_RCC_GPIOH_CLK_ENABLE();
            break;
#endif

    default:
        return EINVAL;
    }

    return 0;
}
