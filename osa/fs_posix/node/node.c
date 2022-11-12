#include "node.h"

#include <stdio.h>

static struct node root_node = {
    .name = "",
    .f_op = NULL,
    .child = NULL,
};

struct node *node_get_root() {
    return &root_node;
}

static struct node *node_create(const char *name, size_t name_length);

static struct node *node_find_sibling(struct node *first_sibling_node,
                                      const char *dir_name, size_t length);
static struct node *node_add_sibling(struct node *first_sibling_node,
                                     struct node *new_node);
static struct node *node_add_child(struct node *parent_node,
                                   struct node *child_node);
static const char *node_get_token(const char *full_path, size_t *rv_length,
                                  const char **parse_context);

struct node *node_mount(const char *mounting_point,
                        const struct file_operations *f_op) {
    if (mounting_point == NULL || f_op == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (node_find(mounting_point, NODE_MODE_FULL_PATH) != NULL) {
        errno = EEXIST;
        return NULL;
    }

    struct node *rnode = &root_node;
    const char *dir_context = NULL;
    size_t dir_name_length;
    const char *dir_name;
    struct node *node;

    while ((dir_name = node_get_token(mounting_point, &dir_name_length,
                                      &dir_context)) != NULL) {
        node = node_find_sibling(rnode->child, dir_name, dir_name_length);
        if (node == NULL) {
            node = node_create(dir_name, dir_name_length);
            if (node == NULL) {
                break;
            }
            node_add_child(rnode, node);
        }
        rnode = node;
    }

    if (node != NULL) {
        node->f_op = *f_op;
        errno = 0;
    }

    return node;
}

struct node *node_find(const char *path, int mode) {
    if (path == NULL) {
        errno = EINVAL;
        return NULL;
    }

    struct node *node = &root_node;
    struct node *prev_node = NULL;
    const char *dir_context = NULL;
    size_t dir_name_length;
    const char *dir_name;

    while ((dir_name = node_get_token(path, &dir_name_length, &dir_context)) !=
           NULL) {
        node = node_find_sibling(node->child, dir_name, dir_name_length);
        if (node == NULL) {
            if (mode == NODE_MODE_NEAREST_PATH) {
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

static struct node *node_create(const char *name, size_t name_length) {
    if (name == NULL) {
        return NULL;
    }
    struct node *node = calloc(1, sizeof(struct node));
    if (node != NULL) {
        strncpy(node->name, name, name_length);
    }
    return node;
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

    // any path must start from '/'
    if (token_start[0] != '/') {
        if (token_start[0] != 0) {
            errno = EINVAL;
        } else {
            errno = 0;  // end of string to parse
        }
        return NULL;
    }

    // handle repetitive slashes
    while (*token_start == '/') {
        token_start++;
    }

    const char *token_end;
    token_end = strstr(token_start, "/");
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

static struct node *node_find_sibling(struct node *first_sibling_node,
                                      const char *dir_name, size_t length) {
    for (struct node *n = first_sibling_node; n != NULL; n = n->sibling) {
        if (strncmp(n->name, dir_name, length) == 0) {
            return n;
        }
    }
    return NULL;
}

static struct node *node_add_sibling(struct node *first_sibling_node,
                                     struct node *new_node) {
    struct node *n;
    for (n = first_sibling_node; n->sibling != NULL; n = n->sibling);
    n->sibling = new_node;
    new_node->parent = n->parent;
    return new_node;
}

static struct node *node_add_child(struct node *parent_node,
                                   struct node *child_node) {
    struct node *n;
    if (parent_node->child == NULL) {
        parent_node->child = child_node;
    } else {
        node_add_sibling(parent_node->child, child_node);
    }
    return child_node;
}
