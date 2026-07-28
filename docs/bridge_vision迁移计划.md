# bridge_vision 模块完整替换 — 迁移计划

> **版本**: v1.0 | **日期**: 2026-07-28  
> **源工程**: `D:\WORKS\2026LunTui\trials\bridge` (已验证可独立运行)  
> **目标工程**: `D:\WORKS\2026LunTui\project`  
> **原则**: 审批通过前严禁更改代码，本文档仅描述迁移方案与步骤。

---

## 一、背景与目标

### 1.1 当前状态

当前 `code1/vision/bridge_vision.c`（~194 行）是一个**薄适配层**，它：

- 调用 `bridge_detection.h` 中的 `bridge_detection_detect_gray()` 进行桥面检测
- 将检测结果 `BridgeDetectionResult` 导出为 `bridge_vision_frame_result_t`（线段坐标表示法）
- 做连续帧滤波，产出 `bridge_vision_output_t`（含 `raw`/`stable` 双副本 + streak 计数）
- 通过全局变量 `g_bridge_vision_output` 发布结果

### 1.2 新算法

`trials/bridge` 工程实现了一套**全新的单边桥三线透视结构提取算法**：

| 对比项 | 旧算法 (bridge_detection) | 新算法 (bridge_detect) |
|---|---|---|
| 算法文件 | `bridge_detection.h` (~10KB 头, 实现在 SDK 中) | `bridge_detect.c` (~1309 行) + `bridge_asm_ops.s` (~177 行) |
| 边缘检测 | 未知实现 | 4×4 可分离二项式-差分卷积（手写汇编 SMLAD 双发射） |
| 特征提取 | 固定阈值二值化 | lock-x 抑制 + p99 动态阈值 + 行背景判断(簇判据) |
| 线提取 | 未知 | 序贯 RANSAC（正/负响应分开） → 间距先验分类 → VP 共点精化 |
| 输出表示 | 5 条**线段坐标**（左/右/上/下/中线） | **直线方程**（x=a·y+b）红/绿/蓝三线 + mode 枚举 |
| 脱出线 | 无 | 门控粉色脱出线（亮区顶边界法，五重校验） |
| 性能 | 未知 | 水平 pass ~1.0 c/MAC，垂直 pass ~3.0 c/MAC，全流水线 int16 |

### 1.3 迁移目标

用 `trials/bridge` 中的新算法**完整替换** `code1/vision/bridge_vision.c` 及其依赖，同时保持对外接口兼容（IPC 数据包字段语义可调整但需同步更新消费者）。

---

## 二、涉及文件总览

### 2.1 源工程文件（需拷贝/迁移）

| 源文件 | 目标位置 | 说明 |
|---|---|---|
| `trials/bridge/project/code/bridge_detect.h` | `code1/vision/bridge_detect.h` | 新检测算法头文件（类型、API 声明） |
| `trials/bridge/project/code/bridge_detect.c` | `code1/vision/bridge_detect.c` | 新检测算法实现（~1309 行） |
| `trials/bridge/project/code/bridge_asm_ops.h` | `code1/vision/bridge_asm_ops.h` | 汇编算子 C 接口 |
| `trials/bridge/project/code/bridge_asm_ops.s` | `code1/vision/bridge_asm_ops.s` | 汇编算子实现（须加入 IAR 工程） |
| `trials/bridge/project/code/tcm.h` | `code1/vision/tcm.h` | TCM 放置宏（ITCM_FUNC / DTCM_BSS / DTCM_DATA） |

> 注意：如果目标工程 `code/` 或 `code1/` 中已存在 `tcm.h`，需比较内容后合并/复用，避免重复定义。

### 2.2 目标工程需修改的文件

| 文件 | 修改类型 | 说明 |
|---|---|---|
| `code1/vision/bridge_vision.h` | **重写** | 改为包装新 API 的适配层 |
| `code1/vision/bridge_vision.c` | **重写** | 改为调用 `bridge_detect_frame()` + 滤波 |
| `code1/vision/vision_ipc_core1.c` | **修改** | `vision_ipc_core1_fill_bridge()` 适配新输出结构 |
| `code1/vision/vision_ipc_core1.h` | **可能修改** | 如果 `vision_ipc_packet_t` 的 bridge 字段调整 |
| `code1/wifi.c` | **修改** | `render_bridge_vision_to_image()` 适配新输出 |
| `user/main_cm7_1.c` | **轻微修改** | include 路径可能调整 |

### 2.3 需删除/废弃的文件

| 文件 | 原因 |
|---|---|
| `code1/vision/bridge_detection.h` | 旧检测算法头文件，新算法不再依赖 |
| 旧 SDK 中 `bridge_detection` 的实现 | 如果以 .a/.o 提供则从链接中移除 |

### 2.4 IAR 工程配置变更

| 操作 | 说明 |
|---|---|
| **添加** `bridge_asm_ops.s` 到 CM7_1 工程 | 右键 → Add → Add Files |
| **添加** `bridge_detect.c` 到 CM7_1 工程 | 如果当前是统一编译 |
| **确认** ICF 链接脚本包含 TCM 段 | `SELF_ITCM` / `SELF_DTCM` 的 `place in` 和 `initialize by copy` 指令 |
| **确认** `code1/vision/` 在 include path 中 | 通常已配置 |

---

## 三、接口差异详细分析

### 3.1 初始化

| | 旧接口 | 新接口 |
|---|---|---|
| 函数 | `void bridge_vision_init(void)` | `void bridge_detect_init(bridge_state_t *st)` |
| 参数 | 无（内部全局状态） | 需传入 `bridge_state_t*`（跨帧状态：间距先验滑动窗 + 底部变白门控锁存） |

**迁移策略**：`bridge_vision_init()` 内部改为调用 `bridge_detect_init(&g_state)`，其中 `g_state` 为模块级静态变量。

### 3.2 帧处理

| | 旧接口 | 新接口 |
|---|---|---|
| 函数 | `void bridge_vision_process_camera_frame(const uint8 *gray)` | `void bridge_detect_frame(const uint8_t *img94, bridge_state_t *st, bridge_result_t *out)` |
| 输入 | 94×60 灰度图 | 94×60 灰度图（相同） |
| 输出方式 | 写入全局变量 `g_bridge_vision_output` | 写入调用者提供的 `bridge_result_t *out` |

**迁移策略**：`bridge_vision_process_camera_frame()` 改为调用 `bridge_detect_frame()`，然后将 `bridge_result_t` 转换为 `bridge_vision_frame_result_t` 并执行滤波。

### 3.3 输出结构体映射

这是迁移的**核心难点**——两种输出表示法语义完全不同：

#### 旧输出 `bridge_vision_frame_result_t`

```
线段坐标表示法：
  left_line   = (x0,y0) → (x1,y1)   // 左线线段
  right_line  = (x0,y0) → (x1,y1)   // 右线线段
  down_line   = (x0,y0) → (x1,y1)   // 下线（入口线）
  up_line     = (x0,y0) → (x1,y1)   // 上线（远端线）
  center_line = (x0,y0) → (x1,y1)   // 中线
  + detected, bridge_detected, state, geometry_valid
```

#### 新输出 `bridge_result_t`

```
直线方程表示法：
  mode: BRIDGE_MODE_NONE / R / B / M / RB / RMB / RM / MB / RB_Q
  has_red, has_green, has_blue, has_top  (标志位)
  red:   bridge_line_t { a, b, rms, n, u_lo, u_hi }   // x = a*y + b
  green: bridge_line_t { a, b, rms, n, u_lo, u_hi }
  blue:  bridge_line_t { a, b, rms, n, u_lo, u_hi }
  top:   bridge_line_t { a, b, rms, n, u_lo, u_hi }   // y = a*x + b
  spacing, mid_ratio, n_lines, n_rows_ok, gate
```

#### 映射方案

由于下游消费者（`vision_ipc_core1_fill_bridge()` → `vision_ipc_packet_t` → `vision_bridge_control.c`）使用 `bridge_center_line_*` 等坐标字段做控制，有两种方案：

**方案 A：在 bridge_vision 层做线段生成（推荐）**

新算法输出的是直线方程，我们在 `bridge_vision.c` 适配层中将其转换为线段坐标：

```c
// 伪代码：从 bridge_line_t 生成线段
void line_to_segment(const bridge_line_t *line, int y_min, int y_max,
                     int16 *x0, int16 *y0, int16 *x1, int16 *y1)
{
    if (line->n < MIN_LINE_INL) {
        // 无效
        *x0 = *y0 = *x1 = *y1 = BRIDGE_VISION_COORD_INVALID;
        return;
    }
    *y0 = (int16)line->u_lo;  // 支撑范围下界
    *y1 = (int16)line->u_hi;  // 支撑范围上界
    *x0 = (int16)(line->a * (*y0) + line->b);
    *x1 = (int16)(line->a * (*y1) + line->b);
    // 钳位到图像范围
}
```

- 左线 ← `red`
- 右线 ← `blue`  
- 中线 ← `green`（或由红蓝中点推导）
- 上线 ← `top`
- 下线：新算法无直接对应，可根据红蓝底部交点或固定 y=55 附近推算

**方案 B：改造下游消费者直接使用直线方程**

`vision_ipc_packet_t` 增加 `bridge_red_a/b/rms` 等字段，`vision_bridge_control.c` 改为使用直线方程做控制计算。工作量大但更精确。

> **建议先采用方案 A**，保持下游接口不变，后续可渐进迁移到方案 B。

### 3.4 滤波逻辑

旧模块在 `bridge_vision_update_filter()` 中维护：
- `detected_streak` / `lost_streak`
- `bridge_detected_streak` / `bridge_lost_streak`
- `stable_detected` / `bridge_stable_detected`
- `raw` / `stable` 双副本

新模块的 `bridge_state_t` 仅维护间距先验滑动窗和门控锁存，**不做帧间滤波**。

**迁移策略**：滤波逻辑保留在 `bridge_vision.c` 适配层中，对新输出进行同样的连续帧确认/丢失保持。

### 3.5 公开全局变量

旧模块公开 4 个全局变量：

| 变量 | 类型 | 新模块是否保持 |
|---|---|---|
| `g_bridge_vision_output` | `bridge_vision_output_t` | ✅ 保持（适配层继续填充） |
| `g_bridge_vision_output_write_busy` | `uint8` | ✅ 保持 |
| `g_bridge_vision_cost_profiler` | `runtime_profiler_t` | ✅ 保持 |
| `g_bridge_vision_frame_profiler` | `runtime_profiler_t` | ✅ 保持 |

---

## 四、调用链变更

### 4.1 当前调用链

```
main_cm7_1.c
  └─ bridge_vision_process_camera_frame(gray)
       └─ bridge_detection_detect_gray()        ← 旧检测器
       └─ bridge_vision_export_result()          ← 结构转换
       └─ bridge_vision_update_filter()          ← 滤波
       └─ bridge_vision_publish()                ← 发布

vision_ipc_core1.c
  └─ vision_ipc_core1_fill_bridge()
       └─ bridge_vision_get_output()             ← 读取
       └─ 填充 bridge_center_line_x0 等

wifi.c
  └─ render_bridge_vision_to_image()
       └─ bridge_vision_get_output()             ← 读取
       └─ 画 left/right/up/down/center 线段

code/vision/vision_bridge_control.c (Core 0)
  └─ 接收 IPC → 读取 bridge_center_line_*
```

### 4.2 新调用链（目标）

```
main_cm7_1.c
  └─ bridge_vision_process_camera_frame(gray)
       └─ bridge_detect_frame()                  ← ★ 新检测器
       └─ bridge_vision_export_result()          ← ★ 重写：line方程→线段
       └─ bridge_vision_update_filter()          ← 保持滤波逻辑
       └─ bridge_vision_publish()                ← 保持发布

vision_ipc_core1.c
  └─ vision_ipc_core1_fill_bridge()
       └─ 填充逻辑不变（如果采用方案 A）

wifi.c
  └─ render_bridge_vision_to_image()
       └─ 画线段逻辑不变（如果采用方案 A）
```

---

## 五、迁移步骤（共 7 步）

### 第 1 步：文件拷贝

将以下 5 个文件拷贝到 `code1/vision/`：

```
trials/bridge/project/code/bridge_detect.h   → code1/vision/bridge_detect.h
trials/bridge/project/code/bridge_detect.c   → code1/vision/bridge_detect.c
trials/bridge/project/code/bridge_asm_ops.h  → code1/vision/bridge_asm_ops.h
trials/bridge/project/code/bridge_asm_ops.s  → code1/vision/bridge_asm_ops.s
trials/bridge/project/code/tcm.h             → code1/vision/tcm.h  (若已存在则比较合并)
```

**风险**：`tcm.h` 可能与 `code/tcm.h` 冲突。检查 `code/tcm.h`（目标工程已有），可能需要统一为一个文件，或给新的改名。

### 第 2 步：IAR 工程配置

1. 将 `bridge_asm_ops.s` 添加到 CM7_1 工程
2. 将 `bridge_detect.c` 添加到 CM7_1 工程
3. 确认 ICF 链接脚本中 TCM 段配置完整
4. 从 CM7_1 工程中移除旧 `bridge_detection` 的 .a/.o 文件（如有）
5. 确认 `code1/vision/` 目录在 include path 中

### 第 3 步：重写 bridge_vision.h

保留对外接口签名不变，内部 include 改为新头文件：

```c
// 旧
#include "bridge_detection.h"

// 新
#include "bridge_detect.h"
```

`bridge_vision_frame_result_t` 和 `bridge_vision_output_t` 结构体定义保持不变（方案 A）。

宏常量 `BRIDGE_VISION_IMAGE_W`(94) / `BRIDGE_VISION_IMAGE_H`(60) 保持不变，与 `bridge_detect.h` 中的 `BRIDGE_W`/`BRIDGE_H` 一致。

新增内部使用的宏或常量：
- `BRIDGE_VISION_PROFILE_TIMER` 保持 `TC_TIME2_CH1`（确认新工程中该定时器可用）

### 第 4 步：重写 bridge_vision.c

```c
// === 新增静态状态 ===
static bridge_state_t  g_bridge_state;     // 新检测器的跨帧状态
static bridge_result_t g_bridge_result;    // 新检测器的单帧结果

// === bridge_vision_init() 改写 ===
void bridge_vision_init(void)
{
    bridge_detect_init(&g_bridge_state);    // ← 替换旧 bridge_detection_default_config
    bridge_vision_reset_filter();
    timer_init(BRIDGE_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(BRIDGE_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_bridge_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_bridge_vision_frame_profiler);
    g_bridge_last_frame_time_us = timer_get(BRIDGE_VISION_PROFILE_TIMER);
}

// === bridge_vision_reset_filter() 改写 ===
void bridge_vision_reset_filter(void)
{
    bridge_detect_init(&g_bridge_state);    // 重置跨帧状态
    memset(&g_bridge_output_shadow, 0, sizeof(g_bridge_output_shadow));
    bridge_vision_publish(&g_bridge_output_shadow);
}

// === bridge_vision_process_camera_frame() 改写 ===
void bridge_vision_process_camera_frame(const uint8 *gray)
{
    // ... 帧间隔统计不变 ...

    RUNTIME_PROFILE_BEGIN(...);
    
    bridge_detect_frame(gray, &g_bridge_state, &g_bridge_result);  // ★ 核心替换
    
    bridge_vision_export_result(&g_bridge_result, &raw);           // ★ 重写
    bridge_vision_update_filter(&raw);
    
    RUNTIME_PROFILE_END(...);
}
```

**核心工作**：重写 `bridge_vision_export_result()` —— 将 `bridge_result_t`（直线方程）转换为 `bridge_vision_frame_result_t`（线段坐标）。

```c
static void bridge_vision_export_result(const bridge_result_t *res,
                                        bridge_vision_frame_result_t *frame)
{
    bridge_vision_clear_frame(frame);
    
    // 1. 基本状态
    frame->bridge_detected = (res->mode != BRIDGE_MODE_NONE) ? 1U : 0U;
    frame->state = (uint8)res->mode;  // 旧 state 是 BridgeDetectionState，新用 bridge_mode_t
    
    // 2. 几何有效 = 至少中线存在或有红蓝双线
    frame->geometry_valid = (res->has_green || 
                             (res->has_red && res->has_blue)) ? 1U : 0U;
    frame->detected = frame->geometry_valid;
    
    // 3. 左线 ← red
    if (res->has_red) {
        line_v_to_segment(&res->red, &frame->left_line_x0, ...);
    }
    // 4. 右线 ← blue
    if (res->has_blue) {
        line_v_to_segment(&res->blue, &frame->right_line_x0, ...);
    }
    // 5. 中线 ← green（或由红蓝中点推导）
    if (res->has_green) {
        line_v_to_segment(&res->green, &frame->center_line_x0, ...);
    } else if (res->has_red && res->has_blue) {
        // 红蓝中点作为中线
        mid_line_from_rb(&res->red, &res->blue, &frame->center_line_x0, ...);
    }
    // 6. 上线 ← top (注意: top 是 y = a*x + b，坐标系不同)
    if (res->has_top) {
        line_h_to_segment(&res->top, &frame->up_line_x0, ...);
    }
    // 7. 下线 ← 红蓝底部估算（新算法无直接 entry_line）
    //    可固定 y≈55 附近取红蓝交点
}
```

### 第 5 步：修改 vision_ipc_core1.c

如果采用方案 A（线段坐标保持不变），`vision_ipc_core1_fill_bridge()` **基本无需改动**，字段映射保持：

| IPC 字段 | 来源 |
|---|---|
| `bridge_detected` | `bridge_output->bridge_raw_detected` |
| `bridge_stable_detected` | `bridge_output->bridge_stable_detected` |
| `bridge_geometry_detected` | `bridge_output->raw_detected` |
| `bridge_geometry_stable_detected` | `bridge_output->stable_detected` |
| `bridge_state` | `ctrl->state`（语义变为 `bridge_mode_t`） |
| `bridge_center_line_x0/y0/x1/y1` | `ctrl->center_line_*` |
| `bridge_left_line_*` | `ctrl->left_line_*` |
| `bridge_right_line_*` | `ctrl->right_line_*` |
| `bridge_up_line_*` | `ctrl->up_line_*` |
| `bridge_down_line_*` | `ctrl->down_line_*` |

**注意**：`packet->bridge_state` 的语义从 `BridgeDetectionState`（NONE/PREPARE_ENTER/ON_BRIDGE/PREPARE_EXIT）变为 `bridge_mode_t`（NONE/R/B/M/RB/RMB/...）。`vision_bridge_control.c`（Core 0 消费者）需要相应调整状态机判断逻辑。

### 第 6 步：修改 wifi.c 渲染

`render_bridge_vision_to_image()` 读取线段坐标画线，如果方案 A 保持线段坐标不变，则**无需修改**。

### 第 7 步：修改 Core 0 消费者

`code/vision/vision_bridge_control.c` 中使用了 `packet->bridge_state` 来做状态判断（如 `BRIDGE_DETECTION_STATE_ON_BRIDGE`）。新算法输出的是 `bridge_mode_t`，需要做语义映射：

| 旧 state | 新 mode | 控制含义 |
|---|---|---|
| `BRIDGE_DETECTION_STATE_NONE` | `BRIDGE_MODE_NONE` | 无桥 |
| `BRIDGE_DETECTION_STATE_PREPARE_ENTER` | `BRIDGE_MODE_RB`（仅红蓝无线） | 接近桥 |
| `BRIDGE_DETECTION_STATE_ON_BRIDGE` | `BRIDGE_MODE_RMB` | 在桥上（三线完整） |
| `BRIDGE_DETECTION_STATE_PREPARE_EXIT` | `has_top == 1` | 准备下桥（顶线出现） |

---

## 六、风险评估与对策

| 风险 | 等级 | 对策 |
|---|---|---|
| **内存溢出**：新算法静态缓冲约 `s_gx[57][91]` + `s_gy[57][91]` ≈ 20KB int16 + RANSAC 工作区 ~8KB + BFS 区域位图 ~1.8KB，总计 ~30KB。现有 DTCM 16KB 不够 | 🔴 高 | 将 `s_gx`/`s_gy` 等大缓冲用 `DTCM_BSS` 以外的方式放在 SRAM；只将 `s_raw`/`s_ringx`/`s_ringy` 保留在 DTCM |
| **浮点运算**：新算法大量使用 `float`（RANSAC、最小二乘拟合、间距计算），Cortex-M7 有硬件 FPU 但需确认 FPU 上下文在中断/IPC 中正确保存 | 🟡 中 | 确认 `vision_ipc_core1` 的 PIT 中断中是否正确保存/恢复 FPU 寄存器；检查 IAR 工程是否开启 FPU（`__FPU_PRESENT`） |
| **`tcm.h` 冲突**：目标工程 `code/tcm.h` 与新版 `code1/vision/tcm.h` 可能重复定义 `ITCM_FUNC` 等宏 | 🟡 中 | 统一为一个文件，放在 `code/` 或 `code1/` 公共路径 |
| **`bridge_state` 语义变化**：旧 `bridge_detection_state` 与新 `bridge_mode_t` 枚举值不完全对应，Core 0 状态机可能误判 | 🟡 中 | 在适配层做映射，或修改 `vision_bridge_control.c` 的状态判断条件 |
| **下线（entry_line）缺失**：新算法不提供 entry_line，旧 IPC 中有 `bridge_down_line_*` | 🟢 低 | 在适配层中由红蓝底部估算 entry_line；如果下游不使用可填无效值 |
| **IAR 汇编文件编译**：`bridge_asm_ops.s` 依赖 `.itcm_text` section，需确认 ICF 配置正确 | 🟡 中 | 对照 trials 工程的 ICF 验证目标工程的 ICF 配置 |
| **测试图片依赖**：`bridge_test_images.c/h` 仅在 trials 工程中用于独立验证，迁移后不用拷贝 | 🟢 低 | 不拷贝，直接在实车图像上测试 |

---

## 七、验证计划

### 7.1 编译验证

1. IAR 工程 Clean + Rebuild，确认无编译错误、无链接错误
2. 检查 `.map` 文件中 ITCM/DTCM 占用是否在 16KB 限制内
3. 检查 `.map` 文件中 `.itcm_text` section 是否放置于 `SELF_ITCM` 地址

### 7.2 功能验证

1. 使用 trials 工程中的 6 帧测试图像，确认新模块输出与 trials 工程一致
2. 在真车上运行，通过 WiFi 调试渲染确认线段绘制正确
3. 通过 IPC 在 Core 0 端观察 `bridge_center_line_*` 坐标是否合理

### 7.3 性能验证

1. 通过 `g_bridge_vision_cost_profiler` 观察新的帧处理耗时，与旧算法对比
2. 确认帧间隔稳定，无超时丢帧

### 7.4 回归验证

1. 科目二（单边桥）完整跑一遍，确认控制效果不退化
2. 确认 PVC 和 bumpy 视觉任务不受影响

---

## 八、审批检查清单

| 检查项 | 状态 |
|---|---|
| 方案 A（线段适配）vs 方案 B（直线方程直传）已确认 | ☐ |
| 内存预算已核算（DTCM ≤ 16KB, 总 SRAM 足够） | ☐ |
| FPU 上下文保存已确认（中断中无浮点寄存器踩踏） | ☐ |
| `tcm.h` 去重方案已确定 | ☐ |
| Core 0 `vision_bridge_control.c` 状态机修改方案已确认 | ☐ |
| 测试图像准备就绪 | ☐ |
| 可回滚（git branch 或备份）已准备 | ☐ |

---

## 附录 A：新旧文件对照表

```
[旧] code1/vision/bridge_vision.h        → [保留，内容重写]
[旧] code1/vision/bridge_vision.c        → [保留，内容重写]
[旧] code1/vision/bridge_detection.h     → [删除/废弃]
[新] code1/vision/bridge_detect.h        ← trials/bridge/project/code/bridge_detect.h
[新] code1/vision/bridge_detect.c        ← trials/bridge/project/code/bridge_detect.c
[新] code1/vision/bridge_asm_ops.h       ← trials/bridge/project/code/bridge_asm_ops.h
[新] code1/vision/bridge_asm_ops.s       ← trials/bridge/project/code/bridge_asm_ops.s
[新] code1/vision/tcm.h                  ← trials/bridge/project/code/tcm.h (注意去重)
[改] code1/vision/vision_ipc_core1.c
[改] code1/vision/vision_ipc_core1.h     (如 vision_ipc_packet_t 字段调整)
[改] code1/wifi.c                        (小幅，视方案而定)
[改] code/vision/vision_bridge_control.c (state 语义适配)
[不改] user/main_cm7_1.c                 (include 路径可能微调)
```

## 附录 B：新算法静态缓冲内存估算

| 缓冲区 | 大小 | 位置建议 |
|---|---|---|
| `s_img[60][94]` | 5,640 B | SRAM |
| `s_gx[57][91]` | 10,374 B (int16) | SRAM |
| `s_gy[57][91]` | 10,374 B (int16) | SRAM |
| `s_pos[57*2]` | ~1,368 B (bpt_t=12B) | SRAM |
| `s_neg[57*2]` | ~1,368 B | SRAM |
| `s_topc[91*2]` | ~2,184 B | SRAM |
| `s_rem` | ~2,184 B | SRAM |
| `s_lines[8]` | ~2,400 B (iline_t) | SRAM |
| `s_region[60][3]*4` | 720 B | SRAM |
| `s_bfs_q[94*60]*2` | 11,280 B | SRAM |
| `s_raw[96]` | 192 B (int16) | **DTCM** |
| `s_ringx[4][92]` | 736 B (int16) | **DTCM** |
| `s_ringy[4][92]` | 736 B (int16) | **DTCM** |
| **DTCM 合计** | **~1.7 KB** | ✅ 远小于 16KB |
| **SRAM 合计** | **~48 KB** | 需确认目标平台可用 SRAM |
