#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"
#include "const.h"

typedef struct {
    float mag_scale;
} ist8310_param_t;

typedef struct {
    float mag_x;
    float mag_y;
    float mag_z;
    uint32_t dt;
} ist8310_data_t;

extern ist8310_data_t ist8310_data;

int IST8310_Init();
void IST8310_Run();
