# UartToX

[English](README_EN.md) | **中文**

基于 RP2040 的多协议转换工具 - 通过串口 Shell 控制 I2C/SPI/UART/GPIO

## 功能特性

- **Shell 交互**: 通过 USB CDC 提供命令行接口
- **多协议支持**: I2C, SPI, UART, GPIO
- **高速传输**: USB 高速通信 + 二进制传输模式
- **双核架构**: Core0 处理命令，Core1 处理协议
- **状态指示**: LED 闪烁表示等待连接，常亮表示已连接

## 硬件要求

- Raspberry Pi Pico 或其他 RP2040 开发板
- USB 数据线

## 引脚分配

| 功能 | GPIO | 功能 | GPIO |
|------|------|------|------|
| I2C0 SDA | GP4 | SPI MISO | GP12 |
| I2C0 SCL | GP5 | SPI MOSI | GP11 |
| I2C1 SDA | GP6 | SPI SCK | GP10 |
| I2C1 SCL | GP7 | SPI CS0 | GP17 |
| UART TX | GP0 | GPIO 0-3 | GP11-14 |
| UART RX | GP1 | LED | GP25 |

## 快速开始

```shell
# 连接 USB 后打开串口终端 (115200 8N1)
> help              # 查看所有命令
> info              # 查看系统信息
> i2c scan          # 扫描 I2C 设备
> i2c read 0x38 6   # 从地址 0x38 读取 6 字节
```

## 文档

- [系统架构](docs/architecture.md)
- [Shell 命令参考](docs/shell_commands.md)
- [测试报告](docs/test_report.md)

## 构建

需要使用 **Pico - Visual Studio Code** 快捷方式打开终端以加载正确的环境变量。

```bash
# 首次构建
mkdir build && cd build
cmake -G Ninja ..
ninja

# 输出文件: build/uart_to_x.uf2
# 按住 BOOTSEL 按钮连接 Pico，拖入 UF2 文件即可烧录
```

## 使用示例

### 读取 AHT10 温湿度传感器

```shell
> i2c scan                           # 扫描设备
Found 1 device(s):
  0x38

> i2c write 0x38 0xE1 0x08 0x00      # 初始化
OK

> i2c write 0x38 0xAC 0x33 0x00      # 触发测量
OK

> i2c read 0x38 6                    # 读取数据
OK 18 A0 8D F5 C2 84
# 解析: 温度 22.0°C, 湿度 62.7%
```

## License

MIT
