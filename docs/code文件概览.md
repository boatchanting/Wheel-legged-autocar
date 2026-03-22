# code 与 user 文件夹新人导读

> 面向第一次接触本仓库的同学：先看 `user/` 的入口，再看 `code/` 的功能模块。

## 1. 整体结构（一句话）

这个工程可以理解为：

- `user/`：**系统入口与中断调度层**（双核主函数、ISR）。
- `code/`：**业务与算法实现层**（配置、控制、导航、任务策略、工具模块）。

主程序会通过公共头文件 `code/zf_common_headfile.h` 将各模块汇总后使用。  

---

## 2. `user/` 文件夹：程序从哪里开始跑

`user/` 下主要有 4 个关键文件：

- `main_cm7_0.c`：CM7_0 核心主入口，负责初始化硬件、传感器、通信与主循环逻辑。
- `cm7_0_isr.c`：CM7_0 的中断服务，核心控制节拍（PIT）和控制环多在这里调度。
- `main_cm7_1.c`：CM7_1 核心主入口，偏向 WiFi 图像发送等并行任务。
- `cm7_1_isr.c`：CM7_1 的中断服务框架（目前多为模板/占位）。

### 新人建议阅读顺序

1. 先看 `user/main_cm7_0.c`，建立“系统初始化 + 主循环”全局认知。  
2. 再看 `user/cm7_0_isr.c`，理解控制周期任务如何被定时中断触发。  
3. 最后看 `user/main_cm7_1.c` 与 `user/cm7_1_isr.c`，理解双核分工。  

---

## 3. `code/` 文件夹：功能代码按模块分层

`code/` 是项目核心实现目录，按“配置 → 算法/控制 → 任务策略 → 工具”组织。

### 3.1 顶层关键文件

- `zf_common_headfile.h`：统一聚合底层 SDK、驱动、设备与本项目各模块头文件，是主程序常用总入口。
- `common.h`：项目通用宏和跨模块共享变量声明（例如导航录制/回放标志位）。
- `gps.c/.h`、`small_driver_uart_control.c/.h`：独立设备/驱动模块。

### 3.2 子目录职责总览

| 子目录 | 作用 | 典型内容 |
|---|---|---|
| `config/` | 全局配置与开关 | `config.h`、`sys_options.h`、`car_select.h` |
| `calculate/` | 算法基础层 | EKF、PID、矩阵运算 |
| `navigation/` | 导航状态与轨迹 | 惯导、GNSS坐标转换、RAM记录与回放 |
| `servo/` | 舵机与执行器控制 | 舵机控制、动作执行、跳跃相关控制 |
| `plan/` | 科目任务策略层 | 地雷区、单边桥等任务逻辑 |
| `tools/` | 通用工具与外设辅助 | 蜂鸣器、Flash、WiFi协议、菜单、SBUS |

---

## 4. `user` 与 `code` 的关系（调用链）

可以按下面的思路理解：

1. `user/main_cm7_0.c` 启动并初始化硬件与模块。  
2. 通过 `zf_common_headfile.h` 引入 `code/` 中各功能头文件。  
3. `user/cm7_0_isr.c` 的 PIT 中断周期性调用：
   - 导航更新（惯导/GNSS）
   - 任务策略（如桥/雷区）
   - 多层控制环（速度、角度、平衡等）
4. `code/` 各模块提供具体实现，`user/` 负责“何时调用、按什么节拍调用”。

---

## 5. 给新人的快速上手路径（建议）

### 第一天

- 读 `docs/任务规划/` 下的任务背景文档（了解业务目标）。
- 通读 `user/main_cm7_0.c` 的初始化顺序。

### 第二天

- 读 `user/cm7_0_isr.c`，画出 5ms/10ms/20ms 控制节拍表。
- 对照看 `code/calculate/`、`code/navigation/` 的被调用函数。

### 第三天

- 按你负责的方向深挖：
  - 任务策略：`code/plan/`
  - 控制算法：`code/calculate/` + `code/servo/`
  - 通信与调试：`code/tools/`

---

## 6. 一张“脑图式”目录简图

```text
repo
├─ user/                      # 入口与中断调度
│  ├─ main_cm7_0.c
│  ├─ cm7_0_isr.c
│  ├─ main_cm7_1.c
│  └─ cm7_1_isr.c
├─ code/                      # 业务实现
│  ├─ zf_common_headfile.h    # 公共头（汇总各模块）
│  ├─ common.h
│  ├─ config/                 # 配置层
│  ├─ calculate/              # 算法层
│  ├─ navigation/             # 导航层
│  ├─ servo/                  # 执行控制层
│  ├─ plan/                   # 任务策略层
│  └─ tools/                  # 工具与外设层
└─ docs/                      # 文档
```

---

如果你愿意，我下一步可以再补一份《`cm7_0_isr.c` 控制节拍详解表》，把每个 `loop_counter % N == 0` 的任务列成可维护的清单。
