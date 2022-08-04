#include "hal.h"

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(Delay);
}

uint32_t HAL_GetTick(void) {
    return xTaskGetTickCount();
}
