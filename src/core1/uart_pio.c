/**
 * UART PIO 驱动
 */

#include "../include/uart_to_x.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "uart_tx.pio.h"
#include "uart_rx.pio.h"

static PIO uart_pio = pio1;
static uint uart_tx_sm = 0;
static uint uart_rx_sm = 1;
static uint uart_tx_offset = 0;
static uint uart_rx_offset = 0;
static bool uart_initialized = false;

/**
 * 初始化 UART PIO
 */
void uart_pio_init(void) {
    // 加载 TX 程序
    uart_tx_offset = pio_add_program(uart_pio, &uart_tx_program);
    uart_tx_sm = pio_claim_unused_sm(uart_pio, true);

    // 加载 RX 程序
    uart_rx_offset = pio_add_program(uart_pio, &uart_rx_program);
    uart_rx_sm = pio_claim_unused_sm(uart_pio, true);

    // 配置引脚
    pio_gpio_init(uart_pio, PIN_UART_TX);
    pio_gpio_init(uart_pio, PIN_UART_RX);

    // 初始化 TX
    uart_tx_program_init(uart_pio, uart_tx_sm, uart_tx_offset,
                         PIN_UART_TX, g_state.uart.baudrate);

    // 初始化 RX
    uart_rx_program_init(uart_pio, uart_rx_sm, uart_rx_offset,
                         PIN_UART_RX, g_state.uart.baudrate);

    uart_initialized = true;
}

/**
 * UART 发送
 */
int uart_pio_send(const uint8_t *data, size_t len) {
    if (!uart_initialized) return -1;

    for (size_t i = 0; i < len; i++) {
        pio_sm_put_blocking(uart_pio, uart_tx_sm, data[i]);
    }

    // 等待发送完成
    uint32_t start = time_us_32();
    while (!pio_sm_is_tx_fifo_empty(uart_pio, uart_tx_sm)) {
        if (time_us_32() - start > 1000000) {
            return -1;
        }
    }

    return len;
}

/**
 * UART 接收
 */
int uart_pio_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms) {
    if (!uart_initialized) return -1;

    size_t count = 0;
    uint32_t start = time_us_32();
    uint32_t timeout_us = timeout_ms * 1000;

    while (count < max_len) {
        // 检查是否有数据
        if (!pio_sm_is_rx_fifo_empty(uart_pio, uart_rx_sm)) {
            data[count++] = pio_sm_get(uart_pio, uart_rx_sm);
            start = time_us_32();  // 重置超时
        } else if (time_us_32() - start > timeout_us) {
            break;  // 超时退出
        }
    }

    return count;
}

/**
 * 配置 UART 波特率
 */
void uart_pio_set_baudrate(uint32_t baudrate) {
    if (!uart_initialized) return;

    float div = (float)clock_get_hz(clk_sys) / (baudrate * 8);
    pio_sm_set_clkdiv(uart_pio, uart_tx_sm, div);
    pio_sm_set_clkdiv(uart_pio, uart_rx_sm, div);
}

/**
 * 检查是否有数据可读 (非阻塞)
 */
int uart_pio_recv_available(void) {
    if (!uart_initialized) return 0;
    return !pio_sm_is_rx_fifo_empty(uart_pio, uart_rx_sm);
}

/**
 * 读取一个字节 (非阻塞)
 * 返回: 读取的字节 (0-255), 或 -1 表示无数据
 */
int uart_pio_recv_byte(void) {
    if (!uart_initialized) return -1;
    if (pio_sm_is_rx_fifo_empty(uart_pio, uart_rx_sm)) {
        return -1;
    }
    return pio_sm_get(uart_pio, uart_rx_sm) & 0xFF;
}
