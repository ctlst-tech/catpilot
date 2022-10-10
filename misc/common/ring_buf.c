#include "ring_buf.h"

ring_buf_t *ring_buf_init(uint16_t size) {
    ring_buf_t *ptr = NULL;

    if (size > RINGBUFFER_MAX_SIZE)
        return NULL;

    ptr = calloc(1, sizeof(ring_buf_t));
    if (ptr == NULL)
        return NULL;

    ptr->mutex = xSemaphoreCreateMutex();
    if (ptr->mutex == NULL)
        return NULL;

    ptr->size = size;
    ptr->start_ptr = calloc(size, sizeof(uint8_t));
    ptr->write_ptr = ptr->start_ptr;
    ptr->read_ptr = ptr->start_ptr;
    ptr->count = 0;

    return ptr;
}

ring_buf_t *ring_buf_delete(ring_buf_t *ring_buf) {
    if (ring_buf == NULL)
        return NULL;

    xSemaphoreTake(ring_buf->mutex, portMAX_DELAY);
    free(ring_buf->start_ptr);
    free(ring_buf);
    xSemaphoreGive(ring_buf->mutex);

    return ring_buf;
}

uint16_t ring_buf_get_data_size(ring_buf_t *ring_buf) {
    uint16_t rv;

    xSemaphoreTake(ring_buf->mutex, portMAX_DELAY);
    rv = ring_buf->count;
    xSemaphoreGive(ring_buf->mutex);

    return rv;
}

uint16_t ring_buf_get_free_size(ring_buf_t *ring_buf) {
    uint16_t rv;

    xSemaphoreTake(ring_buf->mutex, portMAX_DELAY);
    rv = ring_buf->size - ring_buf->count;
    xSemaphoreGive(ring_buf->mutex);

    return rv;
}

int ring_buf_write(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    int rv;

    if (ring_buf == NULL || ring_buf->start_ptr == NULL)
        return -1;

    uint16_t free_size = ring_buf_get_free_size(ring_buf);
    if (length > free_size)
        return -1;

    xSemaphoreTake(ring_buf->mutex, portMAX_DELAY);

    uint16_t size_to_end_of_buf = ring_buf->start_ptr + ring_buf->size -
        ring_buf->write_ptr;

    uint16_t length_min = MIN(length, size_to_end_of_buf);

    memcpy(ring_buf->write_ptr, buf, length_min);
    ring_buf->write_ptr += length_min;

    if (length > length_min) {
        memcpy(ring_buf->start_ptr, buf + length_min, length - length_min);
        ring_buf->write_ptr = ring_buf->start_ptr + length - length_min;
    }

    ring_buf->count += length;
    rv = length;

    xSemaphoreGive(ring_buf->mutex);

    return rv;
}

int ring_buf_read(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    int rv;

    if (ring_buf == NULL || ring_buf->start_ptr == NULL)
        return -1;

    uint16_t data_size = ring_buf_get_data_size(ring_buf);
    if (data_size < 1 || length < 1)
        return 0;

    xSemaphoreTake(ring_buf->mutex, portMAX_DELAY);

    uint16_t size_to_end_of_buf = ring_buf->start_ptr + ring_buf->size -
        ring_buf->read_ptr;

    length = MIN(length, data_size);
    uint16_t length_min = MIN(length, size_to_end_of_buf);

    memcpy(buf, ring_buf->read_ptr, length_min);
    ring_buf->read_ptr += length_min;

    if (length > length_min) {
        memcpy(buf + length_min, ring_buf->start_ptr, length - length_min);
        ring_buf->read_ptr = ring_buf->start_ptr + length - length_min;
    }

    ring_buf->count -= length;
    rv = length;

    xSemaphoreGive(ring_buf->mutex);

    return rv;
}
