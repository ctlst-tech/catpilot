#ifndef CTLST_DEFAULT_H_
#define CTLST_DEFAULT_H_

#include "spi_pdc.h"

#define CS_NCS3_SHIFTER (1 << 3)

#define CS_GYRO_X (CS_NCS3_SHIFTER | 4)
#define CS_GYRO_Y (CS_NCS3_SHIFTER | 5)
#define CS_GYRO_Z (CS_NCS3_SHIFTER | 6)

#define CS_ACCEL_XY (CS_NCS3_SHIFTER | 0)
#define CS_ACCEL_TEMP_XY (CS_NCS3_SHIFTER | 1)

#define CS_ACCEL_ZY (CS_NCS3_SHIFTER | 2)
#define CS_ACCEL_TEMP_ZY (CS_NCS3_SHIFTER | 3)

#define CS_AD7190 (7)

#define CS_DUMMY (7)

#define DEFAULT_TRANS_DUMMY() SPI_PDC_TRANS_DUMMY(CS_DUMMY)

#endif /* CTLST_DEFAULT_H_ */
