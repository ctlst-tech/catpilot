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
        case ODRIVE_INIT:;
            char main_path[32];
            char tm_path[32];
            snprintf(main_path, sizeof(main_path) - 1, "/dev/%s/ch1",
                     p->can_if);
            snprintf(tm_path, sizeof(tm_path) - 1, "/dev/%s/ch2", p->can_if);
            state->main_ch = open(main_path, O_RDWR);
            state->tm_ch = open(tm_path, O_RDWR);
            if (state->main_ch < 0) {
                state->state = ODRIVE_FAIL;
            }
            if (state->tm_ch < 0) {
                close(state->main_ch);
                state->state = ODRIVE_FAIL;
            }
            ioctl(state->main_ch, CAN_IOCTL_SET_RX_FILTER_ID_LOW,
                  ODRIVE_GET_CMD_ID(p->axis, ODRIVE_GET_VERSION));
            ioctl(state->main_ch, CAN_IOCTL_SET_RX_FILTER_ID_HIGH,
                  ODRIVE_GET_CMD_ID(p->axis, ODRIVE_HEARTBEAT));

            ioctl(state->tm_ch, CAN_IOCTL_SET_RX_FILTER_ID_LOW,
                  ODRIVE_GET_CMD_ID(p->axis, ODRIVE_GET_BUS_VOLTAGE_CURRENT));
            ioctl(state->tm_ch, CAN_IOCTL_SET_RX_FILTER_ID_HIGH,
                  ODRIVE_GET_CMD_ID(p->axis, ODRIVE_GET_BUS_VOLTAGE_CURRENT));
            uint8_t version[8] = {0};
            odrive_read(state->main_ch,
                        ODRIVE_GET_CMD_ID(p->axis, ODRIVE_GET_VERSION), version,
                        sizeof(version));
            // Check version
            state->state = ODRIVE_CONF;
            break;
        case ODRIVE_CONF:
            state->state = ODRIVE_UPDATE;
            break;
        case ODRIVE_UPDATE:;
            gpio_toggle(&gpio_fmu_pwm[2]);
            uint8_t heartbeat[8] = {0};
            read(state->main_ch, heartbeat, sizeof(heartbeat));
            uint32_t pos = i->pos * 1000;
            odrive_write(
                state->main_ch,
                ODRIVE_GET_CMD_ID(p->axis, ODRIVE_SET_ABSOLUTE_POSITION), &pos,
                sizeof(pos));

            float volt_cur[2] = {0};
            if (odrive_read(
                    state->tm_ch,
                    ODRIVE_GET_CMD_ID(p->axis, ODRIVE_GET_BUS_VOLTAGE_CURRENT),
                    volt_cur, sizeof(volt_cur)) > 0) {
                o->vol = volt_cur[0];
                o->cur = volt_cur[1];
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
