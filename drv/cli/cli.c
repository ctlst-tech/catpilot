#include "cli.h"
#include <node.h>
#include <sys/termios.h>

static char *device = "CLI";

// Data strucutres
cli_cfg_t cli_cfg;
FILE stdin_stream;
FILE stdout_stream;
FILE stderr_stream;
char stdin_buf[256];
char stdout_buf[256];
char stderr_buf[256];

// Private functions
void CLI_StreamInit(void);

// Sync
static SemaphoreHandle_t cli_put_mutex;

// Public functions
int CLI_Init(usart_cfg_t *usart) {
    if(usart == NULL) return -1;
    cli_cfg.usart = usart;

    int nd = nodereg("/dev/stdout");
    noderegopen(nd, usart_posix_open);
    noderegwrite(nd, usart_posix_write);
    noderegread(nd, usart_posix_read);
    noderegclose(nd, usart_posix_close);
    noderegfilealloc(nd, NULL);
    noderegdevcfg(nd, cli_cfg.usart);

    cli_cfg.fd = open("/dev/stdout", O_RDWR | O_CREAT | O_TRUNC);
    struct termios termios_p = {};
    tcgetattr(cli_cfg.fd, &termios_p);
    cfsetispeed(&termios_p, 115200U);
    cfsetospeed(&termios_p, 115200U);
    tcsetattr(cli_cfg.fd, TCSANOW, &termios_p);
    tcflush(cli_cfg.fd, TCIOFLUSH);

    CLI_StreamInit();

    if(cli_put_mutex == NULL) cli_put_mutex = xSemaphoreCreateMutex();

    return 0;
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

// Private functions
void CLI_StreamInit(void) {
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
