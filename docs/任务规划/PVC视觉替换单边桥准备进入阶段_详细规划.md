# 用 PVC 视觉（RLE 版）替换单边桥视觉状态机「一号情况」（准备进入）—— 详细规划

> **文档版本**: v1.1（决策已拍板，待实施）
> **创建日期**: 2026-08-15
> **目标平台**: 双核架构，Core 1 Cortex-M7，IAR EWARM
> **状态**: ✅ **决策已拍板（2026-08-15），待下达实施指令。**
> **相关文件**:
> - 1 核融合状态机：`code1/vision/bridge_fusion.c/.h`（**状态机架构不变，仅准备进入分支换引擎**）
> - 1 核单边桥专用 PVC（**新模块，复制 pvc_vision 并改名**）：`code1/vision/bridge_pvc_vision.c/.h`
> - 1 核 PVC 检测（**复制源，不改**）：`code1/vision/pvc_vision.c/.h`
> - 1 核参考检测器（保留，仅脱出阶段用）：`code1/vision/bridge_ref_detection.c/.h`
> - 1 核接线：`user/main_cm7_1.c`
> - 0 核：`code/vision/vision_bridge_control.c`（**零改动**）
> - IPC 协议：`code/vision/vision_ipc.h`
> - IAR 工程：cm7_1 `ewp` + `ewt`（新增 bridge_pvc_vision 两条引用）

---

## 目录

1. [背景与目标](#一背景与目标)
2. [现状分析](#二现状分析)
3. [一号情况定位](#三一号情况定位)
4. [替换方案](#四替换方案)
5. [关键映射与正确性要点](#五关键映射与正确性要点)
6. [详细改动清单（按文件）](#六详细改动清单按文件)
7. [决策点（需确认）](#七决策点需确认)
8. [风险与验证](#八风险与验证)
9. [与已有文档衔接](#九与已有文档衔接)

---

## 一、背景与目标

### 1.1 背景

单边桥视觉检测在 1 核由 `bridge_fusion` 做"远近融合"：远处接近入口用 **ref 参考检测器**（亮区连通域 + 凸包 + 多阈值边线拟合，计算极重，文档《远近融合检测接入迁移规划.md》记录 avg 9.25ms / max 15.5ms，几乎吃满 10ms 帧周期），桥上用 **v8 三线透视**，脱出用 **ref 检测器找顶边横线**。

`pvc_vision.c` 已完成 **RLE 游程编码加速**（提交 `cb68ab1 【pvc视觉】pvc视觉加速测试成功`；工作区未提交改动：算法已从 Flood Fill 改为 RLE 游程版，`PVC_VISION_WHITE_THRESHOLD` 200→180），识别对象正是"白色 PVC 入口"，输出目标点、距离、包围框等，计算量远小于 ref。

### 1.2 目标

**用「单边桥专用 PVC」替换单边桥视觉状态机的「一号情况」——准备进入（PREPARE_ENTER）阶段的 ref 引擎**。

关键原则（用户 2026-08-15 定调）：

1. **fusion 状态机整体架构不变**：仍是 `gate_bottom`/`gate_top` 两个锁存门控、三阶段（准备进入→桥上→准备脱出），`bridge_fusion_frame()` 仍是唯一入口。
2. **状态机刚开启时（准备进入阶段）默认跑专用 PVC**，而不是 ref；专用 PVC 检测的**调用在 `bridge_fusion_frame()` 内部、受状态机掌控**，不是独立调用。
3. **专用 PVC 通过自己产生的「最后结束线」（白色连通域底线 `entry_bottom_y`）拖动状态机切到 v8**：当 `entry_bottom_y > 阈值`（可调宏，默认 45）时锁存 `gate_bottom`，下一帧自动进入 v8 桥上阶段。
4. **桥上（v8）与准备脱出（ref 脱出线）阶段保持不变**。
5. 单边桥入口的 PVC 与既有 pvc_vision 使用场景参数不同，**复制一份独立模块 `bridge_pvc_vision` 并改名**，独立调参。

### 1.3 不变范围

| 保持不变的模块 | 原因 |
|---|---|
| `code1/vision/bridge_detect.c/.h`（v8 三线透视） | 桥上阶段继续用 |
| `code1/vision/bridge_ref_detection.c/.h`（ref 检测器） | 准备脱出阶段继续用 |
| `code1/vision/bridge_v2_arbiter.c/.h` | 桥上阶段仲裁继续用 |
| `code1/vision/bridge_output_filter.c/.h` | 中值滤波层继续用（唯一 b2_* 数据源） |
| `code/vision/vision_bridge_control.c/.h`（0 核控制状态机） | **零改动** |
| `code/vision/vision_ipc.h`、`vision_ipc_core0/.1` | IPC 协议与打包链路不动 |
| `code1/vision/pvc_vision.c/.h`（RLE 版） | 作为复制源，本体不动 |

---

## 二、现状分析

### 2.1 单边桥视觉状态机（bridge_fusion）

`bridge_fusion_frame()` 每帧按**上一帧门控**二选一引擎，跑完后用本帧结果更新门控：

```
gate_bottom=0 ──────────> gate_bottom=1 ──────────> gate_top=1
 [ref 检测器]   底部白锁存   [v8 三线透视]  双重门控    [ref 检测器]
 远处中线                  桥上中线        锁存        脱出线
```

帧首唯一分支：

```c
if (st->gate_bottom && !st->gate_top) {
    /* 桥上: v8 */
    ...
} else {
    /* ref 检测器 (远处接近 / 脱出 共用) */
    out->source = BF_SRC_REF;
    bridge_detection_detect_gray(...);          // ref 检测桥面
    bf_center_from_ref(&out->ref, &out->center); // center_segment → 中线
    out->valid = bridge_found && center_segment.valid;
    if (st->gate_top) { bf_update_exit_line(...); }   // 脱出线
    else              { bf_update_gate_bottom_ref(...); } // 准备进入 gate
}
```

### 2.2 三阶段与 B2M 编码

| 阶段 | 门控条件 | 引擎 | `B2M_STAGE_*` |
|---|---|---|---|
| 准备进入 PREPARE_ENTER | `gate_bottom=0, gate_top=0` | ref | `0x00` |
| 桥上 ON_BRIDGE | `gate_bottom=1, gate_top=0` | v8 | `0x01` |
| 准备脱出 PREPARE_EXIT | `gate_top=1` | ref | `0x02` |

`main_cm7_1.c::bridge_fusion_pack_mode()`：

```c
if (r->gate_top)         stage = B2M_STAGE_PREPARE_EXIT;
else if (r->gate_bottom) stage = B2M_STAGE_ON_BRIDGE;
else                     stage = B2M_STAGE_PREPARE_ENTER;
```

### 2.3 PVC 视觉（RLE 版）现状

`pvc_vision_process_camera_frame()` → `g_pvc_vision_output`（`pvc_vision_output_t`），关键字段：

| 字段 | 类型 | 含义 | 本次用途 |
|---|---|---|---|
| `stable_detected` | uint8 | 防抖后稳定看到 | valid 判定 |
| `raw.detected` | uint8 | 瞬时看到 | 备选 valid |
| `stable.target_x_px_x100` | int16 | 入口目标中心横坐标（像素×100） | 引导中线 b |
| `stable.entry_bottom_y` | uint8 | 白斑最下行号（越靠车越大） | gate_bottom 判据 |
| `stable.entry_top_y` | uint8 | 白斑最上行号 | u_lo/u_hi 备选 |
| `stable.forward_mm` | int16 | 前向距离（-1 未知） | gate_bottom 备选判据 |
| `stable.bbox_*` | uint8×4 | 包围框 | 调试/渲染 |

图像尺寸：`PVC_IMAGE_W=94, PVC_IMAGE_H=60`，与 `BF_W=94, BF_H=60` 完全一致，可直接同帧对齐。

### 2.4 调用时序（1 核帧循环，`main_cm7_1.c`）

```c
if (VisionIpc_Core1_ShouldRunBridge()) {
    bridge_fusion_frame(compressed_image_copy[0], &s_fusion_st, &s_fusion_res);
    // ↑ 唯一入口：准备进入分支内部会调用 bridge_pvc_vision_process_camera_frame()
    //   （专用 PVC 检测受 fusion 状态机掌控，不在此处独立调用）
    ...
}
```

> 专用 PVC 检测**在 `bridge_fusion_frame()` 的准备进入分支内执行**，与 ref/v8 引擎同帧、同入口；状态机切到 v8 后自动不再运行专用 PVC。因此专用 PVC 完全跟随 `ShouldRunBridge()` 门控，**无需任何额外 enable**。

### 2.5 0 核消费（确认零改动前提）

0 核 `vision_bridge_control.c` 的 ALIGN 阶段经 `vision_bridge_get_control_measurement()` 消费 `b2_line_a_x1000/b2_line_b_x100`：

```c
if (lookahead_img_y < b2_line_u_lo || lookahead_img_y > b2_line_u_hi) return 0; // 支撑校验
x_at_lookahead = a*25/1000 + b/100;   // 前视点 y=25
heading = atan2(IPM(x_at_lookahead,25)) - atan2(IPM(47,25));
```

**只要把 PVC 的 `target_x` 构造成 `a=0, b=target_x, u_lo=0, u_hi=59` 的竖直线，0 核就会得到"对准入口中心"的差角，无需改动 0 核。**

---

## 三、一号情况定位

**「一号情况」= 准备进入（PREPARE_ENTER，stage 0）= `bridge_fusion_frame()` 的 `else` 分支中 `gate_top==0` 且 `gate_bottom==0` 的子路径。**

证据：

1. `bridge_fusion.c` 注释管线图：`gate_bottom=0 → [参考检测器] 远处中线`；
2. `main_cm7_1.c` 渲染编号：`0 = 准备进入（ref 引擎，远处中线）`；
3. 你此前《单边桥PVC切换排查-执行规划与视觉复核.md》把该阶段称为"PVC 视觉"，与"我的视觉模块（v8）"的切换正是 0→1。

本次替换对象即该子路径内的**引擎选择与 gate 判定**。

---

## 四、替换方案

### 4.1 总体设计

将 `bridge_fusion_frame()` 的 `else` 分支按 `gate_top` **拆成两个明确子分支**；准备进入子分支**在状态机内部调用专用 PVC 检测**（受状态机掌控）：

```
frame 首:
  gate_bottom=0 → gate_bottom=1 → gate_top=1
    ├─ 准备进入: 调用 bridge_pvc_vision 检测（新，替代 ref）┐
    ├─ 桥上:     v8 三线透视（不变）            │
    └─ 准备脱出: ref 检测器（不变，仍找脱出线）  │
                                              ▼
                              gate_bottom/gate_top 锁存逻辑
```

准备进入子分支伪代码：

```c
} else if (st->gate_top) {
    /* 准备脱出: 保持 ref 不变 */
    out->source = BF_SRC_REF;
    bridge_detection_detect_gray(...);
    bf_center_from_ref(...);
    out->valid = bridge_found && center_segment.valid;
    bf_update_exit_line(...);
} else {
    /* 准备进入: 专用 PVC（一号情况替换点）——检测执行与输出读取都在状态机内 */
    out->source = BF_SRC_PVC;                              /* 新增 source 枚举 */
    bridge_pvc_vision_process_camera_frame(img94);          /* ★状态机内执行专用 PVC */
    bf_center_from_pvc(&g_bridge_pvc_vision_output, &out->center, &out->valid);
    /* gate_bottom 判定: PVC「最后结束线」entry_bottom_y > 阈值 → 锁存 → 拖动状态机切 v8 */
    bf_update_gate_bottom_pvc(st, &g_bridge_pvc_vision_output);
}
```

### 4.2 新增/改造函数

| 函数 | 类型 | 说明 |
|---|---|---|
| `bridge_pvc_vision_*`（整组） | **新模块**（复制 pvc_vision 改名） | 单边桥专用 PVC 检测，独立调参，在准备进入分支内被调用 |
| `bf_center_from_pvc(...)` | 新增 | 把专用 PVC `target_x` 构造成 `bridge_line_t`（`a=0,b=target_x,u_lo=0,u_hi=59`），设置 `out->valid`、`out->center` |
| `bf_update_gate_bottom_pvc(...)` | 新增 | 用专用 PVC 的「最后结束线」`entry_bottom_y > 阈值` 锁存 `st->gate_bottom`（拖动状态机切 v8） |
| `bf_update_gate_bottom_ref(...)` | **注释掉** | 准备进入不再调用（用户决策 6：旧进入相关代码直接注释，不删） |

### 4.3 状态机流转（改造后）

```mermaid
stateDiagram-v2
    [*] --> PREPARE_ENTER: gate_bottom=0, gate_top=0
    PREPARE_ENTER --> ON_BRIDGE: PVC「最后结束线」到达<br/>(entry_bottom_y>45 → gate_bottom=1)
    ON_BRIDGE --> PREPARE_EXIT: 结束线+底部全亮<br/>(gate_top=1)
    PREPARE_EXIT --> [*]: 脱出线确认(exit_confirmed)

    note right of PREPARE_ENTER: 引擎: bridge_pvc_vision (专用PVC)<br/>输出: 竖直线 x=target_x
    note right of ON_BRIDGE: 引擎: v8 三线透视<br/>输出: 红蓝中线/绿线
    note right of PREPARE_EXIT: 引擎: ref 检测器<br/>输出: 顶边横线(脱出线)
```

---

## 五、关键映射与正确性要点

### 5.1 PVC 目标点 → 引导中线（center）

```c
center.a    = 0.0f;                        // 竖直线
center.b    = (float)pvc.stable.target_x_px_x100 * 0.01f;  // x = target_x
center.rms  = 0.0f;
center.n    = 0;
center.u_lo = 0.0f;                        // ★全行带
center.u_hi = (float)(BF_H - 1);           // ★59，保证 0核 y=25 前视点在支撑内
```

> **★ 正确性要点 1**：`u_lo/u_hi` 必须给全行带 `[0,59]`，**不能用 PVC 白斑的 `entry_top_y/entry_bottom_y`**。原因：0 核 `vision_bridge_get_control_measurement` 的支撑校验要求 `u_lo <= 25 <= u_hi`；远处入口白斑可能只占图像上半部、不跨 y=25，若用白斑 y 范围会导致 `center_filter_valid` 恒为 0、退化锁角、PVC 引导失效。

> **★ 正确性要点 2**：`target_x` 取 `stable` 而非 `raw`（`stable_detected` 为真时 `stable` 才是防抖后的可信值）。与 `vision_ipc_core1_fill_pvc()` 的取法一致（`ctrl = stable_detected ? &stable : &raw`）。

### 5.2 valid 判定

```c
out->valid = (uint8_t)(pvc.stable_detected != 0U);
```

仅在 `stable_detected` 为真时输出有效中线；否则 `valid=0`，0 核自动锁角兜底（现有逻辑）。

### 5.3 gate_bottom（到达入口）判定 —— 已拍板：判据 A，阈值 45（可调宏）

专用 PVC 用自己产生的「最后结束线」（白色连通域底线 `entry_bottom_y`）拖动状态机切 v8：

```c
#define BF_PVC_GATE_BOT_Y  45   /* ★可调参宏, 用户定初值 45, 现场标定 */
if (!st->gate_bottom && pvc.stable_detected &&
    pvc.stable.entry_bottom_y > BF_PVC_GATE_BOT_Y) {
    st->gate_bottom = 1;   /* 单帧锁存, 与 ref 原 gate 同构 */
}
```

> 语义对照：ref 原 gate 是"底部行带 [52,59] 白占比>75%"（白色桥面充满图像底部）；新判据"白色连通域底线 `entry_bottom_y > 45`"等价于"白色入口下缘已到图像下半部"，语义同构。**严格大于（`>`）** 按用户描述"Y>45"执行。

### 5.4 source 枚举与下游适配

`bf_source_t` 现为 `{BF_SRC_REF=0, BF_SRC_V8=1}`。准备进入改用 PVC 后：

- 新增 `BF_SRC_PVC = 2`；
- `main_cm7_1.c` 接线处当前是 `if (source==V8) {arbiter} else {fill_ref_arb}`，需扩展为三路：
  - V8 → arbiter（桥上，不变）
  - REF → `fill_ref_arb`（准备脱出，不变）
  - PVC → 新的填充路径（准备进入）

### 5.5 下游填充（main_cm7_1.c）适配

现 `bridge_fusion_fill_ref_arb()` 读 `r->center` 与 `r->ref.center_segment`（u_lo/u_hi 来源）。PVC 源无 `ref` 字段，需新增 `bridge_fusion_fill_pvc_arb()`：

```c
static void bridge_fusion_fill_pvc_arb(const bf_result_t *r, bridge_v2_arb_t *out) {
    memset(out, 0, sizeof(*out));
    out->valid  = r->valid;
    out->source = 3;                      /* 3=准备进入, 与 ref 阶段同号或新号均可 */
    out->mode   = bridge_fusion_pack_mode(r);  /* 需支持 PVC 源的 det 打包 */
    out->gate   = r->gate_bottom;
    if (r->valid) {
        out->line_a_x1000 = (int16)(r->center.a * 1000.0f);
        out->line_b_x100  = (int16)(r->center.b * 100.0f);
        out->u_lo = (uint8)r->center.u_lo;
        out->u_hi = (uint8)r->center.u_hi;
    }
}
```

`bridge_fusion_pack_mode()` 的 det 位需为 PVC 源补分支：

```c
else if (r->source == BF_SRC_PVC) {
    det = (uint8)((r->valid ? B2M_DET_GREEN : 0));  /* PVC 入口稳定=检出 */
}
```

> 注意：`pack_mode` 的 stage 分支不变（`gate_bottom`/`gate_top` 逻辑通用）。

### 5.6 enable 门控 —— 已拍板：无需修改

专用 PVC 检测**在 `bridge_fusion_frame()` 准备进入分支内执行**，而 `bridge_fusion_frame()` 本身已受 `VisionIpc_Core1_ShouldRunBridge()` 门控（单边桥任务启动时 `VisionIpc_Core0_SetBridgeEnable(1)` 已开桥检测）。

因此专用 PVC 跟随桥检测自动运行，**0 核 `vision_bridge_control.c` 的 enable 开关、IPC 命令链路全部零改动**。

> 结论：本规划 v1.0 曾提出"SetTask(BRIDGE, BRIDGE|PVC_ENTRY)"，在"专用 PVC 内嵌于 fusion 状态机"的方案下已不需要，予以撤销。

---

## 六、详细改动清单（按文件）

### 6.1 新增：`code1/vision/bridge_pvc_vision.c/.h`（单边桥专用 PVC，复制 pvc_vision 改名）

1. 复制 `pvc_vision.c`（RLE 版）与 `pvc_vision.h` 为 `bridge_pvc_vision.c/.h`；
2. **全部对外符号改名**（避免与原 pvc_vision 冲突）：
   - 类型：`pvc_vision_output_t` → `bridge_pvc_vision_output_t`；`pvc_vision_frame_result_t` → `bridge_pvc_vision_frame_result_t`；
   - 函数：`pvc_vision_init/reset_filter/get_output/process_camera_frame` → `bridge_pvc_vision_*`；
   - 全局：`g_pvc_vision_output` → `g_bridge_pvc_vision_output`；`g_pvc_vision_output_write_busy` → `g_bridge_pvc_vision_output_write_busy`；两个 profiler 加 `bridge_` 前缀；
   - 内部 static 函数与宏前缀：`PVC_VISION_*` → `BRIDGE_PVC_VISION_*`（**独立调参**），`pvc_*` static 函数 → `bpvc_*`；
3. 图像尺寸、IPM 依赖保持一致（`94x60`、复用 `ipm_transform.h`）；
4. **IAR 工程 cm7_1**：`ewp` + `ewt` 的 vision 组各加 `bridge_pvc_vision.c`（`.h` 可入 ewp 便于查看，非必需）。

### 6.2 `code1/vision/bridge_fusion.h`

1. `bf_source_t` 新增 `BF_SRC_PVC = 2`；
2. 新增 gate 判据宏（可调参）：
   ```c
   #define BF_PVC_GATE_BOT_Y        45      /* ★用户定初值, 现场标定: 白色连通域底线>45 即切 v8 */
   ```
3. 新增函数声明：
   ```c
   void bf_center_from_pvc(const bridge_pvc_vision_output_t *pvc, bridge_line_t *c, uint8_t *valid);
   void bf_update_gate_bottom_pvc(bf_state_t *st, const bridge_pvc_vision_output_t *pvc);
   ```
   > 需 `#include "bridge_pvc_vision.h"`。

### 6.3 `code1/vision/bridge_fusion.c`

1. include 增加 `bridge_pvc_vision.h`；
2. 新增 `bf_center_from_pvc()`：按 §5.1 构造竖直线中线 + valid；
3. 新增 `bf_update_gate_bottom_pvc()`：按 §5.3（`entry_bottom_y > 45`）锁存 `gate_bottom`；
4. 改造 `bridge_fusion_frame()`：把 `else` 分支拆成 `else if (st->gate_top) {ref 脱出}` 与 `else {专用 PVC 准备进入}`；准备进入分支**在状态机内调用 `bridge_pvc_vision_process_camera_frame(img94)`**；
5. **注释掉** `bf_update_gate_bottom_ref()`（用户决策 6：旧进入相关代码直接注释，不删）；
6. `bridge_fusion_init()` 不变（仍初始化 v8_st + ref_cfg/scratch，供脱出阶段用；专用 PVC 的 init/reset 接线见 6.4）。

### 6.4 `user/main_cm7_1.c`

1. `bridge_fusion_pack_mode()` 增加 `BF_SRC_PVC` 的 det 打包分支（`r->valid → B2M_DET_GREEN`）；
2. 接线处把 `if (source==V8) ... else ...` 扩成三路，新增 `bridge_fusion_fill_pvc_arb()`；
3. 渲染 `render_bridge_vision_to_image()` 若依赖 ref 字段，需对 PVC 源做空值保护（PVC 源无 `s_fusion_res.ref`）——建议渲染分支按 `source` 区分或跳过 ref 专属渲染；
4. `bridge_pvc_vision` 的 init/reset 接线：
   - init：随 1 核初始化（或 `bridge_fusion_init` 内调用）；
   - reset：`VisionIpc_Core1_TakeBridgeResetRequest()` 处同步 `bridge_pvc_vision_reset_filter()`（与现有 `bridge_fusion_init + bridge_output_filter_reset` 并列）。
   > **无需** 在帧循环里独立调用 PVC 检测（检测执行已在 `bridge_fusion_frame` 内部）。

### 6.5 0 核与 IPC：零改动

- `code/vision/vision_bridge_control.c/.h`：**零改动**（ALIGN/RUN/EXIT、center_filter、err_ramp、exit gate 全不动）；
- `code/vision/vision_ipc_core0.c/.h`、`vision_ipc_core1.c/.h`、`vision_ipc.h`：**零改动**（b2_* 打包链路不动，专用 PVC 结果不进 IPC pvc_* 字段，而是经 b2_line_* 走现有链路）。

### 6.6 不改动清单

- `code1/vision/pvc_vision.c/.h`（复制源，本体不动）
- `code1/vision/bridge_detect.c/.h`、`bridge_v2_arbiter.c/.h`、`bridge_output_filter.c/.h`、`bridge_ref_detection.c/.h`

---

## 七、决策点（已拍板，2026-08-15）

| # | 决策点 | 拍板结果 |
|---|---|---|
| 1 | 引导中线映射 | ✅ 竖直线 `x=target_x`（`a=0,b=target_x`），**全行带支撑** `u_lo=0,u_hi=59` |
| 2 | gate_bottom（到达入口）判据 | ✅ 判据 A：白色连通域底线 `entry_bottom_y > 45`（做成可调宏 `BF_PVC_GATE_BOT_Y`） |
| 3 | valid 判定 | ✅ 与 `target_x` **绑定一致**：`valid = stable_detected`，`x = stable.target_x`（两者同源，不混用 raw） |
| 4 | enable 门控 | ✅ **无需修改**（专用 PVC 内嵌于 `bridge_fusion_frame`，跟随 `ShouldRunBridge()` 门控） |
| 5 | 准备脱出阶段 | ✅ 保留 ref 不变 |
| 6 | 旧 `bf_update_gate_bottom_ref` | ✅ **直接注释掉**（不删） |
| 7 | 专用 PVC 独立模块 | ✅ **复制 pvc_vision 为 `bridge_pvc_vision`，全部改名**，独立调参，不直接调用原 pvc 函数 |

---

## 八、风险与验证

### 8.1 风险

| 风险 | 说明 | 缓解 |
|---|---|---|
| R1 远处小入口漏检 | 专用 PVC 的 `MIN_AREA/MIN_WIDTH` 可能过滤远处小入口 | 独立调参：必要时放宽 `BRIDGE_PVC_VISION_MIN_AREA/MIN_WIDTH` 或降 `BRIDGE_PVC_VISION_WHITE_THRESHOLD` |
| R2 引导线无方向信息 | 竖直线只编码横向对准、无 heading | 0 核 ALIGN 已有"对齐容忍/锁角兜底"，先实测，不行再升级为带斜率线 |
| R3 白斑被红蓝线割裂 | PVC 只认白，桥面红蓝标线可能把白块切开 | 对比 ref 原效果；必要时调 PVC 合并参数 |
| R4 切换空窗 | 切到桥上(v8)后首几帧 b2 数据未稳 | 现有 `on_bridge_frames` 防跳边 + 0 核 8 帧防抖已覆盖 |
| R5 脱出阶段误受影响 | 拆分支后 ref 只在脱出阶段运行，逻辑被收紧 | 重点回归脱出线确认路径（exit_confirmed） |

### 8.2 验证

1. **编译**：`iarbuild` cm7_0 / cm7_1 双核 0 error；
2. **仅 1 核上位机**（`WIFI_CORE_SELECT=1`，渲染阶段编号）：进入任务前数字 `0` 且中线来自 PVC（对准入口）、到达后切 `1`（v8）、脱出前 `2`；
3. **0 核输入对照**：示波器/串口看 `b2_valid/gate/mode`，确认准备进入阶段 `b2_line_*` 由 PVC target_x 驱动；
4. **全链路**：Plan4 或侧键触发，确认 PVC 引导 → 入口到达抬腿 → 桥上 v8 → 脱出，全程无锁角长时间兜底；
5. **回放/实车标定**：标定 `BF_PVC_GATE_BOT_Y`（初值 45）。

---

## 九、与已有文档衔接

- 《PVC连通域RLE替换_详细落地规划.md》：本规划是其下游应用——RLE 版算法被复制为 `bridge_pvc_vision`，由单边桥"准备进入"阶段消费；
- 《单边桥PVC切换排查-执行规划与视觉复核.md》：本规划落成后，"准备进入"阶段引擎由 ref 检测器变为专用 PVC，渲染编号 `0` 的语义同步更新；
- 《远近融合检测接入迁移规划.md》：ref 引擎耗时问题（avg 9.25ms）在"准备进入"阶段被专用 PVC 规避，仅剩"准备脱出"阶段仍用 ref。
