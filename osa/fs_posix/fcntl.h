/*
 * Amazon FreeRTOS POSIX V1.1.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file fcntl.h
 * @brief File control options.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/fcntl.h.html
 */

#ifndef _FREERTOS_POSIX_FCNTL_H_
#define _FREERTOS_POSIX_FCNTL_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name File creation flags for use in the oflag value to open() and openat().
 */
/**@{ */
#define O_CLOEXEC      0x0001 /**< Close the file descriptor upon exec(). */
#define O_CREAT        0x0002 /**< Create file if it does not exist. */
#define O_DIRECTORY    0x0004 /**< Fail if file is a non-directory file. */
#define O_EXCL         0x0008 /**< Exclusive use flag. */
#define O_NOCTTY       0x0010 /**< Do not assign controlling terminal. */
#define O_NOFOLLOW     0x0020 /**< Do not follow symbolic links. */
#define O_TRUNC        0x0040 /**< Truncate flag. */
#define O_TTY_INIT     0x0080 /**< termios structure provides conforming behavior. */
/**@} */

/**
 * @name File status flags for open(), openat(), and fcntl().
 */
/**@{ */
#define O_APPEND      0x0100 /**< Set append mode. */
#define O_DSYNC       0x0200 /**< Write according to synchronized I/O data integrity completion. */
#define O_NONBLOCK    0x0400 /**< Non-blocking mode. */
#define O_RSYNC       0x0800 /**< Synchronized read I/O operations. */
#define O_SYNC        0x0200 /**< Write according to synchronized I/O file integrity completion. */
/**@} */

/**
 * @name Mask for file access modes.
 */
/**@{ */
#define O_ACCMODE    0xF000
/**@} */

/**
 * @name File access modes for open(), openat(), and fcntl().
 */
/**@{ */
#define O_EXEC      0x1000  /**< Open for execute only (non-directory files). */
#define O_RDONLY    0x2000  /**< Open for reading only. */
#define O_RDWR      0xA000  /**< Open for reading and writing. */
#define O_SEARCH    0x4000  /**< Open directory for search only. */
#define O_WRONLY    0x8000  /**< Open for writing only. */
/**@} */

/**
 * @name Group and users
 */
/**@{ */
#define S_IREAD    0400                         /*< Read by owner.  */
#define S_IWRITE   0200                         /*< Write by owner.  */
#define S_IEXEC    0100                         /*< Execute by owner.  */

#define S_IRUSR S_IREAD                         /*< Read by owner.  */
#define S_IWUSR S_IWRITE                        /*< Write by owner.  */
#define S_IXUSR S_IEXEC                         /*< Execute by owner.  */
#define S_IRWXU (S_IREAD|S_IWRITE|S_IEXEC)      /*< Read,Write,Execute by owner */

#define FATFS_R (S_IRUSR | S_IRGRP | S_IROTH)   /*< FatFs Read perms */
#define FATFS_W (S_IWUSR | S_IWGRP | S_IWOTH)   /*< FatFs Write perms */
#define FATFS_X (S_IXUSR | S_IXGRP | S_IXOTH)   /*< FatFs Execute perms */

#define S_IRGRP (S_IRUSR >> 3)  /*< Read by group.  */
#define S_IWGRP (S_IWUSR >> 3)  /*< Write by group.  */
#define S_IXGRP (S_IXUSR >> 3)  /*< Execute by group.  */
#define S_IRWXG (S_IRWXU >> 3)  /*< Read,Write,Execute by user */
#define S_IROTH (S_IRGRP >> 3)  /*< Read by others.  */
#define S_IWOTH (S_IWGRP >> 3)  /*< Write by others.  */
#define S_IXOTH (S_IXGRP >> 3)  /*< Execute by others.  */
#define S_IRWXO (S_IRWXG >> 3)  /*< Read,Write,Execute by other */
/**@} */

/**
 * @name Defines for lseek
 */
/**@{ */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
/**@} */

/**
 * @name Default POSIX functions
 */
/**@{ */
int open(const char *pathname, int flags);
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
int close(int fd);
/**@} */

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FREERTOS_POSIX_FCNTL_H_ */
