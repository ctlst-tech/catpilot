#include "gpio.h"

static int gpio_clock_init(gpio_t *cfg);

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

static int gpio_clock_init(gpio_t *cfg) {
    switch ((uint32_t)(cfg->port)) {
        case GPIOA_BASE:
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;
        case GPIOB_BASE:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;
        case GPIOC_BASE:
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;
        case GPIOD_BASE:
            __HAL_RCC_GPIOD_CLK_ENABLE();
            break;
        case GPIOE_BASE:
            __HAL_RCC_GPIOE_CLK_ENABLE();
            break;
        case GPIOF_BASE:
            __HAL_RCC_GPIOF_CLK_ENABLE();
            break;
        case GPIOG_BASE:
            __HAL_RCC_GPIOG_CLK_ENABLE();
            break;
        case GPIOH_BASE:
            __HAL_RCC_GPIOH_CLK_ENABLE();
            break;
        default:
            return EINVAL;
    }
    return 0;
}
