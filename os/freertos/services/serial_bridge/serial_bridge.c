#include "serial_bridge.h"

static int serial_bridge_priority;
static int serial_bridge_buf_size;

extern const char *board_get_tty_name(char *path);
extern void board_print_tty_name(void);

typedef struct {
    char src_path[MAX_NAME_LEN];
    char dst_path[MAX_NAME_LEN];
    char src_fd;
    char dst_fd;
    int buf_size;
    char *buf;
    int rlen;
    int wlen;
} serial_bridge_t;

int serial_bridge_start(int priority, int buf_size) {
    if (cli_cmd_reg("serial_bridge", serial_bridge_commander) == NULL) {
        return -1;
    }
    serial_bridge_priority = priority;
    serial_bridge_buf_size = buf_size;
    return 0;
}

void *serial_bridge_thread(void *arg) {
    int rv;
    serial_bridge_t *bridge = (serial_bridge_t *)arg;
    bridge->wlen = 0;
    bridge->rlen = 0;

    char name[MAX_NAME_LEN];
    snprintf(name, MAX_NAME_LEN - 1, "%s_bridge_thread",
             strstr(bridge->src_path, "ttyS"));
    pthread_setname_np(name);

    while (1) {
        bridge->rlen = read(bridge->src_fd, bridge->buf, bridge->buf_size);
        bridge->wlen = write(bridge->dst_fd, bridge->buf, bridge->rlen);
        if (bridge->rlen != bridge->wlen) {
            printf(
                "Serial bridge thread: transfer failed\n"
                "From = %s\n"
                "To = %s\n"
                "Received = %d\n"
                "Transmitted = %d\n",
                bridge->src_path, bridge->dst_path, bridge->rlen, bridge->wlen);
        }
    }

    return NULL;
}

static int serial_bridge_open(const char *path, int baudrate) {
    struct termios termios_p;
    int rv;

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("Failed to connect to \"%s\": %s\n", path, strerror(errno));
        return -1;
    }

    if (baudrate != 0) {
        rv = tcgetattr(fd, &termios_p);
        if (rv) {
            printf("tcgetattr failed: %s, %s\n", path, strerror(errno));
            return -1;
        }
        cfsetispeed(&termios_p, baudrate);
        cfsetospeed(&termios_p, baudrate);
        rv = tcsetattr(fd, TCSANOW, &termios_p);
        if (rv) {
            printf("tcsetattr failed: %s, %s\n", path, strerror(errno));
            return -1;
        }
    }

    return fd;
}

static serial_bridge_t *serial_bridge_init(const char *src_path, int src_fd,
                                           const char *dst_path, int dst_fd,
                                           int buf_size) {
    serial_bridge_t *bridge = calloc(1, sizeof(serial_bridge_t));
    if (bridge == NULL) {
        printf("Memory fault: %s, %s\n", src_path, strerror(errno));
        return NULL;
    }

    strncpy(bridge->src_path, src_path, MAX_NAME_LEN - 1);
    strncpy(bridge->dst_path, dst_path, MAX_NAME_LEN - 1);
    bridge->src_fd = src_fd;
    bridge->dst_fd = dst_fd;
    bridge->buf_size = buf_size;
    bridge->buf = calloc(bridge->buf_size, sizeof(char));
    if (bridge->buf == NULL) {
        printf("Buffer init failed: %s, %s\n", src_path, strerror(errno));
        return NULL;
    }

    return bridge;
}

void serial_bridge_help(void) {
    printf(
        "\nUsage: serial_bridge [tty1/port1] [tty2/port2] [baudrate1] "
        "[baudrate2]"
        "\n\n"
        "Examples:\n"
        "   serial_bridge ttyS2 ttyS1 57600 115200\n"
        "   serial_bridge GPS1 TELEM1 115200 57600\n"
        "   serial_bridge GPS1 TELEM1\n"
        "   serial_bridge GPS1 TELEM1 115200\n"
        "   serial_bridge ttyS1 9600\n"
        "   serial_bridge TELEM1\n\n");
    printf("Available tty and ports:\n");
    board_print_tty_name();
}

int serial_bridge_start_service(char *path1, char *path2, int baudrate1,
                                int baudrate2) {
    int fd1 = serial_bridge_open(path1, baudrate1);
    int fd2 = serial_bridge_open(path2, baudrate2);

    if (fd1 < 0 || fd2 < 0) {
        printf("serial_bridge_open failed\n");
        return -1;
    }

    serial_bridge_t *bridge1 =
        serial_bridge_init(path1, fd1, path2, fd2, serial_bridge_buf_size);
    serial_bridge_t *bridge2 =
        serial_bridge_init(path2, fd2, path1, fd1, serial_bridge_buf_size);
    if (bridge1 == NULL || bridge2 == NULL) {
        printf("serial_bridge_init failed\n");
        return -1;
    }

    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

    // TODO: Use in debug mode
    // struct termios termios_p1;
    // struct termios termios_p2;
    // tcgetattr(fd1, &termios_p1);
    // tcgetattr(fd2, &termios_p2);
    // printf("serial_bridge:\n");
    // printf("%s %d\n", path1, cfgetispeed(&termios_p1));
    // printf("%s %d\n", path2, cfgetispeed(&termios_p2));
    // usleep(3000);

    pthread_attr_init(&attr);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = serial_bridge_priority;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setstacksize(&attr, 512);
    pthread_create(&tid, &attr, serial_bridge_thread, (void *)bridge1);

    pthread_attr_init(&attr);
    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = serial_bridge_priority;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setstacksize(&attr, 512);
    pthread_create(&tid, &attr, serial_bridge_thread, (void *)bridge2);

    return 0;
}

int serial_bridge_commander(int argc, char **argv) {
    char path1[MAX_NAME_LEN] = {0};
    char path2[MAX_NAME_LEN] = {0};
    char tty[2][MAX_NAME_LEN] = {0};
    char baudrate_string[2][MAX_NAME_LEN] = {0};
    int baudrate[2] = {0};
    int tty_index = 0;
    int baudrate_index = 0;

    if ((argc == 2 && !strncmp(argv[1], "-h", MAX_NAME_LEN)) || argc > 5 ||
        argc == 1) {
        serial_bridge_help();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strnlen(argv[i], MAX_NAME_LEN - 1) > MAX_NAME_LEN - 1) {
            printf("Too long argument\n");
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (sscanf(argv[i], "%[A-Za-z0-9]", tty[tty_index]) > 0) {
            tty_index++;
            if (tty_index > 1) {
                break;
            }
        } else {
            printf("Incorrect name\n");
            return 0;
        }
    }

    if (tty_index == 0) {
        printf("Path not found\n");
        return 0;
    }

    for (int i = tty_index + 1; i < argc; i++) {
        if (sscanf(argv[i], "%[0-9]", baudrate_string[baudrate_index]) > 0) {
            baudrate_index++;
            if (baudrate_index > 1) {
                break;
            }
        } else {
            printf(
                "Wrong baudrate:\n"
                "Baudrate must be in range: 9600...1000000\n"
                "Or zero value to not change\n");
            return 0;
        }
    }

    if (tty_index == 1) {
        strncpy(tty[1], CLI_PORT, MAX_NAME_LEN);
    }

    for (int i = 0; i < baudrate_index; i++) {
        baudrate[i] = atoi(baudrate_string[i]);
        if ((baudrate[i] > 1000000 || baudrate[i] < 9600) && baudrate[i] != 0) {
            printf(
                "Wrong baudrate:\n"
                "Baudrate must be in range: 9600...1000000\n"
                "Or zero value to not change\n");
            return 0;
        }
    }

    const char *tty_name1 = board_get_tty_name(tty[0]);
    const char *tty_name2 = board_get_tty_name(tty[1]);

    if (tty_name1 == NULL || tty_name2 == NULL) {
        printf("Path not found\n\n");
        printf("tty name must be ttyS[num] or [port][num]\n");
        printf("Available tty and ports:\n");
        board_print_tty_name();
        return 0;
    }

    if (strstr(tty_name1, "ttyS") == NULL ||
        strstr(tty_name2, "ttyS") == NULL) {
        printf("tty name must be ttyS[num] or [port][num]\n");
        printf("Available tty and ports:\n");
        board_print_tty_name();
        return 0;
    }

    snprintf(path1, MAX_NAME_LEN, "/dev/%s", tty_name1);
    snprintf(path2, MAX_NAME_LEN, "/dev/%s", tty_name2);

    if (!strcmp(path1, stdout->node->name) ||
        !strcmp(path2, stdout->node->name)) {
        std_stream_deinit("stdout");
        std_stream_deinit("stdin");
        std_stream_deinit("stderr");
    }

    serial_bridge_start_service(path1, path2, baudrate[0], baudrate[1]);

    return 0;
}
