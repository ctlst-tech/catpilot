#pragma once

#include <string.h>
#include <stdio.h>

#define MAX_NODES 10

typedef struct {
    int (*open)(void *devcfg, void *filecfg, const char* pathname, int flags);
    ssize_t (*write)(void *devcfg, void *filecfg, const void *buf, size_t count);
    ssize_t (*read)(void *devcfg, void *filecfg, const void *buf, size_t count);
    int (*close)(void *devcfg, void *filecfg);
    int (*ioctl)(void *devcfg, int cmd);
    void *(devcfg);
} node_dev_t;

typedef struct {
    char nodepath[MAX_PATH_LENGTH];
    node_dev_t dev;
} node_t;

int nodereg(char *nodepath);
int nodedel(int nd);
int nodefind(const char *nodepath);

int noderegopen(int nd, 
                int (*open)
                (void *devcfg, void *filecfg, const char* pathname, int flags));
int noderegwrite(int nd, 
                ssize_t (*write)
                (void *devcfg, void *filecfg, const void *buf, size_t count));
int noderegread(int nd, 
                ssize_t (*read)
                (void *devcfg, void *filecfg, const void *buf, size_t count));
int noderegclose(int nd, 
                int (*close)
                (void *devcfg, void *filecfg));
int noderegioctl(int nd, 
                int (*ioctl)
                (void *devcfg, int cmd));
int noderegcfg(int nd, 
                void (*devcfg));

int nodeopen(int nd, void *filecfg, const char *pathname, int flags);
ssize_t nodewrite(int nd, void *filecfg, const void *buf, size_t count);
ssize_t noderead(int nd, void *filecfg,const void *buf, size_t count);
int nodeclose(int nd, void *filecfg);
int nodeioctl(int nd, int cmd);

void *nodegetdevcfg(int nd);
