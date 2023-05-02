#ifndef AD7190_H_
#define AD7190_H_

#define AD7190_REG_COMM_WEN ( 1 << 7 )
#define AD7190_REG_COMM_READ  ( 1 << 6 )
#define AD7190_REG_COMM_WRITE  ( 0 << 6 )
#define AD7190_REG_COMM_ADDR(a)  ( ( ( (a) & 0x07 ) << 3 ) )
#define AD7190_REG_COMM_CREAD ( 1 << 2 )

#define AD7190_WRITE_REGISTER(a) 	( ( 0 << 7 ) | AD7190_REG_COMM_WRITE | AD7190_REG_COMM_ADDR ( a ) )
#define AD7190_READ_REGISTER(a) 	( ( 0 << 7 ) | AD7190_REG_COMM_READ | AD7190_REG_COMM_ADDR ( a ) )

#define AD7190_EXTRACT_ADDR(rq) 	( ( (rq) >> 3 ) & 0x07 )

#define AD7190_DATA_EXTRACT_CHNL(d) ( (d) & 0x7 )
#define AD7190_DATA_IS_READY(d) ( (d) & ( 1 << 7 ) )
#define AD7190_DATA_EXTRACT(d) ( (d) >> 8 )

#define AD7190_ADDR_REG_STATUS 		( 0 )
#define AD7190_ADDR_REG_MODE 		( 1 )
#define AD7190_ADDR_REG_CFG 		( 2 )
#define AD7190_ADDR_REG_DATA 		( 3 )
#define AD7190_ADDR_REG_ID 			( 4 )
#define AD7190_ADDR_REG_GPOCON 		( 5 )
#define AD7190_ADDR_REG_OFFSET 		( 6 )
#define AD7190_ADDR_REG_FULL_SCALE 	( 7 )

#define AD7190_REG_MODE_SELECT(m)	( (m) << 21 )
#define AD7190_MODE_CONTINOUS_CONV		0
#define AD7190_MODE_SINGLE				1
#define AD7190_MODE_IDLE				2
#define AD7190_MODE_POWER_DOWN			3
#define AD7190_MODE_INTERNAL_ZERO_CAL	4
#define AD7190_MODE_INTERNAL_FULL_SCALE	5
#define AD7190_MODE_SYSTEM_ZERO_SCALE	6
#define AD7190_MODE_SYSTEM_FULL_SCALE	7

#define AD7190_REG_MODE_DAT_STA		( 1 << 20 )
#define AD7190_REG_MODE_CLOCK(c)	( (c) << 18 )
#define AD7190_REG_MODE_SINC3		( 1 << 15 )
#define AD7190_REG_MODE_ENPAR		( 1 << 13 )
#define AD7190_REG_MODE_SINGLE		( 1 << 11 )
#define AD7190_REG_MODE_REJ60		( 1 << 10 )
#define AD7190_REG_MODE_FLTR_OUT_RATE(fr)	( ( fr & ( 1023 ) ) << 0 )

#define AD7190_REG_CFG_CHOP			( 1 << 23 )
#define AD7190_REG_CFG_REFSEL		( 1 << 20 )
#define AD7190_REG_CFG_CH_SEL(c)	( (c) << 8 )
#define AD7190_REG_CFG_BURN			( 1 << 7 )
#define AD7190_REG_CFG_REFDET		( 1 << 6 )
#define AD7190_REG_CFG_BUF			( 1 << 4 )
#define AD7190_REG_CFG_UNIPOLAR		( 1 << 3 )
#define AD7190_REG_CFG_GAIN(g)		( (g) << 0 )

#define AD7190_MASK_LSB(b) (b & (~(uint32_t)0x1))

int ad7190_start_and_report ( int cs, int dcs, int baro_fix );
int ad7190_test ( int cs, int dummy_cs );

#endif /*AD7190_H_*/
