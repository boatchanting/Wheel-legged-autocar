# 惯导打点 WebView 上位机

## 加速刹车采集器

独立采集器入口：

```bash
python tools/webview_nav_marker速度规划/加速刹车采集器_webview.py
```

也可以双击：

- `tools/webview_nav_marker速度规划/start_accel_brake_collector.bat`

功能：

- 设置目标前进速度，单位为 `mm/s`，输入正数即可。
- 设置刹车阶段目标速度，单位为 `mm/s`，支持有符号值：正数表示继续前进方向，`0` 表示刹停目标，负数表示反向刹车目标。
- 上位机会读取 `code/navigation/inertial_nav.h` 中当前车型的 `SPEED_TO_MM_S`，把前进速度自动换算成车端速度命令；例如当前 `SPEED_TO_MM_S=4.79` 时，`479 mm/s` 会下发约 `-100` 的车端速度命令。
- 选择是否启用多预设 PID：加速阶段 `ACCEL`，保持阶段 `NORMAL`，刹车阶段 `BRAKE`；关闭时全程 `NORMAL`。
- 点击开始后立刻下发换算后的目标速度命令，实测 `|vx_body|` 连续达到目标前进速度的 95% 以上并保持 0.5s 后，再进入刹车阶段；保持段如果速度跌回阈值以下，会重新回到加速段计时。
- 多 PID 模式下，加速段必须下发 `ACCEL`，连续达速保持段会把同一个速度目标重新下发为 `NORMAL`，刹车段必须下发 `BRAKE`；关闭多 PID 时三段都使用 `NORMAL`。
- 刹车阶段会周期性重复下发自定义刹车目标速度；默认 `0`，也可以填负数作为反向刹车命令。刹车目标为 `0` 或负数时，实验仍按实际刹停自动结束，并在结束时补发 `0` 防止继续倒走；正数刹车目标则按降到该前进速度附近自动结束。
- 自动记录加速、保持和刹车阶段日志；正常刹停会以 `completed` 收尾，目标速度达不到、刹车超时或遥测断流会以 `failed` 收尾并关闭本次日志。
- 日志输出到 `tools/webview_nav_marker速度规划/brake_logs/`。
- 每次点击开始都会创建一个独立日志名；即使同一秒内重复实验，也会自动追加 `_001`、`_002` 这类序号，避免覆盖旧日志。
- 上位机按 `10ms` 采集周期拉取新数据；CSV 每一帧都会写 `frame_elapsed_ms`，表示本次点击开始后的相对计时。`host_sample_period_ms=10` 只在本次日志第一帧写入，以后只有刷新率发生变化时才再次写入。实际能写到多少行仍取决于下位机遥测是否按 10ms 稳定到达。

开始按钮/API 返回字段：

- `success`：是否成功开始。
- `msg`：开始结果说明，包含 `mm/s -> 车端速度命令` 的换算。
- `phase`：开始后的阶段，正常为 `accel`。
- `run_id`：本次实验编号。
- `csv_path`：本次逐帧日志路径，开始时已创建并写入表头。
- `json_path`：本次摘要日志路径，自动结束时写入。
- `target_forward_speed_mm_s`：目标前进速度。
- `vehicle_speed_cmd`：实际下发给车端的速度命令。
- `brake_target_speed_mm_s`：刹车阶段目标速度，负数表示反向速度。
- `brake_vehicle_speed_cmd`：刹车阶段实际下发给车端的速度命令。
- `speed_to_mm_s`：本次使用的换算系数。
- `host_sample_period_ms`：上位机设定的数据采集周期，当前固定为 `10`。
- `frame_elapsed_ms`：本次点击开始后的逐帧计时，单位 `ms`。
- `run`：底层 logger 的原始开始信息，保留兼容。

每次实验输出：

- `accel_brake_*.csv`：逐帧数据。
- `accel_brake_*.json`：本次摘要。
- `brake_summary.csv`：所有实验的汇总表。

采集字段包含惯导位置、车身速度、速度绝对值、由速度差分得到的加速度、阶段时间、阶段距离、加速距离、刹车距离、目标前进速度、刹车目标速度、车端速度命令、刹车车端命令、PID 模式、左右轮速度、理论/实际 yaw rate、姿态角、处理后的三轴陀螺仪、低通加速度和重力分量。

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
