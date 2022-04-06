#include "stm32_base.h"
#include "stm32_drv.h"
#include "stm32_periph.h"

// #include "cli.h"
// #include "io.h"
// #include "logger.h"
// #include "sensors.h"

#include <pthread.h>
#include <stdio.h>
#include "ff.h"

#include "swsys.h"
#include "function.h"
#include "fsminst.h"

void posix(void *param);
void *thread(void *param);

static FATFS fs;

int main(void) {
    HAL_Init();
    RCC_Init();

    // CLI_Start();
    // Sensors_Start();
    // IO_Start();
    // Logger_Start();

    // vTaskStartScheduler();
    xTaskCreate(posix, "posix", 12000, NULL, 1, NULL );

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
    FILE *file;

    char foo[255];
    int length;

    f_mount(&fs, "0:", 1);
    file = fopen("blah\n", "a");
    length = sprintf(foo, "check fs\nfile buf ptr: %p\n", &file->buf);
    fwrite(foo, 1, length, file);
    fclose(file);
    file = fopen("blah\n", "r");
    fread(foo, 1, length, file);
    fclose(file);

    swsys_load("mvp_swsys.xml", &sys);

    swsys_top_module_start(&sys);

    while(1);
}
