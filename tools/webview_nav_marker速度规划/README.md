# 惯导打点 WebView 上位机

## 加速刹车采集器

独立采集器入口：

```bash
python tools/webview_nav_marker速度规划/accel_brake_collector_host.py
```

也可以双击：

- `tools/webview_nav_marker速度规划/start_accel_brake_collector.bat`

功能：

- 设置目标速度。
- 选择是否启用多预设 PID：加速阶段 `ACCEL`，保持阶段 `NORMAL`，刹车阶段 `BRAKE`；关闭时全程 `NORMAL`。
- 点击开始后立刻下发目标速度，达到目标速度后保持 0.5s，再下发 0 速刹车。
- 自动记录加速、保持和刹车阶段日志。
- 日志输出到 `tools/webview_nav_marker速度规划/brake_logs/`。

每次实验输出：

- `accel_brake_*.csv`：逐帧数据。
- `accel_brake_*.json`：本次摘要。
- `brake_summary.csv`：所有实验的汇总表。

采集字段包含惯导位置、车身速度、由速度差分得到的加速度、阶段时间、阶段距离、刹车距离、目标速度、PID 模式、左右轮速度、理论/实际 yaw rate、姿态角、处理后的三轴陀螺仪、低通加速度和重力分量。

下位机新增控制码：

- `WIFI_HOST_CTRL_SET_TARGET_SPEED = 0x20`
- payload：`uint8 control_id + float target_speed + uint8 pid_mode + uint8 flags`
- `pid_mode`：`0=NORMAL`，`1=ACCEL`，`2=BRAKE`

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
