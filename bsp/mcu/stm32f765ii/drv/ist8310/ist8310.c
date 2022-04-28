#include "ist8310.h"
#include "ist8310_reg.h"
#include "board.h"
#include "board_cfg.h"

static char *device = "IST8310";

ist8310_data_t ist8310_data;

enum ist8310_state_t {
    IST8310_RESET,
    IST8310_RESET_WAIT,
    IST8310_CONF,
    IST8310_MEAS,
    IST8310_READ
};
enum ist8310_state_t ist8310_state;

uint8_t IST8310_ReadReg(uint8_t reg);
void IST8310_WriteReg(uint8_t reg, uint8_t value);
void IST8310_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits);
int IST8310_Configure();
void IST8310_Meas();
int IST8310_Process();
int IST8310_Probe();

typedef struct {
    i2c_cfg_t *i2c;
    ist8310_param_t param;
} ist8310_cfg_t;
static ist8310_cfg_t ist8310_cfg;

static buffer_t buffer;

static TickType_t t_now  = 0;
static TickType_t t_last = 0;

int IST8310_Init() {
    int rv = 0;

    ist8310_cfg.i2c = &i2c3;
    ist8310_state = IST8310_RESET;

    return rv;
}

void IST8310_Run() {
    switch(ist8310_state) {

    case IST8310_RESET:
        IST8310_WriteReg(CNTL2, SRST);
        ist8310_state = IST8310_RESET_WAIT;
        vTaskDelay(50);
        break;

    case IST8310_RESET_WAIT:
        IST8310_ReadReg(CNTL2);
        if ((IST8310_ReadReg(WHO_AM_I) == DEVICE_ID)
            && ((IST8310_ReadReg(CNTL2) & SRST) == 0)) {
                ist8310_state = IST8310_CONF;
                vTaskDelay(10);
            } else {
                printf("%s: Wrong default registers values after reset\n", device);
                vTaskDelay(1000);
                ist8310_state = IST8310_RESET;
            }
        break;

    case IST8310_CONF:
        if(IST8310_Configure()) {
            ist8310_state = IST8310_MEAS;
        } else {
            printf("%s: Wrong configuration, reset\n", device);
            ist8310_state = IST8310_RESET;
            vTaskDelay(1000);
        }
        break;

    case IST8310_MEAS:
        IST8310_WriteReg(CNTL1, SINGLE_MEAS);
        t_last = xTaskGetTickCount();
        ist8310_state = IST8310_READ;
        break;

    case IST8310_READ:
        t_now = xTaskGetTickCount();
        if((t_now - t_last) >= 20) {
            IST8310_Meas();
            IST8310_Process();
            IST8310_WriteReg(CNTL1, SINGLE_MEAS);
            t_last = t_now;
            ist8310_data.dt = t_now - t_last;
        } else if(t_last > t_now) {
            t_last = t_now;
        }
        break;
    }
}

uint8_t IST8310_ReadReg(uint8_t reg) {
    uint8_t buf[2];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = reg;

    I2C_Transmit(ist8310_cfg.i2c, buf[0], &buf[1], 1);
    I2C_Receive(ist8310_cfg.i2c, buf[0], &buf[1], 1);

    return buf[1];
}

void IST8310_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | WRITE;
    buf[1] = reg;
    buf[2] = value;

    I2C_Transmit(ist8310_cfg.i2c, buf[0], &buf[1], 2);
}

void IST8310_Meas() {
    uint8_t buf[3];

    buf[0] = (ADDRESS << 1) | READ;
    buf[1] = STAT1;

    I2C_Transmit(ist8310_cfg.i2c, buf[0], &buf[1], 1);
    I2C_Receive(ist8310_cfg.i2c, buf[0], (uint8_t *)&buffer, sizeof(buffer_t));
}

int IST8310_Process() {
    if(buffer.STAT1 & DRDY) {
        ist8310_data.mag_x = msblsb16(buffer.DATAXH, buffer.DATAXL) *
                                        ist8310_cfg.param.mag_scale;
        ist8310_data.mag_y = msblsb16(buffer.DATAYH, buffer.DATAYL) *
                                        ist8310_cfg.param.mag_scale;
        ist8310_data.mag_z = msblsb16(buffer.DATAZH, buffer.DATAZL) *
                                        ist8310_cfg.param.mag_scale;
        return 0;
    } else {
        return EPROTO;
    }
}

void IST8310_SetClearReg(uint8_t reg, uint8_t setbits, uint8_t clearbits) {
    uint8_t orig_val = IST8310_ReadReg(reg);
    uint8_t val = (orig_val & ~clearbits) | setbits;
    if (orig_val != val) {
        IST8310_WriteReg(reg, val);
    }
}

int IST8310_Configure() {
    uint8_t orig_val;
    int rv = 1;

    // Set configure
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        IST8310_SetClearReg(reg_cfg[i].reg, reg_cfg[i].setbits, reg_cfg[i].clearbits);
    }

    // Check
    for(int i = 0; i < SIZE_REG_CFG; i++) {
        orig_val = IST8310_ReadReg(reg_cfg[i].reg);

        if((orig_val & reg_cfg[i].setbits) != reg_cfg[i].setbits) {
            printf("%s: 0x%02x: 0x%02x (0x%02x not set)\n", device,
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].setbits);
            rv = 0;
        }

        if((orig_val & reg_cfg[i].clearbits) != 0) {
            printf("%s: 0x%02x: 0x%02x (0x%02x not cleared)\n", device,
            (uint8_t)reg_cfg[i].reg, orig_val, reg_cfg[i].clearbits);
            rv = 0;
        }
    }

    ist8310_cfg.param.mag_scale = (1.f / 1320.f); // 1320 LSB/Gauss

    return rv;
}

int IST8310_Probe() {
    uint8_t whoami;
    whoami = IST8310_ReadReg(WHO_AM_I);
    if(whoami != DEVICE_ID) {
        printf("%s: Unexpected WHO_AM_I reg 0x%02x\n", device, whoami);
        return ENODEV;
    }
    return 0;
}
