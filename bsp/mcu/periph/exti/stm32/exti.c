#include "exti.h"

int exti_init(exti_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return EINVAL;
    }
    if ((rv = gpio_init(&cfg->gpio)) != 0) {
        return rv;
    }

    rv = HAL_EXTI_SetConfigLine((EXTI_HandleTypeDef *)&cfg->handle,
                                (EXTI_ConfigTypeDef *)&cfg->cfg);

    if (rv != 0) {
        return rv;
    }

    if (cfg->gpio.GPIO_InitStruct.Pin == GPIO_PIN_0) {
        cfg->IRQ = EXTI0_IRQn;
    } else if (cfg->gpio.GPIO_InitStruct.Pin == GPIO_PIN_1) {
        cfg->IRQ = EXTI1_IRQn;
    } else if (cfg->gpio.GPIO_InitStruct.Pin == GPIO_PIN_2) {
        cfg->IRQ = EXTI2_IRQn;
    } else if (cfg->gpio.GPIO_InitStruct.Pin == GPIO_PIN_3) {
        cfg->IRQ = EXTI3_IRQn;
    } else if (cfg->gpio.GPIO_InitStruct.Pin == GPIO_PIN_4) {
        cfg->IRQ = EXTI4_IRQn;
    } else if (cfg->gpio.GPIO_InitStruct.Pin >= GPIO_PIN_5 &&
               cfg->gpio.GPIO_InitStruct.Pin <= GPIO_PIN_9) {
        cfg->IRQ = EXTI9_5_IRQn;
    } else if (cfg->gpio.GPIO_InitStruct.Pin >= GPIO_PIN_10 &&
               cfg->gpio.GPIO_InitStruct.Pin <= GPIO_PIN_15) {
        cfg->IRQ = EXTI15_10_IRQn;
    } else {
        rv = EINVAL;
    }

    return rv;
}

void EXTI_EnableIRQ(exti_t *cfg) {
    HAL_NVIC_SetPriority(cfg->IRQ, cfg->irq_priority, 0);
    HAL_NVIC_EnableIRQ(cfg->IRQ);
}

void EXTI_DisableIRQ(exti_t *cfg) {
    HAL_NVIC_SetPriority(cfg->IRQ, cfg->irq_priority, 0);
    HAL_NVIC_DisableIRQ(cfg->IRQ);
}
