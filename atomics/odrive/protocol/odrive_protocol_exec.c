#include <fcntl.h>
#include <ioctl.h>

#include "board.h"
#include "odrive_protocol.h"
#include "odrive_protocol_msg.h"

typedef enum {
    ODRIVE_INIT = 0,
    ODRIVE_CONF = 1,
    ODRIVE_UPDATE = 2,
    ODRIVE_FAIL = 3,
} odrive_state_t;

ssize_t odrive_read(int fd, uint32_t id, void *buf, size_t count) {
    ioctl(fd, CAN_IOCTL_SET_TX_MSG_ID, ODRIVE_GET_CMD_ID(0, id));
    ioctl(fd, CAN_IOCTL_SET_TX_MSG_TYPE, CAN_REMOTE_FRAME);
    write(fd, buf, 0);
    return read(fd, buf, count);
}

ssize_t odrive_write(int fd, uint32_t id, void *buf, size_t count) {
    ioctl(fd, CAN_IOCTL_SET_TX_MSG_ID, ODRIVE_GET_CMD_ID(0, id));
    ioctl(fd, CAN_IOCTL_SET_TX_MSG_TYPE, CAN_DATA_FRAME);
    return write(fd, buf, count);
}

void odrive_protocol_fsm(const odrive_protocol_inputs_t *i,
                         odrive_protocol_outputs_t *o,
                         const odrive_protocol_params_t *p,
                         odrive_protocol_state_t *state) {
    switch (state->state) {
        case ODRIVE_INIT:
            state->fd = open(p->can_ch, O_RDWR);
            if (state->fd < 0) {
                state->state = ODRIVE_FAIL;
            }
            ioctl(state->fd, CAN_IOCTL_SET_RX_FILTER_ID,
                  ODRIVE_GET_AXIS_MASK(p->axis));
            uint8_t version[8] = {0};
            odrive_read(state->fd,
                        ODRIVE_GET_CMD_ID(p->axis, ODRIVE_GET_VERSION), version,
                        sizeof(version));
            // Check version
            state->state = ODRIVE_CONF;
            break;
        case ODRIVE_CONF:
            state->state = ODRIVE_UPDATE;
            break;
        case ODRIVE_UPDATE:;
            uint32_t pos = i->input / 1000;
            odrive_write(
                state->fd,
                ODRIVE_GET_CMD_ID(p->axis, ODRIVE_SET_ABSOLUTE_POSITION), &pos,
                sizeof(pos));

            uint32_t volt_cur[2] = {0};
            if (odrive_read(
                    state->fd,
                    ODRIVE_GET_CMD_ID(p->axis, ODRIVE_GET_BUS_VOLTAGE_CURRENT),
                    volt_cur, sizeof(volt_cur)) > 0) {
                o->output = 1.0 * volt_cur[0];
            }
            break;
        case ODRIVE_FAIL:
            break;
        default:
            break;
    }
}

void odrive_protocol_exec(const odrive_protocol_inputs_t *i,
                          odrive_protocol_outputs_t *o,
                          const odrive_protocol_params_t *p,
                          odrive_protocol_state_t *state) {
    odrive_protocol_fsm(i, o, p, state);
    return;
}
