#pragma once
#include <stdio.h>
#include <string.h>

#define MAX_DEVICES 2
#define MAX_DEVICE_PATH_LENGTH 32

struct dev {
    char path[MAX_DEVICE_PATH_LENGTH];
    int (*open)(const char *pathname, int flags);
    ssize_t (*write)(int fd, const void *buf, size_t count);
    ssize_t (*read)(int fd, const void *buf, size_t count);
    int (*close)(int fd);
};

extern struct dev __dev[MAX_DEVICES];
