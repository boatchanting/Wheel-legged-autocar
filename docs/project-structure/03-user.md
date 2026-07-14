# user/ - 双核程序入口与中断调度

## 概述

`user/` 是双核固件的实际入口层，负责"何时初始化、何时周期调用、如何把算法输出写到硬件"。包含 0 核和 1 核的 main 函数及中断服务程序。

## 目录结构

```text
user/
├─ main_cm7_0.c          # 0 核主入口
├─ cm7_0_isr.c           # 0 核中断服务程序
├─ main_cm7_1.c          # 1 核主入口
├─ cm7_1_isr.c           # 1 核中断服务程序
└─ main0/
   ├─ init_main0.c       # 0 核初始化函数实现
   └─ init_main0.h       # 0 核初始化函数声明和配置
```

## 文件详细说明

### main_cm7_0.c - 0 核主入口

**职责：**
- 选择 250 MHz 系统时钟并初始化调试串口
- 初始化屏幕、蜂鸣器、电机串口、舵机执行器
- 初始化 IMU/EKF、PID、GNSS、导航缓存、Flash、菜单、SBUS
- 初始化 0 核视觉 IPC 与视觉控制器
- 配置 PIT 中断和全局中断
- 主循环中处理菜单刷新、导航 Flash 请求、WiFi 协议轮询、视觉 IPC 结果轮询

**关键变量：**
- `uart_get_data[64]`：串口接收数据缓冲区
- `pit_state`：通道 0 中断标志位
- `g_ekf_profiler`：EKF 运行耗时统计
- `pid_out_speed/angle/pwm`：PID 控制中间变量
- `g_motor_enable`：电机使能安全开关

### cm7_0_isr.c - 0 核中断服务程序

**职责：**
- `pit0_ch0_isr()`：核心 1 ms 控制节拍
  - EKF 姿态更新
  - 导航回放
  - 视觉控制（PVC/颠簸路/桥线/三阶段）
  - 速度/角度/角速度 PID 环
  - 转向控制
  - 电机输出
  - 舵机执行器更新
- `pit0_ch1_isr()`：10 ms 遥控器处理和目标速度/角度映射
- `pit0_ch2_isr()`：2 ms 视觉 IPC 与视觉控制器更新
- UART ISR：处理调试串口或通信数据

**控制链路：**
```
PIT_CH0 1ms
├─ EKF_UpData() / 姿态角更新
├─ 导航回放 NavReplay_Process()
├─ 视觉控制 VisionPvcControl / VisionBumpyControl / VisionBridgeTask
├─ Speed_Loop_Control → Angle_Loop_Control → Gyro_Loop_Control
├─ Turn_Angle_Loop_Control → Turn_Gyro_Loop_Control
├─ small_driver_set_duty(left, right)
└─ servo_executor_update() 或 servo_jump_executor()
```

### main_cm7_1.c - 1 核主入口

**职责：**
- 初始化摄像头 `mt9v03x_init()`
- 初始化 PVC/桥线/颠簸路视觉模块
- 初始化视觉 IPC
- 配置 2 ms PIT 中断维护 IPC
- 主循环等待图像帧完成，按 0 核命令执行视觉算法

**关键流程：**
```
主循环:
1. 等待 mt9v03x_finish_flag
2. 复制原始图像到 image_copy
3. 压缩到 compressed_image_copy
4. 按 0 核命令执行视觉算法
5. 渲染识别结果到压缩图像（图传调试）
6. 通过 IPC 发布最新结果
```

### cm7_1_isr.c - 1 核中断服务程序

**职责：**
- `pit0_ch2_isr()`：2 ms 视觉 IPC 更新，调用 `VisionIpc_Core1_Update_2ms()`
- 其他 PIT 通道预留但当前未使用

### main0/init_main0.c - 0 核初始化函数

**职责：**
- 实现 `Main0_Init()` 函数，包含所有 0 核硬件初始化
- 按顺序初始化：时钟 → 调试串口 → 屏幕 → UART → 无刷电机 → 舵机 → WiFi → 蜂鸣器 → IMU → EKF → PID → 导航 → GNSS → Flash → 菜单 → SBUS → 视觉 IPC
- 预留 DMA 内存区域给 1 核摄像头使用

**关键配置：**
- `MAX_DUTY = 30`：无刷电机最大占空比 30%
- `IPS200_TYPE = IPS200_TYPE_SPI`：屏幕使用 SPI 接口
- `PIT_CH0`：1 ms 控制中断
- `PIT_CH1`：10 ms 遥控器中断

### main0/init_main0.h - 0 核初始化头文件

**职责：**
- 聚合所有需要的头文件
- 声明全局变量和配置宏
- 声明 `Main0_Init()` 函数

## 双核协作流程

```
0 核任务状态机/控制器
        │ 发送视觉任务开关、复位请求
        ▼
Vision IPC 共享结构
        │ 1 核读取命令
        ▼
1 核摄像头帧处理 + 视觉算法
        │ 发布识别结果
        ▼
0 核读取视觉结果并转化为控制输出
```
