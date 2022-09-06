#include "devices.h"
#include "module.h"
#include "init.h"
#include "log.h"

int Devices_Init(void) {
    int rv;

    if(!USART7_Init()) {
        CLI_Init(&usart7);
    }

    WELCOME();

    rv = Board_Init();

    if(rv) {
        LOG_ERROR("BOARD", "Initialization failed");
        return -1;
    } else {
        LOG_INFO("BOARD", "Initialization successful");
    }

    Module_Start("IMU",
                 IMU_Init,
                 IMU_Update,
                 2 * portTICK_PERIOD_MS,
                 10);

    Module_Start("IO",
                 IO_Init,
                 IO_Update,
                 5 * portTICK_PERIOD_MS,
                 10);

    return 0;
}
