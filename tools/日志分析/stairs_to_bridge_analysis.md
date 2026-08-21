# 台阶出口到单边桥入口日志分析

筛选条件：`g_replay_state == 1`；区间起点为三级台阶状态机从 active=1 变为 0，终点为单边桥状态机从 0 变为 1。

## 关键结论

- 三次运行的该区间实际耗时约 5.57 s，积分距离约 6.45 m。
- 实际 `|vx_body|` 中位数约 1084 mm/s；目标转速中位数约 200 rpm，即约 958 mm/s。
- 目标转速不超过 220 rpm 的时间比例约 75.9%，说明这段开放道路大部分时间受 `-200/-220` 级别目标速度限制。
- PWM 中位数约 585，没有表现出长期 PWM 饱和；首要瓶颈是目标速度和状态机接管，不是电机已经饱和。
- 当前路表台阶出口到单边桥入口约 6.66 m，其中目标速度不超过 220 rpm 的路段约占 70.5%。
- 路表在这一段的最大曲率为 0.005574 1/mm，局部曲率上限最低只有 82 rpm；该尖峰会造成台阶出口后的短时降速和 PWM 波动。
- 仅按离线路表速度积分，当前曲线通过这一段约需 6.46 s；候选曲线约为 4.63 s。该值不包含台阶/桥状态机时间，不能直接当作实车成绩。

## 建议的优化顺序

1. 先消掉台阶出口后的局部曲率尖峰：忽略手打普通点，直接以台阶出口走廊和单边桥入口走廊构造较长控制柄的 G2 曲线。只提高速度而保留这个尖峰，会把上限再次压回约 82 rpm。
2. 将桥前固定 `-200` 的 approach 距离从 2500 mm 缩短到 600~700 mm，与 C 侧 `PLAN4_SPECIAL_ALIGN_DISTANCE_MM=600` 的实际对准窗口一致；开放段按曲率上限规划。
3. 检查台阶状态机结束后的 Plan4 handoff：日志中状态机结束后目标速度会落到约 300 rpm 附近，建议把出口再接管窗口从 600 mm 缩短或把 `PLAN4_EXIT_REJOIN_MAX_SPEED_CMD` 提到 450~500，并确认融合坐标重定位后的横向误差仍小于 100 mm。
4. 保留最后 600~700 mm 的桥前低速和桥状态机自身的 `-200`，不要在桥上直接追求开放段速度；用状态机 active 信号作为硬边界。
5. 先用候选曲线图做离线检查，再单独生成候选路表进行一次低风险试跑；本目录中的候选曲线没有写入固件路表。

## 输出文件

- `stairs_to_bridge_summary.csv`: 三次运行的量化指标。
- `stairs_to_bridge_speed_<run>.png`: 每次运行的目标/实际速度、PWM、误差和时间轴。
- `stairs_to_bridge_runs_overlay.png`: 三次运行按台阶出口对齐的对比。
- `stairs_to_bridge_route_candidate.png`: 当前路表和缩短桥前 approach 的候选速度曲线。
  
已完成模块拆分，原入口脚本仍可按原命令运行。
[入口编排脚本](tools/webview_nav_marker科目四/generate_plan4_smooth_path_考虑响应延迟_丝滑轨迹.py)：CLI、整体流程调度
[数据模型与常量](tools/webview_nav_marker科目四/path_and_speed/plan4_models.py)：点类型、状态机段、轨迹段、速度档案、默认参数
[配置模块](tools/webview_nav_marker科目四/path_and_speed/plan4_config.py)：通用/专属 TOML 读取、校验、模板生成
[路线编排](tools/webview_nav_marker科目四/path_and_speed/plan4_route.py)：CSV、状态机识别、预设选择、点过滤
[几何模块](tools/webview_nav_marker科目四/path_and_speed/plan4_geometry.py)：G2 曲线、绕掉头桩、重采样
[速度模块](tools/webview_nav_marker科目四/path_and_speed/plan4_speed.py)：曲率限速、加减速包络、状态机恒速、延迟补偿
[输出模块](tools/webview_nav_marker科目四/path_and_speed/plan4_output.py)：C 路表、CSV、轨迹图和速度热力图