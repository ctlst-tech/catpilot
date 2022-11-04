#include "gpio.h"

int gpio_clock_init(gpio_t *cfg);

int gpio_init(gpio_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return EINVAL;
    }
    if ((rv = gpio_clock_init(cfg)) != 0) {
        return rv;
    }

    HAL_GPIO_Init(cfg->port, (GPIO_InitTypeDef *)&cfg->init);

    return rv;
}

void gpio_set(gpio_t *cfg) {
    HAL_GPIO_WritePin(cfg->port, cfg->init.Pin, GPIO_PIN_SET);
}

void gpio_reset(gpio_t *cfg) {
    HAL_GPIO_WritePin(cfg->port, cfg->init.Pin, GPIO_PIN_RESET);
}

void gpio_toggle(gpio_t *cfg) {
    HAL_GPIO_TogglePin(cfg->port, cfg->init.Pin);
}

void gpio_set_state(gpio_t *cfg, uint8_t state) {
    if (state) {
        gpio_set(cfg);
    } else {
        gpio_reset(cfg);
    }
}

int gpio_read(gpio_t *cfg) {
    int rv;
    rv = HAL_GPIO_ReadPin(cfg->port, cfg->init.Pin);
    return rv;
}

int gpio_clock_init(gpio_t *cfg) {
    switch ((uint32_t)(cfg->port)) {
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
