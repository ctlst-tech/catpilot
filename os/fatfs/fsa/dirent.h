#ifndef DIRENT
#define DIRENT

#include <stdio.h>
#include "ff.h"

struct dirent {
    char d_name[MAX_NAME_LEN];
    unsigned size;
};

typedef struct dirent dirent_t;

DIR *opendir(const char *name);
dirent_t *readdir(DIR *dirp);
int closedir(DIR *dirp);

#endif // DIRENT
