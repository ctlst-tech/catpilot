#include "init.h"
#include "cfg.h"

static char *device = "BOARD";

void Board_Fail() {
    while(1);
}

int Board_Init() {
    int rv = 0;
    rv |= GPIOBoard_Init();
    vTaskDelay(2);
    rv |= USART7_Init();
    rv |= USART6_Init();
    rv |= SPI1_Init();
    rv |= SPI2_Init();
    rv |= SPI4_Init();
    rv |= SDMMC1_Init();

    return rv;
}
