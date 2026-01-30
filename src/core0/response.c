/**
 * 响应格式化
 */

#include "../include/uart_to_x.h"

static char resp_buffer[RESP_BUFFER_SIZE];

void response_ok(const char *data) {
    if (data && data[0]) {
        printf("OK %s\r\n", data);
    } else {
        printf("OK\r\n");
    }
}

void response_error(error_code_t code, const char *msg) {
    const char *err_names[] = {
        "Success",
        "Unknown command",
        "Invalid parameter",
        "No response",
        "Bus busy",
        "Timeout",
        "CRC error",
        "Buffer overflow",
        "Not initialized",
        "Pin conflict"
    };

    if (code < sizeof(err_names) / sizeof(err_names[0])) {
        printf("ERR E%02d %s", code, err_names[code]);
    } else {
        printf("ERR E%02d Unknown error", code);
    }

    if (msg && msg[0]) {
        printf(": %s", msg);
    }
    printf("\r\n");
}

void response_data(const uint8_t *data, size_t len) {
    printf("OK ");
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\r\n");
}
