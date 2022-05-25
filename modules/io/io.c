#include "io.h"
#include "io_conf.h"

void IO_Task(){
    LOG_DEBUG("IO", "Start task");
    PX4IO_Init();
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    while(px4io_state != PX4IO_OPERATION) {
            PX4IO_Run();
            vTaskDelayUntil(&xLastWakeTime, IO_TASK_PERIOD_MS);
    }

    LOG_DEBUG("IO", "Operation");

    while(1) {
        PX4IO_Run();
        vTaskDelayUntil(&xLastWakeTime, IO_TASK_PERIOD_MS);
    }
}

void IO_Start() {
    xTaskCreate(IO_Task, "InputOuput", 512, NULL, IO_TASK_PRIORITY, NULL);
}
