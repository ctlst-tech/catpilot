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

    struct termios 	termios_p;

    int fd = open("/dev/ttyS0", O_RDWR);
    if (fd == -1) {
        LOG_ERROR("EQRB", "ttyS0 open failed");
    }

    tcgetattr(fd, &termios_p);
    cfsetispeed(&termios_p, 1000000U);
    cfsetospeed(&termios_p, 1000000U);
    tcsetattr(fd, TCSANOW, &termios_p);
    tcflush(fd, TCIOFLUSH);

    while(1){
        write(fd, "test\n", 6);
        usleep(1000);
    }

    swsys_load("mvp_swsys.xml", "/", &sys);
    swsys_top_module_start(&sys);

    while(1);
}
