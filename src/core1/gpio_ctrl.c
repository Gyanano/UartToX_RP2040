/**
 * GPIO 控制
 */

#include "../include/uart_to_x.h"

// GPIO 引脚状态跟踪
static bool gpio_pin_initialized[30] = {false};

/**
 * 初始化 GPIO 控制
 */
void gpio_ctrl_init(void) {
    // 预初始化通用 GPIO 引脚
    uint8_t gpio_pins[] = {PIN_GPIO_0, PIN_GPIO_1, PIN_GPIO_2, PIN_GPIO_3};

    for (int i = 0; i < sizeof(gpio_pins); i++) {
        gpio_init(gpio_pins[i]);
        gpio_set_dir(gpio_pins[i], GPIO_IN);
        gpio_pin_initialized[gpio_pins[i]] = true;
    }
}

/**
 * 设置 GPIO 模式
 */
void gpio_ctrl_set_mode(uint8_t pin, uint8_t mode) {
    if (pin >= 30) return;

    if (!gpio_pin_initialized[pin]) {
        gpio_init(pin);
        gpio_pin_initialized[pin] = true;
    }

    switch (mode) {
        case 0:  // Output
            gpio_set_dir(pin, GPIO_OUT);
            break;
        case 1:  // Input
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
            break;
        case 2:  // Input with pull-up
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
            break;
        case 3:  // Input with pull-down
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_down(pin);
            break;
    }
}

/**
 * 写入 GPIO
 */
void gpio_ctrl_write(uint8_t pin, bool value) {
    if (pin >= 30) return;

    if (!gpio_pin_initialized[pin]) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_pin_initialized[pin] = true;
    }

    gpio_put(pin, value);
}

/**
 * 读取 GPIO
 */
bool gpio_ctrl_read(uint8_t pin) {
    if (pin >= 30) return false;

    if (!gpio_pin_initialized[pin]) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pin_initialized[pin] = true;
    }

    return gpio_get(pin);
}
