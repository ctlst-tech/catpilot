#include <stdio.h>

#include "ff.h"

FIL *stream_to_fatfs(FILE *stream);
int fatfs_to_errno(FRESULT result);
int fatfs_open(struct file *file, const char *path);
ssize_t fatfs_write(struct file *file, const char *buf, size_t count);
ssize_t fatfs_read(struct file *file, char *buf, size_t count);
int fatfs_close(struct file *file);
int fatfs_syncfs(struct file *file);
int64_t fatfs_getfree(void);
int64_t fatfs_gettotal(void);
int fatfs_mkdir(const char *pathname, mode_t mode);
char *fatfs_dirname(char *str);
char *fatfs_getcwd(char *pathname, int len);
int fatfs_rename(const char *oldpath, const char *newpath);
int fatfs_rmdir(const char *pathname);
dirent_t *fatfs_readdir(DIR *dirp);
