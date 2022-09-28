#include "init.h"
#include "cfg.h"

static char *device = "BOARD";

void Board_Fail() {
    while(1);
}

int Board_Init() {
    if(GPIOBoard_Init()) {
        LOG_ERROR("GPIO", "Initialization failed");
        return -1;
    }

    vTaskDelay(2);

    if(USART6_Init()) {
        LOG_ERROR("USART6", "Initialization failed");
        return -1;
    }

    if(SPI1_Init()) {
        LOG_ERROR("SPI1", "Initialization failed");
        return -1;     
    }

    if(SPI2_Init()) {
        LOG_ERROR("SPI2", "Initialization failed");
        return -1;         
    }

    if(SPI4_Init()) {
        LOG_ERROR("SPI4", "Initialization failed");
        return -1;       
    }

    if(ADC12_Init()) {
        LOG_ERROR("ADC12", "Initialization failed");
        return -1;    
    }

    if(SDMMC1_Init()) {
        LOG_ERROR("SDMMC1", "Initialization failed");
        return -1;    
    }

    if(USART2_Init()) {
        LOG_ERROR("USART2", "Initialization failed");
        return -1;
    }

    if(USART3_Init()) {
        LOG_ERROR("USART3", "Initialization failed");
        return -1;
    }

    return 0;
}
