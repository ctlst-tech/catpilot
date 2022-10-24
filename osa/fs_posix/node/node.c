#include <node.h>

static struct node *__node;

static struct node *node_create(const char *name);
static struct node *node_set(const char *name, struct node *node);
static struct node *node_get(const char *name, struct node *node);
static int node_transform_path(char *path);

struct node *node_init(void) {
    if (__node == NULL) {
        __node = node_create("");
        if (__node == NULL) {
            errno = ENOMEM;
        }
    }
    return __node;
}

struct node *node_reg(const char *path, struct file_operations *f_op) {
    char path_string[NODE_MAX_NAME_LENGTH] = {};
    char *ptr = path_string;
    struct node *node = __node;
    int length = 0;

    if (path == NULL || f_op == NULL || node == NULL) {
        errno = ENOENT;
        return NULL;
    }

    if (strlen(path) > NODE_MAX_NAME_LENGTH) {
        errno = EINVAL;
        return NULL;
    }

    strcpy(path_string, path);
    if ((length = node_transform_path(path_string)) < 0) {
        errno = EINVAL;
        return NULL;
    }

    while (node != NULL) {
        node = node_get(ptr, node);
        ptr += strlen(ptr) + 1;
        if (node == NULL || ptr >= path_string + length) {
            break;
        }
        node = node->child;
    }

    if (node == NULL) {
        node = __node;
        ptr = path_string;
    } else {
        errno = EEXIST;
        node = NULL;
    }

    while (ptr < (path_string + length) && node != NULL) {
        node = node_set(ptr, node);
        ptr += strlen(ptr) + 1;
    }

    if (node != NULL) {
        memcpy(&node->f_op, f_op, sizeof(struct file_operations));
    }

    return node;
}

struct node *node_find(const char *path) {
    char path_string[NODE_MAX_NAME_LENGTH] = {};
    char *ptr = path_string;
    struct node *node = __node;
    int length = 0;

    if (path == NULL || node == NULL) {
        errno = ENOENT;
        return NULL;
    }

    strncpy(path_string, path, NODE_MAX_NAME_LENGTH);
    if ((length = node_transform_path(path_string)) < 0) {
        errno = EINVAL;
        return NULL;
    }

    while (node != NULL) {
        node = node_get(ptr, node);
        ptr += strlen(ptr) + 1;
        if (node == NULL || ptr >= path_string + length) {
            break;
        }
        node = node->child;
    }

    if (node == NULL) {
        errno = ENOENT;
    }

    return node;
}

static struct node *node_create(const char *name) {
    struct node *node = calloc(1, sizeof(struct node));
    if (node != NULL) {
        strncpy(node->name, name, NODE_MAX_NAME_LENGTH);
    }
    return node;
}

static int node_transform_path(char *path) {
    int length = strlen(path);
    int counter = 0;
    char prev_char = 0;

    if (length <= NODE_MIN_NAME_LENGTH ||
        path[0] != '/') {
        length = -1;
    }

    while (counter < length) {
        if (path[counter] == '/') {
            if (prev_char == '/') {
                length = -1;
            }
            prev_char = path[counter];
            path[counter] = '\0';
        } else {
            prev_char = path[counter];
        }
        counter++;
    }

    return length;
}

static struct node *node_set(const char *name, struct node *node) {
    struct node *next_node;
    const char *next_name = name + strlen(name) + 1;

    if (!strcmp(node->name, name)) {
        next_node = node->child;
        if (next_node == NULL) {
            if (strlen(next_name) > 0) {
                next_node = node_create(next_name);
                next_node->parent = node;
                node->child = next_node;
            } else {
                next_node = node;
            }
        }
    } else {
        next_node = node->sibling;
        if (next_node == NULL) {
            next_node = node_create(name);
            next_node->parent = node->parent;
            node->sibling = next_node;
        }
        next_node = node_set(name, next_node);
    }

    return next_node;
}

static struct node *node_get(const char *name, struct node *node) {
    struct node *next_node;
    next_node = node;

    if (strcmp(node->name, name)) {
        next_node = node->sibling;
        if (next_node != NULL) {
            next_node = node_get(name, next_node);
        }
    }

    return next_node;
}
