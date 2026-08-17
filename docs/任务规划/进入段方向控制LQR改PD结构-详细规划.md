# 进入段方向控制：LQR 改 PD 结构 —— 详细代码更改规划（v1）

> 状态：**已评审通过并执行完成（2026-08-18）**
> 分支：`0818【单边桥】进入逻辑回归稳定pid`（当前 HEAD = `0e7e636`）
> 日期：2026-08-18
> 配套仿真：`tools/03_控制与仿真/lqr_vs_pd_enter_sim.py`、`lqr_vs_pd_validate.py`、`d_sensitivity.py`

## ✅ 执行记录（2026-08-18）

| 提交 | 内容 |
|---|---|
| `29bd46b` | cherry-pick 合并 f87b18b 三个 LQR 修复文件（e 公式 / QY=120/QPSI=32 / locked_yaw 基准） |
| `0e7e636` | 双模式改造：`ENTRY_CTRL_MODE_PD`（默认 K_E=11/P=7/D=1.0，微分滤波 50ms）与 `ENTRY_CTRL_MODE_LQR` 一键切换 |

- cherry-pick 冲突仅出现在被丢弃的 4 个坡道特有文件上（sys_options.h / nav_replay_route_table.h / figures4papers / 科目四 py），已按 §4.5 还原，LQR 三文件干净应用；
- PD / LQR 双模式均通过编译检查（零错误）；
- 推荐参数仿真复验：K_E=11 / P=7 / D=1.0（滤波 50ms）6 工况 × 5 种子全完成，横向峰值 323mm 与 LQR 一致，航向摆动 23.4°→13.8°，收敛慢 0.3s；
- 未跟踪文件：`tools/03_控制与仿真/*.py`（3 个仿真脚本）与本文档，未提交。

---

## 0. 结论速览（评审用）

| 项 | 内容 |
|---|---|
| 新控制律 | `ω = P·ψ_err + D·ψ_err' + K_E·e`（航向 PD + 视觉横向偏差作为扰动） |
| 推荐参数 | **K_E=11.0，P=7.0，D=1.0**（`D` 可调 0 ~ 3.5，甜点 ~0.1~0.25×P） |
| D=5~10×P | **仿真证明不可行**（详见 §3.3，含数学证明与数据） |
| 改动文件 | 仅 `vision_entry_lqr.c` / `vision_entry_lqr.h`（内部改造）+ `vision_slope_control.c`（1 行 bug 修复） |
| 双模式切换 | `ENTRY_CTRL_MODE` 宏一键切换 **PD（默认）/ LQR（保留）**，LQR 原逻辑不删除 |
| LQR 修复版来源 | **已决策：git 合并引入 f87b18b 修复**（cherry-pick 三个修复文件，见 §4.5），非手动重写 |
| 对外接口 | **零改动**：bridge / slope 两个调用方完全不用动 |
| 内建修复 | 新代码直接写入修复后的横向偏差公式 `e=D·sin(β−ψ_err)`（规避历史致命 bug） |

---

## 1. 背景与动机

### 1.1 现状（当前工作区实测）
当前分支 `0818【单边桥】进入逻辑回归稳定pid` 的 `vision_entry_lqr.c/.h` 是 **LQR 原始实现**，实测确认：

- `vision_entry_lqr.c:93`：`e_m = dist_m * sinf(beta_rad + yaw_rad)` —— **横向偏差公式仍为 bug 版**（绝对航向污染，见 f87b18b 修复分析）；
- `vision_entry_lqr.h`：`LQR_QY=150.0f`、`LQR_QPSI=24.0f` —— 原始参数；
- `vision_slope_control.c:115`：`locked_yaw_deg = inertial_nav.relative_yaw` —— **锁角基准 bug 仍在**（应在 PVC 确认瞬间锁定"进入时刻"航向，而非截取当时瞬时航向）。

### 1.2 目标
1. 放弃 LQR（CARE 闭式解、`k2` 随速度调度的黑盒感不利于现场调参），改为**直观的 PD + 视觉扰动结构**：
   - 航向角度经 PD 一直维持在"自适应回归的正确角度"（即进入时刻锁存的基准航向 `entry_yaw`）；
   - 视觉横向偏差 `e` 作为**扰动项**注入（乘以扰动强度放大系数 `K_E`）；
   - 调参旋钮：`K_E`（扰动放大）、`P`（航向比例）、`D`（航向微分），含义直观、逐个可调。
2. 通过仿真找出一组能**基本复刻当前 LQR 行为**的 PD 参数作为起点。
3. 改造过程中**直接内建** f87b18b 的两个修复（e 公式 + 锁角基准），避免重新引入历史 bug。
4. 输出极其详细的改动规划，评审通过前不改代码。
5. **保留 LQR 不删除，双模式宏切换**：`ENTRY_CTRL_MODE` 一键在 PD / LQR 之间切换，便于现场 A/B 对比与快速回退。
6. **LQR 分支采用 f87b18b 已验证修复版，通过 git 合并引入**（cherry-pick 三个修复文件，而非手动重写），保证与已验证行为逐字一致。

---

## 2. 新控制律设计

### 2.1 公式（量纲：ω 为 rad/s，e 为 m，ψ_err 为 rad）

> 本节公式为 **PD 模式**（`ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD`）下的控制律；LQR 模式走 §4.3(c) 中保留的 `#else` 分支，两模式通过 `ENTRY_CTRL_MODE` 宏切换。

```
// 输入（只读，每 2ms 控制周期执行；视觉 IPC 数据采样保持）
ψ_deg   = inertial_nav.relative_yaw              // IMU 航向
ψs_deg  = entry_yaw_deg                          // 进入状态机时刻锁存的基准航向
(β, D)  = (atan2(phy_x, phy_y), hypot(phy_x,phy_y)/1000)   // IPM 车体系方位/距离

// 误差
ψ_err_deg = normalize(ψs_deg − ψ_deg)
ψ_err_rad = ψ_err_deg · deg2rad
e_m       = D · sin(β − ψ_err_rad)               // 横向偏差（修复后公式，投影到入口基准系）

// 航向微分（2ms 差分 + 一阶低通，防 IMU 噪声放大）
ψ_err_dot = (ψ_err_rad − ψ_err_prev_rad) / 0.002
ψ_err_dot_f += α · (ψ_err_dot − ψ_err_dot_f)      // α = DT/(PD_D_TAU_S + DT)

// PD + 视觉扰动
ω = P_PSI · ψ_err_rad + D_PSI · ψ_err_dot_f + K_E · e_m
ω = clamp(ω, −W_MAX, +W_MAX)                     // W_MAX=2.2 rad/s

// 输出（接口不变）
err_degree = clamp(ω · 57.29578 / TURN_ANG_KP, −ERR_MAX_DEG, +ERR_MAX_DEG)   // TURN_ANG_KP=−8
```

### 2.2 与 LQR 的对应关系（为什么能"基本复刻"）

| LQR（现役） | PD 结构（新） | 说明 |
|---|---|---|
| `k1 = √Qy` ≈ 10.95（Qy=120） | **K_E** ≈ 11.0 | 视觉横向通道增益，一一对应 |
| `k2 = √(2·max(v,0.3)·k1+Qψ)` ≈ 6.55@0.5m/s、8.71@2.0m/s | **P** ≈ 7.0（固定） | 航向通道增益；LQR 随 v 调度，PD 取固定折中 |
| （无） | **D** ≈ 0 ~ 1.0 | 新增航向阻尼项，抑制 ω 抖动/航向摆动 |
| e 公式（bug 版） | e = D·sin(β−ψ_err)（修复版） | 新代码直接内建修复 |

> 高速（2.0 m/s）下横向通道主导，固定 P 与 LQR 的速度调度差异可忽略（仿真验证，见 §3.2 场景 E/F）。

### 2.3 IPM 视觉扰动数据链路确认（✅ 已完整实现，改造无需动视觉侧）

新控制律的视觉扰动输入依赖 `phy_x_mm / phy_y_mm`（IPM 物理坐标）。经逐环节核对，**整条链路已实现且完整**：

1. **IPM 查表**（`code1/vision/ipm_transform.c`）：预标定逆透视表 `ipm_table[60][94][2]`（`[y][x][0]=X(mm)`、`[y][x][1]=Y(mm)`），`IPM_GetPhysicalCoord(img_x, img_y)` 完成像素→物理坐标转换，含越界保护与天空/无效区校验（`IPM_INVALID_VAL=32767`），返回 `is_valid` 标志；
2. **1 核视觉管线**（两条均已做 IPM 查表）：
   - 坡道 `code1/vision/pvc_vision.c:91`：`ipm_point = IPM_GetPhysicalCoord(img_x, ymax)` → `phy_x_mm/phy_y_mm`；
   - 单边桥 `code1/vision/bridge_pvc_vision.c:92`：同式 → `phy_x_mm/phy_y_mm`；
   - 查表失败/越界时填 `PVC_VISION_PHY_INVALID_MM / BRIDGE_PVC_VISION_PHY_INVALID_MM`（均=32767）；
3. **1 核 IPC 打包**（`code1/vision/vision_ipc_core1.c`）：
   - `fill_pvc`（line 119-120）：PVC 管线输出（`stable` 优先于 `raw`）→ `packet->pvc_phy_x_mm/y_mm`；
   - `fill_bridge_v2`（line 176-177）：单边桥专用 PVC 的 stable 坐标**旁路透传**覆盖（注释明确"LQR 方向控制用"）；
4. **0 核 IPC 结构**（`code/vision/vision_ipc.h:106-107`）：`int16 pvc_phy_x_mm; int16 pvc_phy_y_mm;`；
5. **0 核控制器读取**：bridge（`vision_bridge_control.c:838`）与 slope（`vision_slope_control.c:266`）均以 `packet->pvc_phy_x_mm/y_mm` 调用 `VisionEntryLqr_UpdateVision`；
6. **有效性值全线一致**：`IPM_INVALID_VAL = PVC_VISION_PHY_INVALID_MM = BRIDGE_PVC_VISION_PHY_INVALID_MM = LQR_PHY_INVALID_MM = 32767`，无效帧会被 `UpdateVision` 识别并触发调用方回退（bridge→盲区锁角，slope→直行搜索）。

**结论**：改造为 PD 结构时，视觉侧（1 核）、IPC 透传、0 核读取**全部零改动**；`e = D·sin(β − ψ_err)` 中的 `β/D` 直接由现成的 `pvc_phy_x_mm / pvc_phy_y_mm` 计算。

---

## 3. 仿真验证结论（决策依据）

### 3.1 仿真模型（与固件约束一致）
- 2ms 控制周期；视觉 30ms 采样 + 1 帧延迟 + 高斯噪声（10/15mm）；IMU 航向白噪声 0.4°；
- 转向执行器：一阶惯性 τ=0.03s + slew 9 rad/s² + |ω|≤2.2 rad/s；
- 视觉检测距离 1.5m；进入段结束判据 D<0.4m（模拟 PVC 确认压上）；
- 6 个标准工况 × 5 个噪声种子；更极端工况（0.5m 偏差 / 10° 航向偏 / 1.0m/s）作鲁棒性验证。

### 3.2 推荐参数（K_E=11, P=7, D=0）复刻效果

| 工况 | LQR 收敛 | PD 收敛 | LQR peak_ψ | PD peak_ψ | 与 LQR 轨迹差 |
|---|---|---|---|---|---|
| A 偏右0.3m 对准 | 2.04s | 2.31s | 38.0° | 33.4° | 0.056 |
| B 对准 航向+5° | 0.90s | 1.11s | 16.9° | 13.7° | 0.041 |
| C 偏右+航向偏 | 1.17s | 1.41s | 22.6° | 16.9° | 0.052 |
| E 偏右 高速2.0 | 0.57s | 0.57s | 27.0° | 27.0° | 0.006 |
| F 航向偏 高速2.0 | 0.39s | 0.39s | 12.3° | 12.2° | 0.009 |

- 低速场景：收敛慢约 0.1~0.24s（可接受），航向摆动更小；
- 高速场景：**与 LQR 几乎逐点一致**（差 <0.01）；
- 更极端工况（0.5m/10°）与 LQR 趋势一致、全部完成。

### 3.3 D 项专项研究（回答"D 调到 5~10 倍 P"）

**数学证明（决定性）**：在运动学极限下 $ψ_{err}'=-\omega$，故

$$\omega = P\psi_{err} + K_E e - D\omega \;\Rightarrow\; \omega = \frac{P\psi_{err}+K_E e}{1+D}$$

- D 项等价于把有效增益压缩 $(1+D)$ 倍；D=5P=35 时只剩 1/36；
- 大 D 配大增益 **无法补偿**：$\omega=\frac{x(P\psi_{err}+K_E e)}{1+xD}\xrightarrow{x\to\infty}\frac{P\psi_{err}+K_E e}{D}$，与增益无关。

**仿真数据（5 种子均值，K_E=11, P=7）**：

| 配置 | 收敛均值 | 全完成 | peak_e 最差 | ω 抖动 | vsLQR 偏差 |
|---|---|---|---|---|---|
| LQR | 1.33s | ✅ | 525mm | 98.5 | — |
| D=0 | 1.45s | ✅ | 525mm | 90.0 | 0.050 |
| D=0.25P(1.75) | 1.64s | ✅ | 525mm | 27.0 | 0.303 |
| D=1P(7) | 1.91s | ✅ | 525mm | 21.5 | 0.409 |
| **D=5P(35)** | **2.13s** | **❌ 拉不回** | 543mm | 21.2 | 0.463 |
| **D=10P(70)** | **2.04s** | **❌ 拉不回** | 577mm | 21.3 | 0.485 |

- 慢执行器（τ=0.1/0.2s）下大 D 依然无价值（收敛 2.2s、拉不回）；
- 大 D 配增益 ×2/×3 无效（收敛仍 2.1s、拉不回）——印证数学证明；
- **D 甜点区 = 0 ~ 0.5×P（绝对 0~3.5）**：D≈0.75（11%P）收敛 1.54s（≈D=0）且 ω 抖动减半（38.7 vs 86）、peak_ψ 由 30.9°→26.3°；D>50%P 后收敛明显变慢。

**结论**：D 取 5~10×P **不可行**（等效增益被压缩、短窗内横向消不完、部分场景拉不回来）。D 的正确用法是"小量阻尼"，甜点 0.1~0.25×P。用户若追求更强的稳定/阻尼，正路是**转向内环**（现役 slew=9 勿动）或**测量滤波**，而非外环 D 无脑放大。

### 3.4 最终推荐

| 用途 | K_E | P | D | 说明 |
|---|---|---|---|---|
| **主推（默认）** | 11.0 | 7.0 | **1.0** | 复刻 LQR + 明显抑制抖动/摆动，收敛损失 <0.2s |
| 严格复刻 LQR | 11.0 | 7.0 | 0.0 | 与现役行为最接近 |
| 保守起步 | 10.0 | 6.0 | 0.5 | 实车首上电建议从保守值起步，观察后再升 |

---

## 4. 代码更改规划（极其详细）

> ⚠️ 本规划评审通过前，禁止改动以下任何代码。以下为**待评审的改动方案**。

### 4.1 文件改动总览

| 文件 | 改动性质 | 风险 |
|---|---|---|
| `code/vision/vision_entry_lqr.h` | 宏替换 + 注释更新 | 低（仅本模块引用） |
| `code/vision/vision_entry_lqr.c` | 内部计算逻辑替换为 PD | 中（核心逻辑，但接口不变） |
| `code/vision/vision_slope_control.c` | **1 行 bug 修复**（locked_yaw） | 低 |
| `code/vision/vision_bridge_control.c` | **不改** | — |
| `code/config/sys_options.h` | **不改**（SBUS_ACTIVE_POINT 维持当前值） | — |

### 4.2 `vision_entry_lqr.h` 改动

**(a) 双模式切换宏 + 参数宏**：

```c
/* ============================================================
 * 进入段方向控制器：双模式切换（一键 A/B）
 *   ENTRY_CTRL_MODE_PD  —— 新 PD 结构（航向 PD + 视觉扰动）【默认】
 *   ENTRY_CTRL_MODE_LQR —— 原 LQR（保留，用于对比 / 回退）
 * 现场切换：只改 ENTRY_CTRL_MODE 一行，重新编译即可。
 * ============================================================ */
#define ENTRY_CTRL_MODE_PD      (1U)
#define ENTRY_CTRL_MODE_LQR     (0U)
#define ENTRY_CTRL_MODE         (ENTRY_CTRL_MODE_PD)   /* ← 切换点 */

/* ---------- 模式 A：LQR（保留原逻辑；参数取 f87b18b 已验证版） ---------- */
#define LQR_QY                  (120.0f)   /* 原 150 → 已验证 120 */
#define LQR_QPSI                (32.0f)    /* 原 24  → 已验证 32 */

/* ---------- 模式 B：PD + 视觉扰动 ---------- */
#define PD_K_E                  (11.0f)   /* 视觉扰动放大系数（对应原 k1=√Qy，起步 11） */
#define PD_P_PSI                (7.0f)    /* 航向比例（对应原 k2，起步 7） */
#define PD_D_PSI                (1.0f)    /* 航向微分阻尼（起步 1.0，甜点 0~3.5，严禁 >0.5×P） */
#define PD_D_TAU_S              (0.05f)   /* 航向微分一阶低通时间常数(s)，防 IMU 噪声放大 */
#define PD_CTRL_HZ              (500.0f)  /* 控制周期 500Hz=2ms，微分分母（若改周期需同步） */

/* ---------- 两模式共用的物理常数（勿当旋钮） ---------- */
#define PD_W_MAX_RADPS          (2.2f)    /* 极限转向角速度（物理常数，勿动） */
#define PD_ERR_MAX_DEG          (PD_W_MAX_RADPS * 57.29578f / 8.0f)  /* ≈15.76°，勿单独调 */
#define PD_DETECT_RANGE_M       (1.5f)    /* 视觉段检测距离（现场标定） */
#define PD_PHY_INVALID_MM       (32767)   /* IPM 无效标记（固定） */
```

> 备注：原 `LQR_W_MAX_RADPS / LQR_ERR_MAX_DEG / LQR_DETECT_RANGE_M / LQR_PHY_INVALID_MM` 统一收拢为 `PD_*` 共用宏（两模式同一套物理限制）；`LQR_V_FLOOR_MPS=0.3` 仅 LQR 分支用，保留在 `#else` 分支内。`LQR_KLOCK` 当前实现未使用（调用方 bridge 用自身的 `yaw_hold_kp=1.8`），不保留。

**(b) 头文件注释**：文件头"LQR 方向控制器"改为"进入段 PD+视觉扰动方向控制器"；§调参纪律注释更新为 PD 语义（见 §5）。

**(c) 状态结构体**（字段名保持兼容，尾部追加）：

```c
typedef struct
{
    float   entry_yaw_deg;     /* 进入状态机时刻锁存的基准航向（ψs_存储） */
    uint8   valid;             /* 本次更新是否有效 */
    float   beta_rad;          /* 桥唇/入口方位角（IPM 车体系，右正） */
    float   dist_m;            /* 桥唇距离 D */
    float   e_m;               /* 视觉横向偏差（扰动源）e = D·sin(β−ψ_err) */
    float   psi_err_rad;       /* 航向偏差 ψ_err = ψs_存储 − ψ */
    float   psi_err_dot_radps; /* 航向偏差微分（滤波后，诊断用） */
    float   omega_radps;       /* 期望角速度 ω（已钳 W_MAX） */
} vision_entry_lqr_state_t;
```

> 兼容性：`slope` 的 `vision_slope_publish_status` 与 `bridge` 的调试打印只读 `e_m / psi_err_rad / dist_m / omega_radps`，字段名不变即零影响。

**(d) 函数声明**：`Reset / UpdateVision / GetErrDegree / GetState` **签名与语义完全不变**，调用方零改动。仅更新 `UpdateVision` 的参数注释（`v_mps` 在新结构中仅保留为将来速度调度预留，当前计算不使用，可传 0）。

### 4.3 `vision_entry_lqr.c` 改动

**(a) 模块级状态**（文件顶部 static 区；微分相关仅在 PD 模式编译）：

```c
static vision_entry_lqr_state_t s_lqr;
#if (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD)
static float s_psi_err_prev_rad = 0.0f;   /* 上一拍航向误差（微分用） */
static float s_psi_err_dot_f    = 0.0f;   /* 滤波后航向误差微分 */
static uint8 s_first_valid      = 0U;     /* 首个有效视觉帧标志 */
#endif
```

**(b) `VisionEntryLqr_Reset()`**：原 `memset + entry_yaw_deg` 基础上，追加清零 PD 微分状态（`#if` 内：`s_psi_err_prev_rad / s_psi_err_dot_f / s_first_valid`），防止上次任务残留。

**(c) `VisionEntryLqr_UpdateVision()` 核心计算 —— 双模式条件编译**（原 88~99 行）：

```c
/* —— 公共输入（两种模式共用，保持不变） —— */
beta_rad    = atan2f(fx, fy);
psi_err_deg = vision_entry_lqr_normalize_angle(s_lqr.entry_yaw_deg - yaw_deg);
psi_err_rad = psi_err_deg * deg2rad;

#if (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD)
    /* ========== 模式 B：PD + 视觉扰动（默认） ========== */
    e_m = dist_m * sinf(beta_rad - psi_err_rad);      /* 修复后横向偏差 */
    /* 航向微分（2ms 差分 + 一阶低通，防 IMU 噪声放大） */
    if (s_first_valid)
    {
        const float psi_err_dot = (psi_err_rad - s_psi_err_prev_rad) * PD_CTRL_HZ;
        const float alpha = (1.0f / PD_CTRL_HZ) / (PD_D_TAU_S + 1.0f / PD_CTRL_HZ);
        s_psi_err_dot_f += alpha * (psi_err_dot - s_psi_err_dot_f);
    }
    s_psi_err_prev_rad = psi_err_rad;
    s_first_valid = 1U;
    omega = PD_P_PSI * psi_err_rad + PD_D_PSI * s_psi_err_dot_f + PD_K_E * e_m;
#else
    /* ========== 模式 A：LQR（保留原逻辑，参数与 e 公式取 f87b18b 已验证版） ========== */
    e_m = dist_m * sinf(beta_rad - psi_err_rad);      /* 修复后横向偏差 */
    k1 = sqrtf(LQR_QY);
    k2 = sqrtf(2.0f * fmaxf(v_mps, 0.3f) * k1 + LQR_QPSI);
    omega = k1 * e_m + k2 * psi_err_rad;
#endif

/* 共用钳位（两种模式同一物理限制） */
omega = vision_entry_lqr_constrain_f(omega, -PD_W_MAX_RADPS, PD_W_MAX_RADPS);

/* 状态填充：PD 模式追加 psi_err_dot_radps；两种模式均填 omega */
s_lqr.psi_err_dot_radps = (ENTRY_CTRL_MODE == ENTRY_CTRL_MODE_PD) ? s_psi_err_dot_f : 0.0f;
```

> **LQR 分支说明（已决策：采用 f87b18b 修复版，git 合并引入）**：当前工作区的 LQR 是 bug 版（`e=D·sin(β+yaw)`、QY=150/QPSI=24）。LQR 分支直接采用 **f87b18b 已验证的修复版**（`e=D·sin(β−ψ_err)`、QY=120/QPSI=32）。**引入方式走 git 合并（cherry-pick）而非手动重写**，详见 §4.5——保证与实车验证过的行为逐字一致，且 PD 模式与 LQR 模式的 e 公式天然同源。

**(d) `GetErrDegree()`**：不变（`err = ω·57.29578/TURN_ANG_KP`，钳 `±PD_ERR_MAX_DEG`）。两种模式共用同一换算/钳位，调用方无感知。

### 4.4 `vision_slope_control.c` 改动（1 行 bug 修复，与 f87b18b 一致）

`vision_slope_set_state()` 内，`VISION_SLOPE_TASK_ENTRY_HOLD` 分支（当前第 111~116 行）：

```c
/* 改前（bug）：PVC 确认瞬间截取当前航向，与 LQR 基准不一致，产生基准跳变/锁住运动中航向 */
s_slope_task.locked_yaw_deg = inertial_nav.relative_yaw;

/* 改后：统一锁定进入状态机时刻记录的基准航向 */
s_slope_task.locked_yaw_deg = s_slope_task.entry_yaw_deg;
```

> 该行与控制器改造相互独立，但既然本次要动视觉进入段逻辑，必须一并修复（f87b18b 已实车验证这是致命转向角 bug）。

### 4.5 LQR 修复版的引入方式（已决策：git cherry-pick f87b18b 三个修复文件）

**背景**：LQR 分支采用 f87b18b 已验证修复版。`f87b18b` 位于 `【slope】新坡道临时调参分支`，该提交本身只改动 7 个文件（相对父提交 `4b03f28`），其中 3 个是 LQR 修复、其余 4 个是坡道分支特有内容（路线表 / SBUS 配置 / 科目四脚本 / 子模块）。若整体 merge 会把坡道分支全部内容（含 1778 行路线表大改）带进当前单边桥分支——**不采用**。

**拓扑确认（无冲突风险）**：
- `4b03f28` 是 merge 提交，双亲 = `43b5d87` + `7f4c242`（当前 HEAD）；
- 中间提交链 `7f4c242..f87b18b` 只由 `f87b18b` 本尊动过三个 LQR 文件；
- `7f4c242` 与 `4b03f28` 在三个 LQR 文件上**零差异** → cherry-pick 干净应用、无冲突。

**操作步骤**（在当前分支 `0818【单边桥】进入逻辑回归稳定pid` 执行）：

```bash
# 1. 应用 f87b18b 的改动到工作区但不提交（-n）
git cherry-pick -n f87b18b6fe65844dc9eb1766afa8fd06174e1653

# 2. 只保留 LQR 修复的 3 个文件，还原其余 4 个坡道分支特有文件
git restore --source=HEAD --staged --worktree \
    code/config/sys_options.h \
    code/navigation/nav_replay_route_table.h \
    tools/figures4papers \
    "tools/webview_nav_marker科目四/generate_plan4_smooth_path_考虑响应延迟.py"

# 3. 核对：剩余改动应仅为以下 3 个文件（含 f87b18b 的修复）
git status
#     code/vision/vision_entry_lqr.c      —— e 公式修复（4 行）
#     code/vision/vision_entry_lqr.h      —— QY=120/QPSI=32（6 行）
#     code/vision/vision_slope_control.c  —— locked_yaw 基准修复（4 行）

# 4. 核对无误后提交（提交信息注明来源 f87b18b）
git add code/vision/vision_entry_lqr.c code/vision/vision_entry_lqr.h code/vision/vision_slope_control.c
git commit -m "【单边桥】合并 f87b18b 的 LQR 修复（e公式+参数+锁角基准）"
```

> 说明：
> - `git cherry-pick -n` 会同时暂存 7 个文件，步骤 2 的 `git restore --source=HEAD --staged --worktree` 把 4 个不需要的文件恢复到 HEAD（丢弃其改动），最终只保留 3 个修复文件；
> - 若步骤 1 意外出现冲突（理论上不会，见拓扑确认），`git cherry-pick --abort` 一键回滚，不影响工作区其他未跟踪文件；
> - 完成后，§4.2/§4.3 的 PD 改造在**已含修复**的三个文件上叠加进行；LQR 的 `#else` 分支代码与 f87b18b 逐字一致。

### 4.6 不改动的部分（明确边界）
- `vision_bridge_control.c`：调用方式、err_ramp、盲区锁角回退逻辑**全部保持**；
- `vision_slope_control.c` 其余状态机（PVC_ALIGN 判据、ENTRY_HOLD/RUN 速度、退出距离）**全部保持**；
- `config/`、`navigation/`、`plan/`：不改。

---

## 5. 调参指南（写入头文件注释）

> 双模式各自独立调参：**PD 模式**用下表的 `K_E / P / D` 三旋钮；**LQR 模式**仍用 `LQR_QY / LQR_QPSI` 两旋钮。切换宏 `ENTRY_CTRL_MODE` 时两套参数互不影响。
>
> PD 模式只有 **3 个旋钮**：`K_E`（扰动放大）、`P`（航向比例）、`D`（航向微分）。

| 症状 | 动作 |
|---|---|
| 唇口横向偏差收敛慢、接近段迟迟拉不回来 | `K_E` +1~2（先动它，它直接决定横向拉力） |
| 接近段横向过冲 / S 形摆动 / `ω` 持续贴 2.2 rad/s 饱和 | `K_E` −1~2 |
| `ψ_err` 唇口偏大（>3°）但横向正常 | `P` +0.5~1 |
| 航向抖动、过桥后保向段来回摆 | `D` +0.5~1（甜点 0~3.5，**严禁 >0.5×P**，否则增益被压缩拉不回） |
| e、ψ 都差 | 先动 `P`（航向消了 e 才有几何收敛条件），再动 `K_E` |
| 只有高速（≥2.5m/s）振荡、低速正常 | 先 `K_E` −2（视觉通道是高速主因），必要时再 `P` −1 |
| 视觉噪声导致 ω 高频抖 | 增大 `PD_D_TAU_S`（0.05→0.1），或给视觉 e 加滤波（本项目另有 PVC 稳定机制，一般不必） |

**调参纪律**：每次只动一个旋钮；在目标最高速度下调；改完若 `ω` 饱和段变长（持续贴 2.2）说明方向性错误，立即回退。`PD_W_MAX_RADPS`、slew（内环 9）、`TURN_ANG_KP` 为物理/换算常数，**禁止当旋钮**。

---

## 6. 验证计划

### 6.1 仿真复验（改动前先跑）
1. `python tools/03_控制与仿真/lqr_vs_pd_enter_sim.py` —— 确认推荐参数（K_E=11, P=7, D=1.0）六工况全完成、与 LQR 轨迹接近；
2. `python tools/03_控制与仿真/lqr_vs_pd_validate.py` —— 鲁棒性（5 种子 + 极端工况）全通过；
3. `python tools/03_控制与仿真/d_sensitivity.py` —— 确认 D 甜点区结论（防止误调大 D）。

### 6.2 实车验证步骤（上电顺序）
0. **合并验证 + 双模式 A/B**：先编译 `ENTRY_CTRL_MODE_LQR`（即合并进来的 f87b18b 修复版）确认实车行为与坡道分支一致（基线复核）；再切 `ENTRY_CTRL_MODE_PD` 各跑 3 次对比收敛/摆动/抖动；确认 PD 与 LQR 行为接近后，切回默认 PD 模式进入正式调试；
1. **首上电静态核对**（不打方向）：遥控使能 → 触发进入段 → 观察 `err_degree` 是否≈0、`e_m` 随车体左右移动的符号是否正确（车偏右→e>0→应向左打）；与现有视觉 PD 的 `filtered_lateral_m` 同号；
2. **低速 0.5m/s 单工况**：横向偏差 0.3m 起步，确认 1.5m 窗内收敛、无饱和振荡；
3. **高速 2.0m/s**：确认高速不发散（若振荡先降 K_E）；
4. **全科目联调**：单边桥（复用同一模块）+ 坡道各跑 3 次；
5. 用上位机 `[BridgeCtrl]` 串口 `lqr_*` 字段（字段名沿用，避免上位机改动）对比 e/ψ/ω 曲线，确认两种模式行为一致。

### 6.3 回退方案
`git stash` 或新建分支保留现状；由于只改 2 个文件且接口不变，`git checkout -- code/vision/vision_entry_lqr.c code/vision/vision_entry_lqr.h` 即可一键回退。

---

## 7. 风险与说明

1. **D 项与量纲**：本文 D 的量纲是 `ω/(rad/s)`（ψ_err 用 rad）。若现场以"度"为单位代入 D，数值需 ×57.3；**D/P 比值不受单位制影响**，"5~10 倍 P"在任何单位制下都被仿真否定。
2. **速度调度移除**：LQR 的 k2 随 v 调度被固定 P 取代。高速（≥2.5m/s）若表现变差，可选恢复速度调度（把 `P` 做成 `P(v)=P_base·(0.8+0.4·v)` 之类线性表，宏开关默认关闭）——本期不做，仅预留。
3. **视觉扰动微分 K_ED 未启用**：视觉 30ms 采样 + 10/15mm 噪声，对 e 微分噪声放大显著，默认 K_ED=0；如确需，必须与 `PD_D_TAU_S` 同级滤波（本期不做）。
4. **e 公式修复内建**：新代码直接写 `sin(β−ψ_err)`，不再存在绝对航向污染问题；同时同步修复 `locked_yaw_deg` 基准 bug。
5. **命名**：宏/注释改为 PD 语义，但文件名、函数名 `VisionEntryLqr_*`、状态字段名**保持不变**（降低上位机/调用方/文档牵连；如评审认为改名更清晰，可单独评估改名成本）。
6. **双模式并存维护**：两套增益宏/两段计算并存，改参数/逻辑时注意两处同步（A/B 对比时正是需要独立参数，属预期）；条件编译保证任一模式只有一套代码进二进制，无运行时开销。LQR 分支已锁定 f87b18b 已验证版本（git cherry-pick 引入，§4.5），后续勿再手动改回 bug 版。
7. **合并操作安全**：cherry-pick 使用 `-n`（不自动提交），且已确认三个修复文件在父提交与当前 HEAD 间零差异（无冲突）。万一异常，`git cherry-pick --abort` 一键回滚；未跟踪文件（如 `tools/03_控制与仿真/*.py`）不受影响。

---

## 附：配套仿真文件清单
- `tools/03_控制与仿真/lqr_vs_pd_enter_sim.py` —— 主仿真（LQR 基准 + PD 扫描 + 对比图）
- `tools/03_控制与仿真/lqr_vs_pd_validate.py` —— 精细扫描 + 鲁棒性 + D 量程研究
- `tools/03_控制与仿真/d_sensitivity.py` —— 慢执行器敏感性 + 大 D 增益补偿 + D 甜点区
