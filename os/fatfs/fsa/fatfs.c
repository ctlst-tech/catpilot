#include "fatfs.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ff.h"

#undef strerror_r

#define FATFS_SEEK_SET 0
#define FATFS_SEEK_CUR 1
#define FATFS_SEEK_END 2

#define FATFS_MAX_FILES 32

FIL fatfs_files[FATFS_MAX_FILES];
int fatfs_index = 0;

FIL *stream_to_fatfs(FILE *stream) {
    if (stream == NULL || stream->private_data == NULL) {
        return NULL;
    }
    return stream->private_data;
}

int fatfs_to_errno(FRESULT result) {
    switch (result) {
        case FR_OK:       /* FatFS (0) Succeeded */
            return (0);   /* POSIX OK */
        case FR_DISK_ERR: /* FatFS (1) A hard error occurred in the low level
                             disk I/O layer */
            return (EIO); /* POSIX Input/output error (POSIX.1) */

        case FR_INT_ERR:    /* FatFS (2) Assertion failed */
            return (EPERM); /* POSIX Operation not permitted (POSIX.1) */

        case FR_NOT_READY:  /* FatFS (3) The physical drive cannot work */
            return (EBUSY); /* POSIX Device or resource busy (POSIX.1) */

        case FR_NO_FILE:     /* FatFS (4) Could not find the file */
            return (ENOENT); /* POSIX No such file or directory (POSIX.1) */

        case FR_NO_PATH:     /* FatFS (5) Could not find the path */
            return (ENOENT); /* POSIX No such file or directory (POSIX.1) */

        case FR_INVALID_NAME: /* FatFS (6) The path name format is invalid */
            return (EINVAL);  /* POSIX Invalid argument (POSIX.1) */

        case FR_DENIED: /* FatFS (7) Access denied due to prohibited access or
                           directory full */
            return (EACCES); /* POSIX Permission denied (POSIX.1) */

        case FR_EXIST:       /* file exists */
            return (EEXIST); /* file exists */

        case FR_INVALID_OBJECT: /* FatFS (9) The file/directory object is
                                   invalid */
            return (EINVAL);    /* POSIX Invalid argument (POSIX.1) */

        case FR_WRITE_PROTECTED: /* FatFS (10) The physical drive is write
                                    protected */
            return (EROFS);      /* POSIX Read-only filesystem (POSIX.1) */

        case FR_INVALID_DRIVE: /* FatFS (11) The logical drive number is invalid
                                */
            return (ENXIO);    /* POSIX No such device or address (POSIX.1) */

        case FR_NOT_ENABLED: /* FatFS (12) The volume has no work area */
            return (ENOSPC); /* POSIX No space left on device (POSIX.1) */

        case FR_NO_FILESYSTEM: /* FatFS (13) There is no valid FAT volume */
            return (ENXIO);    /* POSIX No such device or address (POSIX.1) */

        case FR_MKFS_ABORTED: /* FatFS (14) The f_mkfs(void) aborted due to any
                                 parameter error */
            return (EINVAL);  /* POSIX Invalid argument (POSIX.1) */

        case FR_TIMEOUT:    /* FatFS (15) Could not get a grant to access the
                               volume within defined period */
            return (EBUSY); /* POSIX Device or resource busy (POSIX.1) */

        case FR_LOCKED: /* FatFS (16) The operation is rejected according to the
                           file sharing policy */
            return (EBUSY); /* POSIX Device or resource busy (POSIX.1) */

        case FR_NOT_ENOUGH_CORE: /* FatFS (17) LFN working buffer could not be
                                    allocated */
            return (ENOMEM);     /* POSIX Not enough space (POSIX.1) */

        case FR_TOO_MANY_OPEN_FILES: /* FatFS (18) Number of open files >
                                        _FS_SHARE */
            return (EMFILE);         /* POSIX Too many open files (POSIX.1) */

        case FR_INVALID_PARAMETER: /* FatFS (19) Given parameter is invalid */
            return (EINVAL);       /* POSIX Invalid argument (POSIX.1) */
    }
    return (EBADMSG); /* POSIX Bad message (POSIX.1) */
}

FILE *new_stream(void) {
    FIL *fh;
    FILE *stream;

    stream = (FILE *)calloc(sizeof(FILE), 1);

    if (stream == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    fh = (FIL *)calloc(sizeof(FIL), 1);

    if (fh == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    stream->private_data = (void *)fh;
    return stream;
}

int fatfs_open(struct file *file, const char *path) {
    int fatfs_modes;
    FIL *fh;
    int res;
    int rv = 0;
    errno = 0;

    if (fatfs_index >= FATFS_MAX_FILES - 1) {
        errno = ENOMEM;
        return -1;
    }

    int flags = file->flags;

    if ((flags & O_ACCMODE) == O_RDWR) {
        fatfs_modes = FA_READ | FA_WRITE;
    } else if ((flags & O_ACCMODE) == O_RDONLY) {
        fatfs_modes = FA_READ;
    } else {
        fatfs_modes = FA_WRITE;
    }

    if (flags & O_CREAT) {
        if (flags & O_TRUNC) {
            fatfs_modes |= FA_CREATE_ALWAYS;
        } else {
            fatfs_modes |= FA_CREATE_NEW;
        }
    }

    FIL tmp_file = {0};
    file->private_data = &tmp_file;
    fh = stream_to_fatfs(file);
    res = f_open(fh, path, (BYTE)(fatfs_modes & 0xff));

    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }

    if (flags & O_APPEND) {
        res = f_lseek(fh, f_size(fh));
        if (res != FR_OK) {
            errno = fatfs_to_errno(res);
            f_close(fh);
            return -1;
        }
    }

    if (!rv) {
        // file->private_data = (FIL *)calloc(sizeof(FIL), 1);
        // memcpy(file->private_data, &tmp_file, sizeof(FIL));
        fatfs_files[fatfs_index] = tmp_file;
        file->private_data = &fatfs_files[fatfs_index];
        fatfs_index++;
    }

    return (rv);
}

ssize_t fatfs_write(struct file *file, const char *buf, size_t count) {
    UINT size;
    FRESULT res;
    FIL *fh;
    errno = 0;

    if (file == NULL) {
        errno = EBADF;
        return -1;
    }

    fh = stream_to_fatfs(file);

    if (fh == NULL) {
        errno = EBADF;
        return -1;
    }

    res = f_write(fh, buf, (UINT)count, &size);

    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }
    if (size != count) {
        errno = fatfs_to_errno(res);
        return -1;
    }

    return ((ssize_t)size);
}

ssize_t fatfs_read(struct file *file, char *buf, size_t count) {
    UINT size;
    int res;
    int ret;
    FIL *fh;

    errno = 0;

    if (file == NULL) {
        errno = EBADF;
        return -1;
    }

    fh = stream_to_fatfs(file);
    if (fh == NULL) {
        errno = EBADF;
        return -1;
    }

    res = f_read(fh, buf, (UINT)count, &size);

    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }

    return ((ssize_t)size);
}

int fatfs_close(struct file *file) {
    int res;
    FIL *fh;

    errno = 0;

    if (file == NULL) {
        errno = EBADF;
        return -1;
    }

    fh = stream_to_fatfs(file);
    if (fh == NULL) {
        errno = EBADF;
        return -1;
    }

    res = f_close(fh);


    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }

    return 0;
}

int fatfs_syncfs(struct file *file) {
    FIL *fh;
    FRESULT res;

    errno = 0;

    if (file == NULL) {
        return -1;
    }

    file->flags |= __SUNGET;

    fh = stream_to_fatfs(file);

    if (fh == NULL) {
        errno = EBADF;
        return -1;
    }

    res = f_sync(fh);

    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }

    return 0;
}

int fatfs_lseek(struct file *file, off_t offset, int whence) {
    FIL *fh;
    FRESULT res;

    errno = 0;

    if (file == NULL) {
        return -1;
    }

    fh = stream_to_fatfs(file);

    if (fh == NULL) {
        errno = EBADF;
        return -1;
    }

    switch (whence) {
        case FATFS_SEEK_SET:
            break;

        case FATFS_SEEK_CUR:
            break;

        case FATFS_SEEK_END:
            offset = f_size(fh);
            break;

        default:
            break;
    }

    res = f_lseek(fh, offset);

    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }

    return 0;
}

int64_t fatfs_getfree(void) {
    FATFS *fs;
    DWORD fre_clust, fre_sect;

    /* Get volume information and free clusters of drive 1 */
    FRESULT res = f_getfree("/", &fre_clust, &fs);
    if (res) return (res);

    /* Get total sectors and free sectors */
    fre_sect = fre_clust * fs->csize;
    return (int64_t)(fre_sect)*512;
}

int64_t fatfs_gettotal(void) {
    FATFS *fs;
    DWORD fre_clust, tot_sect;

    /* Get volume information and free clusters of drive 1 */
    FRESULT res = f_getfree("/", &fre_clust, &fs);
    if (res) {
        return (res);
    }

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    return (int64_t)(tot_sect)*512;
}

int fatfs_mkdir(const char *pathname, mode_t mode) {
    errno = 0;

    int res = f_mkdir(pathname);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }
    (void)mode;

    return 0;
}

char *fatfs_dirname(char *str) {
    int end = 0;
    int ind = 0;

    if (!str) {
        return 0;
    }

    while (*str) {
        if (*str == '/') end = ind;
        ++str;
        ++ind;
    }
    return &str[end];
}

char *fatfs_getcwd(char *pathname, int len) {
    int res;
    errno = 0;

    res = f_getcwd(pathname, len);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return NULL;
    }
    return pathname;
}

int fatfs_rename(const char *oldpath, const char *newpath) {
    errno = 0;

    int rc = f_rename(oldpath, newpath);

    if (rc) {
        errno = fatfs_to_errno(rc);
        return -1;
    }
    return 0;
}

int fatfs_rmdir(const char *pathname) {
    errno = 0;

    int res = f_unlink(pathname);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }
    return 0;
}

int fatfs_chdir(const char *path) {
    int res = f_chdir(path);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }
    return 0;
}

int fatfs_stat(const char *pathname, struct stat *stat) {
    FILINFO info;
    int res = f_stat(pathname, &info);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }
    stat->st_size = info.fsize;
    stat->st_mode = info.fattrib;
    // TODO: convert FATFS fdate and ftime to mtime;
    stat->st_mtime = 0;
    return 0;
}

int fatfs_remove(const char *pathname) {
    FILINFO info;
    int res = f_unlink(pathname);
    if (res != FR_OK) {
        errno = fatfs_to_errno(res);
        return -1;
    }
    return 0;
}
