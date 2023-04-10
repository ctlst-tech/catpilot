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
    if (fd_file < 0) {
        printf("%s\n", strerror(errno));
        return;
    }

    int fd_port = open(port, O_RDWR);
    if (fd_port < 0) {
        printf("%s\n", strerror(errno));
        close(fd_file);
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
        ioctl(fd_port, 0, portMAX_DELAY);
        do {
            rb = read(fd_port, buf, sizeof(buf));
            if (rb > 0) {
                write(fd_file, buf, rb);
                fsync(fd_file);
                lseek(fd_file, 0, SEEK_END);
            } else {
                if (errno != ETIMEDOUT) {
                    printf("%s\n", strerror(errno));
                }
            }
            ioctl(fd_port, 0, FILE_UPLOAD_TIMEOUT_MS);
        } while (rb > 0);
        ioctl(fd_port, 0, portMAX_DELAY);
    }
    close(fd_file);
    close(fd_port);
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
