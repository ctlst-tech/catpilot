#include "cli.h"

char *cli_cmd_get_token(char *cmd, size_t *rv_length, char **parse_context,
                        int mode);

static struct cli_node root_node = {
    .name = "",
    .handler = NULL,
    .sibling = NULL,
};

struct cli_node *cli_node_get_root() {
    return &root_node;
}

static int cli_cmd_check_name(const char *name);
static struct cli_node *node_create(const char *name, size_t name_length);
static struct cli_node *node_find_sibling(struct cli_node *first_sibling_node,
                                          const char *dir_name, size_t length);
static struct cli_node *node_add_sibling(struct cli_node *first_sibling_node,
                                         struct cli_node *new_node);

struct cli_node *cli_cmd_reg(char *cmd_name,
                             int (*handler)(int argc, char **argv)) {
    if (cmd_name == NULL || handler == NULL) {
        errno = EINVAL;
        return NULL;
    }
    if (cli_cmd_check_name(cmd_name)) {
        errno = EINVAL;
        return NULL;
    }
    if (cli_cmd_find(cmd_name) != NULL) {
        errno = EEXIST;
        return NULL;
    }

    struct cli_node *rnode = &root_node;
    struct cli_node *cli_node = NULL;
    size_t cmd_name_length;

    cmd_name_length = strnlen(cmd_name, CLI_MAX_NAME_LENGTH);
    cli_node = node_create(cmd_name, cmd_name_length);
    cli_node = node_add_sibling(rnode, cli_node);

    if (cli_node != NULL) {
        cli_node->handler = handler;
        errno = 0;
    }

    return cli_node;
}

struct cli_node *cli_cmd_find(char *cmd_name) {
    if (cmd_name == NULL) {
        errno = EINVAL;
        return NULL;
    }

    struct cli_node *rnode = &root_node;
    struct cli_node *cli_node = NULL;
    size_t cmd_name_length;
    char *cmd = cmd_name;
    char *context = NULL;

    cmd = cli_cmd_get_token(cmd, &cmd_name_length, &context,
                            CLI_GET_TOKEN_DEFAULT);
    cli_node = node_find_sibling(rnode, cmd_name, cmd_name_length);

    if (cli_node == NULL) {
        errno = ENOENT;
    } else {
        errno = 0;
    }

    return cli_node;
}

int cli_cmd_execute(char *cmd) {
    struct cli_node *cli_node;
    int rv;
    errno = 0;

    if (cmd == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (*cmd == '\0') {
        errno = EAGAIN;
        return -1;
    }

    cli_node = cli_cmd_find(cmd);

    if (cli_node == NULL) {
        errno = ENOENT;
        return -1;
    }

    if (cli_node->handler == NULL) {
        errno = ESRCH;
        return -1;
    }

    int argc = 0;
    char *argv[CLI_MAX_ARGS] = {0};
    char *context = NULL;
    size_t length = 0;

    while ((argv[argc] = cli_cmd_get_token(
                cmd, &length, &context, CLI_GET_TOKEN_REPLACE_SPACE)) != NULL) {
        argc++;
    }

    rv = cli_node->handler(argc, argv);

    return rv;
}

char *cli_cmd_get_token(char *cmd, size_t *rv_length, char **parse_context,
                        int mode) {
    int l = 0;
    char *token_start = cmd;

    if (*parse_context == NULL) {
        token_start = cmd;
    } else {
        token_start = *(parse_context);
    }

    while (*token_start == ' ') {
        token_start++;
    }

    char *token_end = strstr(token_start, " ");

    if (token_end == NULL) {
        token_end = token_start + strnlen(token_start, CLI_MAX_NAME_LENGTH);
    } else if (mode == CLI_GET_TOKEN_REPLACE_SPACE) {
        *token_end = '\0';
        token_end++;
    }

    errno = 0;
    *(rv_length) = token_end - token_start;
    *(parse_context) = token_end;
    return (*rv_length == 0 ? NULL : token_start);
}

int cli_cmd_print(void) {
    printf("Available commands:\n");
    for (struct cli_node *cli_node = root_node.sibling; cli_node != NULL;
         cli_node = cli_node->sibling) {
        printf("\t%s\n", cli_node->name);
    }
    return 0;
}

static int cli_cmd_check_name(const char *name) {
    size_t length = strnlen(name, CLI_MAX_NAME_LENGTH);
    for (size_t i = 0; i < length; i++) {
        if (name[i] == ' ') {
            return -1;
        }
    }
    return 0;
}

static struct cli_node *node_create(const char *name, size_t name_length) {
    if (name == NULL) {
        return NULL;
    }
    struct cli_node *cli_node = calloc(1, sizeof(struct cli_node));
    if (cli_node != NULL) {
        strncpy(cli_node->name, name, name_length);
    }
    return cli_node;
}

static struct cli_node *node_find_sibling(struct cli_node *first_sibling_node,
                                          const char *dir_name, size_t length) {
    for (struct cli_node *n = first_sibling_node; n != NULL; n = n->sibling) {
        if (strlen(n->name) == length) {
            if (strncmp(n->name, dir_name, length) == 0) {
                return n;
            }
        }
    }
    return NULL;
}

static struct cli_node *node_add_sibling(struct cli_node *first_sibling_node,
                                         struct cli_node *new_node) {
    struct cli_node *n;
    for (n = first_sibling_node; n->sibling != NULL; n = n->sibling) {
    }
    n->sibling = new_node;
    return new_node;
}
