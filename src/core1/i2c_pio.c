/**
 * I2C 驱动 - 使用硬件 I2C
 */

#include "../include/uart_to_x.h"
#include "hardware/i2c.h"

static i2c_inst_t *i2c_inst = i2c0;
static bool i2c_initialized = false;

/**
 * 初始化 I2C
 */
void i2c_pio_init(void) {
    // 根据端口选择 I2C 实例
    i2c_inst = (g_state.i2c.port == 0) ? i2c0 : i2c1;

    uint sda_pin = (g_state.i2c.port == 0) ? PIN_I2C0_SDA : PIN_I2C1_SDA;
    uint scl_pin = (g_state.i2c.port == 0) ? PIN_I2C0_SCL : PIN_I2C1_SCL;

    // 初始化 I2C
    i2c_init(i2c_inst, g_state.i2c.speed_khz * 1000);

    // 配置 GPIO
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // 上拉电阻
    if (g_state.i2c.pullup) {
        gpio_pull_up(sda_pin);
        gpio_pull_up(scl_pin);
    }

    i2c_initialized = true;
}

/**
 * I2C 写入
 */
int i2c_pio_write(uint8_t addr, const uint8_t *data, size_t len) {
    if (!i2c_initialized || len == 0) return -1;

    int ret = i2c_write_blocking(i2c_inst, addr, data, len, false);
    return (ret == PICO_ERROR_GENERIC) ? -1 : ret;
}

/**
 * I2C 读取
 */
int i2c_pio_read(uint8_t addr, uint8_t *data, size_t len) {
    if (!i2c_initialized || len == 0) return -1;

    int ret = i2c_read_blocking(i2c_inst, addr, data, len, false);
    return (ret == PICO_ERROR_GENERIC) ? -1 : ret;
}

/**
 * I2C 组合传输 (先写后读)
 */
int i2c_pio_transfer(uint8_t addr, const uint8_t *tx, size_t tx_len,
                     uint8_t *rx, size_t rx_len) {
    if (!i2c_initialized) return -1;

    // 写入阶段 (保持总线)
    if (tx && tx_len > 0) {
        int ret = i2c_write_blocking(i2c_inst, addr, tx, tx_len, true);
        if (ret == PICO_ERROR_GENERIC) return -1;
    }

    // 读取阶段
    if (rx && rx_len > 0) {
        int ret = i2c_read_blocking(i2c_inst, addr, rx, rx_len, false);
        if (ret == PICO_ERROR_GENERIC) return -1;
        return ret;
    }

    return 0;
}

/**
 * I2C 总线扫描
 */
int i2c_pio_scan(uint8_t *addrs, size_t max_count) {
    if (!i2c_initialized) return -1;

    int count = 0;
    uint8_t dummy;

    for (uint8_t addr = 0x08; addr < 0x78 && (size_t)count < max_count; addr++) {
        // 尝试读取一个字节来检测设备
        int ret = i2c_read_blocking(i2c_inst, addr, &dummy, 1, false);
        if (ret >= 0) {
            addrs[count++] = addr;
        }
    }

    return count;
}
