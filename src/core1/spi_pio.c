/**
 * SPI 驱动 - 使用硬件 SPI
 */

#include "../include/uart_to_x.h"
#include "hardware/spi.h"

static spi_inst_t *spi_inst = spi0;
static bool spi_initialized = false;
static uint8_t current_cs_pin = PIN_SPI_CS0;

/**
 * 初始化 SPI
 */
void spi_pio_init(void) {
    // 如果已初始化，先反初始化
    if (spi_initialized) {
        spi_deinit(spi_inst);
    }

    // 初始化 CS 引脚 (只做一次)
    if (!spi_initialized) {
        gpio_init(PIN_SPI_CS0);
        gpio_init(PIN_SPI_CS1);
        gpio_init(PIN_SPI_CS2);
        gpio_set_dir(PIN_SPI_CS0, GPIO_OUT);
        gpio_set_dir(PIN_SPI_CS1, GPIO_OUT);
        gpio_set_dir(PIN_SPI_CS2, GPIO_OUT);
        gpio_put(PIN_SPI_CS0, 1);
        gpio_put(PIN_SPI_CS1, 1);
        gpio_put(PIN_SPI_CS2, 1);
    }

    // 初始化 SPI
    spi_init(spi_inst, g_state.spi.speed_khz * 1000);

    // 配置 SPI 模式
    spi_cpol_t cpol = (g_state.spi.mode & 0x02) ? SPI_CPOL_1 : SPI_CPOL_0;
    spi_cpha_t cpha = (g_state.spi.mode & 0x01) ? SPI_CPHA_1 : SPI_CPHA_0;
    spi_set_format(spi_inst, 8, cpol, cpha, SPI_MSB_FIRST);

    // 配置 GPIO
    gpio_set_function(PIN_SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_SCK, GPIO_FUNC_SPI);

    spi_initialized = true;
}

/**
 * 选择 CS
 */
void spi_pio_cs_select(uint8_t cs) {
    current_cs_pin = cs;
    gpio_put(cs, 0);
}

/**
 * 取消选择 CS
 */
void spi_pio_cs_deselect(void) {
    gpio_put(current_cs_pin, 1);
}

/**
 * SPI 传输 (全双工)
 */
int spi_pio_transfer(const uint8_t *tx, uint8_t *rx, size_t len) {
    if (!spi_initialized) return -1;

    if (tx && rx) {
        // 全双工
        spi_write_read_blocking(spi_inst, tx, rx, len);
    } else if (tx) {
        // 只写
        spi_write_blocking(spi_inst, tx, len);
    } else if (rx) {
        // 只读
        spi_read_blocking(spi_inst, 0xFF, rx, len);
    }

    return len;
}
