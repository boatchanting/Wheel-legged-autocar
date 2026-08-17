# 颠簸路段视觉偏差 IPM 解算与单边逆解算方案

> 状态：**待审批**。本文档仅描述现状与方案，审批通过前禁止修改任何代码。
> 日期：2026-08-17（v6：保留 heading_stable 三帧门控；EMA 非零门控、±150mm 容差已确认）
> 相关文档：`颠簸路段视觉跨核接口规划.md`、`颠簸路段新视觉管线落地文档.md`、`颠簸新管线移植内存现状与瘦身方案.md`

---

## 第一部分：现有接入方案详述

### 1.1 总体数据流

```
摄像头 94×60 压缩图 (CM7_1)
  └─ bumpy_pipeline_frame()              横向连通域 + 左右边线提取（像素坐标）
       └─ bumpy_vision_process_camera_frame()  适配层：角度/偏差/IPM
            └─ g_bumpy_vision_output (bumpy_vision_output_t)
                 └─ vision_ipc_core1_fill_bumpy()  打包 → 共享内存 0x28001400 (2ms 心跳)
                      ═══════════ 跨核 IPC ═══════════
                      └─ VisionIpc_Core0_PollResult()  CRC 校验 + seq 去重 (CM7_0)
                           └─ VisionBumpyControl_Update_2ms()  状态机 + PID → err_degree_cmd
                                └─ BumpyRoad_Update_1ms()  仲裁 → 全局 err_degree
                                     └─ 转向角度环(3ms) → 转向角速度环(1ms)
                                          └─ 左右轮差动 PWM（轮足车，无舵机）
```

### 1.2 视觉核（CM7_1）管线现状

**主开关**：`code1/vision/bumpy_vision.h:32` — `BUMPY_USE_NEW_PIPELINE` 当前 = 0（内存审计用，审计完应翻回 1）。**当前实跑旧算法**，新管线代码已落地未启用。

**新管线**（`code1/vision/bumpy_pipeline.c`，输入 94×60）：
1. 7-tap 可分离卷积求 Gx/Gy（`bumpy_conv.c`，汇编内核 ITCM）；
2. p85 分位二值化 + 横向条纹掩膜（条纹方向角 |θ|<20°）；
3. 8 邻域 CCL 连通域 + 每域 PCA 方向角/线性度；
4. 外点提取（每域 x 极值 3 点）+ 跨域剔除；
5. RANSAC 直线拟合（容差 3px，内点 PCA 精化，n≥5 且跨度≥8px）；
6. **帧航向角 `frame_heading()`（:437-452）：全部横向线性连通域方向角的加权圆均值（权重=像素数）**——**条纹倾斜角在划连通域时即已算出，与左右边线是否拟合成功完全无关**；随后用于边线夹角门控（与航向角差 >70° 剔除，:584-589）；
7. 时间验证：连续 3 帧稳定才输出边线 valid。

**关键事实（v3 核实）**：`frame_heading()` 是 `static` 函数，结果存在 `bumpy_pipeline_frame()` 的局部变量 `hdg`（:493, :585）中，**当前未写入输出结构体**——`bumpy_frame_result_t` 只有 `L/R` 两个 `bumpy_line_t`（`bumpy_pipeline.h:75-77`）。即：**角度已算出，只差引出，无需新增任何算法**。

**管线输出**（`bumpy_pipeline.h:67-77`，**纯像素坐标，未做 IPM**）：

```c
typedef struct { int valid; float ang; float cx, cy; int n; } bumpy_line_t;
typedef struct { bumpy_line_t L, R; } bumpy_frame_result_t;   /* 无航向角字段 */
```

**适配层**（`bumpy_vision.c:356-417`）分场景行为：**现状 → 本方案目标** 对照（目标细节见第三部分）：

| 场景 | 字段 | 现状（改动前） | 目标（改动后） |
|---|---|---|---|
| 双侧有效 | yaw_error | **边线角** doubled-angle 平均 → `atan2(-sin,cos)` | **条纹角 `hdg`**（`frame_heading` 引出） |
| 双侧有效 | lateral_mm | `IPM(47,row).x − IPM(cx中点,row).x`（:409），中点随可见边线长度漂移 | 固定 y=40 行交点 IPM，`−(x_L+x_R)/2`；间距自检失败时输出 0 |
| 双侧有效 | meas_valid | 1（IPM 两点均有效时） | **恒 1**（条纹在即 1，与边线解算成败无关） |
| 仅单侧 | yaw_error | **单边线角**直取 | **条纹角 `hdg`** |
| 仅单侧 | lateral_mm | **0** | **单边逆解算 `−(x_edge±500)`** |
| 仅单侧 | meas_valid | **0** | **1** |
| 无线但有条纹 | yaw_error | **0（条纹角已算出却被丢弃）** | **`hdg` 照常输出，参与 0 核接入** |
| 无线但有条纹 | lateral_mm | 0 | 0（**不变**：两侧都没有，偏差输出 0） |
| 无线但有条纹 | meas_valid | 0 | **1**（条纹在） |
| 无线且无条纹 | 全部字段 + meas_valid | 全 0 | 全 0（**不变**：唯一 meas_valid=0 的情况） |

**旧算法**（当前生效，`bumpy_vision.c:418-493`）：188×120 结构张量，`yaw_error = -atan2(dir_x,dir_y)`，**`lateral_mm` 恒 0**（TODO 未实现），`meas_valid = is_bumpy`。

### 1.3 IPM 查表现状

`code1/vision/ipm_transform.h/c`：94×60 专用，`const int16_t ipm_table[60][94][2]`（Flash 22KB）。

- `IPM_GetPhysicalCoord(x, y) → {x_mm, y_mm, is_valid}`（向右为 x+，向前为 y+，无效值 32767）；
- 表内行 0 ≈ 前方 12.6m，行 59 ≈ 前方 0.22m；
- **当前 bumpy 全链路唯一的物理量换算点**：`bumpy_vision.c:405-406`，且只在双侧有效时调用一次。

### 1.4 IPC 契约（已冻结，跨核接口规划 P0~P4 已落地）

`code/vision/vision_ipc.h`：

- 有效位：`VISION_VALID_BUMPY`（bit3，模块有输出）、`VISION_VALID_BUMPY_MEAS`（bit8，本帧测量可信）；
- 字段：`bumpy_detected(u8)`、`bumpy_direction_x/y(float)`、`yaw_error_deg_x100(int16, 0.01°)`、`lateral_mm(int16, mm)`；`forward_mm` 颠簸恒 0；
- 打包 `vision_ipc_core1.c:169-197`：`meas_valid==1` 才置 bit8；`frame_id==0` 或写忙时字段保持 0。

**本方案对契约的语义调整（字段、位数、打包逻辑全部不变）**：bit8 `VISION_VALID_BUMPY_MEAS` 的置位条件从"双侧边线+IPM 有效"放宽为"**条纹角有效（`hdg_valid`）**"——即只标记"条纹也没有"的严重丢失；`bumpy_detected`（bit3 对应的字段）语义不变（= 有有效边线），0 核进入/出口确认逻辑不受影响。

### 1.5 控制核（CM7_0）消费现状

**`code/vision/vision_bumpy_control.c`**（2ms，`cm7_0_isr.c:305`）：

- 状态机 `IDLE/SEARCH/TRACK/STALE`；
- `vision_bumpy_calc_err_degree()`（:55-79）：优先 `yaw_error_deg_x100×0.01×VISION_BUMPY_YAW_SIGN(+1)`，为 0 回退 `-atan2(direction_x, direction_y)`；限幅 ±18°、死区 0.2°；
- PID（Kp=1.0/Ki=0.03/Kd=0.05，输出限幅 ±18°）→ `err_degree_cmd`；
- **现状门控**：仅 `TRACK`（`bumpy_detected=1`）状态跑 PID；`SEARCH`（包有效但未检出）强制 `err_degree_cmd=0`、PID 复位；
- 超时 240ms → STALE：cmd=0、PID 复位、`recorded_lateral_mm` 冻结不清零；
- `meas_valid`（bit8）驱动：`recorded_lateral_mm` EMA(α=0.5，限幅 ±200mm) 记录；相邻帧角度差 ≤1° 连续 3 帧 → `heading_stable=1`；
- 进入/出口：连续 3 帧检出/未检出确认（基于 `bumpy_detected`）。

**`code/plan/bumpy_road.c`**（1ms）仲裁：

- `BumpyRoad_Trigger()` 锁入口航向 + 独占使能（`SetBumpyEnable(1)`）；LQR 方案提前 500mm 触发；
- `BumpyRoad_ApplyYawHold()`（:106-136）：**`heading_stable==1` 才采用** `err_degree_cmd` 实时修朝向（**该三帧稳定门控本方案保留**）；失稳 → 锁失稳前航向（与入口偏差 >10° 才回退锁入口航向）；EMA(α=0.05) 后写全局 `err_degree`；
- 出口：视觉入段确认 + 轮里程 ≥1m 武装 → 出口确认后把融合坐标钉到出口锚点并叠加记录的横向（`lat·sinθ, −lat·cosθ`，符号开关 `BUMPY_LATERAL_OVERLAY_SIGN` 待实车验）；
- 全程 4m 兜底自动退出（不叠加横向）。

**执行链**：`err_degree` → 转向角度环（3ms，限幅 ±45°）→ 转向角速度环（1ms）→ 左右轮 PWM 同向叠加差动（`cm7_0_isr.c:946-947`）。

### 1.6 现状小结

- 跨核契约与 0 核消费链路已完整落地；
- 但 **1 核当前跑旧算法（宏=0），`lateral_mm` 恒 0**，横向记录/出口叠加链路空转；
- 即使翻宏启用新管线，**角度与偏差值仍有三个待改问题（问题 1~3）和一条必须保持的既有语义（保持项）**（见第二部分）。

---

## 第二部分：现存问题

### 问题 1：倾斜角数据源错误——用了边线角，丢了条纹角

适配层当前从**边线拟合结果** `res.L.ang / res.R.ang` 推导角度（`bumpy_vision.c:388-398`）。但条纹倾斜角在管线划横向连通域时就已由 `frame_heading()` 算出（`bumpy_pipeline.c:437-452`），它与边线是否拟合成功无关。现状后果：

- 边线拟合失败（RANSAC 不显著/时间验证不通过）但条纹清晰时，**明明算出了角度却被丢弃**，输出 0；
- 边线角经 RANSAC + 时间验证，环节多、易断；条纹角是连通域直出，链路短、更稳。

### 问题 2：偏差基准点不物理、不可信

现算法（`bumpy_vision.c:403-409`）取左右拟合线**各自中心点** `(cx,cy)` 的均值作为一个像素点，再与图像中列 47 同行 IPM 相减：

- `L.cy` 与 `R.cy` 一般不在同一行，均值行无明确物理意义；
- 中心点位置取决于边线被看到的区段（遮挡、长度变化都会让它沿边线滑动），**同一车身位置下偏差值会随可见边线长度漂移**；
- 没有用"条纹横线物理长度恒为 1m"这一已知强约束做校验或解算。

### 问题 3：单边丢线直接放弃横向

仅单边有效时 `lateral_mm=0, meas_valid=0`（`bumpy_vision.c:400`）。颠簸/倾斜导致单边丢线是常态，等于**最需要横向修正的时刻反而没有横向观测**；实际上单边边线 + 已知 1m 间距足以逆解算中线偏差。

### 问题 4：角度被边线检测状态绑架

现状角度要进到控制需先满足 `bumpy_detected=1`（TRACK 状态）——SEARCH 状态强制 `err_degree_cmd=0`、PID 复位，且 SEARCH 会清 `heading_stable`。条纹角明明是全程最稳的观测量，却被边线检测状态绑架——**测出的角度应持续参与接入，不论边线检测情况**（三帧稳定门控本身保留，见 §3.1）。

### 保持项：无线且无条纹 → 全 0 锁航向（非问题，不改变）

条纹也没有（`hdg_valid=0`）是唯一"严重未测出"场景：`yaw_error=0`、`lateral_mm=0`、`meas_valid=0`，0 核锁航向。**该路径本方案不改变**；同时"两侧边线都没有时 `lateral_mm` 输出 0"的语义保持不变。

---

## 第三部分：目标解算方案

### 3.0 已知物理约束（已确认）

- **条纹横向连通域（横线）的物理长度恒为 1000mm**，即左右边线物理间距 `BUMPY_EDGE_SPACING_MM = 1000`（不是"距小车 1m"）；
- 左右边线物理世界近似平行，且**经 IPM 映射到物理平面后基本竖直**（沿车头纵向延伸）——因此任取一个固定图像行求 x 即有代表性；
- IPM 表：像素 ↔ 物理（mm）互查，原点在车头，x 向右为正、y 向前为正。

### 3.1 倾斜角：引出管线已有 `frame_heading`，持续参与接入

**1 核侧（纯引出，不新增算法）：**

1. `bumpy_frame_result_t` 增加字段：`int hdg_valid; float hdg;`（1 核内部结构体，不影响 IPC/0 核）；
2. `bumpy_pipeline_frame()` 在现有 `frame_heading(ncc, &hdg)` 调用处（:585）把结果写入 `out->hdg / out->hdg_valid`（无线性连通域时 `hdg_valid=0`）；
3. 适配层 `yaw_error` 改为**永远取自 `hdg`**（不再用 `res.L/R.ang`）：
   - `hdg_valid=1` → `yaw_error_deg_x100 = atan2(-sin(hdg), cos(hdg))×100`（沿用现公式，正=需右转）；`direction_x/y` 同步用 `hdg` 填；**`meas_valid=1`**；
   - `hdg_valid=0`（无条纹，唯一严重丢失场景）→ 角度输出 0，**`meas_valid=0`**；
   - **无论边线 L/R 是否 valid，角度都输出**。

**0 核侧（接入方式变更："测出即持续接入，不论边线检测情况"；三帧稳定门控保留）：**

4. `vision_bumpy_control`：`meas_valid=1`（bit8）即跑 PID 产出 `err_degree_cmd`，**不再要求 TRACK 状态**（SEARCH 状态不再强制 cmd=0、不再复位 PID、不再清 `heading_stable`）；仅 `meas_valid=0`/STALE 才 cmd=0、PID 复位、`heading_stable` 清零；
5. `heading_stable` **三帧稳定门控保留**（已确认）：`meas_valid=1` 且相邻帧角度差 ≤1° 连续 3 帧 → 置 1；新语义下 `meas_valid` 不再被边线丢失打断，因此**无线但有条纹时稳定性计数可以延续**，视觉指令不中断；
6. `bumpy_road.c` `ApplyYawHold()` 逻辑不变：`heading_stable==1` → 采用 `err_degree_cmd`；失稳（仅 `meas_valid=0` 或角度跳变）→ 锁失稳前航向/入口航向；输出 EMA(α=0.05) 不变。

### 3.2 横向偏差：固定图像行 y=40 + IPM + 1m 间距

基准行固定为**图像第 40 行**（宏 `BUMPY_IPM_BASE_ROW = 40`，可调）。依据：边线 IPM 后基本竖直，固定行求 x 即可代表整条边线位置，且彻底消除"解算点随可见边线长度漂移"（现问题 2）。

对每条有效边线（像素直线：角 `ang`，过点 `(cx,cy)`）：

1. **求边线与第 40 行的交点像素** `x_pix = cx + (40 − cy) / tan(ang_pix)`（`tan` 近 0 时降级取 `(cx, cy)` 直接 IPM）；
2. **IPM 查表** `IPM_GetPhysicalCoord(x_pix, 40)` → 物理 `x_mm`；`is_valid==false` → 该边线本轮不可解算，按"该侧不存在"降级；
3. **中线横偏**（车身右偏为正，与现契约一致）：
   - 双侧：`x_center = (x_L + x_R) / 2`；
     - 间距自检：`|(x_R − x_L) − 1000| > BUMPY_WIDTH_TOL_MM`（**已确认 = 150**，IPM 表格天然上限）→ 本帧 `lateral_mm=0`（**`meas_valid` 不受影响**，角度照常接入）；
   - 仅左侧：`x_center = x_L + 500`；
   - 仅右侧：`x_center = x_R − 500`；
   - 即**单边逆解算**：用已知 1m 间距把单条边线平移半宽还原中线；
   - **两侧都没有：`lateral_mm = 0`**（保持项，不变）；
4. **输出**：`lateral_mm = −x_center`（正=车身偏右；符号最终实车验证，已有符号开关机制可挂）。

**0 核侧横向记录规则调整（已确认）**：`recorded_lateral_mm` 的 EMA **只在 `lateral_mm ≠ 0` 时更新**——因为新语义下 `meas_valid=1` 不再代表"有横向观测"（无线/自检失败帧横向为 0），非零门控可避免这些帧把记录值向 0 污染。`meas_valid=0` 时记录冻结（不变）。

### 3.3 分场景输出与接入真值表（目标行为）

| 场景 | bumpy_detected | yaw_error（条纹角） | lateral_mm | meas_valid | 0 核行为 |
|---|---|---|---|---|---|
| 双侧有效且间距自检通过 | 1 | `hdg` | `−(x_L+x_R)/2` | 1 | PID 产出 cmd；三帧稳定后采用修朝向；EMA 记录横向 |
| 双侧有效但间距自检失败 | 1 | `hdg` | 0 | 1 | PID 产出 cmd；三帧稳定后采用修朝向；本帧横向不记录（非零门控） |
| 仅单侧有效 | 1 | `hdg` | **单边逆解算 `−(x_edge ± 500)`** | 1 | PID 产出 cmd；三帧稳定后采用修朝向；EMA 记录横向 |
| 无线但有条纹 | 0 | `hdg` | 0 | 1 | **SEARCH 不再断**：PID 照跑、稳定计数延续，已稳定则继续修朝向（本方案核心变更）；横向不记录 |
| 无线且无条纹 | 0 | 0 | 0 | **0** | cmd=0、PID 复位、锁航向（唯一失稳路径，不变） |

> `bumpy_detected` 语义不变（= 有有效边线），**仅继续用于 0 核进入/出口确认**（连续 3 帧检出/未检出），不再参与角度接入门控。

---

## 第四部分：修改点清单（审批后执行）

### 4.1 CM7_1

1. **`code1/vision/bumpy_pipeline.h`**：`bumpy_frame_result_t` 增加 `int hdg_valid; float hdg;`（仅 1 核内部使用，+8B）；
2. **`code1/vision/bumpy_pipeline.c`**（:584-589 区域）：`frame_heading` 结果写入 `out`（约 2 行），**算法零改动**；
3. **`code1/vision/bumpy_vision.c`** 新管线分支（:388-412 区域重写）：
   - 角度：`yaw_error / direction_x/y` 改取 `res.hdg`（`hdg_valid` 门控），删除基于 `L/R.ang` 的 `bumpy_vision_angle_avg` 调用；
   - **`meas_valid = hdg_valid`**（条纹在即 1，唯一 0 的场景是无条纹）；
   - 横向：新增静态函数 `bumpy_vision_edge_x_at_row40(const bumpy_line_t *line, float *x_mm_out)`：像素直线与第 40 行求交 → `IPM_GetPhysicalCoord(x, 40)` → 物理 x；IPM 无效按该侧不存在降级；
   - 双侧：间距自检（≈1000mm，容差 150mm）+ 中点，自检失败 `lateral_mm=0`；单侧：±500mm 逆解算；两侧都没有：`lateral_mm=0`；
   - 真值表按 §3.3 填各字段；
4. **`code1/vision/bumpy_vision.h`**：新增配置宏：
   - `BUMPY_EDGE_SPACING_MM = 1000`（已确认）、`BUMPY_HALF_SPACING_MM = 500`；
   - `BUMPY_IPM_BASE_ROW = 40`、`BUMPY_WIDTH_TOL_MM = 150`（已确认，IPM 表格天然上限）。
5. **不动**：管线算法本身、IPC 字段与打包逻辑（仅 bit8 置位条件随 `meas_valid` 语义自然变化）、IPM 表。

### 4.2 CM7_0（本版起有改动）

1. **`code/vision/vision_bumpy_control.c`**：
   - SEARCH 状态不再强制 `err_degree_cmd=0`、不再复位 PID、不再清 `heading_stable`：`meas_valid=1` 即照常 `calc_err_degree` + PID（TRACK/SEARCH 统一）；仅 `meas_valid=0`/STALE 才 cmd=0、PID 复位、`heading_stable` 清零；
   - `recorded_lateral_mm` EMA 更新加 `lateral_mm != 0` 非零门控（已确认）；
   - `heading_stable` 三帧统计与 `VISION_BUMPY_HEADING_*` 宏**保留不动**（门控保留，已确认）；
2. **`code/plan/bumpy_road.c`**：**零改动**（`ApplyYawHold()` 的 `IsHeadingStable()` 门控保留，失稳锁航向路径不变）；
3. **不动**：进入/出口确认（仍基于 `bumpy_detected` 连续 3 帧）、出口锚点叠加、速度规划、执行链。

### 4.3 前置条件

- `BUMPY_USE_NEW_PIPELINE` 翻回 1（rw data 余量 6,510B——本次改动无新增大缓冲，仅几十行浮点运算 + 结构体 8B，需重新核对 map）；
- 旧算法分支（宏=0）不补横向/不改角度，维持现状。

---

## 第五部分：验证计划

1. **间距常量复核**：实车/赛道尺量条纹横线长度，确认 1000mm；
2. **符号验证**（沿用跨核接口规划第一优先级清单）：`lateral_mm` 正=车身偏右、`yaw_error` 正=需右转，静态摆车验证；
3. **角度持续接入验证**：遮挡边线制造"无线但有条纹"帧，确认 `yaw_error` 持续输出、`heading_stable` 不掉、`err_degree` 持续响应；
4. **静态精度**：车停在已知横偏（±100mm、±200mm）处，对比解算值，双侧/单边各 10 帧取均值，误差目标 ≤±50mm；
5. **间距自检有效性**：遮挡/误检制造间距异常帧，确认 `lateral_mm` 掉 0 且 `recorded_lateral_mm` 不被污染、角度接入不受影响；
6. **动态**：颠簸段实跑，观察全程 `err_degree` 连续性、`recorded_lateral_mm` 收敛性、出口叠加落点改善；
7. **map 复核**：翻宏 + 本次改动后确认 CM7_1 rw data 不超 262,144B。

---

## 第六部分：确认项（全部已裁定，待总体审批）

1. ~~`BUMPY_EDGE_SPACING_MM` 取值~~ → **已确认 = 1000mm**（条纹横线物理长度）；
2. ~~倾斜角数据源~~ → **已确认**：引出管线已有 `frame_heading`（已核实存在，`bumpy_pipeline.c:437-452`，仅需加结构体字段带出，不新增算法）；
3. ~~IPM 基准行~~ → **已确认**：固定图像第 40 行；
4. ~~valid 位语义~~ → **已确认**：`meas_valid` 仅标记"条纹也没有"的严重丢失（= `hdg_valid`）；角度测出即持续参与接入，不论边线检测情况；
5. ~~无线时偏差~~ → **已确认**：两侧边线都没有时 `lateral_mm = 0` 输出；
6. ~~三帧稳定门控~~ → **已确认保留**：`heading_stable` 连续 3 帧平稳才采用视觉指令，`ApplyYawHold()` 逻辑不变；
7. ~~横向记录 EMA 非零门控~~ → **已确认接受**：`lateral_mm≠0` 才更新 EMA；
8. ~~间距自检容差~~ → **已确认接受 = ±150mm**（IPM 表格天然上限，无升级可能）。

---

**审批通过前，禁止对以上文件做任何代码修改。**
