#include <stdio.h>

int fatfs_open(void *devcfg, void *file, const char *pathname, int flags);
ssize_t fatfs_write(void *devcfg, void *file, const void *buf, size_t count);
ssize_t fatfs_read(void *devcfg, void *file, void *buf, size_t count);
int fatfs_close(void *devcfg, void *file);
int fatfs_syncfs(void *devcfg, void *file);
void *fatfs_filealloc(void);
