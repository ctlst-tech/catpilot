#include "devices.h"
#include "module.h"
#include "init.h"
#include "log.h"

int CLI(void) {
//    if(USART7_Init()) return -1;
//    CLI_Init(&usart7);
    if(USART3_Init()) return -1;
    CLI_Init(&usart3);
    WELCOME();
    return 0;
}

int Monitor(void) {
    if(TIM2_Init()) return -1;
    Monitor_Init(&tim2);
    return 0;
}

int Uptime_Init(void) {
    return 0;
}

void Uptime_Update(void) {
    LOG_INFO("Uptime", "");
}

int Devices_Init(void) {
    int rv;

    rv = Board_Init();

    if(rv) {
        LOG_ERROR("BOARD", "Initialization failed");
        return -1;
    } else {
        LOG_INFO("BOARD", "Initialization successful");
    }

//    Module_Start("IMU_INT",
//                 IMU_INT_Init,
//                 IMU_INT_Update,
//                 2,
//                 10);

    Module_Start("BARO_INT",
                 BARO_INT_Init,
                 BARO_INT_Update,
                 11,
                 10);

    Module_Start("IMU_EXT",
                 IMU_EXT_Init,
                 IMU_EXT_Update,
                 2,
                 10);

    Module_Start("IO",
                 IO_Init,
                 IO_Update,
                 4,
                 9);

    Module_Start("Uptime",
                 Uptime_Init,
                 Uptime_Update,
                 60000,
                 1);

//    Module_Start("Monitor",
//                 Monitor,
//                 Monitor_Update,
//                 3000,
//                 1);

    return 0;
}
