#ifndef UART_TO_X_H
#define UART_TO_X_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

// ============================================================================
// 版本信息
// ============================================================================
#define UART_TO_X_VERSION_MAJOR  1
#define UART_TO_X_VERSION_MINOR  0
#define UART_TO_X_VERSION_PATCH  0
#define UART_TO_X_VERSION_STRING "1.0.0"

// ============================================================================
// 引脚定义
// ============================================================================

// I2C 引脚
#define PIN_I2C0_SDA    4
#define PIN_I2C0_SCL    5
#define PIN_I2C1_SDA    6
#define PIN_I2C1_SCL    7

// SPI 引脚
#define PIN_SPI_MISO    16
#define PIN_SPI_MOSI    17
#define PIN_SPI_SCK     18
#define PIN_SPI_CS0     19
#define PIN_SPI_CS1     20
#define PIN_SPI_CS2     21

// UART 引脚
#define PIN_UART_TX     8
#define PIN_UART_RX     9

// 1-Wire 引脚
#define PIN_ONEWIRE     10

// 通用 GPIO
#define PIN_GPIO_0      11
#define PIN_GPIO_1      12
#define PIN_GPIO_2      13
#define PIN_GPIO_3      14

// LED
#define PIN_LED         25

// ============================================================================
// 缓冲区大小
// ============================================================================
#define CMD_BUFFER_SIZE     256     // 命令缓冲区
#define RESP_BUFFER_SIZE    512     // 响应缓冲区
#define DATA_BUFFER_SIZE    4096    // 数据缓冲区
#define RING_BUFFER_SIZE    8192    // 环形缓冲区

// ============================================================================
// 协议配置限制
// ============================================================================
#define I2C_MAX_SPEED_KHZ   1000    // 1 MHz
#define SPI_MAX_SPEED_KHZ   20000   // 20 MHz
#define UART_MAX_BAUD       3000000 // 3 Mbps

// ============================================================================
// 错误码
// ============================================================================
typedef enum {
    ERR_OK              = 0x00,     // 成功
    ERR_UNKNOWN_CMD     = 0x01,     // 未知命令
    ERR_INVALID_PARAM   = 0x02,     // 参数错误
    ERR_NO_RESPONSE     = 0x03,     // 设备无响应
    ERR_BUS_BUSY        = 0x04,     // 总线忙
    ERR_TIMEOUT         = 0x05,     // 超时
    ERR_CRC             = 0x06,     // CRC 错误
    ERR_OVERFLOW        = 0x07,     // 缓冲区溢出
    ERR_NOT_INIT        = 0x08,     // 协议未初始化
    ERR_PIN_CONFLICT    = 0x09,     // 引脚被占用
} error_code_t;

// ============================================================================
// 传输模式
// ============================================================================
typedef enum {
    MODE_TEXT   = 0,    // 文本模式
    MODE_BIN    = 1,    // 二进制模式
    MODE_STREAM = 2,    // 流模式
} transfer_mode_t;

// ============================================================================
// 协议类型
// ============================================================================
typedef enum {
    PROTO_NONE  = 0,
    PROTO_I2C   = 1,
    PROTO_SPI   = 2,
    PROTO_UART  = 3,
    PROTO_W1    = 4,    // 1-Wire
    PROTO_GPIO  = 5,
} protocol_type_t;

// ============================================================================
// I2C 配置
// ============================================================================
typedef struct {
    uint8_t     port;           // 0 或 1
    uint32_t    speed_khz;      // 速度 (kHz)
    bool        pullup;         // 内部上拉
} i2c_config_t;

// ============================================================================
// SPI 配置
// ============================================================================
typedef struct {
    uint32_t    speed_khz;      // 速度 (kHz)
    uint8_t     mode;           // SPI 模式 (0-3)
    uint8_t     cs_pin;         // 片选引脚
    uint8_t     bits;           // 位宽 (8/16)
    bool        lsb_first;      // LSB 优先
} spi_config_t;

// ============================================================================
// UART 配置
// ============================================================================
typedef struct {
    uint32_t    baudrate;       // 波特率
    uint8_t     data_bits;      // 数据位
    uint8_t     stop_bits;      // 停止位
    uint8_t     parity;         // 校验: 0=无, 1=偶, 2=奇
} uart_config_t;

// ============================================================================
// IPC 命令结构 (核间通信)
// ============================================================================
typedef struct {
    uint8_t     cmd;            // 命令类型
    uint8_t     protocol;       // 协议类型
    uint16_t    data_len;       // 数据长度
    uint32_t    param;          // 参数 (地址/引脚等)
} ipc_cmd_t;

// ============================================================================
// IPC 响应结构
// ============================================================================
typedef struct {
    uint8_t     status;         // 状态码
    uint16_t    data_len;       // 数据长度
} ipc_resp_t;

// ============================================================================
// IPC 命令码
// ============================================================================
#define IPC_CMD_I2C_WRITE       0x01
#define IPC_CMD_I2C_READ        0x02
#define IPC_CMD_I2C_TRANSFER    0x03
#define IPC_CMD_I2C_SCAN        0x04
#define IPC_CMD_SPI_TRANSFER    0x10
#define IPC_CMD_SPI_WRITE       0x11
#define IPC_CMD_SPI_READ        0x12
#define IPC_CMD_UART_SEND       0x20
#define IPC_CMD_UART_RECV       0x21
#define IPC_CMD_GPIO_WRITE      0x30
#define IPC_CMD_GPIO_READ       0x31
#define IPC_CMD_CONFIG          0xF0
#define IPC_CMD_PING            0xFE
#define IPC_CMD_RESET           0xFF

// ============================================================================
// 全局状态
// ============================================================================
typedef struct {
    transfer_mode_t mode;
    i2c_config_t    i2c;
    spi_config_t    spi;
    uart_config_t   uart;
    bool            running;
} system_state_t;

extern system_state_t g_state;

// ============================================================================
// 函数声明
// ============================================================================

// Core0 函数
void shell_init(void);
void shell_task(void);
void usb_cdc_init(void);
void usb_cdc_task(void);
int  usb_cdc_read(uint8_t *buf, size_t len);
int  usb_cdc_write(const uint8_t *buf, size_t len);
void response_ok(const char *data);
void response_error(error_code_t code, const char *msg);
void response_data(const uint8_t *data, size_t len);

// Core1 函数
void protocol_manager_init(void);
void protocol_manager_task(void);

// I2C 函数
void i2c_pio_init(void);
int  i2c_pio_write(uint8_t addr, const uint8_t *data, size_t len);
int  i2c_pio_read(uint8_t addr, uint8_t *data, size_t len);
int  i2c_pio_transfer(uint8_t addr, const uint8_t *tx, size_t tx_len,
                      uint8_t *rx, size_t rx_len);
int  i2c_pio_scan(uint8_t *addrs, size_t max_count);

// SPI 函数
void spi_pio_init(void);
int  spi_pio_transfer(const uint8_t *tx, uint8_t *rx, size_t len);
void spi_pio_cs_select(uint8_t cs);
void spi_pio_cs_deselect(void);

// UART 函数
void uart_pio_init(void);
int  uart_pio_send(const uint8_t *data, size_t len);
int  uart_pio_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms);

// GPIO 函数
void gpio_ctrl_init(void);
void gpio_ctrl_set_mode(uint8_t pin, uint8_t mode);
void gpio_ctrl_write(uint8_t pin, bool value);
bool gpio_ctrl_read(uint8_t pin);

// IPC 函数
void fifo_ipc_init(void);
bool fifo_ipc_send_cmd(const ipc_cmd_t *cmd);
bool fifo_ipc_recv_cmd(ipc_cmd_t *cmd);
bool fifo_ipc_send_resp(const ipc_resp_t *resp);
bool fifo_ipc_recv_resp(ipc_resp_t *resp);

// 共享数据缓冲区
extern uint8_t g_shared_tx_buf[DATA_BUFFER_SIZE];
extern uint8_t g_shared_rx_buf[DATA_BUFFER_SIZE];
extern mutex_t g_shared_buf_mutex;

// 工具函数
uint16_t crc16(const uint8_t *data, size_t len);
int hex_to_bytes(const char *hex, uint8_t *bytes, size_t max_len);
void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex);

#endif // UART_TO_X_H
