#include "cli.h"

char git_hash[32];
char git_state[32];

extern int log_print(int argc, char **argv);

char logo[] = "\003\014" 
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
    "                                           \n";


int help(int argc, char **argv) {
    cli_cmd_print();
    return 0;
}

int version(int argc, char **argv) {
    printf("Commit hash: %s\n", git_hash);
    printf("Commit state: %s\n", git_state);
    return 0;
}

int clear(int argc, char **argv) {
    printf("\003\014\n");
    return 0;
}

int cli_cmd_init(char *hash, char *state) {
    printf("%s", logo);
    strncpy(git_hash, hash, 32);
    strncpy(git_state, state, 32);
    if(cli_cmd_reg("help", help) == NULL) {
        return -1;
    }
    if(cli_cmd_reg("version", version) == NULL) {
        return -1;
    }
    if(cli_cmd_reg("clear", clear) == NULL) {
        return -1;
    }
    if(cli_cmd_reg("log", log_print) == NULL) {
        return -1;
    }
    return 0;
}
