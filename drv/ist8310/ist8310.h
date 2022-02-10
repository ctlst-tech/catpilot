#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"
#include "const.h"

typedef struct {
    float mag_scale;
} ist8310_dim_t;

typedef struct {
    i2c_cfg_t i2c;
    ist8310_dim_t dim;
} ist8310_cfg_t;

typedef struct {
    float mag_x;
    float mag_y;
    float mag_z;
} ist8310_data_t;

extern ist8310_data_t ist8310_data;

int IST8310_Init();
void IST8310_Run();

uint8_t IST8310_ReadReg(uint8_t reg);
void IST8310_WriteReg(uint8_t reg, uint8_t value);
void IST8310_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);

int IST8310_Configure();
void IST8310_Meas();
int IST8310_Process();

int IST8310_Probe();
