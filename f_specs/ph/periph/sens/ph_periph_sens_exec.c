#include "ph_periph_sens.h"

extern void ICM20602_Run();
extern void ICM20689_Run();
extern void IST8310_Run();

void ph_periph_sens_exec(ph_periph_sens_outputs_t *o)
{
    ICM20689_Run();
    ICM20602_Run();
    IST8310_Run();
}
