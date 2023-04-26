#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <unistd.h>

#include "ctlst_sensors_imu.h"
#include "spiplr.h"

typedef struct __attribute__((packed)) {
    // Gyro X + Temp
    uint64_t fault_gx;
    uint64_t gyro_x;
    uint64_t temp_gx;

    // Gyro Y + Temp
    uint64_t fault_gy;
    uint64_t gyro_y;
    uint64_t temp_gy;

    // Gyro Z + Temp
    uint64_t fault_gz;
    uint64_t gyro_z;
    uint64_t temp_gz;

    // ADIS16006 ZY
    uint32_t accel_y1;
    uint32_t accel_temp_zy;
    uint32_t accel_z;

    // ADIS16006 XY
    uint32_t accel_x;
    uint32_t accel_temp_xy;
    uint32_t accel_y;

    // AD7190
    uint32_t ad7190_channel_seq[5];
} ctlst_sensors_imu_data_t;

typedef struct {
    struct {
        uint32_t freq;
        uint32_t prio_delta;
        uint32_t period_us;
    } cfg;
    struct {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        pthread_barrier_t start_barrier;
        int chid;
        int coid;
        int rcvid;
        struct _pulse p;
        struct sigevent timer_event;
        struct sigevent intr_event;
        int intr_id;
        int irq_cnt;
        int irq_cnt_init;
        int irq_spiplr_cnt;
    } sync;
    spiplr_instance_t spiplr_instance;
    ctlst_sensors_imu_data_t data;
    ctlst_sensors_imu_outputs_t o;
} ctlst_sensors_imu_t;

int adxrs450_parity_calc_32(uint32_t d);
int adxrs450_parity_check_32(uint32_t d);

#define PULSE_CODE_UPDATE_TIMER 4
#define PULSE_CODE_UPDATE_POLLER_IRQ 5

#define SCLK_CLOCK_DIVIDER 25

#define SPI_PDC_TRANS_WORD(cs, data) SPI_PDC_TRANSMISSION(1, cs, data)
#define SPI_PDC_TRANS_DWORD(cs, data)                                       \
    ((((uint64_t)SPI_PDC_TRANSMISSION(1, cs, ((data)&0x0000FFFF))) << 32) | \
     ((SPI_PDC_TRANSMISSION(0, cs, ((data) >> 16)))))

#define SPI_PDC_TRANSMISSION(__lst, __cs, __data) \
    SPIPLR_COMPOSE_REQ(__cs, __data, !(__lst))

#define FCNAV_SPIPLR_DEV_ID_GYRO_X      2
#define FCNAV_SPIPLR_DEV_ID_GYRO_Y      3
#define FCNAV_SPIPLR_DEV_ID_GYRO_Z      4
#define FCNAV_SPIPLR_DEV_ID_A_XY        5
#define FCNAV_SPIPLR_DEV_ID_A_YZ        6
#define FCNAV_SPIPLR_DEV_ID_BADC        7
#define FCNAV_SPIPLR_DEV_ID_TA_XY       8
#define FCNAV_SPIPLR_DEV_ID_TA_YZ       9

#define ADXRS450_REG_ADDRESS_TEM                    0x02
#define ADXRS450_REG_ADDRESS_FAULT                  0x0A
#define ADXRS450_REG_ACCESS(cs,d) 		            SPI_PDC_TRANS_DWORD ( cs, ( d ) | adxrs450_parity_calc_32 ( d ) )
#define ADXRS450_REG_READ(cs,addr)		            ADXRS450_REG_ACCESS ( cs, ( 1 << 31 ) | ( ( ( addr ) & 0x1FF ) << 17 ) )
#define ADXRS450_REG_WRITE(cs,addr,dat)	            ADXRS450_REG_ACCESS ( cs, ( 1 << 30 ) | ( ( ( addr ) & 0x1FF ) << 17 ) | ( dat & 0xFFFF ) )			
#define ADXRS450_REG_VALUE(d)                       ( ( d >> 5 ) & 0xFFFF )
#define ADXRS450_IS_IO_ERROR_VALUE(d)               ( ( ( d ) & ( ( 1 << 31 ) | ( 1 << 30 ) | ( 1 << 29 ) ) ) == 0 )
#define ADXRS450_READ_OUTPUT__P(cs,d)				SPI_PDC_TRANS_DWORD (cs, ( d ) | adxrs450_parity_calc_32 ( d ) )
#define ADXRS450_READ_OUTPUT__(cs,sq2,sq1,sq0,chk)  ADXRS450_READ_OUTPUT__P (cs,  ( (sq2) << 28 ) | ( (sq0) << 30 ) | ( (sq1) << 31 ) | ( 1 << 29 ) | ( ( chk & 0x01 ) << 1 ) )
#define ADXRS450_READ_OUTPUT_(cs,sq,chk) 			ADXRS450_READ_OUTPUT__ ( cs, ( sq >> 2 ) & 0x01, ( sq >> 1 ) & 0x01, ( sq >> 0 ) & 0x01, chk )
#define ADXRS450_READ_OUTPUT(cs,sq) 				ADXRS450_READ_OUTPUT_(cs,sq,0)
#define ADXRS450_READ_OUTPUT_CHK(cs,sq) 			ADXRS450_READ_OUTPUT_(cs,sq,1)

#define ADIS1600X_CONTROL_AXIS_X			(0)
#define ADIS1600X_CONTROL_AXIS_Y			(1)
#define ADIS1600X_CONTROL_REG(a)			( ( ( (a & 0x01) << 3 ) | ( 1 << 2 ) ) << 8 )
#define ADIS1600X_OUTPUT_READ_X(cs)			SPI_PDC_TRANS_WORD ( cs, ADIS1600X_CONTROL_REG (ADIS1600X_CONTROL_AXIS_X) )
#define ADIS1600X_OUTPUT_READ_Y(cs)			SPI_PDC_TRANS_WORD ( cs, ADIS1600X_CONTROL_REG (ADIS1600X_CONTROL_AXIS_Y) )				
#define ADIS1600X_OUTPUT_READ_TEMP(cs)		SPI_PDC_TRANS_WORD ( cs, 0x0000 )
#define ADIS16003_SCALE                     0.011965811F
#define ADIS16006_SCALE                     0.0383203125
#define ADIS1600X_OFFSET                    2048.0F
#define ADIS1600X_TEMP_SCALE                ( 0.25 ) 
#define ADIS1600X_TEMP__OFFSET              ( 0 )

#define AD7190_ADDR_REG_STATUS      ( 0 )
#define AD7190_ADDR_REG_MODE        ( 1 )
#define AD7190_ADDR_REG_CFG         ( 2 )
#define AD7190_ADDR_REG_DATA        ( 3 )
#define AD7190_ADDR_REG_ID          ( 4 )
#define AD7190_ADDR_REG_GPOCON      ( 5 )
#define AD7190_ADDR_REG_OFFSET      ( 6 )
#define AD7190_ADDR_REG_FULL_SCALE  ( 7 )
#define AD7190_REG_COMM_WEN         ( 1 << 7 )
#define AD7190_REG_COMM_READ        ( 1 << 6 )
#define AD7190_REG_COMM_WRITE       ( 0 << 6 )
#define AD7190_REG_COMM_ADDR(a)     ( ( ( (a) & 0x07 ) << 3 ) )
#define AD7190_REG_COMM_CREAD       ( 1 << 2 )
#define AD7190_WRITE_REGISTER(a) 	( ( 0 << 7 ) | AD7190_REG_COMM_WRITE | AD7190_REG_COMM_ADDR ( a ) )
#define AD7190_READ_REGISTER(a) 	( ( 0 << 7 ) | AD7190_REG_COMM_READ | AD7190_REG_COMM_ADDR ( a ) )
