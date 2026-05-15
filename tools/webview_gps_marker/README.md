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

## 互补滤波打点上位机

运行：

```bash
python tools/webview_gps_marker/cf_marker_host.py
```

功能：

- 实时显示互补滤波输出的平面轨迹
- 保留惯导上位机的自动打点、手动增删点、拖点、点类型编辑、开始发车
- 导出 CSV：`total_count,start_heading,index,x,y,relative_yaw,heading,point_type`

生成互补滤波静态点表：

```bash
python tools/webview_gps_marker/csv_to_cf_nav_table.py <你的csv路径>
```

不传路径时，会自动使用 `tools/webview_gps_marker/` 下最新的 `cf_mark_points_*.csv`。

输出文件：
- `code/navigation/cf_nav_replay_route_table.h`

对 `cf_nav_replay_route_table.h` 做插值平滑：

```bash
python tools/webview_gps_marker/cf_chazhi.py
```

推荐流程：

1. `python tools/webview_gps_marker/cf_marker_host.py`
2. 导出 `cf_mark_points_*.csv`
3. 在 `code/config/sys_options.h` 里手动填写 `CF_MANUAL_LAUNCH_HEADING_DEG`
4. `python tools/webview_gps_marker/csv_to_cf_nav_table.py`
5. `python tools/webview_gps_marker/cf_chazhi.py`

说明：

- `cf_chazhi.py` 会直接读取并回写 `code/navigation/cf_nav_replay_route_table.h`
- 这一步适合在已经完成初始打点后，再做离线路径平滑与重采样
- `csv_to_cf_nav_table.py` 和 `cf_chazhi.py` 都会读取 `CF_MANUAL_LAUNCH_HEADING_DEG`
- CF 方案默认不依赖磁力计 `start_heading`，点表里写入的是手动配置的发车绝对航向

## 使用约束

- 这套方案是“纯 GPS 打点 + 纯 GPS 复刻”，不依赖惯导坐标和陀螺航向。
- 复刻前需要把车重新放回录制时起点附近，再触发 `g_replay_start_request = 1`。
- 纯 GPS 复刻启动后会重置 `gnss_trans` 原点，因此起步前应保持车辆静止并等待 GNSS 有效。
