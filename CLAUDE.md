# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This project uses the Pico SDK with CMake and Ninja. **Must use "Pico - Visual Studio Code" shortcut** on Windows to get proper environment variables.

```bash
# First time setup
mkdir build && cd build
cmake -G Ninja ..

# Build
ninja

# Clean rebuild
del CMakeCache.txt && cmake -G Ninja .. && ninja
```

Output: `build/uart_to_x.uf2` - drag to RP2040 in BOOTSEL mode.

## Architecture Overview

**Dual-Core Design:**
- **Core0**: USB CDC communication + Shell command parsing (runs in main loop)
- **Core1**: Protocol execution (I2C, SPI, UART, GPIO)

**Inter-Core Communication:**
- Uses RP2040 hardware FIFO (`fifo_ipc.c`)
- Shared memory buffers `g_shared_tx_buf` / `g_shared_rx_buf` protected by `g_shared_buf_mutex`
- Command flow: Shell parses → `ipc_cmd_t` sent to Core1 → Core1 executes → `ipc_resp_t` returned

**Key Files:**
- `src/main.c` - Entry point, Core1 launch, main loop with USB connection state tracking
- `src/include/uart_to_x.h` - All type definitions, pin assignments, function declarations
- `src/core0/shell.c` - Command parser with subcommand support (`i2c scan` or `i2c.scan`)
- `src/core1/protocol_manager.c` - Dispatches IPC commands to protocol drivers

**Protocol Implementation:**
- I2C/SPI use **hardware peripherals** (not PIO - PIO had instruction limit issues)
- UART uses PIO for flexible baud rates
- Configuration stored in `g_state` global struct

## Pin Assignments (defined in uart_to_x.h)

| Function | GPIO | Function | GPIO |
|----------|------|----------|------|
| I2C0 SDA | GP4  | SPI MISO | GP12 |
| I2C0 SCL | GP5  | SPI MOSI | GP11 |
| I2C1 SDA | GP6  | SPI SCK  | GP10 |
| I2C1 SCL | GP7  | SPI CS0  | GP17 |
| UART TX  | GP0  | GPIO 0-3 | GP11-14 |
| UART RX  | GP1  | LED      | GP25 |

## Shell Command Pattern

Commands use `cmd_handler_t` function signature. Each command checks `argc` and provides usage hints:
```c
if (argc < required) {
    printf("Usage: ...\r\n");
    return;
}
```

Response format: `OK [data]` or `ERR E<code> <message>`

## Documentation

- [docs/shell_commands.md](docs/shell_commands.md) - Complete command reference
- [docs/architecture.md](docs/architecture.md) - System design details
