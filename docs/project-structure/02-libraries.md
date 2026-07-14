# libraries/ - 逐飞 CYT4BB 开源库

## 概述

`libraries/` 是逐飞科技为 CYT4BB 芯片提供的开源驱动库，版本 V3.6.3，基于 GPL-3.0 许可证。包含芯片 SDK、外设驱动、设备驱动和公共组件。

## 目录结构

```text
libraries/
├─ doc/                          # 许可证和版本说明
├─ sdk/                          # 官方芯片 SDK
├─ zf_common/                    # 公共基础模块（时钟、调试、FIFO、字体、中断）
├─ zf_components/                # 逐飞助手组件
├─ zf_device/                    # 外设设备驱动（屏幕、摄像头、IMU、WiFi 等）
└─ zf_driver/                    # 底层外设驱动（GPIO、UART、SPI、PIT、PWM 等）
```

## doc/ - 文档与许可证

| 文件 | 作用 |
|------|------|
| `GPL3_permission_statement.txt` | GPL-3.0 许可证声明（逐飞 CYT4BB 开源库） |
| `version.txt` | 版本更新日志，从 V3.0.0 到 V3.6.3，记录每个版本的修复和新增功能 |

## sdk/ - 官方芯片 SDK

| 目录 | 作用 |
|------|------|
| `sdk/common/` | 通用 SDK 头文件和源文件（CMSIS、ARM Math 等） |
| `sdk/tviibh4m/` | CYT4BB 芯片特定的头文件和源文件（寄存器定义、设备头文件） |

## zf_common/ - 公共基础模块

| 文件 | 作用 |
|------|------|
| `zf_common_clock.c/h` | 系统时钟配置（250MHz 等） |
| `zf_common_debug.c/h` | 调试串口初始化和输出 |
| `zf_common_fifo.c/h` | FIFO 缓冲区实现 |
| `zf_common_font.c/h` | 字体数据（用于屏幕显示） |
| `zf_common_function.c/h` | 通用工具函数 |
| `zf_common_headfile.h` | **公共总头文件**：聚合所有逐飞 SDK/驱动/设备头文件，用户代码只需包含此文件 |
| `zf_common_interrupt.c/h` | 中断管理（全局中断使能/禁止） |
| `zf_common_typedef.h` | 通用类型定义 |

## zf_components/ - 逐飞助手组件

| 文件 | 作用 |
|------|------|
| `seekfree_assistant.c/h` | 逐飞助手通信协议 |
| `seekfree_assistant_interface.c/h` | 逐飞助手数据接口 |

## zf_device/ - 外设设备驱动

按设备类型分类：

### 显示设备
| 文件 | 作用 |
|------|------|
| `zf_device_ips114.c/h` | IPS114 1.14 寸 TFT 屏驱动 |
| `zf_device_ips200.c/h` | IPS200 2 寸 TFT 屏驱动（当前项目使用） |
| `zf_device_ips200pro.c/h` | IPS200Pro 2 寸屏增强版驱动 |
| `zf_device_oled.c/h` | OLED 显示屏驱动 |
| `zf_device_tft180.c/h` | TFT180 1.8 寸 TFT 屏驱动 |

### 摄像头
| 文件 | 作用 |
|------|------|
| `zf_device_mt9v03x.c/h` | MT9V03X 摄像头驱动（当前项目使用，灰度图，支持多种分辨率） |

### IMU（惯性测量单元）
| 文件 | 作用 |
|------|------|
| `zf_device_imu660ra.c/h` | IMU660RA 6 轴 IMU 驱动 |
| `zf_device_imu660rb.c/h` | IMU660RB 6 轴 IMU 驱动 |
| `zf_device_imu963ra.c/h` | IMU963RA 9 轴 IMU 驱动（含磁力计） |
| `zf_device_icm20602.c/h` | ICM20602 6 轴 IMU 驱动 |

### 导航与定位
| 文件 | 作用 |
|------|------|
| `zf_device_gnss.c/h` | GNSS（GPS/北斗）模块驱动 |

### 无线通信
| 文件 | 作用 |
|------|------|
| `zf_device_wifi_spi.c/h` | WiFi-SPI 模块驱动（当前项目用于图传） |
| `zf_device_wifi_uart.c/h` | WiFi-UART 模块驱动 |
| `zf_device_wireless_uart.c/h` | 无线串口模块驱动 |
| `zf_device_ble6a20.c/h` | BLE6A20 蓝牙模块驱动 |

### 其他外设
| 文件 | 作用 |
|------|------|
| `zf_device_dl1a.c/h` | DL1A 激光测距传感器驱动 |
| `zf_device_dl1b.c/h` | DL1B 激光测距传感器驱动 |
| `zf_device_key.c/h` | 按键驱动 |
| `zf_device_menc15a.c/h` | MENC15A 磁编码器驱动 |
| `zf_device_tsl1401.c/h` | TSL1401 CCD 线性传感器驱动 |
| `zf_device_uart_receiver.c/h` | SBUS 遥控器接收机驱动 |
| `zf_device_type.c/h` | 设备类型定义 |

## zf_driver/ - 底层外设驱动

| 文件 | 作用 |
|------|------|
| `zf_driver_adc.c/h` | ADC（模数转换）驱动 |
| `zf_driver_delay.c/h` | 延时函数（微秒/毫秒） |
| `zf_driver_dma.c/h` | DMA（直接内存访问）驱动 |
| `zf_driver_encoder.c/h` | 正交编码器驱动 |
| `zf_driver_exti.c/h` | 外部中断驱动 |
| `zf_driver_flash.c/h` | Flash 存储器读写驱动 |
| `zf_driver_gpio.c/h` | GPIO（通用输入输出）驱动 |
| `zf_driver_ipc.c/h` | IPC（核间通信）驱动（双核项目关键） |
| `zf_driver_pit.c/h` | PIT（周期中断定时器）驱动（控制节拍核心） |
| `zf_driver_pwm.c/h` | PWM（脉宽调制）驱动 |
| `zf_driver_soft_iic.c/h` | 软件 I2C 驱动 |
| `zf_driver_soft_spi.c/h` | 软件 SPI 驱动 |
| `zf_driver_spi.c/h` | 硬件 SPI 驱动 |
| `zf_driver_timer.c/h` | 定时器驱动 |
| `zf_driver_uart.c/h` | UART（串口）驱动 |

## 注意事项

- 此库为逐飞科技提供的开源库，修改时需保留版权声明
- 当前项目使用的 IMU 型号在 `code/config/car_select.h` 中通过 `IMU_CATEGORY` 宏选择
- 摄像头使用 MT9V03X，分辨率在代码中配置
- 屏幕使用 IPS200，支持 SPI 和并口两种接口
