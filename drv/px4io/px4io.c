#include "px4io.h"
#include "px4io_conf.h"
#include "px4io_protocol.h"
#include <string.h>

static char *device = "PX4IO";

static gpio_cfg_t px4io_tx = GPIO_USART8_TX;
static gpio_cfg_t px4io_rx = GPIO_USART8_RX;

static dma_cfg_t dma_px4io_tx;
static dma_cfg_t dma_px4io_rx;

static px4io_cfg_t px4io_cfg;

static uint8_t px4io_buf[32];
px4io_packet_t px4io_tx_packet;
px4io_packet_t px4io_rx_packet;

enum px4io_state_t {
    PX4IO_RESET,
    PX4IO_CONF
};

enum px4io_state_t px4io_state;

int PX4IO_Init() {
    int rv = 0;

    GPIO_Init(&px4io_tx);
    GPIO_Init(&px4io_rx);

    px4io_cfg.usart.USART = UART8;
    px4io_cfg.usart.gpio_tx_cfg = &px4io_tx;
    px4io_cfg.usart.gpio_rx_cfg = &px4io_rx;
    px4io_cfg.usart.dma_tx_cfg = &dma_px4io_tx;
    px4io_cfg.usart.dma_rx_cfg = &dma_px4io_rx;
    px4io_cfg.usart.speed = PX4IO_SERIAL_BITRATE;
    px4io_cfg.usart.timeout = PX4IO_SERIAL_TIMEOUT;
    px4io_cfg.usart.priority = PX4IO_SERIAL_PRIORITY;
    px4io_cfg.usart.mode = USART_IDLE;

    dma_px4io_tx.DMA_InitStruct.Instance = DMA1_Stream0;
    dma_px4io_tx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_5;
    dma_px4io_tx.DMA_InitStruct.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma_px4io_tx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_px4io_tx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_px4io_tx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_px4io_tx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_px4io_tx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_px4io_tx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_px4io_tx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_px4io_tx.priority = PX4IO_SERIAL_PRIORITY;

    dma_px4io_rx.DMA_InitStruct.Instance = DMA1_Stream6;
    dma_px4io_rx.DMA_InitStruct.Init.Channel = DMA_CHANNEL_5;
    dma_px4io_rx.DMA_InitStruct.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_px4io_rx.DMA_InitStruct.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_px4io_rx.DMA_InitStruct.Init.MemInc = DMA_MINC_ENABLE;
    dma_px4io_rx.DMA_InitStruct.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma_px4io_rx.DMA_InitStruct.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma_px4io_rx.DMA_InitStruct.Init.Mode = DMA_NORMAL;
    dma_px4io_rx.DMA_InitStruct.Init.Priority = DMA_PRIORITY_LOW;
    dma_px4io_rx.DMA_InitStruct.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma_px4io_rx.priority = PX4IO_SERIAL_PRIORITY;

    rv = USART_Init(&px4io_cfg.usart);

    vTaskDelay(1000);

    px4io_state = PX4IO_RESET;

    return rv;
}

void PX4IO_Run() {
    switch(px4io_state) {

    case PX4IO_RESET:
        vTaskDelay(2000);
        uint16_t reg;
        reg = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION);

        if(reg == PX4IO_PROTOCOL_VERSION) {
            uint16_t hardware      = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_HARDWARE_VERSION);
            uint16_t max_actuators = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_ACTUATOR_COUNT);
            uint16_t max_controls  = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_CONTROL_COUNT);
            uint16_t max_transfer  = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_MAX_TRANSFER) - 2;
            uint16_t max_rc_input  = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_RC_INPUT_COUNT);
            reg = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION);
            if ((max_actuators < 1) || (max_actuators > PX4IO_MAX_ACTUATORS) ||
                (max_transfer < 16) || (max_transfer > 255)  ||
                (max_rc_input < 1)  || (max_rc_input > 255)) {
                    // TODO Add error processing
            }
            px4io_state = PX4IO_CONF;
        }

    case PX4IO_CONF:
        break;
    }
}

int PX4IO_Write(uint8_t address, uint16_t length) {
    int rv = 0;
    uint8_t page = address >> 8;
	uint8_t offset = address & 0xFF;

    px4io_tx_packet.count_code = length | PKT_CODE_WRITE;
    px4io_tx_packet.page = page;
    px4io_tx_packet.offset = offset;

    for(uint16_t i = length; i < PKT_MAX_REGS; i++) {
        px4io_tx_packet.regs[i] = 0x55AA;
    }

    px4io_tx_packet.crc = 0;
    px4io_tx_packet.crc = crc_packet(&px4io_tx_packet);

    for (uint8_t att = 0; att < 3; att++) {
        rv = USART_TransmitReceive(&px4io_cfg.usart,
                                    (uint8_t *)&px4io_tx_packet, (uint8_t *)&px4io_rx_packet,
                                    sizeof(px4io_tx_packet), sizeof(px4io_rx_packet));

        if(rv == SUCCESS) {
            if (get_pkt_code(&px4io_rx_packet) == PKT_CODE_ERROR) {
				rv = EINVAL;
            }
            break;
        }
    }

    return rv;
}

int PX4IO_Read(uint8_t address, uint16_t length) {
    int rv = 0;
    uint8_t page = address >> 8;
	uint8_t offset = address & 0xFF;

    px4io_tx_packet.count_code = length | PKT_CODE_READ;
    px4io_tx_packet.page = page;
    px4io_tx_packet.offset = offset;

    px4io_tx_packet.crc = 0;
    px4io_tx_packet.crc = crc_packet(&px4io_tx_packet);

    for (uint8_t att = 0; att < 3; att++) {
        rv = USART_TransmitReceive(&px4io_cfg.usart,
                                    (uint8_t *)&px4io_tx_packet,(uint8_t *)&px4io_rx_packet,
                                    sizeof(px4io_tx_packet), sizeof(px4io_rx_packet));

        if(rv == SUCCESS) {
            if (get_pkt_code(&px4io_rx_packet) == PKT_CODE_ERROR) {
				rv = EINVAL;
			} else if (get_pkt_count(&px4io_rx_packet) != length) {
				rv = EIO;
			}
            break;
        } else {
            (void)rv;
            //TODO Add error proccessing
        }
    }

    return rv;
}

int PX4IO_ReadRegs(uint8_t page, uint8_t offset, uint8_t num) {
    int rv = 0;
    rv = PX4IO_Read((page << 8) | offset, num);
    if(px4io_rx_packet.count_code != (PKT_CODE_SUCCESS | px4io_tx_packet.count_code)) {
        return px4io_rx_packet.count_code;
    }
    return rv;
}

uint16_t PX4IO_ReadReg(uint8_t page, uint8_t offset) {
    int rv = 0;
    rv = PX4IO_ReadRegs(page, offset, 1);
    if(rv == SUCCESS) {
        return px4io_rx_packet.regs[0];
    }
    return rv;
}

void UART8_IRQHandler(void) {
    USART_Handler(&px4io_cfg.usart);
}

void DMA1_Stream0_IRQHandler(void) {
    DMA_IRQHandler(&dma_px4io_tx);
}

void DMA1_Stream6_IRQHandler(void) {
    DMA_IRQHandler(&dma_px4io_rx);
}