# iar/ - IAR 工程配置

## 概述

`iar/` 目录包含 IAR Embedded Workbench 的工程配置文件、链接脚本和构建输出目录。项目使用 IAR 9.40.1 开发，支持 CYT4BB 双核（CM7_0 + CM7_1）编译。

## 目录结构

```text
iar/
├─ cyt4bb7.eww                    # IAR 工作空间文件（入口）
├─ 删除临时文件IAR.bat            # 清理 IAR 临时文件的批处理脚本
├─ icf/
│  └─ linker_directives_tviibh.icf  # 链接脚本
├─ project_config/
│  ├─ cyt4bb7_cm_7_0.ewp         # 0 核工程配置
│  ├─ cyt4bb7_cm_7_1.ewp         # 1 核工程配置
│  ├─ cyt4bb7_cm_7_0.ewd         # 0 核调试配置
│  ├─ cyt4bb7_cm_7_1.ewd         # 1 核调试配置
│  ├─ cyt4bb7_cm_7_0.ewt         # 0 核工具配置
│  ├─ cyt4bb7_cm_7_1.ewt         # 1 核工具配置
│  ├─ cyt4bb7_cm_7_0.eww         # 0 核工作空间
│  ├─ cyt4bb7_cm_7_1.eww         # 1 核工作空间
│  ├─ cyt4bb7_cm_7_0.ewx         # 0 核扩展配置
│  ├─ cyt4bb7_cm_7_0.sim         # 0 核仿真配置
│  ├─ cm7_0_cm7_1_debug.xml      # 双核调试配置
│  ├─ Debug_m7_0/                # 0 核 Debug 构建输出（含 .hex/.out）
│  ├─ Debug_m7_1/                # 1 核 Debug 构建输出（含 .hex/.out）
│  └─ settings/                  # IAR 调试设置文件（.crun/.dbgdt/.cspy 等）
├─ settings/
│  └─ cyt4bb7.wsdt               # IAR 工作空间调试设置
├─ Debug_m7_0/                   # 0 核 Debug 构建输出
└─ Debug_m7_1/                   # 1 核 Debug 构建输出
```

## 关键文件说明

### cyt4bb7.eww - 工作空间文件

IAR 工作空间入口文件，包含双核工程的配置引用。

### icf/linker_directives_tviibh.icf - 链接脚本

IAR 链接器配置文件，定义：
- 内存布局（Flash/RAM 地址和大小）
- 中断向量表位置
- 堆栈大小
- 段分配

### project_config/cyt4bb7_cm_7_0.ewp - 0 核工程

0 核（CM7_0）的 IAR 工程配置，包含：
- 源文件列表（code/、user/main_cm7_0.c、user/cm7_0_isr.c）
- Include 路径配置
- 预处理宏定义
- 编译器和链接器选项
- 调试器配置

### project_config/cyt4bb7_cm_7_1.ewp - 1 核工程

1 核（CM7_1）的 IAR 工程配置，包含：
- 源文件列表（code1/、user/main_cm7_1.c、user/cm7_1_isr.c）
- Include 路径配置
- 预处理宏定义
- 编译器和链接器选项
- 调试器配置

### 删除临时文件IAR.bat - 清理脚本

Windows 批处理脚本，用于清理 IAR 编译产生的临时文件（Obj、List、BrowseInfo 等）。

## 编译流程

1. **打开工作空间**：在 IAR 中打开 `iar/cyt4bb7.eww`
2. **选择工程**：在工作空间中选择 `cyt4bb7_cm_7_0.ewp`（0 核）或 `cyt4bb7_cm_7_1.ewp`（1 核）
3. **配置构建**：选择 Debug 或 Release 配置
4. **编译**：Project → Make 或 Rebuild All
5. **下载烧录**：先下载 0 核，再下载 1 核
6. **调试**：使用 IAR 调试器或 DAP 下载器

## Include 路径配置

IAR 工程需要配置以下 Include 路径：

```
code/
code1/
user/
libraries/zf_common/
libraries/zf_driver/
libraries/zf_device/
libraries/zf_components/
libraries/sdk/common/hdr/
libraries/sdk/tviibh4m/hdr/
```

## 常见问题

### Q: 找不到 zf_common_headfile.h？
检查 Include 路径是否包含 `libraries/zf_common/`

### Q: 找不到 cy_project.h？
检查 Include 路径是否包含 `libraries/sdk/tviibh4m/hdr/`

### Q: 双核下载顺序？
先下载 0 核，再下载 1 核。两个核的固件需要分别编译和下载。

### Q: 如何单独更新某个核？
在 IAR 中只编译和下载对应核的工程，但需要注意 `.gitignore` 中已排除部分配置文件以防止冲突。
