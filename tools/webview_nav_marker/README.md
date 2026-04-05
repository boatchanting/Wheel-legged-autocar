# 惯导打点 WebView 上位机

## 运行上位机

```bash
python tools/webview_nav_marker/nav_marker_host.py
```

默认监听：`192.168.137.1:8086`

## 功能

- 实时显示惯导轨迹（XY）
- 地图支持缩放、平移
- 支持跟踪视角、自适应视角（全图 fit）
- 下位机 `mark_trigger=1` 时自动打点
- 点支持新增、删除、拖拽位移、类型编辑（0~5）
- 导出 CSV：`total_count,index,x,y,point_type`

## 生成静态 C 点表（无需 Flash）

将导出的 CSV 转为 `nav_replay` 可直接使用的 C 点表头文件：

```bash
python tools/webview_nav_marker/csv_to_nav_table.py <你的csv路径>
```

不传路径时，会自动使用 `tools/webview_nav_marker/` 下最新的 `nav_mark_points_*.csv`。

输出文件：

- `code/navigation/nav_replay_route_table.h`

项目已改为静态点表复现模式，复现时不再依赖 `NavFlash_ReadFlashToRam()`。

## 三次样条离线重采样（20mm 点距）

基于 `code/navigation/nav_replay_route_table.h` 已有点表做三次样条插值，按弧长每 20mm 取一点，生成可视化并回写头文件：

```bash
python tools/webview_nav_marker/spline_interpolate.py --interval 20
```

默认输出：

- 轨迹头文件：`code/navigation/nav_replay_route_table.h`
- 预览图：`tools/webview_nav_marker/nav_replay_spline_preview.png`

## 协议字段扩展

下位机在原有 payload 末尾新增：

- `mark_trigger` (`uint8`)
- `point_type` (`uint8`)

兼容说明：上位机同时兼容旧 payload（84 字节）和新 payload（86 字节）。
