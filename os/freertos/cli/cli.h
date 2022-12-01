#ifndef CLI_H_
#define CLI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define CLI_MAX_NAME_LENGTH 64
#define CLI_MODE_NEAREST_PATH 0
#define CLI_MODE_FULL_PATH 1

struct cli_node {
    char name[CLI_MAX_NAME_LENGTH];
    int (*handler)(int argc, char **argv);
    struct cli_node *parent;
    struct cli_node *sibling;
    struct cli_node *child;
};

struct cli_node *cli_cmd_reg(const char *cmd_path,
                             int (*handler)(int argc, char **argv));
struct cli_node *cli_cmd_find(const char *path, int mode);
int cli_cmd_execute(const char *cmd_path, int argc, char **argv);
int cli_show_subcmd(const char *subsystem_name);

#ifdef __cplusplus
}
#endif

#endif  // CLI_H_
