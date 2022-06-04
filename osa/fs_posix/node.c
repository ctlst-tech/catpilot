#include <string.h>
#include <stdio.h>
#include <node.h>

node_t __node[MAX_NODES];
node_t *node[MAX_NODES];

int nodereg(char *nodepath) {
    int nd = -1;
    if(strlen(nodepath) < 1) return -1;

    for(int i = 0; i < MAX_NODES; i++) {
        if(node[i] == NULL) {
            node[i] = &__node[i];
            strcpy(node[i]->nodepath, nodepath);
            nd = i;
            break;
        }
    }

    return nd;
}

int nodedel(int nd) {
    if(nd < 0) return -1;
    if(node[nd] == NULL) return -1;

    memset(node[nd], 0, sizeof(__node[nd]));
    node[nd] = NULL;

    return nd;
}

int nodefind(const char *nodepath) {
    int nd = -1;
    if(nodepath == NULL) return -1;

    size_t maxnodelen = 0;
    char head[MAX_PATH_LENGTH + 1] = {};
    for(int i = 0; i < MAX_NODES; i++) {
        if(node[i] == NULL) continue;
        size_t nodelen = strlen(node[i]->nodepath);
        memcpy(head, nodepath, nodelen);
        head[nodelen] = '\0';
        if((!strcmp(head, node[i]->nodepath)) && nodelen > maxnodelen) {
            nd = i;
            maxnodelen = nodelen;
        }
    }

    return nd;
}

int noderegopen(int nd, 
                int (*open)
                (void *devcfg, void *filecfg, const char* nodepath, int flags)) {
    if(nd < 0) return -1;
    if(node[nd] == NULL) return -1;
    if(open == NULL) return -1;
    
    node[nd]->dev.open = open;
    
    return nd;
}

int noderegwrite(int nd, 
                ssize_t (*write)
                (void *devcfg, void *filecfg, const void *buf, size_t count)) {
    if(nd < 0) return -1;
    if(node[nd] == NULL) return -1;
    if(write == NULL) return -1;

    node[nd]->dev.write = write;
    
    return nd;
}

int noderegread(int nd, 
                ssize_t (*read)
                (void *devcfg, void *filecfg, const void *buf, size_t count)) {
    if(nd < 0) return -1;
    if(node[nd] == NULL) return -1;
    if(read == NULL) return -1;

    node[nd]->dev.read = read;
    
    return nd;
}

int noderegclose(int nd, 
                int (*close)
                (void *devcfg, void *filecfg)) {
    if(nd < 0) return -1;
    if(node[nd] == NULL) return -1;
    if(close == NULL) return -1;

    node[nd]->dev.close = close;
    
    return nd;
}

int noderegioctl(int nd, 
                int (*ioctl)
                (void *devcfg, int cmd)) {
    if(nd < 0) return -1;
    if(node[nd] == NULL) return -1;
    if(ioctl == NULL) return -1;

    node[nd]->dev.ioctl = ioctl;
    
    return nd;
}

int noderegdevcfg(int nd, void (*devcfg)) {
    if(nd < 0) return -1;
    if(node[nd] == NULL) return -1;
    if(devcfg == NULL) return -1;

    node[nd]->dev.devcfg = devcfg;
    
    return nd;
}

int nodeopen(int nd, void *filecfg, const char *pathname, int flags) {
    int rv;
    if(node[nd] == NULL) return -1;
    if(node[nd]->dev.open == NULL) return -1;

    node_dev_t *dev = &node[nd]->dev;
    rv = node[nd]->dev.open(dev->devcfg, filecfg, pathname, flags);

    return rv;
}

ssize_t nodewrite(int nd, void *filecfg, const void *buf, size_t count) {
    int rv;
    if(node[nd] == NULL) return -1;
    if(node[nd]->dev.write == NULL) return -1;

    node_dev_t *dev = &node[nd]->dev;
    rv = node[nd]->dev.write(dev->devcfg, filecfg, buf, count);

    return rv;
}

ssize_t noderead(int nd, void *filecfg, const void *buf, size_t count) {
    int rv;
    if(node[nd] == NULL) return -1;
    if(node[nd]->dev.read == NULL) return -1;

    node_dev_t *dev = &node[nd]->dev;
    rv = node[nd]->dev.read(dev->devcfg, filecfg, buf, count);

    return rv;
}

int nodeclose(int nd, void *filecfg) {
    int rv;
    if(node[nd] == NULL) return -1;
    if(node[nd]->dev.close == NULL) return -1;

    node_dev_t *dev = &node[nd]->dev;
    rv = node[nd]->dev.close(dev->devcfg, filecfg);

    return rv;
}

int nodeioctl(int nd, int cmd) {
    int rv;
    if(node[nd] == NULL) return -1;
    if(node[nd]->dev.ioctl == NULL) return -1;

    node_dev_t *dev = &node[nd]->dev;
    rv = node[nd]->dev.ioctl(dev->devcfg, cmd);

    return rv;  
}

void *nodegetdevcfg(int nd) {
    int rv;
    if(node[nd] == NULL) return NULL;
    if(node[nd]->dev.devcfg == NULL) return NULL;

    return node[nd]->dev.devcfg;  
}
