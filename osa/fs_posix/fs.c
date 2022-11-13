#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "node.h"

FILE __files[MAX_FILES];
FILE *files[MAX_FILES];
static int __fd;

const char *sys_errlist[] = {"OK",
                             "Operation not permitted",
                             "No such file or directory",
                             "No such process",
                             "Interrupted system call",
                             "I/O error",
                             "No such device or address",
                             "Argument list too long",
                             "Exec format error",
                             "Bad file number",
                             "No child processes",
                             "Try again",
                             "Out of memory",
                             "Permission denied",
                             "Bad address",
                             "Block device required",
                             "Device or resource busy",
                             "File exists",
                             "Cross-device link",
                             "No such device",
                             "Not a directory",
                             "Is a directory",
                             "Invalid argument",
                             "File table overflow",
                             "Too many open files",
                             "Not a typewriter",
                             "Text file busy",
                             "File too large",
                             "No space left on device",
                             "Illegal seek",
                             "Read-only file system",
                             "Too many links",
                             "Broken pipe",
                             "Math argument out of domain of func",
                             "Math result not representable",
                             "Bad Message",
                             NULL};

int fd_new(void) {
    int rv = -1;
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
        errno = ENOMEM;
        return fd;
    }

    files[fd] = &__files[fd];
    files[fd]->node = node;
    files[fd]->flags = flags;

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
    if (stream == NULL) {
        errno = ENOENT;
        return -1;
    }

    stream->node->f_op.close(stream);

    return 0;
}

void sync(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i] == NULL) {
            continue;
        }
        files[i]->node->f_op.fsync(files[i]);
    }
}

int fsync(int fileno) {
    FILE *stream;

    errno = 0;

    if (fileno > MAX_FILES) {
        errno = EINVAL;
        return -1;
    }

    if (files[fileno]->node->f_op.fsync(files[fileno])) {
        return -1;
    }

    return 0;
}

void clearerr(FILE *stream) {
    stream->flags = 0;
}

char *strerror(int errnum) {
    return ((char *)sys_errlist[errnum]);
}

void perror(const char *s) {
    const char *ptr = NULL;

    if (errno >= 0 && errno < EBADMSG) {
        ptr = sys_errlist[errno];
    } else {
        ptr = sys_errlist[EBADMSG];
    }
    if (s && *s) {
        printf("%s: %s\n", s, ptr);
    } else {
        printf("%s\n", ptr);
    }
}

void clrerror(FILE *stream) {
    stream->flags &= ~__SEOF;
    stream->flags &= ~__SERR;
}

int ferror(FILE *stream) {
    if (stream->flags & __SERR) {
        return 1;
    }

    return 0;
}
