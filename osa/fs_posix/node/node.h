#ifndef _NODE_H_
#define _NODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define NODE_MAX_NAME_LENGTH 32
#define NODE_MIN_NAME_LENGTH 1

#define NODE_REG  0
#define NODE_FIND 1

struct file {
    unsigned int f_flags;
    struct file_operations *f_op;
    struct node *node;
    void *private_data;
};

struct file_operations {
    ssize_t(*read) (struct file *file, char *buf, size_t count);
    ssize_t(*write) (struct file *file, const char *buf, size_t count);
    int (*ioctl) (struct file *file, unsigned int data);
    int (*open) (struct file *file);
    int (*flush) (struct file *file);
    int (*release) (struct file *file);
    int (*fsync) (struct file *file);
};

struct node {
    char name[NODE_MAX_NAME_LENGTH];
    struct file_operations f_op;
    struct node *parent;
    struct node *sibling;
    struct node *child;
};

struct node *node_init(void);
struct node *node_reg(const char *path, struct file_operations *f_op);
struct node *node_find(const char *path);

#ifdef __cplusplus
}
#endif

#endif // _NODE_H_
