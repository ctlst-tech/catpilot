#include "cli.h"
#include "cli_conf.h"

static char *device = "CLI";

extern usart_cfg_t usart7;

FILE stdin_stream;
FILE stdout_stream;
FILE stderr_stream;
char stdin_buf[256];
char stdout_buf[256];
char stderr_buf[256];
char ttyS0_buf[256];

static FATFS fs;

void stream_init(){
    int rv;
    int fd;

    // rv = f_mount(&fs, "0:", 1);
    // rv = mkdir("dev", S_IRWXU);

    stdin = &stdin_stream;
    stdin->get = cli_get;
    stdin->buf = stdin_buf;
    stdin->size = sizeof(stdin_buf);
    stdin->len = 0;
    stdin->flags = __SRD;

    stdout = &stdout_stream;
    stdout->put = cli_put;
    stdout->buf = stdout_buf;
    stdout->size = sizeof(stdout_buf);
    stdout->len = 0;
    stdout->flags = __SWR | __SRD;

    stderr = &stderr_stream;
    stderr->put = cli_put;
    stderr->buf = stderr_buf;
    stderr->size = sizeof(stderr_buf);
    stderr->len = 0;
    stderr->flags = __SWR | __SRD;

    // FILE *ttyS0;
    // ttyS0 = fopen("/dev/ttyS0", "w+");

    // char *blah = "test\n";

    // fwrite(blah, 5, 1, ttyS0);
}

int CLI_Init() {
    int rv = 0;
    stream_init();
    return rv;
}

// TODO add check transmit/receive status
int cli_put(char c, struct __file * file) {
    file->buf[file->len] = c;
    if(c == '\n' || (file->len + 2) >= file->size) {
        file->len++;
        file->buf[file->len] = '\r';
        USART_Transmit(&usart7, (uint8_t *)file->buf, (uint16_t)(file->len + 1));
        file->len = 0;
    }
    return 0;
}

int cli_get(struct __file * file) {
    uint8_t *ptr = (uint8_t *)&file->buf;
    USART_Receive(&usart7, ptr, 1);
    file->len++;
    return file->buf[0];
}

void cli_put_char(uint8_t c) {
    USART_Transmit(&usart7, &c, 1);
}

int _write(int fd, char* ptr, int len)
{
    (void)fd;
    int i = 0;
    while (ptr[i] && (i < len)) {
        cli_put_char((int)ptr[i]);
        if (ptr[i] == '\n') {
            cli_put_char((int)'\r');
        }
        i++;
    }
    return len;
}
