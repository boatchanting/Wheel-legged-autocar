# Wheel-legged-autocar 工程目录文档

> 面向新加入开发者的项目结构指南，解释每个文件夹和文件的内容与作用。

## 项目概述

Wheel-legged-autocar 是基于逐飞 CYT4BB 开源库、Cortex-M7 双核平台和 IAR Embedded Workbench 的智能轮腿车控制系统。项目使用双核架构：
- **0 核 (CM7_0)**：负责整车控制、平衡、导航、任务状态机
- **1 核 (CM7_1)**：负责摄像头图像处理、视觉识别、图传

## 目录文档索引

| 文档 | 内容 |
|------|------|
| [01-root-config.md](./01-root-config.md) | 根目录文件和顶层配置 |
| [02-libraries.md](./02-libraries.md) | 逐飞开源库、芯片 SDK、外设驱动 |
| [03-user.md](./03-user.md) | 双核程序入口与中断调度 |
| [04-code.md](./04-code.md) | 0 核工程：控制、导航、视觉、工具 |
| [05-code1.md](./05-code1.md) | 1 核工程：视觉算法、图传 |
| [06-project-build.md](./06-project-build.md) | IAR 工程配置、链接脚本 |
| [07-tools.md](./07-tools.md) | PC 端调试、仿真、可视化工具 |
| [08-docs.md](./08-docs.md) | 项目文档、设计说明、任务规划 |
| [09-cyt2bl3foc.md](./09-cyt2bl3foc.md) | CYT2BL3 无刷双驱电机驱动器 |

## 快速导航

- **想了解项目整体架构？** → 阅读根目录 `README.md`
- **想了解某个模块功能？** → 查看对应目录的详细文档
- **想开始开发？** → 从 `03-user.md` 了解程序入口，然后看 `04-code.md` 了解业务逻辑
- **需要调试工具？** → 查看 `07-tools.md`

## 项目核心文件树

```text
project/
├─ README.md                        # 项目主文档（架构、模块、FAQ）
├─ 双轮足并联腿机器人结构与控制架构说明.md  # 双轮足机器人整体架构
├─ 双轮足打滑检测逻辑问题分析与修改方案.md  # 打滑检测问题分析
├─ analyze_slip_logs.py             # 打滑检测日志分析脚本
├─ user/                            # 双核入口与中断调度
├─ code/                            # 0 核工程（控制、导航、视觉控制）
├─ code1/                           # 1 核工程（视觉算法、图传）
├─ libraries/                       # 逐飞 CYT4BB 开源库
├─ CYT2BL3FOC/                      # CYT2BL3 无刷双驱电机驱动器
├─ iar/                             # IAR 工程配置
├─ tools/                           # PC 端调试工具
├─ docs/                            # 项目文档
├─ .vscode/                         # VS Code 配置
├─ .agents/                         # Agent 配置（空）
└─ .github/                         # GitHub Actions 工作流
```
