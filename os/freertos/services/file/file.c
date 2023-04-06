#include "file.h"

static char buf[512];

static void file_print_help(void) {
    printf("Usage: file [operation] [file] [port]\n");
    printf(
        "Example: \n"
        "file upload /fs/cfg/angrate_pos.xml /dev/ttyS1\n"
        "file download /fs/logs/logger_0.eqrb /dev/ttyS2\n");
}

static void file_operation(const char *operation, const char *path,
                           const char *port) {
    int flags = 0;
    int download = 0;

    if (!strncmp(operation, "upload", CLI_MAX_CMD_LENGTH)) {
        flags |= O_CREAT | O_TRUNC | O_WRONLY;
    } else if (!strncmp(operation, "download", CLI_MAX_CMD_LENGTH)) {
        download = 1;
        flags |= O_RDONLY;
    } else {
        file_print_help();
        return;
    }

    int fd_file = open(path, flags);
    int fd_port = open(port, O_RDWR);

    if (fd_file < 0 || fd_port < 0) {
        printf("%s\n", strerror(errno));
        return;
    }

    ssize_t rb = 0;
    if (download) {
        lseek(fd_file, 0, SEEK_SET);
        do {
            rb = read(fd_file, buf, sizeof(buf));
            if (rb > 0) {
                write(fd_port, buf, rb);
            } else {
                printf("%s\n", strerror(errno));
            }
        } while ((size_t)rb == sizeof(buf));
        lseek(fd_file, 0, SEEK_END);
    } else {
        uint32_t start = xTaskGetTickCount();
        uint32_t now = xTaskGetTickCount();
        do {
            now = xTaskGetTickCount();
            rb = read(fd_port, buf, sizeof(buf));
            if (rb > 0) {
                write(fd_file, buf, rb);
                fsync(fd_file);
                lseek(fd_file, 0, SEEK_END);
            } else {
                printf("%s\n", strerror(errno));
            }
        } while (now - start < 5000);
        close(fd_file);
    }
}

int file_commander(int argc, char **argv) {
    if (argc != 4) {
        file_print_help();
    } else {
        file_operation((const char *)argv[1], (const char *)argv[2],
                       (const char *)argv[3]);
    }
    return 0;
}
