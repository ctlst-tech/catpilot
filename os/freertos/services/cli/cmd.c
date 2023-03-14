#include "cli.h"

extern int log_print(int argc, char **argv);

extern void board_reset(void);
extern int board_get_app_status(void);
extern void board_run_app(void);

const char logo[] = "\003\014\n"
    "               __        _ __      __         |\\---/|                 \n"
    "   _________ _/ /_____  (_) /___  / /_        | ,_, |                 \n"
    "  / ___/ __ `/ __/ __ \\/ / / __ \\/ __/         \\_`_/-..----.          \n"
    " / /__/ /_/ / /_/ /_/ / / / /_/ / /_        ___/ `   ' ,\"\"+ \\         \n"
    " \\___/\\__,_/\\__/ .___/_/_/\\____/\\__/       (__...'   __\\    |`.___.'; \n"
    "              /_/                             (_,...'(_,.`__)/'.....+  \n"
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

void system_help(void) {
    printf(
        "Usage: system [options]\n"
        "Options:\n"
        "\tstart\tRun application\n"
        "\treset\tReset board\n");
}

int system_commander(int argc, char **argv) {
    if (argc != 2) {
        system_help();
        return 0;
    }
    if (!strncmp("start", argv[1], MAX_NAME_LEN)) {
        if (board_get_app_status()) {
            printf("Application is already running\n");
        } else {
            board_run_app();
            printf("Application launched\n");
        }
    } else if (!strncmp("reset", argv[1], MAX_NAME_LEN)) {
        board_reset();
    } else {
        system_help();
        return 0;
    }
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
    if (cli_cmd_reg("system", system_commander) == NULL) {
        return -1;
    }
    return 0;
}
