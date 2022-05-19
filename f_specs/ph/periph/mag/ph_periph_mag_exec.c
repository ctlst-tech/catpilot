#include "core_periph_mag.h"

extern void IST8310_Run();

void core_periph_mag_exec(
    core_periph_mag_outputs_t *o,
    const core_periph_mag_injection_t *injection
)
{
    IST8310_Run();
}
