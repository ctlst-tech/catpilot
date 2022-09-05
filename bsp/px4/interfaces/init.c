#include "init.h"
#include "cfg.h"

static char *device = "BOARD";

void Board_Fail(void) {
    while(1);
}

int Board_Init(void) {
    int rv = 0;

    rv |= GPIOBoard_Init();
    vTaskDelay(2);
    rv |= USART7_Init();
    rv |= USART8_Init();
    rv |= SPI1_Init();
    rv |= I2C3_Init();
    rv |= SDMMC1_Init();

    return rv;
}
