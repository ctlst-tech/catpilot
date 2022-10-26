#include <node.h>

static struct node *__node;

static struct node *node_create(const char *name, size_t length);
static struct node *node_create_dir(struct node *first_sibling_node, 
                                    const char *dir_name,
                                    size_t length);
static struct node *node_find_sibling(struct node *first_sibling_node,
                                      const char *dir_name, 
                                      size_t length);
static struct node *node_add_sibling(struct node *first_sibling_node,
                                     struct node *sibling,
                                     const char *dir_name, 
                                     size_t length);
static int node_get_token(const char **path, size_t *length, 
                          const char **context);

struct node *node_init(void) {
    if (__node == NULL) {
        __node = node_create("", 0);
        if (__node == NULL) {
            errno = ENOMEM;
        }
    }
    return __node;
}

struct node *node_mount(const char *mounting_point, 
                        struct file_operations *f_op) {
    struct node *node = __node, *current_node = NULL;
    const char *dir_name = mounting_point, *dir_context = NULL;
    size_t length;
    int chain_flag = 0;

    if (mounting_point == NULL || f_op == NULL || node == NULL) {
        errno = EINVAL;
        return NULL;
    }

    node = node_find(mounting_point);

    if (node == NULL) {
        node = __node;
    } else {
        errno = EEXIST;
        node = NULL;
    }
    // for(node_get_token(&dir_name, &length, &dir_context); 
    //     length != 0 && dir_name != NULL;
    //     node_get_token(&dir_name, &length, &dir_context)) {
    //         if(chain_flag == 1) {
    //             current_node = node_create(dir_name, length);
    //         }
    //         node = node->child;
    //         if(node == NULL) {
    //             break;
    //         }
    //         current_node = node_find_sibling(node, dir_name, length);
    //         if(current_node == NULL) {
    //             current_node = node_create(dir_name, length);
    //             node_add_sibling(node, current_node, dir_name, length);
    //         } 
    //         if(current_node->child == NULL) {
    //             chain_flag = 1;
    //         }
    // }

    if (node != NULL) {
        memcpy(&node->f_op, f_op, sizeof(struct file_operations));
        errno = 0;
    }

    return node;
}

struct node *node_find(const char *path) {
    struct node *node = __node;
    const char *dir_name = path, *dir_context = NULL;
    size_t length;

    if (path == NULL || node == NULL) {
        errno = EINVAL;
        return NULL;
    }

    for(node_get_token(&dir_name, &length, &dir_context); 
        length != 0 && dir_name != NULL;
        node_get_token(&dir_name, &length, &dir_context)) {
            if (node->child == NULL && node == NULL) {
                break;
            }
            node = node->child;
            node = node_find_sibling(node, dir_name, length);
    }

    if (node == NULL) {
        errno = ENOENT;
    } else {
        errno = 0;
    }

    return node;
}

static struct node *node_create(const char *name, size_t length) {
    struct node *node = calloc(1, sizeof(struct node));
    if (node != NULL) {
        strncpy(node->name, name, length);
    }
    return node;
}

static int node_get_token(const char **path, size_t *length, 
                          const char **context) {
    int l = 0;
    const char *ptr_start;
    const char *ptr_end;

    if (*(context) == NULL) {
        ptr_start = *(path);
    } else {
        ptr_start = *(context);
    }

    for (; (*(ptr_start) == '/'); ptr_start++, l++) {
        if (l > NODE_MAX_NAME_LENGTH) {
            errno = EINVAL;
            *(path) = NULL;
            *(length) = 0;
            *(context) = NULL;
            return -1;
        }
    }

    for (ptr_end = ptr_start, l = 0; 
        (*(ptr_end) != '/') && (*(ptr_end) != '\0'); 
        ptr_end++, l++) {
            if (l > NODE_MAX_NAME_LENGTH) {
                errno = EINVAL;
                *(path) = NULL;
                *(length) = 0;
                *(context) = NULL;
                return -1;
            }
    }

    errno = 0;
    *(path) = ptr_start;
    *(length) = l;
    *(context) = ptr_end;
    return 0;
}

static struct node *node_find_sibling(struct node *first_sibling_node,
                                      const char *dir_name, 
                                      size_t length) {
    struct node *n;
    for (n = first_sibling_node; n != NULL; n = n->sibling) {
        if (!strncmp(n->name, dir_name, length)) {
            break;
        }
    }
    return n;
}

static struct node *node_add_sibling(struct node *first_sibling_node,
                                     struct node *sibling,
                                     const char *dir_name, 
                                     size_t length) {
    struct node *n;
    for (n = first_sibling_node; n->sibling != NULL; n = n->sibling);
    n->sibling = sibling;
    sibling->parent = n->parent;
    return n->sibling;
}
