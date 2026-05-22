---
name: iar编译命令
description: 使用 IAR Embedded Workbench 命令行工具编译并验证此逐飞（Seekfree）CYT4BB 双核项目。适用于运行 iarbuild、验证 CM7_0/CM7_1 Debug 版本编译、检查编译产物或解析此仓库的 IAR 编译器警告与错误。
---

# IAR 编译指南

## 默认上下文

- **项目根目录**：请在仓库根路径下执行操作：``。
- **编译工具**：使用 IAR 命令行编译工具：`D:\Softwares\iar\common\bin\iarbuild.exe`。
- **项目文件**：CYT4BB7 项目文件位于 `.\iar\project_config` 目录下。
- **路径规范**：在项目根目录下操作时，请使用 `.\iar\project_config\...` 相对路径，不要使用 `..\iar\project_config\...`。

## 编译命令

当代码更改可能影响固件行为时，请对两个内核执行编译：

```powershell
& "D:\Softwares\iar\common\bin\iarbuild.exe" ".\iar\project_config\cyt4bb7_cm_7_0.ewp" -make Debug
& "D:\Softwares\iar\common\bin\iarbuild.exe" ".\iar\project_config\cyt4bb7_cm_7_1.ewp" -make Debug
```

**编译成功标准：**

- 进程退出代码（Exit Code）为 `0`。
- IAR 报告 `Total number of errors: 0`。
- IAR 报告 `Build succeeded`。
- 生成了预期的产物文件，例如：`cyt4bb7_cm_7_0.out`、`cyt4bb7_cm_7_0.hex`、`cyt4bb7_cm_7_1.out` 以及 `cyt4bb7_cm_7_1.hex`。

## 沙盒环境说明

如果 `iarbuild.exe` 在沙盒内运行返回退出代码 `1` 且没有可见输出，请在取得用户同意后，在沙盒环境外重新运行相同的命令。在此环境下，非受限运行可以产生正常的 IAR 日志。

## 当前基准警告 (Baseline Warnings)

截至 2026-05-22，全量 Debug 编译成功，但存在以下已知警告：

- **CM7_0**: `code\navigation\gnss_transform.c(15)` 警告宏 `PI` 与 CMSIS DSP 的 `arm_math.h` 中的定义冲突（重定义）。
- **CM7_0**: `user\main_cm7_0.c(96)` 警告 `ekf_print_div` 已声明但从未被引用。
- **CM7_1**: `code1\vision\line_vision.c(64)` 警告 `g_line_last_frame_time_us` 已声明但从未被引用。
- **CM7_1**: `code1\vision\bumpy_vision.c(96)` 警告 `g_bumpy_last_frame_time_us` 已声明但从未被引用。
- **CM7_1**: `code1\vision\pvc_vision.c(65)` 警告 `g_pvc_last_frame_time_us` 已声明但从未被引用。

在报告未来的编译结果时，请将这些**既有的基准警告**与当前代码更改引入的**新警告**分开列出。