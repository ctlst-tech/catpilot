#include "ph_periph_imu.h"

extern double ICM20602_Get_ax();
extern double ICM20602_Get_ay();
extern double ICM20602_Get_az();
extern double ICM20602_Get_wx();
extern double ICM20602_Get_wy();
extern double ICM20602_Get_wz();
extern int ICM20602_MeasReady();

void ph_periph_imu_exec(ph_periph_imu_outputs_t *o)
{
    if(ICM20602_MeasReady()) {
        o->ax = ICM20602_Get_ax();
        o->ay = ICM20602_Get_ay();
        o->az = ICM20602_Get_az();
        o->wx = ICM20602_Get_wx();
        o->wy = ICM20602_Get_wy();
        o->wz = ICM20602_Get_wz();
    }
}

