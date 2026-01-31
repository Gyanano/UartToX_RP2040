/**
 * 协议管理器 - Core1 运行
 */

#include "../include/uart_to_x.h"

// 前向声明
static void handle_i2c_cmd(const ipc_cmd_t *cmd);
static void handle_spi_cmd(const ipc_cmd_t *cmd);
static void handle_uart_cmd(const ipc_cmd_t *cmd);
static void handle_gpio_cmd(const ipc_cmd_t *cmd);

/**
 * 协议管理器初始化
 */
void protocol_manager_init(void) {
    // 初始化各协议驱动
    i2c_pio_init();
    spi_pio_init();
    uart_pio_init();
    gpio_ctrl_init();
}

/**
 * 协议管理器任务 - 处理来自 Core0 的命令
 */
void protocol_manager_task(void) {
    ipc_cmd_t cmd;

    // 检查是否有新命令
    if (!fifo_ipc_recv_cmd(&cmd)) {
        tight_loop_contents();
        return;
    }

    // 根据协议类型分发处理
    switch (cmd.protocol) {
        case PROTO_I2C:
            handle_i2c_cmd(&cmd);
            break;

        case PROTO_SPI:
            handle_spi_cmd(&cmd);
            break;

        case PROTO_UART:
            handle_uart_cmd(&cmd);
            break;

        case PROTO_GPIO:
            handle_gpio_cmd(&cmd);
            break;

        default:
            // 未知协议
            {
                ipc_resp_t resp = {
                    .status = ERR_INVALID_PARAM,
                    .data_len = 0
                };
                fifo_ipc_send_resp(&resp);
            }
            break;
    }
}

/**
 * 处理 I2C 命令
 */
static void handle_i2c_cmd(const ipc_cmd_t *cmd) {
    ipc_resp_t resp = {
        .status = ERR_OK,
        .data_len = 0
    };

    uint8_t addr = (cmd->param >> 16) & 0xFF;
    uint16_t len = cmd->param & 0xFFFF;

    switch (cmd->cmd) {
        case IPC_CMD_I2C_SCAN: {
            // 扫描 I2C 总线
            uint8_t found[128];
            int count = i2c_pio_scan(found, 128);
            if (count > 0) {
                memcpy(g_shared_rx_buf, found, count);
                resp.data_len = count;
            }
            break;
        }

        case IPC_CMD_I2C_READ: {
            // 读取数据
            int ret = i2c_pio_read(addr, g_shared_rx_buf, len);
            if (ret < 0) {
                resp.status = ERR_NO_RESPONSE;
            } else {
                resp.data_len = ret;
            }
            break;
        }

        case IPC_CMD_I2C_WRITE: {
            // 写入数据
            int ret = i2c_pio_write(addr, g_shared_tx_buf, cmd->data_len);
            if (ret < 0) {
                resp.status = ERR_NO_RESPONSE;
            }
            break;
        }

        case IPC_CMD_I2C_TRANSFER: {
            // 先写后读
            int ret = i2c_pio_transfer(addr,
                                       g_shared_tx_buf, cmd->data_len,
                                       g_shared_rx_buf, len);
            if (ret < 0) {
                resp.status = ERR_NO_RESPONSE;
            } else {
                resp.data_len = ret;
            }
            break;
        }

        case IPC_CMD_CONFIG: {
            // 重新初始化 I2C (配置已由 Core0 更新到 g_state)
            i2c_pio_init();
            break;
        }

        default:
            resp.status = ERR_UNKNOWN_CMD;
            break;
    }

    fifo_ipc_send_resp(&resp);
}

/**
 * 处理 SPI 命令
 */
static void handle_spi_cmd(const ipc_cmd_t *cmd) {
    ipc_resp_t resp = {
        .status = ERR_OK,
        .data_len = 0
    };

    uint8_t cs_pin = cmd->param & 0xFF;

    switch (cmd->cmd) {
        case IPC_CMD_SPI_TRANSFER: {
            // 全双工传输
            spi_pio_cs_select(cs_pin);
            int ret = spi_pio_transfer(g_shared_tx_buf, g_shared_rx_buf, cmd->data_len);
            spi_pio_cs_deselect();

            if (ret < 0) {
                resp.status = ERR_TIMEOUT;
            } else {
                resp.data_len = cmd->data_len;
            }
            break;
        }

        case IPC_CMD_SPI_WRITE: {
            spi_pio_cs_select(cs_pin);
            int ret = spi_pio_transfer(g_shared_tx_buf, NULL, cmd->data_len);
            spi_pio_cs_deselect();

            if (ret < 0) {
                resp.status = ERR_TIMEOUT;
            }
            break;
        }

        case IPC_CMD_SPI_READ: {
            uint16_t len = cmd->param >> 8;
            spi_pio_cs_select(cs_pin);
            int ret = spi_pio_transfer(NULL, g_shared_rx_buf, len);
            spi_pio_cs_deselect();

            if (ret < 0) {
                resp.status = ERR_TIMEOUT;
            } else {
                resp.data_len = len;
            }
            break;
        }

        case IPC_CMD_CONFIG: {
            // 重新初始化 SPI (配置已由 Core0 更新到 g_state)
            spi_pio_init();
            break;
        }

        default:
            resp.status = ERR_UNKNOWN_CMD;
            break;
    }

    fifo_ipc_send_resp(&resp);
}

/**
 * 处理 UART 命令
 */
static void handle_uart_cmd(const ipc_cmd_t *cmd) {
    ipc_resp_t resp = {
        .status = ERR_OK,
        .data_len = 0
    };

    switch (cmd->cmd) {
        case IPC_CMD_UART_SEND: {
            int ret = uart_pio_send(g_shared_tx_buf, cmd->data_len);
            if (ret < 0) {
                resp.status = ERR_TIMEOUT;
            }
            break;
        }

        case IPC_CMD_UART_RECV: {
            uint32_t timeout = cmd->param;
            int ret = uart_pio_recv(g_shared_rx_buf, DATA_BUFFER_SIZE, timeout);
            if (ret < 0) {
                resp.status = ERR_TIMEOUT;
            } else {
                resp.data_len = ret;
            }
            break;
        }

        case IPC_CMD_UART_STREAM_TX: {
            int ret = uart_pio_send(g_shared_tx_buf, cmd->data_len);
            if (ret < 0) {
                resp.status = ERR_TIMEOUT;
            }
            break;
        }

        case IPC_CMD_UART_STREAM_RX: {
            int count = 0;
            while (count < DATA_BUFFER_SIZE) {
                int byte = uart_pio_recv_byte();
                if (byte < 0) break;
                g_shared_rx_buf[count++] = (uint8_t)byte;
            }
            resp.data_len = count;
            break;
        }

        default:
            resp.status = ERR_UNKNOWN_CMD;
            break;
    }

    fifo_ipc_send_resp(&resp);
}

/**
 * 处理 GPIO 命令
 */
static void handle_gpio_cmd(const ipc_cmd_t *cmd) {
    ipc_resp_t resp = {
        .status = ERR_OK,
        .data_len = 0
    };

    uint8_t pin = cmd->param & 0xFF;
    uint8_t value = (cmd->param >> 8) & 0xFF;

    switch (cmd->cmd) {
        case IPC_CMD_GPIO_WRITE:
            gpio_ctrl_write(pin, value != 0);
            break;

        case IPC_CMD_GPIO_READ:
            g_shared_rx_buf[0] = gpio_ctrl_read(pin) ? 1 : 0;
            resp.data_len = 1;
            break;

        default:
            resp.status = ERR_UNKNOWN_CMD;
            break;
    }

    fifo_ipc_send_resp(&resp);
}
