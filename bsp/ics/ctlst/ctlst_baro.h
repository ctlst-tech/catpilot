#ifndef CTLST_BARO_H
#define CTLST_BARO_H

#include "ad7190.h"

#define PABS_OFFSET (-0xffffff * 0.095 / 2)
#define PABS_SCALE (1000.0F / (0.009F * 0xffffff / 2))

#define PDIFF_OFFSET (0)
#define PDIFF_SCALE (PABS_SCALE)

#define T_OFFSET (0x800000)
#define T_SCALE (1.0 / 2815.0)

#endif /* CTLST_BARO_H */
