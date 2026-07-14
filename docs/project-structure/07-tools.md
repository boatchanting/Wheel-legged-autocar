# tools/ - PC 端调试工具

## 概述

`tools/` 目录包含 PC 端的调试、仿真、可视化和算法验证工具，主要使用 Python 和 HTML 开发。用于离线数据分析、算法原型验证、传感器标定和上位机调试。

## 目录结构

```text
tools/
├─ README.md                              # 工具总览说明
├─ 01_导航与定位可视化/                    # GNSS/惯导轨迹可视化
├─ 02_导航算法分析/                        # 坐标对齐、回环检测、路径规划
├─ 03_控制与仿真/                          # PID 调参、轮腿仿真、MPC/RL 仿真
├─ 04_传感器标定与测试/                    # 磁力计、逆透视、图传压缩测试
├─ 05_通用数据处理工具/                    # 代码量统计、changelog、视频上位机
├─ 06_算法原理动画/                        # 五连杆、惯导等算法演示
├─ 07_针对小车车载视频的cv算法/            # 车载视频 CV 算法原型
├─ cvtest/                                 # 桥、地雷、台阶场景离线 CV 测试
├─ webview_gps_marker/                     # GPS 导航点标注工具
├─ webview_nav_marker/                     # 导航点标注工具
├─ webview_nav_marker速度规划/             # 速度规划版导航标注工具
├─ webview_nav_marker速度规划_科目二/       # 科目二专用速度规划工具
└─ wifi_protocol/                          # WiFi 上位机页面与 Streamlit 调试
```

## 01_导航与定位可视化/

用于 GNSS 和惯性导航轨迹的可视化分析。

| 文件 | 作用 |
|------|------|
| `gnss数据可视化.html` | GNSS 原始数据轨迹可视化 |
| `gnss和惯性导航数据可视化.html` | GNSS 与惯导融合轨迹对比 |
| `gnss数据结合惯性导航数据.html` | GNSS 与惯导融合结果可视化 |
| `惯性导航出图.html` | 惯性导航轨迹可视化 |
| `双文件xy坐标可视化.html` | 双文件 XY 坐标对比可视化 |
| `gnss小操场测试数据可视化.html` | 小操场 GNSS 测试数据可视化 |
| `gnss小操场数据可视化.py` | Python 版 GNSS 数据可视化 |
| `inertial_nav结果可视化.py` | 惯导结果 Python 可视化 |
| `绘图xy.py` | XY 坐标绘图工具 |

## 02_导航算法分析/

导航算法的分析和对比工具。

| 文件 | 作用 |
|------|------|
| `纯gnss的过滤算法.html` | GNSS 过滤算法可视化 |
| `导航路径规划的两种方案对比.html` | 路径规划方案对比分析 |
| `惯性导航融合加速度.py` | 惯导加速度融合分析 |
| `回环检测尝试.py` | 回环检测算法原型 |
| `streamlit自动对齐坐标系.py` | Streamlit 坐标系自动对齐工具 |

## 03_控制与仿真/

控制算法调参和仿真工具。

| 文件/目录 | 作用 |
|------|------|
| `pid调参.py` | PID 参数在线调参辅助工具 |
| `轮腿小车仿真.py` | 轮腿机构运动学仿真 |
| `nav_control/` | 导航控制仿真模块，包含 Pure Pursuit、MPC、强化学习仿真 |

## 04_传感器标定与测试/

传感器标定和功能测试工具。

| 文件 | 作用 |
|------|------|
| `磁力计校准与3d可视化.html` | 磁力计校准数据 3D 可视化 |
| `磁力计校准与校准测试工具.html` | 磁力计校准流程和测试 |
| `逆透视标定.html` | 摄像头逆透视矩阵标定工具 |
| `压缩图传.py` | 图传压缩算法测试 |
| `重力加速度统计测试.py` | IMU 重力加速度统计测试 |

## 05_通用数据处理工具/

通用辅助工具。

| 文件/目录 | 作用 |
|------|------|
| `代码量统计.py` | 项目代码量统计分析 |
| `对比文件.html` | 文件内容对比工具 |
| `changelog_between_tags.py` | Git tag 间 changelog 生成（GitHub Actions 调用） |
| `视频网页上位机/` | 视频流网页上位机（实时显示） |

## 06_算法原理动画/

算法原理的可视化动画演示。

| 文件 | 作用 |
|------|------|
| `五连杆解算.py` | 五连杆机构运动学解算动画 |
| `五连杆解算实时逆解.py` | 五连杆实时逆解动画 |
| `惯性导航.py` | 惯性导航原理动画演示 |

## 07_针对小车车载视频的cv算法/

车载视频的计算机视觉算法原型。

| 目录 | 作用 |
|------|------|
| `3stages/` | 三阶段场景 CV 算法原型 |
| `操场/` | 操场场景 CV 算法 |
| `bev/` | 鸟瞰图（BEV）变换算法 |
| `bridge/` | 单边桥场景 CV 算法 |
| `bridge2/` | 单边桥场景 CV 算法（第二版） |
| `bumpy_road/` | 颠簸路场景 CV 算法 |

## cvtest/ - 离线 CV 测试

离线计算机视觉测试场景。

| 目录 | 作用 |
|------|------|
| `bridge/` | 单边桥场景离线 CV 测试 |
| `minefield/` | 地雷区场景离线 CV 测试 |
| `stage/` | 台阶场景离线 CV 测试 |

## webview_gps_marker/ - GPS 导航点标注

GPS 导航点标注和路线生成工具。

| 文件 | 作用 |
|------|------|
| `gps_marker_host.py` | GPS 标注上位机（TCP 服务端） |
| `gps_marker.html` | GPS 标注 WebView 界面 |
| `mock_log.csv` | 模拟设备日志数据 |
| `chazhi.py` | 插值算法 |
| `csv_to_gps_nav_table.py` | CSV 转 GPS 导航路线表 |
| `README.md` | 使用说明 |

## webview_nav_marker/ - 导航点标注

导航点标注和路线生成工具。

| 文件 | 作用 |
|------|------|
| `nav_marker_host.py` | 导航标注上位机（TCP 服务端） |
| `nav_marker.html` | 导航标注 WebView 界面 |
| `chazhi.py` | 插值算法 |
| `csv_to_nav_table.py` | CSV 转导航路线表 |
| `README.md` | 使用说明 |

## webview_nav_marker速度规划/ - 速度规划导航标注

带速度规划的导航点标注工具。

| 文件 | 作用 |
|------|------|
| `caculate_path.py` | **核心**：自动轨迹解算、速度规划 |
| `nav_marker_host.py` | 导航标注上位机 |
| `nav_marker.html` | 导航标注 WebView 界面 |
| `chazhi.py` | 插值算法 |
| `csv_to_nav_table.py` | CSV 转导航路线表 |
| `test_fast_uturn_path.py` | 快速 U 型弯路径测试 |
| `README.md` | 使用说明 |

## webview_nav_marker速度规划_科目二/ - 科目二速度规划

科目二专用的速度规划工具。

| 文件 | 作用 |
|------|------|
| `nav_marker_host.py` | 科目二导航标注上位机 |
| `nav_marker.html` | 科目二导航标注 WebView 界面 |
| `chazhi.py` | 插值算法 |
| `csv_to_nav_table.py` | CSV 转导航路线表 |
| `README.md` | 使用说明 |

## wifi_protocol/ - WiFi 上位机

WiFi 通信协议调试和可视化工具。

| 文件 | 作用 |
|------|------|
| `wifi_host.py` | WiFi 上位机主程序 |
| `streamlit_wifi.py` | Streamlit 版 WiFi 调试界面 |
| `index.html` | WiFi 上位机主页面 |
| `navigation.html` | 导航数据可视化页面 |
| `visualization/` | 可视化工具目录 |

## 依赖环境

运行 tools/ 下的 Python 脚本通常需要：

- Python 3.9+
- numpy、pandas、matplotlib
- opencv-python（部分 CV 工具）
- streamlit（部分上位机工具）
- 浏览器（打开 HTML 可视化页面）

> 注意：当前仓库没有统一的 `requirements.txt`，请按报错逐项安装依赖。
