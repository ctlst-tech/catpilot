#include <stdio.h>
#include <errno.h>
#include <sys/termios.h>

extern int usart_set_speed(void *dev, uint32_t speed);
extern uint32_t usart_get_speed(void *dev);

int tcgetattr(int __fd, struct termios *__termios_p) {
    if (__fd < 2) {
        errno = EBADF;
        return -1;
    }

    void *dev = files[__fd]->node->f_op.dev;

    if (dev == NULL) {
        errno = ENOENT;
        return -1;
    }

    speed_t speed = usart_get_speed(dev);

    __termios_p->c_ispeed = speed;
    __termios_p->c_ospeed = speed;

    return 0;
}

int tcsetattr(int __fd, int __optional_actions,
              const struct termios *__termios_p) {
    int rv;
    (void)__optional_actions;

    if (__fd < 2) {
        errno = EBADF;
        return -1;
    }

    void *dev = files[__fd]->node->f_op.dev;

    if (dev == NULL) {
        errno = ENOENT;
        return -1;
    }

    speed_t speed = __termios_p->c_ispeed;
    rv = usart_set_speed(dev, speed);
    if (rv < 0) {
        return -1;
    }

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

int cfmakeraw(struct termios *__termios_p) {
    __termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    __termios_p->c_oflag &= ~OPOST;
//    __termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
//    __termios_p->c_cflag &= ~(CSIZE|PARENB);
//    __termios_p->c_cflag |= CS8;
    __termios_p->c_cc[VMIN] = 1;		/* read returns when one char is available.  */
    __termios_p->c_cc[VTIME] = 0;
    return 0;
}
