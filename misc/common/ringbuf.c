#include <stdlib.h>
#include <string.h>
#include "ringbuf.h"

int RingBuf_Init(ringbuf_t *ringbuf, uint16_t size) {
    if(ringbuf == NULL) return -1;
    if(size > RINGBUFFER_MAX_SIZE) return -1;
    if(ringbuf->start_ptr != NULL) return -1;
    ringbuf->size = size;
    ringbuf->start_ptr = calloc(size, sizeof(uint8_t));
    if(ringbuf->start_ptr == NULL) return -1;
    ringbuf->end_ptr = ringbuf->start_ptr + size * sizeof(uint8_t);
    ringbuf->write_ptr = ringbuf->start_ptr;
    ringbuf->read_ptr = ringbuf->start_ptr;
    ringbuf->count = 0;
    return size;
}

int RingBuf_Delete(ringbuf_t *ringbuf) {
    if(ringbuf == NULL) return -1;
    if(ringbuf->start_ptr == NULL) return -1;
    free(ringbuf->start_ptr);
    ringbuf->start_ptr = NULL;
    return (ringbuf->size);
}

int RingBuf_SetOverrunMode(ringbuf_t *ringbuf, bool overrun_enable) {
    if(ringbuf == NULL) return -1;
    ringbuf->overrun_mode = overrun_enable;
    return (ringbuf->size);
}

int RingBuf_GetDataSize(ringbuf_t *ringbuf) {
    return (ringbuf->count);
}

int RingBuf_GetFreeSize(ringbuf_t *ringbuf) {
    return (ringbuf->size - ringbuf->count);
}

bool RingBuf_IsFull(ringbuf_t *ringbuf) {
    if(ringbuf->count < ringbuf->size) return false;
    return true;
}

bool RingBuf_IsEmpty(ringbuf_t *ringbuf) {
    if(ringbuf->count > 0) return false;
    return true;
}

int RingBuf_Write(ringbuf_t *ringbuf, uint8_t *buf, uint16_t length) {
    int rv;
    if(ringbuf == NULL) return -1;
    if(ringbuf->start_ptr == NULL) return -1;
    int max_length = RingBuf_GetFreeSize(ringbuf);
    int to_end_length = ringbuf->end_ptr - ringbuf->write_ptr;
    if(ringbuf->overrun_mode) {
        if(length >= ringbuf->size) {
            buf = buf + length - ringbuf->size;
            length = ringbuf->size;
        }
    } else {
        if(RingBuf_IsFull(ringbuf)) return 0;
        length = (max_length >= length ? length : max_length);
    }
    if(to_end_length >= length) {
        memcpy(ringbuf->write_ptr, buf, length);
        ringbuf->write_ptr += length;
    } else {
        memcpy(ringbuf->write_ptr, buf, to_end_length);
        ringbuf->write_ptr = ringbuf->start_ptr;
        memcpy(ringbuf->write_ptr, buf, length - to_end_length);
    }
    ringbuf->count += length;
    ringbuf->count = (ringbuf->count > ringbuf->size ? ringbuf->size : ringbuf->count);
    return length;
}

int RingBuf_Read(ringbuf_t *ringbuf, uint8_t *buf, uint16_t length) {
    if(ringbuf == NULL) return -1;
    if(ringbuf->start_ptr == NULL) return -1;
    if(RingBuf_IsEmpty(ringbuf)) return 0;
    int max_length = RingBuf_GetDataSize(ringbuf);
    int to_end_length = ringbuf->end_ptr - ringbuf->read_ptr;
    length = (max_length >= length ? length : max_length);
    if(to_end_length >= length) {
        memcpy(buf, ringbuf->read_ptr, length);
        ringbuf->read_ptr += length;
    } else {
        memcpy(buf, ringbuf->read_ptr, to_end_length);
        ringbuf->read_ptr = ringbuf->start_ptr;
        memcpy(buf + to_end_length, ringbuf->read_ptr, length - to_end_length);
    }
    ringbuf->count -= length;
    return length;
}
