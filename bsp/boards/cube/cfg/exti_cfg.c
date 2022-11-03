#include "core.h"
#include "hal.h"
#include "gpio.h"
#include "exti.h"
#include "board.h"

exti_t exti_spi1_drdy1 = {
    .gpio = {GPIOD, {GPIO_PIN_15, GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_FREQ_HIGH, 0}},
    .cfg = {EXTI_LINE_15, EXTI_MODE_INTERRUPT, EXTI_TRIGGER_RISING, EXTI_GPIOD, EXTI_D3_PENDCLR_SRC_NONE},
    .handle = {EXTI_LINE_15, NULL}, 
    .irq_priority = 6, 
    .p = {0}
};
