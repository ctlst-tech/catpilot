#include "devices.h"
#include "module.h"
#include "init.h"
#include "log.h"

int Devices_Init(void) {
    int rv;

    rv = Board_Init();

    if(rv) {
        LOG_ERROR("BOARD", "Initialization failed");
        return -1;
    } else {
        LOG_INFO("BOARD", "Initialization successful");
    }

    Module_Start("IMU_INT",
                 IMU_INT_Init,
                 IMU_INT_Update,
                 2,
                 10);

    Module_Start("IMU_EXT",
                 IMU_EXT_Init,
                 IMU_EXT_Update,
                 2,
                 10);

    Module_Start("IO",
                 IO_Init,
                 IO_Update,
                 5,
                 10);

    return 0;
}

int CLI(void) {
    if(USART2_Init()) return -1;
    CLI_Init(&usart2);
    WELCOME();
    return 0;
}
