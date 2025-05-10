#include "cd.h"

#include <dirent.h>
#include <sys/unistd.h>

static void cd_print_help(void) {
    printf("Usage: cd [path_to_dir]\n");
}

void cd_print_dir_content(const char *path) {
    int rv = chdir(path);
    if (rv < 0) {
        printf("%s\n", strerror(errno));
    }
}

int cd_commander(int argc, char **argv) {
    if (argc > 2) {
        cd_print_help();
    } else if (argc == 2) {
        cd_print_dir_content(argv[1]);
    }

    return 0;
}
