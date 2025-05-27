# SPI 配置说明

## 概述

本项目配置了三路 SPI 接口，用于连接不同类型的外设：

- **SPI1**: 传感器接口 (ICM42688, MS5611, ICM20602)
- **SPI2**: 存储器接口 (FM25V05)
- **SPI3**: 显示屏接口 (ST7735, ST7789)

## 硬件连接

### SPI1 - 传感器接口

- **SCLK**: PA5 (SPI1_SCK)
- **MISO**: PA6 (SPI1_MISO)
- **MOSI**: PA7 (SPI1_MOSI)
- **CS 引脚**:
  - ICM42688: PA4
  - MS5611: PA15
  - ICM20602: PB0

### SPI2 - 存储器接口

- **SCLK**: PB13 (SPI2_SCK)
- **MISO**: PB14 (SPI2_MISO)
- **MOSI**: PB15 (SPI2_MOSI)
- **CS 引脚**:
  - FM25V05: PB9

### SPI3 - 显示屏接口

- **SCLK**: PB3 (SPI3_SCK)
- **MISO**: PB4 (SPI3_MISO)
- **MOSI**: PB5 (SPI3_MOSI)
- **CS 引脚**:
  - ST7735: PB11
  - ST7789: PB12

## 软件配置

### 1. SPI 总线配置

在 `drivers/board.h` 中启用 SPI：

```c
#define BSP_USING_SPI1
#define BSP_USING_SPI2
#define BSP_USING_SPI3
```

### 2. GPIO 配置

在 `drivers/board.c` 的 `HAL_SPI_MspInit` 函数中配置 GPIO：

- SPI1: PA5/PA6/PA7 (AF5)
- SPI2: PB13/PB14/PB15 (AF5)
- SPI3: PB3/PB4/PB5 (AF6)

### 3. 设备配置

每个 SPI 设备都有独立的配置：

#### SPI1 设备 (传感器)

- 数据宽度: 8 位
- 模式: SPI_MODE_3 (CPOL=1, CPHA=1)
- 频率: 10MHz
- 字节序: MSB 优先

#### SPI2 设备 (存储器)

- 数据宽度: 8 位
- 模式: SPI_MODE_0 (CPOL=0, CPHA=0)
- 频率: 20MHz
- 字节序: MSB 优先

#### SPI3 设备 (显示屏)

- 数据宽度: 8 位
- 模式: SPI_MODE_0 (CPOL=0, CPHA=0)
- 频率: 36MHz
- 字节序: MSB 优先

## 使用方法

### 1. 初始化

SPI 设备会在系统启动时自动初始化（通过 `INIT_DEVICE_EXPORT`）。

### 2. 访问设备

通过全局句柄 `spi_handle` 访问各个设备：

```c
#include "app_spi_drv.h"

// 访问ICM42688传感器
struct rt_spi_device *icm42688 = spi_handle.spi1_dev.icm42688;

// 访问FM25V05存储器
struct rt_spi_device *fm25v05 = spi_handle.spi2_dev.fm25v05;

// 访问ST7735显示屏
struct rt_spi_device *st7735 = spi_handle.spi3_dev.st7735;
```

### 3. 数据传输

使用提供的操作函数：

```c
// 基本传输
rt_uint8_t tx_data = 0x75;  // WHO_AM_I寄存器
rt_uint8_t rx_data = 0x00;
spi_handle.transfer(icm42688, &tx_data, &rx_data, 1);

// 先写后读
rt_uint8_t cmd = 0x75;
rt_uint8_t data = 0x00;
spi_handle.write_then_read(icm42688, &cmd, 1, &data, 1);

// 连续写入
rt_uint8_t cmd1 = 0x20;
rt_uint8_t cmd2 = 0x01;
spi_handle.write_then_write(icm42688, &cmd1, 1, &cmd2, 1);
```

### 4. 测试命令

系统提供了 SPI 测试命令：

```bash
# 测试ICM42688传感器
spi_test icm42688

# 测试FM25V05存储器
spi_test fm25v05

# 测试ST7735显示屏
spi_test st7735
```

## 注意事项

1. **时钟频率**: 根据外设规格选择合适的 SPI 时钟频率
2. **SPI 模式**: 不同设备可能需要不同的 CPOL/CPHA 设置
3. **CS 控制**: CS 引脚由 RT-Thread SPI 框架自动控制
4. **中断处理**: 支持 DMA 传输完成中断
5. **线程安全**: 使用信号量保证多线程访问安全

## 故障排除

1. **设备未找到**: 检查设备名称和初始化顺序
2. **通信失败**: 检查 GPIO 配置和 SPI 模式设置
3. **数据错误**: 检查时钟频率和信号完整性
4. **CS 问题**: 确认 CS 引脚配置正确

## 扩展

如需添加新的 SPI 设备：

1. 在 `app_spi_drv.h` 中添加设备结构
2. 在 `app_spi_drv.c` 中添加设备初始化代码
3. 配置相应的 CS 引脚
4. 根据设备规格设置 SPI 参数
