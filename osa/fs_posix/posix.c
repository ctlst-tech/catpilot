#include <string.h>
#include <stdio.h>
#include <node.h>
#include <posix.h>

file_t __file[MAX_FILES];
file_t *file[MAX_FILES];

int newfd(void) {
    int rv = -1;
    for(int i = 0; i < MAX_FILES; i++) {
        if(file[i] == NULL) {
            rv = i;
            break;
        }
    }
    return rv;
}

int freefd(int fd) {
    file[fd] = NULL;
    return fd;
}

int open(const char *pathname, int flags) {
    int rv = -1;
    int fd = newfd();
    int nd = nodefind(pathname);
    if(nd >= 0 && fd >= 0) {
        file[fd] = &__file[fd];
        file[fd]->nd = nd;
        file[fd]->mode = flags;
        file[fd]->file = nodefilealloc(nd);
        rv = nodeopen(file[fd]->nd, file[fd]->file, pathname, flags);
        if(rv < 0) {
            freefd(fd);
            fd = -1;
        }
    }
    return fd;
}

ssize_t write(int fd, const void *buf, size_t count) {
    ssize_t rv = 0;
    if(fd < 0) return -1;
    if(file[fd] == NULL) return -1;

    if(file[fd]->mode && (O_WRONLY || O_RDWR)) {
        rv = nodewrite(file[fd]->nd, file[fd]->file, buf, count);
    } else {
        rv = -1;
    }

    return rv;
}

ssize_t read(int fd, void *buf, size_t count) {
    ssize_t rv = 0;
    if(fd < 0) return -1;
    if(file[fd] == NULL) return -1;

    if((!file[fd]->mode) || (file[fd]->mode && O_RDWR)) {
        rv = noderead(file[fd]->nd, file[fd]->file, buf, count);
    } else {
        rv = -1;
    }

    return rv;
}

int close(int fd) {
    int rv = 0;
    if(fd < 0) return -1;
    if(file[fd] == NULL) return -1;

    rv = nodeclose(file[fd]->nd, file[fd]->file);
    
    if(rv < 0) return -1; 
    
    rv = freefd(fd);

    return rv;
}

int ioctl(int fd, int cmd) {
    int rv = 0;
    if(fd < 0) return -1;
    if(file[fd] == NULL) return -1;

    rv = nodeioctl(file[fd]->nd, cmd);
    
    return rv;
}
