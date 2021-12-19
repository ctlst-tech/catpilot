#include "syscalls.h"

__weak void retarget_put_char(char c) {
    (void)c;
}

int _write(int fd, char* ptr, int len)
{
    (void)fd;
    int i = 0;
    while (ptr[i] && (i < len)) {
        retarget_put_char((int)ptr[i]);
        if (ptr[i] == '\n') {
            retarget_put_char((int)'\r');
        }
        i++;
    }
    return len;
}
