#include <errno.h>
#include <stdio.h>
#include <unistd.h>

extern char _HeapStart;
extern char _HeapLimit;
extern char _end;

extern char _HeapExtraStart;
extern char _HeapExtraSize;
extern char _HeapExtraLimit;

typedef char *caddr_t;

int heap_used;
int heap_total;

int heap_get_total(void) {
    return heap_total;
}

int heap_get_used(void) {
    return heap_used;
}

caddr_t _sbrk(int incr) {
    static char *heap_end;
    static char *limit;

    if (heap_end == 0) {
        heap_end = &_end;
        heap_total = (int)((char *)&_HeapLimit - (char *)&_HeapStart) +
                     (int)((char *)&_HeapExtraLimit - (char *)&_HeapExtraStart);
        limit = (char *)&_HeapLimit;
        heap_used += (int)((char *)&_end - (char *)&_HeapStart);
    }

    char *prev_heap_end = heap_end;
    char *heap_desir = heap_end + incr;

    if (heap_desir > limit) {
        if (limit == (char *)&_HeapLimit) {
            prev_heap_end = (char *)&_HeapExtraStart;
            heap_end = (char *)&_HeapExtraStart;
            limit = (char *)&_HeapExtraLimit;
        } else {
            errno = ENOMEM;
            return ((void *)-1);
        }
    }
    heap_used += incr;
    heap_end += incr;

    return (caddr_t)prev_heap_end;
}
