#include <stdlib.h>
#include <string.h>
#include "fifo.h"

fifo_t *fifo_init(uint32_t size) {
    fifo_t *fifo = NULL;

    if (size > FIFO_MAX_SIZE) 
        return NULL;

    fifo = calloc(1, sizeof(fifo_t));
    if (fifo == NULL)
        return NULL;

    fifo->write_mutex = xSemaphoreCreateMutex();
    if (fifo->write_mutex == NULL)
        return NULL;

    fifo->read_mutex = xSemaphoreCreateMutex();
    if (fifo->read_mutex == NULL)
        return NULL;

    fifo->size = size;
    fifo->count = 0;

    fifo->queue = xQueueCreate(fifo->size, sizeof(uint8_t));
    if (fifo->queue == NULL)
        return NULL;

    return fifo;
}

int fifo_write(fifo_t *fifo, uint8_t *buf, uint16_t length) {
    int i = 0;

    if (fifo == NULL)
        return -1;

    xSemaphoreTake(fifo->write_mutex, portMAX_DELAY);

    for(i = 0; i < length; i++) {
        xQueueSend(fifo->queue, &buf[i], portMAX_DELAY);
        fifo->count++;
    }

    xSemaphoreGive(fifo->write_mutex);

    return i;
}

int fifo_read(fifo_t *fifo, uint8_t *buf, uint16_t length) {
    int i = 0;

    if (fifo == NULL)
        return -1;

    length = (length > 0 ? length : 1);

    xSemaphoreTake(fifo->read_mutex, portMAX_DELAY);

    for(i = 0; i < length; i++) {
        if(i == 0) {
            xQueueReceive(fifo->queue, &buf[i], portMAX_DELAY);
        } else {
            if(xQueueReceive(fifo->queue, &buf[i], 0) == pdFALSE) {
                break;
            }
        }
        fifo->count--;
    }

    xSemaphoreGive(fifo->read_mutex);

    return i;
}

int fifo_get_data_size(fifo_t *fifo) {
    return (fifo->count);
}
