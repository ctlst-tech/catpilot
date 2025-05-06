#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // For write()

#include "macros.h" // Assuming MIN, MAX, LOG_MAX_MODULE_NAME, LOG_EMPTY_TYPE are here

// Configuration for the log buffer
#define MAX_LOG_LINE_LEN 80     // Max characters per log line
#define NUM_LOG_BUFFER_LINES 102 // Number of lines in the buffer (e.g., 8192 / 80)
#define TEMP_FORMAT_BUFFER_SIZE 512 // Temporary buffer for formatting a full log entry before splitting

static char *msg_types[4] = {"INFO", "WARN", "ERROR", "DEBUG"};

static char *msg_color[5] = {
    "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1B[37m", "\x1b[0m",
};
static SemaphoreHandle_t log_mutex = NULL;

// Circular buffer for log lines
static char log_lines_buffer[NUM_LOG_BUFFER_LINES][MAX_LOG_LINE_LEN + 1]; // +1 for null terminator
static int log_buffer_head = 0;
static int log_buffer_tail = 0;
static int log_buffer_line_count = 0;

// Assumed to be defined elsewhere, e.g. in log.h or macros.h
// #define LOG_MAX_MODULE_NAME 20
// #define LOG_EMPTY_TYPE 0xFF (example value)
// #define MIN(a,b) (((a)<(b))?(a):(b))
// #define MAX(a,b) (((a)>(b))?(a):(b))


void log_submit(uint8_t msg_type, const char *module, const char *s, ...) {
    if (log_mutex == NULL) {
        log_mutex = xSemaphoreCreateMutex();
        if (log_mutex == NULL) {
            // Failed to create mutex, cannot log
            return;
        }
    }

    if (msg_type == LOG_EMPTY_TYPE) return;

    char temp_formatted_entry[TEMP_FORMAT_BUFFER_SIZE];
    ssize_t current_length = 0;
    char module_alig[LOG_MAX_MODULE_NAME + 1] = {0}; // +1 for null terminator
    struct timespec t;

    // Prepare aligned module name
    memset(module_alig, ' ', LOG_MAX_MODULE_NAME);
    memcpy(module_alig, module, MIN(strlen(module), LOG_MAX_MODULE_NAME));
    module_alig[LOG_MAX_MODULE_NAME] = '\0'; // Ensure null termination if module name is long

    clock_gettime(CLOCK_MONOTONIC, &t);

    // Format the log prefix
    current_length += snprintf(temp_formatted_entry + current_length,
                               MAX(0, TEMP_FORMAT_BUFFER_SIZE - current_length),
                               "%.3f\t", t.tv_sec + t.tv_nsec * 1e-9);
    current_length += snprintf(temp_formatted_entry + current_length,
                               MAX(0, TEMP_FORMAT_BUFFER_SIZE - current_length),
                               "%s%s%s\t", msg_color[msg_type],
                               msg_types[msg_type], msg_color[4]);
    current_length += snprintf(temp_formatted_entry + current_length,
                               MAX(0, TEMP_FORMAT_BUFFER_SIZE - current_length),
                               "%s\t", module_alig);

    // Format the user message
    va_list arg;
    va_start(arg, s);
    current_length += vsnprintf(temp_formatted_entry + current_length,
                                MAX(0, TEMP_FORMAT_BUFFER_SIZE - current_length),
                                s, arg);
    va_end(arg);

    // Ensure the temporary buffer is null-terminated within its bounds
    if (current_length >= TEMP_FORMAT_BUFFER_SIZE) {
        temp_formatted_entry[TEMP_FORMAT_BUFFER_SIZE - 1] = '\0';
    } else {
        temp_formatted_entry[current_length] = '\0';
    }


    xSemaphoreTake(log_mutex, portMAX_DELAY);

    const char *p_entry = temp_formatted_entry;
    int remaining_len = strlen(p_entry);

    while (remaining_len > 0) {
        int len_to_copy = MIN(remaining_len, MAX_LOG_LINE_LEN);

        strncpy(log_lines_buffer[log_buffer_tail], p_entry, len_to_copy);
        log_lines_buffer[log_buffer_tail][len_to_copy] = '\0'; // Ensure null termination

        log_buffer_tail = (log_buffer_tail + 1) % NUM_LOG_BUFFER_LINES;

        if (log_buffer_line_count < NUM_LOG_BUFFER_LINES) {
            log_buffer_line_count++;
        } else {
            // Buffer is full, overwrite oldest (advance head)
            log_buffer_head = (log_buffer_head + 1) % NUM_LOG_BUFFER_LINES;
        }

        p_entry += len_to_copy;
        remaining_len -= len_to_copy;
    }

    xSemaphoreGive(log_mutex);
}

int log_print(int argc, char **argv) {
    if (log_mutex == NULL) {
        // Mutex not created, nothing to print or protect
        return 0;
    }
    xSemaphoreTake(log_mutex, portMAX_DELAY);

    int current_print_idx = log_buffer_head;
    for (int i = 0; i < log_buffer_line_count; i++) {
        if (log_lines_buffer[current_print_idx][0] != '\0') { // Check if line is not empty
            write(1, log_lines_buffer[current_print_idx], strlen(log_lines_buffer[current_print_idx]));
            write(1, "\r\n", 2); // Add CR LF after each line
        }
        current_print_idx = (current_print_idx + 1) % NUM_LOG_BUFFER_LINES;
    }

    xSemaphoreGive(log_mutex);
    return 0;
}