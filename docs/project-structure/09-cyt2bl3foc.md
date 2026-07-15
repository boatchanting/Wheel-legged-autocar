# CYT2BL3FOC/ - 无刷双驱电机驱动器

## 概述

`CYT2BL3FOC/` 是一个独立的子项目，用于 CYT2BL3 芯片的无刷电机 FOC（磁场定向控制）驱动器。这是一个独立于主项目的电机驱动固件，运行在 M4 内核上。

## 目录结构

```text
CYT2BL3FOC/
├─ libraries/                       # CYT2BL3 开源库（与主项目 libraries/ 结构相同）
│  ├─ doc/
│  ├─ sdk/
│  ├─ zf_common/
│  ├─ zf_components/
│  ├─ zf_device/
│  └─ zf_driver/
└─ project/
   ├─ code/                         # 电机驱动业务代码
   ├─ iar/                          # IAR 工程配置
   └─ user/                         # 程序入口
```

## project/code/ - 电机驱动业务代码

| 文件 | 作用 |
|------|------|
| `fast_foc.c/h` | **核心**：FOC 磁场定向控制算法实现 |
| `motor_control.c/h` | 电机控制逻辑：速度环、电流环控制 |
| `motor_driver_uart_control.c/h` | 电机驱动串口控制：接收主控的占空比命令 |
| `motor_flash.c/h` | 电机参数 Flash 存储 |
| `motor_voice.c/h` | 电机声音提示（蜂鸣器） |
| `hall_gather.c/h` | 霍尔传感器数据采集（转子位置检测） |
| `pwm_output.c/h` | PWM 输出控制（三相逆变器） |
| `sensorless_control.c/h` | 无感控制算法（无霍尔传感器时使用） |
| `user_pwm_in.c/h` | 用户 PWM 输入捕获 |
| `driver_adc.c/h` | ADC 驱动（电流采样） |
| `driver_gpio.c/h` | GPIO 驱动 |
| `driver_config.h` | 驱动配置头文件 |
| `本文件夹作用.txt` | 说明文件 |

## project/user/ - 程序入口

| 文件 | 作用 |
|------|------|
| `main_cm4.c` | M4 内核主入口：初始化 FOC 控制器、电机参数、控制循环 |
| `cm4_isr.c` | M4 中断服务程序：PWM 中断、霍尔传感器中断、控制节拍 |

## project/iar/ - IAR 工程配置

| 文件 | 作用 |
|------|------|
| `cyt2bl3.eww` | IAR 工作空间文件 |
| `CYT2BL3双驱LED及电机声音状态说明.txt` | LED 和电机声音状态说明 |
| `icf/` | 链接脚本目录 |
| `project_config/` | 工程配置文件 |

## libraries/ - CYT2BL3 开源库

结构与主项目 `libraries/` 相同，但针对 CYT2BL3 芯片：
- `zf_common/`：公共基础模块
- `zf_driver/`：底层外设驱动
- `zf_device/`：外设设备驱动
- `sdk/`：官方芯片 SDK

## 与主项目的关系

```
主项目 (CYT4BB 双核)
    │
    ├─ 0 核：整车控制
    │   └─ small_driver_uart_control.c
    │       │ 通过 UART 发送占空比命令
    │       ▼
    └─ CYT2BL3FOC 驱动器 (M4)
        └─ motor_driver_uart_control.c
            │ 接收占空比命令
            └─ fast_foc.c
                └─ 执行 FOC 控制
```

**通信方式**：主项目通过 UART 串口向 CYT2BL3FOC 驱动器发送电机占空比命令，驱动器执行 FOC 算法控制电机。

## FOC 算法简介

FOC（Field Oriented Control，磁场定向控制）是一种先进的电机控制算法：
- **优点**：高效、低噪音、平滑控制
- **核心**：通过坐标变换（Clarke/Park）将三相电流解耦为力矩分量和磁通分量
- **需要**：转子位置（霍尔传感器或无感估算）、电流采样（ADC）

## 注意事项

- 此子项目独立于主项目，有独立的 IAR 工程和库文件
- 修改 FOC 参数后需要重新编译和下载
- 首次调试建议架空电机，避免意外转动
- 电机方向和相序需要根据实际接线配置
