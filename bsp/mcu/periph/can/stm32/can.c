#include "can.h"

static int can_id_init(can_t *cfg);
static int can_clock_init(can_t *cfg);
static uint32_t can_length_to_hal(uint32_t length);
static uint32_t can_length_from_hal(uint32_t dlc);
static uint32_t can_id_type_to_hal(uint32_t id);
static uint32_t can_frame_type_to_hal(uint32_t frame_type);
static uint32_t can_frame_type_from_hal(uint32_t frame_type);
void can_it_handler(void *area);

int can_init(can_t *cfg) {
    int rv = 0;

    if (cfg == NULL) {
        return -1;
    }
    if ((rv = can_id_init(cfg))) {
        return rv;
    }
    if ((rv = can_clock_init(cfg))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->tx))) {
        return rv;
    }
    if ((rv = gpio_init(cfg->rx))) {
        return rv;
    }
    if ((rv = HAL_FDCAN_Init(&cfg->init))) {
        return rv;
    }
    if ((rv =
             irq_init(cfg->p.it0_id, cfg->irq_priority, can_it_handler, cfg))) {
        return rv;
    }
    if ((rv =
             irq_init(cfg->p.it1_id, cfg->irq_priority, can_it_handler, cfg))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.it0_id))) {
        return rv;
    }
    if ((rv = irq_enable(cfg->p.it1_id))) {
        return rv;
    }

    cfg->p.mutex = xSemaphoreCreateMutex();
    if (cfg->p.mutex == NULL) {
        return -1;
    }
    cfg->p.sem = xSemaphoreCreateBinary();
    if (cfg->p.sem == NULL) {
        return -1;
    }

    if ((rv = HAL_FDCAN_Start(&cfg->init))) {
        return rv;
    }
    if ((rv = HAL_FDCAN_ActivateNotification(&cfg->init,
                                             FDCAN_IT_TX_COMPLETE |
                                                 FDCAN_IT_RX_FIFO0_NEW_MESSAGE |
                                                 FDCAN_IT_RX_FIFO1_NEW_MESSAGE,
                                             0xFFFFFFFF))) {
        return rv;
    }

    return rv;
}

int can_transmit(can_t *cfg, can_header_t *header, uint8_t *pdata) {
    int rv = 0;

    if (cfg == NULL || pdata == NULL || header == NULL || header->size > 8) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    xSemaphoreTake(cfg->p.sem, 0);

    cfg->p.state = CAN_TRANSMIT;

    FDCAN_TxHeaderTypeDef hal_header;
    hal_header.Identifier = header->id;
    hal_header.IdType = can_id_type_to_hal(header->id);
    hal_header.TxFrameType = can_frame_type_to_hal(header->frame_type);
    hal_header.DataLength = can_length_to_hal(header->size);
    hal_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    hal_header.BitRateSwitch = FDCAN_BRS_ON;
    hal_header.FDFormat = FDCAN_CLASSIC_CAN;
    hal_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    hal_header.MessageMarker = 0x00;

    rv = HAL_FDCAN_AddMessageToTxFifoQ(&cfg->init, &hal_header, pdata);

    if (rv == HAL_OK) {
        if (cfg->verbosity == CAN_VERBOSITY_HIGH) {
            can_print_info(cfg, header, pdata);
        }
    } else {
        if (cfg->verbosity > CAN_VERBOSITY_OFF) {
            printf("Interface = %s\n", cfg->name);
            printf("Tx Error = %d\n\n", cfg->init.ErrorCode);
        }
    }

    if (rv == HAL_OK &&
        !xSemaphoreTake(cfg->p.sem, pdMS_TO_TICKS(cfg->timeout))) {
        if (cfg->verbosity > CAN_VERBOSITY_OFF) {
            printf("Interface = %s\n", cfg->name);
            printf("Timeout = %d\n\n", cfg->timeout);
        }
        rv = ETIMEDOUT;
    }

    cfg->p.state = CAN_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

int can_receive(can_t *cfg, can_header_t *header, uint8_t *pdata) {
    int rv = 0;

    if (cfg == NULL || pdata == NULL || header == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    cfg->p.state = CAN_RECEIVE;

    FDCAN_RxHeaderTypeDef hal_header;
    rv = HAL_FDCAN_GetRxMessage(&cfg->init, FDCAN_RX_FIFO0, &hal_header, pdata);

    if (rv == HAL_OK) {
        header->id = hal_header.Identifier;
        header->frame_type = can_frame_type_from_hal(hal_header.RxFrameType);
        header->size = can_length_from_hal(hal_header.DataLength);
        if (cfg->verbosity == CAN_VERBOSITY_HIGH) {
            can_print_info(cfg, header, pdata);
        }
    } else {
        if (cfg->verbosity > CAN_VERBOSITY_OFF) {
            printf("Interface = %s\n", cfg->name);
            printf("Rx Error = %d\n\n", cfg->init.ErrorCode);
        }
    }

    cfg->p.state = CAN_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

void can_print_info(can_t *cfg, can_header_t *header, uint8_t *pdata) {
    printf("Interface = %s\n", cfg->name);
    printf("ID = 0x%X\n", header->id);
    printf("Frame type = 0x%X\n", header->frame_type);
    printf("Msg size = 0x%X\n", header->size);
    printf("Data = ", header->size);
    for (uint32_t i = 0; i < header->size; i++) {
        printf("0x%X ", pdata[i]);
    }
    printf("\n\n");
}

void can_it_handler(void *area) {
    can_t *cfg = (can_t *)area;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    HAL_FDCAN_IRQHandler(&cfg->init);

    if (cfg->p.state == CAN_TRANSMIT && cfg->init.ErrorCode == 0) {
        xSemaphoreGiveFromISR(cfg->p.sem, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken == pdTRUE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

static int can_id_init(can_t *cfg) {
    switch ((uint32_t)(cfg->init.Instance)) {
        case FDCAN1_BASE:
            cfg->p.it0_id = FDCAN1_IT0_IRQn;
            cfg->p.it1_id = FDCAN1_IT1_IRQn;
            break;
        case FDCAN2_BASE:
            cfg->p.it0_id = FDCAN2_IT0_IRQn;
            cfg->p.it1_id = FDCAN2_IT1_IRQn;
            break;
        default:
            return EINVAL;
    }
    return 0;
}

static uint32_t can_length_to_hal(uint32_t length) {
    uint32_t rv = 0;
    switch (length) {
        case 0:
            rv = FDCAN_DLC_BYTES_0;
            break;
        case 1:
            rv = FDCAN_DLC_BYTES_1;
            break;
        case 2:
            rv = FDCAN_DLC_BYTES_2;
            break;
        case 3:
            rv = FDCAN_DLC_BYTES_3;
            break;
        case 4:
            rv = FDCAN_DLC_BYTES_4;
            break;
        case 5:
            rv = FDCAN_DLC_BYTES_5;
            break;
        case 6:
            rv = FDCAN_DLC_BYTES_6;
            break;
        case 7:
            rv = FDCAN_DLC_BYTES_7;
            break;
        case 8:
            rv = FDCAN_DLC_BYTES_8;
            break;
        default:
            rv = 0;
    }
    return rv;
}

static uint32_t can_length_from_hal(uint32_t dlc) {
    uint32_t rv = 0;
    switch (dlc) {
        case FDCAN_DLC_BYTES_0:
            rv = 0;
            break;
        case FDCAN_DLC_BYTES_1:
            rv = 1;
            break;
        case FDCAN_DLC_BYTES_2:
            rv = 2;
            break;
        case FDCAN_DLC_BYTES_3:
            rv = 3;
            break;
        case FDCAN_DLC_BYTES_4:
            rv = 4;
            break;
        case FDCAN_DLC_BYTES_5:
            rv = 5;
            break;
        case FDCAN_DLC_BYTES_6:
            rv = 6;
            break;
        case FDCAN_DLC_BYTES_7:
            rv = 7;
            break;
        case FDCAN_DLC_BYTES_8:
            rv = 8;
            break;
        default:
            rv = 0;
    }
    return rv;
}

static uint32_t can_id_type_to_hal(uint32_t id) {
    if (id > 0x7FF && id < (0x1 << 29)) {
        return FDCAN_EXTENDED_ID;
    }
    return FDCAN_STANDARD_ID;
}

static uint32_t can_frame_type_to_hal(uint32_t frame_type) {
    if (frame_type == CAN_REMOTE_FRAME) {
        return FDCAN_REMOTE_FRAME;
    }
    return FDCAN_DATA_FRAME;
}

static uint32_t can_frame_type_from_hal(uint32_t frame_type) {
    if (frame_type == FDCAN_REMOTE_FRAME) {
        return CAN_REMOTE_FRAME;
    }
    return CAN_DATA_FRAME;
}

static int can_clock_init(can_t *cfg) {
    __HAL_RCC_FDCAN_CLK_ENABLE();
    return 0;
}
