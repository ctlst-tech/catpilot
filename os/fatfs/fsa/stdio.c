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
            break;
        }
    }
    return rv;
}

int fd_delete(int fd) {
    if (fd > MAX_FILES && fd < 0) {
        return -1;
    }
    files[fd] = NULL;
    return 0;
}

int fd_check(int fd) {
    if (fd < 0 || fd > MAX_FILES) {
        errno = EBADF;
        return -1;
    }
    return 0;
}

int open(const char *pathname, int flags) {
    int fd, rv;
    struct node *node = node_find(pathname, NODE_MODE_NEAREST_PATH);
    errno = 0;

    if (node == NULL || node->f_op.open == NULL) {
        errno = ENOENT;
        return -1;
    }

    fd = fd_new();

    if (fd_check(fd)) {
        errno = ENOENT;
        return -1;
    }

    files[fd] = &__files[fd];
    files[fd]->node = node;
    files[fd]->flags = flags;

    char *relative_pathname = strstr(pathname, node->name) + strlen(node->name);
    while ((*relative_pathname) == '/' && (*relative_pathname) != '\0') {
        relative_pathname++;
    }
    rv = files[fd]->node->f_op.open(files[fd], relative_pathname);

    if (rv) {
        if (errno == 0) {
            errno = EIO;
        }
        fd_delete(fd);
        rv = -1;
    } else {
        errno = 0;
        rv = fd;
    }

    return rv;
}

ssize_t write(int fd, const void *buf, size_t count) {
    int rv;
    errno = 0;

    if (fd_check(fd)) {
        return -1;
    }

    if (files[fd] == NULL || files[fd]->node->f_op.write == NULL) {
        errno = ENOENT;
        return -1;
    }

    rv = files[fd]->node->f_op.write(files[fd], buf, count);

    return rv;
}

ssize_t read(int fd, void *buf, size_t count) {
    int rv;
    errno = 0;

    if (fd_check(fd)) {
        return -1;
    }

    if (files[fd] == NULL || files[fd]->node->f_op.read == NULL) {
        errno = ENOENT;
        return -1;
    }

    rv = files[fd]->node->f_op.read(files[fd], buf, count);

    return rv;
}

int close(int fd) {
    int rv;
    errno = 0;

    if (fd_check(fd)) {
        return -1;
    }

    if (files[fd] == NULL || files[fd]->node->f_op.close == NULL) {
        errno = ENOENT;
        return -1;
    }

    rv = files[fd]->node->f_op.close(files[fd]);

    if (rv) {
        return rv;
    }

    fd_delete(fd);

    return rv;
}

int fileno(FILE *stream) {
    int fd = -1;

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i] == stream) {
            fd = i;
            break;
        }
    }

    if (fd_check(fd)) {
        return -1;
    }

    return fd;
}

int isatty(int fd) {
    if (fd > 0 && fd < 3) {
        return 1;
    }
    return 0;
}

#define modecmp(str, pat) (strcmp(str, pat) == 0 ? 1 : 0)

FILE *fopen(const char *filename, const char *mode) {
    int flag;
    errno = 0;

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

    if (fd_check(fd)) {
        return NULL;
    }

    files[fd]->flags &= ~__SERR;

    return files[fd];
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    int fd;
    errno = 0;
    size_t count;
    ssize_t count_s;

    if (stream == NULL) {
        errno = ENOENT;
        return -1;
    }

    if ((fd = fileno(stream)) < 0) {
        return 0;
    }

    if (!(stream->flags & O_RDWR || stream->flags & O_RDONLY)) {
        errno = EACCES;
        return -1;
    }

    if ((count_s = read(fd, ptr, size * nmemb)) < 0) {
        return 0;
    }

    count = count_s;

    return count;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    int fd;
    errno = 0;
    size_t count;
    ssize_t count_s;

    if (stream == NULL) {
        errno = ENOENT;
        return -1;
    }

    if ((fd = fileno(stream)) < 0) {
        return 0;
    }

    if (!(stream->flags & O_RDWR || stream->flags & O_WRONLY)) {
        errno = EACCES;
        return -1;
    }

    if ((count_s = write(fd, ptr, size * nmemb)) < 0) {
        return 0;
    }

    count = count_s;

    return count;
}

int fclose(FILE *stream) {
    int rv;
    errno = 0;

    if (stream == NULL) {
        errno = ENOENT;
        return -1;
    }

    rv = stream->node->f_op.close(stream);

    if (rv) {
        stream->flags |= __SERR;
        return rv;
    }

    int fd = fileno(stream);

    return close(fd);
}

void sync(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i] == NULL) {
            continue;
        }
        if (files[i]->node->f_op.fsync != NULL) {
            if (files[i]->node->f_op.fsync(files[i])) {
                files[i]->flags |= __SERR;
            }
        }
    }
}

int fsync(int fileno) {
    FILE *stream;
    errno = 0;

    if (fd_check(fileno)) {
        return -1;
    }

    if (fileno > MAX_FILES) {
        errno = EINVAL;
        return -1;
    }

    if (files[fileno] == NULL || files[fileno]->node->f_op.fsync == NULL) {
        errno = ENOENT;
        return -1;
    }

    stream = files[fileno];

    if (stream->node->f_op.fsync(files[fileno])) {
        stream->flags |= __SERR;
        return -1;
    }

    return 0;
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

void clearerr(FILE *stream) {
    stream->flags &= ~__SEOF;
    stream->flags &= ~__SERR;
}

int ferror(FILE *stream) {
    if (stream->flags & __SERR) {
        return 1;
    }
    return 0;
}

int std_stream_init(const char *stream, void *dev,
                    int (*dev_open)(struct file *file, const char *path),
                    ssize_t (*dev_write)(struct file *file, const char *buf,
                                         size_t count),
                    ssize_t (*dev_read)(struct file *file, char *buf,
                                        size_t count)) {
    int fd;
    char stream_name[16];
    char stream_path[16];
    struct file_operations f_op = {0};

    if (stream == NULL || dev == NULL || dev_open == NULL ||
        dev_write == NULL || dev_read == NULL) {
        return -1;
    }

    f_op.open = dev_open;
    f_op.dev = dev;

    if (!strcmp(stream, "stdin")) {
        f_op.read = dev_read;
    } else if (!strcmp(stream, "stdout") || !strcmp(stream, "stderr")) {
        f_op.write = dev_write;
    } else {
        return -1;
    }

    strncpy(stream_name, stream, sizeof(stream_name) - 1);
    snprintf(stream_path, sizeof(stream_path), "/dev/%s", stream_name);

    if (node_mount(stream_path, &f_op) == NULL) {
        return -1;
    }

    fd = open((const char *)stream_path, O_RDONLY);

    if (fd < 0) {
        return -1;
    }

    return 0;
}

int mkdir(const char *pathname, mode_t mode) {
    int fd, rv;

    struct node *node = node_find(pathname, NODE_MODE_NEAREST_PATH);
    errno = 0;

    if (node == NULL || node->f_op.mkdir == NULL) {
        errno = ENOENT;
        return -1;
    }

    char *relative_pathname = strstr(pathname, node->name) + strlen(node->name);
    while ((*relative_pathname) == '/' && (*relative_pathname) != '\0') {
        relative_pathname++;
    }
    rv = node->f_op.mkdir(relative_pathname, mode);

    return rv;
}

int rmdir(const char *pathname) {
    int fd, rv;

    struct node *node = node_find(pathname, NODE_MODE_NEAREST_PATH);
    errno = 0;

    if (node == NULL || node->f_op.rmdir == NULL) {
        errno = ENOENT;
        return -1;
    }

    char *relative_pathname = strstr(pathname, node->name) + strlen(node->name);
    while ((*relative_pathname) == '/' && (*relative_pathname) != '\0') {
        relative_pathname++;
    }

    rv = node->f_op.rmdir(relative_pathname);

    return rv;
}

extern struct node null_node;

int std_stream_deinit(const char *stream) {
    int fd;
    if (!strcmp(stream, "stdin")) {
        fd = 0;
    } else if (!strcmp(stream, "stdout")) {
        fd = 1;
    } else if (!strcmp(stream, "stderr")) {
        fd = 2;
    } else {
        return -1;
    }

    if (files[fd] == NULL) {
        return -1;
    }

    files[fd]->node = &null_node;
    return 0;
}
