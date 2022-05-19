#include "core_periph_imu.h"

extern void ICM20602_Run();
extern void ICM20689_Run();
extern void BMI055_Run();

void core_periph_imu_exec(
    core_periph_imu_outputs_t *o,
    const core_periph_imu_injection_t *injection
)
{
    ICM20689_Run();
    ICM20602_Run();
}
