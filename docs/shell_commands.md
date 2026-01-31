# UartToX Shell 命令参考手册

## 命令格式规范

### 基本格式
```
<命令> <子命令> [参数...]
```

命令支持两种风格：
- 空格分隔: `i2c scan`
- 点号分隔: `i2c.scan`

### 响应格式
```
OK [数据]           # 成功
ERR <错误码> <描述>  # 失败
```

---

## 1. 系统命令

### help - 帮助信息
显示可用命令列表。
```shell
> help
Available commands:
  help       Show help information
  info       Show system information
  reset      Reset system
  bootsel    Enter BOOTSEL mode for flash
  mode       Set transfer mode
  i2c        I2C operations
  spi        SPI operations
  uart       UART operations
  gpio       GPIO operations

> help i2c    # 显示特定命令帮助
```

### info - 系统信息
显示系统版本和状态。
```shell
> info
UartToX v1.0.0
RP2040 @ 125 MHz
Protocols: I2C SPI UART GPIO
Mode: TEXT
```

### reset - 复位系统
```shell
> reset             # 软复位
> reset factory     # 恢复出厂设置
```

### bootsel - 进入烧录模式
进入 BOOTSEL 模式，设备会变成 USB 存储设备，可直接拖入 uf2 固件烧录。
无需手动按住 BOOTSEL 按钮重新插拔。
```shell
> bootsel
Entering BOOTSEL mode...
Device will appear as USB drive.
# 设备变成 RPI-RP2 U盘，拖入 uart_to_x.uf2 即可烧录
```

### mode - 传输模式
切换数据传输模式。
```shell
> mode              # 查看当前模式
Current mode: text

> mode text         # 文本模式 (默认)
> mode bin          # 二进制模式
> mode stream       # 流模式
```

---

## 2. I2C 命令

### i2c config - 配置 I2C
```shell
> i2c config                 # 查看当前配置
Port: 0, Speed: 400 kHz, Pullup: ON

> i2c config speed 100       # 设置速度 100kHz
> i2c config speed 400       # 设置速度 400kHz
> i2c config speed 1000      # 设置速度 1MHz
> i2c config port 0          # 使用 I2C0 (GP4/GP5)
> i2c config port 1          # 使用 I2C1 (GP6/GP7)
> i2c config pullup on       # 开启内部上拉
> i2c config pullup off      # 关闭内部上拉
```

### i2c scan - 扫描设备
扫描 I2C 总线上的所有设备。
```shell
> i2c scan
Scanning I2C bus...
Found 2 device(s):
  0x27
  0x68
```

### i2c detect - 检测单个地址
检测指定地址是否有设备响应。
```shell
> i2c detect 0x68
OK Device found at 0x68

> i2c detect 0x50
ERR E03 No device at 0x50
```

### i2c read - 读取数据
```shell
# 直接读取: i2c read <addr> <len>
> i2c read 0x68 6
OK 01 02 03 04 05 06

# 带寄存器读取: i2c read <addr> <reg> <len>
> i2c read 0x68 0x3B 6
OK 00 01 40 00 00 00
```

### i2c write - 写入数据
```shell
# 格式: i2c write <addr> <data...>
> i2c write 0x68 0x6B 0x00
OK

# 写入多字节
> i2c write 0x50 0x00 0x00 0x48 0x65 0x6C 0x6C 0x6F
OK
```

### i2c transfer - 读写组合
先写入后读取，适用于需要先发送寄存器地址再读取的场景。
```shell
# 格式: i2c transfer <addr> <write_data...> -r <read_len>
> i2c transfer 0x68 0x3B -r 14
OK 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D
```

### i2c dump - 寄存器转储
以表格形式显示设备的寄存器内容。
```shell
# 格式: i2c dump <addr> [start_reg] [len]
> i2c dump 0x68
     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
00: 68 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
10: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
...

> i2c dump 0x68 0x3B 16    # 从寄存器 0x3B 开始读取 16 字节
```

---

## 3. SPI 命令

### spi config - 配置 SPI
```shell
> spi config                 # 查看配置
Speed: 1000 kHz, Mode: 0, CS: GP17, Bits: 8, Order: MSB

> spi config speed 1000      # 1 MHz
> spi config speed 10000     # 10 MHz
> spi config speed 20000     # 20 MHz
> spi config mode 0          # CPOL=0, CPHA=0
> spi config mode 1          # CPOL=0, CPHA=1
> spi config mode 2          # CPOL=1, CPHA=0
> spi config mode 3          # CPOL=1, CPHA=1
> spi config cs 0            # 使用 CS0 (GP17)
> spi config cs 1            # 使用 CS1 (GP18)
> spi config cs 2            # 使用 CS2 (GP19)
```

### spi transfer - 传输数据
SPI 全双工传输，写入同时读取。
```shell
# 格式: spi transfer <data...>
> spi transfer 0x9F 0x00 0x00 0x00
OK EF 40 18 00
```

### spi write - 只写数据
只发送数据，不关心返回。
```shell
# 格式: spi write <data...>
> spi write 0x06              # 发送 Write Enable 命令
OK Sent 1 bytes
```

### spi read - 只读数据
只读取数据（发送 0xFF 作为时钟）。
```shell
# 格式: spi read <len>
> spi read 4
OK FF FF FF FF
```

---

## 4. UART 命令

### uart config - 配置 UART
```shell
> uart config                # 查看配置
Baudrate: 115200, Data: 8N1

> uart config baud 9600
> uart config baud 115200
> uart config baud 921600
> uart config baud 3000000   # 3Mbps
```

### uart send - 发送数据
```shell
# 发送十六进制
> uart send 0x48 0x65 0x6C 0x6C 0x6F
OK Sent 5 bytes

# 发送字符串
> uart send "Hello World\r\n"
OK Sent 13 bytes
```

### uart recv - 接收数据
```shell
# 格式: uart recv [timeout_ms]
> uart recv                  # 默认 1000ms 超时
OK 48 65 6C 6C 6F           # 收到 "Hello"

> uart recv 5000             # 5 秒超时
OK No data received
```

### uart bridge - 透传模式
进入双向透传模式，键盘输入直接发送到 UART TX，UART RX 接收的数据实时显示。
适合与其他设备进行交互式通信。
```shell
# 格式: uart bridge [--hex]
> uart bridge
--- UART Bridge Mode (Ctrl+] x3 to exit) ---
--- Baudrate: 115200, Data: 8N1 ---
Hello from device!          ← 从 UART 收到的数据
Hi there!                   ← 你输入的，直接发送到 UART
Response received           ← 从 UART 收到的数据
--- Exited bridge mode ---
>

# 十六进制显示模式
> uart bridge --hex
--- UART Bridge Mode (Ctrl+] x3 to exit) ---
--- HEX display mode ---
48 65 6C 6C 6F              ← 显示十六进制而非字符
```

**退出方式**: 连续按三次 `Ctrl+]`

### uart monitor - 监听模式
进入只读监听模式，只显示 UART RX 收到的数据，不发送任何数据。
适合数据抓包和日志监控。
```shell
# 格式: uart monitor [--hex]
> uart monitor
--- UART Monitor Mode (Ctrl+] x3 to exit) ---
--- Baudrate: 115200 ---
[设备持续输出的数据会实时显示在这里]
--- Exited monitor mode ---
>

# 十六进制显示模式
> uart monitor --hex
```

**退出方式**: 连续按三次 `Ctrl+]`

---

## 5. GPIO 命令

### gpio mode - 设置模式
```shell
# 格式: gpio mode <pin> <mode>
# mode: out, in, in_pullup, in_pulldown

> gpio mode 15 out           # 输出模式
OK

> gpio mode 15 in            # 输入模式 (浮空)
OK

> gpio mode 15 in_pullup     # 输入上拉
OK

> gpio mode 15 in_pulldown   # 输入下拉
OK
```

### gpio write - 写入电平
```shell
# 格式: gpio write <pin> <0|1|toggle>

> gpio write 15 1            # 高电平
OK

> gpio write 15 0            # 低电平
OK

> gpio write 15 toggle       # 翻转
OK
```

### gpio read - 读取电平
```shell
# 读取单个引脚
> gpio read 15
OK 1

# 读取所有通用 GPIO (GP11-GP14)
> gpio read all
OK GP11:1 GP12:0 GP13:1 GP14:0
```

---

## 6. 错误码

| 代码 | 说明 |
|------|------|
| E01 | 未知命令 |
| E02 | 参数错误 / 无效子命令 |
| E03 | 设备无响应 / 操作失败 |
| E05 | 超时 |

---

## 7. 引脚分配

| 功能 | 引脚 | 说明 |
|------|------|------|
| I2C0 SDA | GP4 | I2C 数据线 |
| I2C0 SCL | GP5 | I2C 时钟线 |
| I2C1 SDA | GP6 | 备用 I2C 数据线 |
| I2C1 SCL | GP7 | 备用 I2C 时钟线 |
| SPI MISO | GP12 | SPI 主入从出 |
| SPI MOSI | GP11 | SPI 主出从入 |
| SPI SCK | GP10 | SPI 时钟 |
| SPI CS0 | GP17 | SPI 片选 0 |
| SPI CS1 | GP18 | SPI 片选 1 |
| SPI CS2 | GP19 | SPI 片选 2 |
| UART TX | GP0 | UART 发送 |
| UART RX | GP1 | UART 接收 |
| GPIO | GP11-GP14 | 通用 GPIO |
| LED | GP25 | 状态指示灯 |

---

## 8. 快速入门示例

### 示例1：扫描 I2C 设备
```shell
> i2c config speed 400
OK I2C speed set to 400 kHz

> i2c scan
Scanning I2C bus...
Found 1 device(s):
  0x68
```

### 示例2：读取 MPU6050 加速度计
```shell
> i2c write 0x68 0x6B 0x00      # 唤醒 MPU6050
OK

> i2c transfer 0x68 0x3B -r 6   # 读取加速度
OK 00 00 40 00 00 00
```

### 示例3：读取 SPI Flash ID
```shell
> spi config speed 1000
OK SPI speed set to 1000 kHz

> spi transfer 0x9F 0x00 0x00 0x00
OK EF 40 18 00
# Manufacturer: Winbond (0xEF), Device: W25Q128 (0x4018)
```

### 示例4：GPIO 控制
```shell
> gpio mode 15 out
OK

> gpio write 15 1
OK

> gpio read 15
OK 1

> gpio write 15 toggle
OK
```
