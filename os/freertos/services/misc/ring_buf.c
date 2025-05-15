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
    ring_buf->program_counter = 1;
    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    ring_buf->program_counter = 2;
    rv = ring_buf->count;
    xSemaphoreGive(ring_buf->rw_mutex);
    ring_buf->program_counter = 3;
    return rv;
}

uint16_t ring_buf_get_free_size(ring_buf_t *ring_buf) {
    uint16_t rv;
    ring_buf->program_counter = 4;
    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    rv = ring_buf->size - ring_buf->count;
    ring_buf->program_counter = 5;
    xSemaphoreGive(ring_buf->rw_mutex);
    ring_buf->program_counter = 6;
    return rv;
}

int ring_buf_write(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    int rv;
    uint16_t t_length;
    uint16_t counter = length;
    ring_buf->program_counter = 7;
    if (ring_buf == NULL || ring_buf->start_ptr == NULL) {
        return -1;
    }
    ring_buf->program_counter = 8;
    xSemaphoreTake(ring_buf->cont_mutex, portMAX_DELAY);
    ring_buf->program_counter = 9;
    while (counter != 0) {
        ring_buf->program_counter = 10;
        while ((t_length = ring_buf_get_free_size(ring_buf)) == 0) {
            xSemaphoreTake(ring_buf->r_sem, portMAX_DELAY);
        }
        ring_buf->program_counter = 11;

        t_length = MIN(t_length, counter);
        ring_buf->program_counter = 12;
        xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);

        uint16_t size_to_end_of_buf =
            ring_buf->start_ptr + ring_buf->size - ring_buf->write_ptr;

        uint16_t length_min = MIN(t_length, size_to_end_of_buf);
        ring_buf->program_counter = 13;
        memcpy(ring_buf->write_ptr, buf, length_min);
        ring_buf->write_ptr += length_min;
        ring_buf->program_counter = 14;
        if (t_length > length_min) {
            memcpy(ring_buf->start_ptr, buf + length_min,
                   t_length - length_min);
            ring_buf->write_ptr = ring_buf->start_ptr + t_length - length_min;
        }
        ring_buf->program_counter = 15;
        ring_buf->count += t_length;
        buf += t_length;
        counter -= t_length;
        ring_buf->program_counter = 16;
        xSemaphoreGive(ring_buf->w_sem);
        ring_buf->program_counter = 17;
        xSemaphoreGive(ring_buf->rw_mutex);
    }
    ring_buf->program_counter = 18;
    xSemaphoreGive(ring_buf->cont_mutex);

    rv = length;

    return rv;
}

int ring_buf_read(ring_buf_t *ring_buf, uint8_t *buf, uint16_t length) {
    int rv;
    uint16_t data_size;
    ring_buf->program_counter = 19;
    if (ring_buf == NULL || ring_buf->start_ptr == NULL) {
        return -1;
    }
    ring_buf->program_counter = 20;
    while ((data_size = ring_buf_get_data_size(ring_buf)) < 1) {
        ring_buf->program_counter = 21;
        if (ring_buf->g_semaphore_taken) {
            // Re-entrant take attempt detected – enter safe fail loop
            while (1) {
                ring_buf->panic_counter++;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        ring_buf->program_counter = 22;
        ring_buf->g_semaphore_taken = 1;  // Mark as taken
        ring_buf->program_counter = 23;
        if (!xSemaphoreTake(ring_buf->w_sem, portMAX_DELAY)) {
            ring_buf->g_semaphore_taken = 0;  // Reset if take failed
            return -1;
        }
        ring_buf->program_counter = 24;
        ring_buf->g_semaphore_taken = 0;  // Reset after successful take
    }
    ring_buf->program_counter = 25;
    if (ring_buf->g_semaphore_taken2) {
        // Re-entrant take attempt detected – enter safe fail loop
        ring_buf->program_counter = 26;
        while (1) {
            ring_buf->panic_counter2++;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    ring_buf->program_counter = 27;
    ring_buf->g_semaphore_taken2 = 1;  // Mark as taken
    ring_buf->program_counter = 28;
    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    ring_buf->g_semaphore_taken2 = 0;  // Mark as taken
    ring_buf->program_counter = 29;
    uint16_t size_to_end_of_buf =
        ring_buf->start_ptr + ring_buf->size - ring_buf->read_ptr;

    length = MIN(length, data_size);
    uint16_t length_min = MIN(length, size_to_end_of_buf);
    ring_buf->program_counter = 30;
    memcpy(buf, ring_buf->read_ptr, length_min);
    ring_buf->read_ptr += length_min;
    ring_buf->program_counter = 31;
    if (length > length_min) {
        memcpy(buf + length_min, ring_buf->start_ptr, length - length_min);
        ring_buf->read_ptr = ring_buf->start_ptr + length - length_min;
    }
    ring_buf->program_counter = 32;
    ring_buf->count -= length;
    rv = length;
    ring_buf->program_counter = 33;
    if (rv > 0) {
        xSemaphoreGive(ring_buf->r_sem);
    }
    ring_buf->program_counter = 34;
    xSemaphoreGive(ring_buf->rw_mutex);
    ring_buf->program_counter = 35;
    return rv;
}

int ring_buf_read_timeout(ring_buf_t *ring_buf, uint8_t *buf, uint32_t length,
                          uint32_t timeout) {
    int rv;
    uint16_t data_size;
    ring_buf->program_counter = 36;
    if (ring_buf == NULL || ring_buf->start_ptr == NULL) {
        return -1;
    }
    ring_buf->program_counter = 37;
    if (timeout == 0) {
        timeout = portMAX_DELAY;
    }
    ring_buf->program_counter = 38;
    while ((data_size = ring_buf_get_data_size(ring_buf)) < 1) {
        ring_buf->program_counter = 39;
        if (ring_buf->g_semaphore_taken) {
            // Re-entrant take attempt detected – enter safe fail loop
            while (1) {
                ring_buf->panic_counter++;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        ring_buf->program_counter = 40;
        ring_buf->g_semaphore_taken = 1;  // Mark as taken
        if (!xSemaphoreTake(ring_buf->w_sem, timeout)) {
            ring_buf->program_counter = 41;
            ring_buf->g_semaphore_taken = 0;  // Reset after successful take
            return -1;
        }
        ring_buf->program_counter = 42;
        ring_buf->g_semaphore_taken = 0;  // Reset after successful take
    }
    ring_buf->program_counter = 43;
    if (ring_buf->g_semaphore_taken2) {
        ring_buf->program_counter = 44;
        // Re-entrant take attempt detected – enter safe fail loop
        while (1) {
            ring_buf->panic_counter2++;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    ring_buf->program_counter = 45;
    ring_buf->g_semaphore_taken2 = 1;  // Mark as taken
    ring_buf->program_counter = 46;
    xSemaphoreTake(ring_buf->rw_mutex, portMAX_DELAY);
    ring_buf->g_semaphore_taken2 = 0;  // Mark as taken
    ring_buf->program_counter = 47;
    uint16_t size_to_end_of_buf =
        ring_buf->start_ptr + ring_buf->size - ring_buf->read_ptr;
    ring_buf->program_counter = 48;
    length = MIN(length, data_size);
    uint16_t length_min = MIN(length, size_to_end_of_buf);
    ring_buf->program_counter = 49;
    memcpy(buf, ring_buf->read_ptr, length_min);
    ring_buf->read_ptr += length_min;
    ring_buf->program_counter = 50;
    if (length > length_min) {
        memcpy(buf + length_min, ring_buf->start_ptr, length - length_min);
        ring_buf->read_ptr = ring_buf->start_ptr + length - length_min;
    }
    ring_buf->program_counter = 51;
    ring_buf->count -= length;
    rv = length;

    if (rv > 0) {
        xSemaphoreGive(ring_buf->r_sem);
    }
    ring_buf->program_counter = 52;
    xSemaphoreGive(ring_buf->rw_mutex);
    ring_buf->program_counter = 53;
    return rv;
}
