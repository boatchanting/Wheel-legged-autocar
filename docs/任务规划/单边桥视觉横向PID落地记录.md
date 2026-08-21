# 单边桥视觉横向 PID —— 实际落地记录

> 状态：**已落地（代码已改）**，本文件记录实际改动、与规划的差异、验证情况与待办验证。
> 日期：2026-08-16
> 规划依据：`docs/任务规划/单边桥视觉横向PID移植规划.md`
> 改动文件：
> - `code/vision/vision_bridge_control.h`
> - `code/vision/vision_bridge_control.c`

---

## 1. 落地概览

按规划把单边桥方向控制从"几何夹角 θ 直接给底层"改为**按视觉状态机分阶段的横向乘性 PID + 独立锁角**，但**保持 `err_degree → 底层转向角环` 接口不变**（已确认）。

状态机阶段与方向控制映射（实际落地）：

| 视觉状态机 `b2_mode` 低3位 | 阶段 | 方向控制 |
|---|---|---|
| 0 = PREPARE_ENTER | ALIGN（进场/PVC） | **IMU 锁角**（PVC 只做入口检测，不提供转向） |
| 1 = ON_BRIDGE | RUN（桥上/v8） | **横向乘性 PID**（有中线用视觉，丢线锁角，超距锁角） |
| 2 = PREPARE_EXIT | RUN 末段 / EXIT | **IMU 锁 entry_yaw 冲出**（沿用现有） |

---

## 2. 头文件改动（vision_bridge_control.h）

1. 删除 `VISION_BRIDGE_TASK_PD_SPEED_LOW_MM_S` 等一组"速度调度 PD"宏（`4.5` 节）。
2. 新增 `4.5` 节：方向控制可调参数面板 `vision_bridge_tune_t` 结构体 + 宏：

```c
typedef struct {
    float lat_kp;               /* 6.0   横向 P */
    float lat_ki;               /* 0.0   横向 I (保留, 未启用) */
    float lat_kd;               /* 6.0   横向 D */
    float lat_int_max;          /* 3.0   ∫e 限幅 */
    uint8 lat_adaptive_enable;  /* 1     1=乘性 0=固定 */
    float edot_alpha;           /* 0.25  ė 低通 */
    float edot_clamp_mps;       /* 3.0   ė 限幅 m/s */
    float edot_fps;             /* 30.0  微分节拍 Hz */
    float lookahead_m;          /* 1.0   前视(文档用) */
    float yaw_hold_kp;          /* 1.8   锁角增益 rad/s per rad */
    uint8 yaw_hold_src_sel;     /* 0     0=entry_yaw 1=路表 */
    float out_max_deg;          /* 22.9  输出限幅 deg */
    float ramp_step_deg_per_2ms;/* 0.5   换源 slew */
    float lat_sign;             /* +1.0  横向符号 */
    float edot_sign;            /* +1.0  D 符号 */
    float yaw_hold_sign;        /* +1.0  锁角符号 */
} vision_bridge_tune_t;

#define VISION_BRIDGE_YAWHOLD_SRC_ENTRY  (0U)
#define VISION_BRIDGE_YAWHOLD_SRC_ROUTE  (1U)
#define VISION_BRIDGE_TURN_ANG_KP_REF     (TURN_ANG_KP)   /* = -8.0f, pid-new.h */
```

3. `vision_bridge_task_status_t` 增加两个调试字段：`filtered_lateral_m`、`edot_mps`。
4. 新增 `extern const vision_bridge_tune_t g_vision_bridge_tune_defaults;`。

---

## 3. 源文件改动（vision_bridge_control.c）

### 3.1 参数面板默认值（`g_vision_bridge_tune_defaults`）

在 `s_bridge_task` 之后定义，值同第 2 节注释（`lat_kp=6, lat_ki=0, lat_kd=6, yaw_hold_kp=1.8, out_max_deg=22.9, ...`），全部与规划一致。

### 3.2 测量函数扩展：新增横向误差 e

`vision_bridge_get_control_measurement()` 增加第 4 个出参 `lateral_m`：

```c
*lateral_m = (float)(target_point.x_mm - reference_point.x_mm) / 1000.0f;  /* m, 向右为正 */
```

即 `e = (前视点物理x − 中心列同行参考点物理x)/1000`，直接用 IPM 物理坐标差，无需 θ→e 反解。

### 3.3 中心滤波：滤波对象增加 e，并计算 ė

`vision_bridge_update_center_filter()` 保持原有有效性滞回（8 丢 / 4 恢复 + 跳变抑制），在此基础上：

- 接受新视觉包时，同步低通 `filtered_lateral_m`（α = `VISION_BRIDGE_TASK_CENTER_FILTER_ALPHA` = 0.40，用于 P 项）；
- 用**原始 e 帧差**求 ė：

```c
d_raw = clamp((lateral_m - last_lateral_m) * edot_fps, -edot_clamp, +edot_clamp);
edot_mps += edot_alpha * (d_raw - edot_mps);   /* α=0.25 */
```

- 首次/重捕获时 `edot_has_history=0`、`edot_mps=0`，防微分冲击。

### 3.4 视觉控制函数（替代原几何角函数）

`vision_bridge_calc_visual_err_degree()`：

```c
v = fabsf(inertial_nav.vx_body) / 1000.0f;          /* m/s */
ω_radps = lat_sign·lat_kp·e·v + edot_sign·lat_kd·ė;  /* 乘性; Ki 未启用 */
err_degree = ω_radps · (180/π) / TURN_ANG_KP;        /* ÷(−8) */
clamp(err_degree, ±out_max_deg);
```

固定增益模式（`lat_adaptive_enable=0`）时 P 项不乘 v。

### 3.5 锁角控制函数（独立纯 P）

- `vision_bridge_yaw_hold_target_deg()`：目标航向按 `yaw_hold_src_sel` 选择，默认 ENTRY 返回 `locked_yaw_deg`（= 进入任务时刻 yaw）。
- `vision_bridge_calc_yaw_hold_err()`：返回 `ψ_err = normalize(目标 − relative_yaw)`，限幅 ±10°。
- `vision_bridge_calc_yaw_hold_err_degree()`：

```c
err_degree = yaw_hold_sign · (yaw_hold_kp/|TURN_ANG_KP|) · ψ_err;  /* = 0.225·ψ_err */
clamp(err_degree, ±out_max_deg);
```

### 3.6 状态机

- **ALIGN**：改为**仅 IMU 锁角**（原"视觉有效时用 θ 转向"已删除）；对齐判据改用 `ψ_err ≤ ALIGN_YAW_TOL_DEG`。
- **RUN**：
  - 脱出阶段（stage2）→ 锁角；
  - `traveled ≤ 1.1m` 且有可靠中线 → `vision_bridge_calc_visual_err_degree()`（视觉），否则锁角；
  - `> 1.1m` → 锁角 + 速度 ×2（沿用）。
- **EXIT**：锁角（`_degree()` 版）。
- `vision_bridge_apply_err_ramp()` 的步长改用面板 `ramp_step_deg_per_2ms`。
- `vision_bridge_publish_status()` 发布 `filtered_lateral_m`、`edot_mps`。
- 调试 printf 增加 `e=` 与 `ed=` 字段。

---

## 4. 与规划文档的差异 / 偏差说明（重要）

| 项 | 规划 | 实际落地 | 说明 |
|---|---|---|---|
| Ki | lat_ki=0，面板保留 | **未实现积分项** | Ki=0 时 ∫e 无意义，代码未加累加器；面板字段保留 |
| stage0 锁角目标源 ROUTE | 宏开关接入路表 target_yaw | **ENRY 已实现，ROUTE 为桩** | 桥任务期间 nav_replay 暂停，无统一 current index 接口（规划风险项 6）；ROUTE 分支当前回退 `locked_yaw_deg`，留 TODO |
| e 的滤波 | 规划说"滤波对象改为 e/ė" | e 用中心滤波 α=0.40 低通（P 项），ė 用 α=0.25 低通 | 与 track.html 略不同（track.html P 用原始 e）；低通 e 更稳，属合理落地选择 |
| 符号宏 | 默认 +1 | 默认 +1 | **需现场按符号审计翻转**（见第 6 节） |

---

## 5. 已完成验证

1. **静态一致性检查**：逐段重读 `.c/.h`，确认：
   - 旧函数 `vision_bridge_calc_geometry_err_degree` 已无引用；
   - 新函数 `calc_visual_err_degree` / `calc_yaw_hold_err_degree` 在 ALIGN/RUN/EXIT 正确接线；
   - `vision_bridge_get_control_measurement` 唯一调用点已改为 4 参数；
   - `g_vision_bridge_tune_defaults` 定义与头文件 extern 匹配；
   - 状态结构体新增字段与 publish 赋值一致。
2. **IDE 语法/诊断检查**：`get_errors` 对两个文件返回 **No errors found**。
3. **旧宏引用扫描**：`VISION_BRIDGE_TASK_PD_*` 已无代码引用（仅规划文档提及）。
4. **IAR 命令行编译（Core0，通过）**：
   - 工具：`D:\tools\IAR Systems\Embedded Workbench 9.2\common\bin\iarbuild.exe`
   - 命令：`iarbuild "iar\project_config\cyt4bb7_cm_7_0.ewp" -build Debug -parallel 8`
   - 结果：**Total number of errors: 0，Build succeeded**；14 条 warning 均为改动前已存在的未用变量/函数与 `PI` 宏重定义，`vision_bridge_control.c/.h` 无任何新增警告/错误。
   - 说明：`vision_bridge_tune_t` 的 C99 指定初始化器被 IAR 9.2 正常接受。

**未做**：实车/台架符号审计与调参（本环境无硬件）。

---

## 6. 待用户执行的验证（必须）

1. **Core1 编译（如需）**：本次改动仅在 Core0（`vision_bridge_control.c/.h` 属 0 核）；Core1 工程 `iar\project_config\cyt4bb7_cm_7_1.ewp` 不引用本模块，无需重编。若全量回归，按同命令编译 Core1 即可。
2. **符号审计**（抬车，逐通道）：
   - 视觉线偏右（`e>0`）→ 前轮应朝左打（实际以回正为准），反向则翻转 `lat_sign`；
   - 制造横向相对运动 → D 项应起阻尼，助摆则翻转 `edot_sign`；
   - 转动车头使 `relative_yaw` 偏离目标 → 前轮应回正，反向则翻转 `yaw_hold_sign`；
   - 确认 `vx_body` 前进符号（取 `fabsf` 已规避符号，但乘性项要求 v 恒正）。
3. **现场调参**（按规划调节指南）：
   - 先定 Kp 量级：仿真 3 m/s 调出 6，实车 1.44 m/s，乘性力度约 0.48 倍，Kp 起步 ≈12；
   - 再 Kd、再锁角 Kψ_lock（0.225 偏软，需验证）、再换源 slew。
4. **三阶段回归**：stage0 锁角进场 → 上桥 → stage1 视觉循迹 → stage2 锁角冲出；重点看两个换源点与丢线表现。
5. **路表 ROUTE 源接入（可选）**：如要启用 `yaw_hold_src_sel=1`，在 `vision_bridge_yaw_hold_target_deg()` 的 TODO 处接入路表当前点 `target_yaw_deg`。

---

## 7. 风险提示（落地后仍成立）

- 视觉贴线（横向收敛）↔ 锁角保向（横向开环）的目标差异无法消除：丢线瞬间若已有横向偏差，锁角会把它原样保留，单边桥上存在压边风险。
- `out_max_deg=22.9°` 对应底层 `err_degree` 输入 ±45° 限幅内，安全；但底层角度环 `TURN_ANG_KP=-8` 若被改动，会破坏 `err_degree = ω·R2D/TURN_ANG_KP` 的换算（已用 `VISION_BRIDGE_TURN_ANG_KP_REF` 引用，改 pid-new.h 需同步确认）。
