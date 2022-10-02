#include "init.h"
#include "cfg.h"

gpio_cfg_t gpio_periph_pwr  = GPIO_PERIPH_EN;
gpio_cfg_t gpio_sensors_pwr = GPIO_SENSORS_EN;

gpio_cfg_t gpio_fmu_pwm[6] = {
    GPIO_FMU_PWM_1,
    GPIO_FMU_PWM_2,
    GPIO_FMU_PWM_3,
    GPIO_FMU_PWM_4,
    GPIO_FMU_PWM_5,
    GPIO_FMU_PWM_6,
};

int GPIOBoard_Init() {
    int rv = 0;

    rv |= GPIO_Init(&gpio_periph_pwr);
    rv |= GPIO_Init(&gpio_sensors_pwr);
    GPIO_Reset(&gpio_periph_pwr);
    GPIO_Set(&gpio_sensors_pwr);

    rv |= GPIO_Init(&gpio_fmu_pwm[0]);
    rv |= GPIO_Init(&gpio_fmu_pwm[1]);
    rv |= GPIO_Init(&gpio_fmu_pwm[2]);
    rv |= GPIO_Init(&gpio_fmu_pwm[3]);
    rv |= GPIO_Init(&gpio_fmu_pwm[4]);
    rv |= GPIO_Init(&gpio_fmu_pwm[5]);
    GPIO_Reset(&gpio_fmu_pwm[0]);
    GPIO_Reset(&gpio_fmu_pwm[1]);
    GPIO_Reset(&gpio_fmu_pwm[2]);
    GPIO_Reset(&gpio_fmu_pwm[3]);
    GPIO_Reset(&gpio_fmu_pwm[4]);
    GPIO_Reset(&gpio_fmu_pwm[5]);

    return rv;
}
