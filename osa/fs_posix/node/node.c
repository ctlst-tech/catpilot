#include <node.h>

static struct node *__node;

static struct node *node_create(const char *name, struct node *parent);
static struct node *node_create_dir(const char *name, struct node *node);
static struct node *node_get_sibling(const char *name, struct node *node);
static int node_transform_path(char *dest, const char *src);

struct node *node_init(void) {
    if (__node == NULL) {
        __node = node_create("", NULL);
        if (__node == NULL) {
            errno = ENOMEM;
        }
    }
    return __node;
}

struct node *node_reg(const char *path, struct file_operations *f_op) {
    char path_string[NODE_MAX_NAME_LENGTH] = { 0 };
    char *dir_name = path_string;
    struct node *node = __node;
    int length = 0;

    if (path == NULL || f_op == NULL || node == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (strlen(path) >= NODE_MAX_NAME_LENGTH) {
        errno = EINVAL;
        return NULL;
    }

    if ((length = node_transform_path(path_string, path)) < 0) {
        errno = EINVAL;
        return NULL;
    }

    for (node = __node; node != NULL; node = node->child) {
        node = node_get_sibling(dir_name, node);
        dir_name += strlen(dir_name) + 1;
        if (node == NULL || dir_name >= path_string + length) {
            break;
        }
    }

    if (node == NULL) {
        node = __node;
        dir_name = path_string;
    } else {
        errno = EEXIST;
        node = NULL;
    }

    for (; node != NULL; node = node->child) {
        node = node_create_dir(dir_name, node);
        dir_name += strlen(dir_name) + 1;
        if (node == NULL || dir_name >= path_string + length) {
            break;
        }
    }

    if (node != NULL) {
        memcpy(&node->f_op, f_op, sizeof(struct file_operations));
        errno = 0;
    }

    return node;
}

struct node *node_find(const char *path) {
    char path_string[NODE_MAX_NAME_LENGTH] = { 0 };
    char *dir_name = path_string;
    struct node *node = __node;
    int length = 0;

    if (path == NULL || node == NULL) {
        errno = ENOENT;
        return NULL;
    }

    if ((length = node_transform_path(path_string, path)) < 0) {
        errno = EINVAL;
        return NULL;
    }

    for (node = __node; node != NULL; node = node->child) {
        node = node_get_sibling(dir_name, node);
        dir_name += strlen(dir_name) + 1;
        if (node == NULL || dir_name >= path_string + length) {
            break;
        }
    }

    if (node == NULL) {
        errno = ENOENT;
    } else {
        errno = 0;
    }

    return node;
}

static struct node *node_create(const char *name, struct node *parent) {
    struct node *node = calloc(1, sizeof(struct node));
    if (node != NULL) {
        strncpy(node->name, name, NODE_MAX_NAME_LENGTH);
        node->parent = parent;
    }
    return node;
}

static int node_transform_path(char *dest, const char *src) {
    int length = strlen(src);
    int counter = 0;
    char prev_char = 0;

    if (length >= NODE_MAX_NAME_LENGTH - 2 || length <= NODE_MIN_NAME_LENGTH) {
        length = -1;
    }

    if (src[0] != '/') {
        dest++;
    }

    for (int i = 0; i < length; i++) {
        if (src[i] == '/') {
            if (prev_char == '/') {
                continue;
            }
            dest[counter] = '\0';
        } else {
            dest[counter] = src[i];
        }
        prev_char = src[i];
        counter++;
    }

    return length;
}

static struct node *node_create_dir(const char *name, struct node *node) {
    struct node *n;
    const char *next_name = name + strlen(name) + 1;
    int not_last = strlen(next_name) > 0;

    for (n = node; n != NULL; n = n->sibling) {
        if (!strcmp(n->name, name)) {
            if (n->child == NULL && not_last) {
                n->child = node_create(next_name, n);
            }
            break;
        } else if (n->sibling == NULL) {
            n->sibling = node_create(name, n->parent);
        }
    }
    return n;
}

static struct node *node_get_sibling(const char *name, struct node *node) {
    struct node *n;
    for (n = node; n != NULL && strcmp(n->name, name); n = n->sibling);
    return n;
}
