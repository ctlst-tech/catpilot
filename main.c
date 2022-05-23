#include "stm32_base.h"
#include "drv.h"
#include "stm32_periph.h"

#include "cli.h"
#include "icm20602.h"
#include "icm20689.h"
#include "bmi055.h"
#include "ist8310.h"
#include "px4io.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "ff.h"

#include "log.h"

#include "swsys.h"
#include "function.h"
#include "fsminst.h"

void main_thread(void *param);
void ctlst(void *param);

int main(void) {
    HAL_Init();
    RCC_Init();

    xTaskCreate(main_thread, "main_thread", 42000, NULL, 1, NULL );

    vTaskStartScheduler();

    while(1) {
    }
}

void main_thread(void *param) {
    pthread_t tid;
    pthread_attr_t attr;
    int arg = 0;

    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, ctlst, &arg);
    pthread_join(tid, NULL);
    pthread_exit(NULL);
}

static FATFS fs;

void ctlst(void *param) {
    static FRESULT res;
    swsys_t sys;
    int rv = 0;

    rv = Board_Init();
    CLI_Init();

    printf("\n\n\n \t\tCATALYST AUTOPILOT DEMO PROJECT\n");

    if(rv) {
        LOG_ERROR("BOARD", "Initialization failed");
    } else {
        LOG_INFO("BOARD", "Initialization successful")
    }

    ICM20602_Init();
    ICM20689_Init();
    BMI055_Init();
    IST8310_Init();
    PX4IO_Init();
    usleep(1000);

    res = f_mount(&fs, "0:", 1);

    int fd_dev;
    int fd;
    fd_dev = open("/dev/ttyS0", O_RDWR);
    char *blah = "ping\n";
    write(fd_dev, blah, sizeof(blah));

    fd = open("file.txt", O_RDWR | O_CREAT | O_TRUNC);
    write(fd, "\ntext\ntext\n", 12);
    close(fd);

    fd = open("file.txt", O_RDONLY);
    char buf[25];
    read(fd, buf, sizeof(buf));
    close(fd);

    write(fd_dev, buf, sizeof(buf));

    swsys_load("mvp_swsys.xml", "/", &sys);
    swsys_top_module_start(&sys);

    while(1);
}
