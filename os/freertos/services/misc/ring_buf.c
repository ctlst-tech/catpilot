#include "ring_buf.h"

ring_buf_t *ring_buf_init(uint16_t size) {
    if (size == 0 || size > RINGBUFFER_MAX_SIZE) {
        return NULL;
    }

    ring_buf_t *ring_buf = calloc(1, sizeof(ring_buf_t));
    if (!ring_buf) {
        return NULL;
    }

    ring_buf->start_ptr = calloc(size, sizeof(uint8_t));
    if (!ring_buf->start_ptr) {
        free(ring_buf);
        return NULL;
    }

    ring_buf->rw_mutex = xSemaphoreCreateMutex();
    if (!ring_buf->rw_mutex) {
        free(ring_buf->start_ptr);
        free(ring_buf);
        return NULL;
    }

    ring_buf->cont_mutex = xSemaphoreCreateMutex();
    if (!ring_buf->cont_mutex) {
        vSemaphoreDelete(ring_buf->rw_mutex);
        free(ring_buf->start_ptr);
        free(ring_buf);
        return NULL;
    }

    ring_buf->r_sem = xSemaphoreCreateCounting(RINGBUFFER_MAX_SIZE, 0);
    if (!ring_buf->r_sem) {
        vSemaphoreDelete(ring_buf->cont_mutex);
        vSemaphoreDelete(ring_buf->rw_mutex);
        free(ring_buf->start_ptr);
        free(ring_buf);
        return NULL;
    }

    ring_buf->w_sem = xSemaphoreCreateCounting(RINGBUFFER_MAX_SIZE, size);
    if (!ring_buf->w_sem) {
        vSemaphoreDelete(ring_buf->r_sem);
        vSemaphoreDelete(ring_buf->cont_mutex);
        vSemaphoreDelete(ring_buf->rw_mutex);
        free(ring_buf->start_ptr);
        free(ring_buf);
        return NULL;
    }

    ring_buf->size = size;
    ring_buf->write_ptr = ring_buf->start_ptr;
    ring_buf->read_ptr = ring_buf->start_ptr;
    ring_buf->count = 0;

    return ring_buf;
}

uint16_t ring_buf_get_data_size(ring_buf_t *ring_buf) {
    if (!ring_buf) {
        return 0;
    }

    uint16_t count;
    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    count = ring_buf->count;
    xSemaphoreGive(ring_buf->rw_mutex);
    return count;
}

uint16_t ring_buf_get_free_size(ring_buf_t *ring_buf) {
    if (!ring_buf) {
        return 0;
    }

    uint16_t free_size;
    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    free_size = ring_buf->size - ring_buf->count;
    xSemaphoreGive(ring_buf->rw_mutex);
    return free_size;
}

int ring_buf_write(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    if (!ring_buf || !buf || !ring_buf->start_ptr || length == 0) {
        return -1;
    }

    xSemaphoreTake(ring_buf->cont_mutex, portMAX_DELAY);
    uint16_t remaining = length;
    uint8_t *data = buf;
    int bytes_written = 0;

    while (remaining > 0) {
        uint16_t free_size;
        while ((free_size = ring_buf_get_free_size(ring_buf)) == 0) {
            if (!xSemaphoreTake(ring_buf->r_sem, portMAX_DELAY)) {
                xSemaphoreGive(ring_buf->cont_mutex);
                return -1;
            }
        }

        uint16_t to_write = MIN(free_size, remaining);
        xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);

        uint16_t space_to_end = ring_buf->start_ptr + ring_buf->size - ring_buf->write_ptr;
        uint16_t chunk = MIN(to_write, space_to_end);

        memcpy(ring_buf->write_ptr, data, chunk);
        ring_buf->write_ptr += chunk;

        if (chunk < to_write) {
            uint16_t second_chunk = to_write - chunk;
            memcpy(ring_buf->start_ptr, data + chunk, second_chunk);
            ring_buf->write_ptr = ring_buf->start_ptr + second_chunk;
        }

        ring_buf->count += to_write;
        data += to_write;
        remaining -= to_write;
        bytes_written += to_write;

        for (uint16_t i = 0; i < to_write; i++) {
            xSemaphoreGive(ring_buf->w_sem);
        }

        xSemaphoreGive(ring_buf->rw_mutex);
    }

    xSemaphoreGive(ring_buf->cont_mutex);
    return bytes_written;
}

int ring_buf_read(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    return ring_buf_read_timeout(ring_buf, buf, length, portMAX_DELAY);
}

int ring_buf_read_timeout(ring_buf_t *ring_buf, uint8_t *buf, uint32_t length, uint32_t timeout) {
    if (!ring_buf || !buf || !ring_buf->start_ptr || length == 0) {
        return -1;
    }

    uint16_t data_size;
    while ((data_size = ring_buf_get_data_size(ring_buf)) == 0) {
        if (!xSemaphoreTake(ring_buf->w_sem, timeout)) {
            return -1;
        }
    }

    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);

    uint16_t to_read = MIN(length, data_size);
    uint16_t space_to_end = ring_buf->start_ptr + ring_buf->size - ring_buf->read_ptr;
    uint16_t chunk = MIN(to_read, space_to_end);

    memcpy(buf, ring_buf->read_ptr, chunk);
    ring_buf->read_ptr += chunk;

    if (chunk < to_read) {
        uint16_t second_chunk = to_read - chunk;
        memcpy(buf + chunk, ring_buf->start_ptr, second_chunk);
        ring_buf->read_ptr = ring_buf->start_ptr + second_chunk;
    }

    ring_buf->count -= to_read;

    for (uint16_t i = 0; i < to_read; i++) {
        xSemaphoreGive(ring_buf->r_sem);
    }

    xSemaphoreGive(ring_buf->rw_mutex);
    return to_read;
}