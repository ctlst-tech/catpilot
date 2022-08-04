#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

#define PX4IO_RC_CHANNELS 16
#define PX4IO_MAX_OUTPUT  8

#pragma pack(push, 1)
typedef struct {
    uint16_t general_status;
    uint16_t vservo_status;
    uint16_t vrssi_status;
    uint16_t alarms_status;
    uint32_t arm_status;
    uint16_t rc[PX4IO_RC_CHANNELS];
    uint16_t outputs[PX4IO_MAX_OUTPUT];
    uint32_t protocol_version;
    uint32_t hardware_version;
    uint32_t max_actuators;
    uint32_t max_controls;
    uint32_t max_transfer;
    uint32_t max_rc_input;
    uint16_t arm;
    uint16_t max_pwm;
    uint16_t min_pwm;
} px4io_reg_t;
#pragma pack(pop)

enum px4io_state_t {
    PX4IO_RESET,
    PX4IO_CONF,
    PX4IO_OPERATION,
    PX4IO_ERROR,
};

extern enum px4io_state_t px4io_state;

int PX4IO_Init();
void PX4IO_Run();
int PX4IO_Ready();

void PX4IO_SetArm(bool arm);
void PX4IO_SetMaxPWM(int pwm);
void PX4IO_SetMinPWM(int pwm);
void PX4IO_SetOutput(int channel, double out);
void PX4IO_UpdateOutput();

double PX4IO_GetRC(int channel);
