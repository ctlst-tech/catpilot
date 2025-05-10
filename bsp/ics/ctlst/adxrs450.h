#ifndef ADXRS450_H_
#define ADXRS450_H_

#include "spi_pdc.h"
#include <math.h>

#define deg2rad(d) ((d) * (M_PI / 180.0))

#define ADXRS450_OUTPUT_CST_PLL (1 << 7)
#define ADXRS450_OUTPUT_CST_Q (1 << 6)
#define ADXRS450_OUTPUT_CST_NVM (1 << 5)
#define ADXRS450_OUTPUT_CST_POR (1 << 4)
#define ADXRS450_OUTPUT_CST_PWR (1 << 3)
#define ADXRS450_OUTPUT_CST_CST (1 << 2)
#define ADXRS450_OUTPUT_CST_CHK (1 << 1)

#define ADXRS450_OUTPUT_CST_BIT_MASK                     \
    (ADXRS450_OUTPUT_CST_PLL | ADXRS450_OUTPUT_CST_Q |   \
     ADXRS450_OUTPUT_CST_NVM | ADXRS450_OUTPUT_CST_POR | \
     ADXRS450_OUTPUT_CST_PWR | ADXRS450_OUTPUT_CST_CST)

#define ADXRS450_OUTPUT_GET_ST_SELFTEST (2)
#define ADXRS450_OUTPUT_GET_ST_DATA (1)
#define ADXRS450_OUTPUT_GET_ST_BITS(v) (((v) >> 26) & 0x3)

#define ADXRS450_REG_ADDRESS_TEM 0x02
#define ADXRS450_REG_ADDRESS_FAULT 0x0A

#define ADXRS450_REG_ADDRESS_SNH 0x0E
#define ADXRS450_REG_ADDRESS_SNL 0x10

int adxrs450_parity_calc_32(uint32_t d);
int adxrs450_parity_check_32(uint32_t d);

#define ADXRS450_REG_ACCESS(cs, d) \
    SPI_PDC_TRANS_DWORD(cs, (d) | adxrs450_parity_calc_32(d))

#define ADXRS450_REG_READ(cs, addr) \
    ADXRS450_REG_ACCESS(cs, (1 << 31) | (((addr)&0x1FF) << 17))
#define ADXRS450_REG_WRITE(cs, addr, dat) \
    ADXRS450_REG_ACCESS(cs, (1 << 30) | (((addr)&0x1FF) << 17) | (dat & 0xFFFF))

#define ADXRS450_REG_VALUE(d) ((d >> 5) & 0xFFFF)
#define ADXRS450_IS_IO_ERROR_VALUE(d) \
    (((d) & ((1 << 31) | (1 << 30) | (1 << 29))) == 0)

#define ADXRS450_READ_OUTPUT__P(cs, d) \
    SPI_PDC_TRANS_DWORD(cs, (d) | adxrs450_parity_calc_32(d))
#define ADXRS450_READ_OUTPUT__(cs, sq2, sq1, sq0, chk)          \
    ADXRS450_READ_OUTPUT__P(cs, ((sq2) << 28) | ((sq0) << 30) | \
                                    ((sq1) << 31) | (1 << 29) | \
                                    ((chk & 0x01) << 1))
#define ADXRS450_READ_OUTPUT_(cs, sq, chk)                         \
    ADXRS450_READ_OUTPUT__(cs, (sq >> 2) & 0x01, (sq >> 1) & 0x01, \
                           (sq >> 0) & 0x01, chk)
#define ADXRS450_READ_OUTPUT(cs, sq) ADXRS450_READ_OUTPUT_(cs, sq, 0)
#define ADXRS450_READ_OUTPUT_CHK(cs, sq) ADXRS450_READ_OUTPUT_(cs, sq, 1)

#define ADXRS450_SCALE ((1.0 / 80.0) * deg2rad(1.0))

#define ADXRS450_TEMP_SCALE (1.0F / 5)
#define ADXRS450_TEMP_OFFSET (-45 / ADXRS450_TEMP_SCALE)

int adxrs450_diag_get_sn(int cs, int dummy_cs, uint32_t *sn);
int adxrs450_diag(int cs, int dcs, uint32_t *sn, uint16_t *fault_reg,
                  uint32_t *diag_res);
int adxrs450_diag_and_report(char *dn, int cs, int dcs);

char *adxrs450_strerror(int err);

#endif /*ADXRS450_H_*/
