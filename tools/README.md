# tools 工具目录说明

本目录用于存放项目开发中用到的脚本、小型可视化页面和实验工具。已按用途进行分门别类，便于查找和维护。

## 目录结构

- `01_导航与定位可视化/`：GNSS、惯导、轨迹与坐标可视化工具
- `02_导航算法分析/`：导航算法验证、融合分析、路径方案对比工具
- `03_控制与仿真/`：控制算法调参与车辆/控制仿真工具
- `04_传感器标定与测试/`：磁力计、重力加速度等传感器标定与测试工具
- `05_通用数据处理工具/`：通用辅助工具（代码量统计、文件对比等）
- `cvtest/`：视觉识别实验工具（单边桥、地雷区、台阶）
- `wifi_protocol/`：WiFi 协议通信与可视化相关工具

---

## 各类工具清单

### 1) `01_导航与定位可视化`
- `gnss数据可视化.html`：GNSS 数据可视化页面。
- `gnss小操场数据可视化.py`：小操场 GNSS 数据可视化脚本。
- `gnss小操场测试数据可视化.html`：小操场测试数据展示页面。
- `gnss和惯性导航数据可视化.html`：GNSS 与惯导数据联合可视化。
- `gnss数据结合惯性导航数据.html`：GNSS 与惯导融合结果可视化页面。
- `inertial_nav结果可视化.py`：惯导结果绘图/分析脚本。
- `惯性导航出图.html`：惯导数据图形展示页面。
- `双文件xy坐标可视化.html`：双文件 XY 坐标对比可视化。
- `绘图xy.py`：XY 坐标绘图脚本。

### 2) `02_导航算法分析`
- `纯gnss的过滤算法.html`：纯 GNSS 过滤算法效果展示。
- `导航路径规划的两种方案对比.html`：路径规划方案对比可视化。
- `回环检测尝试.py`：回环检测实验脚本。
- `惯性导航融合加速度.py`：惯导与加速度融合分析脚本。
- `streamlit自动对齐坐标系.py`：基于 Streamlit 的坐标系自动对齐工具。

### 3) `03_控制与仿真`
- `pid调参.py`：PID 参数调试脚本。
- `轮腿小车仿真.py`：轮腿小车仿真脚本。
- `nav_control/`：控制算法仿真子目录：
  - `pure_pursuit_simulator.py`：Pure Pursuit 仿真
  - `mpc_simulator.py`：MPC 控制仿真
  - `RL_simulator.py` / `RL2c.py`：强化学习控制仿真
  - `RL_simulator_visulisation.py`：强化学习仿真可视化
  - `仿真控制实验.txt`：仿真实验说明

### 4) `04_传感器标定与测试`
- `磁力计校准与3d可视化.html`：磁力计校准与 3D 可视化。
- `磁力计校准与校准测试工具.html`：磁力计校准测试页面。
- `重力加速度统计测试.py`：重力加速度统计测试脚本。

### 5) `05_通用数据处理工具`
- `代码量统计.py`：统计代码行数/代码量。
- `对比文件.html`：文件对比页面。

### 6) `cvtest`（视觉实验）
- `bridge/`：单边桥识别与场景工具。
- `minefield/`：地雷区识别、标定与坐标计算工具。
- `stage/`：台阶识别及调试工具。

### 7) `wifi_protocol`（通信协议）
- `wifi_host.py`：WiFi 主机侧工具脚本。
- `streamlit_wifi.py`：基于 Streamlit 的 WiFi 数据工具。
- `index.html`、`navigation.html`：协议/导航相关页面。
- `visualization/`：CSV、紧耦合、投影法等可视化工具。

---

## 使用建议
- 优先按目录用途查找，避免在根目录堆积脚本。
- 新增工具时，放入最匹配的分类目录，并在本文件补充用途说明。
- 对实验性质强的脚本建议附带简短 `*.md` 使用说明（输入数据格式、运行方式、输出说明）。

### 8) `05_通用数据处理工具/changelog_between_tags.py`（两个 Tag 间自动更新日志）
- 输入：`from_tag`、`to_tag`。
- 输出：按 commit 消息分组的 changelog，支持 `【功能】消息` 这类格式。
- 过滤：默认过滤 merge 和格式化提交（`fmt:` / `format:` / `style:` / `chore(format):`）。
- 可选：`--markdown` 直接输出可发布 Markdown。

示例：
```bash
# 本地一键（Markdown）
python3 tools/05_通用数据处理工具/changelog_between_tags.py v1.0.0 v1.1.0 --markdown

# 保留 merge/格式化提交
python3 tools/05_通用数据处理工具/changelog_between_tags.py v1.0.0 v1.1.0 --markdown --include-merge --include-format
```

CI 示例：
- 使用 `.github/workflows/changelog-between-tags.yml`，通过 `workflow_dispatch` 输入 `from_tag` 和 `to_tag` 后自动生成并上传 `changelog.txt`。
