#include "rm.h"

#include <dirent.h>
#include <errno.h>

int rm_commander(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: rm path [path_1] ...\n");
    } else if (argc == 2 && argv[1][0] == '*') {
        char buf[MAX_NAME_LEN];
        getcwd(buf, MAX_NAME_LEN);
        DIR *dir = opendir(buf);
        dirent_t *d;
        while ((d = readdir(dir)) != NULL) {
            remove(d->d_name);
        }
        closedir(dir);
    } else {
        for (int i = 1; i < argc; ++i) {
            if (remove(argv[i])) {
                printf("rm: cannot remove '%s': %s\n", argv[i], strerror(errno));
            }
        }
    }
    return 0;
}
