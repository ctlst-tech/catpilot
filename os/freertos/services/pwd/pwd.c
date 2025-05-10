#include "ls.h"

#include <dirent.h>
#include <sys/stat.h>

int pwd_commander(int argc, char **argv) {
    char buf[MAX_NAME_LEN];
    getcwd(buf, MAX_NAME_LEN);
    printf("%s\n", buf);
    return 0;
}
