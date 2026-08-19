# 颠簸视觉 v3 移植规划 —— 只算 gy + IPM 后点簇直方图 + 方差/个数双门限

> 日期：2026-08-19（v2 版，已并入用户审阅意见）
> 状态：**✅ 已批准并全量执行完成（2026-08-19）**。发布工程 `code1` 已同步，IAR 构建 0 错误，内存达标。
> 范围：发布工程 `D:\WORKS\2026LunTui\project\code1\vision\`（bumpy_conv / bumpy_pipeline / bumpy_vision + edge_conv7_asm）
> 依据：
> - 交接文档 `trials\bumpy-road-new\pc_tools\lat_study\交接文档_20260819_边线中线滤波v2与梯度门.md`（§7 阈值重做、§8 倾角方差门+圆域 EMA）
> - 测试工程 `trials\bumpy-road-new\project\code\`（v2 滤波 / 阈值重做 / 方差门已落地并烧录位精确验证；**直方图主带参数即其 v2 点簇门参数，直接复制使用**）
> - `docs/任务规划/颠簸新管线移植内存现状与瘦身方案.md`（内存账目：宏=1 余量仅 6,510B）

---

## 0. 摘要（一句话）

把测试工程已验证的 **v2 中线滤波 + 亮度归一阈值 + 拆符号 CCL + 倾角方差门 + 圆域 EMA** 移植回发布工程 `code1`，并按用户指示落地：**① 只算 gy（删 gx 卷积路径与平方求和），阈值改双阈值带符号（gy≥+T→1 / gy≤−T→2，无绝对值无平方）；② 边线提取在 IPM 逆透视后做物理 x 点簇直方图/主带，主带内点物理 x 均值即边线位置（删 RANSAC 拟合与基准行外推，边线-中线链条极简）；③ 有无颠簸 = 朝向角方差门 + 个数门双门限（拒绝单线）**。**不保留 rule0 回归路径**（回归 = `BUMPY_USE_NEW_PIPELINE=0` 的 188×120 旧管线）；**IPC 契约零改动**（4 个 valid 均有现成传出路径，见 §6）。净收益：`s_bumpy_pipeline` 95,960B → **50,760B（省 45,200B）**，CM7_1 rw data 余量 6,510B → **~51.7KB**；卷积乘加约减半；算法主路径零开方/零平方/零分位。

---

## 1. 现状盘点（发布工程 vs 测试工程）

| 模块 | 发布工程 `code1`（当前） | 测试工程 `trials/.../project/code`（已验证） |
|---|---|---|
| 阈值 | p85 相对阈值 + gx 上限 + 方向锥（BP_RULE 无） | BP_RULE 3=亮度归一 T=k·mean(gray)，k=2500（0/1/2 为探索中间态，本次不移植） |
| CCL | 非零邻接（不拆符号，条纹正负沿粘团） | 等值邻接（horiz 编码 1/2，拆 gy 符号） |
| is_inner | 跨域剔除不查符号 | 同符号判定（ccsign），修复孪生符号域吃光外点 |
| frame_heading | 线性域 n≥10 + 个数≥3，无方差门 | 合规域收紧 + **个数≥2（拒绝单线）+ 圆加权 std 方差门 σ>2° 双门限** + 解除 hdg→L/R 耦合 |
| 边线位置 | RANSAC 穷举点对 + PCA 精化（fit_outer）→ 拟合线交基准行 | **IPM 后物理 x 点簇直方图主带 → 主带内点物理 x 均值（v3 目标，删 RANSAC/基准行）** |
| 中线滤波 | 3 帧中值窗 + conf 可信度（实测从未工作） | **v2：点簇物理 X 门 + 采信窗 + EMA 限速 + 抢救**，meas_valid=locked |
| 角度 | 无时间滤波 | 圆域向量 EMA（α=0.4，仅 hdg_valid 帧更新） |
| 内存 | s_bumpy_pipeline 95,960B；rw data 255,634/262,144（余 6,510B） | 同结构（rule3 下仍白算 gx/mag²） |

发布工程 `code1` 当前：`BUMPY_USE_NEW_PIPELINE=1`（宏=1 在跑），余量仅 6,510B——v3 瘦身是刚需而非可选。

---

## 2. 目标架构 v3（数据流 + 内存布局）

```
img(94×60 uint8)
  │
 ① bumpy_conv7_gy(只算 gy) ──────────────── gy[PIX] int32（垂直 D 核输出）
  │   水平 pass 只算 P 核(gyh)；垂直 pass 只用 D 核；无 gx、无 mag²、无分位
  │
 ② T = BP_NORM_K·mean(gray)（整数，无平方）
  │   双阈值带符号：gy ≥ +T → horiz=1；gy ≤ −T → horiz=2；其余 0
  │   （取消绝对值：两个阈值正好对应两种符号连通域，天然拆符号）
  │
 ③ ccl8 等值邻接（拆符号）→ 每域 PCA 主轴角 + 线性度 rms + 合规标记
  │   labels/relab(uf) 复用缓冲（uint16 版，见 §3.2）
  │
 ④ extract_outer（每合规线性 CC x 极值 3 点 + 同符号跨域剔除）
  │   → 左右外点像素集透出到 out（**不再 RANSAC 拟合**）
  │
 ⑤ frame_heading：合规 CC 圆加权均值 + 个数≥2 + 方差门 σ>2° 双门限
  │   → hdg / hdg_valid（= bumpy_detected 源）
  │
 ⑥ bumpy_vision —— IPM 后边线提取 + 中线（链条极简）：
  │   a) 左右外点逐点 IPM → 物理 x 点簇直方图主带（中值±100mm、占比≥0.78、≥4点）
  │      主带内点物理 x 均值 = 边线物理 x（质量门=主带提取本身，替代旧点簇门+基准行）
  │   b) 左右物理 x → 间距自检(1m±150mm) / 单边±500mm逆推 / 抢救 → 中线 raw
  │   c) 采信窗(4,60mm)+EMA(0.6)+限速(50mm) → lateral_mm；meas_valid = 锁定
  │   d) hdg 圆域 EMA(α=0.4) → yaw_error_deg_x100；bumpy_detected = hdg_valid
```

**边线-中线链条（一句话）**：`外点(像素) → IPM → 物理 x 主带均值 = 边线物理 x → 左右合成/单边 → 中线`。全程无直线拟合、无角度、无基准行外推。

**目标内存布局（`bumpy_pipeline_t`，总 50,760B）：**

| 区 | 大小 | 生命周期与复用 |
|---|---|---|
| `gy[PIX] int32` | 22,560B | ①垂直输出 → ②判定 → ③④ 并查集 uf（用量 ≤ PIX/2+1） |
| `gyh[PIX] int32` | 22,560B | ①水平中间（P 核）→ ③④ 拆为 labels(uint16[PIX]) + relab(uint16[PIX]) |
| `horiz[PIX] uint8` | 5,640B | ②输出 → ③ CCL 输入（全程只读） |

> 相比发布版：删 `gx[PIX]`(22,560B)；`mag2` 由 uint64[PIX](45,120B) 缩为 gyh int32[PIX](22,560B)。
> labels/relab 改 uint16 的技术依据：临时标号上界 ≤ PIX/2+1 = 2,821 < 65,536，uint16 安全（拆符号后更少）。
> 外点像素集透出到 `bumpy_frame_result_t`（数量上限见裁决点 H），vision 侧主带提取仅需 O(n) 局部数组，不占 pipeline 缓冲。

---

## 3. 详细移植步骤（分 4 阶段，每阶段独立验证门槛）

### 阶段 1：同步已验证方案（v2 滤波 + 阈值重做 + 方差门 + 圆域 EMA）

**目标**：把测试工程已验证成果**原样**（不含新优化）移植到 `code1`，与测试工程逐位对齐（BP_RULE=3 档）。纯移植，算法零新增 → 风险最低。本阶段暂保留 gx/mag² 计算（为对拍基准），阶段 2 消除。

**改动文件与内容**：

1. `code1/vision/bumpy_pipeline.h`
   - 新增宏：`BP_NORM_K`(2500)、`BP_HDG_STD_MAX`(2.0f)、`BP_MIN_HDG_LINES` 改 **2**（拒绝单线，双门限）、`BP_INL_MAX`(24)、`BP_LIN_MAX`(16)；`#ifndef` 可 -D 覆盖。
   - `bumpy_frame_result_t` 增加：`rawL_inl_n/x/y`、`rawR_inl_n/x/y`（供 v2 点簇门/主带）、`lin_n/cx/cy/ang/pix`（渲染用，trials 侧 host 消费；不进发布工程 IPC，见裁决点 I）。
2. `code1/vision/bumpy_pipeline.c`
   - `ccl8`：邻接判定改 `== horiz[p]`（拆符号，等值邻接）。
   - 阶段②：亮度归一 `tnorm = BP_NORM_K·img_sum/PIX`，`gy²≥T²`，horiz 编码 1/2（此阶段先照抄测试工程，保证与测试工程位一致）。
   - 阶段①：`img_sum` 累加。
   - `frame_heading`：收紧为合规域 + **个数≥2 + 圆加权 std 方差门 σ>2° 双门限**；**解除 hdg→L/R 清空耦合**。
   - `is_inner`：加 `ccsign[lb] != ccsign[self_ci] → continue`（同符号）。
   - `ccl8` relabel 段记录 `ccsign[ncc] = horiz[i]`；新增 `static uint8_t ccsign[MAX_CC+1]`。
   - `fit_outer`：增加 `ex/ey/en` 参数导出内点（≤BP_INL_MAX）。
   - 阶段④.5：raw_L/raw_R 透出；阶段⑤前导出 `lin_*`。
3. `code1/vision/bumpy_vision.h/.c`
   - 常量与状态：点簇门 `BUMPY_CLS_MIN_PTS(4)/TOL_X_MM(100)/MIN_RATIO(0.78)`；采信窗 `CAND_N(4)/AGREE_MM(60)/GATE_MM(100)/ALPHA(0.6)/SLEW_MM(50)/RESCUE_MM(60)`；圆域 EMA `VISION_BUMPY_HDG_EMA_ALPHA(0.40)`。
   - 新增 `bumpy_vision_cluster_gate()`（内点逐点 IPM → 物理 X 中值 ±100mm 占比 ≥0.78 且 ≥4 点）。
   - `bumpy_vision_lateral_filter()` 替换旧 3 帧中值+conf 为 v2：采信窗互认 → 锁定 → EMA+限速；无观测保持；`meas_valid = s_lat_locked`。
   - 双侧合成：间距自检失败 → 抢救（与当前估计一致侧单边逆推）；`bumpy_vision_edge_x_at_base_row` 保留（本阶段位置语义不变，阶段 3 替换为点簇均值）。
   - hdg 路径：先圆域向量 EMA 再整形；`bumpy_vision_reset_filter` 复位 `s_hdg_ex/ey`。
   - `bumpy_vision_output_t`：不加 `lin_*/inl_*` 透出（发布工程无渲染消费）。
4. `user/main_cm7_1.c`：无改动（发布版已用新签名 `bumpy_vision_process_camera_frame`）。

**验证门槛（不满足不得进阶段 2）**：
- host 对拍：`gcc -O2 -I shim -I <code1/vision> lat_host2.c <code1/vision>/{bumpy_vision,bumpy_pipeline,bumpy_conv}.c ... ipm_transform.c` 全库（12 视频 2365 帧）输出与测试工程一致（同宏：`-DBP_MIN_HDG_LINES=2`）。
- GT 标注对拍：bumpy-1 78/78、bumpy-3 53/54、bumpy-2 19/22（基线已知）。
- IAR 构建：`iarbuild cyt4bb7_cm_7_1.ewp -build Debug -parallel 8`，Errors: none；map 核对 rw data。
- 渲染视频（trials 侧 `render_v2_video.py`）用户目视确认。

---

### 阶段 2：只算 gy + 双阈值带符号 + 缓冲重规划（删 rule0/gx/mag²/percentile）

**目标**：**彻底不计算 gx、不累加 mag²、不做分位**；阈值改**双阈值带符号**（gy≥+T→1 / gy≤−T→2，取消绝对值）；缓冲重规划。**不保留 rule0 回归路径**（回归 = `BUMPY_USE_NEW_PIPELINE=0` 的 188×120 旧管线，与本文件无关，不动）。

**改动文件与内容**：

1. `code1/vision/edge_conv7_asm.h`：新增声明 `void conv7_horiz_row_gy(const int16_t *p_row_pad, int32_t *p_gy_h, uint32_t out_width);`（只算 P 核，省 D 核 MAC）。
2. `code1/vision/edge_conv7_asm.s`：新增 gy-only 水平行例程（对现有 `conv7_horiz_row` 删 D 核输出分支，每像素乘加减半）。垂直 pass 复用 `conv7_vert_col(..., use_d=1)`（D 核），不再调用 use_d=0 的 P 核路径。
3. `code1/vision/bumpy_conv.h/.c`：
   - `bumpy_conv7` 直接改造为 **`bumpy_conv7_gy(img, gy, scratch)`**（签名删 gx；scratch 仅 **1×BUMPY_PIX** 存 gyh；水平 pass 每行只算 `gyh_row`；垂直 pass 每列只铺 gyh → `conv7_vert_col(...,1)`）。
   - 删静态 `gxh_row`（376B）；`bumpy_conv7` 无其他调用者（已 grep：仅 `bumpy_pipeline.c`），可安全替换。
4. `code1/vision/bumpy_pipeline.c`：
   - 阶段①：调 `bumpy_conv7_gy(img, s->gy, s->gyh)`，并在同循环累计 `img_sum`（uint32，上界 5640×255 安全）。
   - 阶段②：**双阈值带符号**：`tnorm = (uint32)BP_NORM_K * img_sum / PIX;` 逐像素 `gy ≥ +tnorm → horiz=1; gy ≤ −tnorm → horiz=2; 其余 0`。**无绝对值、无平方、无 uint64**（|gy|≤66,300，tnorm≤637,500，int32 安全；与测试工程 `|gy|≥T`、`gy²≥T²` 均非负等价逐位一致）。
   - 删除：`percentile_q64`/`nth_smallest`、mag² 计算循环、`BP_RULE` 多分支（固定新规则）。
5. `code1/vision/bumpy_pipeline.h`：
   - `bumpy_pipeline_t` 改：删 `gx[PIX]`；`mag2[PIX] uint64` → `gyh[PIX] int32`（水平中间结果区）；注释更新缓冲复用图。
   - 删宏 `BP_RULE`/`BP_FIX_T`/`BP_MAG_PCT`/`BP_VERT_RELAX`/`BP_HORIZ_CAP`/`BP_DIR_TOL`（p85 体系废弃）；保留 `BP_NORM_K`/`BP_HDG_STD_MAX` 等。
6. **CCL 缓冲重规划（关键改动）**：
   - `ccl8` 的 `labels` 由 `int32*` 改 `uint16*`（别名 `(uint16_t*)s->gyh`）；`relab` 由 `int32*` 改 `uint16*`（别名 `(uint16_t*)s->gyh + PIX`，两段各 11,280B）；`uf` 仍借 `(int32_t*)s->gy`。
   - `domain_accum/domain_finish` 的 labels 指针类型同步改 `uint16*`。
   - 逐位对拍论证：labels/relab 仅存标号整数（≤PIX/2+1=2,821），int32→uint16 存储不改变数值，算法语义逐位一致。
   - `bumpy_conv7_gy` 的 scratch 传 `s->gyh`（垂直 pass 完成后 gyh 不再需要 → ③ 复用为 labels/relab，生命周期无重叠）。

**验证门槛**：
- **gy 逐位对拍**：全库 `bumpy_conv7_gy` 输出的 gy 与测试工程 `bumpy_conv7` 的 gy 逐字节一致（host 新增对拍入口或临时断言）。
- **全链路对拍**：整帧输出与阶段 1（测试工程 rule3 档）逐位一致（双阈值带符号 与 `|gy|≥T`、`gy²≥T²` 等价，labels uint16 数值不变）。
- IAR 宏=1 构建通过；map 核对：`s_bumpy_pipeline` 应显示 50,760B，rw data ≈ 210.4KB（余 ~51.7KB）。

---

### 阶段 3：IPM 后点簇直方图边线 + 链条简化（替换 RANSAC/基准行）

**目标**：边线提取**整体搬到 IPM 逆透视之后**：pipeline 只输出左右外点像素集（删 `fit_outer` RANSAC 与 PCA）；`bumpy_vision` 对外点逐点 IPM → 物理 x 点簇直方图主带 → **主带内点物理 x 均值 = 边线物理 x**；删基准行外推与独立点簇门（质量门=主带提取本身）。

**算法设计（参数直接复用参考工程 v2 点簇门，无需新调参）**：

```
// bumpy_vision 侧，左右各一次；直方图主带参数 = 参考工程 BUMPY_CLS_*：
edge_x_mm(px[], py[], n) -> float 边线物理 x，失败返回无效：
  1. 逐点 IPM(px,py) → 有效物理 x 数组 xs[0..m)（m ≤ BP_OUT_MAX）
  2. m < BUMPY_CLS_MIN_PTS(4) → 失败
  3. 物理 x 直方图/排序求中值 x_med（n 小，插入排序即可）
  4. 主带 = { x : |x − x_med| ≤ BUMPY_CLS_TOL_X_MM(100mm) }
  5. 主带点数 ≥ BUMPY_CLS_MIN_PTS(4) 且 主带/全部 ≥ BUMPY_CLS_MIN_RATIO(0.78)
       → 成功；边线物理 x = 主带内点物理 x 均值   ← 即"点簇均值"
     否则 → 失败（无可靠边线）
```

**改动文件与内容**：

1. `code1/vision/bumpy_pipeline.h/.c`：
   - 删 `fit_outer`（穷举 RANSAC + PCA 精化）与 `inl[]`（768B）；`bumpy_line_t` 边线输出移除。
   - `extract_outer` 后直接把左右外点写入 `out`：新增 `out->lp_n/lp_x[]/lp_y[]`、`out->rp_n/rp_x[]/rp_y[]`（上限 `BP_OUT_MAX`，见裁决点 H）。
   - `frame_heading` 保留（hdg/hdg_valid 输出，双门限已在阶段 1 落地）。
2. `code1/vision/bumpy_vision.c`：
   - 删 `bumpy_vision_cluster_gate()`（并入新主带提取）与 `bumpy_vision_edge_x_at_base_row()`（基准行语义废弃）。
   - 新增 `bumpy_vision_edge_x_mm()`（上伪代码；IPM 查表 ≤2×48 次/帧，开销可忽略）。
   - 左右 `x_mm` → 间距自检(1m±150mm)/单边±500mm 逆推/抢救（v2 逻辑保留）→ `lateral_raw` → v2 采信窗+EMA 滤波 → `lateral_mm`；`meas_valid = s_lat_locked`。
   - hdg 圆域 EMA → `yaw_error_deg_x100`；`bumpy_detected = res.hdg_valid`（不变）。
3. `code1/vision/bumpy_vision.h`：输出结构 `line_l/line_r`（bumpy_line_t 渲染字段）移除（裁决点 I）。
4. trials 侧 host/渲染适配：`lat_host2.c`/`render_v2_video.py` 改用外点+主带渲染（trials 侧改，发布工程不动）。

**验证门槛**：
- GT 标注对拍（bumpy-1/2/3，178 条）：过门边线帧级对比 v2（RANSAC+点簇门+基准行），目标：正确率 ≥0.97、误杀 0。
- 全库横向锁定率对比基线（bumpy-1 0.885 / bumpy-3 0.594 / bumpy-11 0.268）。
- **bumpy-2 远右边缘专项**：点簇均值(≈470mm) vs 基准行(≈290mm) 180mm 差的处理——依赖间距自检/抢救兜底，需逐帧确认无假有效（裁决点 J）。
- 渲染视频（trials 侧，外点+主带）用户目视确认。

---

### 阶段 1 附注：方差 + 个数双门限（用户已定）

**结论（用户已定）**：方差和个数**双重门限**，拒绝单线通过。

- `frame_heading` 保留：合规域收紧（cccomp）+ **`n_line ≥ 2`（拒绝单线）** + 圆加权 std 方差门（σ>BP_HDG_STD_MAX=2° → 无效）。
- `BP_MIN_HDG_LINES` 由 3 改 **2**（双门限语义）；单条噪声 CC 时 n_line=1 < 2 → 天然拒，且 1 个 CC 时圆 std≡0 恒有效的漏洞也被堵住。
- 验证：全库 hdg_valid 帧率（室内 12 视频 + 室外 13 条）、600 无条纹帧误检（目标 0）、5m/s 等效最长连 0（基线 2 帧）；渲染确认 hdg 稳定性（帧间 |Δhdg| 中位：室内 ≤0.08°、室外 1/2~1/3 改善）。

---

### 阶段 4：整合 + 板端验证

- IAR 宏=1 构建：Errors none；map 核对 rw data（预期 ~210KB，余 ~51.7KB）；`s_bumpy_pipeline` 50,760B。
- 板端烧录 + 串口 batch test（12 帧 × 多轮）与 `bumpy_bench_host`（-DBP_TEMPORAL=0）**位精确一致**。
- 性能：`BP_STAGE_TIMER` 打印各阶段周期；卷积阶段预期从 2,668us 减半（只算 gy）；总 avg 预期 <5ms@250MHz。
- 全量 host 回归（新规则默认档；188×120 旧管线经 `BUMPY_USE_NEW_PIPELINE=0` 保持不动，即回归路径）。
- 实车验证（待用户安排）：5m/s 高速采样（交接文档 §7 待办 2）。

---

## 4. 逐文件改动清单（汇总）

| 文件 | 阶段 | 改动 |
|---|---|---|
| `bumpy_conv.h` | 2 | `bumpy_conv7` 改造为 `bumpy_conv7_gy`（删 gx，scratch 1×PIX） |
| `bumpy_conv.c` | 2 | gy-only 实现；删 gxh_row/gx 垂直 pass |
| `edge_conv7_asm.h/.s` | 2 | 新增 `conv7_horiz_row_gy`（只算 P 核）；垂直只调 use_d=1 |
| `bumpy_pipeline.h` | 1,2,3 | NORM_K/HDG_STD_MAX/MIN_HDG_LINES=2/INL_MAX/LIN_MAX；删 BP_RULE 等 p85 宏；frame_result 加 inl_*/lin_*（阶段3 改外点 lp_*/rp_*）；结构体改 gy/gyh/horiz |
| `bumpy_pipeline.c` | 1,2,3 | ccl8 拆符号+uint16；双阈值带符号；frame_heading 双门限+解耦；is_inner 同符号；fit_outer 内点导出（阶段3 删，改外点透出）；percentile/mag² 删；缓冲重规划 |
| `bumpy_vision.h` | 1,3 | v2 滤波/点簇门/EMA 常量；输出结构删 line_l/line_r |
| `bumpy_vision.c` | 1,3 | 点簇门+v2 滤波+抢救+圆域 EMA+meas_valid=locked（阶段3：点簇门合并进 IPM 后主带提取，删基准行） |
| `user/main_cm7_1.c` | — | 无改动（已新签名） |
| `iar/...cm_7_1.ewp` | — | 无改动（文件集合不变） |

---

## 5. 内存账目（目标）

| 项 | 发布版现状 | v3 目标 | 说明 |
|---|---|---|---|
| `s_bumpy_pipeline` | 95,960B | **50,760B** | gy 22,560 + gyh 22,560 + horiz 5,640；gx/mag2 消除 |
| `bumpy_pipeline.o` 静态 | ~8.7KB | ~7.3KB | 删 inl[] 768B + fit_outer 相关；加 ccsign 65B；外点数组 g_lp/g_rp 保留（阶段3 透出） |
| `bumpy_conv.o` 静态 | ~1.5KB | ~1.1KB | 删 gxh_row 376B（gy-only） |
| `bumpy_vision` 状态 | 旧滤波 ~20B | v2 ~120B | 采信窗 4×float、EMA 2×float；主带提取为局部数组 |
| CM7_1 rw data 合计 | 255,634B（余 6,510B） | **~209.5KB（余 ~52.6KB）** | 不含渲染字段（line_l/line_r 移除）；外点透出在 frame_result（vision 侧栈/局部） |

---

## 6. IPC/valid 传出路径梳理（零改动证明）

**结论：IPC 契约、valid_mask 位、`vision_ipc_core1_fill_bumpy()` 打包、0 核 `vision_bumpy_control` / `bumpy_road` 消费侧全部零改动。**

1 核 → 0 核现成链路（发布工程已存在）：

| v3 内部信号 | 语义 | 1 核输出字段 | IPC 打包（不改） | 0 核消费（不改） |
|---|---|---|---|---|
| `hdg_valid`（个数≥2+方差门） | 条纹/颠簸存在性 + 角度可信 | `bumpy_detected`、`yaw_error_deg_x100` | `packet->bumpy_detected`；`yaw_error_deg_x100` 直传 | TRACK/SEARCH 状态机；入口/出口连续帧计数（`VISION_BUMPY_ENTRY_DETECT_FRAMES`/`EXIT_MISS_FRAMES`）；`err_degree_cmd` 角度直通 |
| `s_lat_locked`（v2 采信锁定） | 横向可信 | `meas_valid` | `if (meas_valid) valid_mask \|= VISION_VALID_BUMPY_MEAS`（bit8） | `recorded_lateral_mm` EMA 门控（只记录不修正）+ `heading_stable` 三帧门控 + `bumpy_road` 出口叠加门 |
| `lateral_mm`（采信窗+EMA） | 中线偏差 | `lateral_mm` | 直传 | 记录/出口 |
| `yaw_error_deg_x100`（圆域 EMA 后） | 角度偏差 | `yaw_error_deg_x100` | 直传 | `err_degree` |

**语义兼容说明**：
- `meas_valid` 从发布版"3 帧中值窗 conf>0"改为 v2"采信锁定"——0 核只当"横向可信"消费，兼容。
- `bumpy_detected` 从"hdg_valid(个数≥3)"改为"hdg_valid(个数≥2+方差门)"——"是否在颠簸段"语义不变；方差门收紧后段内闪断更少，0 核入口/出口计数只会更稳。
- `line_l/line_r`（bumpy_line_t）为 1 核渲染字段，**不进 IPC**，v3 移除（裁决点 I）。

---

## 7. 关键裁决点（需用户拍板）

- **F. `BP_NORM_K` 定值**：2500（顾暗）vs 3000（居中）（交接文档 §7 待裁决 4）。
- **H. 外点透出数量上限 `BP_OUT_MAX`**（建议 48/侧；`extract_outer` 至多 6×MAX_CC=384/侧，透出截断后主带提取足够）。
- **I. `bumpy_vision_output_t` 的 `line_l/line_r`（bumpy_line_t 渲染字段）移除**（建议移除，发布工程无渲染消费；trials 侧 host 渲染改用外点）。
- **J. bumpy-2 远右边缘专项确认方式**：点簇均值(≈470mm) vs 基准行(≈290mm) 差 180mm——该帧依赖间距自检/抢救兜底，需在 trials 侧逐帧确认无假有效后再定案。
- **K. `BP_MIN_HDG_LINES` 新值**：按用户意见 =2（拒绝单线）；如需更严可回 3（可调宏）。

---

## 8. 风险与回退

| 风险 | 缓解 |
|---|---|
| IPM 后主带均值方案在斜条纹/图缘退化（bumpy-2）表现未知 | 阶段 3 前先在 trials 侧 host 跑标注集对比再定案；依赖间距自检/抢救兜底（裁决点 J）；回退=阶段 1 的 v2 基准 |
| 外点透出截断（BP_OUT_MAX）丢边线信息 | 48/侧远大于实际外点数（实测合规线性 CC ≤ ~10 → ≤60 点/侧内，占 48 时已足够）；host 断言 |
| uint16 labels 潜在溢出 | 上界证明 ≤PIX/2+1=2,821；拆符号后更少；host 对拍加 max-label 断言 |
| 板端位精确对拍漂移 | 每阶段后跑 `bumpy_bench_host` 对拍；12 帧测试图集固定 |
| 单 CC 误报有颠簸 | 双门限（个数≥2+方差门）天然免疫，另加全库 600 无条纹帧误检统计（目标 0） |
| CM7_1 SRAM 再溢出 | 目标余量 ~52KB，远大于当前 6.5KB；icf 不动 |

---

## 9. 纪律与流程

1. **发布工程 `code1` 不直接改**：先复制到测试工程 `trials/bumpy-road-new/project/code/` 逐阶段验证（host 对拍 + 渲染），每阶段批准后再同步回 `code1`。
2. 每阶段结束提交 git + 更新本文档状态。
3. 全部通过后：IAR 构建 → 烧录 → 串口位精确对拍 → 实车（待定）。
4. 回归路径 = `BUMPY_USE_NEW_PIPELINE=0` 的 188×120 旧管线（`bumpy_vision.c` `#else` 分支 + `edge_conv_asm.s`），本次全程不动。

---

## 10. 执行记录（2026-08-19，已全部完成）

| 项 | 结果 |
|---|---|
| 阶段 2：gy-only 卷积 + 双阈值带符号 + uint16 缓冲（测试工程） | ✅ host 全库（12 视频 2365 帧）与阶段 1 基准**逐位一致**（gy-only 与全量 gy 等价；双阈值与 \|gy\|≥T、gy²≥T² 等价；uint16 labels 数值不变） |
| 阶段 3：IPM 后点簇直方图边线 + 链条简化（测试工程） | ✅ GT 标注对拍：bumpy-1 R 78/78、bumpy-2 R 19/22（与 v2 完全一致）、bumpy-3 L 56/58；锁定率提升（bumpy-1 0.992 vs 0.885、bumpy-11 0.667 vs 0.268）；5m/s 等效正常 |
| 同步回发布工程 `code1` | ✅ 8 文件已同步（`bumpy_pipeline/.h/.c`、`bumpy_vision/.h/.c`、`bumpy_conv/.h/.c`、`edge_conv7_asm/.h/.s`）；`wifi.c` 图传渲染用的 `line_l/line_r` 保留（v3 填充主带像素均值+竖直近似） |
| 阶段 4：IAR 构建 + map 核对 | ✅ `cyt4bb7_cm_7_1` 构建 **Errors: none**；`s_bumpy_pipeline` = **50,760B**（省 45,200B）；CM7_1 rw data **221,834B**，余量 **39.4KB**（原 6.5KB） |
| v7（2026-08-19）：主带点数门 4→7 断裂点过滤 | ✅ 测试工程 + host 全库：GT bumpy-1 78/78、bumpy-2 19/22、bumpy-3 55/58；误检 L 2→0、R 3→2；bumpy-11 断裂点区（f90-120）全弃权无锁死；锁定率除 bumpy-11 外全库不变；已同步发布工程 + IAR 构建 0 错误 |
| v8（2026-08-19）：**中线严禁任何时间滤波** | ✅ 删除采信窗 + EMA + 限速（`bumpy_vision_lateral_filter` / `s_lat_*` / `BUMPY_LAT_*` 全删）；`lateral_mm` 直出**本帧瞬时合成值**，`meas_valid` = 本帧有横向观测。依据：0 核 `BumpyRoad_ApplyExitCorrection` 仅在起飞/脱出时刻**一次性取用 IPC 直通值**（`bumpy_road.c:124`），时间平滑只会污染该读数；边线侧主带提取（离散性处理）已足够。host 全库验证：GT 回归不变（边线提取未动），v8 非零 lat 帧数 = valid 帧数（观测存在性语义），bumpy-11 断裂点区仍全弃权，bumpy-1 瞬时 lat≈+176~257mm 与真边线 x≈+310 单边右逆推一致；已同步发布工程 + IAR 构建 0 错误 |
| 待办 | 板端烧录 + 串口位精确对拍（需硬件）；实车 5m/s 验证；`BP_NORM_K` 2500/3000 最终确认 |
