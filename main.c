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

FILE stdout_stream;
char stdout_buf[256];

FILE stderr_stream;
char stderr_buf[256];

void posix(void *param);
void *thread(void *param);

extern int cli_put(char c, struct __file * file);

static FATFS fs;
static FIL filefs;
static DIR dirfs;
static FILINFO filinfo;

int main(void) {
    HAL_Init();
    RCC_Init();

    stdout = &stdout_stream;
    stdout->put = cli_put;
    stdout->buf = stdout_buf;
    stdout->size = 256;
    stdout->flags = __SWR;

    stderr = &stderr_stream;
    stderr->put = cli_put;
    stderr->buf = stderr_buf;
    stderr->size = 256;
    stderr->flags = __SWR;

    // CLI_Start();
    // Sensors_Start();
    // IO_Start();
    // Logger_Start();

    // vTaskStartScheduler();
    xTaskCreate(posix, "posix", 16000, NULL, 1, NULL );

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
    char foo2[1024];
    int length;
    int i;

    i = f_mount(&fs, "0:", 1);
    file = fopen("blah.txt\n", "a");
    length = sprintf(foo, "check fs\nfile buf ptr: %p\n", &file->buf);
    fwrite(foo, 1, length, file);
    fclose(file);
    file = fopen("blah.txt\n", "r");
    fread(foo2, 1, length, file);
    fclose(file);
    // printf("sss");

    i = f_open(&filefs, "./mvp_swsys.xml", FA_READ);
    i = f_read(&filefs, foo2, 1024, &i);

    CLI_Init();

    ICM20602_Init();
    IST8310_Init();
    usleep(1000);

    printf("Catalyst demo project\n");

    swsys_load("mvp_swsys.xml", &sys);
    swsys_top_module_start(&sys);

    while(1);
}
