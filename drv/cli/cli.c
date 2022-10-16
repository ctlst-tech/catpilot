#include "cli.h"
#include <node.h>
#include <sys/termios.h>
#include "monitor.h"

static char *device = "CLI";

// Data strucutres
cli_cfg_t cli_cfg;
FILE stdin_stream;
FILE stdout_stream;
FILE stderr_stream;
char stdin_buf[256];
char stdout_buf[256];
char stderr_buf[256];
static uint8_t buf_read[1024];
static uint8_t buf_write[1024];

// Private functions
static void CLI_StreamInit(void);
void CLI_EchoThread(void *ptr);

// Sync
static SemaphoreHandle_t cli_put_mutex;

// Public functions
int CLI_Init(usart_cfg_t *usart) {
    if(usart == NULL) return -1;
    cli_cfg.usart = usart;

    int nd = nodereg("/dev/cli");
    noderegopen(nd, usart_posix_open);
    noderegwrite(nd, usart_posix_write);
    noderegread(nd, usart_posix_read);
    noderegclose(nd, usart_posix_close);
    noderegfilealloc(nd, NULL);
    noderegdevcfg(nd, cli_cfg.usart);

    cli_cfg.fd = open("/dev/cli", O_RDWR);

    struct termios termios_p = {};

    tcgetattr(cli_cfg.fd, &termios_p);
//    cfsetispeed(&termios_p, 115200U);
//    cfsetospeed(&termios_p, 115200U);
    cfsetispeed(&termios_p, 57600U);
    cfsetospeed(&termios_p, 57600U);
    tcsetattr(cli_cfg.fd, TCSANOW, &termios_p);
    tcflush(cli_cfg.fd, TCIOFLUSH);

    CLI_StreamInit();

    if(cli_put_mutex == NULL) cli_put_mutex = xSemaphoreCreateMutex();

    return 0;
}

void _putchar(char character) {
    char crlf[2] = {'\r', '\n'};
    char data;
    char *ptr;
    int length;

    data = character;
    if(data == '\n') {
        ptr = crlf;
        length = 2;
    } else {
        ptr = &data;
        length = 1;
    }

    write(cli_cfg.fd, (uint8_t *)ptr, length);
}

// TODO add check transmit/receive status
int CLI_Put(char c, struct __file * file) {
    xSemaphoreTake(cli_put_mutex, portMAX_DELAY);
    int len = 0;
    file->buf[len] = c;
    if((c == '\n')) {
        len++;
        file->buf[len] = '\r';
    }
    write(cli_cfg.fd, (uint8_t *)file->buf, (uint16_t)(len + 1));
    file->len = 0;
    xSemaphoreGive(cli_put_mutex);
    return 0;
}

int CLI_Get(struct __file * file) {
    uint8_t *ptr = (uint8_t *)&file->buf;
    read(cli_cfg.fd, ptr, 1);
    file->len++;
    return file->buf[0];
}

int CLI_EchoStart(void) {
    int rv;
    rv = xTaskCreate(CLI_EchoThread, "CLI_Echo", 512, NULL, 1, NULL);
    if(rv != pdPASS) {
        return -1;
    }
    return 0;
}

// Private functions
static void CLI_StreamInit(void) {
    int rv;
    stdin = &stdin_stream;
    stdin->get = CLI_Get;
    stdin->buf = stdin_buf;
    stdin->size = sizeof(stdin_buf);
    stdin->len = 0;
    stdin->flags = __SRD;

    stdout = &stdout_stream;
    stdout->put = CLI_Put;
    stdout->buf = stdout_buf;
    stdout->size = sizeof(stdout_buf);
    stdout->len = 0;
    stdout->flags = __SWR | __SRD;

    stderr = &stderr_stream;
    stderr->put = CLI_Put;
    stderr->buf = stderr_buf;
    stderr->size = sizeof(stderr_buf);
    stderr->len = 0;
    stderr->flags = __SWR | __SRD;
}

int allow_stream = 0;

#define CMD_STR_LEN 120
char cmd_str [CMD_STR_LEN + 1];
int cmd_str_offset = 0;

const char *core_swsys_set_params(const char *task_name, const char *cmd);

static void process_command(const char *cmd) {
    const char *s;
    if (strstr(cmd, "stop") == cmd) {
        allow_stream = 0;
    } else if (strstr(cmd, "start") == cmd) {
        allow_stream = -1;
    } else if (strstr(cmd, "param") == cmd) {
        char task_name[32]; // FIXME check max len
        char cmd_str[64]; // FIXME check max len
        // FIXME need proper shifting
        /*
         * param control_flow func_name=pid_angrate_x Kp=0.65
         * param control_flow func_name=pid_angrate_x Ki=0.65
         * param control_flow func_name=pid_angrate_x fake_param=0.65
         * param fake_task func_name=pid_angrate_x Kp=0.65
         * param control_flow func_name=fake_func Kp=0.65
         */
        if (sscanf(cmd + strlen("param"), " %[A-Za-z0-9_] %[^\n]", task_name, cmd_str) < 2){
            CLI_write_string("\n\rInvalid \"param\" command format: task_name param_alias=value\n\r");
            return;
        }

        // FIXME process parameters after task execution, now it is not thread safe
        const char *rv = core_swsys_set_params(task_name, cmd_str);
        CLI_write_string("\n\r");
        CLI_write_string(rv);
        CLI_write_string("\n\r");
    } else if (strstr(cmd, "monitor") == cmd) {
        Monitor_Update();
    } else {
        CLI_write_string("\n\rInvalid command\n\r");
    }
}


void CLI_EchoThread(void *ptr) {
    int rd_len, wr_len;
    char new_line[4] = "\r\n# ";
    while(1) {
        rd_len = 0;
        wr_len = 0;
        rd_len = read(cli_cfg.fd, buf_read, 1024);
        for(int i = 0; i < rd_len; i++) {
            if(buf_read[i] == '\r') {
                memcpy(buf_write + wr_len, new_line, sizeof(new_line));
                wr_len += sizeof(new_line);

                if (cmd_str_offset > 0) {
                    process_command(cmd_str);
                    cmd_str_offset = 0;
                }
            } else {
                buf_write[wr_len] = buf_read[i];
                wr_len++;

                cmd_str[cmd_str_offset] = buf_read[i];
                if (cmd_str_offset < CMD_STR_LEN) {
                    cmd_str_offset++;
                }
            }
        }
        wr_len = write(cli_cfg.fd, buf_write, wr_len);
    }
}

void CLI_write_string(const char *str) {

    int bw = write(cli_cfg.fd, str, strlen(str));
    if (bw < 0) {
        write(cli_cfg.fd, str, 0);
        return;
    }
}

void CLI_write_stream_string(const char *str) {
    if (allow_stream) {
        CLI_write_string(str);
    }
}
