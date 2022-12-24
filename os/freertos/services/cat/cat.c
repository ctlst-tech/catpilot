#include "cat.h"

static char buffer[1024];

static void cat_print_help(void) {
    printf("Usage: cat [path_to_file]\n");
}

static void cat_print_file_content(const char *path) {
    int fd = open(path, O_WRONLY);

    if (fd < 0) {
        printf("%s\n", strerror(errno));
        return;
    }

    read(fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

int cat_commander(int argc, char **argv) {
    int optind = 1;

    if (argc != 2) {
        cat_print_help();
    } else {
        cat_print_file_content((const char *)argv[1]);
    }

    return 0;
}
