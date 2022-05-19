#include "ph_periph_mag.h"

extern void IST8310_Run();

void ph_periph_mag_exec(
    ph_periph_mag_outputs_t *o,
    const ph_periph_mag_injection_t *injection
)
{
    IST8310_Run();
}
