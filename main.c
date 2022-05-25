#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "stm32_base.h"
#include "stm32_periph.h"
#include "drv.h"

#include "ff.h"
#include "log.h"

#include "swsys.h"
#include "function.h"
#include "fsminst.h"

#include "imu.h"
#include "io.h"
#include "mag.h"

void main_thread(void *param);
void *ctlst(void *param);
static FATFS fs;

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

void *ctlst(void *param) {
    static FRESULT res;
    swsys_t sys;
    int rv = 0;
    int fd;

    rv = Board_Init();
    CLI_Init();

    printf("\n\n\n \t\tCATALYST AUTOPILOT DEMO PROJECT\n");

    if(rv) {
        LOG_ERROR("BOARD", "Initialization failed");
    } else {
        LOG_INFO("BOARD", "Initialization successful")
    }

    IMU_Start();
    MAG_Start();
    IO_Start();

    res = f_mount(&fs, "0:", 1);
    fd = open("/dev/ttyS0", O_RDWR);

    if(fd < 0) {
        LOG_ERROR("ttyS0", "Failed to open");
    } else {
        LOG_DEBUG("ttyS0", "Opened successfully");
    }

    if (res == FR_OK) {
        swsys_rv_t swsys_rv = swsys_load("mvp_swsys.xml", "/", &sys);
        if (swsys_rv == swsys_e_ok) {
            LOG_INFO("SYSTEM", "System starts")
            swsys_top_module_start(&sys);
        } else {
            LOG_ERROR("SYSTEM", "SWSYS config load error")
        }
    } else {
        LOG_ERROR("BOARD", "f_mount error")
    }

    while(1);
}
