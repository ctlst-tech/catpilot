#include "devices.h"
#include "init.h"

int IO_Init(void) {
    int rv;

    rv = PX4IO_Init(&usart6);
    if(rv) return -1;

    while(PX4IO_Operation()) {
        PX4IO_Run();
        vTaskDelay(5);
    }

    return 0;
}

void IO_Update(void) {
    PX4IO_Run();
}
