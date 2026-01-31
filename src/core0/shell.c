/**
 * Shell 命令解析器
 */

#include "../include/uart_to_x.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include <stdlib.h>
#include <ctype.h>

// 命令缓冲区
static char cmd_buffer[CMD_BUFFER_SIZE];
static size_t cmd_pos = 0;

// 命令历史
#define HISTORY_SIZE 10
static char cmd_history[HISTORY_SIZE][CMD_BUFFER_SIZE];
static int history_count = 0;
static int history_index = -1;

// 命令处理函数类型
typedef void (*cmd_handler_t)(int argc, char *argv[]);

// 命令结构
typedef struct {
    const char *name;
    const char *help;
    cmd_handler_t handler;
} cmd_entry_t;

// 前向声明
static void cmd_help(int argc, char *argv[]);
static void cmd_info(int argc, char *argv[]);
static void cmd_reset(int argc, char *argv[]);
static void cmd_bootsel(int argc, char *argv[]);
static void cmd_mode(int argc, char *argv[]);
static void cmd_i2c(int argc, char *argv[]);
static void cmd_spi(int argc, char *argv[]);
static void cmd_uart(int argc, char *argv[]);
static void cmd_gpio(int argc, char *argv[]);

// 命令表
static const cmd_entry_t commands[] = {
    {"help",    "Show help information",        cmd_help},
    {"info",    "Show system information",      cmd_info},
    {"reset",   "Reset system",                 cmd_reset},
    {"bootsel", "Enter BOOTSEL mode for flash", cmd_bootsel},
    {"mode",    "Set transfer mode",            cmd_mode},
    {"i2c",     "I2C operations",               cmd_i2c},
    {"spi",     "SPI operations",               cmd_spi},
    {"uart",    "UART operations",              cmd_uart},
    {"gpio",    "GPIO operations",              cmd_gpio},
    {NULL, NULL, NULL}
};

/**
 * 初始化 Shell
 */
void shell_init(void) {
    memset(cmd_buffer, 0, sizeof(cmd_buffer));
    cmd_pos = 0;
}

/**
 * 解析十六进制或十进制数
 */
static uint32_t parse_number(const char *str) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return strtoul(str + 2, NULL, 16);
    }
    return strtoul(str, NULL, 10);
}

/**
 * 分割命令行参数
 */
static int parse_args(char *line, char *argv[], int max_args) {
    int argc = 0;
    char *p = line;
    bool in_quote = false;

    while (*p && argc < max_args) {
        // 跳过空白
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        // 处理引号
        if (*p == '"') {
            in_quote = true;
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') *p++ = '\0';
        } else {
            argv[argc++] = p;
            while (*p && !isspace(*p)) p++;
            if (*p) *p++ = '\0';
        }
    }
    return argc;
}

/**
 * 执行命令
 */
static void execute_command(char *line) {
    char *argv[32];
    int argc = parse_args(line, argv, 32);

    if (argc == 0) return;

    // 查找命令
    for (const cmd_entry_t *cmd = commands; cmd->name != NULL; cmd++) {
        if (strcmp(argv[0], cmd->name) == 0) {
            cmd->handler(argc, argv);
            return;
        }
    }

    // 检查是否是带点的子命令 (如 i2c.scan)
    char *dot = strchr(argv[0], '.');
    if (dot) {
        *dot = '\0';
        // 构建新的参数列表
        char *new_argv[33];
        new_argv[0] = argv[0];
        new_argv[1] = dot + 1;
        for (int i = 1; i < argc; i++) {
            new_argv[i + 1] = argv[i];
        }

        for (const cmd_entry_t *cmd = commands; cmd->name != NULL; cmd++) {
            if (strcmp(new_argv[0], cmd->name) == 0) {
                cmd->handler(argc + 1, new_argv);
                return;
            }
        }
    }

    printf("ERR E01 Unknown command: %s\r\n", argv[0]);
}

/**
 * 处理输入字符
 */
static void process_char(char c) {
    switch (c) {
        case '\r':
        case '\n':
            printf("\r\n");
            if (cmd_pos > 0) {
                cmd_buffer[cmd_pos] = '\0';
                // 保存到历史
                if (history_count < HISTORY_SIZE) {
                    strcpy(cmd_history[history_count++], cmd_buffer);
                } else {
                    memmove(cmd_history[0], cmd_history[1],
                            (HISTORY_SIZE - 1) * CMD_BUFFER_SIZE);
                    strcpy(cmd_history[HISTORY_SIZE - 1], cmd_buffer);
                }
                history_index = -1;

                // 执行命令
                execute_command(cmd_buffer);
                cmd_pos = 0;
            }
            printf("> ");
            break;

        case '\b':
        case 0x7F:  // DEL
            if (cmd_pos > 0) {
                cmd_pos--;
                printf("\b \b");
            }
            break;

        case 0x03:  // Ctrl+C
            printf("^C\r\n> ");
            cmd_pos = 0;
            break;

        default:
            if (cmd_pos < CMD_BUFFER_SIZE - 1 && c >= 0x20) {
                cmd_buffer[cmd_pos++] = c;
                putchar(c);
            }
            break;
    }
}

/**
 * Shell 任务 - 在主循环中调用
 */
void shell_task(void) {
    int c = getchar_timeout_us(0);
    if (c != PICO_ERROR_TIMEOUT) {
        process_char((char)c);
    }
}

// ============================================================================
// 命令处理函数
// ============================================================================

static void cmd_help(int argc, char *argv[]) {
    if (argc > 1) {
        // 显示特定命令的帮助
        for (const cmd_entry_t *cmd = commands; cmd->name != NULL; cmd++) {
            if (strcmp(argv[1], cmd->name) == 0) {
                printf("%s - %s\r\n", cmd->name, cmd->help);
                return;
            }
        }
        printf("ERR E01 Unknown command: %s\r\n", argv[1]);
        return;
    }

    printf("Available commands:\r\n");
    for (const cmd_entry_t *cmd = commands; cmd->name != NULL; cmd++) {
        printf("  %-10s %s\r\n", cmd->name, cmd->help);
    }
    printf("\r\nUse '<cmd>.subcmd' or '<cmd> subcmd' format.\r\n");
    printf("Example: i2c.scan or i2c scan\r\n");
}

static void cmd_info(int argc, char *argv[]) {
    (void)argc; (void)argv;
    printf("UartToX v%s\r\n", UART_TO_X_VERSION_STRING);
    printf("RP2040 @ %lu MHz\r\n", clock_get_hz(clk_sys) / 1000000);
    printf("Protocols: I2C SPI UART GPIO\r\n");
    printf("Mode: %s\r\n",
        g_state.mode == MODE_TEXT ? "TEXT" :
        g_state.mode == MODE_BIN ? "BIN" : "STREAM");
}

static void cmd_reset(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "factory") == 0) {
        printf("Factory reset...\r\n");
    }
    printf("Resetting...\r\n");
    sleep_ms(100);
    watchdog_reboot(0, 0, 0);
}

static void cmd_bootsel(int argc, char *argv[]) {
    (void)argc; (void)argv;
    printf("Entering BOOTSEL mode...\r\n");
    printf("Device will appear as USB drive.\r\n");
    sleep_ms(100);
    reset_usb_boot(0, 0);
}

static void cmd_mode(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Current mode: %s\r\n",
            g_state.mode == MODE_TEXT ? "text" :
            g_state.mode == MODE_BIN ? "bin" : "stream");
        return;
    }

    if (strcmp(argv[1], "text") == 0) {
        g_state.mode = MODE_TEXT;
        printf("OK Mode set to TEXT\r\n");
    } else if (strcmp(argv[1], "bin") == 0) {
        g_state.mode = MODE_BIN;
        printf("OK Entering binary mode\r\n");
    } else if (strcmp(argv[1], "stream") == 0) {
        g_state.mode = MODE_STREAM;
        printf("OK Mode set to STREAM\r\n");
    } else {
        printf("ERR E02 Invalid mode. Use: text, bin, stream\r\n");
    }
}

// ============================================================================
// I2C 命令
// ============================================================================

static void cmd_i2c(int argc, char *argv[]) {
    if (argc < 2) {
        printf("I2C subcommands: config, scan, detect, read, write, transfer, dump\r\n");
        return;
    }

    const char *subcmd = argv[1];

    if (strcmp(subcmd, "config") == 0) {
        if (argc < 3) {
            printf("Port: %d, Speed: %lu kHz, Pullup: %s\r\n",
                g_state.i2c.port,
                g_state.i2c.speed_khz,
                g_state.i2c.pullup ? "ON" : "OFF");
            return;
        }
        bool config_changed = false;
        if (strcmp(argv[2], "speed") == 0 && argc > 3) {
            g_state.i2c.speed_khz = parse_number(argv[3]);
            printf("OK I2C speed set to %lu kHz\r\n", g_state.i2c.speed_khz);
            config_changed = true;
        } else if (strcmp(argv[2], "port") == 0 && argc > 3) {
            g_state.i2c.port = parse_number(argv[3]) & 1;
            printf("OK I2C port set to %d\r\n", g_state.i2c.port);
            config_changed = true;
        } else if (strcmp(argv[2], "pullup") == 0 && argc > 3) {
            g_state.i2c.pullup = strcmp(argv[3], "on") == 0;
            printf("OK Pullup %s\r\n", g_state.i2c.pullup ? "enabled" : "disabled");
            config_changed = true;
        }
        // 配置改变时重新初始化 I2C 硬件
        if (config_changed) {
            ipc_cmd_t cmd = {
                .cmd = IPC_CMD_CONFIG,
                .protocol = PROTO_I2C,
                .data_len = 0,
                .param = 0
            };
            fifo_ipc_send_cmd(&cmd);
            ipc_resp_t resp;
            fifo_ipc_recv_resp(&resp);
        }
    }
    else if (strcmp(subcmd, "scan") == 0) {
        printf("Scanning I2C bus...\r\n");

        // 发送 IPC 命令到 Core1
        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_I2C_SCAN,
            .protocol = PROTO_I2C,
            .data_len = 0,
            .param = 0
        };
        fifo_ipc_send_cmd(&cmd);

        // 等待响应
        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK && resp.data_len > 0) {
                printf("Found %d device(s):\r\n", resp.data_len);
                for (int i = 0; i < resp.data_len; i++) {
                    printf("  0x%02X\r\n", g_shared_rx_buf[i]);
                }
            } else {
                printf("No devices found.\r\n");
            }
        } else {
            printf("ERR E05 Timeout\r\n");
        }
    }
    else if (strcmp(subcmd, "detect") == 0 && argc > 2) {
        uint8_t addr = parse_number(argv[2]);
        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_I2C_READ,
            .protocol = PROTO_I2C,
            .data_len = 0,
            .param = (addr << 16) | 1  // 地址左移16位，读取1字节
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK Device found at 0x%02X\r\n", addr);
            } else {
                printf("ERR E03 No device at 0x%02X\r\n", addr);
            }
        }
    }
    else if (strcmp(subcmd, "read") == 0 && argc > 3) {
        uint8_t addr = parse_number(argv[2]);
        uint16_t len;
        uint8_t reg = 0;
        bool has_reg = false;

        if (argc > 4) {
            // 有寄存器地址
            reg = parse_number(argv[3]);
            len = parse_number(argv[4]);
            has_reg = true;
        } else {
            len = parse_number(argv[3]);
        }

        mutex_enter_blocking(&g_shared_buf_mutex);
        if (has_reg) {
            g_shared_tx_buf[0] = reg;
        }

        ipc_cmd_t cmd = {
            .cmd = has_reg ? IPC_CMD_I2C_TRANSFER : IPC_CMD_I2C_READ,
            .protocol = PROTO_I2C,
            .data_len = has_reg ? 1 : 0,
            .param = (addr << 16) | len
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK ");
                for (int i = 0; i < resp.data_len; i++) {
                    printf("%02X ", g_shared_rx_buf[i]);
                }
                printf("\r\n");
            } else {
                printf("ERR E03 Read failed\r\n");
            }
        }
        mutex_exit(&g_shared_buf_mutex);
    }
    else if (strcmp(subcmd, "write") == 0 && argc > 3) {
        uint8_t addr = parse_number(argv[2]);

        mutex_enter_blocking(&g_shared_buf_mutex);
        int data_len = 0;
        for (int i = 3; i < argc && data_len < DATA_BUFFER_SIZE; i++) {
            g_shared_tx_buf[data_len++] = parse_number(argv[i]);
        }

        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_I2C_WRITE,
            .protocol = PROTO_I2C,
            .data_len = data_len,
            .param = (addr << 16)
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK\r\n");
            } else {
                printf("ERR E03 Write failed\r\n");
            }
        }
        mutex_exit(&g_shared_buf_mutex);
    }
    else if (strcmp(subcmd, "transfer") == 0) {
        // 解析: i2c transfer <addr> <write_data...> -r <read_len>
        if (argc < 4) {
            printf("Usage: i2c transfer <addr> [data...] -r <len>\r\n");
            return;
        }

        uint8_t addr = parse_number(argv[2]);
        int read_len = 0;
        int write_len = 0;

        mutex_enter_blocking(&g_shared_buf_mutex);

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
                read_len = parse_number(argv[i + 1]);
                break;
            }
            g_shared_tx_buf[write_len++] = parse_number(argv[i]);
        }

        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_I2C_TRANSFER,
            .protocol = PROTO_I2C,
            .data_len = write_len,
            .param = (addr << 16) | read_len
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK ");
                for (int i = 0; i < resp.data_len; i++) {
                    printf("%02X ", g_shared_rx_buf[i]);
                }
                printf("\r\n");
            } else {
                printf("ERR E03 Transfer failed\r\n");
            }
        }
        mutex_exit(&g_shared_buf_mutex);
    }
    else if (strcmp(subcmd, "dump") == 0) {
        // i2c dump <addr> [start_reg] [len]
        if (argc < 3) {
            printf("Usage: i2c dump <addr> [start_reg] [len]\r\n");
            return;
        }
        uint8_t addr = parse_number(argv[2]);
        uint8_t start_reg = (argc > 3) ? parse_number(argv[3]) : 0;
        uint16_t len = (argc > 4) ? parse_number(argv[4]) : 256;

        printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");

        mutex_enter_blocking(&g_shared_buf_mutex);
        for (int row = 0; row < 16 && (row * 16) < len; row++) {
            printf("%02X: ", start_reg + row * 16);
            for (int col = 0; col < 16; col++) {
                uint8_t reg = start_reg + row * 16 + col;
                if ((row * 16 + col) >= len) {
                    printf("   ");
                    continue;
                }
                g_shared_tx_buf[0] = reg;
                ipc_cmd_t cmd = {
                    .cmd = IPC_CMD_I2C_TRANSFER,
                    .protocol = PROTO_I2C,
                    .data_len = 1,
                    .param = (addr << 16) | 1
                };
                fifo_ipc_send_cmd(&cmd);

                ipc_resp_t resp;
                if (fifo_ipc_recv_resp(&resp) && resp.status == ERR_OK) {
                    printf("%02X ", g_shared_rx_buf[0]);
                } else {
                    printf("-- ");
                }
            }
            printf("\r\n");
        }
        mutex_exit(&g_shared_buf_mutex);
    }
    else {
        printf("ERR E02 Unknown I2C subcommand: %s\r\n", subcmd);
    }
}

// ============================================================================
// SPI 命令
// ============================================================================

static void cmd_spi(int argc, char *argv[]) {
    if (argc < 2) {
        printf("SPI subcommands: config, transfer, write, read\r\n");
        return;
    }

    const char *subcmd = argv[1];

    if (strcmp(subcmd, "config") == 0) {
        if (argc < 3) {
            printf("Speed: %lu kHz, Mode: %d, CS: GP%d, Bits: %d, Order: %s\r\n",
                g_state.spi.speed_khz,
                g_state.spi.mode,
                g_state.spi.cs_pin,
                g_state.spi.bits,
                g_state.spi.lsb_first ? "LSB" : "MSB");
            return;
        }
        bool config_changed = false;
        if (strcmp(argv[2], "speed") == 0 && argc > 3) {
            g_state.spi.speed_khz = parse_number(argv[3]);
            printf("OK SPI speed set to %lu kHz\r\n", g_state.spi.speed_khz);
            config_changed = true;
        } else if (strcmp(argv[2], "mode") == 0 && argc > 3) {
            g_state.spi.mode = parse_number(argv[3]) & 0x03;
            printf("OK SPI mode set to %d\r\n", g_state.spi.mode);
            config_changed = true;
        } else if (strcmp(argv[2], "cs") == 0 && argc > 3) {
            uint8_t cs = parse_number(argv[3]);
            switch (cs) {
                case 0: g_state.spi.cs_pin = PIN_SPI_CS0; break;
                case 1: g_state.spi.cs_pin = PIN_SPI_CS1; break;
                case 2: g_state.spi.cs_pin = PIN_SPI_CS2; break;
                default: printf("ERR E02 Invalid CS\r\n"); return;
            }
            printf("OK CS set to %d\r\n", cs);
            // CS 选择不需要重新初始化
        }
        // 配置改变时重新初始化 SPI 硬件
        if (config_changed) {
            ipc_cmd_t cmd = {
                .cmd = IPC_CMD_CONFIG,
                .protocol = PROTO_SPI,
                .data_len = 0,
                .param = 0
            };
            fifo_ipc_send_cmd(&cmd);
            ipc_resp_t resp;
            fifo_ipc_recv_resp(&resp);
        }
    }
    else if (strcmp(subcmd, "transfer") == 0 && argc > 2) {
        mutex_enter_blocking(&g_shared_buf_mutex);
        int data_len = 0;
        for (int i = 2; i < argc && data_len < DATA_BUFFER_SIZE; i++) {
            g_shared_tx_buf[data_len++] = parse_number(argv[i]);
        }

        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_SPI_TRANSFER,
            .protocol = PROTO_SPI,
            .data_len = data_len,
            .param = g_state.spi.cs_pin
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK ");
                for (int i = 0; i < resp.data_len; i++) {
                    printf("%02X ", g_shared_rx_buf[i]);
                }
                printf("\r\n");
            } else {
                printf("ERR E03 Transfer failed\r\n");
            }
        }
        mutex_exit(&g_shared_buf_mutex);
    }
    else if (strcmp(subcmd, "write") == 0 && argc > 2) {
        mutex_enter_blocking(&g_shared_buf_mutex);
        int data_len = 0;
        for (int i = 2; i < argc && data_len < DATA_BUFFER_SIZE; i++) {
            g_shared_tx_buf[data_len++] = parse_number(argv[i]);
        }

        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_SPI_WRITE,
            .protocol = PROTO_SPI,
            .data_len = data_len,
            .param = g_state.spi.cs_pin
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK Sent %d bytes\r\n", data_len);
            } else {
                printf("ERR E03 Write failed\r\n");
            }
        }
        mutex_exit(&g_shared_buf_mutex);
    }
    else if (strcmp(subcmd, "read") == 0 && argc > 2) {
        uint16_t len = parse_number(argv[2]);
        if (len > DATA_BUFFER_SIZE) len = DATA_BUFFER_SIZE;

        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_SPI_READ,
            .protocol = PROTO_SPI,
            .data_len = 0,
            .param = g_state.spi.cs_pin | (len << 8)
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK ");
                for (int i = 0; i < resp.data_len; i++) {
                    printf("%02X ", g_shared_rx_buf[i]);
                }
                printf("\r\n");
            } else {
                printf("ERR E03 Read failed\r\n");
            }
        }
    }
    else {
        printf("ERR E02 Unknown SPI subcommand: %s\r\n", subcmd);
    }
}

// ============================================================================
// UART 命令
// ============================================================================

static void cmd_uart(int argc, char *argv[]) {
    if (argc < 2) {
        printf("UART subcommands: config, send, recv, bridge, monitor\r\n");
        return;
    }

    const char *subcmd = argv[1];

    if (strcmp(subcmd, "config") == 0) {
        if (argc < 3) {
            printf("Baudrate: %lu, Data: %d%c%d\r\n",
                g_state.uart.baudrate,
                g_state.uart.data_bits,
                g_state.uart.parity == 0 ? 'N' : (g_state.uart.parity == 1 ? 'E' : 'O'),
                g_state.uart.stop_bits);
            return;
        }
        if (strcmp(argv[2], "baud") == 0 && argc > 3) {
            g_state.uart.baudrate = parse_number(argv[3]);
            uart_pio_set_baudrate(g_state.uart.baudrate);
            printf("OK Baudrate set to %lu\r\n", g_state.uart.baudrate);
        }
    }
    else if (strcmp(subcmd, "send") == 0 && argc > 2) {
        mutex_enter_blocking(&g_shared_buf_mutex);
        int data_len = 0;

        // 检查是否是字符串
        if (argv[2][0] == '"') {
            char *str = argv[2] + 1;
            while (*str && *str != '"' && data_len < DATA_BUFFER_SIZE) {
                if (*str == '\\' && *(str + 1)) {
                    str++;
                    switch (*str) {
                        case 'r': g_shared_tx_buf[data_len++] = '\r'; break;
                        case 'n': g_shared_tx_buf[data_len++] = '\n'; break;
                        case 't': g_shared_tx_buf[data_len++] = '\t'; break;
                        default: g_shared_tx_buf[data_len++] = *str; break;
                    }
                } else {
                    g_shared_tx_buf[data_len++] = *str;
                }
                str++;
            }
        } else {
            // 十六进制数据
            for (int i = 2; i < argc && data_len < DATA_BUFFER_SIZE; i++) {
                g_shared_tx_buf[data_len++] = parse_number(argv[i]);
            }
        }

        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_UART_SEND,
            .protocol = PROTO_UART,
            .data_len = data_len,
            .param = 0
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK) {
                printf("OK Sent %d bytes\r\n", data_len);
            } else {
                printf("ERR E03 Send failed\r\n");
            }
        }
        mutex_exit(&g_shared_buf_mutex);
    }
    else if (strcmp(subcmd, "recv") == 0) {
        // uart recv [timeout_ms]
        uint32_t timeout = (argc > 2) ? parse_number(argv[2]) : 1000;

        ipc_cmd_t cmd = {
            .cmd = IPC_CMD_UART_RECV,
            .protocol = PROTO_UART,
            .data_len = 0,
            .param = timeout
        };
        fifo_ipc_send_cmd(&cmd);

        ipc_resp_t resp;
        if (fifo_ipc_recv_resp(&resp)) {
            if (resp.status == ERR_OK && resp.data_len > 0) {
                printf("OK ");
                for (int i = 0; i < resp.data_len; i++) {
                    printf("%02X ", g_shared_rx_buf[i]);
                }
                printf("\r\n");
            } else if (resp.data_len == 0) {
                printf("OK No data received\r\n");
            } else {
                printf("ERR E05 Timeout\r\n");
            }
        }
    }
    else if (strcmp(subcmd, "bridge") == 0) {
        // uart bridge [--hex]
        bool hex_mode = false;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--hex") == 0 || strcmp(argv[i], "-x") == 0) {
                hex_mode = true;
            }
        }

        printf("--- UART Bridge Mode (Ctrl+] x3 to exit) ---\r\n");
        printf("--- Baudrate: %lu, Data: %d%c%d ---\r\n",
            g_state.uart.baudrate,
            g_state.uart.data_bits,
            g_state.uart.parity == 0 ? 'N' : (g_state.uart.parity == 1 ? 'E' : 'O'),
            g_state.uart.stop_bits);
        if (hex_mode) {
            printf("--- HEX display mode ---\r\n");
        }

        g_state.stream_hex = hex_mode;
        g_state.stream_mode = STREAM_UART_BRIDGE;
        // 主循环会处理流模式，此函数返回后不需要打印提示符
    }
    else if (strcmp(subcmd, "monitor") == 0) {
        // uart monitor [--hex]
        bool hex_mode = false;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--hex") == 0 || strcmp(argv[i], "-x") == 0) {
                hex_mode = true;
            }
        }

        printf("--- UART Monitor Mode (Ctrl+] x3 to exit) ---\r\n");
        printf("--- Baudrate: %lu ---\r\n", g_state.uart.baudrate);
        if (hex_mode) {
            printf("--- HEX display mode ---\r\n");
        }

        g_state.stream_hex = hex_mode;
        g_state.stream_mode = STREAM_UART_MONITOR;
    }
    else {
        printf("ERR E02 Unknown UART subcommand: %s\r\n", subcmd);
    }
}

// ============================================================================
// GPIO 命令
// ============================================================================

static void cmd_gpio(int argc, char *argv[]) {
    if (argc < 2) {
        printf("GPIO subcommands: mode, write, read\r\n");
        return;
    }

    const char *subcmd = argv[1];

    if (strcmp(subcmd, "mode") == 0) {
        if (argc < 4) {
            printf("Usage: gpio mode <pin> <out|in|in_pullup|in_pulldown>\r\n");
            return;
        }
        uint8_t pin = parse_number(argv[2]);
        const char *mode = argv[3];

        gpio_init(pin);
        if (strcmp(mode, "out") == 0) {
            gpio_set_dir(pin, GPIO_OUT);
        } else if (strcmp(mode, "in") == 0) {
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
        } else if (strcmp(mode, "in_pullup") == 0) {
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
        } else if (strcmp(mode, "in_pulldown") == 0) {
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
        } else {
            printf("ERR E02 Invalid mode. Use: out, in, in_pullup, in_pulldown\r\n");
            return;
        }
        printf("OK\r\n");
    }
    else if (strcmp(subcmd, "write") == 0) {
        if (argc < 4) {
            printf("Usage: gpio write <pin> <0|1|toggle>\r\n");
            return;
        }
        uint8_t pin = parse_number(argv[2]);
        if (strcmp(argv[3], "toggle") == 0) {
            gpio_put(pin, !gpio_get(pin));
        } else {
            gpio_put(pin, parse_number(argv[3]) != 0);
        }
        printf("OK\r\n");
    }
    else if (strcmp(subcmd, "read") == 0) {
        if (argc < 3) {
            printf("Usage: gpio read <pin|all>\r\n");
            return;
        }
        if (strcmp(argv[2], "all") == 0) {
            printf("OK ");
            for (int pin = PIN_GPIO_0; pin <= PIN_GPIO_3; pin++) {
                printf("GP%d:%d ", pin, gpio_get(pin));
            }
            printf("\r\n");
        } else {
            uint8_t pin = parse_number(argv[2]);
            printf("OK %d\r\n", gpio_get(pin));
        }
    }
    else {
        printf("ERR E02 Unknown GPIO subcommand: %s\r\n", subcmd);
    }
}
