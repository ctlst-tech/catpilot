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
 * @file errno.h
 * @brief System error numbers.
 *
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html
 *
 * The values defined in this file may not be compatible with the strerror
 * function provided by this system.
 */

#ifndef _FREERTOS_POSIX_ERRNO_H_
#define _FREERTOS_POSIX_ERRNO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Undefine all errnos to avoid redefinition errors with system errnos. */
#undef EPERM
#undef ENOENT
#undef EBADF
#undef EAGAIN
#undef ENOMEM
#undef EEXIST
#undef EBUSY
#undef EINVAL
#undef ENOSPC
#undef ERANGE
#undef ENAMETOOLONG
#undef EDEADLK
#undef EOVERFLOW
#undef ENOSYS
#undef EMSGSIZE
#undef ENOTSUP
#undef ETIMEDOUT

/**
 * @name Definition of POSIX errnos.
 */
/**@{ */
#define EOK             0   /**< NO ERROR */
#define EPERM           1   /**< Operation not permitted. */
#define ENOENT          2   /**< No such file or directory. */
#define ESRCH           3   /**< No such process */
#define EINTR           4   /**< Interrupted system call. */
#define EIO             5   /**< Input/output error. */
#define ENXIO           6   /**< No such device or address. */
#define E2BIG           7   /**< Argument list too long */
#define ENOEXEC         8   /**< Exec format error */
#define EBADF           9   /**< Bad file descriptor. */
#define ECHILD          10  /**< No child processes */
#define EAGAIN          11  /**< Resource unavailable, try again. */
#define ENOMEM          12  /**< Not enough space. */
#define EACCES          13  /**< Permission denied */
#define EFAULT          14  /**< Bad address */
#define ENOTBLK         15  /**< Block device required */
#define EBUSY           16  /**< Device or resource busy */
#define EEXIST          17  /**< File exists */
#define EXDEV           18  /**< Cross-device link */
#define ENODEV          19  /**< No such device */
#define ENOTDIR         20  /**< Not a directory */
#define EISDIR          21  /**< Is a directory */
#define EINVAL          22  /**< Invalid argument */
#define ENFILE          23  /**< File table overflow */
#define EMFILE          24  /**< Too many open files */
#define ENOTTY          25  /**< Not a typewriter */
#define ETXTBSY         26  /**< Text file busy */
#define EFBIG           27  /**< File too large */
#define ENOSPC          28  /**< No space left on device */
#define ESPIPE          29  /**< Illegal seek */
#define EROFS           30  /**< Read-only file system */
#define EMLINK          31  /**< Too many links */
#define EPIPE           32  /**< Broken pipe */
#define EDOM            33  /**< Math argument out of domain of func */
#define ERANGE          34  /**< Math result not representable */
#define EBADMSG         35  /**< Bad Message */
#define ENAMETOOLONG    36  /**< File name too long. */
#define EDEADLK         45  /**< Resource deadlock would occur. */
#define EPROTO          71  /**< Protocol error. */
#define EOVERFLOW       75  /**< Value too large to be stored in data type. */
#define ENOSYS          88  /**< Function not supported. */
#define EMSGSIZE        90  /**< Message too long. */
#define ENOTSUP         95  /**< Operation not supported. */
#define ETIMEDOUT       116 /**< Connection timed out. */

/* for robust mutexes */
#define	EOWNERDEAD	    130	/**< Owner died */
#define	ENOTRECOVERABLE	131	/**< State not recoverable */
/**@} */

/**
 * @name System Variable
 *
 * @brief Define FreeRTOS+POSIX errno, if enabled.
 * Set configUSE_POSIX_ERRNO to enable, and clear to disable. See FreeRTOS.h.
 *
 * @{
 */
extern int FreeRTOS_errno;
#define errno    FreeRTOS_errno
/**@} */

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FREERTOS_POSIX_ERRNO_H_ */
