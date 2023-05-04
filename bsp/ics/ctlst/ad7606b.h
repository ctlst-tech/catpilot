#ifndef AD7606B_H
#define AD7606B_H

#include <stdint.h>

#define AD7606B_BASE_ADDR                     0x43C90E00

#define AD7606B_ID_OFF                        0x0000
#define AD7606B_CONTROL_OFF                   0x0004

#define AD7606B_CONTROL_ENABLE               (1 << 0)
#define AD7606B_CONTROL_DISABLE              (1 << 1)


#define AD7606B_CHN_SETUP_OFF                 0x0008
#define AD7606B_DISC_PERIOD_OFF               0x000C
#define AD7606B_STATUS_OFF                    0x0010

#define AD7606B_STATUS_CORE_BUSY             (1 << 0)
#define AD7606B_STATUS_BUSY_ERROR_FLAG       (1 << 1)

#define AD7606B_BUSY_ERROR_COUNTER_OFF        0x0014
#define AD7606B_IRQ_CONTROL_OFF               0x0018

#define AD7606B_IRQ_CONTROL_ENABLE            (1 << 0)
#define AD7606B_IRQ_CONTROL_DISABLE           (1 << 1)


#define AD7606B_IRQ_COUNTER_OFF               0x001C
#define AD7606B_IRQ_PROC_TIME_OFF             0x0020
#define AD7606B_VIN_12_CHN_ADC_0_OFF          0x0024
#define AD7606B_VIN_34_CHN_ADC_0_OFF          0x0028
#define AD7606B_VIN_56_CHN_ADC_0_OFF          0x002C
#define AD7606B_VIN_78_CHN_ADC_0_OFF          0x0030
#define AD7606B_VIN_12_CHN_ADC_1_OFF          0x0034
#define AD7606B_VIN_34_CHN_ADC_1_OFF          0x0038
#define AD7606B_VIN_56_CHN_ADC_1_OFF          0x003C
#define AD7606B_VIN_78_CHN_ADC_1_OFF          0x0040

#define AD7606B_VIN_1_CHN_MUX_01_ADC_0_ADDR   0x0044

#define AD7606B_REG_MAP_SIZE                  128

typedef struct {
    uintptr_t base;
    uintptr_t phys_base;
} ad7606b_instance_t;

int adc_ad7606b_init(ad7606b_instance_t *i, uintptr_t phys_base);
int adc_ad7606b_set_disc_period(ad7606b_instance_t *i, uint32_t period_us, uint32_t mck);
int adc_ad7606b_start(ad7606b_instance_t *i);
int adc_ad7606b_stop(ad7606b_instance_t *i);
int adc_ad7606b_get_status(ad7606b_instance_t *i, uint32_t *status);

int adc_ad7606b_get_adc_value(ad7606b_instance_t *i, uint8_t adc_num, uint8_t channel, int16_t* value);

void adc_ad7606b_print_raw_adc_values(ad7606b_instance_t *i);
void adc_ad7606b_print_all_adc_values(ad7606b_instance_t *i);

int adc_get_error_counter(ad7606b_instance_t *i, uint32_t* err_counter);

float adc_convert_value(int16_t value, int8_t range);

#endif  // AD7606B_H
