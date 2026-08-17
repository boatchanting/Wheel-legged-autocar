# 单边桥上桥 / 坡道进入段 LQR 方向控制接入规划（修订 v5）

> 状态：**待审批**（未获批准前，严禁改动任何代码）
> 定稿依据：**一切冲突以 `D:\WORKS\2026LunTui\trials\无降速版调参文档.md` 为准**
> 参考：`D:\WORKS\2026LunTui\trials\单边桥上桥控制算法说明.md`（v3）、`D:\WORKS\2026LunTui\trials\index.html`
> 修订日期：2026-08-17

---

## 0. 修订记录

| 版本 | 变更 |
|---|---|
| v1 | 含速度自治器 / 近唇蠕行 / `v_set` 输出 |
| v2 | ① 速度全部移除；② 纠正 bridge 数据源 |
| v3 | 按说明书 v3「全程单条 LQR + 盲区里程计 e」展开 |
| v4 | 冲突以《无降速版调参文档》为准（盲区锁角、Qy150/Qψ24、1.5m、2.5m/s） |
| v5（本版） | ① **移除符号翻转宏**，符号改为推导锚定现有已验证逻辑；② ψ存储=**进入状态机时刻 yaw**；③ v 来源=**车身速度（互补滤波 `vx_body`）**；④ 删除 PVC 帧同步顾虑（PVC 自带稳定）；⑤ 检测距离标定归用户 |

---

## 0.5 约束定稿（用户已裁定）

以《无降速版调参文档.md》为唯一裁决依据：

1. **无任何指令降速**：唯一速度损失 = 转弯物理 `v_target = v_set − drop·|ω|`（既有转向/速度环物理产生，非控制器写入）。
2. **无里程计预修正**：视觉之外无可靠桥口横向参考，只有进入航向可信 → **盲区段锁角保向**（K_lock=1.8）。
3. **检测距离 1.5m**；**参数 Qy=150、Qψ=24**。
4. **结论**：最大巡航 ≈ **2.5 m/s**（e≤5.4cm、ψ≤4.5°）；舒适区 ≤ **2.0 m/s**；3.0 m/s 失败。

---

## 0.6 「stage0 的 PD 对正逻辑」现状调查结论（用户询问项）

- 该 PD 逻辑 = `vision_bridge_control.c:353` 的 `vision_bridge_calc_visual_err_degree()`：横向乘性 PID `ω = lat_kp·e·v + lat_kd·ė`（`lat_kp=6`/`lat_kd=6`，基于 b2_line 前视横向误差 `filtered_lateral_m`，右正）。
- **现状**：只在 **RUN 阶段**被调用（`:918`，条件「前 1.2m 或准备脱出 stage=2」且 `center_filter_valid`），用于桥上循线。
- **stage0（准备进入，对应 `VISION_BRIDGE_TASK_ALIGN`）不再调用它**：`:829-831` 注释「stage0 = PVC + IMU: 方向只来自 IMU 锁角」，方向 = `vision_bridge_calc_yaw_hold_err_degree()`。
- 结论：该 PD 对正在 stage0 已被纯锁角取代，只残留在 RUN 循线段。**本次 LQR 正是把进入段（stage0）升级替换回来**——用 LQR 替代「纯锁角」，PD 逻辑保持现状不动（RUN 循线仍用它）。

---

## 1. 目标与范围

用 trials 的 **LQR 状态反馈律 `ω = k1·e + k2(v)·ψ_err`** 替换以下两处「进入 PVC 时」的**方向控制逻辑**（仅方向，不碰速度）：

| 场景 | 文件 | 状态机阶段 | 现状方向律 | 替换后方向律 |
|---|---|---|---|---|
| 单边桥上桥 | `code/vision/vision_bridge_control.c` | `VISION_BRIDGE_TASK_ALIGN` | 纯 IMU 锁角 | 视觉段（D≤1.5m）LQR ↔ 盲区段锁角保向 |
| 坡道进入 | `code/vision/vision_slope_control.c` | `VISION_SLOPE_TASK_PVC_ALIGN` | 复用 PVC 纯追踪 `err_degree_cmd` | 视觉段 LQR（e=D·sin(β+ψ)） |

**明确不动的部分（冻结）**：
- `target_speed_set` 与所有速度宏——**速度完全归路径/导航管理，v_set 恒定 = 巡航设定**。
- 底层转向链（`cm7_0_isr.c` 转向角环、`TURN_ANG_KP=-8`、`pid-new.h`）。
- 上桥/进坡**状态转移判据**保留现状（§6.3 可选扩展）。
- RUN 阶段的 PD 循线逻辑（`vision_bridge_calc_visual_err_degree`）**不动**。

---

## 2. 现状基线（已核实到源码）

### 2.1 bridge ALIGN 阶段（`vision_bridge_control.c:806-854`）
- 方向：`err_cmd = vision_bridge_calc_yaw_hold_err_degree()`（纯 IMU 锁角，`err_source=1`）。
- 速度：`VISION_BRIDGE_TASK_RUN_SPEED_SET`（-200，冻结）。
- 上桥判据：`b2_valid && b2_gate` / 惯导门 `ON_BRIDGE_TRIGGER_MM=900` / 超时兜底。
- 数据源：bridge V2 管线（`b2_*`）。

### 2.2 slope PVC_ALIGN 阶段（`vision_slope_control.c:247-292`）
- 方向：`err_cmd = g_vision_pvc_control_status.err_degree_cmd`（PVC 底层纯追踪）。
- 速度：`VISION_SLOPE_TASK_PVC_ALIGN_SPEED_SET`（-500，冻结）。
- 进坡判据：`pvc_stable_detected && bbox_area_ratio_u16>=400` 连续 50ms → `ENTRY_HOLD`；超时 → `FAILSAFE`。
- 数据源：标准 PVC 管线（`pvc_*`）。

### 2.3 数据源关键事实
- **单边桥对准阶段 1 核已在运行专用 PVC**（`bridge_pvc_vision`，由 `bridge_fusion.c`「准备进入」阶段每帧调用）。
- **唯一缺口**：专用 PVC 的 `phy_x_mm/phy_y_mm` 未透传到 0 核 IPC `b2_*` → 需 1 处透传（§4.2）。
- **PVC 自带稳定**：`bridge_pvc_vision` / `pvc_vision` 均有「连续 3 帧确认 + 前视/横向平滑」的 `stable_detected` 输出，`stable.phy_x_mm/phy_y_mm` 可直接用，**无需再滤波**。
- slope 的标准 PVC 管线已把 `pvc_phy_x_mm/phy_y_mm` 透传到 0 核，**无 1 核改动**。

---

## 3. LQR 控制律 → 固件接口映射（只输出方向）

### 3.1 公式落地

```
// 输入（只读）
ψ_deg    = inertial_nav.relative_yaw                  // IMU 航向（-180~180）
ψs_deg   = entry_yaw（bridge=entry_yaw_deg / slope=entry_yaw_deg，进入状态机时刻锁存）
v_mps    = fabsf(inertial_nav.vx_body)/1000.0f        // 车身速度（互补滤波，只读，仅 k2 调度）

// 误差
ψ_err_deg = normalize_angle(ψs_deg − ψ_deg)           // 与锁角函数同源同号
ψ_err_rad = ψ_err_deg · DEG2RAD
β_rad     = atan2f(phy_x_mm, phy_y_mm)                // 桥唇/入口方位角（IPM 车体系，右正）
D_m       = sqrtf(phy_x²+phy_y²) / 1000.0f
e_m       = D_m · sinf(β_rad + ψ_rad)                 // 横向偏差重建（ψ_rad = relative_yaw 弧度）

// LQR 增益（CARE 闭式解，R=1 归一）
k1 = sqrtf(LQR_QY)                                    // Qy=150 → ≈12.25
k2 = sqrtf(2·max(v_mps,0.3)·k1 + LQR_QPSI)            // Qψ=24，随 v 调度

// 输出（只写 err_degree）
视觉段:  ω_radps = k1·e_m + k2·ψ_err_rad
盲区段:  ω_radps = LQR_KLOCK · ψ_err_rad             // 锁角保向 K_lock=1.8（bridge）/ slope 回退 0
ω_radps = clamp(ω_radps, −LQR_W_MAX, +LQR_W_MAX)      // W_MAX=2.2 rad/s
err_degree = ω_radps · 57.29578f / TURN_ANG_KP        // TURN_ANG_KP=−8（带符号）
err_degree = clamp(err_degree, −LQR_ERR_MAX_DEG, +LQR_ERR_MAX_DEG)
```

### 3.2 符号锚定（替代翻转宏，用户要求：算法不应出错）

LQR 的符号**不由宏翻转**，而是逐通道锚定到现有已实车验证的逻辑：

| 通道 | 定义 | 锚定基准 | 结论 |
|---|---|---|---|
| 航向 ψ_err | `normalize(ψ存储 − relative_yaw)` | **与现有锁角函数 `vision_bridge_calc_yaw_hold_err()` 完全同源同号** | 航向通道符号零偏差（锁角已验证收敛） |
| 横向 e | `D·sin(β + ψ)`，β=atan2(phy_x,phy_y)（IPM 右正） | **与现有视觉 PD 的 `filtered_lateral_m`（IPM x 差，右正）同号** | e 右正（车偏左时 e>0） |
| 换算 | `err_degree = ω×57.29578/TURN_ANG_KP`（−8） | **与现有视觉 PD `vision_bridge_calc_visual_err_degree` 换算一致** | 单位自洽 |

- 由于 ψ_err 通道 = 锁角、e 通道 = 视觉 PD、换算 = 视觉 PD，三条均已实车验证，LQR 合成符号由它们共同保证，**无需任何 `*_SIGN` 宏**。
- **落地注意**：说明书的 `err_degree=ω×57.29578/TURN_ANG_KP` 与现有锁角函数内部用 `|TURN_ANG_KP|`（+8）的换算存在已知符号差。LQR 落地时**以现有视觉 PD 的换算（TURN_ANG_KP=−8）为准**，并让 ψ_err 的定义与锁角严格一致，避免正反馈。
- 实车首上电做一次「打角方向核对」（给正 e 看是否右转、给正 ψ_err 看是否左转）属**标定复核**；若不符，定位到 TURN_ANG_KP/电机接线/`relative_yaw` 正方向等底层问题，**修底层，不加宏**。

### 3.3 单位与换算（已核对）

| 量 | 表达式 | 单位 | 依据 |
|---|---|---|---|
| `err_degree` 落地 | `ω × 57.29578 / (-8.0)` | deg | `pid-new.h:87`；`cm7_0_isr.c:632` |
| LQR 输出钳位 | `±(2.2 × 57.29578 / 8)` ≈ **±15.76°** | deg | W_MAX=2.2 |
| `phy_x/phy_y` | int16，mm，x 向右 y 向前 | mm | IPM 查表 |
| `inertial_nav.relative_yaw` | 度 | deg | `inertial_nav.h` |
| `inertial_nav.vx_body` | mm/s（互补滤波车身速度，仅读 k2 调度） | mm/s | `inertial_nav.h` |

---

## 4. 数据来源方案（精确）

### 4.1 slope —— 零 1 核改动
视觉段用 `packet->pvc_phy_x_mm / pvc_phy_y_mm`；有效 = `pvc_stable_detected && phy≠32767`。

### 4.2 bridge 视觉段 —— 1 处透传
`code1/vision/vision_ipc_core1.c` 的 `vision_ipc_core1_fill_bridge_v2()` 末尾，读 `bridge_pvc_vision_get_output()`（去 volatile 拷贝），把 `stable.phy_x_mm/phy_y_mm` 填入 `packet->pvc_phy_x_mm/phy_y_mm`。IPC 结构零改动。

### 4.3 bridge 盲区段 —— 锁角保向（无数据源改动）
D>1.5m 或视觉无效 → 方向 = `vision_bridge_calc_yaw_hold_err_degree()`（现状锁角），不做里程计横向纠偏。

---

## 5. 逐文件精确变更清单

### 5.1 新增 `code/vision/vision_entry_lqr.h`
- 宏（无任何 `*_SIGN` 翻转宏）：
  - `LQR_QY 150.0f`、`LQR_QPSI 24.0f`
  - `LQR_W_MAX_RADPS 2.2f`、`LQR_ERR_MAX_DEG ≈15.76f`
  - `LQR_V_FLOOR_MPS 0.3f`、`LQR_PHY_INVALID_MM 32767`
  - `LQR_DETECT_RANGE_M 1.5f`、`LQR_KLOCK 1.8f`
- 结构体 `vision_entry_lqr_state_t`：`entry_yaw_deg`、`valid`、`beta_rad`、`dist_m`、`e_m`、`psi_err_rad`、`omega_radps`。
- 函数：
  - `void VisionEntryLqr_Reset(float entry_yaw_deg);`
  - `uint8 VisionEntryLqr_UpdateVision(int16 phy_x_mm, int16 phy_y_mm, float yaw_deg, float v_mps);`
  - `float VisionEntryLqr_GetErrDegree(void);`

### 5.2 新增 `code/vision/vision_entry_lqr.c`
- 视觉段 `e = D·sin(β+ψ)`、增益 `k1/k2`、`err_degree = ω×57.29578/TURN_ANG_KP`、钳 W_MAX；无 `target_speed_set` 引用。

### 5.3 修改 `code/vision/vision_slope_control.c`
- `#include "vision/vision_entry_lqr.h"`；`vision_slope_task_ctx_t` 增 `float entry_yaw_deg;`。
- `vision_slope_enter_task()`：锁 `entry_yaw_deg = inertial_nav.relative_yaw`（**进入状态机时刻**）+ `VisionEntryLqr_Reset(entry_yaw_deg)`。
- `VISION_SLOPE_TASK_PVC_ALIGN` 方向替换为：
  ```c
  if (VisionEntryLqr_UpdateVision(packet->pvc_phy_x_mm, packet->pvc_phy_y_mm,
                                  inertial_nav.relative_yaw, v_mps))
      err_cmd = VisionEntryLqr_GetErrDegree();
  else
      err_cmd = 0.0f;   /* 回退直行搜索（现状，已确认正确） */
  ```
- 速度保持现状；进坡判据保持现状；其余阶段不动。

### 5.4 修改 `code/vision/vision_slope_control.h`
- `vision_slope_task_status_t` 增 `lqr_e_m / lqr_psi_err_deg / lqr_dist_m`。

### 5.5 修改 `code/vision/vision_bridge_control.c`
- `#include "vision/vision_entry_lqr.h"`；`enter_task()` 追加 `VisionEntryLqr_Reset(s_bridge_task.entry_yaw_deg)`。
- `VISION_BRIDGE_TASK_ALIGN` 方向替换为：
  ```c
  if (VisionEntryLqr_UpdateVision(packet->pvc_phy_x_mm, packet->pvc_phy_y_mm,
                                  inertial_nav.relative_yaw, v_mps))
  {
      err_cmd = VisionEntryLqr_GetErrDegree();
      s_bridge_task.err_source = 0U;   /* 视觉 LQR */
  }
  else
  {
      err_cmd = vision_bridge_calc_yaw_hold_err_degree();  /* 盲区锁角（现状，已确认正确） */
      s_bridge_task.err_source = 1U;
  }
  ```
- `apply_err_ramp` 换源限速保留；速度保持现状；上桥判据保持现状；`RUN/EXIT/FINISH/FAILSAFE` 不动（RUN 的 PD 循线不动）。

### 5.6 修改 `code/vision/vision_bridge_control.h`
- `vision_bridge_task_status_t` 增 `lqr_e_m / lqr_psi_err_deg / lqr_dist_m`。

### 5.7 修改 `code1/vision/vision_ipc_core1.c`
- `vision_ipc_core1_fill_bridge_v2()` 末尾追加专用 PVC phy 透传（§4.2）；文件顶部 `#include "bridge_pvc_vision.h"`。

### 5.8 IAR 双核工程
- 新增 `vision_entry_lqr.c/.h`：cm7_0 的 **`ewp` + `ewt` 双加**（历史坑 C7）。
- 1 核仅改 `vision_ipc_core1.c`，无需新增文件条目。

### 5.9 不改的文件（冻结）
`vision_pvc_control.c/.h`、`vision_ipc.h`、`bridge_fusion.*`、`bridge_v2_arbiter.*`、`bridge_output_filter.*`、`bridge_pvc_vision.*`、`pvc_vision.*`、`cm7_0_isr.c`、`pid-new.h`、`inertial_nav.*`、`ipm_transform.*`、`main_cm7_0.c`、`main_cm7_1.c`、`plan3/plan4_*.c`、`sys_options.h`。

---

## 6. 状态机与触发时序

### 6.1 bridge（视觉 LQR ↔ 盲区锁角）
```
IDLE --Start--> ALIGN:  SetBridgeEnable(1)；VisionEntryLqr_Reset(entry_yaw_deg)
ALIGN:  每视觉包: 视觉有效（D≤1.5m 且 phy≠32767）→ UpdateVision → LQR
                  否则 → 锁角保向（现状）
        err_degree 经 apply_err_ramp
        speed 保持现状
        判据 b2_valid&&b2_gate / 惯导门900mm / 超时 → RUN（现状）
RUN/EXIT/FINISH: 不变
```

### 6.2 slope（视觉段 LQR）
```
IDLE --Start--> PVC_ALIGN:  SetPvcEnable(1)；VisionEntryLqr_Reset(entry_yaw)
PVC_ALIGN: 视觉有效 → UpdateVision → LQR；否则 err=0（直行搜索）
           speed 保持现状；判据 稳定+bbox占比≥40% 连续50ms → ENTRY_HOLD（现状）
其余阶段不变
```

### 6.3 可选扩展（默认不做）
- 上桥判据是否引入「`|e|≤8cm && |ψ|≤5°`」附加确认——默认不引入。

---

## 7. 参数宏清单（`vision_entry_lqr.h`）

| 宏 | 值 | 含义 | 调参 |
|---|---|---|---|
| `LQR_QY` | 150.0f | k1=√Qy（横向权重） | 唇口 e 大→+10~25；过冲/ω 贴 2.2→−10~25；上限自觉线≈200，下限≥100 |
| `LQR_QPSI` | 24.0f | 航向权重 | ψ唇>3°→+2~4；航向抖动→−2~4 |
| `LQR_DETECT_RANGE_M` | 1.5f | 视觉段检测距离 | 现场自行标定 |
| `LQR_W_MAX_RADPS` | 2.2f | ω 钳位 | 外部标定 |
| `LQR_ERR_MAX_DEG` | ≈15.76f | err_degree 钳位 | 随 W_MAX |
| `LQR_V_FLOOR_MPS` | 0.3f | k2 内 max(v,0.3) | 一般不调 |
| `LQR_KLOCK` | 1.8f | 盲区锁角增益 | 盲区只保向，不调 |

**调参纪律（文档 B §3）**：
1. **先 Qψ 后 Qy**：短窗架构航向是稀缺资源，横向其次。
2. **在目标最高速度下调**。
3. 恶化方向 = ω 饱和段变长 → 方向反了，回退。
4. 不动的参数：`K_lock=1.8`、`drop=0.8`（物理测量值）、内环 PD；`W_SLEW` 必须 ≥6 rad/s²（命门，现役 9 勿动）。

---

## 8. 诊断 / 上位机可见性
- bridge：`[BridgeCtrl]` 串口追加 `e=%.3f psi=%.1f D=%.2f lqr=%u`。
- slope：`vision_slope_publish_status()` 追加 `lqr_*` 三字段。
- 只动 `LQR_QY` / `LQR_QPSI` 两个旋钮。

---

## 9. 风险点与待确认项

1. **W_SLEW ≥ 6 rad/s² 命门**：slew 减半到 4.5 时高速段崩（39/60）；现役 9 有 50% 裕度，**内环 PD 勿动**。
2. **符号（已按推导锚定，不留宏）**：ψ_err 同锁角、e 同视觉 PD、换算同视觉 PD；实车首上电做一次打角方向复核，不符则修底层（§3.2）。
3. **ψ存储语义（已定）**：bridge/slope 均用 **entry_yaw（进入状态机时刻 yaw）**，与说明书 ψ_存储 一致。
4. **LQR 无效回退（已确认正确）**：slope 回退 0（直行搜索）、bridge 回退锁角，按现状保留。
5. **PVC 稳定（已确认）**：`bridge_pvc_vision`/`pvc_vision` 自带连续 3 帧确认 + 平滑，直接读 `stable.phy_x/phy_y`，无需额外滤波；旁路透传读最近一帧 PVC 即可。
6. **v 来源（已定）**：`inertial_nav.vx_body`（互补滤波车身速度），仅 k2 调度只读。
7. **检测距离标定**：1.5m 为定稿值，现场由用户自行标定。
8. **IAR 双核工程**：新增 `vision_entry_lqr.c/.h` 需 cm7_0 的 `ewp`+`ewt` 双加；1 核仅改 `vision_ipc_core1.c`。

---

## 10. 分阶段实施步骤（审批通过后执行）

1. **P0 共享模块**：新增 `vision_entry_lqr.c/.h`（视觉段 LQR）；IAR cm7_0 `ewp`+`ewt` 双加；编译 0 错。
2. **P1 slope 落地**：改 `vision_slope_control.c/.h`；编译验证；仅 1 核上位机看 `lqr_*`。
3. **P2 bridge 落地**：改 `vision_ipc_core1.c`（phy 旁路透传）+ `vision_bridge_control.c/.h`；编译双核。
4. **P3 实车标定**：`LQR_QY/QPSI`、打角方向复核、检测距离复核。
5. **P4 回退验证**：每步留 git 提交点。

---

## 11. 回退方案

- **L1（参数级）**：把 LQR 有效判定恒置 0（等效回退现状锁角/纯追踪）。
- **L2（代码级）**：`git checkout` 恢复 P0~P2 各提交点，逐个回退。
