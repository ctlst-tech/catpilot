#include "adxrs450.h"

#include <errno.h>
#include <f/ferrors.h>
#include <f/fsyslog.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "devices_common.h"
#include "sens_spi.h"

int adxrs450_parity_calc_32(uint32_t d) {
    int i;
    char p;
    p = (d >> 1) & 0x00000001;
    for (i = 2; i < 32; i++) {
        p ^= ((d >> i) & 0x01);
    }
    return (p == 1) ? 0 : 1;
}

int adxrs450_parity_check_32(uint32_t d) {
    return ((adxrs450_parity_calc_32(d) ^ (d & 0x00000001)) == 0) ? EOK
                                                                  : EBADMSG;
}

#define ADXRS450_INTERACTION_TYPE_READ_OUT 0
#define ADXRS450_INTERACTION_TYPE_READ_REG 1

int adxrs450_interact(int cs, int dummy_cs, uint64_t req, uint32_t *resp) {
    struct __attribute__((packed)) {
        uint64_t req;
        uint32_t dummy;
        uint64_t resp;
        uint32_t dummy1;
    } tx, rx;

    uint32_t tresp;

    tx.req = req;
    tx.dummy = tx.dummy1 = SPI_PDC_TRANS_DUMMY(dummy_cs);
    tx.resp = ADXRS450_REG_READ(cs, ADXRS450_REG_ADDRESS_TEM);

    fcnav_spi_trans((uint32_t *)&tx, sizeof(tx) / sizeof(uint32_t),
                    (uint32_t *)&rx);

    tresp = SPI_PDC_EXTRACT_DWORD(rx.resp);
    if (adxrs450_parity_check_32(tresp) != EOK) return EIO;

    *resp = tresp;

    return EOK;
}

int adxrs450_read_register(int cs, int dummy_cs, uint16_t addr,
                           uint16_t *regv) {
    int rv;
    uint32_t resp;

    if ((rv = adxrs450_interact(cs, dummy_cs, ADXRS450_REG_READ(cs, addr),
                                &resp)) != EOK)
        return rv;
    *regv = ADXRS450_REG_VALUE(resp);
    return EOK;
}

void adxrs450_print_test_flags(uint32_t out) {
    printf("Selftest bits 0x%02X\n", ADXRS450_OUTPUT_GET_ST_BITS(out));
    printf("Output bit %s status %s\n", "PLL",
           (out & ADXRS450_OUTPUT_CST_PLL) ? "is set" : "is cleared");
    printf("Output bit %s status %s\n", "Q",
           (out & ADXRS450_OUTPUT_CST_Q) ? "is set" : "is cleared");
    printf("Output bit %s status %s\n", "NVM",
           (out & ADXRS450_OUTPUT_CST_NVM) ? "is set" : "is cleared");
    printf("Output bit %s status %s\n", "POR",
           (out & ADXRS450_OUTPUT_CST_POR) ? "is set" : "is cleared");
    printf("Output bit %s status %s\n", "PWR",
           (out & ADXRS450_OUTPUT_CST_PWR) ? "is set" : "is cleared");
    printf("Output bit %s status %s\n", "CST",
           (out & ADXRS450_OUTPUT_CST_CST) ? "is set" : "is cleared");
}

int adxrs450_test_faults(int cs, int dummy_cs, uint16_t *st_check_result,
                         uint16_t *st_result) {
    int en = 0;
    uint32_t out_chk_asserted, out_chk_not_asserted;
    int rv;
#define ST_EN_MAX 5

    en = 0;
    do {
        if ((rv = adxrs450_interact(cs, dummy_cs,
                                    ADXRS450_READ_OUTPUT_CHK(cs, 0),
                                    &out_chk_asserted)) != EOK)
            return rv;
        delay(60);
        en++;
    } while ((ADXRS450_OUTPUT_GET_ST_BITS(out_chk_asserted) !=
              ADXRS450_OUTPUT_GET_ST_SELFTEST) &
             (en < ST_EN_MAX));

    if (en >= ST_EN_MAX) return ETIME;

    if (st_check_result != NULL)
        *st_check_result = out_chk_asserted & ADXRS450_OUTPUT_CST_BIT_MASK;

    en = 0;
    do {
        if ((rv = adxrs450_interact(cs, dummy_cs, ADXRS450_READ_OUTPUT(cs, 0),
                                    &out_chk_not_asserted)) != EOK)
            return rv;
        delay(60);
        en++;
    } while ((ADXRS450_OUTPUT_GET_ST_BITS(out_chk_not_asserted) !=
              ADXRS450_OUTPUT_GET_ST_DATA) &
             (en < ST_EN_MAX));

    if (en >= ST_EN_MAX) return ETIME;

    if (st_result != NULL)
        *st_result = out_chk_not_asserted & ADXRS450_OUTPUT_CST_BIT_MASK;

    return EOK;
}

int adxrs450_diag_get_sn(int cs, int dummy_cs, uint32_t *sn) {
    uint16_t snh, snl;
    int rv;

    if ((rv = adxrs450_read_register(cs, dummy_cs, ADXRS450_REG_ADDRESS_SNH,
                                     &snh)) != EOK)
        return rv;

    if ((rv = adxrs450_read_register(cs, dummy_cs, ADXRS450_REG_ADDRESS_SNL,
                                     &snl)) != EOK)
        return rv;

    *sn = (snh << 16) | snl;

    return EOK;
}

char *adxrs450_strerror(int err) {
    switch (err) {
        case EIO:
            return "SPI interaction error";

        case ETIME:
            return "selftest response timeout";

        case EFAULT:
            return "hardware faults detected";

        case EOK:
            return "no error";

        default:
            return "unknown error";
    }
}

int adxrs450_diag(int cs, int dcs, uint32_t *sn, uint16_t *fault_reg,
                  uint32_t *diag_res) {
    uint16_t st_check_result, st_result;
    int rv;
    if ((rv = adxrs450_diag_get_sn(cs, dcs, sn)) != EOK) return rv;

    if ((rv = adxrs450_read_register(cs, dcs, ADXRS450_REG_ADDRESS_FAULT,
                                     fault_reg)) != EOK)
        return rv;

    if ((rv = adxrs450_test_faults(cs, dcs, &st_check_result, &st_result)) !=
        EOK)
        return rv;

    if ((st_check_result != ADXRS450_OUTPUT_CST_BIT_MASK) | (st_result != 0)) {
        if (diag_res != NULL) *diag_res = (st_check_result << 16) | st_result;
        return EFAULT;
    }

    return EOK;
}

int adxrs450_diag_and_report(char *dn, int cs, int dcs) {
    uint32_t sn = 0;
    uint32_t diag_res = 0;
    uint16_t fault_reg = 0;
    int rv;

    switch (rv = adxrs450_diag(cs, dcs, &sn, &fault_reg, &diag_res)) {
        case EIO:
            error("%s ADXRS45x does not respond", dn);
            printf("fnav. %s does not respond\n", dn);
            break;

        case ETIME:
        case EFAULT:
            error(
                "%s ADXRS45x SN=%08X failure (fault=0x%04X selftest=%08X): %s",
                dn, sn, fault_reg, diag_res, adxrs450_strerror(rv));
            printf("%s fault reg:\n", dn);
            adxrs450_print_test_flags(fault_reg);
            break;

        case EOK:
            error("%s ADXRS45x SN=%08X - OK", dn, sn);
            break;

        default:
            error("%s ADXRS45x SN=%08X, unknown state", dn, sn);
            break;
    }

    return rv;
}
