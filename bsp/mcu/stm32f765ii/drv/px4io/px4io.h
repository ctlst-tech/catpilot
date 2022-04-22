#include "stm32_base.h"
#include "stm32_periph.h"
#include "gpio_cfg.h"
#include <stdbool.h>

#define PX4IO_RC_CHANNELS 16
#define PX4IO_MAX_OUTPUT  8

typedef struct {
    uint32_t status;
    uint32_t arming;
    uint32_t vservo;
    uint32_t vrssi;
    uint32_t alarms;
    uint32_t rc[PX4IO_RC_CHANNELS];
    uint32_t outputs[PX4IO_MAX_OUTPUT];
    uint32_t protocol_version;
    uint32_t hardware_version;
    uint32_t max_actuators;
    uint32_t max_controls;
    uint32_t max_transfer;
    uint32_t max_rc_input;

    uint32_t arm;
    uint32_t max_pwm;
    uint32_t min_pwm;
} px4io_reg_t;

int PX4IO_Init();
void PX4IO_Run();

void PX4IO_SetArm(bool arm);
void PX4IO_SetMaxPWM(int pwm);
void PX4IO_SetMinPWM(int pwm);
void PX4IO_SetOutput(int channel, int out);

uint16_t PX4IO_GetRC(int channel);
uint32_t PX4IO_GetState();
