#include <stdlib.h>
#include <string.h>
#include "ringbuf.h"

ringbuf_t *RingBuf_Init(uint16_t size) {
    ringbuf_t *ptr = NULL;
    if(size > RINGBUFFER_MAX_SIZE) return NULL;
    ptr = calloc(1, sizeof(ringbuf_t));
    if(ptr == NULL) return NULL;
    ptr->size = size;
    ptr->start_ptr = calloc(size, sizeof(uint8_t));
    ptr->end_ptr = ptr->start_ptr + size * sizeof(uint8_t);
    ptr->write_ptr = ptr->start_ptr;
    ptr->read_ptr = ptr->start_ptr;
    ptr->count = 0;
    ptr->mutex = xSemaphoreCreateMutex();
    if(ptr->mutex == NULL) return NULL;
    return ptr;
}

ringbuf_t *RingBuf_Delete(ringbuf_t *ringbuf) {
    ringbuf_t *ptr = NULL;
    if(ringbuf == NULL) return NULL;
    ptr = ringbuf;
    free(ptr->start_ptr);
    free(ptr);
    return ptr;
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
    xSemaphoreTake(ringbuf->mutex, portMAX_DELAY);
    int max_length = RingBuf_GetFreeSize(ringbuf);
    int to_end_length = ringbuf->end_ptr - ringbuf->write_ptr;
    if(ringbuf->overrun_mode) {
        if(length >= ringbuf->size) {
            buf = buf + length - ringbuf->size;
            length = ringbuf->size;
        }
    } else {
        if(RingBuf_IsFull(ringbuf)) {
            xSemaphoreGive(ringbuf->mutex);
            return 0;
        }
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
    xSemaphoreGive(ringbuf->mutex);
    ringbuf->count = (ringbuf->count > ringbuf->size ? ringbuf->size : ringbuf->count);
    return length;
}

int RingBuf_Read(ringbuf_t *ringbuf, uint8_t *buf, uint16_t length) {
    if(ringbuf == NULL) return -1;
    if(ringbuf->start_ptr == NULL) return -1;
    if(RingBuf_IsEmpty(ringbuf)) return 0;
    xSemaphoreTake(ringbuf->mutex, portMAX_DELAY);
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
    xSemaphoreGive(ringbuf->mutex);
    return length;
}
