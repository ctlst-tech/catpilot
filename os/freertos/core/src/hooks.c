#include "stm32_base.h"

static volatile int stack_overflow_cnt = 0;
static volatile int stack_overflow_margin = 0;

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                    char *pcTaskName) {
    stack_overflow_margin = vTaskGetStackMargin(xTask);
    stack_overflow_cnt++;
}
