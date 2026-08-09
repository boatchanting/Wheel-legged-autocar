# AGENTS.md — TongjiCar1 双核轮腿车工程（AI 代理速查）

面向 AI 编程助手的工程速查。详细文档见 `docs/` 与根目录 `README.md`；本文件只记录“探索成本高、容易踩坑、文档没写全”的知识，其余一律链接到文档（Link, don't embed）。

## 1. 工程是什么

- 智能轮腿车（双轮足并联腿）竞赛工程：平衡行驶、GNSS/惯导导航、轨迹回放、单边桥、地雷区、颠簸路、跳跃等任务。
- 平台：CYT4BB（Cortex-M7 **双核**）＋ 逐飞开源库，IDE = IAR（**无 CLI 构建**）。
- 双核分工：
  - **Core0**（`user/main_cm7_0.c`、`user/cm7_0_isr.c`）＝控制 / 导航 / 任务状态机 / 0 核视觉控制。
  - **Core1**（`user/main_cm7_1.c`、`user/cm7_1_isr.c`）＝摄像头图像处理 / 视觉识别 / 图传。
  - 跨核通信：`code/vision/vision_ipc.h`、`code/vision/vision_ipc_core0.*`、`code1/vision/vision_ipc_core1.*`。

## 2. 构建与烧录

- IAR 工程：`iar/cyt4bb7.eww`；0 核 `iar/project_config/cyt4bb7_cm_7_0.ewp`、1 核 `cyt4bb7_cm_7_1.ewp`。
- ⚠️ IAR **不自动收集源文件**：新增/删除 `.c` 必须手动改 `.ewp`（0 核工程）与 `.ewt`，否则链接失败。
- 比赛/性能前必须把 `code/config/sys_options.h` 的 `DEBUG_LOG_ENABLE` 置 0（串口日志拖慢性能）。
- 清理临时文件：`iar/删除临时文件IAR.bat`。

## 3. 配置开关（改行为先看这里）

- `code/config/sys_options.h`：
  - `CURRENT_NAV_PLAN`：1/2/3/4/99 = 科目几（当前 **4＝科目四**，融合桥/坡/跳/颠簸/雷区）。
  - `REMOTE_CONTROL`、`GNSS_NAV`、`ROLL_BALANCE_ENABLE_INIT`（默认 0＝Roll 横滚环全局关闭）、`SBUS_ACTIVE_POINT`（4=单边桥遥控触发键）。
- `code/config/car_select.h`：`CAR_SELECT`（0 学习板 / 3 新车），同一套代码按车型切换 PID 参数（`code/plan/bridge.h` 的 HIGH 姿态宏也按此区分）。

## 4. 0 核任务调度（`user/cm7_0_isr.c` 的 `pit0_ch0_isr`，1ms 基准 `loop_counter`）

| 周期 | 内容 |
| --- | --- |
| 1ms | 陀螺平衡环 `Gyro_Loop_Control`、IMU、`Control_Profile_Update1ms` |
| 2ms | `VisionIpc_Core0_Update_2ms` ＋ 各视觉任务 `*_Update_2ms`（`VisionBridgeTask`、`VisionSlopeTask`、`VisionThreeStageControl`、PVC、Bumpy） |
| 3ms / 5ms / 9ms | 角度环、转向环；Roll 环(5ms，需 `roll_balance_enable`)、舵机速度环＋前馈(9ms) —— 详见 `code/calculate/pid-new.c` |
| 10ms | `NavReplay_Process()`（科目导航复现；**特殊任务激活时被 gate 跳过**） |
| 20ms | `Bridge_Test_Triple_SingleSide_Inertial()`（纯惯导桥测试，见坑点 C1） |
| 100ms | GNSS（`GNSS_NAV==1` 时） |

## 5. 科目四（`CURRENT_NAV_PLAN==4`）单边桥全链路 —— 重点

总链路：**静态路线表 → Plan4(LQR) 追踪 → 距入口 500mm 交接给视觉桥任务 → 2ms 状态机 → `bridge.c` 抬升+PID 插值 → `pid-new.c` 各环执行**；1 核视觉（新管线 `bridge_detect`）把“仲裁控制线 / 退出线 / gate”通过 IPC b2_* 送 0 核。

### 5.1 路线与 Plan4

- 静态路线：`code/navigation/nav_replay_route_table.h`（约 1005 点，点距约 50mm），由 PC 端 `generate_plan4_smooth_path.py` 生成（见 `code/navigation/nav_replay/plan4/plan4_lqr_speed_planning.h` 头注释）。
- Plan4 源码：`code/navigation/nav_replay/plan4/plan4_lqr_speed_planning.c`。注意：`nav_replay.c` 用 `#include "xxx.c"` 方式包含 plan1/2/3，但 **plan4 是独立编译单元**（`nav_replay.h` 按 `CURRENT_NAV_PLAN` include 其 .h；.c 已加入 ewp）。
- 控制律：`u = Kff·v·κ + Ky·ey + Kpsi·εψ`，`PLAN4_LQR_SIGN=1`；速度单位换算 `PLAN4_LQR_SPEED_TO_MM_S=4.79`；速度/转角指令有 slew 与限幅（宏全在 plan4 header）。
- 定位用**融合坐标 `nav_vision_fusion_x/y`**（视觉任务可 rebase 它），速度用 `inertial_nav.vx_body`。

### 5.2 特殊任务交接（Plan4 ↔ 视觉任务）

- 路点成对：入口（`NAV_POINT_BRIDGE` 等）＋ 出口（`NAV_POINT_BRIDGE_EXIT`）。
- 距入口 `PLAN4_SPECIAL_HANDOFF_LEAD_MM(500)` 内 → `Plan4_StartSpecial` → 对桥调用 `VisionBridgeTask_Start()`，置 `g_special_action_trigger=1`。
- **所有权约定**：`g_special_action_trigger≠0` 或 `Plan4_SpecialIsActive()` 期间，`NavReplay_Process` 一律不输出 `err_degree / target_speed_set`，绝不覆盖特殊任务指令。
- 结束：`Plan4_CompleteSpecial`；**仅当** `g_bridge_vision_task_exit_reason == VISION_BRIDGE_EXIT_VISUAL_CONFIRMED` 才把 `nav_vision_fusion` rebase 到出口点（`AUTO_TIMEOUT` 不 rebase，避免错误重定位）。

### 5.3 视觉桥任务（新管线：1核 `bridge_detect` + 仲裁 → 0核 `vision_bridge_control.c`，2ms）

- **1核**：`bridge_detect.c`（三线透视+MLP 退出线）→ `bridge_v2_arbiter.c` 仲裁（RB/RMB→红蓝中点、RM/MB→绿线、R/B/M/NONE/RB_Q→失能）→ `vision_ipc_core1.c` 发布 `b2_*`（`vision_ipc.h`）。
- **0核** 状态机：`IDLE → ALIGN(对齐) → RUN(上桥) → EXIT(下桥缓冲) → FINISH`，异常走 `FAILSAFE`。所有阈值在 `code/vision/vision_bridge_control.h` 的 `VISION_BRIDGE_TASK_*` 宏（tick＝2ms）。
- 对齐：控制线 b2（系数代入 y=25 + 支撑校验）经“跳变拒绝＋一阶滤波”（按 `packet->seq`）→ IPM 前视差角 → `err_degree`；失能 N=8 帧回锁角、恢复 M=4 帧回视觉（`VISION_BRIDGE_TASK_VALID_LOST/RECOVER_FRAMES`）。
- 上桥判定（C11）：**改听惯导** `traveled ≥ ON_BRIDGE_TRIGGER_MM(900 待标定)`；RUN 内 `b2_gate` 刷新 `bridge_hold_ticks` 保持高姿态；前 1.2m 用视觉，之后锁航向提速 ×2。
- 出口判定：**里程门 + 退出线视觉确认 + 超时兜底**（沿用旧逻辑接新信号；0808 分支的 1D EKF 退出融合实测反复失败，已放弃）——`traveled ≥ RUN_MIN_MM(2300)` 且退出线（`b2_top`，桥远端上沿线）在 x=47 处行坐标 < `EXIT_LINE_TOP_Y_PX(10)` → `VISUAL_CONFIRMED`（rebase）；超时 5000 ticks → `AUTO_TIMEOUT`（不 rebase）。
- 姿态切换：`vision_bridge_apply_high_posture / normal_posture`。结束 `FINISH` 用 `cleanup(0)` 交还 Plan4。
- ⚠️ 无 V2 开关：旧 `bridge_vision` 管线已删除（2026-08-08 全切换），`b2_*` 为唯一桥协议。

### 5.4 高度与 PID 插值（`code/plan/bridge.c`）

- `Smooth_Height_Control`：步进升降 `servo_height`；每次变化即调 `PID_Dynamic_Update_By_Height`。
- `PID_Dynamic_Update_By_Height`：按高度比例 `ratio=(h-h_normal)/(h_bridge-h_normal)` 线性插值 `pid_angle / pid_gyro / pid_servo_speed / pid_turn_*` 的 NORMAL↔HIGH 参数（宏在 `code/plan/bridge.h`，按 `CAR_SELECT` 区分）。
- `bridge_params`（`Bridge_Init`）：`height_normal=3.0 / height_bridge=6.0`、`height_step_rise=0.5`、`servo_acc/dec_bridge=100`。
- ⚠️ `bridge_params.speed_brake/ready/climb/normal`（-30/-45/-120/-180）是**旧状态机**用的，视觉任务不用它们。

### 5.5 底层执行（`code/calculate/pid-new.c`）

- 角度环 3ms → 期望角速度；陀螺环 1ms → 轮子平衡 PWM；舵机速度环 9ms → `g_target_pwm_speed_adj`（前倾）；`Roll_Balance_Control` 5ms → `g_target_pwm_roll_adj`（收腿）。
- 全部位置式 PD（Ki=0，保留积分+限幅结构）；微分基于误差差分（非测量微分）；Kp/Kd 多为负值（符号在控制方向中消化）。细节见 `docs/pid参数.md`。

## 6. 已知坑 / 摩擦点（改代码前必读）

- **C1 桥代码现状（2026-08-08 全切换后）**：
  1. 正式（科目四在用）：新管线——1核 `bridge_detect.c` + `bridge_v2_arbiter.c` → IPC `b2_*` → 0核 `VisionBridgeTask_*`（`code/vision/vision_bridge_control.c`，消费 b2_*，出口判定为里程门+退出线视觉确认，无 EKF）。
  2. 旧距离状态机：`Bridge_Trigger() / Bridge_Update()` —— **当前无任何调用（死代码）**，但 `docs/模块文档/bridge.md` 仍把它描述为“正式流程”，极易误导。
  3. 测试：`Bridge_Test_Triple_SingleSide_Inertial()` 每 20ms 被调，但需 `debug_triple_bridge_test_enable=1`（`Bridge_Test_Triple_SingleSide_Start` 在 `user/main_cm7_0.c` 中被注释）才工作，当前休眠。
- **C2 Roll 横滚补偿当前是关的**：`ROLL_BALANCE_ENABLE_INIT=0`，且 `vision_bridge_control.c` 的 `vision_bridge_apply_high_posture` 显式 `roll_balance_enable=0`（注释却写“开启”，误导）。`Roll_Balance_Control` 需 `roll_balance_enable=1` 才输出 `g_target_pwm_roll_adj`。想启用“过桥主动收腿”必须同时打开这两处。
- **C3 速度符号**：`target_speed_set` 负数＝前进；`VISION_BRIDGE_TASK_*_SPEED_SET` 全为负。
- **C4 坐标/图像约定**：IPM 坐标 X 右、Y 前；图像 94×60；`VISION_BRIDGE_TASK_IMAGE_CENTER_X=47` 为现场标定值。
- **C5 IPC 防抖**：控制线跳变拒绝阈值 8px/8deg、确认 3px/3deg；滤波只在新 `seq` 时更新；原始可信 = `b2_valid`（失能 N=8 帧回锁角，恢复 M=4 帧回视觉）。
- **C6 出口判定**：退出线信号已由旧 `up_line` 端点换成新管线 `b2_top` 直线系数（同一物理线：桥远端上沿线）；判定逻辑沿用旧结构——里程门 2300mm + 退出线 y@x=47 < 10px 视觉确认 + 超时 5000 ticks 兜底。（0808 分支曾实现 1D EKF 退出融合，实测反复失败已放弃，勿回退到该方案。）
- **C7 IAR 文件管理**：新 `.c` 不自动入工程；`nav_replay.c` 是 `#include .c` 风格（plan1/2/3），plan4 例外（独立编译）。
- **C8 文档与代码漂移**：`docs/任务规划/*.md`、`docs/模块文档/bridge.md` 部分内容已过时（尤其“正式流程＝`Bridge_Update`”的旧说法），一律以代码为准；新管线设计/执行见 `docs/任务规划/新单边桥视觉管线接入设计.md` 与 `新单边桥接入-执行记录.md`。
- **C9 `dt` 是保留宏**：`code/calculate/ekf.h` 有 `#define dt (0.001f)`，任何局部变量**不要命名为 `dt`**（会被宏替换导致 Pe040）。

## 7. 文档导航（详细内容一律看这里）

- 架构总览：`README.md`、`docs/project-structure/README.md`、`docs/code文件概览.md`、根目录 `双轮足并联腿机器人结构与控制架构说明.md`。
- 单边桥：`docs/模块文档/bridge.md`、`docs/任务规划/单边桥元素控制.md`（四种过桥策略方案）、`docs/任务规划/科目三视觉融合惯导方案.md`。
- PID / 控制：`docs/pid参数.md`、`docs/多预设PID调用指南.md`、`docs/PID_preset_work_log_260712.md`、`docs/任务放在哪个核里.md`、`docs/待优化的代码.md`。
- 视觉：`docs/任务规划/视觉识别及控制优化建议.md`、`docs/任务规划/视觉识别及控制优化建议2.md`、`docs/边缘检测模块接入记录.md`。
