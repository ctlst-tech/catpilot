#include "devices.h"
#include "ist8310.h"

int MAG_Init(void) {
    IST8310_Init();
    while(ist8310_state != IST8310_READ) {
            IST8310_Run();
            vTaskDelayUntil(&xLastWakeTime, MAG_TASK_PERIOD_MS);
    }
    return 0;
}

void MAG_Update(void) {
    ICM20602_Run();
    ICM20689_Run(); 
}
