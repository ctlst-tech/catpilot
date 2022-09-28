#include "init.h"
#include "cfg.h"

gpio_cfg_t gpio_periph_pwr  = GPIO_PERIPH_EN;
gpio_cfg_t gpio_sensors_pwr = GPIO_SENSORS_EN;

gpio_cfg_t gpio_fmu_pwm_1 = GPIO_FMU_PWM_1;
gpio_cfg_t gpio_fmu_pwm_2 = GPIO_FMU_PWM_2;
gpio_cfg_t gpio_fmu_pwm_3 = GPIO_FMU_PWM_3;
gpio_cfg_t gpio_fmu_pwm_4 = GPIO_FMU_PWM_4;
gpio_cfg_t gpio_fmu_pwm_5 = GPIO_FMU_PWM_5;
gpio_cfg_t gpio_fmu_pwm_6 = GPIO_FMU_PWM_6;

int GPIOBoard_Init() {
    int rv = 0;

    rv |= GPIO_Init(&gpio_periph_pwr);
    rv |= GPIO_Init(&gpio_sensors_pwr);
    GPIO_Reset(&gpio_periph_pwr);
    GPIO_Set(&gpio_sensors_pwr);

    rv |= GPIO_Init(&gpio_fmu_pwm_1);
    rv |= GPIO_Init(&gpio_fmu_pwm_2);
    rv |= GPIO_Init(&gpio_fmu_pwm_3);
    rv |= GPIO_Init(&gpio_fmu_pwm_4);
    rv |= GPIO_Init(&gpio_fmu_pwm_5);
    rv |= GPIO_Init(&gpio_fmu_pwm_6);
    GPIO_Reset(&gpio_fmu_pwm_1);
    GPIO_Reset(&gpio_fmu_pwm_2);
    GPIO_Reset(&gpio_fmu_pwm_3);
    GPIO_Reset(&gpio_fmu_pwm_4);
    GPIO_Reset(&gpio_fmu_pwm_5);
    GPIO_Reset(&gpio_fmu_pwm_6);

    return rv;
}
