#include "cli.h"

extern int log_print(int argc, char **argv);

char logo[] = "\003\014\n"
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
    printf("uas-catpilot %s%s\n", GIT_HASH_UAS, GIT_STATE_UAS);
    printf("catpilot %s%s\n", GIT_HASH_CATPILOT, GIT_STATE_CATPILOT);
    return 0;
}

int clear(int argc, char **argv) {
    printf("\003\014\n");
    return 0;
}

int cli_cmd_init(void) {
    printf("%s", logo);

    if (cli_cmd_reg("help", help) == NULL) {
        return -1;
    }
    if (cli_cmd_reg("version", version) == NULL) {
        return -1;
    }
    if (cli_cmd_reg("clear", clear) == NULL) {
        return -1;
    }
    if (cli_cmd_reg("log", log_print) == NULL) {
        return -1;
    }
    return 0;
}
