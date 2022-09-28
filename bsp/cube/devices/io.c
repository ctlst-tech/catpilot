#include "devices.h"
#include "init.h"

int IO_Init(void) {
    int rv;

    rv = CubeIO_Init(&usart6);
    if(rv) return -1;

    while(CubeIO_Operation()) {
        CubeIO_Run();
        vTaskDelay(1);
    }

    return 0;
}

void IO_Update(void) {
    CubeIO_Run();
}
