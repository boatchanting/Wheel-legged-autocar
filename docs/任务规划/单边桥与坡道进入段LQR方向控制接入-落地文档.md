# 单边桥 / 坡道进入段 LQR 方向控制接入 · 落地文档

> 对应规划：`docs/任务规划/单边桥与坡道进入段LQR方向控制接入规划.md`（v5，已批准执行）
> 执行日期：2026-08-17
> 执行结论：**双核编译 0 错误**（cm7_0 14 既有警告 / cm7_1 8 既有警告，均为历史无害警告）

---

## 1. 改动文件清单

| 文件 | 类型 | 说明 |
|---|---|---|
| `code/vision/vision_entry_lqr.h` | 新增 | 共享 LQR 方向控制器头文件 |
| `code/vision/vision_entry_lqr.c` | 新增 | 共享 LQR 方向控制器实现 |
| `code/vision/vision_slope_control.c` | 修改 | 坡道 PVC_ALIGN 方向改 LQR |
| `code/vision/vision_slope_control.h` | 修改 | 状态结构增 3 个 LQR 诊断字段 |
| `code/vision/vision_bridge_control.c` | 修改 | 单边桥 ALIGN 方向改 LQR |
| `code/vision/vision_bridge_control.h` | 修改 | 状态结构增 3 个 LQR 诊断字段 |
| `code1/vision/vision_ipc_core1.c` | 修改 | bridge 专用 PVC phy 旁路透传 |
| `iar/project_config/cyt4bb7_cm_7_0.ewp` | 修改 | cm7_0 工程 vision 组加 2 文件 |
| `iar/project_config/cyt4bb7_cm_7_0.ewt` | 修改 | cm7_0 工程 vision 组加 2 文件 |

---

## 2. 新增 `code/vision/vision_entry_lqr.c/.h`

### 2.1 宏（`vision_entry_lqr.h`）
```
LQR_QY = 150.0f          LQR_QPSI = 24.0f
LQR_W_MAX_RADPS = 2.2f   LQR_ERR_MAX_DEG = 2.2·57.29578/8 ≈ 15.76°
LQR_V_FLOOR_MPS = 0.3f   LQR_PHY_INVALID_MM = 32767
LQR_DETECT_RANGE_M = 1.5f LQR_KLOCK = 1.8f
```

### 2.2 接口
- `VisionEntryLqr_Reset(entry_yaw_deg)` — 复位并锁存基准航向。
- `VisionEntryLqr_UpdateVision(phy_x_mm, phy_y_mm, yaw_deg, v_mps)` — 视觉段 LQR 更新，返回 1=有效。
- `VisionEntryLqr_GetErrDegree()` — 返回 `err_degree = ω·57.29578/TURN_ANG_KP`，钳 ±LQR_ERR_MAX_DEG。
- `VisionEntryLqr_GetState()` — 只读状态（诊断用）。

### 2.3 核心公式（严格按规划 §3.1）
```c
dist_m   = hypot(phy_x, phy_y)/1000;             // 超出 1.5m 或 phy 无效 → 返回 0
beta_rad = atan2f(phy_x, phy_y);
yaw_rad  = yaw_deg · DEG2RAD;
psi_err_deg = normalize(entry_yaw_deg − yaw_deg);
psi_err_rad = psi_err_deg · DEG2RAD;
e_m      = dist_m · sinf(beta_rad + yaw_rad);    // 横向偏差重建
k1 = sqrtf(LQR_QY);  k2 = sqrtf(2·fmax(v,0.3)·k1 + LQR_QPSI);
omega = clamp(k1·e_m + k2·psi_err_rad, ±2.2);
err_degree = clamp(omega·57.29578f/TURN_ANG_KP, ±15.76°);
```

---

## 3. 修改详情

### 3.1 `vision_slope_control.c`
- `#include "vision/vision_entry_lqr.h"`。
- 内部上下文 `vision_slope_task_ctx_t` 增 `float entry_yaw_deg;`。
- `vision_slope_enter_task()`：新增 `entry_yaw_deg = inertial_nav.relative_yaw` + `VisionEntryLqr_Reset(entry_yaw_deg)`。
- `VISION_SLOPE_TASK_PVC_ALIGN` 分支方向替换：
  ```c
  if (VisionEntryLqr_UpdateVision(packet->pvc_phy_x_mm, packet->pvc_phy_y_mm,
                                  inertial_nav.relative_yaw, fabsf(inertial_nav.vx_body)/1000.0f))
      err_cmd = VisionEntryLqr_GetErrDegree();
  else
      err_cmd = 0.0f;   // 回退直行搜索（现状）
  ```
  速度 `PVC_ALIGN_SPEED_SET` 与进坡判据**保持原样**。
- `vision_slope_publish_status()`：追加 `lqr_e_m / lqr_psi_err_deg / lqr_dist_m`。

### 3.2 `vision_slope_control.h`
- `vision_slope_task_status_t` 增 `lqr_e_m / lqr_psi_err_deg / lqr_dist_m`。

### 3.3 `vision_bridge_control.c`
- `#include "vision/vision_entry_lqr.h"`。
- `vision_bridge_enter_task()`：新增 `VisionEntryLqr_Reset(entry_yaw_deg)`。
- `VISION_BRIDGE_TASK_ALIGN` 分支方向替换：
  ```c
  if (VisionEntryLqr_UpdateVision(packet->pvc_phy_x_mm, packet->pvc_phy_y_mm,
                                  inertial_nav.relative_yaw, vision_bridge_abs_f(inertial_nav.vx_body)/1000.0f))
  {   err_cmd = VisionEntryLqr_GetErrDegree();  s_bridge_task.err_source = 0U; }
  else
  {   err_cmd = vision_bridge_calc_yaw_hold_err_degree();  s_bridge_task.err_source = 1U; }
  ```
  速度 `RUN_SPEED_SET`、上桥判据（`b2_valid&&b2_gate` / 惯导门 / 超时）、`apply_err_ramp` 换源限速**保持原样**。
- `vision_bridge_publish_status()`：追加 `lqr_e_m / lqr_psi_err_deg / lqr_dist_m`。
- RUN 阶段 PD 循线（`vision_bridge_calc_visual_err_degree`）**未改动**。

### 3.4 `vision_bridge_control.h`
- `vision_bridge_task_status_t` 增 `lqr_e_m / lqr_psi_err_deg / lqr_dist_m`。

### 3.5 `code1/vision/vision_ipc_core1.c`
- `#include "bridge_pvc_vision.h"`。
- `vision_ipc_core1_fill_bridge_v2()` 末尾追加：
  ```c
  bridge_pvc_vision_output_t pvc_local;
  pvc_local = *bridge_pvc_vision_get_output();
  if (pvc_local.stable_detected)
  {   packet->pvc_phy_x_mm = pvc_local.stable.phy_x_mm;
      packet->pvc_phy_y_mm = pvc_local.stable.phy_y_mm;   }
  ```
  IPC 结构体 `vision_ipc_packet_t` **零改动**（复用 `pvc_phy_x_mm/phy_y_mm` 字段）。

### 3.6 IAR 工程
- `cyt4bb7_cm_7_0.ewp` + `.ewt` 的 `vision` 组，在 `vision_bridge_control.h` 之后插入 `vision_entry_lqr.c/.h` 两条目。

---

## 4. 编译验证

| 工程 | 命令 | 结果 |
|---|---|---|
| cm7_0 | `iarbuild cyt4bb7_cm_7_0.ewp -build Debug -parallel 8` | **0 错误**，14 警告（均为既有：PI 宏重定义、未使用变量等） |
| cm7_1 | `iarbuild cyt4bb7_cm_7_1.ewp -build Debug -parallel 8` | **0 错误**，8 警告（均为既有：未使用变量/函数） |

构建工具：`D:\tools\IAR Systems\Embedded Workbench 9.2\common\bin\iarbuild.exe`（V9.40.1）。

---

## 5. 实现决策说明（供审查）

1. **新增 `VisionEntryLqr_GetState()`**：规划 §5.1 函数清单未列此接口，但 §5.4/§8 要求发布 `lqr_e_m/lqr_psi_err_deg/lqr_dist_m` 诊断字段，必须能读取 LQR 内部状态。故补充只读 getter，服务于规划已批准的要求。
2. **e 公式基准**：严格按规划 §3.1，`e_m = D·sin(β + yaw_rad)`，其中 `yaw_rad = relative_yaw` 弧度（绝对航向）；而 `ψ_err = entry_yaw − relative_yaw`（相对桥面基准）。两者基准不同的实现如实记录，符号正确性由 §3.2 锚定 + 实车打角方向复核兜底。
3. **v 来源**：`inertial_nav.vx_body`（互补滤波车身速度），仅 k2 增益调度只读。
4. **ψ存储**：均取 `entry_yaw`（进入状态机时刻 yaw）。
5. **`fabsf`/`sqrtf`/`atan2f`/`sinf`**：经 `zf_common_headfile.h → arm_math.h` 提供，编译已通过。

---

## 6. 待实车标定 / 复核项（未批准前不改）

1. **打角方向复核**：正 e 是否右转、正 ψ_err 是否左转（§3.2）；不符则查 TURN_ANG_KP/电机接线/relative_yaw 正方向。
2. `LQR_QY / LQR_QPSI` 现场调参（先 Qψ 后 Qy，Qy 上限自觉线 200、下限 100）。
3. 检测距离 1.5m 现场标定（用户自行调节）。
4. `W_SLEW ≥ 6 rad/s²` 复核，内环 PD 勿动。

---

## 7. LQR 调参参数全集与日志说明（2026-08-17 增补）

> 本次增补：为 LQR 调参补齐「日志可见性」——确认当前生效参数 + 观察运行时值，全部落入现有日志。仅改日志/诊断，**控制算法零改动**。

### 7.1 LQR 调节所需参数全集（`vision_entry_lqr.h`）

控制律：`ω = k1·e + k2(v)·ψ_err`，`k1=√Qy`、`k2=√(2·max(v,0.3)·√Qy+Qψ)`，
`err_degree = ω·(180/π)/TURN_ANG_KP`（TURN_ANG_KP=-8）。

| 宏 | 值 | 类别 | 是否旋钮 | 作用 |
|---|---|---|---|---|
| `LQR_QY` | 150.0f | 权重 | ✅ **旋钮** | k1=√Qy，横向收敛速度；唇口 e 大→+10~25；过冲/ω贴2.2→−10~25；上限≈200、下限≥100 |
| `LQR_QPSI` | 24.0f | 权重 | ✅ **旋钮** | 航向权重；ψ唇>3°→+2~4；航向抖动→−2~4 |
| `LQR_DETECT_RANGE_M` | 1.5f | 检测窗 | ✅ 现场标定 | 视觉段检测距离；1.5m 撑 2.5 m/s |
| `LQR_W_MAX_RADPS` | 2.2f | 执行器极限 | ❌ 外部标定 | ω 钳位；换车/换硬件才改 |
| `LQR_ERR_MAX_DEG` | ≈15.76° | 派生 | ❌ 勿单独调 | err_degree 钳位 = W_MAX·57.29578/8 |
| `LQR_V_FLOOR_MPS` | 0.3f | 调度下限 | ❌ 一般不调 | k2 内 max(v,0.3) |
| `LQR_KLOCK` | 1.8f | 冗余 | ❌ 未使用 | 实现中未使用（盲区锁角由 bridge `yaw_hold_kp=1.8` 负责） |
| `LQR_PHY_INVALID_MM` | 32767 | 固定常量 | ❌ | phy 无效标记 |

**只动 Qy / Qψ 两个旋钮；检测距离现场标定；其余勿动。**

### 7.2 运行时诊断值（调参观察对象）

| 字段 | 含义 | 单位 |
|---|---|---|
| `valid` | 本周期视觉段是否有效（1=LQR 输出，0=盲区回退） | - |
| `e_m` | 横向偏差重建 e = D·sin(β+ψ) | m |
| `psi_err_deg` | 航向偏差 ψ_err = ψ_存储 − ψ | deg |
| `dist_m` | 桥唇/入口距离 D | m |
| `omega_radps` | 期望角速度 ω（已钳 W_MAX） | rad/s |
| `entry_yaw_deg` | ψ_存储 基准航向 | deg |

### 7.3 日志落地明细（本次改动）

1. **状态结构体**（`vision_bridge_task_status_t` / `vision_slope_task_status_t`，上位机可读）：
   在原有 `lqr_e_m / lqr_psi_err_deg / lqr_dist_m` 基础上新增
   `lqr_valid / lqr_omega_radps / lqr_entry_yaw_deg`，以及参数副本
   `lqr_qy / lqr_qpsi / lqr_detect_range_m / lqr_w_max_radps / lqr_v_floor_mps / lqr_err_max_deg`。
2. **`[BridgeCtrl]` 串口日志**（`vision_bridge_control.c`，500ms/条）：
   行尾追加 `lqr=%u le=%.3f lpsi=%.1f lD=%.2f lw=%.2f`（valid / e / ψ / D / ω）。
   命名加 `l` 前缀，与既有 `e/ed`（RUN 段 PID）区分。
3. **`[LqrParam]` 参数集打印**（`vision_bridge_control.c`，进入任务时打印一次）：
   `qy=%.1f qpsi=%.1f D=%.2f wmax=%.2f vfloor=%.2f errmax=%.1f`，确认本次构建生效参数。
4. **示波器**（`main_cm7_0.c`）：新增「5.【调 LQR 进入段】」注释调试组
   （valid / e / ψ / D / ω / err_degree_cmd / entry_yaw / traveled），取消注释并注释掉「1.直立环」组即可使用。

> 说明：规划 §8 要求 `[BridgeCtrl]` 追加 LQR 字段（原始接入落地时漏做，仅加了 status 三字段），本次一并补齐。

### 7.4 调参时怎么看日志

- 进任务瞬间先看 `[LqrParam]`：确认 Qy/Qψ/检测距离是否期望值。
- 视觉段（`lqr=1`）逐条看 `le / lpsi / lD / lw`：
  - 唇口 e 大、横向收敛慢 → **Qy +10~25**
  - 接近段过冲 / S 形摆动 / `lw` 持续贴 2.2 → **Qy −10~25**
  - ψ唇>3° 但 e 正常 → **Qψ +2~4**；航向抖动 → **Qψ −2~4**
  - e、ψ 都差 → 先动 Qψ，再动 Qy（短窗架构航向是稀缺资源）
  - `lqr=0` 期间为盲区锁角/直行回退，属正常
- 参数副本（`lqr_qy` 等）随状态结构体持续可读，供上位机确认版本。

---

## 8. 回退方案

- **L1（参数级）**：将 `VisionEntryLqr_UpdateVision` 的有效判定恒置 0（等效回退现状锁角/直行搜索）。
- **L2（代码级）**：`git checkout` 回退以下文件：
  `code/vision/vision_entry_lqr.c/.h`（删除）、`vision_slope_control.c/.h`、`vision_bridge_control.c/.h`、`code1/vision/vision_ipc_core1.c`、`iar/project_config/cyt4bb7_cm_7_0.ewp/.ewt`。
- **L3（日志级）**：本次日志增补为纯加法，如需撤销仅回退 `vision_entry_lqr.c/.h` 参数副本字段与 `vision_bridge_control.c` printf 即可。
