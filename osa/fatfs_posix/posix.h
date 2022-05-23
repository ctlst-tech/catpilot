#include <stdio.h>

int fatfs_open(const char *pathname, int flags);
ssize_t fatfs_write(int fd, const void *buf, size_t count);
ssize_t fatfs_read(int fd, void *buf, size_t count);
int fatfs_close(int fileno);
