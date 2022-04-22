#include "cli.h"
#include "cli_conf.h"

static char *device = "CLI";

gpio_cfg_t gpio_cli_tx = GPIO_USART7_TX;
gpio_cfg_t gpio_cli_rx = GPIO_USART7_RX;

dma_cfg_t dma_cli_tx;
dma_cfg_t dma_cli_rx;

usart_cfg_t cli_cfg;

FILE stdin_stream;
char stdin_buf[256];

FILE stdout_stream;
char stdout_buf[256];

FILE stderr_stream;
char stderr_buf[256];

void stream_init(){
    stdin = &stdin_stream;
    stdin->get = cli_get;
    stdin->buf = stdin_buf;
    stdin->size = 256;
    stdin->flags = __SWR;

    stdout = &stdout_stream;
    stdout->put = cli_put;
    stdout->buf = stdout_buf;
    stdout->size = 256;
    stdout->flags = __SWR;

    stderr = &stderr_stream;
    stderr->put = cli_put;
    stderr->buf = stderr_buf;
    stderr->size = 256;
    stderr->flags = __SWR;
}

int CLI_Init() {
    stream_init();
    return rv;
}

// TODO add check transmit/receive status
int cli_put(char c, struct __file * file) {
    USART_Transmit(&cli_cfg, (uint8_t *)&c, 1);
    file->len--;
    if(c == '\n') USART_Transmit(&cli_cfg, (uint8_t *)"\r", 1);
    return 0;
}

int cli_get(struct __file * file) {
    uint8_t *ptr = (uint8_t *)&file->buf;
    USART_Receive(&cli_cfg, ptr, 1);
    file->len++;
    return file->buf[0];
}

void cli_put_char(uint8_t c) {
    USART_Transmit(&cli_cfg, &c, 1);
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
