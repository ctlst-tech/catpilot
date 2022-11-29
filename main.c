#include <pthread.h>

#include "board.h"
#include "swsys.h"
#include "xml_inline.h"

void *catpilot(void *param);
swsys_t core_sys;

int main(void) {
    pthread_t tid;
    pthread_attr_t attr;
    int arg = 0;

    board_start();

    while (1) {
    }
}

void *catpilot(void *param) {
    pthread_setname_np((char *)__func__);

    board_cli_init();
    board_periph_init();
    board_fs_init();
    board_services_start();

    xml_inline_mount("/cfg");

    swsys_rv_t swsys_rv = swsys_load("/cfg/mvp_swsys.xml", "/cfg/", &core_sys);
    if (swsys_rv == swsys_e_ok) {
        LOG_INFO("SYSTEM", "Configuration loading successful");
        swsys_rv = swsys_top_module_start(&core_sys);
        if (swsys_rv != swsys_e_ok) {
            LOG_ERROR("SYSTEM", "Module start error");
        }
    } else {
        LOG_ERROR("SYSTEM", "Configuration loading error");
    }

    board_fail();

    return NULL;
}
