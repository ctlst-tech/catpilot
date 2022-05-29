#include <stdint.h>
#include <stdbool.h>

#define RINGBUFFER_MAX_SIZE 1024

typedef struct {
   uint8_t* write_ptr;
   uint8_t* read_ptr;
   uint8_t* start_ptr;
   uint8_t* end_ptr;
   uint16_t size;
   uint16_t count;
   bool overrun_mode;
} ringbuf_t;

int RingBuf_Init(ringbuf_t *ringbuf, uint16_t size);
int RingBuf_Delete(ringbuf_t *ringbuf);
int RingBuf_SetOverrunMode(ringbuf_t *ringbuf, bool overrun_enable);
int RingBuf_GetDataSize(ringbuf_t *ringbuf);
int RingBuf_GetFreeSize(ringbuf_t *ringbuf);
bool RingBuf_IsFull(ringbuf_t *ringbuf);
bool RingBuf_IsEmpty(ringbuf_t *ringbuf);
int RingBuf_Write(ringbuf_t *ringbuf, uint8_t *buf, uint16_t length);
int RingBuf_Read(ringbuf_t *ringbuf, uint8_t *buf, uint16_t length);

