# UartToX

**English** | [中文](README.md)

A multi-protocol converter tool based on RP2040 - Control I2C/SPI/UART/GPIO via serial shell

## Features

- **Shell Interface**: Command-line interface via USB CDC
- **Multi-Protocol Support**: I2C, SPI, UART, GPIO
- **High-Speed Transfer**: USB high-speed communication + binary transfer mode
- **Dual-Core Architecture**: Core0 handles commands, Core1 handles protocols
- **Status Indicator**: LED blinks when waiting for connection, solid when connected

## Hardware Requirements

- Raspberry Pi Pico or other RP2040-based development board
- USB data cable

## Pin Assignment

| Function | GPIO | Function | GPIO |
|----------|------|----------|------|
| I2C0 SDA | GP4 | SPI MISO | GP12 |
| I2C0 SCL | GP5 | SPI MOSI | GP11 |
| I2C1 SDA | GP6 | SPI SCK | GP10 |
| I2C1 SCL | GP7 | SPI CS0 | GP17 |
| UART TX | GP0 | GPIO 0-3 | GP11-14 |
| UART RX | GP1 | LED | GP25 |

## Quick Start

```shell
# Open serial terminal after USB connection (115200 8N1)
> help              # Show all commands
> info              # Show system information
> i2c scan          # Scan I2C bus
> i2c read 0x38 6   # Read 6 bytes from address 0x38
```

## Documentation

- [System Architecture](docs/architecture.md)
- [Shell Command Reference](docs/shell_commands.md)
- [Test Report](docs/test_report.md)

## Building

You need to use the **Pico - Visual Studio Code** shortcut to open the terminal with proper environment variables.

```bash
# First time build
mkdir build && cd build
cmake -G Ninja ..
ninja

# Output: build/uart_to_x.uf2
# Hold BOOTSEL button while connecting Pico, then drag the UF2 file to flash
```

## Usage Example

### Reading AHT10 Temperature & Humidity Sensor

```shell
> i2c scan                           # Scan devices
Found 1 device(s):
  0x38

> i2c write 0x38 0xE1 0x08 0x00      # Initialize
OK

> i2c write 0x38 0xAC 0x33 0x00      # Trigger measurement
OK

> i2c read 0x38 6                    # Read data
OK 18 A0 8D F5 C2 84
# Parsed: Temperature 22.0°C, Humidity 62.7%
```

## Supported Commands

| Command | Description |
|---------|-------------|
| `help` | Show help information |
| `info` | Show system information |
| `reset` | Reset system |
| `mode` | Set transfer mode (text/bin/stream) |
| `i2c` | I2C operations (config/scan/detect/read/write/transfer) |
| `spi` | SPI operations (config/transfer) |
| `uart` | UART operations (config/send) |
| `gpio` | GPIO operations (mode/write/read) |

## Response Format

```
OK [data]           # Success
ERR <code> <msg>    # Failure
```

| Error Code | Description |
|------------|-------------|
| E01 | Unknown command |
| E02 | Invalid parameter / Unknown subcommand |
| E03 | No response / Operation failed |
| E05 | Timeout |

## License

MIT
