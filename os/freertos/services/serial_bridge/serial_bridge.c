#include "serial_bridge.h"

static int serial_bridge_priority;
static int serial_bridge_buf_size;

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

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("Failed to connect to \"%s\": %s\n", path, strerror(errno));
        return -1;
    }

    int rv = tcgetattr(fd, &termios_p);
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

int serial_bridge_commander(int argc, char **argv) {
    int optind = 1;

    if (argc < 5) {
        printf(
            "Usage: serial_bridge [path1] [path2] [baudrate1] [baudrate2]\n"
            "Example:\n"
            "serial_bridge /dev/ttyS1 /dev/ttyS2 57600 115200\n");
        return 0;
    }

    char path1[MAX_NAME_LEN];
    char path2[MAX_NAME_LEN];
    strncpy(path1, argv[1], MAX_NAME_LEN - 1);
    strncpy(path2, argv[2], MAX_NAME_LEN - 1);
    if (strstr(path1, "/dev/ttyS") == NULL ||
        strstr(path2, "/dev/ttyS") == NULL) {
        printf(
            "Wrong path:\n"
            "Path must begin with: /dev/ttyS\n");
        return 0;
    }
    if (!strcmp(path1, stdout->node->name) ||
        strcmp(path2, stdout->node->name)) {
        std_stream_deinit("stdout");
        std_stream_deinit("stdin");
        std_stream_deinit("stderr");
    }

    int baudrate1 = atoi(argv[3]);
    int baudrate2 = atoi(argv[4]);
    if (baudrate1 > 1000000 || baudrate2 > 1000000 || baudrate1 < 9600 ||
        baudrate1 < 9600) {
        printf(
            "Wrong baudrate:\n"
            "Baudrate must be in range: 9600...1000000\n");
        return 0;
    }

    int fd1 = serial_bridge_open(path1, baudrate1);
    int fd2 = serial_bridge_open(path2, baudrate2);

    if (fd1 < 0 || fd2 < 0) {
        printf("serial_bridge_open failed\n");
        return 0;
    }

    serial_bridge_t *bridge1 =
        serial_bridge_init(path1, fd1, path2, fd2, serial_bridge_buf_size);
    serial_bridge_t *bridge2 =
        serial_bridge_init(path2, fd2, path1, fd1, serial_bridge_buf_size);
    if (bridge1 == NULL || bridge2 == NULL) {
        printf("serial_bridge_init failed\n");
    }

    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

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
