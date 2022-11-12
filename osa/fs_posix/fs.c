#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "node.h"

FILE __files[MAX_FILES];
FILE *files[MAX_FILES];
static int __fd;

int fd_new(void) {
    int rv = -1;
    errno = ENOMEM;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i] == NULL) {
            rv = i;
            errno = 0;
            break;
        }
    }
    return rv;
}

int fd_delete(int fd) {
    if (fd > MAX_FILES) {
        return -1;
    }
    files[fd] = NULL;
    return 0;
}

int open(const char *pathname, int flags) {
    int fd, rv;
    struct node *node = node_find(pathname, NODE_MODE_NEAREST_PATH);

    if (node == NULL || node->f_op.open == NULL) {
        errno = ENOENT;
        return -1;
    }

    fd = fd_new();

    if (fd < 0) {
        return fd;
    }

    files[fd] = &__files[fd];
    files[fd]->node = node;
    files[fd]->f_flags = flags;

    rv = files[fd]->node->f_op.open(files[fd], pathname);
    if (rv) {
        errno = EIO;
        fd_delete(fd);
    } else {
        errno = 0;
        rv = fd;
    }
    return rv;
}

ssize_t write(int fd, const void *buf, size_t count) {
    int rv;
    if (files[fd] == NULL || files[fd]->node->f_op.write == NULL) {
        errno = ENOENT;
        return -1;
    }

    rv = files[fd]->node->f_op.write(files[fd], buf, count);
    return rv;
}

ssize_t read(int fd, void *buf, size_t count) {
    int rv;
    if (files[fd] == NULL || files[fd]->node->f_op.read == NULL) {
        errno = ENOENT;
        return -1;
    }

    rv = files[fd]->node->f_op.read(files[fd], buf, count);
    return rv;
}

int close(int fd) {
    int rv;
    if (files[fd] == NULL || files[fd]->node->f_op.close == NULL) {
        errno = ENOENT;
        return -1;
    }

    rv = files[fd]->node->f_op.close(files[fd]);
    fd_delete(fd);
    return rv;
}

#define modecmp(str, pat) (strcmp(str, pat) == 0 ? 1 : 0)

FILE *fopen(const char *filename, const char *mode) {
    int flag;
    if (modecmp(mode, "r") || modecmp(mode, "rb")) {
        flag = O_RDONLY;
    }
    if (modecmp(mode, "r+") || modecmp(mode, "r+b") || modecmp(mode, "rb+")) {
        flag = O_RDWR | O_TRUNC;
    }
    if (modecmp(mode, "w") || modecmp(mode, "wb")) {
        flag = O_WRONLY | O_CREAT | O_TRUNC;
    }
    if (modecmp(mode, "w+") || modecmp(mode, "w+b") || modecmp(mode, "wb+")) {
        flag = O_RDWR | O_CREAT | O_TRUNC;
    }
    if (modecmp(mode, "a") || modecmp(mode, "ab")) {
        flag = O_WRONLY | O_CREAT | O_APPEND;
    }
    if (modecmp(mode, "a+") || modecmp(mode, "a+b") || modecmp(mode, "ab+")) {
        flag = O_RDWR | O_CREAT | O_APPEND;
    }

    int fd = open(filename, flag);

    if (fd > 0) {
        return files[fd];
    } else {
        return NULL;
    }
}

int fclose(FILE *stream) {
    if(stream == NULL) {
        errno = ENOENT;
        return -1;
    }
    stream->node->f_op.close(stream);
    return 0;
}
