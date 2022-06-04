#include <stdio.h>

int fatfs_open(void *devcfg, void *filecfg, const char* pathname, int flags);
ssize_t fatfs_write(void *devcfg, void *filecfg, const void *buf, size_t count);
ssize_t fatfs_read(void *devcfg, void *filecfg, void *buf, size_t count);
int fatfs_close(void *devcfg, void *filecfg);
