# 3stage 与 slope 的 PVC 视觉独立调参规划

> 状态：**待审批**
> 目的：让三级跳（3stage）与斜坡（slope）两处消费 PVC 视觉的 0 核控制参数完全独立，可各自调参互不影响。
> 修订日期：2026-08-20
> 前提：先完成本文档「阶段 0」的决策点拍板，未获批前不改任何代码。

---

## 0. 结论摘要

| 问题 | 现状 |
|---|---|
| 3stage / slope 的 0 核控制参数能否独立？ | **基本已独立**（各自头文件），但有 3 个真实共享点：①1 核检测参数全局宏；②`vision_entry_lqr` 被 slope 与 **bridge** 共享；③slope 隐式依赖 `vision_pvc_control` 模块（仅取 `bbox_area_ratio_u16`）。 |
| 3stage 的视觉方向修正是否生效？ | **否（死代码）**。`vision_three_stage_apply_err_from_pvc()` 算出的误差被紧随其后的 `vision_three_stage_apply_locked_heading()` 无条件覆盖，实际方向 = 纯惯导航向保持。 |

---

## 1. 现状审核（已核实，含代码行号）

### 1.1 数据流

```
1 核 pvc_vision（检测参数在 code1/vision/pvc_vision.h，单套全局宏）
   │  pvc_* 17 字段 + pvc_phy_x/y 旁路（vision_ipc.h:99-117，VISION_VALID_PVC）
   ▼
IPC packet（g_vision_ipc_latest）
   ▼
0 核 2ms ISR（user/cm7_0_isr.c:300-310，调用顺序固定）：
   VisionIpc_Core0_Update_2ms()
   → VisionPvcControl_Update_2ms()        ← 共享 PID 控制大脑
   → VisionBumpyControl_Update_2ms()
   → VisionBridgeTask_Update_2ms()
   → VisionSlopeTask_Update_2ms()         ← slope（PVC_ALIGN 用 LQR，覆盖上方输出）
   → VisionThreeStageControl_Update_2ms() ← 3stage（自包含）
```

### 1.2 两处调用方式（完全不同）

**A. slope（`code/vision/vision_slope_control.c`）**
- 入口：`vision_slope_enter_task()`（L181-183）执行 `VisionIpc_Core0_SetPvcEnable(1U)` + `VisionPvcControl_SetEnable(1U)`。
- PVC_ALIGN 阶段（L261-278）：
  - 方向 = `VisionEntryLqr_UpdateVision(packet->pvc_phy_x_mm, pvc_phy_y_mm, …)` → `VisionEntryLqr_GetErrDegree()`（PD 模式），**覆盖** `vision_pvc_control` 算出的 `err_degree`；
  - 速度 = `VISION_SLOPE_TASK_PVC_ALIGN_SPEED_SET`（-500），**覆盖** `target_speed_set`；
- 真正消费 PVC 控制模块的只有（L284-285）：
  - `g_vision_pvc_control_status.stable_detected`
  - `g_vision_pvc_control_status.bbox_area_ratio_u16 >= VISION_SLOPE_TASK_PVC_FULL_RATIO_U16(400)`，连续 `VISION_SLOPE_TASK_PVC_ALIGN_OK_TICKS(25)=50ms` → 判定“已压上入口”锁角。
- 即：**slope 对 vision_pvc_control 的依赖 = `bbox_area_ratio_u16` 计算 + `stable_detected` 透传**（该函数 `vision_pvc_calc_bbox_ratio_u16` 是 static，只能通过 enable 模块间接拿到）。

**B. 3stage（`code/vision/vision_three_stage_control.c`）**
- **不调用** `VisionPvcControl_SetEnable/Update`；`VisionThreeStageControl_Start()`（L245）用 `VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY)` 让 1 核跑 PVC 检测，自己读 packet 的 `pvc_stable_detected / pvc_entry_bottom_y / pvc_entry_top_y / pvc_lateral_mm / pvc_yaw_error_deg_x100`。
- 方向（L160-205）：
  - `vision_three_stage_apply_err_from_pvc()`：jump1 远景修正用 `pvc_lateral_mm` × LPF × `JUMP1_CORRECTION_*` 参数；
  - **但随后（L406-409）`vision_three_stage_apply_locked_heading()` 无条件把 `err_degree` 覆盖为 `s_locked_relative_yaw_deg - inertial_nav.relative_yaw`** → 视觉修正结果被丢弃。
  - 佐证：`vision_three_stage_control.h` 中 `VISION_THREE_STAGE_LATERAL_SIGN=0.0f`、`JUMP1_CORRECTION_LATERAL_SIGN=0.0f`（注释：cxz 发车不使用）。
- 触发（第二/三跳已改为固定延时 `JUMP2/JUMP3_DELAY_*`，L426-458；仅第一跳与脱出用视觉阈值 `jump1_bottom_y / exit_top_y`）。
- 速度：`g_vision_three_stage_speed_*`（volatile，在线可调，独立）。

### 1.3 共享点矩阵

| 层 | 参数位置 | 3stage | slope | bridge | 是否共享 |
|---|---|---|---|---|---|
| 1 核检测 | `pvc_vision.h`（THRESHOLD/MIN_AREA/MIN_WIDTH/MIN_HEIGHT/MIN_FILL_RATIO/MIN_DECISION_SCORE/CONFIRM_FRAMES/LOST_HOLD_FRAMES/SMOOTH） | ✔ | ✔ | ✔(经 pvc_phy) | **是，单套全局宏** |
| IPC 结构 | `vision_ipc.h` pvc_* 字段 | ✔ | ✔ | ✔ | 固定，不可分 |
| 0 核 | `vision_pvc_control.h`（PID/分速/距离/STOP_BBOX_RATIO） | ✘ | 间接(bbox_ratio+stable) | ✘ | 仅 slope 消费 bbox |
| 0 核 | `vision_entry_lqr.h`（PD_K_E/P/D、DETECT_RANGE 或 LQR_QY/QPSI） | ✘ | ✔(方向) | ✔(方向) | **slope 与 bridge 共享** |
| 0 核 | `vision_three_stage_control.h`（修正/触发/延时） | ✔ | ✘ | ✘ | 已独立 |
| 0 核 | `vision_slope_control.h`（速度/时间/ratio 阈值） | ✘ | ✔ | ✘ | 已独立 |

### 1.4 关键问题清单

1. **3stage 视觉方向修正未生效（死代码）** — 被 `apply_locked_heading()` 覆盖；需拍板意图。
2. **slope 方向参数与 bridge 共享** — `vision_entry_lqr` 单实例单宏；已有《单边桥与坡道LQR参数分离规划.md》待审批（本规划直接承接）。
3. **slope 隐式依赖 vision_pvc_control** — 仅为拿 `bbox_ratio_u16` 而 enable 整个 PID 大脑，参数语义混乱（如误改 `VISION_PVC_CONTROL_*` 会“以为有效实被覆盖”）。
4. **1 核检测参数全局共享** — 两处若对“检测灵敏度/抗反光”诉求不同，当前无法分开调。

---

## 2. 目标

- 3stage 与 slope 的 **0 核控制参数（方向 + 速度 + 触发）完全独立**，改一处不影响另一处。
- 消除 slope 对 `vision_pvc_control` 的隐式依赖，让“谁在控制方向盘”一目了然。
- （可选）1 核检测参数可按任务切换，两处检测灵敏度独立。
- 兼容现有上位机/示波器监控字段，不破坏 IPC 契约。

---

## 3. 决策点（阶段 0，需先拍板）

| 编号 | 决策 | 选项 | 推荐 |
|---|---|---|---|
| **D1** | 3stage 方向策略 | A. 维持纯锁航向（现状，现场已验证）<br>B. 恢复视觉修正（先修死代码，视觉优先）<br>C. 视觉修正 + 锁航向兜底融合 | **A（若场地已验证）/ C（若希望 3stage 视觉真正参与方向）** |
| **D2** | slope 方向来源 | A. 维持 LQR/PD，但参数与 bridge 分离（承接已有规划）<br>B. 改用 `vision_pvc_control` 的 PID（回归纯 PVC 追踪） | **A** |
| **D3** | 1 核检测参数 | A. 维持全局共享（成本低，两处共用一套检测灵敏度）<br>B. 按任务切换参数集（1 核多套参数，成本高） | **A（先做 0 核分离，检测层按需再议）** |
| **D4** | 运行时可调 | A. 保持编译期宏<br>B. 关键参数 volatile 化（遥控/上位机在线调，同 3stage 速度参数做法） | **B（推荐，便于现场调参）** |

---

## 4. 实施方案（推荐路径，分阶段）

### 阶段 1 — slope 与 bridge 的 LQR/PD 参数分离（承接已有规划）

**目标**：`vision_entry_lqr` 支持运行时参数集，slope / bridge 各持一套。

1. `code/vision/vision_entry_lqr.h`
   - 新增参数结构体（沿用已有规划 §3.1）：
     ```c
     typedef struct {
         float k_e;            /* PD: 视觉扰动放大（LQR: qy 推导 k1=√qy） */
         float p_psi;          /* PD: 航向比例（LQR: qpsi） */
         float d_psi;          /* PD: 航向微分阻尼 */
         float d_tau_s;        /* PD: 微分一阶低通时间常数 */
         float detect_range_m; /* 视觉段检测距离 */
         float w_max_radps;    /* ω 钳位（执行器极限） */
         float v_floor_mps;    /* k2 内 max(v, v_floor) 下限 */
     } vision_entry_lqr_param_t;
     ```
   - `VisionEntryLqr_Reset(float entry_yaw_deg, const vision_entry_lqr_param_t *param);` 签名加参。
   - `UpdateVision` / `GetErrDegree` 内部改用 `s_lqr.param.*`。
2. `code/vision/vision_bridge_control.h/.c`
   - 新增 `VISION_BRIDGE_PD_*` / `VISION_BRIDGE_LQR_*` 宏（沿用现值：K_E=11/P=7/D=1.0/τ=0.05/RANGE=1.5/W_MAX=2.2）。
   - 定义 `static const vision_entry_lqr_param_t s_bridge_lqr_param`，`vision_bridge_enter_task()`（L771）传 `&s_bridge_lqr_param`。
3. `code/vision/vision_slope_control.h/.c`
   - 新增 `VISION_SLOPE_PD_*` 宏（**初值沿用 bridge 同值**，结构上已独立，后续只改 slope 宏）。
   - 定义 `static const vision_entry_lqr_param_t s_slope_lqr_param`，`vision_slope_enter_task()`（L181）传 `&s_slope_lqr_param`。
4. 验证：双核 iarbuild 0 错误；行为不变（两处初值相同）。

> 若 D4=B：把 `s_bridge_lqr_param` / `s_slope_lqr_param` 改为 `volatile` 可写实例，并在各自 `status` 结构暴露当前参数，供上位机在线改。

### 阶段 2 — slope 解耦 vision_pvc_control

**目标**：slope 不再 `VisionPvcControl_SetEnable(1U)` 整个 PID 大脑，只取所需数据。

1. 抽公共函数：把 `vision_pvc_calc_bbox_ratio_u16()`（`vision_pvc_control.c` 内 static）提升为公共接口
   - 方案 A（推荐，侵入小）：在 `vision_ipc_core0.h` 或新建 `vision_pvc_common.h` 声明
     `uint16 VisionPvc_CalcBboxRatioU16(const vision_ipc_packet_t *packet);`，实现移到公共处（可保留原 static 包装）。
   - 方案 B：slope 内直接内联相同计算（复制 20 行，最简单，但两份逻辑）。
2. `vision_slope_control.c`
   - `vision_slope_enter_task()`：去掉 `VisionPvcControl_SetEnable(1U)`，保留 `VisionIpc_Core0_SetPvcEnable(1U)`。
   - L284-285 改用 `VisionPvc_CalcBboxRatioU16(packet) >= VISION_SLOPE_TASK_PVC_FULL_RATIO_U16` 与 `packet->pvc_stable_detected`（不再读 `g_vision_pvc_control_status`）。
   - `vision_slope_cleanup()`：去掉 `VisionPvcControl_SetEnable(0U)`。
   - `vision_slope_publish_status()` 的 `pvc_ratio_u16 / pvc_steer_error_px_x100` 改从 packet 直接取。
3. 结果：PVC 控制模块只在“真正用它的任务”被启用；ISR 顺序中 `VisionPvcControl_Update_2ms` 无启用时输出 idle（已如此），slope 方向/速度由 LQR + 自身宏唯一决定。

### 阶段 3 — 3stage 逻辑理顺（依 D1 拍板结果）

**若 D1=A（维持纯锁航向）**：
- 明确注释：3stage 方向 = 锁航向；`apply_err_from_pvc` 保留但标注“预留/未启用”，或直接删除死分支，避免后续维护误判。
- 删除/清空 `VISION_THREE_STAGE_LATERAL_SIGN`、`JUMP1_CORRECTION_LATERAL_SIGN` 的 0.0f 陷阱注释，改为说明性注释。

**若 D1=B/C（视觉修正生效）**：
- 修改 `vision_three_stage_control.c` Update 段：`apply_locked_heading()` 不再无条件覆盖；
  - B：视觉有效时用视觉误差，失效时回退锁航向（门控与单边桥 RUN 视觉优先逻辑一致）；
  - C：视觉误差与锁航向误差按权重叠加（如 `err = α·visual + (1-α)·locked`，α 可调）。
- 打开 `VISION_THREE_STAGE_LATERAL_SIGN` / `JUMP1_CORRECTION_LATERAL_SIGN` 为 ±1，并按现场标定符号。
- 此分支需单独验证（3stage 全流程跑通）。

### 阶段 4 — 1 核检测参数按任务分离（仅 D3=B 时执行）

**目标**：3stage / slope 用不同检测灵敏度。
- `pvc_vision.h` 参数宏 → 运行时结构 `pvc_vision_param_t`（同 LQR 做法），1 核按 `active_target` 或 0 核下发的 `enable_mask` 切换参数集。
- 0 核 IPC 命令增加“当前消费方”提示（或按 `VISION_TARGET_PVC_ENTRY` 与 slope/3stage 激活状态推断）。
- 成本高、影响 1 核主循环，**建议暂缓**，先验证 0 核分离是否已满足需求。

### 阶段 5 — 运行时可调 + 监控（D4=B 时执行）

- 3stage：`g_vision_three_stage_*`（jump1_bottom_y / exit_top_y / speed_* 已 volatile）补齐修正参数（`lateral_sign/k_lat` 等）。
- slope：`VISION_SLOPE_TASK_*` 关键参数（速度/ratio 阈值/OK_TICKS）volatile 化 + 上位机通道。
- `g_slope_vision_task_status` / `g_vision_three_stage_control_status` 增加当前参数快照字段，示波器可见。

---

## 5. 验证方案

| 阶段 | 验证 |
|---|---|
| 1 | 双核 iarbuild 0 错误；两处初值相同 → 行为不变（回归）；改 slope 宏验证 bridge 不受影响 |
| 2 | slope 全流程（PVC 校准→锁角→上坡→下坡）跑通；示波器确认 `pvc_ratio_u16` 仍正常 |
| 3 | 3stage 全流程（3 跳 + 脱出）跑通；按 D1 分支验证方向行为 |
| 4 | 两处场景分别验证检测灵敏度 |
| 5 | 遥控/上位机在线改参生效 |

## 6. 回退方案

- L1（参数级）：slope 宏改回与 bridge 同值 → 等效旧行为。
- L2（逻辑级）：`git checkout` 回退本次改动；若已提交，按 git 安全规则需用户批准后执行 revert。
- 3stage 若采用 D1=B/C：切回 A 仅需恢复 `apply_locked_heading()` 无条件覆盖一行 + 两处 SIGN 置 0。

## 7. 涉及文件清单

| 文件 | 阶段 | 变更 |
|---|---|---|
| `code/vision/vision_entry_lqr.h/.c` | 1 | 参数结构体化、Reset 加参 |
| `code/vision/vision_bridge_control.h/.c` | 1 | bridge 参数宏 + `s_bridge_lqr_param` |
| `code/vision/vision_slope_control.h/.c` | 1、2 | slope 参数宏 + `s_slope_lqr_param`；解耦 vision_pvc_control |
| `code/vision/vision_ipc_core0.h`（或新 `vision_pvc_common.h/.c`） | 2 | 公共 `VisionPvc_CalcBboxRatioU16` |
| `code/vision/vision_pvc_control.c` | 2 | 暴露 bbox 计算（保留 static 包装） |
| `code/vision/vision_three_stage_control.c/.h` | 3 | 按 D1 理顺方向逻辑 |
| `code1/vision/pvc_vision.h/.c` | 4（可选） | 参数结构体化按任务切换 |
| `user/cm7_0_isr.c` | — | 不改（顺序保持不变） |
