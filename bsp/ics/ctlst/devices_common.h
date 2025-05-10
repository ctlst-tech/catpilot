#ifndef DEVICES_COMMON_H
#define DEVICES_COMMON_H

#define TWOS_CONVERT16(d) ((int32_t)((d) > 32767) ? (d)-65536 : (d))
#define TWOS_CONVERT14(d) ((int32_t)((d) > 8191) ? (d)-16384 : (d))
#define TWOS_CONVERT12(d) ((int32_t)((d) > 2047) ? (d)-4096 : (d))
#define TWOS_CONVERT10(d) ((int32_t)((d) > 511) ? (d)-1024 : (d))

#define TWOS_CONVERT24(d) ((int32_t)((d) > 8388608) ? (d)-16777216 : (d))

#endif // DEVICES_COMMON_H
