#ifndef _FREERTOS_POSIX_IOCTL_H_
#define _FREERTOS_POSIX_IOCTL_H_

#ifdef __cplusplus
extern "C" {
#endif

int ioctl(int fd, int request, ...);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FREERTOS_POSIX_IOCTL_H_ */
