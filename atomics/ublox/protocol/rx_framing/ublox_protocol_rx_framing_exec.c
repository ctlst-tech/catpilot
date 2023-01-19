#include <sys/termios.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "ublox_protocol_rx_framing.h"

fspec_rv_t ublox_protocol_rx_framing_pre_exec_init(
    const ublox_protocol_rx_framing_params_t *p,
    ublox_protocol_rx_framing_state_t *s
)
{
    struct termios 	termios_p;

    s->fd = open("/dev/cu.usbmodem2101", O_RDWR);
    if (s->fd < 0) {
        return fspec_rv_system_err;
    }

    int rv = tcgetattr(s->fd, &termios_p);
    if (rv) {
        return fspec_rv_system_err;
    }

    cfsetispeed(&termios_p, p->baudrate);
    cfsetospeed(&termios_p, p->baudrate);
    cfmakeraw(&termios_p);
    rv = tcsetattr(s->fd, TCSANOW, &termios_p);
    if (rv) {
        return fspec_rv_system_err;
    }

    return fspec_rv_ok;
}

typedef struct  __attribute__((packed)) ubx_header {
    uint8_t cls;
    uint8_t id;
    uint16_t len;
} ubx_header_t;

#define UBX_RESET 0
#define UBX_WAIT_SC1 1
#define UBX_WAIT_SC2 2
#define UBX_WAIT_LENGTH 3
#define UBX_WAIT_REST 4

uint16_t ubx_calc_crc(const uint8_t *buf, size_t n) {
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;

    for (int i = 0; i < n; i++) {
        ck_a = ck_a + buf[i];
        ck_b = ck_b + ck_a;
    }

    return ck_b << 8 | ck_a;
}

static unsigned ubx_process(ublox_protocol_rx_framing_state_t *s, ublox_protocol_rx_framing_outputs_t *o, ublox_protocol_rx_framing_outputs_update_flags_t *out_update_cmd) {

    ubx_header_t *hdr =  (void *) s->ubx_buf.vector;
    uint8_t c;
#   define BUF_LEN_TO_PAYLOAD_SIZE(l__) ((l__) - sizeof(*hdr) - 2)

#   define PAYLOAD_BEGIN_OFFSET(buf__) ((buf__) + sizeof(*hdr))
#   define PAYLOAD_END_OFFSET(buf__,pl__) (PAYLOAD_BEGIN_OFFSET(buf__) + (pl__))

    do {
        c = s->rx_buf.vector[s->rx_buf_index];

        switch (s->ubx_rx_state) {
            default:
            case UBX_RESET:
                s->ubx_rx_state = UBX_WAIT_SC1;
                s->ubx_buf.curr_len = 0;
                // fall through ...

            case UBX_WAIT_SC1:
                if (c == 0xb5) {
                    s->ubx_rx_state = UBX_WAIT_SC2;
                }
                break;

            case UBX_WAIT_SC2:
                if (c == 0x62) {
                    s->ubx_rx_state = UBX_WAIT_LENGTH;
                } else {
                    s->ubx_rx_state = UBX_RESET;
                    // suspect other frame
//                    if (s->rx_buf_index > 2) {
//                        s->rx_buf_index -= 2; // try to shift frame back
//                    }
                }
                break;

            case UBX_WAIT_LENGTH:
                s->ubx_buf.vector[s->ubx_buf.curr_len++] = c;
                if (s->ubx_buf.curr_len == 4) {
                    if (hdr->len > BUF_LEN_TO_PAYLOAD_SIZE(s->ubx_buf.max_len)) {
                        // frame is too big for our buffer
                        s->ubx_rx_state = UBX_RESET;
                    } else {
                        s->ubx_rx_state = UBX_WAIT_REST;
                    }
                }
                break;

            case UBX_WAIT_REST:
                s->ubx_buf.vector[s->ubx_buf.curr_len++] = c;
                if (BUF_LEN_TO_PAYLOAD_SIZE(s->ubx_buf.curr_len) == hdr->len) {
                    uint16_t ck = ubx_calc_crc(s->ubx_buf.vector, hdr->len + sizeof(*hdr));
                    if (memcmp(&ck, PAYLOAD_END_OFFSET(s->ubx_buf.vector, hdr->len), 2) == 0) {
                        // got frame
                        memcpy(o->ubx_frame.vector, s->ubx_buf.vector, s->ubx_buf.curr_len);
                        o->ubx_frame.curr_len = s->ubx_buf.curr_len;
                        out_update_cmd->ubx_frame_updated = 1;
                        s->ubx_frame_cnt++;

                        printf("UBX. got frame %02X %02X len == %d | rx_bytes % 6d; frame_cnt % 4d; crc_err %d;\n",
                               hdr->cls, hdr->id, hdr->len,
                               s->ubx_rx_bytes_cnt, s->ubx_frame_cnt, s->ubx_err_crc_cnt);

                    } else {
                        s->ubx_err_crc_cnt++;
                    }
                    s->ubx_rx_state = UBX_RESET;
                }
                break;
        }
        s->rx_buf_index++;

    } while (s->rx_buf_index < s->rx_buf.curr_len &&
                s->ubx_rx_state != UBX_RESET);

    return s->ubx_rx_state;
}

void ublox_protocol_rx_framing_exec(
    ublox_protocol_rx_framing_outputs_t *o,
    const ublox_protocol_rx_framing_params_t *p,
    ublox_protocol_rx_framing_state_t *s,
    ublox_protocol_rx_framing_outputs_update_flags_t *out_update_cmd
)
{

    uint8_t c;
    ssize_t br;

    do {
        if (s->rx_buf_index >= s->rx_buf.curr_len) {
            br = read(s->fd, s->rx_buf.vector, s->rx_buf.max_len);
            if (br < 0) {
                return; // TODO mark a problem
            }
            s->ubx_rx_bytes_cnt += br;
            s->rx_buf.curr_len = br;
            s->rx_buf_index = 0;
        }

        unsigned ubx_state = ubx_process(s, o, out_update_cmd);

    } while(!(out_update_cmd->ubx_frame_updated || out_update_cmd->rtcm_frame_updated));


}
