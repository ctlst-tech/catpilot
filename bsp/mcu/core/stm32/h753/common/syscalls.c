#include <errno.h>
#include <stdio.h>
#include <unistd.h>

extern char _end;
extern char _HeapLimit;
typedef char *caddr_t;

static int total;

caddr_t _sbrk(int incr) {
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }

    prev_heap_end = heap_end;

    char *heap_desir = heap_end + incr;
    char *heap_max = (char *)&_HeapLimit;
    total += incr;

    if (heap_desir > heap_max) {
        errno = ENOMEM;
        return ((void *)-1);
    }
    heap_end += incr;

    return (caddr_t)prev_heap_end;
}
