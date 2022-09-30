#pragma once
#include "stm32_base.h"
#include "stm32_periph.h"

typedef struct {
    usart_cfg_t *usart;
} cubeio_cfg_t;

enum cubeio_state_t {
    CubeIO_RESET,
    CubeIO_CONF,
    CubeIO_OPERATION,
    CubeIO_FAIL,
};

enum cubeio_event_t {
    CubeIO_SET_PWM = 1,
    CubeIO_SET_FAILSAFE_PWM,
    CubeIO_FORCE_SAFETY_OFF,
    CubeIO_FORCE_SAFETY_ON,
    CubeIO_ENABLE_SBUS_OUT,
    CubeIO_ONESHOT_ON,
    CubeIO_BRUSHED_ON,
    CubeIO_SET_RATES,
    CubeIO_SET_IMU_HEATER_DUTY,
    CubeIO_SET_DEFAULT_RATE,
    CubeIO_SET_SAFETY_MASK,
    CubeIO_MIXING,
    CubeIO_GPIO,
};

typedef uint32_t cubeio_eventmask_t;

int CubeIO_Init(usart_cfg_t *usart);
int CubeIO_Operation(void);
void CubeIO_Run(void);

void CubeIO_SetPWM(uint8_t channels, uint16_t *pwm);
void CubeIO_SetPWMCh(uint8_t channel, uint16_t pwm);
uint16_t CubeIO_GetPWMCh(uint8_t channel);
void CubeIO_SetFailsafePWM(uint8_t channels, uint16_t pwm);
void CubeIO_SetSafetyMask(uint16_t safety_mask);
void CubeIO_SetFreq(uint16_t chmask, uint16_t freq);
uint16_t CubeIO_GetFreq(uint16_t channel);
void CubeIO_SetDefaultFreq(uint16_t freq);
void CubeIO_SetOneshotMode(void);
void CubeIO_SetBrushedMode(void);
int CubeIO_GetSafetySwitchState(void);
void CubeIO_ForceSafetyOn(void);
void CubeIO_ForceSafetyOff(void);
void CubeIO_SetIMUHeaterDuty(uint8_t duty);
int16_t CubeIO_GetRSSI(void);
void CubeIO_GetRC(uint16_t *ptr);
uint16_t CubeIO_GetRCCh(uint8_t channel);
void CubeIO_EnableSBUSOut(uint16_t freq);