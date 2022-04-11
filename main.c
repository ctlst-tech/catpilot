#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"

#include "cli.h"
#include "icm20602.h"
#include "ist8310.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "ff.h"

#include "swsys.h"
#include "function.h"
#include "fsminst.h"

void main_thread(void *param);

int main(void) {
    HAL_Init();
    RCC_Init();

    xTaskCreate(main_thread, "main_thread", 16000, NULL, 1, NULL );

    vTaskStartScheduler();

    while(1) {
    }
}

void main_thread(void *param) {
    swsys_t sys;

    CLI_Init();
    ICM20602_Init();
    IST8310_Init();
    usleep(1000);

    printf("Catalyst demo project\n");

    swsys_load("mvp_swsys.xml", &sys);
    swsys_top_module_start(&sys);

    while(1);
}
