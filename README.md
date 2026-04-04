# TongjiCar1 智能轮腿车控制系统

![Build](https://img.shields.io/badge/build-manual%20verify-blue)
![Version](https://img.shields.io/badge/version-0.1.0-informational)
![License](https://img.shields.io/badge/license-GPL--3.0-green)
![Language](https://img.shields.io/badge/language-C%20%7C%20Python-orange)

> 面向智能车竞赛场景的双核嵌入式控制工程，集成姿态解算、导航记录与回放、任务策略执行、外设通信与调参可视化工具。

---

## 简介

`TongjiCar1` 是基于 CYT4BB 平台与逐飞开源库构建的智能轮腿车控制项目。工程采用「`user/` 调度层 + `code/` 功能层 + `tools/` 上位机分析工具 + `docs/` 设计文档」的组织方式，支持从底盘平衡控制到赛道任务执行的完整开发流程。

该项目重点覆盖以下典型竞赛需求：

- 双核任务分工与中断节拍调度（CM7_0 / CM7_1）
- IMU + EKF 的姿态估计与导航状态更新
- 路径点记录（RAM/Flash）与轨迹回放
- 单边桥、地雷区等场景化任务策略
- WiFi/SBUS/串口等调试与通信链路

## 特性

- ✅ **双核架构**：`user/main_cm7_0.c` 与 `user/main_cm7_1.c` 分离实时控制与并行任务。
- ✅ **导航能力**：提供惯导、GNSS 坐标转换、轨迹录制/存储/回放模块。
- ✅ **控制算法**：内置 PID、EKF、矩阵运算与执行器控制链路。
- ✅ **任务策略层**：支持桥、雷区等竞赛元素的策略化执行。
- ✅ **调试生态**：包含 WiFi 协议、菜单、蜂鸣器、Flash 工具及 Python 可视化脚本。

## 技术栈

| 类别 | 技术选型 |
| --- | --- |
| 嵌入式语言 | C (Cortex-M7, CYT4BB) |
| 上位机与分析 | Python、HTML 可视化页面 |
| 核心算法 | EKF、PID、矩阵运算、惯导融合 |
| 通信与外设 | UART、WiFi、SBUS、Flash、IPS 屏幕 |
| 工程组织 | `user/`、`code/`、`tools/`、`docs/` 分层 |

## 快速开始

### 环境要求

- CYT4BB 对应 SDK / 逐飞库（与工程头文件保持一致）
- IAR Embedded Workbench（仓库注释中使用 IAR 9.40.1）
- Python 3.9+（用于 `tools/` 下的可视化与离线分析脚本）

### 安装步骤

```bash
# 1) 克隆仓库
git clone <your-repo-url>
cd tongjicar1

# 2) 使用 IAR 打开工程（按你的本地工程文件路径）
# 3) 配置硬件与系统选项（见 code/config/sys_options.h）
```

### 编译与烧录

```text
1. 在 IAR 中选择目标工程与编译配置。
2. 根据硬件版本检查 IMU、遥控器、WIFI 开关配置。
3. 编译通过后下载至开发板。
4. 上电后通过串口/屏幕观察初始化日志。
```

> 提示：比赛前建议关闭高频日志开关（如 `DEBUG_LOG_ENABLE`）以减少性能开销。

## 用法示例

### 1) 切换系统功能开关（嵌入式侧）

```c
// code/config/sys_options.h
#define WIFI_USE 0
#define DEBUG_DISPLAY 1
#define REMOTE_CONTROL 1
#define IMU_CATEGORY 3
```

### 2) 运行 Python 可视化工具（上位机侧）

```bash
python tools/pid调参.py
python tools/gnss小操场数据可视化.py
```

### 3) 典型调试路径

1. 先检查 `user/main_cm7_0.c` 初始化流程（时钟、串口、IMU、EKF、控制器）。
2. 再阅读 `user/cm7_0_isr.c` 理解控制周期任务。
3. 按模块深入 `code/navigation/`、`code/calculate/`、`code/plan/`。

## 项目结构（可选）

```text
tongjicar1/
├─ user/                        # 双核入口与中断调度
│  ├─ main_cm7_0.c
│  ├─ cm7_0_isr.c
│  ├─ main_cm7_1.c
│  └─ cm7_1_isr.c
├─ code/                        # 嵌入式功能层
│  ├─ config/                   # 系统开关与车型配置
│  ├─ calculate/                # PID / EKF / Matrix
│  ├─ navigation/               # 惯导/GNSS/轨迹存储回放
│  ├─ servo/                    # 舵机与执行器控制
│  ├─ plan/                     # 任务策略（桥、雷区等）
│  ├─ tools/                    # beep/flash/wifi/sbus/menu
│  ├─ gps.c
│  └─ zf_common_headfile.h
├─ tools/                       # Python/HTML 调参与可视化工具
├─ docs/                        # 任务规划与模块文档
└─ README.md
```

## 贡献

欢迎通过 Issue / Pull Request 提交改进建议。

1. Fork 本仓库并创建分支：`git checkout -b feature/xxx`
2. 保持模块边界清晰（建议按 `code/` 子目录职责提交）
3. 补充必要文档与注释（特别是任务策略与参数调整依据）
4. 提交 PR 并说明测试场景、硬件版本与关键日志

## 许可证

本项目依赖并基于逐飞 CYT4BB 开源库生态，仓库内头文件注释显示采用 **GPL-3.0** 许可证体系。若你计划二次发布或商用，请先核验完整许可证文本与第三方依赖条款。

## 致谢

- 逐飞科技（SEEKFREE）及 CYT4BB 开源库生态
- 项目内文档与工具脚本贡献者
- 参与智能车竞赛调试与测试的同学和队友

---

如该项目对你有帮助，欢迎 Star ⭐ 并提交改进建议。
