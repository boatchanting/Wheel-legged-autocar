# GPS 打点 WebView 上位机

## 运行

```bash
python tools/webview_gps_marker/gps_marker_host.py
```

默认监听：`192.168.137.1:8086`

## 功能

- 实时显示下位机投影后的 GPS 平面轨迹
- 兼容下位机 `mark_trigger=1` 自动打点
- 支持手动加点、删点、拖点、改点类型
- 导出 CSV：`total_count,index,x,y,point_type`

## 生成纯 GPS 静态点表

```bash
python tools/webview_gps_marker/csv_to_gps_nav_table.py <你的csv路径>
```

不传路径时，会自动使用 `tools/webview_gps_marker/` 下最新的 `gps_mark_points_*.csv`。  
输出文件：
- `code/navigation/gps_nav_replay_route_table.h`

## 使用约束

- 这套方案是“纯 GPS 打点 + 纯 GPS 复刻”，不依赖惯导坐标和陀螺航向。
- 复刻前需要把车重新放回录制时起点附近，再触发 `g_replay_start_request = 1`。
- 纯 GPS 复刻启动后会重置 `gnss_trans` 原点，因此起步前应保持车辆静止并等待 GNSS 有效。
