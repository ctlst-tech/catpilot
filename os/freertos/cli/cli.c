#include "cli.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct cli_node root_node = {
    .name = "",
    .parent = NULL,
    .child = NULL,
    .sibling = NULL,
    .handler = NULL,
};

struct cli_node *cli_node_get_root() {
    return &root_node;
}

static struct cli_node *node_create(const char *name, size_t name_length);

static struct cli_node *node_find_sibling(struct cli_node *first_sibling_node,
                                          const char *dir_name, size_t length);
static struct cli_node *node_add_sibling(struct cli_node *first_sibling_node,
                                         struct cli_node *new_node);
static struct cli_node *node_add_child(struct cli_node *parent_node,
                                       struct cli_node *child_node);
static const char *node_get_token(const char *full_path, size_t *rv_length,
                                  const char **parse_context);

struct cli_node *cli_cmd_reg(const char *cmd_path,
                             int (*handler)(int argc, char **argv)) {
    if (cmd_path == NULL || handler == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (cli_cmd_find(cmd_path, CLI_MODE_FULL_PATH) != NULL) {
        errno = EEXIST;
        return NULL;
    }

    struct cli_node *rnode = &root_node;
    const char *dir_context = NULL;
    size_t dir_name_length;
    const char *cmd_name;
    struct cli_node *cli_node;

    while ((cmd_name = node_get_token(cmd_path, &dir_name_length,
                                      &dir_context)) != NULL) {
        cli_node = node_find_sibling(rnode->child, cmd_name, dir_name_length);
        if (cli_node == NULL) {
            cli_node = node_create(cmd_name, dir_name_length);
            if (cli_node == NULL) {
                break;
            }
            node_add_child(rnode, cli_node);
        }
        rnode = cli_node;
    }

    if (cli_node != NULL) {
        cli_node->handler = handler;
        errno = 0;
    }

    return cli_node;
}

struct cli_node *cli_cmd_find(const char *path, int mode) {
    if (path == NULL) {
        errno = EINVAL;
        return NULL;
    }

    struct cli_node *node = &root_node;
    struct cli_node *prev_node = NULL;
    const char *dir_context = NULL;
    size_t dir_name_length;
    const char *dir_name;

    while ((dir_name = node_get_token(path, &dir_name_length, &dir_context)) !=
           NULL) {
        node = node_find_sibling(node->child, dir_name, dir_name_length);
        if (node == NULL) {
            if (mode == CLI_MODE_NEAREST_PATH) {
                node = prev_node;
            }
            break;
        }
        prev_node = node;
    }

    if (node == NULL) {
        errno = ENOENT;
    } else {
        errno = 0;
    }

    return node;
}

int cli_cmd_execute(const char *cmd_path, int argc, char **argv) {
    struct cli_node *cli_node;
    int rv;

    if (cmd_path == NULL) {
        errno = EINVAL;
        return -1;
    }

    cli_node = cli_cmd_find(cmd_path, CLI_MODE_FULL_PATH);

    if (cli_node->handler == NULL) {
        errno = ESRCH;
        return -1;
    }

    rv = cli_node->handler(argc, argv);

    return rv;
}

int cli_show_subcmd(const char *subsystem_name) {
    struct cli_node *cli_node;
    struct cli_node *cli_node_par;

    cli_node = cli_cmd_find(subsystem_name, CLI_MODE_FULL_PATH);
    cli_node_par = cli_node;

    if (cli_node == NULL) {
        return -1;
    }

    printf("Available commands");

    if(cli_node != &root_node) {
        printf(" for \"");
        while(cli_node_par != &root_node) {
            printf("%s", cli_node_par->name);
            cli_node_par = cli_node_par->parent;
            if(cli_node_par != &root_node) {
                printf(" ");
            }
        }
        printf("\":");
    } else {
        printf(":");
    }
    printf("\n");

    cli_node = cli_node->child;

    while (cli_node != NULL) {
        printf("\t%s\n", cli_node->name);
        cli_node = cli_node->sibling;
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

static const char *node_get_token(const char *full_path, size_t *rv_length,
                                  const char **parse_context) {
    int l = 0;
    const char *token_start;

    if (*parse_context == NULL) {
        token_start = full_path;
    } else {
        token_start = *(parse_context);
    }

    // handle repetitive spaces
    while (*token_start == ' ') {
        token_start++;
    }

    const char *token_end;
    token_end = strstr(token_start, " ");
    if (token_end == NULL) {
        token_end =
            token_start +
            strlen(token_start);  // last token, token end points to '\0'
    }

    errno = 0;
    *(rv_length) = token_end - token_start;
    *(parse_context) = token_end;
    return (*rv_length == 0 ? NULL : token_start);
}

static struct cli_node *node_find_sibling(struct cli_node *first_sibling_node,
                                          const char *dir_name, size_t length) {
    for (struct cli_node *n = first_sibling_node; n != NULL; n = n->sibling) {
        if (strncmp(n->name, dir_name, length) == 0) {
            return n;
        }
    }
    return NULL;
}

static struct cli_node *node_add_sibling(struct cli_node *first_sibling_node,
                                         struct cli_node *new_node) {
    struct cli_node *n;
    for (n = first_sibling_node; n->sibling != NULL; n = n->sibling);
    n->sibling = new_node;
    new_node->parent = n->parent;
    return new_node;
}

static struct cli_node *node_add_child(struct cli_node *parent_node,
                                       struct cli_node *child_node) {
    struct cli_node *n;
    if (parent_node->child == NULL) {
        parent_node->child = child_node;
        child_node->parent = parent_node;
    } else {
        node_add_sibling(parent_node->child, child_node);
    }
    return child_node;
}
