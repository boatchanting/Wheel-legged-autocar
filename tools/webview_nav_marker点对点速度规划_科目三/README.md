# 科目三点对点速度规划打点 WebView 上位机

本目录是科目三专用副本，对应 `plan3_point_speed_planning` 的逐点导航方案。

## 运行上位机

```bash
python tools/webview_nav_marker点对点速度规划_科目三/nav_marker_host.py
```

默认监听：`192.168.137.1:8086`

## 功能

- 实时显示惯导轨迹（XY）
- 地图支持缩放、平移
- 支持跟踪视角、自适应视角（全图 fit）
- 下位机 `mark_trigger=1` 时自动打点
- 点支持新增、删除、拖拽位移、类型编辑（0~5）
- 特殊点支持记录 `relative_yaw`
- 导出 CSV：`total_count,start_heading,index,x,y,relative_yaw,heading,point_type`

## 生成静态 C 点表（无需 Flash）

将导出的 CSV 转为 `nav_replay` 可直接使用的 C 点表头文件：

```bash
python tools/webview_nav_marker点对点速度规划_科目三/csv_to_nav_table.py <你的csv路径>
```

不传路径时，会自动使用 `tools/webview_nav_marker点对点速度规划_科目三/` 下最新的 `nav_mark_points_*.csv`。

输出文件：

- `code/navigation/nav_replay_route_table.h`

项目已改为静态点表复现模式，复现时不再依赖 `NavFlash_ReadFlashToRam()`。

## 使用说明

- 普通路径点：状态机只要求到点，进入通过半径后直接切下一点。
- 特殊点：状态机要求到点且对准 `target_yaw_deg`，然后再触发对应动作。
- 本方案为在线速度规划，不需要 `chazhi.py` 离线插值与速度打表脚本。

## 协议字段扩展

下位机在原有 payload 末尾新增：

- `mark_trigger` (`uint8`)
- `point_type` (`uint8`)

兼容说明：上位机同时兼容旧 payload（84 字节）和新 payload（86 字节）。
