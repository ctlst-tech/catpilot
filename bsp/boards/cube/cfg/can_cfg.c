#include "board.h"
#include "core.h"
#include "hal.h"

can_t can1 = {
    .name = "can1",
    .alt_name = "CAN1",
    .init = {
        .Instance = FDCAN1,
        .Init = {
            .FrameFormat = FDCAN_FRAME_CLASSIC,
            .Mode = FDCAN_MODE_NORMAL,
            .AutoRetransmission = DISABLE,
            .TransmitPause = DISABLE,
            .ProtocolException = DISABLE,
            .NominalPrescaler = 100,
            .NominalSyncJumpWidth = 1,
            .NominalTimeSeg1 = 6,
            .NominalTimeSeg2 = 2,
            .DataPrescaler = 1,
            .DataSyncJumpWidth = 1,
            .DataTimeSeg1 = 1,
            .DataTimeSeg2 = 1,
            .MessageRAMOffset = 0,
            .StdFiltersNbr = 0,
            .ExtFiltersNbr = 0,
            .RxFifo0ElmtsNbr = 32,
            .RxFifo0ElmtSize = FDCAN_DATA_BYTES_8,
            .RxFifo1ElmtsNbr = 32,
            .RxFifo1ElmtSize = FDCAN_DATA_BYTES_8,
            .RxBuffersNbr = 32,
            .RxBufferSize = FDCAN_DATA_BYTES_8,
            .TxEventsNbr = 32,
            .TxBuffersNbr = 32,
            .TxFifoQueueElmtsNbr = 32,
            .TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION,
            .TxElmtSize = FDCAN_DATA_BYTES_8,
        }
    },
    .tx = &gpio_can1_tx,
    .rx = &gpio_can1_rx,
    .timeout = 20,
    .buf_size = 256,
    .irq_priority = 10,
    .task_priority = 2,
    .verbosity = CAN_VERBOSITY_HIGH,
    .p = {0}
};

can_t can2 = {
    .name = "can2",
    .alt_name = "CAN2",
    .init = {
        .Instance = FDCAN2,
        .Init = {
            .FrameFormat = FDCAN_FRAME_CLASSIC,
            .Mode = FDCAN_MODE_NORMAL,
            .AutoRetransmission = DISABLE,
            .TransmitPause = DISABLE,
            .ProtocolException = DISABLE,
            .NominalPrescaler = 100,
            .NominalSyncJumpWidth = 1,
            .NominalTimeSeg1 = 6,
            .NominalTimeSeg2 = 2,
            .DataPrescaler = 1,
            .DataSyncJumpWidth = 1,
            .DataTimeSeg1 = 1,
            .DataTimeSeg2 = 1,
            .MessageRAMOffset = 0,
            .StdFiltersNbr = 0,
            .ExtFiltersNbr = 0,
            .RxFifo0ElmtsNbr = 32,
            .RxFifo0ElmtSize = FDCAN_DATA_BYTES_8,
            .RxFifo1ElmtsNbr = 32,
            .RxFifo1ElmtSize = FDCAN_DATA_BYTES_8,
            .RxBuffersNbr = 32,
            .RxBufferSize = FDCAN_DATA_BYTES_8,
            .TxEventsNbr = 32,
            .TxBuffersNbr = 32,
            .TxFifoQueueElmtsNbr = 32,
            .TxFifoQueueMode = FDCAN_TX_QUEUE_OPERATION,
            .TxElmtSize = FDCAN_DATA_BYTES_8,
        },
    },
    .tx = &gpio_can2_tx,
    .rx = &gpio_can2_rx,
    .timeout = 20,
    .buf_size = 256,
    .irq_priority = 10,
    .task_priority = 2,
    .verbosity = CAN_VERBOSITY_HIGH,
    .p = {0}
};
