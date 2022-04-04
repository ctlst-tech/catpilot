#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"

// #include "cli.h"
// #include "io.h"
// #include "logger.h"
// #include "sensors.h"

#include <pthread.h>

#include "swsys.h"
#include "function.h"
#include "fsminst.h"

void posix(void *param);
void *thread(void *param);

int main(void) {
    HAL_Init();
    RCC_Init();

    // CLI_Start();
    // Sensors_Start();
    // IO_Start();
    // Logger_Start();

    // vTaskStartScheduler();
    xTaskCreate(posix, "posix", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

    vTaskStartScheduler();
    while(1) {
    }
}

void posix(void *param) {
    int arg = 0;

    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, thread, &arg);
    pthread_join(tid, NULL);
    while(1);
}

void *thread(void *param) {
    swsys_t sys;

    swsys_load("tests/sens/mvp_swsys.xml", &sys);

    swsys_top_module_start(&sys);

    while(1);
}
