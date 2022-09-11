#include <stdlib.h>
#include <string.h>
#include "ringbuf.h"

ringbuf_t *RingBuf_Init(uint16_t size) {
    ringbuf_t *ptr = NULL;
    if(size > RINGBUFFER_MAX_SIZE) return NULL;

    ptr = calloc(1, sizeof(ringbuf_t));
    if(ptr == NULL) return NULL;

    ptr->mutex = xSemaphoreCreateMutex();
    if(ptr->mutex == NULL) return NULL;

    ptr->size = size;
    ptr->start_ptr = calloc(size, sizeof(uint8_t));
    ptr->end_ptr = ptr->start_ptr + size * sizeof(uint8_t);
    ptr->write_ptr = ptr->start_ptr;
    ptr->read_ptr = ptr->start_ptr;
    ptr->count = 0;
    return ptr;
}

ringbuf_t *RingBuf_Delete(ringbuf_t *ringbuf) {
    if(ringbuf == NULL) return NULL;

    free(ringbuf->start_ptr);
    free(ringbuf);
    return ringbuf;
}

uint16_t RingBuf_GetDataSize(ringbuf_t *ringbuf) {
    return (ringbuf->count);
}

uint16_t RingBuf_GetFreeSize(ringbuf_t *ringbuf) {
    return (ringbuf->size - ringbuf->count);
}

int RingBuf_Write(ringbuf_t *ringbuf, uint8_t *buf, uint16_t length) {
    int rv;
    if(ringbuf == NULL) return -1;
    if(ringbuf->start_ptr == NULL) return -1;
    if(length > ringbuf->size) return -1;

    xSemaphoreTake(ringbuf->mutex, portMAX_DELAY);

    uint16_t free_size = RingBuf_GetFreeSize(ringbuf);
    uint16_t size_to_end_of_buf = ringbuf->end_ptr - ringbuf->write_ptr;

    if(length <= free_size) {
        if(size_to_end_of_buf >= length) {
            memcpy(ringbuf->write_ptr, buf, length);
            ringbuf->write_ptr += length;
        } else {
            memcpy(ringbuf->write_ptr, buf, size_to_end_of_buf);
            ringbuf->write_ptr = ringbuf->start_ptr;
            memcpy(ringbuf->write_ptr, buf, length - size_to_end_of_buf);
            ringbuf->write_ptr += length - size_to_end_of_buf;
        }
        ringbuf->count += length;
        rv = length;
    } else {
        rv = -1;
    }

    xSemaphoreGive(ringbuf->mutex);

    return rv;
}

int RingBuf_Read(ringbuf_t *ringbuf, uint8_t *buf, uint16_t length) {
    int rv;
    if(ringbuf == NULL) return -1;
    if(ringbuf->start_ptr == NULL) return -1;

    xSemaphoreTake(ringbuf->mutex, portMAX_DELAY);

    uint16_t data_size = RingBuf_GetDataSize(ringbuf);
    uint16_t size_to_end_of_buf = ringbuf->end_ptr - ringbuf->read_ptr;
    length = (data_size >= length ? length : data_size);

    if(size_to_end_of_buf >= length) {
        memcpy(buf, ringbuf->read_ptr, length);
        ringbuf->read_ptr += length;
    } else {
        memcpy(buf, ringbuf->read_ptr, size_to_end_of_buf);
        ringbuf->read_ptr = ringbuf->start_ptr;
        memcpy(buf + size_to_end_of_buf, ringbuf->read_ptr, length - size_to_end_of_buf);
        ringbuf->read_ptr += length - size_to_end_of_buf;
    }
    ringbuf->count -= length;
    rv = length;

    xSemaphoreGive(ringbuf->mutex);

    return rv;
}
