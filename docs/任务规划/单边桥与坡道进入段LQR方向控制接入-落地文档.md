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

## 7. 回退方案

- **L1（参数级）**：将 `VisionEntryLqr_UpdateVision` 的有效判定恒置 0（等效回退现状锁角/直行搜索）。
- **L2（代码级）**：`git checkout` 回退以下文件：
  `code/vision/vision_entry_lqr.c/.h`（删除）、`vision_slope_control.c/.h`、`vision_bridge_control.c/.h`、`code1/vision/vision_ipc_core1.c`、`iar/project_config/cyt4bb7_cm_7_0.ewp/.ewt`。
