#include "devices.h"
#include "module.h"
#include "init.h"
#include "log.h"

int Devices_Init(void) {
    int rv;

    rv = Board_Init();
    CLI_Init(&usart7);

    WELCOME();

    if(rv) {
        LOG_ERROR("GPIO", "Initialization failed");
    } else {
        LOG_INFO("GPIO", "Initialization successful");
    }

    Module_Start("IMU",
                 IMU_Init,
                 IMU_Update,
                 2 * portTICK_PERIOD_MS,
                 10);

    // Module_Start("MAG",
    //              MAG_Init,
    //              MAG_Update,
    //              20 * portTICK_PERIOD_MS,
    //              10);

    Module_Start("IO",
                 IO_Init,
                 IO_Update,
                 5 * portTICK_PERIOD_MS,
                 10);

    return 0;
}
