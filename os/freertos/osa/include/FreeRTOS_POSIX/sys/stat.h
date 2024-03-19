#ifndef _FREERTOS_POSIX_STAT_H_
#define _FREERTOS_POSIX_STAT_H_

#ifdef __cplusplus
extern "C" {
#endif

struct stat {
	off_t st_size;
    time_t st_mtime;
    mode_t st_mode;
};

int stat(const char *path, struct stat *stat);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FREERTOS_POSIX_STAT_H_ */
