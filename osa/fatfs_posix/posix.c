#include <stdio.h>
#include <string.h>
#include "fatfs_posix.h"
#include "dev_posix.h"

struct dev __dev[MAX_DEVICES];
char *__dev_table[MAX_DEVICES];

int open(const char *pathname, int flags) {
    int rv;
    for(int i = 0; i < MAX_DEVICES; i++) {
        if(strcmp(pathname, __dev[i].path)) {
            continue;
        }
        if(!strcmp(__dev_table[i], __dev[i].path)) {
            return (i + 3);
        }
        __dev_table[i] = __dev[i].path;
        if(__dev[i].open == NULL) {
            errno = EBADF;
            return -1;
        }
        rv = __dev[i].open(pathname, flags);
        if(rv) {
            errno = EPROTO;
            return -1;
        }
        rv = fatfs_open(pathname, O_RDWR);
        if(rv < 0) return -1;
        return (i + 3);
    }
    rv = fatfs_open(pathname, flags);
    return rv;
}

ssize_t write(int fd, const void *buf, size_t count) {
    int rv;
    int dev_fd = fd - 3;
    if(!strcmp(__dev_table[dev_fd], __dev[dev_fd].path)) {
        if(__dev[dev_fd].write == NULL) {
            errno = EBADF;
            return -1;
        }
        rv = __dev[dev_fd].write(dev_fd, buf, count);
        if(rv) {
            errno = EPROTO;
            return -1;
        }
        return count;
    }
    rv = fatfs_write(fd, buf, count);
    return rv;
}

ssize_t read(int fd, void *buf, size_t count) {
    int rv;
    int dev_fd = fd - 3;
    if(!strcmp(__dev_table[dev_fd], __dev[dev_fd].path)) {
        if(__dev[dev_fd].read == NULL) {
            errno = EBADF;
            return -1;
        }
        rv = __dev[dev_fd].read(dev_fd, buf, count);
        if(rv) {
            errno = EPROTO;
            return -1;
        }
        return count;
    }
    rv = fatfs_read(fd, buf, count);
    return rv;
}

int close(int fd) {
    int rv;
    int dev_fd = fd - 3;
    if(!strcmp(__dev_table[dev_fd], __dev[dev_fd].path)) {
        if(__dev[dev_fd].close == NULL) {
            errno = EBADF;
            return -1;
        }
        rv = __dev[dev_fd].close(dev_fd);
        if(rv) {
            errno = EPROTO;
            return -1;
        }
        // rv = fatfs_close(fd); don't delete dev files
        return 0;
    }
    rv = fatfs_close(fd);
    return rv;
}
