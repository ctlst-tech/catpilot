#include <dirent.h>
#include <errno.h>
#include <fatfs.h>
#include <stdio.h>
#include <string.h>

static DIR _dir;
static dirent_t _de;

DIR *opendir(const char *name) {
    int res = f_opendir(&_dir, name);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return NULL;
    }

    return &_dir;
}

dirent_t *readdir(DIR *dirp) {
    FILINFO fno;
    int len;
    int res;
    errno = 0;

    _de.d_name[0] = 0;
    res = f_readdir(dirp, &fno);
    if (res != FR_OK || fno.fname[0] == 0) {
        errno = fatfs_to_errno(res);
        return NULL;
    }

    len = strlen(fno.fname);
    strncpy(_de.d_name, fno.fname, MAX_NAME_LEN);
    _de.d_name[len] = 0;
    _de.size = fno.fsize;

    return ((dirent_t *)&_de);
}

int closedir(DIR *dirp) {
    int res = f_closedir(dirp);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }

    return 0;
}

int chdir(const char *path) {
    return fatfs_chdir(path);
}

char *getcwd(char *buf, size_t size) {
    return fatfs_getcwd(buf, size);
}

int stat(const char *path, struct stat *stat) {
    return fatfs_stat(path, stat);
}

int remove(const char *path) {
    return fatfs_remove(path);
}
