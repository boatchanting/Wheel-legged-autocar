# 科目二速度规划打点 WebView 上位机

本目录是科目二专用副本，和科目一/通用 `tools/webview_nav_marker速度规划` 分开维护。

## 运行上位机

```bash
python tools/webview_nav_marker速度规划_科目二/nav_marker_host.py
```

默认监听：`192.168.137.1:8086`

## 功能

- 实时显示惯导轨迹（XY）
- 地图支持缩放、平移
- 支持跟踪视角、自适应视角（全图 fit）
- 下位机 `mark_trigger=1` 时自动打点
- 点支持新增、删除、拖拽位移、类型编辑（0~5）
- 导出 CSV：`total_count,start_heading,index,x,y,relative_yaw,heading,point_type`

## 生成静态 C 点表（无需 Flash）

将导出的 CSV 转为 `nav_replay` 可直接使用的 C 点表头文件：

```bash
python tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py <你的csv路径>
```

不传路径时，会自动使用 `tools/webview_nav_marker速度规划_科目二/` 下最新的 `nav_mark_points_*.csv`。

输出文件：

- `code/navigation/nav_replay_route_table.h`

项目已改为静态点表复现模式，复现时不再依赖 `NavFlash_ReadFlashToRam()`。

## 插值与离线速度规划

基于 `code/navigation/nav_replay_route_table.h` 已有点表做插值、曲率限速、加减速扫描，并回写 6 字段路表：

```bash
python tools/webview_nav_marker速度规划_科目二/chazhi.py --method 4 --no-plot
```

默认输出：

- 轨迹头文件：`code/navigation/nav_replay_route_table.h`
- 点格式：`{x, y, target_yaw_deg, heading_deg, (uint8)point_type, target_speed}`
- 默认速度参数偏保守，优先保证雷区进框停车稳定。

## 协议字段扩展

下位机在原有 payload 末尾新增：

- `mark_trigger` (`uint8`)
- `point_type` (`uint8`)

兼容说明：上位机同时兼容旧 payload（84 字节）和新 payload（86 字节）。
