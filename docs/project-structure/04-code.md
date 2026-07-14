# code/ - 0 核工程

## 概述

`code/` 是 0 核（主核）的业务代码目录，包含控制算法、导航、任务策略、执行器控制、视觉控制和调试工具。0 核负责整车控制、平衡、导航回放和任务状态机调度。

## 目录结构

```text
code/
├─ common.h                           # 跨模块通用声明与共享状态
├─ small_driver_uart_control.c/h      # 无刷电机/小驱动串口控制
├─ config/                            # 车型与全局开关配置
├─ calculate/                         # 计算与控制基础层
├─ navigation/                        # 导航、轨迹记录和回放
├─ plan/                              # 科目/场景任务策略
├─ servo/                             # 舵机、轮腿机构和跳跃动作
├─ tools/                             # 0 核通用工具与通信
└─ vision/                            # 0 核侧视觉任务控制
```

## 根文件

| 文件 | 作用 |
|------|------|
| `common.h` | 跨模块通用声明：外部中断端口定义、导航控制标志位全局声明 |
| `small_driver_uart_control.c/h` | 无刷电机串口驱动控制：通过 UART 发送占空比命令控制左右轮电机 |
| `本文件夹作用.txt` | 说明：用户添加自己的代码文件时存放在此目录 |

## config/ - 配置层

| 文件 | 作用 |
|------|------|
| `config.h` | 配置聚合头文件，包含 `car_select.h` 和 `sys_options.h` |
| `car_select.h` | 车型选择：`CAR_SELECT` 宏定义当前车辆编号（1/2/3），决定硬件差异 |
| `sys_options.h` | 全局功能开关：WiFi、显示、遥控、IMU 类型、当前科目等编译期配置 |
| `wifi_options.h` | WiFi 相关配置：SSID、密码、IP 地址等 |

**关键配置项：**
```c
#define WIFI_USE 0              // WiFi 是否启用
#define DEBUG_DISPLAY 1         // 屏幕调试显示
#define REMOTE_CONTROL 1        // 遥控器启用
#define IMU_CATEGORY 3          // IMU 型号选择
#define CURRENT_NAV_PLAN 2      // 当前导航方案
```

## calculate/ - 计算与控制基础层

| 文件 | 作用 |
|------|------|
| `ekf.c/h` | 扩展卡尔曼滤波器（EKF）：IMU 姿态解算、航向角、陀螺校准、磁力计校准 |
| `matrix.c/h` | 矩阵运算工具：小规模矩阵运算、归一化、限幅等数学工具 |
| `pid-new.c/h` | PID 控制器：速度环、角度环、角速度环、转向角度/角速度环、横滚平衡、制动前馈 |

**EKF 输出：**
- `euler_angle`：欧拉角（pitch/roll/yaw）
- 陀螺校准后的角速度
- 航向角更新

**PID 控制链路：**
```
速度环 → 角度环 → 角速度环 → 电机输出
转向角度环 → 转向角速度环 → 舵机输出
```

## navigation/ - 导航模块

| 文件 | 作用 |
|------|------|
| `gnss_transform.c/h` | GNSS 坐标转换：经纬度到高斯-克吕格/局部坐标转换 |
| `inertial_nav.c/h` | 惯性导航状态初始化 |
| `nav_ram.c/h` | RAM 轨迹记录：轨迹点存储、计数、任务点蜂鸣提示 |
| `nav_replay.c/h` | 轨迹回放：最近点搜索、弯道预判、任务触发 |
| `nav_replay_route_table.h` | 静态回放路线表 |
| `gps_nav_replay_route_table.h` | GPS 导航回放路线表 |
| `ram2flash.c/h` | RAM/Flash 读写：轨迹数据在 RAM 和 Flash 之间传输 |

### nav_replay/ - 导航回放方案

```text
nav_replay/
├─ nav_options.h                    # 导航方案选择配置
├─ nav_replay.h                     # 回放接口声明
├─ plan1/                           # 方案一：纯追踪 + GNSS
│  ├─ plan1_gnss.c/h               # GNSS 导航逻辑
│  ├─ plan1_pure_pursuit.c/h       # 纯追踪算法
│  ├─ plan1_pure_pursuit_speed_planning.c/h  # 纯追踪 + 速度规划 + Stanley CTE 补偿
│  ├─ plan1_lqr_tracking.c/h       # LQR 跟踪算法
│  └─ plan1_lqr_tuning_guide.md    # LQR 调参指南
├─ plan2/                           # 方案二：纯追踪 + 精确控制
│  ├─ plan2_pure_pursuit.c/h       # 纯追踪算法
│  ├─ plan2_pure_pursuit_speed_planning.c/h  # 纯追踪 + 速度规划
│  ├─ plan2_precise.c/h            # 精确控制逻辑
│  └─ plan2_point_speed_planning.c/h  # 逐点速度规划
├─ plan3/                           # 方案三：精确控制
│  └─ plan3_precise.c/h            # 精确控制逻辑
└─ template/                        # 模板
   └─ nav_plan_template.c/h        # 导航方案模板
```

## plan/ - 任务策略

| 文件 | 作用 |
|------|------|
| `bridge.c/h` | 单边桥任务：姿态、高度、横滚控制，三连桥测试 |
| `bumpy_road.c/h` | 颠簸路任务：状态机、距离计算、视觉转向辅助 |
| `minefield.c/h` | 地雷区任务：旋转控制与状态管理 |

## servo/ - 舵机控制

| 文件 | 作用 |
|------|------|
| `servo.c/h` | 舵机基础控制：角度/占空比映射、高度姿态表、当前角度缓存 |
| `servo_executor.c/h` | 常规舵机执行器：目标角度平滑过渡 |
| `servo_jump.c/h` | 跳跃动作执行器：台阶/跳跃动作序列、动量轮控制接口 |

## tools/ - 0 核通用工具

| 文件 | 作用 |
|------|------|
| `beep.c/h` | 蜂鸣器初始化与提示音 |
| `flash.c/h` | 参数 Flash 读写 |
| `menu.c/h` | IPS 屏菜单：按键操作、任务启动/保存入口 |
| `sbus.c/h` | SBUS 遥控器解析：目标速度/转向映射 |
| `wifi.c/h` | 0 核 WiFi 初始化、连接、PID 参数更新辅助 |
| `wifi_protocol.c/h` | 0 核自定义 WiFi 帧协议 |
| `telemetry_ipc.h` | 遥测 IPC 数据结构定义 |
| `telemetry_ipc_core0.c/h` | 0 核遥测数据发布 |
| `runtime_profiler.h` | 运行耗时统计结构 |

## vision/ - 视觉控制

| 文件 | 作用 |
|------|------|
| `vision_ipc.h` | 跨核视觉 IPC 公共结构定义 |
| `vision_ipc_core0.c/h` | 0 核视觉命令发布与结果轮询 |
| `vision_pvc_control.c/h` | PVC 视觉结果到转向误差的控制接口 |
| `vision_bumpy_control.c/h` | 颠簸路视觉辅助控制 |
| `vision_bridge_control.c/h` | 单边桥视觉对线/姿态控制 |
| `vision_three_stage_control.c/h` | 三阶段视觉任务与跳跃触发控制 |

## 模块调用关系

```
user/main_cm7_0.c (初始化)
    │
    ├─ config/config.h (配置)
    ├─ calculate/ekf.* (姿态)
    ├─ calculate/pid-new.* (控制)
    ├─ navigation/* (导航)
    ├─ servo/* (执行器)
    ├─ tools/* (辅助)
    └─ vision/* (视觉控制)

user/cm7_0_isr.c (1ms 周期)
    │
    ├─ EKF_UpData()
    ├─ NavReplay_Process()
    ├─ VisionPvcControl / VisionBumpyControl / VisionBridgeTask
    ├─ Speed/Angle/Gyro Loop Control
    ├─ small_driver_set_duty()
    └─ servo_executor_update()
```
