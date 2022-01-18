#include "bit.h"

uint16_t msblsb16(uint8_t msb, uint8_t lsb) {
    return (msb << 8 | lsb);
}
