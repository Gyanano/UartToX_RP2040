/**
 * 核间通信 (IPC) - 使用 RP2040 硬件 FIFO
 */

#include "../include/uart_to_x.h"
#include "pico/multicore.h"

#define IPC_TIMEOUT_MS  1000

void fifo_ipc_init(void) {
    // RP2040 FIFO 在 multicore_launch_core1 时自动初始化
    // 清空 FIFO
    while (multicore_fifo_rvalid()) {
        multicore_fifo_pop_blocking();
    }
}

/**
 * 发送命令到 Core1
 */
bool fifo_ipc_send_cmd(const ipc_cmd_t *cmd) {
    // 打包命令为 32 位值
    // [31:24] cmd, [23:16] protocol, [15:0] data_len
    uint32_t word0 = ((uint32_t)cmd->cmd << 24) |
                     ((uint32_t)cmd->protocol << 16) |
                     cmd->data_len;
    uint32_t word1 = cmd->param;

    // 推送到 FIFO
    if (!multicore_fifo_wready()) {
        return false;
    }
    multicore_fifo_push_blocking(word0);
    multicore_fifo_push_blocking(word1);

    return true;
}

/**
 * 从 FIFO 接收命令 (Core1 调用)
 */
bool fifo_ipc_recv_cmd(ipc_cmd_t *cmd) {
    if (!multicore_fifo_rvalid()) {
        return false;
    }

    uint32_t word0 = multicore_fifo_pop_blocking();
    uint32_t word1 = multicore_fifo_pop_blocking();

    cmd->cmd = (word0 >> 24) & 0xFF;
    cmd->protocol = (word0 >> 16) & 0xFF;
    cmd->data_len = word0 & 0xFFFF;
    cmd->param = word1;

    return true;
}

/**
 * 发送响应到 Core0 (Core1 调用)
 */
bool fifo_ipc_send_resp(const ipc_resp_t *resp) {
    // 打包响应
    uint32_t word = ((uint32_t)resp->status << 16) | resp->data_len;

    if (!multicore_fifo_wready()) {
        return false;
    }
    multicore_fifo_push_blocking(word);
    return true;
}

/**
 * 接收响应 (Core0 调用)
 */
bool fifo_ipc_recv_resp(ipc_resp_t *resp) {
    // 等待响应，带超时
    uint32_t start = time_us_32();
    while (!multicore_fifo_rvalid()) {
        if ((time_us_32() - start) > (IPC_TIMEOUT_MS * 1000)) {
            return false;
        }
        tight_loop_contents();
    }

    uint32_t word = multicore_fifo_pop_blocking();
    resp->status = (word >> 16) & 0xFF;
    resp->data_len = word & 0xFFFF;

    return true;
}
