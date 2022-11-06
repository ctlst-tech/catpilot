#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "node.h"

struct file __file[MAX_FILES];
struct file *file[MAX_FILES];
static int __fd;

int fd_new(void) {
    int rv = -1;
    errno = ENOMEM;
    for(int i = 3; i < MAX_FILES; i++) {
        if(file[i] == NULL) {
            rv = i;
            errno = 0;
            break;
        }
    }
    return rv;
}

int open(const char *pathname, int flags) {
    int fd, rv;
    struct node *node = node_find(pathname, NODE_MODE_NEAREST_PATH);
    
    if (node == NULL || node->f_op.open == NULL) {
        errno = ENOENT;
        return -1;
    }

    fd = fd_new();

    if (fd < 0) {
        return fd;
    }

    file[fd] = &__file[fd];
    file[fd]->node = node;
    file[fd]->f_flags = flags;

    rv = file[fd]->node->f_op.open(file[fd], pathname);
    if (rv) {
        errno = EIO;
    } else {
        errno = 0;
        rv = fd;
    }
    return rv;
}

ssize_t write(int fd, const void *buf, size_t count) {
    int rv;
    if(file[fd] == NULL || file[fd]->node->f_op.write == NULL) {
        errno = ENOENT;
        return -1;
    }
    
    rv = file[fd]->node->f_op.write(file[fd], buf, count);
    return rv;
}

ssize_t read(int fd, void *buf, size_t count) {
    int rv;
    if(file[fd] == NULL || file[fd]->node->f_op.read == NULL) {
        errno = ENOENT;
        return -1;
    }
    
    rv = file[fd]->node->f_op.read(file[fd], buf, count);
    return rv;
}

int close(int fd) {
    int rv;
    if(file[fd] == NULL || file[fd]->node->f_op.close == NULL) {
        errno = ENOENT;
        return -1;
    }
    
    rv = file[fd]->node->f_op.close(file[fd]);
    return rv;
}
