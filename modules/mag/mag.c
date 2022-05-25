#include "mag.h"
#include "mag_conf.h"
#include "log.h"

void MAG_Task(void *pvParameters) {
    LOG_DEBUG("MAG", "Start task");
    IST8310_Init();
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    while(ist8310_state != IST8310_READ) {
            IST8310_Run();
            vTaskDelayUntil(&xLastWakeTime, MAG_TASK_PERIOD_MS);
    }

    LOG_DEBUG("MAG", "Start of measurements");

    while(1) {
        IST8310_Run();
        xTaskDelayUntil(&xLastWakeTime, MAG_TASK_PERIOD_MS);
    }
}

void MAG_Start() {
    xTaskCreate(MAG_Task, "MAG", 512, NULL, MAG_TASK_PRIORITY, NULL);
}
