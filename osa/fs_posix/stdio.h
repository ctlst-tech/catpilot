/*
 * Copyright (C) Siddharth Bharat Purohit 2017
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdint.h>
#include <stddef.h>
#include <ff.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include "printf.h"

#undef EDOM
#undef ERANGE
#define MAXLN 128
#define MAX_FILES 32
#define MAX_PATH_LENGTH 32
#define MAX_NAME_LEN 13

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t      off_t;
typedef off_t        fpos_t;
extern int errno;

struct stat
{
    dev_t     st_dev;    /*<  ID of device containing file */
    ino_t     st_ino;    /*<  inode number */
    mode_t    st_mode;   /*<  protection */
    nlink_t   st_nlink;  /*<  number of hard links */
    uid_t     st_uid;    /*<  user ID of owner */
    gid_t     st_gid;    /*<  group ID of owner */
    dev_t     st_rdev;   /*<  device ID (if special file) */
    off_t     st_size;   /*<  total size, in bytes */
    uint32_t st_blksize; /*<  blocksize for filesystem I/O */
    uint32_t  st_blocks; /*<  number of 512B blocks allocated */
    time_t    st_atime;  /*<  time of last access */
    time_t    st_mtime;  /*<  time of last modification */
    time_t    st_ctime;  /*<  time of last status change */
};

typedef struct utimbuf
{
    time_t actime;       /* access time */
    time_t modtime;      /* modification time */
} utime_t;

typedef struct dirent {
    char d_name[MAX_NAME_LEN]; /* filename */
} dirent_t;

#define __SRD   0x0001      /* OK to read */
#define __SWR   0x0002      /* OK to write */
#define __SSTR  0x0004      /* this is an sprintf/snprintf string */
#define __SPGM  0x0008      /* fmt string is in progmem */
#define __SERR  0x0010      /* found error */
#define __SEOF  0x0020      /* found EOF */
#define __SUNGET 0x040      /* ungetc() happened */
#define __SMALLOC 0x80      /* handle is malloc()ed */

struct __file {
    char *buf;                       /* buffer pointer */
    unsigned char unget;                /* ungetc() buffer */
    uint8_t flags;                      /* flags, see below */
    int size;                           /* size of buffer */
    int len;                            /* characters read or written so far */
    int (*put)(char, struct __file *);  /* write one char to device */
    int (*get)(struct __file *);        /* read one char from device */
    void *udata;                        /* User defined and accessible data. */
};

#define O_ACCMODE  00000003 /*< read, write, read-write modes */
#define O_RDONLY   00000000 /*< Read only */
#define O_WRONLY   00000001 /*< Write only */
#define O_RDWR     00000002 /*< Read/Write */
#define O_CREAT    00000100 /*< Create file only if it does not exist */
#define O_EXCL     00000200 /*< O_CREAT option, Create fails if file exists */
#define O_NOCTTY   00000400 /*< @todo */
#define O_TRUNC    00001000 /*< Truncate if exists */
#define O_APPEND   00002000 /*< All writes are to EOF */
#define O_NONBLOCK 00004000 /*< @todo */
#define O_BINARY   00000004 /*< Binary */
#define O_TEXT     00000004 /*< Text End Of Line translation */
#define O_CLOEXEC  00000000
#define S_IFMT     0170000  /*< These bits determine file type.  */
#define S_IFDIR    0040000  /*< Directory.  */
#define S_IFCHR    0020000  /*< Character device.  */
#define S_IFBLK    0060000  /*< Block device.  */
#define S_IFREG    0100000  /*< Regular file.  */
#define S_IFIFO    0010000  /*< FIFO.  */
#define S_IFLNK    0120000  /*< Symbolic link.  */
#define S_IFSOCK   0140000  /*< Socket.  */
#define S_IREAD    0400     /*< Read by owner.  */
#define S_IWRITE   0200     /*< Write by owner.  */
#define S_IEXEC    0100     /*< Execute by owner.  */

#define S_ISTYPE(mode, mask)    (((mode) & S_IFMT) == (mask))
#define S_ISDIR(mode)           S_ISTYPE((mode), S_IFDIR)
#define S_ISCHR(mode)           S_ISTYPE((mode), S_IFCHR)
#define S_ISBLK(mode)           S_ISTYPE((mode), S_IFBLK)
#define S_ISREG(mode)           S_ISTYPE((mode), S_IFREG)

#define S_IRUSR S_IREAD                     /*< Read by owner.  */
#define S_IWUSR S_IWRITE                    /*< Write by owner.  */
#define S_IXUSR S_IEXEC                     /*< Execute by owner.  */
#define S_IRWXU (S_IREAD|S_IWRITE|S_IEXEC)  /*< Read,Write,Execute by owner */

#define S_IRGRP (S_IRUSR >> 3)  /*< Read by group.  */
#define S_IWGRP (S_IWUSR >> 3)  /*< Write by group.  */
#define S_IXGRP (S_IXUSR >> 3)  /*< Execute by group.  */
#define S_IRWXG (S_IRWXU >> 3)  /*< Read,Write,Execute by user */
#define S_IROTH (S_IRGRP >> 3)  /*< Read by others.  */
#define S_IWOTH (S_IWGRP >> 3)  /*< Write by others.  */
#define S_IXOTH (S_IXGRP >> 3)  /*< Execute by others.  */
#define S_IRWXO (S_IRWXG >> 3)  /*< Read,Write,Execute by other */

#define modecmp(str, pat) (strcmp(str, pat) == 0 ? 1: 0)

#define FATFS_R (S_IRUSR | S_IRGRP | S_IROTH)   /*< FatFs Read perms */
#define FATFS_W (S_IWUSR | S_IWGRP | S_IWOTH)   /*< FatFs Write perms */
#define FATFS_X (S_IXUSR | S_IXGRP | S_IXOTH)   /*< FatFs Execute perms */

#define EOF (-1)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct __file FILE;

extern FILE *__iob[MAX_FILES];

#undef stdin
#undef stdout
#undef stderr

#define stdin (__iob[0])
#define stdout (__iob[1])
#define stderr (__iob[2])

#define fdev_set_udata(stream, u) do { (stream)->udata = u; } while(0)
#define fdev_get_udata(stream) ((stream)->udata)

#define _FDEV_EOF (-1)
#define _FDEV_ERR (-2)

int vasprintf(char **strp, const char *fmt, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list arg);
int asprintf(char **strp, const char *fmt, ...);
int scanf(const char *fmt, ...);
int sscanf(const char *buf, const char *fmt, ...);
int vsscanf(const char *buf, const char *s, va_list ap);
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);

/* posix.c */
int open(const char *pathname, int flags);
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
int close(int fd);

/* fatfs_posix.c */
int isatty(int fileno);
int fgetc(FILE *stream);
int fputc(int c, FILE *stream);
void clearerr(FILE *stream);

#ifndef IO_MACROS
int getchar(void);
int putchar(int c);
int putc(int c, FILE *stream);
int puts(const char *str);
#endif

char *fgets(char *str, int size, FILE *stream);
int fputs(const char *str, FILE *stream);

int feof(FILE *stream);
int fgetpos(FILE *stream, size_t *pos);
int fseek(FILE *stream, long offset, int whence);
int fsetpos(FILE *stream, size_t *pos);
long ftell(FILE *stream);
off_t lseek(int fileno, off_t position, int whence);
void rewind(FILE *stream);
int fileno(FILE *stream);
FILE *fileno_to_stream(int fileno);
FILE *fopen(const char *path, const char *mode);
size_t __wrap_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int ftruncate(int fd, off_t length);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
void sync(void);
int fsync(int fd);
int truncate(const char *path, off_t length);
int __wrap_fclose(FILE *stream);
FILE *__wrap_freopen(const char *filename, const char *mode, FILE *stream);
int getc(FILE *fp);
char *gets(char *p);

int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

int64_t fs_getfree(void);
int64_t fs_gettotal(void);
int stat(const char *name, struct stat *buf);
char *basename(const char *str);
char *baseext(char *str);
int chdir(const char *pathname);
int chmod(const char *pathname, mode_t mode);
char *dirname(char *str);
int utime(const char *filename, const struct utimbuf *times);

char *getcwd(char *pathname, int len);
int mkdir(const char *pathname, mode_t mode);
int rename(const char *oldpath, const char *newpath);
int rmdir(const char *pathname);
int unlink(const char *pathname);
int remove(const char *pathname);
int closedir(DIR *dirp);
DIR *opendir(const char *pathdir);
struct dirent *readdir(DIR *dirp);
void clrerror(FILE *stream);
int ferror(FILE *stream);
void perror(const char *s);
char *strerror(int errnum);
char *__wrap_strerror_r(int errnum, char *buf, size_t buflen);
FILE *fdevopen(int (*put)(char, FILE *), int (*get)(FILE *));
int fatfs_getc(FILE *stream);
int fatfs_putc(char c, FILE *stream);
int fatfs_to_errno(FRESULT Result);
int fatfs_to_fileno(FIL *fh);
time_t fat_time_to_unix(uint16_t date, uint16_t time);
void unix_time_to_fat(time_t epoch, uint16_t *date, uint16_t *time);
FIL *fileno_to_fatfs(int fileno);
int free_file_descriptor(int fileno);
int new_file_descriptor(void);
int posix_fopen_modes_to_open(const char *mode);

int __wrap_fprintf(FILE *fp, const char *format, ...);
int fprintf(FILE *fp, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // _STDIO_H_
