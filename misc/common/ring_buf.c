#include <stdlib.h>
#include <string.h>
#include "ring_buf.h"

ring_buf_t *ring_buf_init(uint16_t size) {
    ring_buf_t *ptr = NULL;

    if (size > RINGBUFFER_MAX_SIZE) {
        return NULL;
    }

    ptr = calloc(1, sizeof(ring_buf_t));
    if (ptr == NULL) {
        return NULL;
    }

    ptr->rw_mutex = xSemaphoreCreateMutex();
    if (ptr->rw_mutex == NULL) {
        return NULL;
    }

    ptr->cont_mutex = xSemaphoreCreateMutex();
    if (ptr->cont_mutex == NULL) {
        return NULL;
    }

    ptr->r_sem = xSemaphoreCreateBinary();
    if (ptr->r_sem == NULL) {
        return NULL;
    }

    ptr->w_sem = xSemaphoreCreateBinary();
    if (ptr->w_sem == NULL) {
        return NULL;
    }

    ptr->size = size;
    ptr->start_ptr = calloc(size, sizeof(uint8_t));
    ptr->write_ptr = ptr->start_ptr;
    ptr->read_ptr = ptr->start_ptr;
    ptr->count = 0;

    return ptr;
}

uint16_t ring_buf_get_data_size(ring_buf_t *ring_buf) {
    uint16_t rv;

    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    rv = ring_buf->count;
    xSemaphoreGive(ring_buf->rw_mutex);

    return rv;
}

uint16_t ring_buf_get_free_size(ring_buf_t *ring_buf) {
    uint16_t rv;

    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    rv = ring_buf->size - ring_buf->count;
    xSemaphoreGive(ring_buf->rw_mutex);

    return rv;
}

int ring_buf_write(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    int rv;
    uint16_t t_length;
    uint16_t counter = length;

    if (ring_buf == NULL || ring_buf->start_ptr == NULL) {
        return -1;
    }

    xSemaphoreTake(ring_buf->cont_mutex, portMAX_DELAY);

    while (counter != 0) {
        while ((t_length = ring_buf_get_free_size(ring_buf)) == 0) {
            xSemaphoreTake(ring_buf->r_sem, portMAX_DELAY);
        }

        t_length = MIN(t_length, counter);

        xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);

        uint16_t size_to_end_of_buf =
            ring_buf->start_ptr + ring_buf->size - ring_buf->write_ptr;

        uint16_t length_min = MIN(t_length, size_to_end_of_buf);

        memcpy(ring_buf->write_ptr, buf, length_min);
        ring_buf->write_ptr += length_min;

        if (t_length > length_min) {
            memcpy(ring_buf->start_ptr, buf + length_min,
                   t_length - length_min);
            ring_buf->write_ptr = ring_buf->start_ptr + t_length - length_min;
        }

        ring_buf->count += t_length;
        buf += t_length;
        counter -= t_length;

        xSemaphoreGive(ring_buf->w_sem);
        xSemaphoreGive(ring_buf->rw_mutex);
    }

    xSemaphoreGive(ring_buf->cont_mutex);

    rv = length;

    return rv;
}

int ring_buf_read(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    int rv;
    uint16_t data_size;

    if (ring_buf == NULL || ring_buf->start_ptr == NULL) {
        return -1;
    }

    while ((data_size = ring_buf_get_data_size(ring_buf)) < 1) {
        xSemaphoreTake(ring_buf->w_sem, portMAX_DELAY);
    }

    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);

    uint16_t size_to_end_of_buf =
        ring_buf->start_ptr + ring_buf->size - ring_buf->read_ptr;

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

    if (rv > 0) {
        xSemaphoreGive(ring_buf->r_sem);
    }

    xSemaphoreGive(ring_buf->rw_mutex);

    return rv;
}
