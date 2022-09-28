#pragma once

#include <string.h>
#include <stdio.h>

#define MAX_NODES 10

typedef struct {
    int (*open)(void *devcfg, void *file, const char* pathname, int flags);
    ssize_t (*write)(void *devcfg, void *file, const void *buf, size_t count);
    ssize_t (*read)(void *devcfg, void *file, void *buf, size_t count);
    int (*close)(void *devcfg, void *file);
    int (*ioctl)(void *devcfg, int cmd);
    void *(*filealloc)(void);
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
                (void *devcfg, void *file, const char* pathname, int flags));
int noderegwrite(int nd, 
                ssize_t (*write)
                (void *devcfg, void *file, const void *buf, size_t count));
int noderegread(int nd, 
                ssize_t (*read)
                (void *devcfg, void *file, void *buf, size_t count));
int noderegclose(int nd, 
                int (*close)
                (void *devcfg, void *file));
int noderegioctl(int nd, 
                int (*ioctl)
                (void *devcfg, int cmd));
int noderegdevcfg(int nd, 
                void (*devcfg));
int noderegfilealloc(int nd, 
                void *(*filealloc)(void));

int nodeopen(int nd, void *file, const char *pathname, int flags);
ssize_t nodewrite(int nd, void *file, const void *buf, size_t count);
ssize_t noderead(int nd, void *file, void *buf, size_t count);
int nodeclose(int nd, void *file);
int nodeioctl(int nd, int cmd);
void *nodefilealloc(int nd);

void *nodegetdevcfg(int nd);
