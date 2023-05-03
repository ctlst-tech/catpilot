#include <errno.h>
#include <f/ferrors.h>
#include <f/fsyslog.h>
#include <f/verbosity.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "adxrs450.h"
#include "devices_common.h"
#include "sens_spi.h"

#include "ctlst_periph.h"

#define ST_EN_MAX 5

int fc_dxrs450_parity_calc_32(uint32_t d) {
    int i;
    char p;
    p = (d >> 1) & 0x00000001;
    for (i = 2; i < 32; i++) {
        p ^= ((d >> i) & 0x01);
    }
    return (p == 1) ? 0 : 1;
}

int fc_dxrs450_parity_check_32(uint32_t d) {
    return ((fc_dxrs450_parity_calc_32(d) ^ (d & 0x00000001)) == 0) ? EOK
                                                                    : EBADMSG;
}

#define ADXRS450_INTERACTION_TYPE_READ_OUT 0
#define ADXRS450_INTERACTION_TYPE_READ_REG 1

int fc_dxrs450_interact(int cs, int dummy_cs, uint64_t req, uint32_t *resp) {
    struct __attribute__((packed)) {
        uint64_t req;
        uint64_t resp;
    } tx, rx;

    uint32_t tresp;

    tx.req = req;
    tx.resp = ADXRS450_REG_READ(cs, ADXRS450_REG_ADDRESS_TEM);

    fcnav_spi_trans((uint32_t *)&tx, sizeof(tx) / sizeof(uint32_t),
                    (uint32_t *)&rx);

    tresp = SPI_PDC_EXTRACT_DWORD(rx.resp);

    if (fc_dxrs450_parity_check_32(tresp) != EOK) {
        return EIO;
    }

    *resp = tresp;

    return EOK;
}

int fc_dxrs450_read_register(int cs, int dummy_cs, uint16_t addr,
                             uint16_t *regv) {
    int rv;
    uint32_t resp;

    if ((rv = fc_dxrs450_interact(cs, dummy_cs, ADXRS450_REG_READ(cs, addr),
                                  &resp)) != EOK)
        return rv;
    *regv = ADXRS450_REG_VALUE(resp);
    return EOK;
}

void fc_dxrs450_print_test_flags(uint32_t out) {
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

int fc_dxrs450_test_faults(int cs, int dummy_cs, uint16_t *st_check_result,
                           uint16_t *st_result) {
    int en = 0;
    uint32_t out_chk_asserted, out_chk_not_asserted;
    int rv;

    en = 0;
    do {
        if ((rv = fc_dxrs450_interact(cs, dummy_cs,
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
        if ((rv = fc_dxrs450_interact(cs, dummy_cs, ADXRS450_READ_OUTPUT(cs, 0),
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

int fc_dxrs450_diag_get_sn(int cs, int dummy_cs, uint32_t *sn) {
    uint16_t snh, snl;
    int rv;

    if ((rv = fc_dxrs450_read_register(cs, dummy_cs, ADXRS450_REG_ADDRESS_SNH,
                                       &snh)) != EOK)
        return rv;

    if ((rv = fc_dxrs450_read_register(cs, dummy_cs, ADXRS450_REG_ADDRESS_SNL,
                                       &snl)) != EOK)
        return rv;

    *sn = (snh << 16) | snl;

    return EOK;
}

char *fc_dxrs450_strerror(int err) {
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

int fc_dxrs450_diag(int cs, int dcs, uint32_t *sn, uint16_t *fault_reg,
                    uint32_t *diag_res) {
    uint16_t st_check_result, st_result;
    int rv;
    if ((rv = fc_dxrs450_diag_get_sn(cs, dcs, sn)) != EOK) return rv;

    if ((rv = fc_dxrs450_read_register(cs, dcs, ADXRS450_REG_ADDRESS_FAULT,
                                       fault_reg)) != EOK)
        return rv;

    if ((rv = fc_dxrs450_test_faults(cs, dcs, &st_check_result, &st_result)) !=
        EOK)
        return rv;

    if ((st_check_result != ADXRS450_OUTPUT_CST_BIT_MASK) | (st_result != 0)) {
        if (diag_res != NULL) *diag_res = (st_check_result << 16) | st_result;
        return EFAULT;
    }

    return EOK;
}

int fc_dxrs450_diag_and_report(char *dn, int cs, int dcs) {
    uint32_t sn = 0;
    uint32_t diag_res = 0;
    uint16_t fault_reg = 0;
    int rv;

    switch (rv = fc_dxrs450_diag(cs, dcs, &sn, &fault_reg, &diag_res)) {
        case EIO:
            error("%s ADXRS45x does not respond", dn);
            printf("fnav. %s does not respond\n", dn);
            break;

        case ETIME:
        case EFAULT:
            error(
                "%s ADXRS45x SN=%08X failure (fault=0x%04X selftest=%08X): %s",
                dn, sn, fault_reg, diag_res, fc_dxrs450_strerror(rv));
            printf("%s fault reg:\n", dn);
            fc_dxrs450_print_test_flags(fault_reg);
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

#include <unistd.h>

#include "ad7190.h"

#define AD7190_REQ(__cs, __pos, __size, __data) \
    SPI_PDC_TRANSMISSION(((__pos) == ((__size)-1)) ? 1 : 0, __cs, __data)
#define ONES_SEQ_LNG 7

int fc_d7190_reset(int cs, int dummy_cs) {
    int i;
    uint32_t request[ONES_SEQ_LNG];
    uint32_t response[ONES_SEQ_LNG];

    for (i = 0; i < ONES_SEQ_LNG; i++) {
        request[i] = AD7190_REQ(cs, i, ONES_SEQ_LNG, 0xff);
    }

    fcnav_spi_trans(request, ONES_SEQ_LNG, response);

    delay(1);

    return EOK;
}

int fc_d7190_get_reg_size(uint8_t a) {
    switch (a) {
        default:
        case AD7190_ADDR_REG_STATUS:
        case AD7190_ADDR_REG_ID:
        case AD7190_ADDR_REG_GPOCON:
            return 1;

        case AD7190_ADDR_REG_MODE:
        case AD7190_ADDR_REG_CFG:
        case AD7190_ADDR_REG_OFFSET:
        case AD7190_ADDR_REG_FULL_SCALE:
            return 3;

        case AD7190_ADDR_REG_DATA:
            return 4;
    }
}

int fc_d7190_interact(int cs, int dummy_cs, uint8_t *req, uint32_t *resp) {
    uint32_t request[6];
    uint32_t response[6];
    uint32_t *_response;
    int resp_size, req_size, i;
    uint32_t rv = 0;

    resp_size = fc_d7190_get_reg_size(AD7190_EXTRACT_ADDR(req[0]));
    req_size = resp_size + 1;

    for (i = 0; i < req_size; i++) {
        request[i] = AD7190_REQ(cs, i, req_size, req[i]);
    }

    fcnav_spi_trans(request, req_size, response);

    _response = &response[1];
    for (i = 0; i < resp_size; i++) {
        rv |= (SPI_PDC_EXTRACT_BYTE(_response[i]) << (8 * (resp_size - i - 1)));
    }

    if ((rv == 0x3511) || (rv == 0x00188009)) {
        err(NULL, "Barometric ADC AD7190 got last bit reading issue\n");
    }

    if (resp != NULL) *resp = rv;

    return EOK;
}

int fc_d7190_write_register(int cs, int dummy_cs, uint8_t addr, uint32_t data) {
    uint8_t d[5];

    d[0] = AD7190_WRITE_REGISTER(addr);
    d[1] = (data >> 16) & 0xFF;
    d[2] = (data >> 8) & 0xFF;
    d[3] = (data)&0xFF;
    d[4] = 0;

    return fc_d7190_interact(cs, dummy_cs, d, NULL);
}

int fc_d7190_read_register(int cs, int dummy_cs, uint8_t addr, uint32_t *regv) {
    uint8_t d[5];

    d[0] = AD7190_READ_REGISTER(addr);
    d[1] = d[2] = d[3] = d[4] = 0xFF;

    return fc_d7190_interact(cs, dummy_cs, d, regv);
}

int fc_d7190_start(int cs, int dcs, int baro_fix) {
    int rv = EOK;

    uint32_t out, reg_cfg, reg_mode;

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    error("%s: AD7190_ADDR_REG_CFG read == %08X", __func__, out);

    fc_d7190_reset(cs, dcs);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    error("%s: AD7190_ADDR_REG_CFG read == %08X", __func__, out);

    reg_cfg = 0 | AD7190_REG_CFG_BUF |
              AD7190_REG_CFG_CH_SEL(((1 << 0)) | (1 << 4) | (1 << 2) |
                                    (baro_fix ? (1 << 6) : (1 << 5)));

    reg_mode = AD7190_REG_MODE_SELECT(AD7190_MODE_CONTINOUS_CONV) |
               AD7190_REG_MODE_DAT_STA | AD7190_REG_MODE_CLOCK(2) |
               AD7190_REG_MODE_SINC3 | AD7190_REG_MODE_FLTR_OUT_RATE(40);

    if ((reg_cfg & 0x00000001) || (reg_mode & 0x00000001)) {
        err(__func__,
            "reg_cfg or reg_mode has non null lsb, this may lead to "
            "undeterminded diag result");
    }

    fc_d7190_write_register(cs, dcs, AD7190_ADDR_REG_CFG, reg_cfg);
    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    if (reg_cfg != AD7190_MASK_LSB(out)) {
        err(__func__, "AD7190_ADDR_REG_CFG set == %08X, but read == %08X",
            reg_cfg, out);
        rv = EIO;
    }

    fc_d7190_write_register(cs, dcs, AD7190_ADDR_REG_MODE, reg_mode);
    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);

    if (reg_mode != AD7190_MASK_LSB(out)) {
        err(__func__, "AD7190_ADDR_REG_MODE set == %08X, but read == %08X",
            reg_mode, out);
        rv = EIO;
    }

    return rv;
}

int fc_d7190_start_and_report(int cs, int dcs, int baro_fix) {
    int rv;

    rv = fc_d7190_start(cs, dcs, baro_fix);
    switch (rv) {
        default:
            err(__func__, "AD7190 unknown failure");
            error("AD7190 unknown failure");
            break;
        case EIO:
            err(__func__, "AD7190 setup communication failure");
            error("AD7190 setup communication failure");

            break;

        case EOK:
            break;
    }

    return rv;
}

int fc_d7190_test(int cs, int dcs) {
    uint32_t out;

    fc_d7190_reset(cs, dcs);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_STATUS, &out);
    printf("%s. Status register %08X\n", __func__, out);
    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_ID, &out);
    printf("%s. ID register %08X\n", __func__, out);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    printf("%s. Config register %08X\n", __func__, out);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);
    printf("%s. Mode register %08X\n", __func__, out);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);
    printf("%s. Mode register %08X\n", __func__, out);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);
    printf("%s. Mode register %08X\n", __func__, out);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);
    printf("%s. Mode register %08X\n", __func__, out);

    out = AD7190_REG_MODE_SELECT(AD7190_MODE_CONTINOUS_CONV) |
          AD7190_REG_MODE_DAT_STA | AD7190_REG_MODE_CLOCK(2) |
          AD7190_REG_MODE_SINC3 | AD7190_REG_MODE_FLTR_OUT_RATE(8);

    printf("%s. Setting mode register %08X\n", __func__, out);
    fc_d7190_write_register(cs, dcs, AD7190_ADDR_REG_MODE, out);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);
    printf("%s. Mode register %08X\n", __func__, out);

    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_FULL_SCALE, &out);
    printf("%s. Full scale register %08X\n", __func__, out);
    fc_d7190_read_register(cs, dcs, AD7190_ADDR_REG_OFFSET, &out);
    printf("%s. Offset register %08X\n", __func__, out);

    return EOK;
}
