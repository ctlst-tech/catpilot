#include "io.h"
#include "io_conf.h"

void IO_Task(){
    PX4IO_Init();
    while(1) {
        PX4IO_Run();
        vTaskDelay(20);
    }
}

void IO_Start() {
    xTaskCreate(IO_Task, "Sensors", 512, NULL, IO_TASK_PRIORITY, NULL);
}
