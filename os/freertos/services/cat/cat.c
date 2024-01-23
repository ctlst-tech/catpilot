#include "cat.h"

#define CLI_CAT_MAX_LEGNTH 1024

static char buf[CLI_CAT_MAX_LEGNTH + 1];

static void cat_print_help(void) {
    printf("Usage: cat [path_to_file]\n");
}

static void cat_print_file_content(const char *path) {
    char cur_path[4 * MAX_NAME_LEN];
    getcwd(cur_path, 4 * MAX_NAME_LEN);
    char path_ext[4 * MAX_NAME_LEN];

    snprintf(path_ext, 4 * MAX_NAME_LEN, "/%s%s/%s", "fs", cur_path, path);
    chdir("/");

    int fd = open(path_ext, O_RDONLY);
    chdir(cur_path);

    if (fd < 0) {
        printf("%s\n", strerror(errno));
        return;
    }

    int rb = 0;
    lseek(fd, 0, SEEK_SET);
    do {
        rb = read(fd, buf, sizeof(buf));
        if (rb > 0) {
            write(1, buf, rb);
        } else {
            printf("%s\n", strerror(errno));
        }
    } while ((size_t)rb == sizeof(buf));
    lseek(fd, 0, SEEK_END);
    close(fd);
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
