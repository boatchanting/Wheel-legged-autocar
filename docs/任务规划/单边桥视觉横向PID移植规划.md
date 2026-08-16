# 单边桥方向控制移植规划（v2：按视觉状态机分阶段）

> 状态：**仅文档讨论，严禁改动代码**
> 日期：2026-08-16
> 参照：`D:\WORKS\2026LunTui\trials\track.html`（stage1 v8 循迹）、`D:\WORKS\2026LunTui\trials\index.html`（stage0/锁角）
> 目标文件：`code/vision/vision_bridge_control.c/.h`、`code/calculate/pid-new.h`（底层 TURN_ANG_KP 引用）

---

## 0. 结论速览

按视觉状态机 `b2_mode & B2M_STAGE_MASK` 分阶段：

| 视觉状态机 | 阶段名 | 跑的视觉算法 | 方向控制逻辑 | 参照 |
|---|---|---|---|---|
| 0 | PREPARE_ENTER | PVC（入口检测） | **IMU 锁角**（目标角来源用宏开关二选一） | index.html 的锁角 |
| 1 | ON_BRIDGE | v8（完整中线） | **track.html 横向 PID**（乘性速度自适应） | track.html |
| 2 | PREPARE_EXIT | ref（脱出线） | **IMU 锁 entry_yaw 冲出**（沿用现有） | 现有 EXIT |

最终输出仍走 **`err_degree(deg) → 底层转向角环 Turn_Angle`**（已确认不绕开角度环）。因此仿真里的 `P=6/I=0/D=6`、`Kψ=1.8` 需要按"ω→err_degree"做一次换算（第 5 节）。

**最重要的结论（速度尺度）**：仿真 Kp=6 是在车速 ≈3 m/s 下调的；实车桥上 `*_SPEED_SET=-300` 是**转速档**，换算线速度 = 300×`SPEED_TO_MM_S(4.79)` ≈ **1437 mm/s ≈ 1.44 m/s**（高速上桥）。乘性形式 `ω=Kp·e·v` 的横向力度随 v 线性缩放，**同样 Kp=6 在 1.44 m/s 下约是仿真的 0.48 倍**，需把 Kp/Kd 按 ≈2× 放大（见第 8 节调节指南）。这是移植后第一个要现场重调的点。

---

## 1. 两个参考仿真的控制律（只取与本任务相关的部分）

### 1.1 track.html（stage1，v8 循迹）——默认 kp=6, ki=0, kd=6, kyaw=1.8

```
有视觉(循迹):
    e  = 前视点处赛道中点的横向误差 (m)
    ė  = e 的帧差分 + 低通(α=0.25) + 限幅 ±3 m/s
    v  = 实测车速 (m/s)
    ω  = (Kp·e + Ki·∫e)·v + Kd·ė      // 乘性自适应, Kp=6, Ki=0, Kd=6
丢线(记忆回正):
    ω  = Kψ · ψ_err                  // 纯 P, Kψ=1.8 (rad/s per rad)
```

### 1.2 index.html（stage0 参照；本任务只取"锁角"与"宏开关"两个概念）

```
桥上/丢线锁角:
    ω = Kψh · ψ_err                  // 纯 P, Kψh(klock)=1.8 (rad/s per rad)
```

index.html 里另一条"视觉 β/D 重建 e + IMU 航向锚定 ω=Kp·e·v+Kψ·ψ_err"的循迹律**不采用**——因为 stage0 视觉算法是 PVC（只做入口检测），方向控制角完全由 IMU 提供（用户明确）。

---

## 2. 当前嵌入式代码的控制模型（与上一版相同，作为差异基线）

```
视觉路径(stage 有效时):
  θ = filtered_heading_deg(deg)
  err_v = LINE_SIGN·θ = −θ            (限幅 ±16°)
  δ = Kp_v·err_v + Kd_v·d(err_v)/dt   (800/1450 mm/s 线性插值调度, Ki=0)
  err_degree = ramp(δ)  (0.5°/2ms)

锁角路径:
  ψ_err = normalize(locked_yaw − relative_yaw)  (deg, 限幅 ±10°)
  δ = 同一套 Kp_v/Kd_v 的 PD(ψ_err)
  err_degree = ramp(δ)

下游(两路径共用):
  ω_target[°/s] = TURN_ANG_KP·err_degree = −8·err_degree
  → Turn_Gyro(°/s→PWM)
```

---

## 3. 与当前代码的关键差异总表

| 维度 | 当前代码 | 移植后（本版） |
|---|---|---|
| 阶段划分 | ALIGN/RUN/EXIT 三态 | 保持三态，但按 `b2_mode` 阶段决定算法与转向源 |
| stage0 转向 | 视觉有效时用 ref 中线 θ 转向 | **仅 IMU 锁角**（PVC 不提供转向） |
| stage1 转向 | 视觉 θ 角度 PD（查表调度） | **e 横向乘性 PID**（track.html，无查表） |
| stage2 转向 | IMU 锁 entry_yaw | IMU 锁 entry_yaw（不变） |
| 视觉误差量 | 几何夹角 θ(deg) | 横向误差 e(m)（由 IPM x 差直接求，无需 θ→e 反解） |
| 视觉控制律 | Kp_v·θ + Kd_v·θ̇ | (Kp·e+Ki·∫e)·v + Kd·ė |
| 输出接口 | err_degree(deg) | **err_degree(deg)**（保留，经 ω→err_degree 换算） |
| 速度调度 | 800/1450 插值 | 乘性自适应内建（v 因子），删查表 |
| 锁角控制 | 与视觉共用 PD | 独立纯 P（Kψ_lock） |
| 微分信号 | dθ/dt（帧间隔差分） | ė（帧差分 + α=0.25 低通 + ±3 限幅） |
| stage0 锁角目标 | 固定 entry_yaw | 宏开关二选一（entry_yaw / 路表 target_yaw） |

---

## 4. 数学分析：锁角路径与视觉路径的"接口不一致"

两条路径最终都输出 `err_degree`，但**误差物理量不同、稳态目标不同、速度缩放不同**。这是换源表现差异的根因。

### 4.1 几何关系（核心）

对直参考线，设 ψ=航向偏差、e0=车体横向偏移、Ld=前视距离、θ=视觉视线角，小角度：

```
θ = ψ + e0/Ld
```

而 track.html 里的 e 是"前视点处横向误差"，记为 e_L：

```
e_L = e0 + Ld·ψ = Ld·θ
```

即 **仿真里的 e（前视横向误差）与当前代码的 θ 只差一个 Ld 尺度**：`e_L = Ld·θ`。本移植不绕这个弯——嵌入式直接用 IPM 的物理坐标差求 e_L（见 5.1），不需要反解 e0。

### 4.2 稳态目标不同（换源 = 换被控目标）

- 视觉路径（stage1）：`e_L → 0` ⇒ `e0 = −Ld·ψ`，车以"对准前视点"方式收敛，最终**贴线 e0→0**。
- 锁角路径（stage0/2、丢线）：`ψ → 0` ⇒ 车头回正，但 **e0 保持原值**（横向开环）。

例：Ld=0.7m，丢线瞬间 ψ=0 但 e0=5cm ⇒ `e_L = 5cm`，视觉看到误差会修正，锁角看到 0° 会直线冲出、把 5cm 偏移原样保留。单边桥上半宽约 12.5cm，这 5cm 就是压边风险。**ramp 只平滑数值，不平滑目标跳变。**

### 4.3 增益等效性

对 e_L 施加增益 K（视觉）：`K·e_L = K·Ld·ψ + K·e0`，即同时反馈 ψ 与 e0；锁角只有 `K·ψ`，**无横向通道**。因此两条路径不能共用同一套增益，必须分开（本版已分开：视觉 Kp/Kd，锁角 Kψ_lock）。

### 4.4 闭环阶次

运动学 `ẏ=vψ, ψ̇=ω`（y=e0）：

- 视觉（P 主导）：二阶，`s² + G·s + v·G/Ld = 0`，横向收敛，随 v 变化。
- 锁角（P）：一阶，`ψ̇ = −G·ψ`，横向开环。

换源 = 二阶↔一阶极点跳变，输出 ramp 无法消除，只能靠"两路径输出同量纲（本版已满足）+ 换源 slew"缓解。

### 4.5 速度缩放不一致（本版新增的重点）

- 视觉路径力度 ∝ `Kp·e_L·v`，随 v 线性缩放。
- 锁角路径力度 ∝ `Kψ_lock·ψ`，与 v 无关。

实车桥上 ≈1.44 m/s（`*_SPEED_SET=-300` 转速档 ×4.79；仿真 3 m/s），因此视觉通道力度约为仿真 0.48 倍，换到锁角通道时“软硬感”不同。这需要现场按第 8 节重新标定 Kp（或锁角增益），不是照抄仿真值。

### 4.6 小结

> 接口不一致是结构性的：两条路径被控量、稳态目标、阶次、速度缩放都不同。**本版的对策是：输出统一为 err_degree（同量纲）、两条路径增益独立、换源走 slew；但"视觉贴线 / 锁角保向"的目标差异无法消除，只能接受，并在丢线策略上做保护（第 10 节）。**

---

## 5. 移植后的控制律（保持 err_degree 接口）

约定：`TURN_ANG_KP = −8`（`pid-new.h`），`R2D = 180/π`。

### 5.1 stage1（v8，视觉有效）横向乘性 PID

```
e   = (target.x_mm − reference.x_mm) / 1000.0f        // 前视横向误差, m（IPM 直接给）
ė   = 帧差 e + 低通(α=edot_alpha) + 限幅 ±edot_clamp   // 仅新视觉包更新
v   = fabsf(inertial_nav.vx_body) / 1000.0f           // m/s；vx_body 单位 mm/s（SPEED_TO_MM_S 换算），前进符号不统一故取绝对值
∫e  = clamp(∫e + e·dt, ±lat_int_max)                  // Ki=0 时恒 0

ω_radps = LAT_SIGN·Kp·e·v + EDOT_SIGN·Kd·ė            // Ki=0
err_degree = ω_radps · R2D / TURN_ANG_KP
           = −(R2D/8) · ω_radps
```

默认（仿真）：`Kp=6, Ki=0, Kd=6`，`LAT_SIGN=+1, EDOT_SIGN=+1`（符号见第 8 节审计）。

### 5.2 stage0/stage2（IMU 锁角）纯 P

```
ψ_err_deg = normalize(ψ_target − inertial_nav.relative_yaw)   // 与当前代码一致
err_degree = YAWHOLD_SIGN · Kψ_lock · ψ_err_deg
Kψ_lock = yaw_hold_kp / |TURN_ANG_KP| = 1.8 / 8 = 0.225        // (° per °)
```

- `ψ_target` 来源由 `yaw_hold_src_sel` 宏开关决定：
  - `0 = ENTRY`：进入任务时刻 `entry_yaw_deg`（当前行为）。
  - `1 = ROUTE`：路表当前点 `target_yaw`（nav_ram/nav_replay 提供，逐点更新）。
- `YAWHOLD_SIGN = +1` 默认与当前代码同号（当前 `err_degree ≈ Kp_v·ψ_err`，Kp_v≈1.0；新值 0.225 更软，需现场确认）。

### 5.3 stage2（PREPARE_EXIT）保持现有

沿用现有 `IMU 锁 entry_yaw 冲出桥区`，即 5.2 公式且 `ψ_target = entry_yaw_deg`（`yaw_hold_src_sel` 强制按 ENTRY）。

### 5.4 换源/输出限幅

- 两条路径最终都产出 `err_degree`，换源时对 `err_degree` 做 slew（沿用现有 `ramp_step_deg_per_2ms`，默认 0.5°/2ms）。
- 输出限幅 `out_max_deg`（默认 22.9° ≈ 仿真 3.2 rad/s 经 ÷8 换算），小于底层角度环 ±45° 输入限幅，不触发饱和。

---

## 6. 外部宏与符号清单（接口核对）

以下为本规划引用的**全部外部符号**，移植前务必逐一核对（尤其单位与符号）。本文件内部的 `vision_bridge_tune_t` 面板不算外部。

### 6.1 底层转向环（code/calculate/pid-new.h / pid-new.c）

| 符号 | 定义位置 | 值 | 单位/含义 |
|---|---|---|---|
| TURN_ANG_KP | pid-new.h | −8.0f | 转向角环 Kp；`turn_angle_loop_out[°/s] = TURN_ANG_KP · err_degree[deg]` |
| TURN_ANG_KD | pid-new.h | 0.0f | 转向角环 Kd |
| TURN_ANG_MAX_O | pid-new.h | 8000.0f | 转向角环输出限幅(°/s) |
| TURN_GYR_KP | pid-new.h | 20.0f | 转向角速度环 Kp |
| TURN_GYR_KD | pid-new.h | 16.0f | 转向角速度环 Kd |
| TURN_GYR_MAX_O | pid-new.h | 8000.0f | 角速度环输出限幅(PWM) |
| TURN_GYR_MAX_O_BRIDGE | pid-new.h | 9000.0f | 单边桥角速度环输出限幅(PWM) |
| target_speed_set | pid-new.c:67 | volatile float | 目标速度，单位=转速档；**×SPEED_TO_MM_S(4.79)=mm/s** |
| turn_angle_loop_out | pid-new.c:77 | volatile float | 角度环输出=期望角速度(°/s) |
| Turn_Angle_Loop_Control(angle_error) | pid-new.c:1376 | 函数 | 入参 deg，返回 °/s |
| Turn_Gyro_Loop_Control(target_gyro, actual_gyro) | pid-new.c:1444 | 函数 | 入参 °/s，返回 PWM |

### 6.2 惯导（code/navigation/inertial_nav.h / inertial_nav.c）

| 符号 | 值 | 单位/含义 |
|---|---|---|
| inertial_nav.vx_body | — | 纵向速度 **mm/s**（header 注释“前进为正”，工程多处前进为负，使用时 fabsf） |
| inertial_nav.relative_yaw | — | 相对初始方向的偏航角 **deg，[-180,180]** |
| SPEED_TO_MM_S | 4.79f | 速度档→mm/s 换算（车轮半径系数） |
| NAV_DT | 0.01f | 惯导解算周期 10ms |
| NAV_ALPHA_VEL | 1.0f | 纵向速度融合系数（1.0=全信轮速） |
| WHEEL_BASE_MM | 175.0f | 轮距 mm |

### 6.3 视觉 IPC（code/vision/vision_ipc.h / vision_ipc_core0.h）

| 符号 | 值 | 含义 |
|---|---|---|
| B2M_STAGE_MASK | 0x07U | b2_mode 低 3 位=阶段 |
| B2M_STAGE_PREPARE_ENTER | 0x00U | 阶段0=PVC |
| B2M_STAGE_ON_BRIDGE | 0x01U | 阶段1=v8 |
| B2M_STAGE_PREPARE_EXIT | 0x02U | 阶段2=ref 脱出线 |
| VisionIpc_Core0_GetLatest() | 函数 | 返回 `const volatile vision_ipc_packet_t*` |

packet 相关字段（`vision_ipc_packet_t`，94×60 图像系，线 `x=a·y+b`）：

| 字段 | 类型 | 含义 |
|---|---|---|
| b2_valid | uint8 | 本帧控制线可信(0=失能) |
| b2_mode | uint8 | 位掩码（低3位=阶段，高4位=检出） |
| b2_gate | uint8 | 桥面底部变白锁存 |
| b2_has_top | uint8 | 脱出线有效 |
| b2_line_u_lo / b2_line_u_hi | uint8 | 控制线支撑 y 范围 |
| b2_line_a_x1000 | int16 | 控制线斜率 a×1000 |
| b2_line_b_x100 | int16 | 控制线截距 b×100 |
| b2_top_a_x1000 / b2_top_b_x100 | int16 | 脱出线系数 |

### 6.4 IPM 查表（code1/vision/ipm_transform.h）

| 符号 | 值 | 含义 |
|---|---|---|
| IPM_IMG_WIDTH | 94 | 图像宽 |
| IPM_IMG_HEIGHT | 60 | 图像高 |
| IPM_Point_t.x_mm | int16 | 物理横向坐标 **向右为正** |
| IPM_Point_t.y_mm | int16 | 物理纵向坐标 **向前为正** |
| IPM_GetPhysicalCoord(img_x, img_y) | 函数 | 像素→物理(mm)，含 is_valid |

> 本规划用 `e = (target.x_mm − reference.x_mm)/1000` 求前视横向误差(m)。其中 target=前视点，reference=图像中心列(IMAGE_CENTER_X=47)同行的物理点；两者 y_mm 同为前视距离。

### 6.5 0核 ISR 消费侧（user/cm7_0_isr.c）

| 符号 | 位置 | 值/含义 |
|---|---|---|
| err_degree | :73 | volatile float，转向角误差(deg)，由本模块写入 |
| 角度环周期 | :615 | `loop_counter%3==1`，3ms |
| yaw_error 处理 | :632-660 | `yaw_error=err_degree` → wrap ±180 → **clamp ±45°** → Turn_Angle_Loop_Control |
| current_actual_speed | :494 | `=0.5f*(right_speed−left_speed)`，单位同 target_speed_set（转速档） |

### 6.6 本文件内部宏（code/vision/vision_bridge_control.h，移植时替换/引用）

| 宏 | 值 | 含义 |
|---|---|---|
| VISION_BRIDGE_TASK_RUN_SPEED_SET | −300.0f | 桥上正常跑（转速档 → 1437 mm/s ≈ **1.44 m/s**） |
| VISION_BRIDGE_TASK_BRIDGE_SPEED_SET | −300.0f | 看见黑块（同上） |
| VISION_BRIDGE_TASK_BLIND_SPEED_SET | −300.0f | 盲跑 |
| VISION_BRIDGE_TASK_EXIT_SPEED_SET | −300.0f | 下桥缓冲 |
| VISION_BRIDGE_TASK_LINE_SIGN | −1.0f | 旧 θ 通道符号 |
| VISION_BRIDGE_TASK_MAX_ERR_DEG | 16.0f | 旧视觉输出限幅 |
| VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG | 10.0f | 锁角输入限幅 |
| VISION_BRIDGE_TASK_ERR_RAMP_STEP_DEG | 0.5f | 换源 ramp |
| VISION_BRIDGE_TASK_IMAGE_CENTER_X | 47.0f | 直行对应图像中心 |
| VISION_BRIDGE_TASK_LOOKAHEAD_Y | 25U | 前视控制行 |
| VISION_BRIDGE_TASK_VALID_LOST_FRAMES | 8U | 失能连续帧回锁角 |
| VISION_BRIDGE_TASK_VALID_RECOVER_FRAMES | 4U | 恢复连续帧回视觉 |

### 6.7 关键单位结论（务必确认）

- `target_speed_set` 与 `current_actual_speed` 是**转速档**（不是 mm/s）：`mm/s = 档位 × SPEED_TO_MM_S(4.79)`。
- 因此桥上 `RUN_SPEED_SET=-300` → **1437 mm/s ≈ 1.44 m/s**，是**高速上桥**（与你的判断一致）。
- `inertial_nav.vx_body` 是 **mm/s**，除以 1000 即 m/s，可直接用于乘性项 `ω=Kp·e·v`。
- 仿真 3 m/s vs 实车 1.44 m/s：乘性力度比 ≈ 0.48，Kp/Kd 需按 ≈2× 放大（不是之前的 10×）。

---

## 7. 头文件参数面板（完整、可调节）

以下为建议放在 `vision_bridge_control.h` 的结构体 + 宏（仅规划，不落地代码）。

```c
/* ================= 单边桥方向控制可调参数面板 (v2) =================
 * 参照: trials/track.html (stage1), trials/index.html (锁角)
 * 单位: 物理公式用 SI (m, m/s, rad/s); err_degree 落地域用 deg
 * 关键换算: err_degree = ω_radps·(180/π) / TURN_ANG_KP,  TURN_ANG_KP = −8
 * ================================================================== */
typedef struct
{
    /* ---- stage1 (v8 循迹) 横向乘性 PID ---- */
    float lat_kp;                /* 6.0   [1/m²]   ω_P = Kp·e·v */
    float lat_ki;                /* 0.0   [1/(m²·s)] ω_I = Ki·∫e·v (默认关) */
    float lat_kd;                /* 6.0   [1/m]    ω_D = Kd·ė */
    float lat_int_max;           /* 3.0   [m·s]    ∫e 限幅 */
    uint8 lat_adaptive_enable;   /* 1     1=乘性 ω=(Kp·e+Ki·∫e)·v+Kd·ė
                                   *       0=固定 ω=Kp·e+Kd·ė+Ki·∫e */

    /* ---- ė 微分滤波 (复刻 track.html) ---- */
    float edot_alpha;            /* 0.25  ė += α·(dRaw − ė) */
    float edot_clamp_mps;        /* 3.0   [m/s]  ė 限幅 */
    float edot_fps;              /* 30.0  [Hz]   视觉帧率(微分节拍) */

    /* ---- 前视 ---- */
    float lookahead_m;           /* 1.0   [m] 文档/限幅用; e 由 IPM x 差直接求 */

    /* ---- 锁角 (IMU, stage0/丢线/stage2 共用) ---- */
    float yaw_hold_kp;           /* 1.8   [rad/s per rad] 仿真锁角增益 */
    uint8 yaw_hold_src_sel;      /* 0     0=entry_yaw(进状态机锁角) 1=路表当前点target_yaw */

    /* ---- 输出与限幅 (err_degree 落地域) ---- */
    float out_max_deg;           /* 22.9  [deg] err_degree 输出限幅(≈3.2rad/s÷8·R2D) */
    float ramp_step_deg_per_2ms; /* 0.5   [deg] 换源/输出变化率(每2ms) */

    /* ---- 符号通道 (现场翻转, 勿改逻辑) ---- */
    float lat_sign;              /* +1.0  横向通道符号 */
    float edot_sign;             /* +1.0  D 通道符号 */
    float yaw_hold_sign;         /* +1.0  锁角通道符号 */
} vision_bridge_tune_t;

extern const vision_bridge_tune_t g_vision_bridge_tune_defaults;

/* 宏: 锁角目标源选择 */
#define VISION_BRIDGE_YAWHOLD_SRC_ENTRY  (0U)
#define VISION_BRIDGE_YAWHOLD_SRC_ROUTE  (1U)
/* 宏: 底层转向角环 Kp 引用(换算用), 定义在 pid-new.h: TURN_ANG_KP = -8.0f */
#define VISION_BRIDGE_TURN_ANG_KP_REF     (TURN_ANG_KP)
```

### 7.1 面板字段说明表

| 字段 | 默认 | 单位 | 来源 | 建议范围 | 说明 |
|---|---|---|---|---|---|
| lat_kp | 6.0 | 1/m² | track.html kp | 0~30 | 横向 P；乘性下按速度比≈2×放大（见 8.1） |
| lat_ki | 0.0 | 1/(m²·s) | track.html ki | 0~3 | 默认 0，不积分 |
| lat_kd | 6.0 | 1/m | track.html kd | 0~30 | 横向 D，作用在 ė 上 |
| lat_int_max | 3.0 | m·s | track.html | — | ∫e 抗饱和限幅 |
| lat_adaptive_enable | 1 | — | track.html adaptive | 0/1 | 1=乘性，0=固定增益 |
| edot_alpha | 0.25 | — | track.html | 0.1~0.5 | ė 低通系数 |
| edot_clamp_mps | 3.0 | m/s | track.html | 1~5 | ė 限幅，防量化跳变放大 |
| edot_fps | 30.0 | Hz | track.html | — | 视觉帧率，微分节拍 |
| lookahead_m | 1.0 | m | track.html look | — | 前视参考（文档/限幅） |
| yaw_hold_kp | 1.8 | rad/s per rad | index.html klock | 0.5~5 | 锁角增益；落地位 Kψ_lock=1.8/8=0.225 |
| yaw_hold_src_sel | 0 | — | 用户要求 | 0/1 | 0=entry_yaw，1=路表 target_yaw |
| out_max_deg | 22.9 | deg | 仿真 3.2rad/s | 10~45 | err_degree 输出限幅 |
| ramp_step_deg_per_2ms | 0.5 | deg | 现有 | 0.2~1.5 | 换源 slew |
| lat_sign / edot_sign / yaw_hold_sign | +1 | — | 符号审计 | ±1 | 现场反向时翻转 |

---

## 8. 调节指南

### 8.0 第一步：先做符号审计（不先做，一切调参都白费）

抬车/低速、逐通道验证，看前轮方向：

1. 让视觉看到线偏左（`e > 0`，即 target.x − reference.x > 0）→ 前轮应朝**右**打。若反，翻转 `lat_sign`。
2. 让线快速横向移动（制造 ė）→ D 项应起阻尼（抑制摆动）。若反而助摆，翻转 `edot_sign`。
3. 原地转动车头使 `relative_yaw` 偏离目标 → 前轮应朝回正方向打。若反，翻转 `yaw_hold_sign`。
4. 确认 `vx_body` 前进为负：`v = fabsf(vx_body)/1000` 必须为正，否则乘性项变号。

> 提醒：本版把 ω 除以 `TURN_ANG_KP(−8)` 落到 err_degree，这一除号已经隐含一次符号翻转。因此**不要凭纸面推导定符号，以 1~3 步实测为准**。

### 8.1 第二步：先定 Kp 的量级（速度尺度是最大坑）

- 仿真 `Kp=6` 在 v≈3 m/s 下有效，横向力度 ∝ `Kp·e·v`。
- 实车桥上 `target_speed_set=-300`（转速档）→ 300×`SPEED_TO_MM_S(4.79)` ≈ **1437 mm/s ≈ 1.44 m/s**（高速上桥）。
- 同样 Kp=6 时力度约为仿真的 1.44/3 ≈ 0.48 倍，**需要把 Kp 按 ≈2× 放大**（起步 `Kp≈12`），而非 10×。
- **推荐起步**：保持乘性（`lat_adaptive_enable=1`），`Kp≈12` 起步再下调；或用固定增益模式（`lat_adaptive_enable=0`）以 `Kp=6` 起步（固定模式下力度与车速无关）。
- 目标：5cm 横向偏差时，前轮应有肉眼可见、但不猛的回正动作（参考当前代码约 4° 的 err_degree 量级）。

### 8.2 第三步：调 Kd

- 先 `Kd=0`，把 Kp 调到"轻微过冲/微摆"。
- 再加 Kd 抑制摆动，直到"收敛快、不抖"。仿真 Kd=6 同样有速度尺度问题（ė∝v），低速下需按需放大。
- 若视觉帧率不稳导致 D 项抖，先降 `edot_alpha`（更平滑）或 `edot_clamp_mps`（更小限幅），再调 Kd。

### 8.3 第四步：调锁角 Kψ_lock

- 起步 `yaw_hold_kp=1.8`（落地位 0.225°/°）。
- 丢线后应"平稳回正、不蛇形"。过软→加大；过冲振荡→减小。
- 与当前代码（Kp_v≈1.0）相比，0.225 明显更软，预计需要往上调或现场验证。

### 8.4 第五步：换源 slew 与目标源

- `ramp_step_deg_per_2ms`：换源瞬间若方向跳变明显，调小；若回正太拖，调大。
- `yaw_hold_src_sel`：先用 `0=entry_yaw`（与现状一致）；若进场阶段需要跟着规划路线走，再切 `1=路表 target_yaw` 验证。

### 8.5 第六步：三阶段回归

按 `stage0(锁角) → 上桥 → stage1(循迹) → stage2(锁角冲出)` 完整跑，重点看两个换源点（0→1 上桥、1→2 脱出）是否平稳、丢线时是否直线带偏。

---

## 9. 与当前代码的逐项改动点清单

| # | 当前代码 | 移植后 |
|---|---|---|
| 1 | `vision_bridge_calc_geometry_err_degree()` 输出 θ(deg) | 改为横向误差：`e=(target.x_mm−reference.x_mm)/1000` + ė 滤波 |
| 2 | `vision_bridge_pd_calc()/pd_get_gains()` 角度 PD + 插值调度 | 改为横向乘性 PID，删查表；`err_degree = ω·R2D/TURN_ANG_KP` |
| 3 | `vision_bridge_calc_yaw_hold_err()` 输出 ψ 并走同一 PD | 改为纯 P：`err_degree = YAWHOLD_SIGN·Kψ_lock·ψ_err` |
| 4 | `vision_bridge_apply_err_ramp()` | 保留，作用于 err_degree（沿用 0.5°/2ms） |
| 5 | `VISION_BRIDGE_TASK_PD_*` 角度增益宏 | 替换为 `vision_bridge_tune_t` 面板 + 换算宏 |
| 6 | stage0 ALIGN：视觉有效时用 θ 转向 | 改为仅 IMU 锁角（PVC 不提供转向），目标角走 `yaw_hold_src_sel` |
| 7 | stage1 RUN：前 1.2m 视觉 θ、超距锁角 | 视觉段改 e 乘性 PID；丢线/超距仍锁角 |
| 8 | 速度调度 | 乘性内建，删 800/1450 插值 |
| 9 | 视觉源仅新包更新 PD | 保留：e/ė 仅新包更新；锁角每 2ms 更新 |
| 10 | 锁角目标锁定时刻改为 entry_yaw | 保留 ENTRY 语义；新增 ROUTE 宏开关 |

---

## 10. 风险与待确认

1. **符号链**：`vx_body` 前进为负、`relative_yaw` 顺时针为正、`TURN_ANG_KP=−8` 的符号链必须按 8.0 实测，不能纸面推导。
2. **e 的求法**：`e=(target.x_mm−reference.x_mm)/1000` 依赖 IPM 在前视行给出物理 x；需确认 stage1(v8) 时 `b2_line_*` 的 IPM 查表在前视行有效。
3. **PVC 阶段**：stage0 仅用 PVC 的入口检测（`b2_gate`），转向完全 IMU；需确认 PVC 期间 `center_filter_valid` 不会被误置为"视觉转向可用"。
4. **速度尺度**：Kp/Kd 的 6/6 是 3 m/s 仿真值；实车桥上 ≈1.44 m/s（高速上桥），按 ≈2× 放大 Kp/Kd（见 8.1），否则横向力度不足。
5. **Kψ_lock 量级**：仿真锁角 1.8 → 落地位 0.225°/°，比当前约 1.0 软很多，需现场验证。
6. **路表 target_yaw 访问**：`yaw_hold_src_sel=1` 需要确认从 nav_ram/nav_replay 取当前点 target_yaw 的具体接口。
7. **换源目标跳变**：视觉贴线 ↔ 锁角保向的目标差异无法消除；建议丢线瞬间记录 e 方向，或在桥上"保向优先"，接受横向开环（仿真同样如此）。

---

## 11. 分步落地（仅描述，不写代码）

1. **符号与单位冻结**：按 8.0 实测并固定各通道符号、vx_body 符号、IPM x 差求 e 的方向。
2. **只改 vision_bridge_control.c**：落地 `err_degree = ω_radps·R2D/TURN_ANG_KP`，底层不动，先验证 e/ė/ω/err_degree 数值合理。
3. **stage1 视觉换 e 乘性 PID**，删查表调度。
4. **stage0/stage2 换独立纯 P 锁角**，加 `yaw_hold_src_sel` 宏开关。
5. **参数逼近**：按第 8 节顺序，先符号、再 Kp 量级、再 Kd、再锁角、再 slew。
6. **三阶段回归**：完整跑通 stage0→1→2 与丢线回退，再固化宏/结构体默认值。

---

*本文档只做方案与数学讨论，未改动任何源码。*

