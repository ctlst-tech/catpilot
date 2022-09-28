#include "devices.h"
#include "init.h"

int MAG_Init(void) {
    int rv;

    rv = IST8310_Init(&i2c3);
    if(rv) return -1;

    while(IST8310_Operation()) {
        IST8310_Run();
        vTaskDelay(20);
    }

    return 0;
}

void MAG_Update(void) {
    IST8310_Run();
}
