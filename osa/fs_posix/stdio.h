#ifndef _STDIO_H_
#define _STDIO_H_

#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <fs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAXLN 128
#define MAX_FILES 32
#define MAX_NODES 10
#define MAX_NAME_LEN 64
#define EOF -1

typedef struct file FILE;

extern int errno;

typedef off_t fpos_t;

extern FILE *__iob[MAX_FILES];

#define __SRD   0x0001      /* OK to read */
#define __SWR   0x0002      /* OK to write */
#define __SSTR  0x0004      /* this is an sprintf/snprintf string */
#define __SPGM  0x0008      /* fmt string is in progmem */
#define __SERR  0x0010      /* found error */
#define __SEOF  0x0020      /* found EOF */
#define __SUNGET 0x040      /* ungetc() happened */
#define __SMALLOC 0x80      /* handle is malloc()ed */

#define modecmp(str, pat) (strcmp(str, pat) == 0 ? 1: 0)

#undef stdin
#undef stdout
#undef stderr

#define stdin (__iob[0])
#define stdout (__iob[1])
#define stderr (__iob[2])

// Compiler functions
int scanf(const char *fmt, ...);
int sscanf(const char *buf, const char *fmt, ...);
int vsscanf(const char *buf, const char *s, va_list ap);

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

char *getcwd(char *pathname, int len);
int mkdir(const char *pathname, mode_t mode);
int fprintf(FILE *fp, const char *format, ...);

#include <printf.h>

#ifdef __cplusplus
}
#endif

#endif // _STDIO_H_
