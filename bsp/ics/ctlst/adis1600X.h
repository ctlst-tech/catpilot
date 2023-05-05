#ifndef ADIS1600X_H_
#define ADIS1600X_H_

#include "spi_pdc.h"

#define ADIS1600X_CONTROL_AXIS_X			(0)
#define ADIS1600X_CONTROL_AXIS_Y			(1)
#define ADIS1600X_CONTROL_REG(a)			( ( ( (a & 0x01) << 3 ) | ( 1 << 2 ) ) << 8 )

#define ADIS1600X_OUTPUT_READ_X(cs)			SPI_PDC_TRANS_WORD ( cs, ADIS1600X_CONTROL_REG (ADIS1600X_CONTROL_AXIS_X) )
#define ADIS1600X_OUTPUT_READ_Y(cs)			SPI_PDC_TRANS_WORD ( cs, ADIS1600X_CONTROL_REG (ADIS1600X_CONTROL_AXIS_Y) )				

#define ADIS1600X_OUTPUT_READ_TEMP(cs)		SPI_PDC_TRANS_WORD ( cs, 0x0000 )


#define ADIS16003_SCALE 0.011965811F

#define ADIS16006_SCALE 0.0383203125
#define ADIS1600X_OFFSET 2048.0F

#define ADIS1600X_TEMP_SCALE ( 0.25 ) 
#define ADIS1600X_TEMP_OFFSET ( 0 )

#endif /*ADIS1600X_H_*/