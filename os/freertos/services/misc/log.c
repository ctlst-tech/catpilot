#include "log.h"

#include "macros.h"

static char *msg_types[4] = {"INFO", "WARN", "ERROR", "DEBUG"};

static char *msg_color[5] = {
    "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1B[37m", "\x1b[0m",
};

typedef struct {
    char *buf;
    size_t buf_size;
    unsigned int offset;
    int fd;
    SemaphoreHandle_t mutex;
    enum log_type type;
} log_module_t;

static log_module_t log = {0};

int log_init(const char *prefix, const char *path, enum log_type type,
             size_t buf_size) {
    if (path == NULL || log.fd > 0) {
        return -1;
    }

    if (strlen(prefix) + strlen(path) > MAX_NAME_LEN) {
        return -1;
    }

    if (mkdir(path, 0) && errno != EEXIST) {
        return -1;
    }

    log.mutex = xSemaphoreCreateMutex();
    if (log.mutex == NULL) {
        return -1;
    }

    log.buf = calloc(buf_size, sizeof(char));
    if (log.buf == NULL) {
        return -1;
    }
    log.buf_size = buf_size;

    char file_name[MAX_NAME_LEN];
    int file_num = 0;
    int fd = 0;
    log.type = type;

    if (log.type == LOG_TO_FILE) {
        do {
            snprintf(file_name, MAX_NAME_LEN, "%s/%s_%d.txt", path, prefix,
                     file_num);
            fd = open(file_name, O_RDWR | O_CREAT | O_EXCL);
            file_num++;
        } while (fd < 0 && errno == EEXIST);

        if (fd > 0) {
            log.fd = fd;
        }
    }

    return 0;
}

void log_module(uint8_t msg_type, const char *module, const char *s, ...) {
    if ((log.fd <= 0 && log.type == LOG_TO_FILE) || (log.buf == NULL)) {
        return;
    }

    ssize_t length = 0;
    char string[LOG_MAX_LENGTH] = {};
    char module_alig[20] = {};
    struct timespec t;

    if (msg_type == LOG_EMPTY_TYPE) {
        return;
    }

    memset(module_alig, ' ', LOG_MAX_MODULE_NAME - 1);
    memcpy(module_alig, module, MIN(strlen(module), LOG_MAX_MODULE_NAME));

    clock_gettime(CLOCK_MONOTONIC, &t);

    xSemaphoreTake(log.mutex, portMAX_DELAY);
    length += sprintf(string + length, "%.3f\t", t.tv_sec + t.tv_nsec * 1e-9);
    length += sprintf(string + length, "%s%s%s\t", msg_color[msg_type],
                      msg_types[msg_type], msg_color[4]);
    length += sprintf(string + length, "%s\t", module_alig);

    va_list arg;
    va_start(arg, s);
    length +=
        vsnprintf(string + length, MAX(LOG_MAX_LENGTH - length, 0), s, arg);
    va_end(arg);

    length += sprintf(string + length, "\r\n");

    if (log.type == LOG_TO_FILE) {
        write(log.fd, string, length);
        fsync(log.fd);
    } else {
        if (strlen(string) > log.buf_size - log.offset) {
            printf("Log buffer overflow!\n");
        } else {
            log.offset += sprintf(log.buf + log.offset, "%s", string);
        }
    }

    xSemaphoreGive(log.mutex);
}

int log_print(int argc, char **argv) {
    xSemaphoreTake(log.mutex, portMAX_DELAY);
    if (log.type == LOG_TO_FILE) {
        lseek(log.fd, 0, SEEK_SET);
        ssize_t rb = 0;
        do {
            rb = read(log.fd, log.buf, log.buf_size);
            if (rb > 0) {
                write(1, log.buf, rb);
            }
        } while ((size_t)rb == log.buf_size);
        lseek(log.fd, 0, SEEK_END);
    } else {
        write(1, log.buf, log.offset);
    }
    xSemaphoreGive(log.mutex);
    return 0;
}
