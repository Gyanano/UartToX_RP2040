# UartToX

基于 RP2040 的多协议转换工具 - 通过串口 Shell 控制 I2C/SPI/UART/1-Wire

## 功能特性

- **Shell 交互**: 通过 USB CDC 提供命令行接口
- **多协议支持**: I2C, SPI, UART, 1-Wire, GPIO
- **高速传输**: USB 高速通信 + 二进制传输模式
- **双核架构**: Core0 处理命令，Core1 处理协议
- **PIO 实现**: 精确时序，高性能

## 硬件要求

- Raspberry Pi Pico 或其他 RP2040 开发板
- USB 数据线

## 引脚分配

| 功能 | GPIO |
|------|------|
| I2C0 SDA | GP4 |
| I2C0 SCL | GP5 |
| SPI MISO | GP16 |
| SPI MOSI | GP17 |
| SPI SCK | GP18 |
| SPI CS0 | GP19 |
| UART TX | GP8 |
| UART RX | GP9 |
| 1-Wire | GP10 |

## 快速开始

```shell
# 连接 USB 后打开串口终端
> help
> i2c.scan
> spi.transfer 0x9F 0x00 0x00
```

## 文档

- [系统架构](docs/architecture.md)
- [Shell 命令参考](docs/shell_commands.md)

## 构建

```bash
mkdir build && cd build
cmake ..
make -j4
```

## License

MIT
