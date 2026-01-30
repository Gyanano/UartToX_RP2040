/**
 * UartToX - 多协议转换工具
 * 主程序入口
 */

#include "include/uart_to_x.h"

// 全局状态
system_state_t g_state = {
    .mode = MODE_TEXT,
    .i2c = {
        .port = 0,
        .speed_khz = 400,
        .pullup = true
    },
    .spi = {
        .speed_khz = 1000,
        .mode = 0,
        .cs_pin = PIN_SPI_CS0,
        .bits = 8,
        .lsb_first = false
    },
    .uart = {
        .baudrate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 0
    },
    .running = true
};

// 共享数据缓冲区
uint8_t g_shared_tx_buf[DATA_BUFFER_SIZE];
uint8_t g_shared_rx_buf[DATA_BUFFER_SIZE];
mutex_t g_shared_buf_mutex;

/**
 * Core1 入口函数
 * 负责协议处理
 */
void core1_entry(void) {
    // 初始化协议管理器
    protocol_manager_init();

    // 协议处理主循环
    while (g_state.running) {
        protocol_manager_task();
    }
}

/**
 * 系统初始化
 */
static void system_init(void) {
    // 初始化标准库
    stdio_init_all();

    // 初始化互斥锁
    mutex_init(&g_shared_buf_mutex);

    // 初始化 IPC
    fifo_ipc_init();

    // 初始化 LED
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);
    gpio_put(PIN_LED, 1);

    // 初始化 USB CDC
    usb_cdc_init();

    // 初始化 Shell
    shell_init();
}

/**
 * 主函数
 */
int main(void) {
    // 系统初始化
    system_init();

    // 启动 Core1
    multicore_launch_core1(core1_entry);

    // 连接状态跟踪
    bool was_connected = false;
    uint32_t last_blink_time = 0;

    // Core0 主循环
    while (g_state.running) {
        bool is_connected = stdio_usb_connected();

        if (is_connected) {
            // 连接时 LED 常亮
            gpio_put(PIN_LED, 1);

            // 刚连接时打印欢迎信息
            if (!was_connected) {
                sleep_ms(100);  // 等待终端就绪
                printf("\r\n");
                printf("╔═══════════════════════════════════════╗\r\n");
                printf("║         UartToX v%s                ║\r\n", UART_TO_X_VERSION_STRING);
                printf("║   Multi-Protocol Converter Tool       ║\r\n");
                printf("║   Type 'help' for commands            ║\r\n");
                printf("╚═══════════════════════════════════════╝\r\n");
                printf("\r\n> ");
            }

            usb_cdc_task();
            shell_task();
        } else {
            // 断开时 LED 闪烁（500ms 周期）
            uint32_t now = time_us_32() / 1000;
            if (now - last_blink_time >= 250) {
                gpio_put(PIN_LED, !gpio_get(PIN_LED));
                last_blink_time = now;
            }

            // 断开时重置 shell 状态
            if (was_connected) {
                shell_init();
            }
            sleep_ms(10);  // 降低 CPU 占用
        }

        was_connected = is_connected;
    }

    return 0;
}
