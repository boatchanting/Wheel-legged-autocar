# 根目录文件与顶层配置

## 概述

根目录包含项目配置文件、版本控制文件和一些顶层文档。

## 文件说明

### 版本控制与配置

| 文件 | 作用 |
|------|------|
| `.git/` | Git 仓库数据 |
| `.gitignore` | Git 忽略规则：排除编译产物、IDE 缓存、Python 虚拟环境、CSV 数据、IAR 构建输出等 |
| `.github/` | GitHub Actions 工作流配置 |
| `.vscode/` | VS Code 编辑器配置 |
| `.agents/` | Agent 配置目录（当前为空） |

### 项目文档

| 文件 | 作用 |
|------|------|
| `README.md` | 项目主文档：架构概述、目录结构、模块说明、环境依赖、安装步骤、FAQ、开发建议 |
| `双轮足并联腿机器人结构与控制架构说明.md` | 双轮足并联腿机器人整体结构与控制架构说明 |
| `双轮足打滑检测逻辑问题分析与修改方案.md` | 双轮足打滑检测逻辑的问题分析与修改方案 |
| `analyze_slip_logs.py` | 打滑检测日志分析脚本 |

## .vscode/ 目录

| 文件 | 作用 |
|------|------|
| `settings.json` | VS Code 基础设置：启用 Codegeex 仓库索引、Python 环境配置 |
| `iar-vsc.json` | IAR-VSC 扩展配置：指向 IAR 工作空间文件，配置双核工程的 Debug 配置 |

## .github/ 目录

| 文件 | 作用 |
|------|------|
| `workflows/changelog-between-tags.yml` | GitHub Actions 工作流：在两个 Git tag 之间自动生成 changelog，调用 `tools/05_通用数据处理工具/changelog_between_tags.py` |

## .gitignore 排除规则摘要

```gitignore
# IDE 缓存
.vscode/
.vs/

# 数据文件
data/
*.csv
.venv/

# IAR 构建产物
iar/Debug*/
iar/Release*/
iar/**/Obj/
iar/**/List/
iar/**/settings/
iar/project_config/Debug_m7_0/
iar/project_config/Debug_m7_1/

# CYT2BL3FOC 构建产物
CYT2BL3FOC/project/iar/settings/
CYT2BL3FOC/project/iar/project_config/settings/
CYT2BL3FOC/project/iar/Debug_m4/

# Python 缓存
__pycache__/
*.pyc
```
