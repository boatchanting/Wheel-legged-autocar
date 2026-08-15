# PVC 连通域替换 —— 游程编码 (RLE) 多阈值提取 详细落地规划

> **文档版本**: v1.0  
> **创建日期**: 2026-07-27  
> **目标平台**: 双核架构，Core 1 Cortex-M7，IAR EWARM  
> **参考工程**: `D:\WORKS\2026TUsmart-Gited\new-vision\project\code\cc_extract.c`  
> **状态**: ⚠️ **待审批。审批通过前禁止修改任何代码。**

---

## 目录

1. [改动范围与不变范围](#一改动范围与不变范围)
2. [现状分析：当前 Flood Fill 方案的完整解剖](#二现状分析当前-flood-fill-方案的完整解剖)
3. [参考工程核心算法解析](#三参考工程核心算法解析)
4. [替换方案：RLE 游程编码连通域提取](#四替换方案rle-游程编码连通域提取)
5. [逐函数改动对照表](#五逐函数改动对照表)
6. [新增函数完整规格](#六新增函数完整规格)
7. [数据结构变更详情](#七数据结构变更详情)
8. [内存布局对比](#八内存布局对比)
9. [外部接口兼容性保证](#九外部接口兼容性保证)
10. [分阶段实施步骤](#十分阶段实施步骤)
11. [验证策略](#十一验证策略)
12. [风险评估与缓解](#十二风险评估与缓解)

---

## 一、改动范围与不变范围

### 1.1 仅修改的文件

| 文件 | 改动类型 | 说明 |
|------|----------|------|
| `code1/vision/pvc_vision.c` | **重写内部实现** | 所有 `static` 函数和数据结构的替换 |
| ~~`code1/vision/pvc_vision.h`~~ | **不改** | 所有对外类型、宏、函数声明保持不变 |

### 1.2 绝对不动的文件（零改动）

| 文件 | 原因 |
|------|------|
| `code1/vision/pvc_vision.h` | 对外接口完全不变 |
| `code1/vision/vision_ipc_core1.c` | 只读 `pvc_vision_get_output()` 和 `g_pvc_vision_output` |
| `code1/vision/vision_ipc_core1.h` | 只 `#include "pvc_vision.h"` |
| `code1/wifi.c` | `render_pvc_vision_to_image()` 只读 `g_pvc_vision_output` |
| `code1/wifi.h` | 无改动 |
| `code1/tools/camera_menu.c` | 无改动 |
| `code/vision/vision_pvc_control.c` | Core 0 控制侧，完全不变 |
| `code/vision/vision_pvc_control.h` | Core 0 控制侧，完全不变 |
| `code/vision/vision_three_stage_control.c` | Core 0 三级跳，完全不变 |
| `user/main_cm7_1.c` | 调用方不变 |

### 1.3 只改 `pvc_vision.c` 内部的原因

```
┌─────────────────────────────────────────────────────────────────┐
│  对外接口层 (pvc_vision.h) — 零改动                               │
│                                                                   │
│  extern g_pvc_vision_output       ← 不变                         │
│  extern g_pvc_vision_cost_profiler ← 不变                        │
│  pvc_vision_init()                ← 签名不变                     │
│  pvc_vision_reset_filter()        ← 签名不变                     │
│  pvc_vision_get_output()          ← 签名不变                     │
│  pvc_vision_process_camera_frame() ← 签名不变                    │
│                                                                   │
│  pvc_vision_frame_result_t        ← 结构体定义不变               │
│  pvc_vision_output_t              ← 结构体定义不变               │
├─────────────────────────────────────────────────────────────────┤
│  内部实现层 (pvc_vision.c) — 全部替换                             │
│                                                                   │
│  pvc_scratch_t          → 删除，替换为游程缓冲                    │
│  pvc_component_t        → 删除，替换为 pvc_blob_t (内部)          │
│  pvc_flood_component()  → 删除                                    │
│  pvc_collect_components() → 替换为 pvc_extract_level()           │
│  pvc_filter_candidates() → 合并到 pvc_detect_frame()             │
│  pvc_sort_by_area()     → 删除                                    │
│  pvc_sort_by_score()    → 删除 (改为遍历维护 Top-1)              │
│  pvc_copy_best_to_result() → 内联到 pvc_detect_frame()           │
│  pvc_detect_frame()     → 重写主流程                              │
│                                                                   │
│  新增: pvc_run_t, pvc_blob_t (内部类型)                           │
│  新增: pvc_scan_row(), pvc_merge_rows(), pvc_extract_level()     │
│  新增: pvc_blob_finalize(), pvc_score_blob()                     │
│                                                                   │
│  不动: pvc_update_filter(), pvc_clear_frame_result(),            │
│        pvc_score_component() 公式, pvc_estimate_*(), IPM, 防抖    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 二、现状分析：当前 Flood Fill 方案的完整解剖

### 2.1 当前数据流

```
pvc_vision_process_camera_frame(gray)
  │
  ├─ pvc_detect_frame(gray, &raw)
  │   │
  │   ├─ pvc_collect_components(gray)
  │   │   ├─ memset(visited, 0, 5640)          ← 每帧清零
  │   │   ├─ for i=0..5639:                    ← 全图逐像素扫描
  │   │   │   if !visited[i] && gray[i]>=200:
  │   │   │     pvc_flood_component(gray, i, &components[count])
  │   │   │       ├─ BFS 四邻域展开 (栈操作, 分支密集)
  │   │   │       ├─ 累加 area, sum_x, sum_y, sum_gray
  │   │   │       ├─ 更新 xmin/xmax/ymin/ymax
  │   │   │       └─ 计算 centroid, fill_ratio, touches_border,
  │   │   │          mean_gray (全是 float)
  │   │   └─ pvc_sort_by_area(components, count)  ← 全排序
  │   │
  │   ├─ pvc_filter_candidates(count)
  │   │   ├─ for each component:
  │   │   │   ├─ pvc_score_component()  ← 浮点打分(先打分后淘汰!)
  │   │   │   ├─ 面积<120? 淘汰
  │   │   │   ├─ 宽<12 或 高<4? 淘汰
  │   │   │   └─ fill_ratio<0.25? 淘汰
  │   │   └─ pvc_sort_by_score(candidates, count) ← 全排序
  │   │
  │   └─ if candidates>0:
  │       pvc_copy_best_to_result(gray, best, result)
  │         ├─ pvc_extract_target_x_from_bottom_rows(gray, best)
  │         │   ← 重新扫描 best 的 bbox 底部12行! (二次遍历)
  │         ├─ pvc_estimate_forward_mm_from_row(best->ymax)
  │         ├─ pvc_estimate_lateral_mm_from_x(target_x)
  │         └─ pvc_fill_physical_coord_from_ipm(best, result)
  │
  └─ pvc_update_filter(&raw)
      ├─ 防抖 (3帧确认/2帧保持)
      ├─ IIR 平滑 ((3×旧+新)/4)
      └─ 写回 g_pvc_vision_output (带写保护锁)
```

### 2.2 当前方案的性能瓶颈

| 瓶颈 | 位置 | 量化 |
|------|------|------|
| `visited[5640]` memset | `pvc_collect_components()` | 每帧清零 5640 字节 |
| BFS 栈操作 | `pvc_flood_component()` | 每个像素 4 次分支 + push/pop |
| 访问模式不连续 | Flood Fill | 四邻域跳转, Cache miss 严重 |
| 先打分后淘汰 | `pvc_filter_candidates()` | 注定淘汰的噪声块也做了浮点打分 |
| 两次全排序 | `pvc_sort_by_area()` + `pvc_sort_by_score()` | O(N²) 插入排序, 32 个元素 |
| 二次扫描底部行 | `pvc_extract_target_x_from_bottom_rows()` | 在 best bbox 内重新逐像素扫描 |
| 浮点计算密集 | 多处 | centroid, fill_ratio, mean_gray, score 都是 float |

### 2.3 当前内存占用

```c
typedef struct {
    uint8  visited[5640];                    // 5640 bytes
    uint16 stack[5640];                      // 11280 bytes  ← 最大项
    pvc_component_t components[32];          // 32 × 44 ≈ 1408 bytes
    pvc_component_t candidates[32];          // 32 × 44 ≈ 1408 bytes
} pvc_scratch_t;
// 总: ~19736 bytes (≈ 19.3 KB)
```

---

## 三、参考工程核心算法解析

参考工程 `cc_extract.c` 的游程编码连通域提取流程：

```
cc_process_frame(whitehat, original, &out)
  │
  ├─ 步骤1: cc_gen_level_bits(whitehat, thr, num_levels)
  │   生成多层级打包位掩码 (C 版或 ASM 版 USUB8 并行)
  │
  ├─ 步骤2: FOR each level:
  │   cc_extract_level(level_bits[lv], ..., whitehat, original)
  │     │
  │     ├─ FOR y=0..119:
  │     │   ├─ cc_scan_row_level(row_bits) → curr_runs[]
  │     │   │   逐字节 bit 扫描, 提取连续前景段
  │     │   │
  │     │   ├─ cc_merge_rows_level(curr, prev, next)
  │     │   │   阶段B: 全背景行 → prev runs skip_cnt++
  │     │   │   阶段C: 双指针重叠检测 → 继承 blob_idx
  │     │   │   阶段D: 孤儿 run → 分配新 blob_idx
  │     │   │   阶段E: 同 blob 行内缝合 (间隙 ≤15px)
  │     │   │   阶段F: 面积/bbox/剖面 增量累计 ← 一次产出所有变量
  │     │   │   阶段G: 当前行推入 next_prev
  │     │   │
  │     │   └─ cc_swap_run_buffers()  ← 三缓冲旋转, 无 memcpy
  │     │
  │     └─ 筛选: 面积/宽高在范围内 → 压缩有效 blob
  │
  ├─ 步骤3: cc_unify_output() → 跨层关联 + 统一输出
  │
  └─ 步骤4-6: 跨帧追踪 + 形状评分 + HDM
```

### 3.1 关键设计原则（对齐到 PVC）

| 参考工程做法 | PVC 适配 |
|-------------|---------|
| 多层级 bitmask 生成 | PVC 只需**单层级**，可跳过 bitmask 步骤，直接在 `pvc_scan_row()` 中判断 `gray[x] >= threshold` |
| 双指针跨行继承 | 直接复用，判定条件 `overlap > curr_len/2` |
| 三缓冲指针旋转 | 直接复用 |
| 阶段 F 增量累加 | 直接复用，但累加的字段按 PVC 需求定制 |
| 剖面采集器 (profile) | PVC 不需要，删除 |
| 跨层关联 | PVC 不需要，删除 |
| 跨帧追踪 | PVC 已有 `pvc_update_filter()`，不动 |

### 3.2 PVC 适配简化

PVC 的最大简化点在于：**只有单一阈值**。这意味着：

- 不需要 `cc_gen_level_bits()` — 不需要打包位掩码
- 不需要 `g_level_bits[][]` 数组
- 直接在 `pvc_scan_row()` 中用 `gray[x] >= threshold` 判断
- 不需要跨层关联 (`cc_detect_containment`)
- 不需要 `cc_unify_output()`

核心保留的是：**逐行游程扫描 + 双指针跨行合并 + 阶段 F 增量累加 + 三缓冲旋转**。

---

## 四、替换方案：RLE 游程编码连通域提取

### 4.1 新数据流

```
pvc_vision_process_camera_frame(gray)   ← 接口不变
  │
  ├─ pvc_detect_frame(gray, &raw)       ← 内部重写
  │   │
  │   ├─ [新] pvc_extract_level(gray, 200, blob_pool, 32)
  │   │   │
  │   │   ├─ FOR y=0..59:
  │   │   │   ├─ [新] pvc_scan_row(gray[y], 200, curr_buf)
  │   │   │   │   顺序扫描 94 像素, 提取连续白色游程
  │   │   │   │   输出: x0, x1 (uint8)
  │   │   │   │
  │   │   │   ├─ [新] pvc_merge_rows(curr, prev, next, blob_pool, y)
  │   │   │   │   阶段B: 全背景行 → prev runs skip_cnt++
  │   │   │   │   阶段C: 双指针重叠检测 → 继承 blob_idx
  │   │   │   │   阶段D: 孤儿 → 分配新 blob_idx
  │   │   │   │   阶段E: 同 blob 行内缝合
  │   │   │   │   阶段F: ★ 增量累加 ★
  │   │   │   │     ├─ area += run_len
  │   │   │   │     ├─ sum_x += (x0+x1)*run_len/2
  │   │   │   │     ├─ sum_y += y*run_len
  │   │   │   │     ├─ sum_gray += Σgray[x]  (run 内累加)
  │   │   │   │     ├─ 更新 min_x/max_x/min_y/max_y
  │   │   │   │     ├─ touches_border 检测 (短路)
  │   │   │   │     └─ bottom_row_data 环形记录
  │   │   │   │   阶段G: 行交接
  │   │   │   │
  │   │   │   └─ [新] pvc_swap_buffers()  三缓冲旋转
  │   │   │
  │   │   └─ 扫描完成后筛选(面积/宽高) + 压缩有效 blob
  │   │
  │   ├─ [新] pvc_blob_finalize(blob_pool, count)
  │   │   计算 bbox_area (遍历期没算的补齐)
  │   │
  │   ├─ [新] 边打分边维护 Top-1 (替代 pvc_filter_candidates 的两遍排序)
  │   │   for each valid blob:
  │   │     fill_ratio = area / bbox_area
  │   │     mean_gray = sum_gray / area
  │   │     if 硬淘汰条件: continue   ← 先淘汰后打分!
  │   │     score = pvc_score_blob(blob, fill_ratio, mean_gray)
  │   │     if score > best_score: best = blob
  │   │
  │   └─ if best:
  │       [内联] pvc_copy_best_to_result 逻辑
  │         ├─ target_x = 从 bottom_row_data 环形缓冲计算 ← 不再二次扫描!
  │         ├─ 填充 result 各字段
  │         ├─ pvc_estimate_forward_mm_from_row()
  │         ├─ pvc_estimate_lateral_mm_from_x()
  │         └─ IPM 物理坐标
  │
  └─ pvc_update_filter(&raw)           ← 完全不动
```

### 4.2 核心加速点总结

| 优化项 | 原来 | 现在 |
|--------|------|------|
| 内存访问 | BFS 随机跳转 | 逐行顺序扫描, Cache 友好 |
| visited 清零 | `memset(visited, 5640)` 每帧 | 无 (不需要 visited) |
| 栈操作 | 每次 BFS push/pop, 最坏 5640 次 | 无 |
| 打分顺序 | 先打分后淘汰 | 先淘汰后打分 |
| 排序 | `sort_by_area` + `sort_by_score` 两次全排序 | 遍历维护 Top-1, O(N) |
| 底部行提取 | 二次扫描 bbox 内像素 | 从遍历期 bottom_row_data 直接读取 |
| 浮点计算 | 每个 component 都算 centroid/fill/mean | 仅 Top-1 候选需要 (或淘汰时也只需要整数比较) |

---

## 五、逐函数改动对照表

### 5.1 删除的函数

| 函数 | 行号 (约) | 删除原因 |
|------|-----------|----------|
| `pvc_flood_component()` | ~230-370 | BFS 连通域, 替换为游程扫描 |
| `pvc_collect_components()` | ~375-410 | 全图 Flood Fill 扫描, 替换为 `pvc_extract_level()` |
| `pvc_filter_candidates()` | ~415-460 | 两遍排序+过滤, 合并到 `pvc_detect_frame()` 中边打分边维护 Top-1 |
| `pvc_sort_by_area()` | ~320-335 | 不再需要全排序 |
| `pvc_sort_by_score()` | ~298-315 | 不再需要全排序 |
| `pvc_component_width()` | ~88-91 | pvc_component_t 删除后自然失效 |
| `pvc_component_height()` | ~93-96 | pvc_component_t 删除后自然失效 |
| `pvc_copy_best_to_result()` | ~465-500 | 内联到 `pvc_detect_frame()` |

### 5.2 修改的函数

| 函数 | 改动 |
|------|------|
| `pvc_detect_frame()` | 完全重写主流程, 签名不变 |
| `pvc_fill_physical_coord_from_ipm()` | 参数从 `pvc_component_t*` 改为直接传入 `centroid_x, ymax` |
| `pvc_score_component()` | 保留公式, 改为接受 `pvc_blob_t*` (或直接 float 参数) |

### 5.3 新增的函数

| 函数 | 职责 |
|------|------|
| `pvc_scan_row()` | 逐行提取游程 |
| `pvc_merge_rows()` | 跨行合并 + 增量累加 (阶段 B~G) |
| `pvc_swap_buffers()` | 三缓冲指针旋转 |
| `pvc_extract_level()` | 单阈值完整游程提取 |
| `pvc_blob_finalize()` | 遍历后统一计算派生变量 |
| `pvc_score_blob()` | 从 `pvc_blob_t` 计算分数 (复用 pvc_score_component 公式) |
| `pvc_extract_target_x_from_blob()` | 从 bottom_row_data 环形缓冲计算目标 X (替代原二次扫描版本) |

### 5.4 不变更的函数

| 函数 | 原因 |
|------|------|
| `pvc_min_f()`, `pvc_max_f()` | 工具函数, 继续使用 |
| `pvc_float_to_i16_x100()` | 不变 |
| `pvc_estimate_forward_mm_from_row()` | 不变 |
| `pvc_estimate_lateral_mm_from_x()` | 不变 |
| `pvc_clear_frame_result()` | 不变 |
| `pvc_update_filter()` | 不变 (防抖/平滑逻辑完全不涉及连通域) |
| `pvc_vision_init()` | 不变 (只做初始化, 改为初始化游程缓冲) |
| `pvc_vision_reset_filter()` | 不变 |
| `pvc_vision_get_output()` | 不变 |
| `pvc_vision_process_camera_frame()` | 不变 (只做框架调用) |

---

## 六、新增函数完整规格

### 6.1 `pvc_scan_row()` — 逐行游程提取

```c
/**
 * @brief 扫描一行灰度图, 提取所有连续白色游程
 *
 * 替换原有 pvc_collect_components() 中的逐像素 Flood Fill 扫描。
 * 顺序访问内存, 缓存友好。
 *
 * @param gray_row   当前行的灰度数据 (94 像素, uint8 数组)
 * @param threshold  白色判定阈值 (如 PVC_VISION_WHITE_THRESHOLD = 200)
 * @param out_runs   输出的游程数组
 * @param max_runs   游程数组容量
 * @return           本行提取的游程数量
 */
static uint8 pvc_scan_row(
    const uint8 *gray_row,
    uint8 threshold,
    pvc_run_t *out_runs,
    uint8 max_runs);

/* 实现逻辑:
 *   x = 0;
 *   while (x < 94) {
 *     while (x < 94 && gray_row[x] < threshold) x++;   // 跳过背景
 *     if (x >= 94) break;
 *     x0 = x;
 *     while (x < 94 && gray_row[x] >= threshold) x++;  // 扫描前景
 *     x1 = x - 1;
 *     out_runs[run_count++] = {x0, x1, 0xFF, 0};
 *   }
 */
```

### 6.2 `pvc_merge_rows()` — 跨行合并 + 增量累加

```c
/**
 * @brief 跨行游程合并 —— 整条管线的核心
 *
 * 七个阶段一气呵成:
 *   B) 全背景行处理: 当前行无游程 → prev runs skip_cnt++
 *   C) 双指针重叠检测: overlap > curr_len/2 → 继承 blob_idx
 *   D) 孤儿分配: 未继承的 curr run → 分配新 blob_idx, 初始化 blob
 *   E) 行内缝合: 同 blob、间距 ≤ GAP_MAX → 合并 run
 *   F) ★ 增量累加 ★: area, sum_x, sum_y, sum_gray, bbox, touches_border,
 *                     bottom_row_data — 遍历期一次产出所有变量
 *   G) 行交接: 当前行有 blob_idx 的 run → 推入 next_prev
 *
 * @param curr_runs       当前行游程 (in, 阶段E会修改)
 * @param curr_count      当前行游程数 (in, 阶段E会修改)
 * @param prev_runs       上一行游程 (in/out)
 * @param prev_count      上一行游程数 (in/out, 阶段B会修改)
 * @param next_runs       下一行"的"上一行"游程缓冲 (out)
 * @param next_count      输出游程数
 * @param blob_pool       blob 累加器池
 * @param max_blobs       池容量
 * @param next_blob_idx   下一个可用的 blob 索引 (in/out)
 * @param y               当前行号
 * @param gray_row        当前行灰度数据
 */
static void pvc_merge_rows(
    pvc_run_t *curr_runs,  uint8 curr_count,
    pvc_run_t *prev_runs,  uint8 *prev_count,
    pvc_run_t *next_runs,  uint8 *next_count,
    pvc_blob_t *blob_pool, uint8 max_blobs,
    uint8 *next_blob_idx,
    uint8 y,
    const uint8 *gray_row);
```

**阶段 F 增量累加的关键细节（这是提速的核心）**:

```c
/* ── 阶段F: 对每个游程增量更新所属 blob ── */
for (uint8 i = 0; i < curr_count; i++) {
    uint8 bidx = curr_runs[i].blob_idx;
    if (bidx == 0xFF) continue;

    pvc_blob_t *b = &blob_pool[bidx];
    uint8 run_len = curr_runs[i].x1 - curr_runs[i].x0 + 1u;
    uint8 x0 = curr_runs[i].x0;
    uint8 x1 = curr_runs[i].x1;

    /* —— 整数累加 (无浮点) —— */
    b->area  += run_len;
    b->sum_x += (uint32)(x0 + x1) * run_len / 2u;
    b->sum_y += (uint32)y * run_len;

    /* —— 包围盒更新 —— */
    if (x0 < b->min_x) b->min_x = x0;
    if (x1 > b->max_x) b->max_x = x1;
    b->max_y = y;  /* 单调递增, 无需比较 */

    /* —— 灰度累加 (run 内部循环, 但 run 通常很短) —— */
    for (uint8 x = x0; x <= x1; x++) {
        b->sum_gray += gray_row[x];
    }

    /* —— 触边检测 (短路求值) —— */
    if (!b->touches_border) {
        if (x0 == 0 || x1 == (PVC_IMAGE_W - 1u)
            || y == 0 || y == (PVC_IMAGE_H - 1u)) {
            b->touches_border = 1;
        }
    }

    /* —— 底部行数据环形记录 (替代二次扫描) —— */
    {
        uint8 idx = b->last_rows & 0x0Fu;  /* mod 16 环形 */
        b->bottom_rows[idx].y    = y;
        b->bottom_rows[idx].xmin = x0;
        b->bottom_rows[idx].xmax = x1;
        b->last_rows++;
    }

    b->row_count++;
}
```

### 6.3 `pvc_swap_buffers()` — 三缓冲旋转

```c
/**
 * @brief 三缓冲指针旋转 (无 memcpy)

 * prev ← curr, curr ← next, next ← old_prev
 */
static void pvc_swap_buffers(
    pvc_run_t **curr_buf, uint8 *curr_count,
    pvc_run_t **prev_buf, uint8 *prev_count,
    pvc_run_t **next_buf, uint8 *next_count);
```

### 6.4 `pvc_extract_level()` — 单阈值完整提取

```c
/**
 * @brief 单阈值层级完整游程连通域提取

 * 替代 pvc_collect_components() + pvc_flood_component() 的全部逻辑。
 * 一次顺序遍历 94×60 全图, 产出 blob_pool 中的所有 blob 完整信息。

 * @param gray        灰度图 [60][94]
 * @param threshold   白色阈值
 * @param blob_pool   输出的 blob 累加器池
 * @param max_blobs   池容量
 * @return            通过面积/宽高筛选的有效 blob 数量
 */
static uint8 pvc_extract_level(
    const uint8 gray[PVC_IMAGE_H][PVC_IMAGE_W],
    uint8 threshold,
    pvc_blob_t *blob_pool,
    uint8 max_blobs);
```

实现逻辑:

```
1. memset(blob_pool, 0, max_blobs * sizeof(pvc_blob_t))
2. 初始化 min_x=255, min_y=255, max_x=0, max_y=0  (所有 blob)
3. 三缓冲指针指向 g_runs_buf0/1/2
4. FOR y = 0..59:
     curr_count = pvc_scan_row(gray[y], threshold, curr_buf, 60)
     pvc_merge_rows(curr_buf, curr_count, prev_buf, &prev_count,
                    next_buf, &next_count, blob_pool, max_blobs,
                    &next_blob_idx, y, gray[y])
     pvc_swap_buffers(&curr, &curr_count, &prev, &prev_count,
                      &next, &next_count)
     prev_count = next_count; next_count = 0
5. 筛选: 面积<120 或 宽<12 或 高<4 → 淘汰
6. 压缩有效 blob 到数组前部
7. return out_count
```

### 6.5 `pvc_blob_finalize()` — 遍历后统一计算

```c
/**
 * @brief 对 blob_pool 中每个有效 blob 计算遍历期未算的派生变量

 * 在 pvc_extract_level() 之后调用一次。
 */
static void pvc_blob_finalize(pvc_blob_t *pool, uint8 count);
```

只做一件事：计算 `bbox_area`（之前只在遍历期维护了 min/max，`(max_x-min_x+1)*(max_y-min_y+1)` 需要后算，避免每次 run 更新都乘一次）。

### 6.6 `pvc_score_blob()` — 适配新数据结构的打分

```c
/**
 * @brief 从 pvc_blob_t 计算 PVC 相似度分数
 *
 * 复用原 pvc_score_component() 的公式, 但输入从 pvc_component_t 改为直接传参
 */
static float pvc_score_blob(const pvc_blob_t *blob,
                             float fill_ratio,
                             float mean_gray);
```

公式完全不变：
```
score = 0.38*min(area/600, 1) + 0.20*min(width/45, 1)
      + 0.16*min(height/18, 1) + 0.16*min(fill/0.55, 1)
      + 0.10*min(max((mean_gray-235)/20, 0), 1)
```

### 6.7 `pvc_extract_target_x_from_blob()` — 从环形缓冲计算目标 X

```c
/**
 * @brief 从 blob 遍历期记录的底部行数据计算加权目标 X 坐标

 * 替代原 pvc_extract_target_x_from_bottom_rows()。
 * 数据来源从"重新扫描 gray 图像"变为"从 blob->bottom_rows 环形缓冲读取"。
 *
 * @param blob  目标 blob (含 bottom_rows 数据)
 * @return      加权目标 X 坐标
 */
static float pvc_extract_target_x_from_blob(const pvc_blob_t *blob);
```

实现逻辑（与原函数等效）：

```
取 bottom_rows 中最近 min(12, last_rows) 条记录
按 y 排序(环形缓冲天然近似有序), 越靠下的行权重越大
每行: row_center = (xmin + xmax) / 2.0f
     weight = 1.0f + 0.08f * (y - y_start)
加权平均 → target_x
```

---

## 七、数据结构变更详情

### 7.1 删除的数据结构

```c
// 【删除】pvc_component_t — 替换为 pvc_blob_t
typedef struct {
    uint16 area;
    uint8  xmin, ymin, xmax, ymax;
    float  centroid_x, centroid_y;
    float  fill_ratio;
    uint8  touches_border;
    float  mean_gray;
    float  score;
} pvc_component_t;

// 【删除】pvc_scratch_t — 替换为独立全局数组
typedef struct {
    uint8  visited[5640];
    uint16 stack[5640];
    pvc_component_t components[32];
    pvc_component_t candidates[32];
} pvc_scratch_t;
```

### 7.2 新增的数据结构

```c
/* ═══════════════════════════════════════════════════════════════
 * 游程段 (对齐参考工程 cc_run_t, 4 bytes)
 * ═══════════════════════════════════════════════════════════════ */
typedef struct {
    uint8 x0;         /* 游程起始 X (含) */
    uint8 x1;         /* 游程结束 X (含) */
    uint8 blob_idx;   /* 所属 blob 索引 (0xFF = 孤儿/未分配) */
    uint8 skip_cnt;   /* 跳行计数 (连续全背景行) */
} pvc_run_t;

/* ═══════════════════════════════════════════════════════════════
 * 底部行数据记录 (用于 pvc_extract_target_x_from_blob)
 * ═══════════════════════════════════════════════════════════════ */
typedef struct {
    uint8 y;
    uint8 xmin;
    uint8 xmax;
} pvc_bottom_row_t;

/* ═══════════════════════════════════════════════════════════════
 * Blob 累加器 — 遍历期增量累加, 一次产出所有变量
 *
 * 所有字段在 pvc_merge_rows() 的阶段 F 中增量更新。
 * 无 float 字段 — 派生变量 (centroid, fill_ratio, mean_gray)
 * 在遍历结束后统一计算。
 * ═══════════════════════════════════════════════════════════════ */
typedef struct {
    /* —— 基础几何 (遍历期累加, 全部整数) —— */
    uint32 area;            /* 总像素数 */
    uint32 sum_x;           /* Σx, centroid_x = sum_x / area */
    uint32 sum_y;           /* Σy, centroid_y = sum_y / area */
    uint32 sum_gray;        /* Σ灰度值, mean_gray = sum_gray / area */
    uint8  min_x, max_x;    /* 包围盒 X */
    uint8  min_y, max_y;    /* 包围盒 Y */
    uint8  row_count;       /* 有效行数 */
    uint8  touches_border;  /* 是否触边 (0/1) */
    uint16 bbox_area;       /* 包围盒面积 (遍历后由 pvc_blob_finalize 填充) */
    float  score;           /* 最终得分 (遍历后填充) */

    /* —— 底部行数据 (替代 pvc_extract_target_x_from_bottom_rows 的二次扫描) —— */
    uint8  last_rows;                         /* 已记录行数 */
    pvc_bottom_row_t bottom_rows[16];         /* 环形缓冲, 最多 16 行 */
    uint8  is_valid;                          /* 槽位有效标志 */
} pvc_blob_t;
/* sizeof ≈ 80 bytes, 32 个 ≈ 2560 bytes */
```

### 7.3 全局变量变更

```c
/* ── 删除 ── */
// static pvc_scratch_t g_pvc_scratch;  ← 约 19.3KB

/* ── 新增 ── */
static pvc_run_t  g_runs_buf0[60];    /* 当前行游程, 60×4 = 240 bytes */
static pvc_run_t  g_runs_buf1[60];    /* 上一行游程, 240 bytes */
static pvc_run_t  g_runs_buf2[60];    /* 临时缓冲, 240 bytes */
static pvc_blob_t g_blob_pool[32];    /* blob 累加器池, 32×80 ≈ 2560 bytes */
static pvc_blob_t g_candidates[32];   /* Top-1 用不到 32 个, 保留以匹配
                                         PVC_VISION_MAX_COMPONENTS 语义 */
/* 总计: 720 + 2560 + 2560 = 5840 bytes ≈ 5.7 KB */
```

---

## 八、内存布局对比

| 项目 | 原 Flood Fill | 新 RLE 游程 | 变化 |
|------|--------------|-------------|------|
| `visited[5640]` | 5640 B | 0 (删除) | **-5640** |
| `stack[11280]` | 11280 B | 0 (删除) | **-11280** |
| `components[32]` | ~1408 B | 0 (删除, 改为 blob_pool) | — |
| `candidates[32]` | ~1408 B | ~2560 B (保留) | +1152 |
| `runs_buf0[60]` | 0 | 240 B | +240 |
| `runs_buf1[60]` | 0 | 240 B | +240 |
| `runs_buf2[60]` | 0 | 240 B | +240 |
| `blob_pool[32]` | 0 | ~2560 B | +2560 |
| **总计** | **~19736 B** | **~5840 B** | **节省 ~13.9 KB** |

> **关键**: 省掉了 5640 字节的 `visited` 和 11280 字节的 `stack`，这两个是 Flood Fill 的刚性开销。

---

## 九、外部接口兼容性保证

### 9.1 不改动的头文件 (`pvc_vision.h`)

所有宏、类型、extern 声明、函数声明全部保持不变：

```c
// 以下全部不动
#define PVC_IMAGE_W                     (94U)
#define PVC_IMAGE_H                     (60U)
#define PVC_IMAGE_SIZE                  (PVC_IMAGE_W * PVC_IMAGE_H)
#define PVC_VISION_ENABLE               (1)
#define PVC_VISION_WHITE_THRESHOLD      (200U)
#define PVC_VISION_MIN_AREA             (120)
#define PVC_VISION_MIN_WIDTH            (12)
#define PVC_VISION_MIN_HEIGHT           (4)
#define PVC_VISION_MIN_FILL_RATIO       (0.25f)
#define PVC_VISION_MIN_DECISION_SCORE   (0.58f)
#define PVC_VISION_MAX_COMPONENTS       (32)
#define PVC_VISION_CONFIRM_FRAMES       (3U)
#define PVC_VISION_LOST_HOLD_FRAMES     (2U)
// ... 所有 define 不动

typedef struct { /* pvc_vision_frame_result_t */ }  // 不动
typedef struct { /* pvc_vision_output_t */ }         // 不动

extern volatile pvc_vision_output_t g_pvc_vision_output;  // 不动

void pvc_vision_init(void);                   // 不动
void pvc_vision_reset_filter(void);            // 不动
const volatile pvc_vision_output_t *pvc_vision_get_output(void); // 不动
void pvc_vision_process_camera_frame(const uint8 *gray);         // 不动
```

### 9.2 不改动的调用方

| 调用方 | 调用的接口 | 影响 |
|--------|-----------|------|
| `user/main_cm7_1.c` | `pvc_vision_init()`, `pvc_vision_reset_filter()`, `pvc_vision_process_camera_frame()` | 零影响 |
| `vision_ipc_core1.c` | `pvc_vision_get_output()`, `g_pvc_vision_output` | 零影响 |
| `wifi.c` | `g_pvc_vision_output` (渲染用) | 零影响 |
| `vision_pvc_control.c` (Core 0) | IPC 数据包中的 PVC 字段 | 零影响 |
| `vision_three_stage_control.c` (Core 0) | IPC 数据包中的 PVC 字段 | 零影响 |

### 9.3 输出数据一致性保证

`pvc_vision_frame_result_t` 的所有字段必须被填充为**完全相同的语义和数值范围**：

| 字段 | 原来来源 | 新来源 | 一致性 |
|------|---------|--------|--------|
| `detected` | `best->score >= 0.58` | 同 | ✅ |
| `component_count` | `pvc_collect_components()` 返回值 | `pvc_extract_level()` 返回值 | ✅ |
| `candidate_count` | `pvc_filter_candidates()` 返回值 | 遍历中维护的 candidate_count | ✅ |
| `area` | `best->area` | `blob->area` (uint16 cast) | ✅ |
| `bbox_xmin/ymin/xmax/ymax` | `best->xmin...` | `blob->min_x...` | ✅ |
| `entry_bottom_y` | `best->ymax` | `blob->max_y` | ✅ |
| `entry_top_y` | `best->ymin` | `blob->min_y` | ✅ |
| `confidence` | `best->score` | `blob->score` | ✅ |
| `centroid_x` | `(float)sum_x/area` | 同公式 | ✅ |
| `centroid_y` | `(float)sum_y/area` | 同公式 | ✅ |
| `fill_ratio` | `area/bbox_area` | 同公式 | ✅ |
| `mean_gray` | `sum_gray/area` | 同公式 | ✅ |
| `target_x_px_x100` | `pvc_extract_target_x_from_bottom_rows()` | `pvc_extract_target_x_from_blob()` | ⚠️ 需验证 |
| `steer_error_px_x100` | `target_x - 图像中心` | 同公式 | ✅ |
| `forward_mm` | `(59-row)*20` | 同 | ✅ |
| `lateral_mm` | `(x-46.5)*8` | 同 | ✅ |
| `phy_x_mm/phy_y_mm` | IPM 查表 | 同 | ✅ |

> ⚠️ `target_x_px_x100` 是新旧方案最可能产生微小差异的字段。原方案在 Flood Fill 完成后**重新扫描** best 的 bbox 内底部 12 行的每个像素来判断 `gray[x] >= 200`；新方案用遍历期的 `bottom_rows` 环形缓冲记录的逐行 `xmin/xmax`。两者在 blob 边界像素的判定上可能因 Flood Fill 的邻域传播而产生 1~2 像素偏差。**需要在实车数据上做 A/B 对比验证。**

---

## 十、分阶段实施步骤

### 阶段 1: 新建内部类型 + 工具函数 (只加不改, 可编译)

1. 在 `pvc_vision.c` 中新增 `pvc_run_t`、`pvc_bottom_row_t`、`pvc_blob_t` 定义
2. 新增全局缓冲: `g_runs_buf0/1/2[60]`, `g_blob_pool[32]`, `g_candidates[32]`
3. 删除 `pvc_scratch_t` 和 `g_pvc_scratch`
4. 实现 `pvc_scan_row()` — 最简函数, 先验证正确性
5. 实现 `pvc_swap_buffers()` — 3 行指针交换
6. 编译通过

### 阶段 2: 实现核心合并函数 (最关键的 200 行)

1. 实现 `pvc_merge_rows()` 
   - 阶段 B: 全背景行处理
   - 阶段 C: 双指针重叠检测
   - 阶段 D: 孤儿分配新 blob
   - 阶段 E: 行内缝合
   - 阶段 F: 增量累加 (area, sum_x/y, sum_gray, bbox, touches_border, bottom_rows)
   - 阶段 G: 行交接
2. 实现 `pvc_extract_level()` — 循环调用 scan_row + merge_rows + swap
3. 编译通过 (此时还未接入主流程)

### 阶段 3: 实现后处理函数

1. 实现 `pvc_blob_finalize()`
2. 实现 `pvc_score_blob()` — 复用原公式
3. 实现 `pvc_extract_target_x_from_blob()` — 从环形缓冲计算

### 阶段 4: 重写 `pvc_detect_frame()` (替换主流程)

1. 替换 `pvc_detect_frame()` 主体逻辑
2. 删除 `pvc_flood_component()`, `pvc_collect_components()`, `pvc_filter_candidates()`
3. 删除 `pvc_sort_by_area()`, `pvc_sort_by_score()`
4. 删除 `pvc_component_width()`, `pvc_component_height()`
5. 内联 `pvc_copy_best_to_result()` 逻辑
6. 编译通过

### 阶段 5: 调整 `pvc_fill_physical_coord_from_ipm()` 参数

1. 改为接受独立的 `centroid_x, ymax` 参数
2. 或者直接在 `pvc_detect_frame()` 中内联实现

### 阶段 6: 实车 A/B 对比验证

1. 用同一批录制的 raw 灰度帧序列, 分别跑新旧两版算法
2. 对比输出: `detected`, `confidence`, `bbox`, `target_x_px_x100`, `forward_mm`
3. 确认差异在可接受范围 (1~2 像素)
4. 对比耗时: `g_pvc_vision_cost_profiler.last_us`

### 阶段 7: 清理 + 注释

1. 删除 `pvc_component_t` 定义
2. 更新文件头注释
3. 添加调参注释

---

## 十一、验证策略

### 11.1 离线验证 (PC 端优先)

利用现有的 PC C 原型 (`tools/07_.../bridge/c_pvc_detector/`):

1. 在 `pvc_detector.c` 中实现 RLE 版 `pvc_detect_frame_gray_rle()`
2. 修改 `pvc_video_cli.c` 增加 `--mode rle` 选项
3. 用同一批 PGM 帧跑新旧两版, 对比 JSON 输出
4. 确认: `detected` 一致率 > 99%, `bbox` 偏差 ≤ 2px

### 11.2 嵌入式验证

1. 在 `pvc_vision.c` 中 `#ifdef PVC_VISION_RLE_MODE` 条件编译
2. 保留旧代码, 新代码用宏开关
3. 实车录制场景: 远距离、中距离、近距离、强反光、暗光
4. WiFi 图传观察 `render_pvc_vision_to_image()` 的渲染框是否一致
5. 用 `runtime_profiler` 对比耗时

### 11.3 验收标准

| 指标 | 标准 |
|------|------|
| `detected` 一致率 | ≥ 99% (同一批帧) |
| `bbox` 偏差 | ≤ 2 像素 |
| `target_x` 偏差 | ≤ 1.5 像素 |
| 单帧耗时 | 比原方案降低 ≥ 30% |
| P99 耗时 (强反光场景) | 比原方案降低 ≥ 50% |
| 内存占用 | ≤ 6 KB (原 19.3 KB) |

---

## 十二、风险评估与缓解

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|----------|
| 游程合并逻辑 bug 导致 blob 分裂 | 中 | 高: 一个大白块被识别为多个小块, score 降低 | 阶段 2 先在 PC 端单步调试, 对比原版 |
| `bottom_rows` 环形缓冲数据不一致 | 低 | 中: target_x 偏差 | 阶段 6 A/B 对比 |
| 三缓冲旋转 bug 导致 run 丢失 | 低 | 高: blob 面积变小 | 参考工程已验证, 直接复用逻辑 |
| 跳行继承 (全背景行) 逻辑遗漏 | 中 | 中: 白色区域有断行时 blob 断开 | PVC 入口通常是连续白色, 影响小; 保留 SKIP_LINES_MAX=3 |
| 行内缝合间隙参数不当 | 低 | 低: blob 分裂 | 参考工程 GAP_MAX=15, PVC 用 10 |
| 新代码引入的除零风险 | 低 | 高: HardFault | 所有 `area > 0` 检查 |

---

## 附录 A: 参考工程关键参数映射

| 参考工程 (cc_extract) | 值 | PVC 建议值 | 说明 |
|----------------------|-----|-----------|------|
| `CC_MAX_RUNS_PER_ROW` | 60 (188宽) | 30 (94宽) | 94 宽最多 47 个交替游程, 30 足够 |
| `CC_MAX_BLOB_POOL` | 40 | 32 | 复用 `PVC_VISION_MAX_COMPONENTS` |
| `CC_SKIP_LINES_MAX` | 3 | 3 | 跳行继承上限 |
| `CC_MERGE_GAP_MAX` | 15 | 10 | PVC 白色更连续, 间隙更小 |
| `CC_PACKED_W` | 24 (188→24B) | 不需要 | PVC 不打包 bitmask |

## 附录 B: 改动文件清单

| 文件 | 改动行数 (估算) | 说明 |
|------|----------------|------|
| `code1/vision/pvc_vision.c` | -300 +350 行 | 删除 Flood Fill 全套, 新增游程全套 |
| `code1/vision/pvc_vision.h` | 0 | 不改 |

**总计**: 1 个文件, 净增约 50 行, 删除约 300 行旧代码, 新增约 350 行新代码。
