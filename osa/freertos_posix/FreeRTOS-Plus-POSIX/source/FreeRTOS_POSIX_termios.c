#include <sys/termios.h>
#include <stdio.h>
#include <node.h>
#include <posix.h>

// Portable specific
#include "usart.h"

extern file_t *file[MAX_FILES];

int tcgetattr(int __fd, struct termios *__termios_p) {
    if(__fd < 0) {
        errno = EBADF;
        return -1;
    }

    memset(__termios_p, 0, sizeof(*__termios_p));

    usart_t *cfg = nodegetdevcfg(file[__fd]->nd);
    speed_t speed = USART_GetSpeed(cfg);
    __termios_p->c_ispeed = speed;
    __termios_p->c_ospeed = speed;
    return 0;
}

int tcsetattr(int __fd, int __optional_actions, const struct termios *__termios_p) {
    (void) __optional_actions;
    int rv;
    if(__fd < 0) {
        errno = EBADF;
        return -1;
    }
    usart_t *cfg = nodegetdevcfg(file[__fd]->nd);
    speed_t speed = __termios_p->c_ispeed;
    rv = USART_SetSpeed(cfg, speed);
    if(rv < 0) return -1;
    return 0;
}

speed_t cfgetospeed(const struct termios *__termios_p) {
    return __termios_p->c_ospeed;
}

speed_t cfgetispeed(const struct termios *__termios_p) {
    return __termios_p->c_ispeed;
}

int cfsetospeed(struct termios *__termios_p, speed_t __speed) {
    __termios_p->c_ospeed = __speed;
    return 0;
}

int cfsetispeed(struct termios *__termios_p, speed_t __speed) {
    __termios_p->c_ispeed = __speed;
    return 0;
}

int tcflush(int __fd, int __queue_selector) {
    return 0;
}
