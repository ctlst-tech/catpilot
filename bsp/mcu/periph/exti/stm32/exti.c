#include "exti.h"

static int exti_get_id(exti_t *cfg);

int exti_init(exti_t *cfg, void (*handler)(void *area), void *area) {
    int rv = 0;

    if (cfg == NULL) {
        return EINVAL;
    }
    if ((rv = exti_get_id(cfg)) != 0) {
        return rv;
    }
    if ((rv = gpio_init(&cfg->gpio)) != 0) {
        return rv;
    }
    rv = HAL_EXTI_SetConfigLine((EXTI_HandleTypeDef *)&cfg->handle,
                                (EXTI_ConfigTypeDef *)&cfg->cfg);
    if (rv != HAL_OK) {
        return rv;
    }
    if ((rv = irq_init(cfg->p.id, cfg->irq_priority, handler, area))) {
        return rv;
    }
    irq_disable(cfg->p.id);

    return rv;
}

static int exti_get_id(exti_t *cfg) {
    switch (cfg->gpio.init.Pin) {
        case GPIO_PIN_0:
            cfg->p.id = EXTI0_IRQn;
            break;
        case GPIO_PIN_1:
            cfg->p.id = EXTI1_IRQn;
            break;
        case GPIO_PIN_2:
            cfg->p.id = EXTI2_IRQn;
            break;
        case GPIO_PIN_3:
            cfg->p.id = EXTI3_IRQn;
            break;
        case GPIO_PIN_4:
            cfg->p.id = EXTI4_IRQn;
            break;
        case GPIO_PIN_5:
        case GPIO_PIN_6:
        case GPIO_PIN_7:
        case GPIO_PIN_8:
        case GPIO_PIN_9:
            cfg->p.id = EXTI9_5_IRQn;
            break;
        case GPIO_PIN_10:
        case GPIO_PIN_11:
        case GPIO_PIN_12:
        case GPIO_PIN_13:
        case GPIO_PIN_14:
        case GPIO_PIN_15:
            cfg->p.id = EXTI15_10_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}
