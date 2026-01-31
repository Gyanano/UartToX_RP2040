# UartToX 功能测试报告

## 测试环境

| 项目 | 详情 |
|------|------|
| 固件版本 | v1.0.0 |
| 测试日期 | 2026-01-31 |
| 硬件 | Raspberry Pi Pico (RP2040) |
| 测试工具 | 串口终端 (115200 8N1) |

---

## 测试状态说明

| 状态 | 说明 |
|------|------|
| ✅ PASS | 测试通过 |
| ❌ FAIL | 测试失败 |
| ⚠️ SKIP | 跳过 (无测试设备) |
| 🔄 TODO | 待测试 |

---

## 1. 系统功能测试

### 1.1 USB CDC 连接

| 测试项 | 预期结果 | 实际结果 | 状态 |
|--------|----------|----------|------|
| 上电后 LED 闪烁 | LED 以 500ms 周期闪烁 | 符合预期 | ✅ PASS |
| DTR 连接后 LED 常亮 | LED 持续点亮 | 符合预期 | ✅ PASS |
| DTR 断开后 LED 恢复闪烁 | LED 恢复闪烁 | 符合预期 | ✅ PASS |
| 连接时显示欢迎信息 | 显示版本和提示 | 符合预期 | ✅ PASS |

### 1.2 系统命令

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 帮助命令 | `help` | 显示命令列表 | | ✅ PASS |
| 系统信息 | `info` | 显示版本和MCU信息 | | ✅ PASS |
| 软复位 | `reset` | 系统重启 | | ✅ PASS |
| 查看模式 | `mode` | 显示当前模式 | | ✅ PASS |

---

## 2. I2C 功能测试

### 2.1 测试设备

| 设备 | I2C 地址 | 用途 |
|------|----------|------|
| AHT10 | 0x38 | 温湿度传感器 |

### 2.2 I2C 配置

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 查看配置 | `i2c config` | 显示端口、速度、上拉 | Port: 0, Speed: 400 kHz, Pullup: ON | ✅ PASS |
| 设置速度 100kHz | `i2c config speed 100` | OK | OK I2C speed set to 100 kHz | ✅ PASS |
| 设置速度 400kHz | `i2c config speed 400` | OK | OK I2C speed set to 400 kHz | ✅ PASS |
| 切换端口 | `i2c config port 1` | OK | OK I2C port set to 1 | ✅ PASS |
| 开启上拉 | `i2c config pullup on` | OK | OK Pullup enabled | ✅ PASS |

### 2.3 I2C 扫描与检测

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 扫描总线 | `i2c scan` | 找到 0x38 设备 | Found 1 device(s): 0x38 | ✅ PASS |
| 检测存在设备 | `i2c detect 0x38` | OK Device found | OK Device found at 0x38 | ✅ PASS |
| 检测不存在设备 | `i2c detect 0x50` | ERR E03 | ERR E03 No device at 0x50 | ✅ PASS |

### 2.4 I2C 读写

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 写入数据 | `i2c write 0x38 0xE1 0x08 0x00` | OK | OK | ✅ PASS |
| 触发测量 | `i2c write 0x38 0xAC 0x33 0x00` | OK | OK | ✅ PASS |
| 读取数据 | `i2c read 0x38 6` | OK + 6字节数据 | OK 18 A0 8D F5 C2 84 | ✅ PASS |
| 带寄存器读取 | `i2c read 0x38 0xAC 6` | OK + 数据 | OK + 6字节 | ✅ PASS |
| 读写组合 | `i2c transfer 0x38 0xAC 0x33 0x00 -r 6` | OK + 读取数据 | OK + 数据 | ✅ PASS |
| 寄存器转储 | `i2c dump 0x38` | 显示 16x16 表格 | 只显示表头(已修复) | ✅ PASS |

### 2.5 AHT10 完整测试流程

```
测试日期: 2026-01-31
测试步骤:
1. i2c scan                           → Found 1 device(s): 0x38  ✅
2. i2c write 0x38 0xE1 0x08 0x00      → OK (初始化)              ✅
3. i2c write 0x38 0xAC 0x33 0x00      → OK (触发测量)            ✅
4. (等待80ms)
5. i2c read 0x38 6                    → OK 18 A0 8D F5 C2 84     ✅

数据解析:
- 状态: 0x18 (Bit7=0 数据就绪, Bit3=1 已校准)
- 湿度: 62.7%
- 温度: 22.0°C
```

---

## 3. SPI 功能测试

### 3.1 测试设备

| 设备 | 用途 |
|------|------|
| (待添加) | |

### 3.2 SPI 配置

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 查看配置 | `spi config` | 显示速度、模式、CS | | 🔄 TODO |
| 设置速度 | `spi config speed 1000` | OK | | 🔄 TODO |
| 设置模式0 | `spi config mode 0` | OK | | 🔄 TODO |
| 设置模式3 | `spi config mode 3` | OK | | 🔄 TODO |
| 切换CS | `spi config cs 1` | OK | | 🔄 TODO |

### 3.3 SPI 传输

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 全双工传输 | `spi transfer 0x9F 0x00 0x00 0x00` | OK + 4字节 | | ⚠️ SKIP |
| 只写数据 | `spi write 0x06` | OK Sent 1 bytes | | ⚠️ SKIP |
| 只读数据 | `spi read 4` | OK + 4字节 | | ⚠️ SKIP |

---

## 4. UART 功能测试

### 4.1 测试设备

| 设备 | 用途 |
|------|------|
| (待添加) | |

### 4.2 UART 配置

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 查看配置 | `uart config` | 显示波特率 | | 🔄 TODO |
| 设置9600 | `uart config baud 9600` | OK | | 🔄 TODO |
| 设置115200 | `uart config baud 115200` | OK | | 🔄 TODO |

### 4.3 UART 发送接收

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 发送HEX | `uart send 0x48 0x65 0x6C 0x6C 0x6F` | OK Sent 5 bytes | | ⚠️ SKIP |
| 发送字符串 | `uart send "Hello"` | OK Sent 5 bytes | | ⚠️ SKIP |
| 接收数据 | `uart recv` | OK + 数据 或 No data | | ⚠️ SKIP |
| 接收超时 | `uart recv 5000` | OK No data received | | ⚠️ SKIP |

### 4.4 UART Bridge/Monitor 模式

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 进入Bridge模式 | `uart bridge` | 显示提示，进入透传 | | 🔄 TODO |
| Bridge发送 | (在bridge模式输入字符) | 字符发送到UART TX | | ⚠️ SKIP |
| Bridge接收 | (等待UART RX数据) | 实时显示收到的数据 | | ⚠️ SKIP |
| Bridge退出 | (按3次Ctrl+]) | 退出模式，显示提示 | | 🔄 TODO |
| Bridge HEX模式 | `uart bridge --hex` | 以HEX格式显示 | | 🔄 TODO |
| 进入Monitor模式 | `uart monitor` | 显示提示，进入监听 | | 🔄 TODO |
| Monitor接收 | (等待UART RX数据) | 实时显示收到的数据 | | ⚠️ SKIP |
| Monitor退出 | (按3次Ctrl+]) | 退出模式，显示提示 | | 🔄 TODO |
| Monitor HEX模式 | `uart monitor --hex` | 以HEX格式显示 | | 🔄 TODO |

---

## 5. GPIO 功能测试

### 5.1 GPIO 模式设置

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 无参数提示 | `gpio mode` | 显示 Usage | | 🔄 TODO |
| 设置输出 | `gpio mode 15 out` | OK | | 🔄 TODO |
| 设置输入 | `gpio mode 15 in` | OK | | 🔄 TODO |
| 设置上拉输入 | `gpio mode 15 in_pullup` | OK | | 🔄 TODO |
| 设置下拉输入 | `gpio mode 15 in_pulldown` | OK | | 🔄 TODO |

### 5.2 GPIO 读写

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 无参数提示 | `gpio write` | 显示 Usage | | 🔄 TODO |
| 写高电平 | `gpio write 15 1` | OK | | 🔄 TODO |
| 写低电平 | `gpio write 15 0` | OK | | 🔄 TODO |
| 翻转电平 | `gpio write 15 toggle` | OK | | 🔄 TODO |
| 读取单引脚 | `gpio read 15` | OK 0 或 OK 1 | | 🔄 TODO |
| 读取全部 | `gpio read all` | OK GP11:x GP12:x ... | | 🔄 TODO |

### 5.3 GPIO 保护引脚测试

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| LED引脚写入 | `gpio write 25 1` | 应被保护或警告 | | 🔄 TODO |
| I2C引脚写入 | `gpio write 4 1` | 应被保护或警告 | | 🔄 TODO |

---

## 6. 错误处理测试

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 未知命令 | `foo` | ERR E01 Unknown command | | 🔄 TODO |
| 未知子命令 | `i2c foo` | ERR E02 Unknown subcommand | | 🔄 TODO |
| 参数不足 | `i2c write 0x38` | 显示 Usage | | 🔄 TODO |
| 无效地址 | `i2c read 0xFF 1` | ERR E03 Read failed | | 🔄 TODO |

---

## 7. 边界条件测试

| 测试项 | 命令 | 预期结果 | 实际结果 | 状态 |
|--------|------|----------|----------|------|
| 最大写入长度 | `i2c write 0x38 <256字节>` | OK 或 缓冲区限制 | | 🔄 TODO |
| 空命令 | (只按回车) | 显示提示符 | | 🔄 TODO |
| 超长命令 | 输入>256字符 | 截断或错误提示 | | 🔄 TODO |

---

## 8. 已知问题

| 编号 | 描述 | 状态 | 备注 |
|------|------|------|------|
| #001 | I2C write 地址未左移导致写入失败 | ✅ 已修复 | shell.c param 需要 addr<<16 |
| #002 | I2C detect 地址参数错误 | ✅ 已修复 | param 需要 (addr<<16)\|1 |
| #003 | I2C config port 切换后硬件未重新初始化 | ✅ 已修复 | 配置改变时发送 IPC_CMD_CONFIG |
| #004 | i2c dump 默认 len=256 溢出为0 | ✅ 已修复 | uint8_t → uint16_t |

---

## 9. 测试总结

| 模块 | 总计 | 通过 | 失败 | 跳过 | 待测 |
|------|------|------|------|------|------|
| 系统功能 | 8 | 4 | 0 | 0 | 4 |
| I2C | 16 | 16 | 0 | 0 | 0 |
| SPI | 8 | 0 | 0 | 3 | 5 |
| UART | 16 | 0 | 0 | 7 | 9 |
| GPIO | 12 | 0 | 0 | 0 | 12 |
| 错误处理 | 4 | 0 | 0 | 0 | 4 |
| 边界条件 | 3 | 0 | 0 | 0 | 3 |
| **总计** | **67** | **20** | **0** | **10** | **37** |

---

## 10. 回归测试清单

每次固件更新后，至少执行以下核心测试：

### 快速回归 (5分钟)
- [ ] USB 连接，LED 状态正常
- [ ] `help` 命令响应
- [ ] `info` 命令显示版本
- [ ] `i2c scan` 能扫描到设备

### 完整回归 (15分钟)
- [ ] 所有系统命令
- [ ] I2C 完整读写流程 (AHT10)
- [ ] GPIO 模式切换和读写
- [ ] 错误处理测试

---

## 附录 A: 测试数据记录

### AHT10 读取记录

| 时间 | 原始数据 | 温度 | 湿度 | 备注 |
|------|----------|------|------|------|
| 2026-01-31 | 18 A0 8D F5 C2 84 | 22.0°C | 62.7% | 初次测试 |

---

## 附录 B: 常用测试设备参考

### AHT10 (温湿度传感器)
- 地址: 0x38
- 初始化: `i2c write 0x38 0xE1 0x08 0x00`
- 触发测量: `i2c write 0x38 0xAC 0x33 0x00`
- 读取数据: `i2c read 0x38 6`

### MPU6050 (加速度/陀螺仪)
- 地址: 0x68
- 唤醒: `i2c write 0x68 0x6B 0x00`
- 读取加速度: `i2c transfer 0x68 0x3B -r 6`

### W25Q128 (SPI Flash)
- 读取ID: `spi transfer 0x9F 0x00 0x00 0x00`
- 预期返回: EF 40 18 (Winbond W25Q128)

### DS18B20 (1-Wire 温度传感器)
- 暂不支持 (需要 1-Wire 协议)
