# TongjiCar1 智能轮腿车双核控制系统

![Build](https://img.shields.io/badge/build-IAR%20manual-blue)
![Platform](https://img.shields.io/badge/platform-CYT4BB%20%7C%20Cortex--M7-orange)
![Core](https://img.shields.io/badge/core-CM7__0%20%2B%20CM7__1-purple)
![Language](https://img.shields.io/badge/language-C%20%7C%20Python%20%7C%20HTML-informational)
![License](https://img.shields.io/badge/license-GPL--3.0-green)
![Docs](https://img.shields.io/badge/docs-README%20%2B%20module%20notes-brightgreen)

> 面向智能轮腿车 / 智能车竞赛场景的双核嵌入式工程。项目以 `user/` 为主入口，在 0 核侧完成系统初始化、平衡控制、导航回放和任务状态机调度，在 1 核侧完成摄像头图像处理、视觉识别和跨核视觉结果发布。

---

## 1. 项目概述

TongjiCar1 是基于逐飞 CYT4BB 开源库、Cortex-M7 双核平台和 IAR Embedded Workbench 的车载控制项目，核心目标是让轮腿车完成平衡行驶、GNSS/惯导导航、轨迹记录与回放、单边桥、地雷区、颠簸路、三阶段跳跃等竞赛任务。

本仓库中双核工程的实际目录命名为：

| 用户描述 | 仓库实际目录 | 主要职责 |
| --- | --- | --- |
| `code/0核工程/` | `code/` | 0 核业务与算法实现：配置、EKF/PID、惯导/GNSS、轨迹记录回放、舵机/电机控制、任务状态机、0 核侧视觉控制与调试工具。 |
| `code/1核工程/` | `code1/` | 1 核视觉与图传实现：摄像头图像压缩、PVC/单边桥线/颠簸路视觉识别、视觉 IPC 发布、WiFi 图传与示波协议。 |
| `user/` | `user/` | 双核程序入口与中断调度：`main_cm7_0.c`、`cm7_0_isr.c` 调用 0 核功能；`main_cm7_1.c`、`cm7_1_isr.c` 调用 1 核视觉功能。 |

### 核心功能链路

1. `user/main_cm7_0.c` 启动 0 核，初始化时钟、调试串口、屏幕、电机驱动、舵机执行器、蜂鸣器、IMU、EKF、GNSS、导航缓存、视觉 IPC、任务控制器和 PIT 中断。
2. `user/cm7_0_isr.c` 在 1 ms / 2 ms / 10 ms 等周期中断中调度姿态更新、导航回放、遥控器、视觉控制、平衡 PID、转向控制、电机输出和舵机动作。
3. `user/main_cm7_1.c` 启动 1 核，初始化摄像头、PVC 视觉、桥线视觉、颠簸路视觉和 1 核 IPC，并在每帧图像到达时根据 0 核命令运行对应视觉算法。
4. `code/vision/vision_ipc_core0.*` 与 `code1/vision/vision_ipc_core1.*` 通过跨核共享结构完成视觉任务开关、复位请求、结果发布与轮询。
5. `tools/` 提供离线可视化、导航分析、控制仿真、传感器标定、上位机图传、CV 算法验证等辅助工具。

---

## 2. 目录结构

> 说明：以下树形图保留关键目录和核心文件，辅助脚本与文档按类别汇总展示。

```text
tongjicar1/
├─ README.md                              # 项目说明文档
├─ user/                                  # 双核入口与中断调度层
│  ├─ main_cm7_0.c                        # 0 核主入口：初始化硬件、控制模块、导航、视觉 IPC、PIT
│  ├─ cm7_0_isr.c                         # 0 核 ISR：平衡控制、遥控器、导航回放、任务状态机、电机/舵机输出
│  ├─ main_cm7_1.c                        # 1 核主入口：摄像头、视觉算法、图像压缩、视觉 IPC
│  └─ cm7_1_isr.c                         # 1 核 ISR：视觉 IPC 2 ms 更新、摄像头采集中断框架
├─ code/                                  # 0 核工程：控制、导航、任务、外设和 0 核视觉控制
│  ├─ zf_common_headfile.h                # 公共总头文件：聚合逐飞 SDK/驱动/设备头和项目模块头
│  ├─ common.h                            # 跨模块通用声明与共享状态
│  ├─ gps.c / gps.h                       # GNSS 数据处理、简易滤波、平面坐标转换与稳定性检测
│  ├─ small_driver_uart_control.c/.h      # 无刷电机/小驱动串口控制
│  ├─ config/                             # 车型与全局开关配置
│  │  ├─ config.h                         # 配置聚合头，包含 car_select.h 与 sys_options.h
│  │  ├─ car_select.h                     # 车型选择及硬件差异说明
│  │  └─ sys_options.h                    # WiFi、显示、遥控、IMU、科目等全局开关
│  ├─ calculate/                          # 计算与控制基础层
│  │  ├─ ekf.c / ekf.h                    # IMU 姿态 EKF、航向角、陀螺校准、磁力计校准
│  │  ├─ matrix.c / matrix.h              # 小规模矩阵运算、归一化、限幅等数学工具
│  │  └─ pid-new.c / pid-new.h            # 速度/角度/角速度/转向/横滚 PID 与制动前馈
│  ├─ navigation/                         # 导航、轨迹记录和回放
│  │  ├─ inertial_nav.c/.h                # 惯性导航状态初始化
│  │  ├─ gnss_transform.c/.h              # GNSS 经纬度到高斯-克吕格/局部坐标转换
│  │  ├─ nav_ram.c/.h                     # RAM 中轨迹点记录、计数与任务点蜂鸣提示
│  │  ├─ nav_replay.c/.h                  # 轨迹回放、最近点搜索、弯道预判和任务触发
│  │  ├─ nav_replay_route_table.h         # 静态回放路线表
│  │  └─ ram2flash.c/.h                   # RAM 轨迹与 Flash 之间的读写请求处理
│  ├─ plan/                               # 科目/场景任务策略
│  │  ├─ bridge.c/.h                      # 单边桥姿态、高度、横滚与三连桥测试控制
│  │  ├─ bumpy_road.c/.h                  # 颠簸路状态机、距离计算、视觉转向辅助
│  │  └─ minefield.c/.h                   # 地雷区旋转控制与状态管理
│  ├─ servo/                              # 舵机、轮腿机构和跳跃动作
│  │  ├─ servo.c/.h                       # 舵机角度/占空比映射、高度姿态表、当前角度缓存
│  │  ├─ servo_executor.c/.h              # 常规舵机目标平滑执行器
│  │  └─ servo_jump.c/.h                  # 台阶/跳跃动作序列与动量轮控制接口
│  ├─ tools/                              # 0 核通用工具与通信
│  │  ├─ beep.c/.h                        # 蜂鸣器初始化与提示
│  │  ├─ flash.c/.h                       # 参数 Flash 读写
│  │  ├─ menu.c/.h                        # IPS 屏菜单、按键操作、任务启动/保存入口
│  │  ├─ sbus.c/.h                        # SBUS 遥控器解析与目标速度/转向映射
│  │  ├─ wifi.c/.h                        # 0 核 WiFi 初始化、连接、PID 参数更新辅助
│  │  ├─ wifi_protocol.c/.h               # 0 核自定义 WiFi 帧协议
│  │  ├─ telemetry_ipc.h                  # 遥测 IPC 数据结构
│  │  ├─ telemetry_ipc_core0.c/.h         # 0 核遥测发布
│  │  └─ runtime_profiler.h               # 运行耗时统计结构
│  └─ vision/                             # 0 核侧视觉任务控制
│     ├─ vision_ipc.h                     # 跨核视觉 IPC 公共结构
│     ├─ vision_ipc_core0.c/.h            # 0 核视觉命令发布与结果轮询
│     ├─ vision_pvc_control.c/.h          # PVC 视觉结果到转向误差的控制接口
│     ├─ vision_bumpy_control.c/.h        # 颠簸路视觉辅助控制
│     ├─ vision_bridge_control.c/.h       # 单边桥视觉对线/姿态控制任务
│     └─ vision_three_stage_control.c/.h  # 三阶段视觉任务与跳跃触发控制
├─ code1/                                 # 1 核工程：视觉算法、图传和 1 核 IPC
│  ├─ wifi.c/.h                           # 1 核图像压缩、视觉结果渲染、WiFi 初始化辅助
│  ├─ wifi_diff_stream.c/.h               # 灰度图差分帧流发送
│  ├─ wifi_protocol.c/.h                  # 1 核 WiFi 控制/示波协议
│  └─ vision/
│     ├─ pvc_vision.c/.h                  # PVC 白色目标检测、连通域筛选、物理坐标估计
│     ├─ line_vision.c/.h                 # 单边桥/直线检测与中心线输出
│     ├─ bumpy_vision.c/.h                # 颠簸路白/暗特征检测与滤波
│     ├─ ipm_transform.c/.h               # 逆透视映射查询与物理距离计算
│     ├─ vision_ipc_core1.c/.h            # 1 核读取命令、发布视觉结果/空闲状态
│     └─ telemetry_ipc_core1.c/.h         # 1 核读取遥测数据
├─ docs/                                  # 设计说明、任务规划、模块文档、调试记录
│  ├─ code文件概览.md                     # code/user 新人导读
│  ├─ 任务放在哪个核里.md                 # 双核任务分配草案
│  ├─ 模块文档/                           # beep/flash/menu/sbus/wifi/舵机/屏幕/GPIO 等模块说明
│  └─ 任务规划/                           # GNSS、科目一二三、视觉融合、底层优化等规划
├─ tools/                                 # PC 端调试、仿真、可视化和算法验证工具
│  ├─ 01_导航与定位可视化/                # GNSS/惯导轨迹 HTML 与 Python 可视化
│  ├─ 02_导航算法分析/                    # 坐标对齐、回环检测、路径规划方案对比
│  ├─ 03_控制与仿真/                      # PID 调参、轮腿仿真、MPC/Pure Pursuit/RL 仿真
│  ├─ 04_传感器标定与测试/                # 磁力计、逆透视、重力加速度、图传压缩测试
│  ├─ 05_通用数据处理工具/                # 代码量统计、changelog、视频网页上位机
│  ├─ 06_算法原理动画/                    # 五连杆、惯导等算法演示
│  ├─ 07_针对小车车载视频的cv算法/        # 车载视频 CV 算法原型与 C 检测器
│  ├─ cvtest/                             # 桥、地雷、台阶场景的离线 CV 测试
│  ├─ webview_nav_marker/                 # 导航点标注、CSV 转路线表和 WebView 工具
│  └─ wifi_protocol/                      # WiFi 上位机页面、Streamlit 调试与 CSV 可视化
└─ iar/
   ├─ icf/linker_directives_tviibh.icf    # IAR 链接脚本
   └─ project_config/                     # CM7_0 / CM7_1 IAR 工程配置文件
```

---

## 3. 核心工程说明

### 3.1 0 核工程（`code/` + `user/main_cm7_0.c` + `user/cm7_0_isr.c`）

0 核是整车控制主核，偏实时控制与任务决策。

#### 主要模块

| 模块 | 关键文件 | 功能 |
| --- | --- | --- |
| 配置层 | `code/config/*.h` | 统一选择车型、IMU、WiFi、屏幕、遥控器、当前科目等编译期配置。 |
| 姿态与数学 | `code/calculate/ekf.*`、`matrix.*` | IMU 采样、陀螺校准、EKF 姿态解算、欧拉角输出、航向角更新和基础矩阵运算。 |
| PID 控制 | `code/calculate/pid-new.*` | 速度环、角度环、角速度环、转向角度/角速度环、横滚平衡、制动前馈。 |
| 执行器 | `code/small_driver_uart_control.*`、`code/servo/*` | 小驱动串口电机占空比输出，舵机占空比/角度控制，常规平滑执行器和跳跃动作执行器。 |
| 导航 | `code/gps.*`、`code/navigation/*` | GNSS 处理、局部坐标转换、惯导状态、RAM/Flash 轨迹记录、静态路线加载、回放和任务点触发。 |
| 任务策略 | `code/plan/*` | 单边桥、颠簸路、地雷区等科目状态机和专用控制逻辑。 |
| 视觉控制 | `code/vision/*` | 0 核下发视觉任务命令，读取 1 核视觉结果，并转化为 PVC、颠簸路、桥线和三阶段控制输出。 |
| 调试工具 | `code/tools/*` | 蜂鸣器、Flash 参数、菜单、SBUS 遥控器、WiFi 协议、遥测 IPC、耗时统计。 |

#### 初始化与调度

0 核主函数典型初始化顺序如下：

1. `clock_init(SYSTEM_CLOCK_250M)` 和 `debug_init()`。
2. 初始化 IPS200 屏幕、UART FIFO、调试串口和串口接收中断。
3. 初始化无刷电机串口驱动、舵机执行器、蜂鸣器。
4. 按配置初始化 WiFi / 摄像头辅助功能（当前 `WIFI_USE` 默认为 `0`）。
5. 初始化 IMU、EKF、PID 参数、导航 RAM、GNSS 转换、Flash 导航请求、菜单、SBUS 遥控器、视觉 IPC 与视觉控制器。
6. 配置 PIT 周期中断：0 核主要使用 1 ms 控制中断、10 ms 遥控/辅助中断以及 2 ms 视觉 IPC/视觉控制更新节拍。

#### 主要输出

- 左右轮电机 PWM/占空比：通过 `small_driver_set_duty()` 输出到小驱动。
- 舵机角度/占空比：通过 `servo_executor_update()` 或 `servo_jump_executor()` 输出。
- 任务状态和导航状态：保存在全局状态、RAM 轨迹缓存和 Flash 中。
- 屏幕/蜂鸣器/串口/WiFi 调试信息：用于现场调参和状态确认。

### 3.2 1 核工程（`code1/` + `user/main_cm7_1.c` + `user/cm7_1_isr.c`）

1 核是视觉和图像处理辅助核，偏并行计算与数据发布。

#### 主要模块

| 模块 | 关键文件 | 功能 |
| --- | --- | --- |
| 图像采集与压缩 | `user/main_cm7_1.c`、`code1/wifi.*` | 初始化 MT9V03X 摄像头，将原图复制并压缩到视觉算法输入尺寸。 |
| PVC 识别 | `code1/vision/pvc_vision.*` | 检测白色 PVC 目标，输出连通域、置信度、中心点、物理坐标等结果。 |
| 桥线/直线识别 | `code1/vision/line_vision.*` | 提取近端白线/暗色桥体特征，输出线中心和偏差。 |
| 颠簸路识别 | `code1/vision/bumpy_vision.*` | 检测颠簸路相关白色/暗色结构，输出候选位置和置信度。 |
| IPM 坐标 | `code1/vision/ipm_transform.*` | 通过逆透视查表将像素位置转换为物理坐标或距离。 |
| 跨核通信 | `code1/vision/vision_ipc_core1.*` | 读取 0 核命令、处理复位请求、发布当前视觉结果或空闲状态。 |
| 图传/协议 | `code1/wifi_diff_stream.*`、`code1/wifi_protocol.*` | 差分帧流、控制帧、示波数据发送；部分调用当前被注释，按调试需求启用。 |

#### 运行逻辑

1. 1 核初始化时钟、调试信息、摄像头、PVC/桥线/颠簸路视觉模块和视觉 IPC。
2. 配置 `PIT_CH2` 为 2 ms 中断，调用 `VisionIpc_Core1_Update_2ms()` 维护跨核通信状态。
3. 主循环等待 `mt9v03x_finish_flag`，一旦摄像头帧完成：
   - 复制原始图像到 `image_copy`；
   - 压缩到 `compressed_image_copy`；
   - 按 0 核命令执行 PVC、桥线、颠簸路中的一个或多个视觉算法；
   - 将识别结果渲染到压缩图像（用于图传调试）；
   - 通过 IPC 发布最新结果供 0 核读取。

#### 主要输出

- 视觉识别结构体：目标是否有效、置信度、像素坐标、物理坐标、角度/偏差等。
- 跨核 IPC 数据包：由 1 核发布，0 核轮询读取。
- 可选 WiFi 图像/示波数据：用于上位机调试，当前部分发送逻辑需要按现场配置打开。

### 3.3 双核差异与协作关系

| 维度 | 0 核 | 1 核 |
| --- | --- | --- |
| 实时性重点 | 平衡控制、电机输出、任务状态机、遥控安全 | 摄像头帧处理、视觉算法、图像调试 |
| 主要入口 | `user/main_cm7_0.c` | `user/main_cm7_1.c` |
| 主要 ISR | `user/cm7_0_isr.c`：PIT 控制节拍、遥控器、UART | `user/cm7_1_isr.c`：视觉 IPC 2 ms、摄像头采集框架 |
| 共享方式 | 通过 `VisionIpc_Core0_*` 下发任务、轮询结果 | 通过 `VisionIpc_Core1_*` 接收任务、发布结果 |
| 输出对象 | 电机、舵机、导航/任务状态、屏幕/蜂鸣器 | 视觉结果、可选图传/示波数据 |

协作流程可概括为：

```text
0 核任务状态机/控制器
        │ 发送视觉任务开关、复位请求
        ▼
Vision IPC 共享结构
        │ 1 核读取命令
        ▼
1 核摄像头帧处理 + 视觉算法
        │ 发布识别结果
        ▼
0 核读取视觉结果并转化为 err_degree / target_speed / 舵机动作 / 任务触发
```

---

## 4. `user/` 调用说明

`user/` 目录不是独立算法库，而是双核固件的实际入口层。它负责“何时初始化、何时周期调用、如何把算法输出写到硬件”。

### 4.1 0 核入口：`user/main_cm7_0.c`

主要职责：

- 选择 250 MHz 系统时钟并初始化调试串口。
- 初始化屏幕、蜂鸣器、电机串口、舵机执行器、IMU/EKF、PID、GNSS、导航缓存、Flash、菜单和 SBUS。
- 初始化 0 核视觉 IPC 与 PVC/颠簸路/桥线/三阶段视觉控制器。
- 配置 PIT 中断和全局中断。
- 主循环中处理菜单刷新、导航 Flash 请求、WiFi 协议轮询、视觉 IPC 结果轮询等低频任务。

典型调用片段：

```c
#include "zf_common_headfile.h"
#include "config/config.h"
#include "vision/vision_ipc_core0.h"

int main(void)
{
    clock_init(SYSTEM_CLOCK_250M);
    debug_init();

    small_driver_uart_init();
    servo_executor_init();
    EKF_Init();
    NavRam_Init();
    VisionIpc_Core0_Init();

    pit_ms_init(PIT_CH0, 1);   // 1 ms 控制节拍
    interrupt_global_enable(0);

    while(true)
    {
        VisionIpc_Core0_PollResult();
        NavFlash_ProcessRequests();
    }
}
```

### 4.2 0 核中断：`user/cm7_0_isr.c`

主要职责：

- `pit0_ch0_isr()`：核心 1 ms 控制节拍，包含 EKF 更新、制动前馈、速度估计、导航回放、视觉控制、平衡/转向 PID、电机输出和舵机执行器更新。
- `pit0_ch1_isr()`：遥控器处理和目标速度/角度映射，急停时停止导航回放并复位 PID。
- `pit0_ch2_isr()`：2 ms 视觉 IPC 与视觉控制器更新。
- UART ISR：处理调试串口或通信数据。

典型控制链路：

```text
PIT_CH0 1ms
├─ EKF_UpData() / 姿态角更新
├─ 导航回放 NavReplay_Process()
├─ 视觉控制 VisionPvcControl / VisionBumpyControl / VisionBridgeTask
├─ Speed_Loop_Control → Angle_Loop_Control → Gyro_Loop_Control
├─ Turn_Angle_Loop_Control → Turn_Gyro_Loop_Control
├─ small_driver_set_duty(left, right)
└─ servo_executor_update() 或 servo_jump_executor()
```

### 4.3 1 核入口：`user/main_cm7_1.c`

主要职责：

- 初始化摄像头 `mt9v03x_init()`。
- 初始化 `pvc_vision_init()`、`line_vision_init()`、`bumpy_vision_init()`。
- 初始化 `VisionIpc_Core1_Init()`，并用 2 ms PIT 更新 IPC。
- 主循环等待图像帧完成，根据 0 核命令执行对应视觉算法。

典型调用片段：

```c
#include "../code1/vision/pvc_vision.h"
#include "../code1/vision/bumpy_vision.h"
#include "../code1/vision/vision_ipc_core1.h"

int main(void)
{
    clock_init(SYSTEM_CLOCK_250M);
    debug_info_init();

    mt9v03x_init();
    pvc_vision_init();
    line_vision_init();
    bumpy_vision_init();
    VisionIpc_Core1_Init();

    pit_ms_init(PIT_CH2, 2);
    interrupt_global_enable(0);

    while(true)
    {
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
            compress_image_to_target();

            if(VisionIpc_Core1_ShouldRunPvc())
            {
                pvc_vision_process_camera_frame(compressed_image_copy[0]);
                VisionIpc_Core1_PublishCurrent();
            }
        }
    }
}
```

### 4.4 常用配置示例

#### 切换车型

```c
// code/config/car_select.h
#define CAR_SELECT 3
```

#### 切换科目与调试开关

```c
// code/config/sys_options.h
#define WIFI_USE 0
#define DEBUG_DISPLAY 1
#define REMOTE_CONTROL 1
#define DEBUG_LOG_ENABLE 0
#define IMU_CATEGORY 3
#define CURRENT_NAV_PLAN 2
```

#### 开启 1 核视觉任务（概念示例）

0 核侧根据任务状态调用视觉 IPC 控制接口：

```c
VisionIpc_Core0_SetPvcEnable(1);
VisionIpc_Core0_SetBridgeLineEnable(1);
VisionIpc_Core0_SetBumpyEnable(0);
VisionIpc_Core0_Update_2ms();
VisionIpc_Core0_PollResult();
```

1 核侧在每帧中按命令运行算法：

```c
if(VisionIpc_Core1_ShouldRunBridgeLine())
{
    line_vision_process_camera_frame(compressed_image_copy[0]);
    render_line_vision_to_image();
}
```

---

## 5. 环境与依赖

### 5.1 嵌入式固件环境

| 项目 | 要求 |
| --- | --- |
| MCU / 平台 | CYT4BB / Cortex-M7 双核平台 |
| 开发工具 | IAR Embedded Workbench，源文件注释中标注的开发环境为 IAR 9.40.1 |
| 底层库 | 逐飞 CYT4BB 开源库、官方 SDK、CMSIS/ARM Math |
| 工程配置 | `iar/project_config/cyt4bb7_cm_7_0.ewp`、`iar/project_config/cyt4bb7_cm_7_1.ewp` |
| 链接脚本 | `iar/icf/linker_directives_tviibh.icf` |
| 传感器/外设 | IMU660RA/IMU660RB/IMU963RA、GNSS、MT9V03X 摄像头、IPS200 屏幕、SBUS 遥控器、蜂鸣器、Flash、WiFi 模块、小驱动/无刷电机、舵机 |

> 待补充：仓库未包含完整 `libraries/`、逐飞 SDK 和官方芯片 SDK 文件。请在本地 IAR 工程中确认 include path、库路径和启动文件配置完整。

### 5.2 PC 工具环境

`tools/` 下脚本以 Python 与 HTML 为主，按具体工具可能需要：

- Python 3.9+
- 常见科学计算/可视化库：`numpy`、`pandas`、`matplotlib`、`opencv-python`、`streamlit` 等
- 浏览器：用于打开 HTML 可视化页面和 WebView 上位机
- PowerShell / GCC 或 MinGW：用于 `tools/07_针对小车车载视频的cv算法/*/c_*_detector/` 下的 C 检测器构建脚本

> 待补充：仓库当前没有统一的 `requirements.txt`。运行某个 Python 工具时，如提示缺少包，请按报错逐项安装。

---

## 6. 安装与运行

### 6.1 获取代码

```bash
git clone <repo-url>
cd tongjicar1
```

### 6.2 配置嵌入式依赖

1. 安装 IAR Embedded Workbench（建议与逐飞示例工程一致，当前源码注释指向 IAR 9.40.1）。
2. 准备逐飞 CYT4BB 开源库、官方 SDK、启动文件、芯片头文件和设备驱动。
3. 打开 `iar/project_config/` 下的两个工程配置：
   - `cyt4bb7_cm_7_0.ewp`：0 核工程；
   - `cyt4bb7_cm_7_1.ewp`：1 核工程。
4. 检查 include path 是否能找到：
   - `code/`、`code1/`、`user/`；
   - 逐飞 `zf_common_*`、`zf_driver_*`、`zf_device_*`；
   - 芯片 SDK 头文件，如 `cy_project.h`、`cy_device_headers.h`、`arm_math.h`。
5. 检查链接脚本是否使用 `iar/icf/linker_directives_tviibh.icf`。

### 6.3 修改关键配置

1. 在 `code/config/car_select.h` 中设置当前车辆：

```c
#define CAR_SELECT 3
```

2. 在 `code/config/sys_options.h` 中设置功能开关：

```c
#define WIFI_USE 0
#define DEBUG_DISPLAY 1
#define REMOTE_CONTROL 1
#define DEBUG_LOG_ENABLE 0
#define IMU_CATEGORY 3
#define CURRENT_NAV_PLAN 2
```

3. 根据实车检查：
   - 舵机机械零点、方向、限幅；
   - 电机左右方向；
   - IMU 安装方向与零偏；
   - GNSS 串口配置；
   - 摄像头分辨率与曝光；
   - SBUS 遥控器方向和急停开关。

### 6.4 编译与烧录

```text
1. 在 IAR 中分别打开 CM7_0 与 CM7_1 工程。
2. Clean 后重新 Build，确保双核工程均编译通过。
3. 先烧录/下载 0 核与 1 核固件到目标板。
4. 上电后观察 IPS 屏幕、蜂鸣器、调试串口输出。
5. 确认 IMU 初始化与陀螺校准完成后，再允许电机使能。
```

### 6.5 运行 PC 调试工具

示例：

```bash
# 导航 / 惯导轨迹可视化
python tools/01_导航与定位可视化/inertial_nav结果可视化.py

# PID 调参辅助
python tools/03_控制与仿真/pid调参.py

# WiFi 协议 Streamlit 上位机
streamlit run tools/wifi_protocol/streamlit_wifi.py

# 导航点标注上位机
python tools/webview_nav_marker/nav_marker_host.py
```

> 注意：部分工具依赖本地 CSV、视频、串口或网络环境，脚本启动参数与输入文件格式请结合对应目录下的 README 或脚本注释确认。

---

## 7. 常见问题

### Q1：用户说明中的 `code/0核工程/`、`code/1核工程/` 在仓库中找不到？

当前仓库实际目录为 `code/` 和 `code1/`。根据文件内容和调用关系，`code/` 对应 0 核工程，`code1/` 对应 1 核工程。若后续希望与说明完全一致，可以重命名目录或在文档/工程配置中统一叫法，但重命名会影响 IAR 工程 include path，需要谨慎处理。

### Q2：IAR 编译提示找不到 `zf_common_*`、`zf_driver_*`、`zf_device_*` 或 `cy_project.h`？

这些属于逐飞 CYT4BB 开源库和芯片 SDK 依赖，当前仓库未包含完整库目录。请检查：

- IAR 工程 include path 是否指向本地逐飞库；
- SDK 版本是否与 CYT4BB 平台匹配；
- `zf_common_headfile.h` 中引用的设备驱动是否存在。

### Q3：上车后电机突然高速转动或无法直立？

优先按安全顺序排查：

1. 保持 `g_motor_enable = 0` 或遥控急停关闭，先看姿态角是否稳定。
2. 检查 IMU 类型 `IMU_CATEGORY`、安装方向、陀螺零偏和 `g_yaw_initialized`。
3. 检查左右电机方向、PWM 极性和 `small_driver_set_duty()` 输出方向。
4. 检查 `pid-new.h` 中速度环、角度环、角速度环参数与机械零点。
5. 首次调试建议架空车体、降低限幅、关闭自动导航回放。

### Q4：视觉结果一直无效？

可能原因：

- 1 核工程未运行或未烧录；
- `VisionIpc_Core0_Set*Enable()` 没有打开对应视觉任务；
- 摄像头 `mt9v03x_finish_flag` 未置位或摄像头初始化失败；
- 图像压缩尺寸与视觉算法宏定义不一致；
- 阈值、曝光、赛道颜色与当前环境不匹配；
- IPC 更新中断 `PIT_CH2` 未启动。

### Q5：WiFi 图传或上位机没有数据？

当前 0 核和 1 核中部分 WiFi 初始化、连接、差分帧发送和示波发送逻辑是可选或注释状态。请确认：

- `WIFI_USE`、`WIFI_IMAGE_SEND`、`WIFI_CAMERA_AND_ASSISTANT` 是否符合当前核的使用方式；
- WiFi SSID、密码、目标 IP、目标端口是否正确；
- 是否调用了 `wifi_connect_tcp_server()` 或对应发送函数；
- 0 核 WiFi 摄像头辅助与 1 核视觉图传不要同时抢占同一硬件资源。

### Q6：导航记录/回放无效？

检查：

- `NavRam_Init()` 是否执行；
- 菜单或遥控器是否设置了 `g_nav_start_recording`、`g_save_flash_request`、`g_load_flash_request`、`g_replay_start_request`；
- Flash 读写是否成功；
- `CURRENT_NAV_PLAN` 是否符合当前科目；
- GNSS 原点和高斯-克吕格坐标转换是否已初始化。

---

## 8. 开发建议

- 新增 0 核业务模块时，优先放入 `code/` 对应子目录，并在 `code/zf_common_headfile.h` 或局部源文件中显式包含头文件。
- 新增 1 核视觉算法时，优先放入 `code1/vision/`，并通过 `vision_ipc_core1` 发布统一格式结果，避免 0 核直接依赖具体算法内部变量。
- 涉及实时控制的逻辑尽量放在固定周期 ISR 中；耗时打印、Flash 写入、上位机协议解析等低频任务放在主循环。
- 比赛/实车运行前关闭高频串口日志，保留必要蜂鸣器和屏幕状态提示。
- 修改 PID、舵机零点、电机方向、跳跃动作表后，必须先架空或限幅测试，再落地调试。

---

## 9. 待补充信息

- 完整逐飞库 / 官方 SDK 的版本、路径和获取方式。
- IAR workspace（`.eww`）或一键构建说明。
- 统一 Python `requirements.txt`。
- 各科目最终参数表、实车标定记录和安全调试 SOP。
- WiFi 上位机协议字段的正式版本说明。
- LICENSE 文件或项目自有代码许可证声明；当前源码头部主要继承逐飞 CYT4BB GPL-3.0 声明。

---

## 10. 更新记录

- `2026-05-09 11:28:21`：重写 README，补充双核目录映射、模块职责、调用链路、环境依赖、安装运行步骤与常见问题。
- `2026-05-09 11:35:03`：补充 README 顶部徽章块，展示构建方式、平台、双核架构、语言、许可证与文档状态。

**最后更新时间：2026-05-09 11:35:03**
