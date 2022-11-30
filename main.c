#include <pthread.h>

#include "board.h"
#include "swsys.h"
#include "xml_inline.h"

swsys_t core_sys;

int main(void) {
    board_start();
    while (1) {
    }
}

void *catpilot(void *param) {
    pthread_setname_np((char *)__func__);

    board_cli_init();

    printf("\n\n\n\n"
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFFFFFFFFFFFFFFFFFE       FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFFFFF#             EFF  FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFFF    EFFFFFFFF FFFF  FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFF  FF  FFFFFE    EF  FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFE FFE      4FFF FF  FFFFFFE       EFE        F  FFFFE       FFF         FFFF\n"
    "FFFF     EFF FE  FFFF  FFFFFFF  FFFFFFFFFFFF  FFFF  FFFFF   FFFFFFFFFF  FFFFFFFF\n"
    "FFFF   EFFFFFF   FFE  FFFFFFFF  FFFFFFFFFFFF  FFFF  FFFFFEEEEE   FFFFF  FFFFFFFF\n"
    "FFFFFFFFFFFF   FFFE  FFFFFFFFFF       FFFFFF  FFFFF     FF       FFFFF  FFFFFFFF\n"
    "FFFFFFFFFF   FFFFF  FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFFFFFF          FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFF\n"
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF\n"
    "               __        _ __      __      \n"
    "   _________ _/ /_____  (_) /___  / /_     \n"
    "  / ___/ __ `/ __/ __ \\/ / / __ \\/ __/   \n"
    " / /__/ /_/ / /_/ /_/ / / / /_/ / /_       \n"
    " \\___/\\__,_/\\__/ .___/_/_/\\____/\\__/  \n"
    "              /_/                          \n"
    "                                           \n");
    printf("Catalyst Aerospace Technologies\n");
    printf("CatPilot\n");
    printf("Commit hash:  "GIT_HASH"\n");
    printf("Commit state: "GIT_STATE"\n");
    printf("\n");

    board_periph_init();
    board_fs_init();
    board_services_start();

    xml_inline_mount("/cfg");

    swsys_rv_t swsys_rv = swsys_load("/cfg/mvp_swsys.xml", "/cfg", &core_sys);
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
