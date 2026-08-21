# 颠簸路到雷区衔接日志分析

输入：`nav_replay_route_table_08181919.h` 与 2026-08-18 三份 Wi-Fi 遥测。
同一 CSV 中若 `loop` 回退，会拆成独立的回放片段；仅统计同时记录到颠簸退出和雷区开始转圈的片段。

## 路线事实

- 颠簸出口 type=50：路表 index `610`，坐标 `(-16488, 2145)`。
- 首个雷区 type=1：路表 index `728`，坐标 `(-11085, 3936)`。
- 二者沿路表距离约 `5786 mm`；这段路线没有新的配对视觉任务，属于可提速的普通导航段。
- 离线速度表在该段峰值为 `1498 rpm`（约 `7174 mm/s`），位于出口后约 `3180 mm`。

## 实测汇总

| run | bumpy_exit_loop | mine_spin_loop | transition_s | route_remaining_at_exit_mm | straight_distance_at_exit_mm | initial_actual_forward_mm_s | brake_model_distance_at_exit_mm | estimated_progress_mm | mean_actual_forward_mm_s | max_actual_forward_mm_s | mean_command_forward_mm_s | zero_command_s | crawl_command_s | yaw_block_zero_s | max_abs_err_deg | max_route_offset_mm |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| wifi_telemetry_20260818_175457_873_雷区未进 / run 1 | 236962 | 240552 | 3.6 | 2505.5 | 2652.3 | 2090.8 | 1378.4 | 5630.2 | 706.9 | 6715.6 | 618.9 | 0.6 | 2.7 | 0.1 | 52.0 | 1503.9 |
| wifi_telemetry_20260818_182825_112 / run 3 | 25453 | 31352 | 5.9 | 3905.5 | 3884.8 | 6720.4 | 10244.4 | 5580.2 | 609.3 | 6720.4 | 700.4 | 0.4 | 5.2 | 0.0 | 32.1 | 1118.7 |

## 根因

- `BumpyRoad_Update_1ms()` 在视觉确认出口后仍执行 `BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM = 1500 mm`，期间继续独占速度控制。日志显示状态机真正结束时，按路线最近邻估计只剩约 2.5–3.9 m 到雷区；该距离不足以执行路表前半段的高速曲线。融合坐标有跳变，因此该数字用于量级判断而非定位精度。
- `NavReplay_Process()` 只要下一个特殊点为 type=1，就无条件调用 `Plan4_ProcessMinefieldApproach()` 并返回，完全绕过普通 LQR 和离线路表速度。它不会等待距离缩短到专用的雷区接管门限。
- 雷区接近的实测首帧速度与当前刹车多项式对应的刹车距离见表。速度高于 `1500 mm/s` 且距离已小于该模型值时，代码立即将 `target_speed_set` 置零；速度降到阈值后，改为固定 `750 mm/s` 蠕行直到 250 mm 执行圆。两份完整日志的 `crawl_command_s` 正是主要时间损失。
- 红色阴影是零速度指令。航向门限 +/-35 度在一份日志中只贡献约 0.06 s，另一份没有触发；它不是本次 3.6–5.9 s 过渡的主因。20 个导航周期的交接斜坡同样不足以解释该量级。

## 建议的优化顺序

1. 先改颠簸退出时机，而非盲目提高雷区速度：把视觉出口后的 `BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM` 逐档试为 1000、750、500 mm，并记录坐标锚定后的横向误差。目标是把至少 4–5 m 的普通路段交还 Plan4，同时保持退出重定位稳定。
2. 增加雷区的 LQR 接管距离：type=1 在远距离时仍按路表跟踪；接近到由当前实测速度和刹车模型决定的安全距离时，再进入 `Plan4_ProcessMinefieldApproach()`。现有代码对 type=1 无条件提前接管，是离线速度曲线失效的直接原因。
3. 将雷区接近改成连续的速度包络，而不是“零指令减到 1500 mm/s 后固定 750 mm/s”。例如以 `v^2 = 2*a*(distance-execute_radius)` 为主曲线，并对实测速度超出曲线的部分施加受限减速度；只有安全越界才使用零指令。这样能避免长距离蠕行，同时不削弱超速保护。
4. 最后再改轨迹：保持 type=50 到 type=1 的直线末端切向圆心，避免最后 +/-35 度才原地对正。路线几何优化有价值，但在第 2 条之前不会提高实际速度，因为当前实现根本不消费这段路表。
5. 用同轮胎、同电压、同场地重测刹车距离，再拟合 `PLAN4_MINEFIELD_BRAKE_POLY_*`。在未验证前不要只调低 `PLAN4_MINEFIELD_BRAKE_DIST_RATIO` 或调高触发速度，这会直接缩小转圈前的停车裕度。

图表：`bumpy_to_minefield_analysis.png`。最近邻路线投影仅用于判断趋势；融合坐标会在视觉锚定时跳变，因此不用于控制精度结论。
