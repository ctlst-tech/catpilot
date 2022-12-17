#include "bit.h"

uint16_t msb_lsb_16(uint8_t msb, uint8_t lsb) {
    return (msb << 8 | lsb);
}

uint32_t get_num_from_mask(uint32_t reg) {
    for(int i = 0; i < 32; i++) {
        if(reg & (1 << i)) {
            return i;
        }
    }
    return 0;
}

uint32_t get_mask_from_num(uint32_t num) {
    return (0x01 << num);
}
