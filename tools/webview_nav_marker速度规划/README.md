# 惯导打点 WebView 上位机

## 加速刹车采集器

独立采集器入口：

```bash
python tools/webview_nav_marker速度规划/加速刹车采集器_webview.py
```

也可以双击：

- `tools/webview_nav_marker速度规划/start_accel_brake_collector.bat`

功能：

- 设置目标速度，单位为 `mm/s`，支持有符号值：正数表示前进，负数表示反向。
- 设置刹车阶段目标速度，单位为 `mm/s`，支持有符号值：正数表示继续前进方向，`0` 表示刹停目标，负数表示反向刹车目标。
- 上位机会读取 `code/navigation/inertial_nav.h` 中当前车型的 `SPEED_TO_MM_S`，把目标速度自动换算成车端速度命令；例如当前 `SPEED_TO_MM_S=4.79` 时，`+479 mm/s` 会下发约 `-100`，`-479 mm/s` 会下发约 `+100`。
- 选择是否启用多预设 PID：加速阶段 `ACCEL`，保持阶段 `NORMAL`，刹车阶段 `BRAKE`；关闭时全程 `NORMAL`。
- 点击开始后先让下位机进入 `WiFi直接速度测试状态机`，再下发换算后的目标速度命令。状态机运行期间不接受遥控器速度映射写入，避免 10ms 遥控器任务把上位机目标速度覆盖为 0；CH5 刹车和 CH6 总开关仍然保留最高优先级。
- 实测速度沿目标方向达到目标速度以上后，再检测最近 `0.15s` 的短窗口波动；短窗口内都不低于目标且 `max-min <= 150mm/s` 时，进入保持段。
- 如果目标速度设得偏高导致车速达不到目标，但车速已经超过目标的 `60%` 且至少 `300mm/s`，并在最近 `0.35s` 内进入稳定平台期，也会进入保持段作为兜底，避免一直跑不刹车。
- 保持段连续 `0.5s` 后进入刹车。目标达速触发的保持段如果速度跌回目标以下，会重新回到加速段；平台期兜底触发的保持段不会因为低于目标速度被踢回加速段。
- 上位机提供 `开始刹车` 按钮，当前 run 处于加速或保持时可手动切到刹车阶段，继续写入同一个日志。
- 多 PID 模式下，加速段必须下发 `ACCEL`，连续达速保持段会把同一个速度目标重新下发为 `NORMAL`，刹车段必须下发 `BRAKE`；关闭多 PID 时三段都使用 `NORMAL`。
- 刹车阶段会周期性重复下发自定义刹车目标速度；默认 `0`，也可以填负数作为反向目标。刹车目标接近 `0` 时按实际刹停自动结束；非零目标会按有符号速度到达方向自动结束，例如 `+100mm/s` 表示降到前进 100mm/s 附近，`-100mm/s` 表示达到反向 100mm/s 附近。
- 自动记录加速、保持和刹车阶段日志；正常刹停会以 `completed` 收尾，目标速度达不到、刹车超时或遥测断流会以 `failed` 收尾并关闭本次日志。自动结束/取消/失败时，上位机会先发 0 速度，再发送 `STOP_DIRECT_SPEED` 释放本次测试状态机。
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
- `target_motion_speed_mm_s`：本次目标速度，带符号，正数前进、负数反向。
- `target_forward_speed_mm_s`：本次目标速度绝对值，用于兼容旧表格。
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

采集字段包含惯导位置、车身速度、速度绝对值、由速度差分得到的加速度、阶段时间、阶段距离、加速距离、刹车距离、带符号目标速度、刹车目标速度、车端速度命令、刹车车端命令、PID 模式、左右轮速度、理论/实际 yaw rate、姿态角、处理后的三轴陀螺仪、低通加速度和重力分量。

下位机新增控制码：

- `WIFI_HOST_CTRL_SET_TARGET_SPEED = 0x20`
- `WIFI_HOST_CTRL_ARM_DIRECT_SPEED = 0x21`
- `WIFI_HOST_CTRL_STOP_DIRECT_SPEED = 0x22`
- payload：`uint8 control_id + float target_speed + uint8 pid_mode + uint8 flags`
- `pid_mode`：`0=NORMAL`，`1=ACCEL`，`2=BRAKE`
- `ARM_DIRECT_SPEED` 用于进入独立速度测试状态机；非零 `SET_TARGET_SPEED` 只有在该状态机激活后才会被接受。
- 状态机激活期间，`pit0_ch1_isr` 不再把遥控器油门映射到 `target_speed_set`。如果 CH5、反向刹车、CH6 失能或倒地保护触发，状态机会立刻清零目标并退出 active，让遥控安全路径恢复接管。
- 小车收到状态机 ARM、STOP 或目标速度变化时会触发一次短蜂鸣，用于确认车端确实收到了上位机命令。

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
