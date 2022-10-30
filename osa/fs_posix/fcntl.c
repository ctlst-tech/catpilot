#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <fs.h>

struct file __file[MAX_FILES];
struct file *file[MAX_FILES];

int open(const char *pathname, int flags) {

}

ssize_t write(int fd, const void *buf, size_t count) {

}

ssize_t read(int fd, void *buf, size_t count) {

}

int close(int fd) {
    
}
