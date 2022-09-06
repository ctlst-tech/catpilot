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

#include "fatfs_posix.h"

#define LOG_STDOUT_ENABLE 1
#define ECHO_ENABLE 1

void main_thread(void *param);
void *ctlst(void *param);
static FATFS fs;

int main(void) {
    HAL_Init();
    RCC_Init();
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

uint8_t buf_read[1024];
uint8_t buf_write[4096];
void echo(void) {
    int fd;
    int rd_len, wr_len;
    char new_line[4] = "\r\n# ";
    fd = open("/dev/ttyS0", O_RDWR | O_CREAT | O_TRUNC);
    while(1) {
        rd_len = 0;
        wr_len = 0;
        rd_len = read(fd, buf_read, 1024);
        for(int i = 0; i < rd_len; i++) {
            if(buf_read[i] == '\r') {
                memcpy(buf_write + wr_len, new_line, sizeof(new_line));
                wr_len += sizeof(new_line);
            } else {
                buf_write[wr_len] = buf_read[i];
                wr_len++;
            }
        }
        wr_len = write(fd, buf_write, wr_len);
    }
}

void *ctlst(void *param) {
    static FRESULT res;
    swsys_t sys;
    int rv = 0;
    int fd;

    pthread_setname_np(__func__);

    #if(LOG_STDOUT_ENABLE)
        log_enable(true);
    #else
        log_enable(false);
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

    nd = nodereg("/dev/ttyS0");
    noderegopen(nd, usart_posix_open);
    noderegwrite(nd, usart_posix_write);
    noderegread(nd, usart_posix_read);
    noderegclose(nd, usart_posix_close);
    noderegfilealloc(nd, NULL);
    noderegdevcfg(nd, &usart7);

    Logger_Init();

    if (res == FR_OK) {
        LOG_INFO("SDMMC", "Mount successful");
    } else {
        LOG_ERROR("SDMMC", "Mount error");
    }

    #if(ECHO_ENABLE)
        xTaskCreate(echo, "echo", 512, NULL, 1, NULL );
    #endif

    if (res == FR_OK) {
        swsys_rv_t swsys_rv = swsys_load("/fs/config/mvp_swsys.xml", "/fs/config", &sys);
        if (swsys_rv == swsys_e_ok) {
            LOG_INFO("SYSTEM", "System starts")
            swsys_top_module_start(&sys);
        } else {
            LOG_ERROR("SYSTEM", "SWSYS config load error")
        }
    } else {
    }

    while(1);
}

