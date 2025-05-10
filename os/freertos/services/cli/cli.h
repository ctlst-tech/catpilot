#ifndef CLI_H_
#define CLI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CLI_MAX_ARGS 10
#define CLI_MAX_NAME_LENGTH 64
#define CLI_MAX_CMD_LENGTH 256
#define CLI_GET_TOKEN_DEFAULT 0
#define CLI_GET_TOKEN_REPLACE_SPACE 1

struct cli_node {
    char name[CLI_MAX_NAME_LENGTH];
    int (*handler)(int argc, char **argv);
    struct cli_node *sibling;
};

struct cli_service {
    int buf_size;
    char *rbuf;
    char *wbuf;
    char *cmd;
    char **cmd_prev;
    int rlen;
    int wlen;
    int cmd_len;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int history_size;
    int history_pos;
};

struct cli_node *cli_cmd_reg(char *cmd_name,
                             int (*handler)(int argc, char **argv));
struct cli_node *cli_cmd_find(char *cmd_name);

int cli_service_start(int buf_size, int history_size, int priority);
int cli_cmd_execute(char *cmd);
int cli_cmd_print(void);
int cli_cmd_init(void);

#ifdef __cplusplus
}
#endif

#endif  // CLI_H_
