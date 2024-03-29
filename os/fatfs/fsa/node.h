#ifndef NODE_H_
#define NODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdarg.h>

#define NODE_MAX_NAME_LENGTH 64
#define NODE_MODE_NEAREST_PATH 0
#define NODE_MODE_FULL_PATH 1

struct file {
    unsigned int flags;
    struct node *node;
    void *private_data;
};

struct file_operations {
    int (*open)(struct file *file, const char *path);
    ssize_t (*write)(struct file *file, const char *buf, size_t count);
    ssize_t (*read)(struct file *file, char *buf, size_t count);
    int (*close)(struct file *file);
    int (*ioctl)(struct file *file, int request, va_list args);
    int (*flush)(struct file *file);
    int (*fsync)(struct file *file);
    int (*lseek)(struct file *file, off_t offset, int whence);
    int (*mkdir)(const char *path, mode_t mode);
    int (*rmdir)(const char *path);
    void *dev;
};

struct node {
    char name[NODE_MAX_NAME_LENGTH];
    struct file_operations f_op;
    struct node *parent;
    struct node *sibling;
    struct node *child;
};

struct node *node_mount(const char *mounting_point,
                        const struct file_operations *f_op);
struct node *node_find(const char *path, int mode);

#ifdef __cplusplus
}
#endif

#endif  // NODE_H_
