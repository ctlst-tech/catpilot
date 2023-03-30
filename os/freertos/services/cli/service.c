#include "cli.h"

static char nl[2] = "\r\n";
static char inv[2] = "# ";

void *cli_echo(void *arg) {
    struct cli_service *cli = (struct cli_service *)arg;
    pthread_setname_np((char *)__func__);

    if (write(1, nl, sizeof(nl)) < 0) {
        return NULL;
    }
    if (write(1, inv, sizeof(inv)) < 0) {
        return NULL;
    }

    cli->cmd_len = 0;
    cli->cmd_prev_len = 0;

    while (1) {
        cli->rlen = 0;
        cli->wlen = 0;

        cli->rlen = read(0, cli->rbuf, cli->buf_size);

        for (int i = 0; i < cli->rlen; i++) {
            if (cli->rbuf[i] == '\r') {
                cli->cmd[cli->cmd_len] = '\0';
                pthread_mutex_lock(&cli->mutex);
                pthread_cond_broadcast(&cli->cond);
                pthread_mutex_unlock(&cli->mutex);
                if (cli->cmd_len != 0) {
                    strncpy(cli->cmd_prev, cli->cmd, cli->cmd_len);
                    cli->cmd_prev_len = cli->cmd_len;
                }
                cli->cmd_len = 0;
            } else if (cli->rbuf[i] == '\b' || cli->rbuf[i] == 0x7F ||
                       (!strncmp("\033[3~", &cli->rbuf[i], 4))) {
                cli->wbuf[cli->wlen] = '\b';
                cli->wlen++;
                cli->wbuf[cli->wlen] = ' ';
                cli->wlen++;
                cli->wbuf[cli->wlen] = '\b';
                cli->wlen++;
                if (cli->cmd_len > 0) {
                    cli->cmd_len--;
                }
            } else if (!strncmp(cli->rbuf, "\033[A", cli->rlen) &&
                       cli->cmd_len == 0) {
                strncpy(cli->wbuf, cli->cmd_prev, cli->cmd_prev_len);
                strncpy(cli->cmd, cli->cmd_prev, cli->cmd_prev_len);
                cli->wlen = cli->cmd_prev_len;
                cli->cmd_len = cli->cmd_prev_len;
                break;
            } else if (cli->rbuf[i] < ' ' || cli->rbuf[i] > 127) {
                break;
            } else {
                cli->wbuf[cli->wlen] = cli->rbuf[i];
                cli->wlen++;
                if (cli->cmd_len < cli->buf_size) {
                    cli->cmd[cli->cmd_len] = cli->rbuf[i];
                    cli->cmd_len++;
                }
            }
        }

        int wl = write(1, cli->wbuf, cli->wlen);
    }
}

void *cli_invoker(void *arg) {
    int rv;
    struct cli_service *cli = (struct cli_service *)arg;
    pthread_setname_np((char *)__func__);

    while (1) {
        pthread_mutex_lock(&cli->mutex);
        pthread_cond_wait(&cli->cond, &cli->mutex);
        write(1, nl, sizeof(nl));
        if (cli_cmd_execute(cli->cmd)) {
            if (errno != ECHILD) {
                printf("Unknown command: \"%s\"\n", cli->cmd);
                cli_cmd_print();
                write(1, nl, sizeof(nl));
            }
        } else {
            write(1, nl, sizeof(nl));
        }
        pthread_mutex_unlock(&cli->mutex);
        write(1, inv, sizeof(inv));
    }
}

int cli_service_start(int buf_size, int priority) {
    static int init = 0;

    if (init != 0) {
        return 0;
    }

    init = 1;

    struct cli_service *cli = calloc(1, sizeof(struct cli_service));
    if (cli == NULL) {
        return -1;
    }

    cli->buf_size = buf_size;

    cli->rbuf = calloc(cli->buf_size, sizeof(char));
    if (cli->rbuf == NULL) {
        return -1;
    }

    cli->wbuf = calloc(cli->buf_size, sizeof(char));
    if (cli->wbuf == NULL) {
        return -1;
    }

    cli->cmd = calloc(cli->buf_size, sizeof(char));
    if (cli->cmd == NULL) {
        return -1;
    }

    cli->cmd_prev = calloc(cli->buf_size, sizeof(char));
    if (cli->cmd_prev == NULL) {
        return -1;
    }

    pthread_mutex_init(&cli->mutex, NULL);
    pthread_cond_init(&cli->cond, NULL);

    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = priority;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setstacksize(&attr, 2048);
    pthread_create(&tid, &attr, cli_echo, cli);
    pthread_create(&tid, &attr, cli_invoker, cli);

    return 0;
}
