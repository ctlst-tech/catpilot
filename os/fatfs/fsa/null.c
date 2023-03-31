#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "node.h"

int null_open(struct file *file, const char *path) {
    return 0;
}

ssize_t null_write(struct file *file, const char *buf, size_t count) {
    return count;
}

ssize_t null_read(struct file *file, char *buf, size_t count) {
    sleep(10);
    return count;
}

int null_close(struct file *file) {
    return 0;
}

int null_ioctl(struct file *file, int request, va_list args) {
    return 0;
}

int null_flush(struct file *file) {
    return 0;
}

int null_fsync(struct file *file) {
    return 0;
}

struct node null_node = {
    .name = "null",
    .f_op =
        {
            .open = null_open,
            .write = null_write,
            .read = null_read,
            .close = null_close,
            .flush = null_flush,
            .ioctl = null_ioctl,
            .fsync = null_fsync,
            .dev = NULL,
        },
    .parent = NULL,
    .sibling = NULL,
    .child = NULL,
};
