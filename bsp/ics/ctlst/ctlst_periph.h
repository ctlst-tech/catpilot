#ifndef CTLST_PERIPH_H
#define CTLST_PERIPH_H

#include "spiplr.h"

#undef SPI_PDC_TRANSMISSION
#define SPI_PDC_TRANSMISSION(__lst, __cs, __data) \
    SPIPLR_COMPOSE_REQ(__cs, __data, !(__lst))

int fc_dxrs450_diag_and_report(char *dn, int cs, int dcs);
int fc_d7190_start_and_report(int cs, int dcs, int baro_fix);

#endif  // CTLST_PERIPH_H
