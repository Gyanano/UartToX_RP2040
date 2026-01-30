/**
 * USB CDC 驱动
 * 注意：使用 pico_stdio_usb，不需要直接操作 TinyUSB
 */

#include "../include/uart_to_x.h"

void usb_cdc_init(void) {
    // stdio_init_all() 已经初始化了 USB CDC
    // 不需要额外初始化
}

void usb_cdc_task(void) {
    // stdio_usb 内部处理，不需要手动调用 tud_task()
    // 保持函数为空或执行其他周期性任务
    tight_loop_contents();
}

int usb_cdc_read(uint8_t *buf, size_t len) {
    size_t count = 0;
    while (count < len) {
        int c = getchar_timeout_us(0);
        if (c == PICO_ERROR_TIMEOUT) {
            break;
        }
        buf[count++] = (uint8_t)c;
    }
    return count;
}

int usb_cdc_write(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        putchar(buf[i]);
    }
    return len;
}
