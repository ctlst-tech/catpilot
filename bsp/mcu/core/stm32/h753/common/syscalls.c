#include <errno.h>
#include <stdio.h>
#include <unistd.h>

extern char _HeapAXIStart;
extern char _HeapAXILimit;
extern char _end;

extern char _HeapDTCMStart;
extern char _HeapDTCMSize;
extern char _HeapDTCMLimit;

extern char _HeapSRAM123Start;
extern char _HeapSRAM123Size;
extern char _HeapSRAM123Limit;

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
        heap_total = (int)((char *)&_HeapAXILimit - (char *)&_HeapAXIStart) +
                     (int)((char *)&_HeapDTCMLimit - (char *)&_HeapDTCMStart);
                    //  (int)((char *)&_HeapSRAM123Limit - (char *)&_HeapSRAM123Start);
        limit = (char *)&_HeapAXILimit;
        heap_used += (int)((char *)&_end - (char *)&_HeapAXIStart);
    }

    char *prev_heap_end = heap_end;
    char *heap_desir = heap_end + incr;

    if (heap_desir > limit) {
        if (limit == (char *)&_HeapAXILimit) {
            prev_heap_end = (char *)&_HeapDTCMStart;
            heap_end = (char *)&_HeapDTCMStart;
            limit = (char *)&_HeapDTCMLimit;
        // } else if (limit == (char *)&_HeapDTCMLimit) {
        //     prev_heap_end = (char *)&_HeapSRAM123Start;
        //     heap_end = (char *)&_HeapSRAM123Start;
        //     limit = (char *)&_HeapSRAM123Limit;
        } else {
            errno = ENOMEM;
            return ((void *)-1);
        }
    }
    heap_used += incr;
    heap_end += incr;

    return (caddr_t)prev_heap_end;
}
