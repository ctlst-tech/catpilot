#include "ad7190.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ctlst_default.h"
#include "devices_common.h"
#include "sens_spi.h"
#include "spi_pdc.h"

#include "ctlst_nav.h"

#define AD7190_TRANS_DUMMY() \
    DEFAULT_TRANS_DUMMY() /*SPI_PDC_TRANS_DUMMY(7)*/
#define ONES_SEQ_LNG 7

int at7190_reset(int cs, int dummy_cs) {
    uint32_t dummy;
    int i;
    uint32_t request[ONES_SEQ_LNG] = {};
    uint32_t response[ONES_SEQ_LNG] = {};

    for (i = 0; i < ONES_SEQ_LNG; i++) {
        request[i] = SPI_PDC_TRANS_BYTE(cs, 0xFF);
    }

    dummy = AD7190_TRANS_DUMMY();

    fcnav_spi_trans(request, ONES_SEQ_LNG, response);

    delay(1);

    return EOK;
}

int at7190_get_reg_size(uint8_t a) {
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

int ad7190_interact(int cs, int dummy_cs, uint8_t *req, uint32_t *resp) {
    uint32_t dummy;

    uint32_t request[6] = {};
    uint32_t response[6] = {};
    uint32_t *_response;
    int resp_size, req_size, i;
    uint32_t rv = 0;

    resp_size = at7190_get_reg_size(AD7190_EXTRACT_ADDR(req[0]));
    req_size = resp_size + 1;

    for (i = 0; i < req_size; i++) request[i] = SPI_PDC_TRANS_BYTE(cs, req[i]);

    dummy = AD7190_TRANS_DUMMY();

    fcnav_spi_trans(request, req_size, response);

    _response = &response[1];
    for (i = 0; i < resp_size; i++) {
        rv |= (SPI_PDC_EXTRACT_BYTE(_response[i]) << (8 * (resp_size - i - 1)));
    }

    if ((rv == 0x3511) || (rv == 0x00188009)) {
        error("Barometric ADC AD7190 got last bit reading issue\n");
    }

    if (resp != NULL) *resp = rv;

    return EOK;
}

int ad7190_write_register(int cs, int dummy_cs, uint8_t addr, uint32_t data) {
    uint8_t d[5];

    d[0] = AD7190_WRITE_REGISTER(addr);
    d[1] = (data >> 16) & 0xFF;
    d[2] = (data >> 8) & 0xFF;
    d[3] = (data)&0xFF;
    d[4] = 0;

    return ad7190_interact(cs, dummy_cs, d, NULL);
}

int ad7190_read_register(int cs, int dummy_cs, uint8_t addr, uint32_t *regv) {
    uint8_t d[5];

    d[0] = AD7190_READ_REGISTER(addr);
    d[1] = d[2] = d[3] = d[4] = 0xFF;

    return ad7190_interact(cs, dummy_cs, d, regv);
}

int ad7190_start(int cs, int dcs, int baro_fix) {
    int rv = EOK;

    uint32_t out, reg_cfg, reg_mode;
    out = 0;

    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    error(__func__, "AD7190_ADDR_REG_CFG read == %08X", out);

    at7190_reset(cs, dcs);

    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    error(__func__, "AD7190_ADDR_REG_CFG read == %08X", out);

    reg_cfg = 0 | AD7190_REG_CFG_BUF |
              AD7190_REG_CFG_CH_SEL(((1 << 0)) | (1 << 4) | (1 << 2) |
                                    (baro_fix ? (1 << 6) : (1 << 5)));

    reg_mode = AD7190_REG_MODE_SELECT(AD7190_MODE_CONTINOUS_CONV) |
               AD7190_REG_MODE_DAT_STA | AD7190_REG_MODE_CLOCK(2) |
               AD7190_REG_MODE_SINC3 | AD7190_REG_MODE_FLTR_OUT_RATE(40);

    if ((reg_cfg & 0x00000001) || (reg_mode & 0x00000001)) {
        error(
            "reg_cfg or reg_mode has non null lsb, this may lead to "
            "undeterminded diag result");
    }

    ad7190_write_register(cs, dcs, AD7190_ADDR_REG_CFG, reg_cfg);
    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    if (reg_cfg != AD7190_MASK_LSB(out)) {
        error("AD7190_ADDR_REG_CFG set == %08X, but read == %08X",
            reg_cfg, out);
        rv = EIO;
    }

    ad7190_write_register(cs, dcs, AD7190_ADDR_REG_MODE, reg_mode);
    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);

    if (reg_mode != AD7190_MASK_LSB(out)) {
        error("AD7190_ADDR_REG_MODE set == %08X, but read == %08X",
            reg_mode, out);
        rv = EIO;
    }

    return rv;
}

int ad7190_start_and_report(int cs, int dcs, int baro_fix) {
    int rv;

    rv = ad7190_start(cs, dcs, baro_fix);
    switch (rv) {
        default:
            error( "AD7190 unknown failure");
            error("AD7190 unknown failure");
            break;
        case EIO:
            error( "AD7190 setup communication failure");
            error("AD7190 setup communication failure");

            break;

        case EOK:
            break;
    }

    return rv;
}

int ad7190_test(int cs, int dcs) {
    uint32_t out;

    at7190_reset(cs, dcs);

    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_STATUS, &out);
    printf("%s. Status register %08X\n", __func__, out);
    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_ID, &out);
    printf("%s. ID register %08X\n", __func__, out);

    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_CFG, &out);
    printf("%s. Config register %08X\n", __func__, out);

    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);
    printf("%s. Mode register %08X\n", __func__, out);

    out = AD7190_REG_MODE_SELECT(AD7190_MODE_CONTINOUS_CONV) |
          AD7190_REG_MODE_DAT_STA | AD7190_REG_MODE_CLOCK(2) |
          AD7190_REG_MODE_SINC3 | AD7190_REG_MODE_FLTR_OUT_RATE(8);

    printf("%s. Setting mode register %08X\n", __func__, out);
    ad7190_write_register(cs, dcs, AD7190_ADDR_REG_MODE, out);

    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_MODE, &out);
    printf("%s. Mode register %08X\n", __func__, out);

    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_FULL_SCALE, &out);
    printf("%s. Full scale register %08X\n", __func__, out);
    ad7190_read_register(cs, dcs, AD7190_ADDR_REG_OFFSET, &out);
    printf("%s. Offset register %08X\n", __func__, out);

    return EOK;
}
