#ifndef SPI_PDC_H_
#define SPI_PDC_H_

#include <stdint.h>

#define SPI_PDC_TRANSMISSION(lst, cs, data) \
    ((((lst)&0x01) << 24) | (((cs)&0x0F) << 16) | ((data)&0xffff))

#define SPI_PDC_TRANS_WORD(cs, data) SPI_PDC_TRANSMISSION(1, cs, data)
#define SPI_PDC_TRANS_BYTE(cs, data) SPI_PDC_TRANSMISSION(1, cs, (data)&0x00ff)
#define SPI_PDC_TRANS_DWORD(cs, data)                                       \
    ((((uint64_t)SPI_PDC_TRANSMISSION(1, cs, ((data)&0x0000FFFF))) << 32) | \
     ((SPI_PDC_TRANSMISSION(0, cs, ((data) >> 16)))))

#define SPI_PDC_TRANS_DUMMY(cs) SPI_PDC_TRANS_WORD(cs, 0x0000)
#define SPI_PDC_TRANS_DUMMY_ONES(cs) SPI_PDC_TRANS_WORD(cs, 0xFFFF)

#define SPI_PDC_EXTRACT_BYTE(v) ((uint8_t)((v)&0x000000FF))
#define SPI_PDC_EXTRACT_WORD(v) ((uint16_t)((v)&0x0000FFFF))
#define SPI_PDC_EXTRACT_DWORD(v)                       \
    (((uint32_t)(((v)&0x0000FFFF00000000ULL) >> 32)) | \
     ((uint32_t)(((v)&0x000000000000FFFFULL) << 16)))

#define SPI_PDC_EXTRACT_STRUCT_BYTE(b, v) SPI_PDC_EXTRACT_BYTE(b->v)
#define SPI_PDC_EXTRACT_STRUCT_WORD(b, v) SPI_PDC_EXTRACT_WORD(b->v)
#define SPI_PDC_EXTRACT_STRUCT_DWORD(b, v) SPI_PDC_EXTRACT_DWORD(b->v)

#endif /*SPI_PDC_H_*/
