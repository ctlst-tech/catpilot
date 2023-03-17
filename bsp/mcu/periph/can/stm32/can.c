#include "can.h"

static int can_id_init(can_t *cfg);
static int can_clock_init(can_t *cfg);
static uint32_t can_length_to_dlc(uint32_t length);
static uint32_t can_dlc_to_length(uint32_t dlc);
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

    if (cfg->p.mutex == NULL) {
        cfg->p.mutex = xSemaphoreCreateMutex();
    }
    if (cfg->p.sem == NULL) {
        cfg->p.sem = xSemaphoreCreateBinary();
    }
    if ((rv = HAL_FDCAN_Start(&cfg->init))) {
        return rv;
    }
    if ((rv = HAL_FDCAN_ActivateNotification(&cfg->init, FDCAN_IT_TX_COMPLETE | FDCAN_IT_RX_BUFFER_NEW_MESSAGE, FDCAN_TX_BUFFER0))) {
        return rv;
    }
    // struct file_operations f_op = {.open = can_open,
    //                                .write = can_write,
    //                                .read = can_read,
    //                                .close = can_close,
    //                                .dev = cfg};

    // char path[32];
    // sprintf(path, "/dev/%s", cfg->name);
    // if (node_mount(path, &f_op) == NULL) {
    //     return -1;
    // }

    // cfg->p.read_buf = ring_buf_init(cfg->buf_size);
    // cfg->p.write_buf = ring_buf_init(cfg->buf_size);

    // cfg->p.dma_rx_buf = calloc(cfg->buf_size, sizeof(uint8_t));
    // cfg->p.dma_tx_buf = calloc(cfg->buf_size, sizeof(uint8_t));

    // cfg->p.periph_init = true;

    return rv;
}

int can_transmit(can_t *cfg, uint32_t id, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || length > 8 || pdata == NULL || id > 0xFFF) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    cfg->p.state = CAN_TRANSMIT;

    FDCAN_TxHeaderTypeDef msg_header;
    msg_header.Identifier = id;
    msg_header.IdType = FDCAN_STANDARD_ID;
    msg_header.TxFrameType = FDCAN_DATA_FRAME;
    msg_header.DataLength = can_length_to_dlc(length);
    msg_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    msg_header.BitRateSwitch = FDCAN_BRS_ON;
    msg_header.FDFormat = FDCAN_CLASSIC_CAN;
    msg_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    msg_header.MessageMarker = 0x00;

    rv = HAL_FDCAN_AddMessageToTxFifoQ(&cfg->init, &msg_header, pdata);

    cfg->p.state = CAN_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

int can_receive(can_t *cfg, uint32_t *id, uint8_t *pdata, uint16_t length) {
    int rv = 0;

    if (length == 0 || pdata == NULL) {
        return EINVAL;
    }

    if (!xSemaphoreTake(cfg->p.mutex, pdMS_TO_TICKS(cfg->timeout))) {
        return ETIMEDOUT;
    }

    cfg->p.state = CAN_RECEIVE;

    FDCAN_RxHeaderTypeDef msg_header;
    uint8_t msg_data[8];
    rv = HAL_FDCAN_GetRxMessage(&cfg->init, FDCAN_RX_FIFO0, &msg_header, msg_data);
    memcpy(pdata, msg_data, can_dlc_to_length(msg_header.DataLength));
    *id = msg_header.Identifier & 0x7FF;

    cfg->p.state = CAN_FREE;
    xSemaphoreGive(cfg->p.mutex);

    return rv;
}

// void can_read_task(void *cfg_ptr) {
//     can_t *cfg = (can_t *)cfg_ptr;
//     uint8_t *buf = cfg->p.dma_rx_buf;
//     while (1) {
//         if (can_receive(cfg, buf, cfg->buf_size)) {
//             cfg->p.error = ERROR;
//         } else {
//             cfg->p.error = SUCCESS;
//         }
//         ring_buf_write(cfg->p.read_buf, buf,
//                        MIN(cfg->p.rx_count, cfg->buf_size));
//     }
// }

// void can_write_task(void *cfg_ptr) {
//     can_t *cfg = (can_t *)cfg_ptr;
//     uint8_t *buf = cfg->p.dma_tx_buf;
//     uint16_t length;
//     while (1) {
//         length = ring_buf_read(cfg->p.write_buf, buf, cfg->buf_size);
//         if (can_transmit(cfg, buf, length)) {
//             cfg->p.error = ERROR;
//         } else {
//             cfg->p.error = SUCCESS;
//         }
//     }
// }

// int can_open(FILE *file, const char *path) {
//     can_t *cfg = (can_t *)file->node->f_op.dev;

//     errno = 0;

//     if (cfg->p.tasks_init) {
//         return 0;
//     }

//     if (cfg->buf_size <= 0) {
//         errno = EINVAL;
//         return -1;
//     }

//     if (cfg->p.read_buf == NULL || cfg->p.write_buf == NULL) {
//         errno = ENOMEM;
//         return -1;
//     }

//     char name[32];
//     snprintf(name, MAX_NAME_LEN, "%s_read_thread", cfg->name);
//     xTaskCreate(can_read_task, name, 128, cfg, cfg->task_priority, NULL);
//     snprintf(name, MAX_NAME_LEN, "%s_wirte_thread", cfg->name);
//     xTaskCreate(can_write_task, name, 128, cfg, cfg->task_priority, NULL);

//     cfg->p.tasks_init = true;
//     errno = 0;

//     return 0;
// }

// ssize_t can_write(FILE *file, const char *buf, size_t count) {
//     ssize_t rv;
//     errno = 0;
//     can_t *cfg = (can_t *)file->node->f_op.dev;

//     rv = ring_buf_write(cfg->p.write_buf, (uint8_t *)buf, count);
//     if (cfg->p.error) {
//         errno = EPROTO;
//         return -1;
//     }
//     return rv;
// }

// ssize_t can_read(FILE *file, char *buf, size_t count) {
//     ssize_t rv;
//     errno = 0;
//     can_t *cfg = (can_t *)file->node->f_op.dev;

//     rv = ring_buf_read(cfg->p.read_buf, (uint8_t *)buf, count);
//     if (cfg->p.error) {
//         errno = EPROTO;
//         return -1;
//     }
//     return rv;
// }

// int can_close(FILE *file) {
//     (void)file;
//     errno = 0;
//     return 0;
// }


void can_it_handler(void *area) {
    can_t *cfg = (can_t *)area;
    HAL_FDCAN_IRQHandler(&cfg->init);
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

static uint32_t can_length_to_dlc(uint32_t length) {
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

static uint32_t can_dlc_to_length(uint32_t dlc) {
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

static int can_clock_init(can_t *cfg) {
    __HAL_RCC_FDCAN_CLK_ENABLE();
    return 0;
}
