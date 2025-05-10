#include "ls.h"

#include <dirent.h>
#include <sys/stat.h>

static void ls_print_help(void) {
    printf("Usage: ls [path_to_dir]\n");
}

void ls_print_dir_content(const char *path) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        struct stat st;
        if (stat(path, &st) < 0) {
            printf("%s\n", strerror(errno));
            return;
        }
        printf("%-10u\t%s\n", st.st_size, path);
    } else {
        int loop = -1;
        while (loop) {
            dirent_t *d = readdir(dir);
            if (d != NULL) {
                printf("%-10u\t%s\n", d->size, d->d_name);
            } else {
                loop = 0;
            }
        }
        closedir(dir);
    }
}

int ls_commander(int argc, char **argv) {
    if (argc > 2) {
        ls_print_help();
    } else if (argc == 2) {
        ls_print_dir_content(argv[1]);
    } else {
        char buf[MAX_NAME_LEN];
        ls_print_dir_content(getcwd(buf, MAX_NAME_LEN));
    }

    return 0;
}
