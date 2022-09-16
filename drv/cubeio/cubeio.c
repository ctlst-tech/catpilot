#include "cubeio.h"
#include "cubeio_reg.h"
#include "init.h"
#include "cfg.h"
#include "func.h"
#include <string.h>
#include "timer.h"

static char *device = "CubeIO";

// Data structures
static cubeio_cfg_t cubeio_cfg;
static cubeio_reg_t cubeio_reg;
static enum cubeio_state_t cubeio_state;
static cubeio_packet_t cubeio_tx_packet;
static cubeio_packet_t cubeio_rx_packet;

static struct page_config config;

// Private functions
static int CubeIO_WriteRegs(uint8_t page, 
                            uint8_t offset, 
                            uint8_t count, 
                            uint16_t *ptr);

static int CubeIO_ReadRegs(uint8_t page, 
                           uint8_t offset, 
                           uint8_t count, 
                           uint16_t *ptr);

static int CubeIO_WriteReg(uint8_t page, 
                           uint8_t offset, 
                           uint16_t data);

static int CubeIO_SetClearReg(uint8_t page, 
                              uint8_t offset, 
                              uint16_t setbits, 
                              uint16_t clearbits);

// Sync
static int timer_id;
static int timer_status;
static SemaphoreHandle_t iordy_semaphore;
static uint32_t attempt;

// Public functions
int CubeIO_Init(usart_cfg_t *usart) {
    if(usart == NULL) return -1;

    cubeio_cfg.usart = usart;

    if(iordy_semaphore == NULL) iordy_semaphore = xSemaphoreCreateBinary();

    timer_id = Timer_Create("CubeIO_Timer");

    xSemaphoreGive(iordy_semaphore);
    cubeio_state = CubeIO_RESET;

    return 0;
}

void CubeIO_Run(void) {
    int rv;

    switch(cubeio_state) {

    case CubeIO_RESET:
        vTaskDelay(2000);

        // Check protocol version
        rv = CubeIO_ReadRegs(PAGE_CONFIG, 
                             0, 
                             sizeof(config) / 2, 
                             (uint16_t *)&config);

        if(config.protocol_version != PROTOCOL_VERSION ||
           config.protocol_version2 != PROTOCOL_VERSION2) {
            LOG_ERROR(device, "Wrong protocols versions: %lu, %lu", 
                              config.protocol_version, 
                              config.protocol_version2);
            attempt++;
            if(attempt > 5) {
                LOG_ERROR(device, "Fatal error");
                cubeio_state = CubeIO_FAIL;
            }
        }

        // Set ARM


        break;

    case CubeIO_CONF:
        break;

    case CubeIO_OPERATION:
        xSemaphoreGive(iordy_semaphore);
        break;

    case CubeIO_FAIL:
        vTaskDelay(1000);
        break;
    }
}

int CubeIO_Operation(void) {
    if(cubeio_state == CubeIO_OPERATION) {
        return 0;
    } else {
        return -1;
    }
}

int CubeIO_Ready(void) {
    xSemaphoreTake(iordy_semaphore, portMAX_DELAY);
    return 1;
}

// Private functions
static int CubeIO_WriteRegs(uint8_t page, 
                            uint8_t offset, 
                            uint8_t count, 
                            uint16_t *regs) {
    int rv;
    cubeio_packet_t tx_pkt = {};
    cubeio_packet_t rx_pkt = {};

    tx_pkt.count_code = count | PKT_CODE_WRITE;
    tx_pkt.page = page;
    tx_pkt.offset = offset;
    tx_pkt.crc = 0;
    memcpy(tx_pkt.regs, regs, 2 * count);
    tx_pkt.crc = crc_packet(&tx_pkt);

    for (uint8_t att = 0; att < 3; att++) {
        rv = USART_TransmitReceive(cubeio_cfg.usart,
                                    (uint8_t *)&tx_pkt, (uint8_t *)&rx_pkt,
                                    sizeof(tx_pkt), sizeof(rx_pkt));

        if(rv != SUCCESS) {
            continue;
        }
        if(get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(device, "Bad code, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EINVAL;
            continue;
        }
        if(crc_packet(&rx_pkt) != rx_pkt.crc) {
            LOG_ERROR(device, "Bad crc, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EPROTO;
            continue;
        }
        break;
    }

    return rv;
}

static int CubeIO_ReadRegs(uint8_t page, 
                           uint8_t offset, 
                           uint8_t count, 
                           uint16_t *regs) {
    int rv;
    cubeio_packet_t tx_pkt = {};
    cubeio_packet_t rx_pkt = {};

    tx_pkt.count_code = count | PKT_CODE_READ;
    tx_pkt.page = page;
    tx_pkt.offset = offset;
    tx_pkt.crc = 0;
    memcpy(tx_pkt.regs, regs, 2 * count);
    tx_pkt.crc = crc_packet(&tx_pkt);

    for (uint8_t att = 0; att < 3; att++) {
        rv = USART_TransmitReceive(cubeio_cfg.usart,
                                    (uint8_t *)&tx_pkt, (uint8_t *)&rx_pkt,
                                    sizeof(tx_pkt), sizeof(rx_pkt));

        if(rv != SUCCESS) {
            continue;
        }
        if(get_pkt_code(&rx_pkt) != PKT_CODE_SUCCESS) {
            LOG_ERROR(device, "Bad code, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EINVAL;
            continue;
        }
        if(get_pkt_size(&rx_pkt) != count) {
            LOG_ERROR(device, "Bad count, %d, %d",
                      count, get_pkt_size(&rx_pkt));
            rv = EINVAL;
            continue;
        }
        if(crc_packet(&rx_pkt) != rx_pkt.crc) {
            LOG_ERROR(device, "Bad crc, 0x%X, 0x%X, %d",
                        page, offset, count);
            rv = EPROTO;
            continue;
        }
        memcpy(regs, rx_pkt.regs, 2 * count);
        break;
    }

    return rv;
}

static int CubeIO_WriteReg(uint8_t page, 
                           uint8_t offset, 
                           uint16_t reg) {
    return CubeIO_WriteRegs(page, offset, 1, &reg);
}

static int CubeIO_SetClearReg(uint8_t page, 
                              uint8_t offset, 
                              uint16_t setbits, 
                              uint16_t clearbits) {
    uint16_t reg = 0;
    if(CubeIO_ReadRegs(page, offset, 1, &reg)) return -1;
    reg = (reg & ~clearbits) | setbits;
    return CubeIO_WriteReg(page, offset, reg);
}
