#ifndef _STDIO_H_
#define _STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>
#include <stdarg.h>
#include <sys/types.h>

#include "node.h"

#define MAXLN 128
#define MAX_FILES 32
#define MAX_NODES 10
#define MAX_NAME_LEN 64
#define EOF -1

extern int errno;
extern struct file *files[MAX_FILES];

typedef off_t fpos_t;

typedef struct file FILE;

#define __SRD   0x0001      /* OK to read */
#define __SWR   0x0002      /* OK to write */
#define __SSTR  0x0004      /* this is an sprintf/snprintf string */
#define __SPGM  0x0008      /* fmt string is in progmem */
#define __SERR  0x0010      /* found error */
#define __SEOF  0x0020      /* found EOF */
#define __SUNGET 0x040      /* ungetc() happened */
#define __SMALLOC 0x80      /* handle is malloc()ed */

#undef stdin
#undef stdout
#undef stderr

#define stdin  (files[0])
#define stdout (files[1])
#define stderr (files[2])

FILE *fopen(const char *path, const char *mode);
int fclose(FILE *stream);
char *getcwd(char *pathname, int len);
int mkdir(const char *pathname, mode_t mode);
int sscanf(const char *buf, const char *fmt, ...);

#include <printf.h>

#ifdef __cplusplus
}
#endif

#endif // _STDIO_H_
