#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <node.h>
#include <sys/termios.h>

#include "stm32_base.h"
#include "stm32_periph.h"

#include "ff.h"
#include "log.h"

#include "swsys.h"
#include "function.h"
#include "fsminst.h"

#include "init.h"
#include "devices.h"
#include "logger.h"
#include "module.h"

#include "fatfs_posix.h"

#define LOG_STDOUT_ENABLE 1
#define ECHO_ENABLE 1

void main_thread(void *param);
void *ctlst(void *param);
int xml_inline_mount(const char *mount_to);

static FATFS fs;

int main(void) {
    HAL_Init();
    RCC_Init();
    Module_Start("Monitor",
                Monitor,
                Monitor_Update,
                1000,
                8);
    xTaskCreate(main_thread, "main_thread", 100, NULL, 3, NULL );
    vTaskStartScheduler();
    while(1);
}

void main_thread(void *param) {
    pthread_t tid;
    pthread_attr_t attr;
    int arg = 0;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 65535);
    pthread_create(&tid, &attr, ctlst, &arg);
    pthread_join(tid, NULL);
    pthread_exit(NULL);
}

static void uart_testing() {
    int rv1;
    int rv2;
    const char *buf1 = "ttyS1 probe\n\r";
    const char *buf2 = "ttyS2 probe\n";
    const char *buf1_r = "same_fd_write\n\r";
    int ttyS1 = open("/dev/ttyS1", O_RDWR);
    int ttyS1_d = open("/dev/ttyS1", O_RDWR);
    //    int ttyS2 = open("/dev/ttyS2", O_RDWR);
    int cntr = 0;
    while(1) {
        cntr++;
        if (cntr > 10) {
            write(ttyS1, buf1, strlen(buf1));
            cntr = 0;
        }
        write(ttyS1_d, buf1_r, strlen(buf1_r));
    //        write(ttyS2, buf2, strlen(buf2));
        vTaskDelay(10);
    }
}

void *ctlst(void *param) {
    static FRESULT res;
    swsys_t sys;
    int rv = 0;
    int fd;

    pthread_setname_np((char *)__func__);

    CLI();

    #if(LOG_STDOUT_ENABLE)
        log_enable(true);
    #else
        log_enable(false);
    #endif

    #if(ECHO_ENABLE)
        CLI_EchoStart();
    #endif

    if(Devices_Init()) {
        LOG_INFO("BOARD", "Sleeping...");
        while(1) {
            vTaskDelay(1000);
        }
    }

    res = f_mount(&fs, "/", 1);
    mkdir("/fs", S_IRWXU);
    mkdir("/fs/config", S_IRWXU);

    int nd = nodereg("/fs");
    noderegopen(nd, fatfs_open);
    noderegwrite(nd, fatfs_write);
    noderegread(nd, fatfs_read);
    noderegclose(nd, fatfs_close);
    noderegfilealloc(nd, fatfs_filealloc);
    noderegdevcfg(nd, NULL);

    Logger_Init();

    if (res == FR_OK) {
        LOG_INFO("SDMMC", "Mount successful");
    } else {
        LOG_ERROR("SDMMC", "Mount error");
    }



    xml_inline_mount("/cfg");

    if (res == FR_OK) {
        swsys_rv_t swsys_rv = swsys_load("/cfg/mvp_swsys.xml", "/cfg/", &sys);
        if (swsys_rv == swsys_e_ok) {
            LOG_INFO("SYSTEM", "System starts")
            swsys_top_module_start(&sys);
        } else {
            LOG_ERROR("SYSTEM", "SWSYS config load error")
        }
    } else {
    }

    while(1) {
        vTaskDelay(10000);
    }
}

