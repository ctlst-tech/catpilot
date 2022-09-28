#include "devices.h"
#include "init.h"

int BARO_INT_Init(void) {
    int rv;

    rv = MS5611_Init(&spi1, 
                     &gpio_spi1_cs2);

    if(rv) return -1;

    while(MS5611_Operation()) {
        MS5611_Run();
    }

    return 0;
}

void BARO_INT_Update(void) {
    MS5611_Run();
}
