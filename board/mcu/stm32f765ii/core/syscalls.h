#include  <errno.h>
#include  <sys/unistd.h>

#define __weak   __attribute__((weak))

int _write(int fd, char* ptr, int len);
