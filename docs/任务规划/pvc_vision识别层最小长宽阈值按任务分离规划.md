# pvc_vision 识别层参数（最小长宽阈值）按任务分离规划

> 状态：**待审批**
> 目的：让 1 核 PVC 入口检测器（`pvc_vision`）在 **3stage（三级跳）** 与 **slope（斜坡）** 两个场景下使用**各自独立的尺寸/阈值参数**（重点：最小宽度、最小高度、最小面积），两处可独立调参互不影响。
> 修订日期：2026-08-20
> 范围：**仅 1 核识别层**（`code1/vision/pvc_vision.*` + IPC 命令通道）。0 核控制参数分离见《3stage与slope的PVC视觉独立调参规划.md》，本文档不含控制层。

---

## 0. 结论摘要

| 问题 | 现状 |
|---|---|
| 3stage / slope 能否独立调"最小长宽阈值"？ | **不能**。两者触发的是**完全相同**的 1 核任务（`active_target=PVC_ENTRY` + `enable_mask=PVC`），共用同一个 `pvc_vision` 检测器、同一套编译期宏参数。 |
| 阈值在哪里生效？ | `pvc_vision.c` 两处：`pvc_extract_level()` L246-247、`pvc_detect_frame()` L276-277（`MIN_AREA/MIN_WIDTH/MIN_HEIGHT`），另 L279 `MIN_FILL_RATIO`、L292 `MIN_DECISION_SCORE`。 |
| 有没有现成通道？ | 命令结构已有预留字段 `pvc_min_score_u16`（初始化 580 但**无人消费**）、`reserved0`、`flags`、`roi_*`；CRC 用 `offsetof(crc)` 全字段求和，**新增字段自动纳入校验**，零 CRC 改动。 |

---

## 1. 现状审核（已核实，含代码行号）

### 1.1 两处调用在 IPC 命令层完全相同

```
slope（vision_slope_control.c:183）
  VisionPvcControl_SetEnable(1U)
    → VisionIpc_Core0_SetPvcEnable(1U)          (vision_ipc_core0.c:78)
    → VisionIpc_Core0_SetTask(PVC_ENTRY, MASK_PVC_ENTRY)

3stage（vision_three_stage_control.c:245）
  VisionThreeStageControl_Start()
    → VisionIpc_Core0_SetTask(PVC_ENTRY, MASK_PVC_ENTRY)
```

- 1 核侧 `vision_ipc_core1_command_wants_pvc()`（vision_ipc_core1.c:72-77）只看 `active_target==PVC_ENTRY || enable_mask&PVC` → **两处完全等价**，1 核无法区分场景。
- 1 核主循环 `main_cm7_1.c:246-249`：`ShouldRunPvc()` 为真 → `pvc_vision_process_camera_frame()`，即两处共用**同一个检测器实例**。

### 1.2 "最小长宽阈值"（编译期宏，单套）

`code1/vision/pvc_vision.h:69-73`：

| 宏 | 值 | 含义 | 生效位置 |
|---|---|---|---|
| `PVC_VISION_MIN_AREA` | 120 | 白点少于 120 不要（面积） | `pvc_vision.c:246,276` |
| `PVC_VISION_MIN_WIDTH` | 20 | 不够宽不要（最小宽度） | `pvc_vision.c:247,277` |
| `PVC_VISION_MIN_HEIGHT` | 4 | 不够高不要（最小高度） | `pvc_vision.c:247,277` |
| `PVC_VISION_MIN_FILL_RATIO` | 0.25f | 填充率（稀疏白点不要） | `pvc_vision.c:279` |
| `PVC_VISION_MIN_DECISION_SCORE` | 0.58f | 最终打分门限 | `pvc_vision.c:292` |
| `PVC_VISION_WHITE_THRESHOLD` | 180 | 亮度阈值 | `pvc_vision.c:231`（pvc_extract_level 入参） |
| `PVC_VISION_CONFIRM_FRAMES` | 3 | 连续 3 帧确认稳定 | `pvc_vision.c:353`（防抖） |
| `PVC_VISION_LOST_HOLD_FRAMES` | 2 | 允许短暂丢失帧数 | `pvc_vision.c:359`（防抖） |

> 全部是 `#define` 宏，检测器单实例单参数，**两处无法分开调**。

### 1.3 两处场景诉求差异（为何要分离）

- **3stage（三级跳）**：识别台阶上的白色条带/台阶边缘，**目标小、可能远** → 通常希望 `MIN_AREA/MIN_WIDTH/MIN_HEIGHT` 偏小、抓得早、抓得多；用 `entry_top_y/entry_bottom_y` 行号触发跳跃，行号精度敏感。
- **slope（斜坡）**：识别大面积白色 PVC 斜坡入口，**目标大** → 需要抗反光/抗误检（注释："下午反光调大该参数"），`MIN_WIDTH` 偏大、要求更"成块"；只关心是否"已压上入口"（bbox 占比）。

### 1.4 有利条件（降低改造风险）

1. `vision_ipc_command_t`（vision_ipc.h:51-69）已有未用字段：
   - `pvc_min_score_u16`（L61，init 设 580，**无任何消费方**）—— 预留的"0 核可下发最低分"通道
   - `reserved0`（L62，uint16）
   - `flags`（L58，uint32）、`roi_xmin..roi_ymax`（L66-69）
2. CRC = `checksum16(cmd, offsetof(crc))`（vision_ipc.h:189-192）→ **新增/修改 crc 之前任何字段自动纳入校验**，不需要改 CRC 代码。
3. bridge 使用另一套检测器 `bridge_pvc_vision`（不走 `pvc_vision` 尺寸阈值）→ 本改动不影响单边桥。
4. 已存在"重置防抖"接口 `pvc_vision_reset_filter()`，参数切换后可复用。

---

## 2. 目标

- `pvc_vision` 检测器参数运行时化，至少两套（3stage 场景 / slope 场景），由 0 核按当前任务下发场景 ID 切换。
- 最小长宽阈值（及配套面积/填充率/打分/防抖）两处独立，改一处不影响另一处。
- 不改 `vision_ipc_packet_t` 结果结构，不改 0 核控制模块，兼容上位机/示波器现有字段。

---

## 3. 决策点（需先拍板）

| 编号 | 决策 | 选项 | 推荐 |
|---|---|---|---|
| **P1** | 分离范围 | A. 只分离"尺寸门槛"（MIN_AREA/MIN_WIDTH/MIN_HEIGHT/MIN_FILL_RATIO）<br>B. 全参数（含 WHITE_THRESHOLD / MIN_DECISION_SCORE / CONFIRM_FRAMES / LOST_HOLD_FRAMES） | **B**（一次做全，结构相同，成本几乎一样） |
| **P2** | slope 场景初值 | 由你给定（当前先 = 3stage 现值：AREA=120/W=20/H=4/FILL=0.25/SCORE=0.58/THR=180） | 先同值，验证架构后再调 |
| **P3** | 参数下发方式 | A. 编译期两套静态表 + 命令只传"场景 ID"（1bit）<br>B. 运行时可调：0 核通过命令下发具体数值（可在线调，占用更多命令字段） | **A**（简单可靠，够用） |
| **P4** | 场景 ID 用哪个字段 | A. `reserved0`（uint16，天然 CRC 覆盖）<br>B. `flags` 低位 | **A** |

---

## 4. 实施方案（推荐路径）

### 阶段 1 — 参数运行时化（`code1/vision/pvc_vision.h/.c`）

1. 新增参数结构体（放 `pvc_vision.h`）：
   ```c
   typedef struct {
       uint8  white_threshold;      /* 180 */
       uint16 min_area;             /* 120 */
       uint8  min_width;            /* 20  */
       uint8  min_height;           /* 4   */
       float  min_fill_ratio;       /* 0.25f */
       float  min_decision_score;   /* 0.58f */
       uint8  confirm_frames;       /* 3   */
       uint8  lost_hold_frames;     /* 2   */
       uint8  bottom_target_rows;   /* 12  */
   } pvc_vision_param_t;

   typedef enum {
       PVC_PARAM_DEFAULT = 0,   /* 默认 / 3stage */
       PVC_PARAM_SLOPE   = 1,   /* slope */
       PVC_PARAM_SETS    = 2
   } pvc_param_set_id_e;
   ```
2. `pvc_vision.c` 新增静态参数表（初值 = 当前宏值）：
   ```c
   static const pvc_vision_param_t g_pvc_param_table[PVC_PARAM_SETS] = {
       [PVC_PARAM_DEFAULT] = { 180, 120, 20, 4, 0.25f, 0.58f, 3, 2, 12 },
       [PVC_PARAM_SLOPE]   = { 180, 120, 20, 4, 0.25f, 0.58f, 3, 2, 12 },  /* 初值同默认，待 P2 给定 */
   };
   static const pvc_vision_param_t *g_pvc_active_param = &g_pvc_param_table[PVC_PARAM_DEFAULT];
   ```
3. 替换宏使用点（`pvc_vision.c`）：
   - `pvc_extract_level()` L246-247：`MIN_AREA/MIN_WIDTH/MIN_HEIGHT` → `g_pvc_active_param->min_area/min_width/min_height`
   - `pvc_detect_frame()` L276-277、L279、L292：同上 + `min_fill_ratio/min_decision_score`
   - L231：`pvc_extract_level(..., PVC_VISION_WHITE_THRESHOLD, ...)` → `g_pvc_active_param->white_threshold`
   - `pvc_update_filter()` L353/359：`CONFIRM_FRAMES/LOST_HOLD_FRAMES` → 参数化
   - 若 P1=B，`pvc_vision.h` 宏可保留为"默认值定义"（表初值引用），或直接删除（表内写值）。
4. 新增切换接口（供 1 核 IPC 调用）：
   ```c
   void pvc_vision_set_param_set(uint8 set_id);  /* 越界钳到 DEFAULT；切换后自动 pvc_vision_reset_filter() */
   ```

### 阶段 2 — IPC 命令通道（`code/vision/vision_ipc.h` + `vision_ipc_core0.c` + `vision_ipc_core1.c`）

1. `vision_ipc_command_t` 增加语义注释：`reserved0` 低 8 位 = `pvc_param_id`（0=默认/3stage，1=slope）。不改结构布局（避免破坏 0/1 核对齐）。
2. 0 核新增接口（`vision_ipc_core0.h/.c`）：
   ```c
   void VisionIpc_Core0_SetPvcParamId(uint8 id);
   /* 实现：g_core0_command_shadow.reserved0 = (uint16)(g_core0_command_shadow.reserved0 & 0xFF00U) | id;
            seq++; dirty = 1; */
   ```
   （复用 `SetTask` 的 flush 机制，CRC 自动覆盖。）
3. 1 核 `VisionIpc_Core1_PollCommand()`（vision_ipc_core1.c:283-316）：
   - 解析 `cmd.reserved0 & 0xFFU` 为 `pvc_param_id`
   - 与 shadow 比较，变化时：`pvc_vision_set_param_set(pvc_param_id)`（内部含 reset_filter）
   - `g_core1_command_shadow` 同步保存新值

### 阶段 3 — 0 核调用方接线

1. `code/vision/vision_slope_control.c` `vision_slope_enter_task()`（L181-183）：
   ```c
   VisionIpc_Core0_SetPvcEnable(1U);
   VisionIpc_Core0_SetPvcParamId(PVC_PARAM_SLOPE);   /* 新增：slope 场景参数集 */
   ```
2. `code/vision/vision_three_stage_control.c` `VisionThreeStageControl_Start()`（L245 后）：
   ```c
   VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);
   VisionIpc_Core0_SetPvcParamId(PVC_PARAM_DEFAULT); /* 新增：3stage 场景参数集 */
   ```
3. 两处 cleanup 保持 `SetPvcEnable(0U)` / `SetTask(NONE,0)` 不变；参数 ID 随下次启动重新下发，无需单独复位（或 cleanup 时也置回 DEFAULT，二选一，建议保持"启动时下发"单一来源）。

> 注意：`SetPvcEnable(1)` 内部会 `SetTask(PVC_ENTRY, MASK)`（vision_ipc_core0.c:78-80），故 slope 必须先 enable 再 SetPvcParamId，顺序不能反；3stage 同理在 SetTask 之后设置。

### 阶段 4 —（可选，P3=B 时）运行时可调

- 若需 0 核在线下发数值：扩展 `pvc_min_score_u16`（已有）承载最低分，再在 `flags` 或新字段放 `min_width/min_height/min_area`（需 0 核侧 volatile 变量 + 上位机通道）。
- 该方案侵入更大，建议先按 P3=A 落地。

---

## 5. 验证方案

| 阶段 | 验证 |
|---|---|
| 1 | 双核 iarbuild 0 错误；行为不变（两套参数初值相同） |
| 2 | 3stage / slope 分别跑通；上位机观察 `pvc_candidate_count` / `pvc_area` 反映阈值 |
| 3 | 改 slope 表 `min_width`（如 20→40），确认 slope 识别变严、3stage 不受影响 |
| 4 | 场景切换瞬间无防抖残留（reset_filter 生效，无"幽灵检测"） |

## 6. 回退方案

- L1（参数级）：slope 表项改回与默认同值 → 等效旧行为。
- L2（逻辑级）：`git checkout` 回退；若已提交按 git 安全规则需用户批准后执行 revert。
- 结构上：由于 `vision_ipc_packet_t` 未动、0 核控制模块未动，回退面仅限 `pvc_vision.*` + IPC 命令通道 3 处文件。

## 7. 涉及文件清单

| 文件 | 变更 |
|---|---|
| `code1/vision/pvc_vision.h` | 新增 `pvc_vision_param_t`、`pvc_param_set_id_e`、`pvc_vision_set_param_set()` 声明 |
| `code1/vision/pvc_vision.c` | 宏 → 参数表/活动指针；替换 5 处阈值使用点；新增切换接口 |
| `code/vision/vision_ipc.h` | `reserved0` 低 8 位语义注释（pvc_param_id） |
| `code/vision/vision_ipc_core0.h/.c` | 新增 `VisionIpc_Core0_SetPvcParamId()` |
| `code1/vision/vision_ipc_core1.c` | PollCommand 解析 pvc_param_id → 切换参数集 |
| `code/vision/vision_slope_control.c` | enter_task 增加 `SetPvcParamId(PVC_PARAM_SLOPE)` |
| `code/vision/vision_three_stage_control.c` | Start 增加 `SetPvcParamId(PVC_PARAM_DEFAULT)` |
| `user/main_cm7_1.c` | 不改（检测器调度不变） |
