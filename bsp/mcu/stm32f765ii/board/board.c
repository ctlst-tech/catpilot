#include "board.h"
#include "board_cfg.h"

void Board_Fail() {
    while(1);
}

void Board_Init() {
    if(GPIOBoard_Init()) {
        Board_Fail();
    }

    vTaskDelay(2);

    if(USART7_Init()) {
        Board_Fail();
    }
    if(USART8_Init()) {
        Board_Fail();
    }
    if(SPI1_Init()) {
        Board_Fail();
    }
    if(I2C3_Init()) {
        Board_Fail();
    }
    if(SDMMC1_Init()){
        Board_Fail();
    }
}
