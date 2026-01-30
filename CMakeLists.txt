cmake_minimum_required(VERSION 3.13)

# 初始化 Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(uart_to_x C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化 SDK
pico_sdk_init()

# 添加可执行文件
add_executable(uart_to_x
    src/main.c
    src/core0/shell.c
    src/core0/usb_cdc.c
    src/core0/response.c
    src/core0/cmd_parser.c
    src/core1/protocol_manager.c
    src/core1/i2c_pio.c
    src/core1/spi_pio.c
    src/core1/uart_pio.c
    src/core1/gpio_ctrl.c
    src/common/fifo_ipc.c
    src/common/ring_buffer.c
    src/common/utils.c
)

# 生成 PIO 头文件
pico_generate_pio_header(uart_to_x ${CMAKE_CURRENT_LIST_DIR}/src/pio/i2c.pio)
pico_generate_pio_header(uart_to_x ${CMAKE_CURRENT_LIST_DIR}/src/pio/spi.pio)
pico_generate_pio_header(uart_to_x ${CMAKE_CURRENT_LIST_DIR}/src/pio/uart_tx.pio)
pico_generate_pio_header(uart_to_x ${CMAKE_CURRENT_LIST_DIR}/src/pio/uart_rx.pio)

# 包含目录
target_include_directories(uart_to_x PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

# 链接库
target_link_libraries(uart_to_x
    pico_stdlib
    pico_multicore
    pico_sync
    hardware_pio
    hardware_dma
    hardware_irq
    hardware_gpio
    hardware_pwm
    hardware_i2c
    hardware_spi
    hardware_clocks
)

# 使用 USB 作为 stdio
pico_enable_stdio_usb(uart_to_x 1)
pico_enable_stdio_uart(uart_to_x 0)

# 生成额外的输出文件
pico_add_extra_outputs(uart_to_x)
