#include "px4io.h"
#include "px4io_conf.h"
#include "px4io_reg.h"
#include <string.h>

static char *device = "PX4IO";

static gpio_cfg_t px4io_tx = GPIO_USART8_TX;
static gpio_cfg_t px4io_rx = GPIO_USART8_RX;

static dma_cfg_t dma_px4io_tx;
static dma_cfg_t dma_px4io_rx;

static px4io_cfg_t px4io_cfg;

px4io_packet_t px4io_tx_packet;
px4io_packet_t px4io_rx_packet;

static uint16_t rc[PX4IO_RC_CHANNELS];

enum px4io_state_t {
    PX4IO_RESET,
    PX4IO_CONF,
    PX4IO_OPERATION,
    PX4IO_ERROR,
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
    int rv;
    uint16_t outputs[PX4IO_MAX_ACTUATORS];
    switch(px4io_state) {

    case PX4IO_RESET:
        vTaskDelay(2000);

        rv = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION);

        if(rv == PX4IO_PROTOCOL_VERSION) {

            // Check PX4IO configuration
            uint16_t hardware      = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_HARDWARE_VERSION);
            uint16_t max_actuators = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_ACTUATOR_COUNT);
            uint16_t max_controls  = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_CONTROL_COUNT);
            uint16_t max_transfer  = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_MAX_TRANSFER) - 2;
            uint16_t max_rc_input  = PX4IO_ReadReg(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_RC_INPUT_COUNT);
            if ((max_actuators < 1) || (max_actuators > PX4IO_MAX_ACTUATORS) ||
                (max_transfer < 16) || (max_transfer > 255)  ||
                (max_rc_input < 1)  || (max_rc_input > 255)) {
                    // TODO Add error processing
                    px4io_state = PX4IO_ERROR;
            } else {
                px4io_state = PX4IO_CONF;
            }

            // Get last IO state
            rv = PX4IO_ReadRegs(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 1);
            if(rv) px4io_state = PX4IO_ERROR;

            // Disarm IO
            rv = PX4IO_SetClearReg(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, 0, PX4IO_P_SETUP_ARMING_FMU_ARMED | PX4IO_P_SETUP_ARMING_LOCKDOWN);
            if(rv) px4io_state = PX4IO_ERROR;
        }
        break;

    case PX4IO_CONF:
        // Here we set number of rc channels, actuators, max/min rate, max/min pwm
        PX4IO_SetArmingState();
        px4io_state = PX4IO_OPERATION;
        break;

    case PX4IO_OPERATION:
        PX4IO_GetRC();
        for(int i = 0; i < PX4IO_MAX_ACTUATORS; i++) {
            outputs[i] = rc[i];
        }
        PX4IO_SetPWM(outputs, PX4IO_MAX_ACTUATORS);
        PX4IO_ReadRegs(PX4IO_PAGE_DIRECT_PWM, 0, PX4IO_MAX_ACTUATORS);
        PX4IO_GetIOStatus();
        break;

    case PX4IO_ERROR:
        // Nothing to do
        break;
    }
}

int PX4IO_SetArmingState() {
    int rv = 0;
    uint16_t set = 0;
    uint16_t clear = 0;

    rv = PX4IO_SetClearReg(PX4IO_PAGE_DISARMED_PWM, 0, 90, PX4IO_MAX_ACTUATORS);

    rv = PX4IO_SetClearReg(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS,
                PX4IO_P_STATUS_FLAGS_SAFETY_OFF | PX4IO_P_STATUS_FLAGS_ARM_SYNC | PX4IO_P_STATUS_FLAGS_INIT_OK, 0);

    // Only for testing, when we don't have a broker
    set |= PX4IO_P_SETUP_ARMING_FMU_ARMED;
    set |= PX4IO_P_SETUP_ARMING_FMU_PREARMED;
    set |= PX4IO_P_SETUP_ARMING_IO_ARM_OK;

    clear |= PX4IO_P_SETUP_ARMING_LOCKDOWN;
    clear |= PX4IO_P_SETUP_ARMING_FORCE_FAILSAFE;

    rv = PX4IO_SetClearReg(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING, set, clear);

    return rv;
}

int PX4IO_GetIOStatus() {
	int rv = 0;

    rv = PX4IO_ReadRegs(PX4IO_PAGE_STATUS, PX4IO_P_STATUS_FLAGS, 6);
    if(rv) return rv;

    uint16_t STATUS_FLAGS  = px4io_rx_packet.regs[0];
    uint16_t STATUS_ALARMS = px4io_rx_packet.regs[1];
    uint16_t STATUS_VSERVO = px4io_rx_packet.regs[4];
    uint16_t STATUS_VRSSI  = px4io_rx_packet.regs[5];

    uint16_t SETUP_ARMING  = PX4IO_ReadReg(PX4IO_PAGE_SETUP, PX4IO_P_SETUP_ARMING);

    return rv;
}

int PX4IO_GetRC() {
    int rv = 0;
    const uint32_t prolog = (PX4IO_P_RAW_RC_BASE - PX4IO_P_RAW_RC_COUNT);
    rv = PX4IO_ReadRegs(PX4IO_PAGE_RAW_RC_INPUT, PX4IO_P_RAW_RC_COUNT, prolog + PX4IO_RC_CHANNELS);
    for(int i = 0; i < PX4IO_RC_CHANNELS; i++){
        rc[i] = px4io_rx_packet.regs[i + prolog];
    }
    return rv;
}

int PX4IO_SetPWM(uint16_t *outputs, uint16_t num) {
    int rv = 0;
    rv = PX4IO_WriteRegs(PX4IO_PAGE_DIRECT_PWM, 0, outputs, num);
    return rv;
}

int PX4IO_Write(uint16_t address, uint16_t *data, uint16_t length) {
    int rv = 0;
    uint8_t page = address >> 8;
    uint8_t offset = address & 0xFF;

    px4io_tx_packet.count_code = length | PKT_CODE_WRITE;
    px4io_tx_packet.page = page;
    px4io_tx_packet.offset = offset;

    memcpy((void *)&px4io_tx_packet.regs[0], (void *)data, (2 * length));

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
        } else {
            //TODO Add error proccessing
            return rv;
        }
    }

    return rv;
}

int PX4IO_Read(uint16_t address, uint16_t length) {
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
            } else {
                break;
            }
        } else {
            //TODO Add error proccessing
            return rv;
        }
    }

    return rv;
}

int PX4IO_ReadRegs(uint8_t page, uint8_t offset, uint8_t num) {
    int rv = 0;
    rv = PX4IO_Read((page << 8) | offset, num);
    return rv;
}

int PX4IO_WriteRegs(uint8_t page, uint8_t offset, uint16_t *data, uint8_t num) {
    int rv = 0;
    rv = PX4IO_Write((page << 8) | offset, data, num);
    return rv;
}

uint32_t PX4IO_ReadReg(uint8_t page, uint8_t offset) {
    int rv = 0;
    rv = PX4IO_ReadRegs(page, offset, 1);
    if(rv != SUCCESS) return PX4IO_READREG_ERROR;
    return px4io_rx_packet.regs[0];
}

int PX4IO_WriteReg(uint8_t page, uint8_t offset, uint16_t data) {
    int rv = 0;
    rv = PX4IO_WriteRegs(page, offset, &data, 1);
    if(rv != SUCCESS) return ERROR;
    return rv;
}

int PX4IO_SetClearReg(uint8_t page, uint8_t offset, uint16_t setbits, uint16_t clearbits) {
    int value = 0;

    value = PX4IO_ReadReg(page, offset);
    if(value < 0) return ERROR;

    value &= ~clearbits;
    value |= setbits;

    return PX4IO_WriteReg(page, offset, value);
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