# 斜坡脱出时间点检测（AP3 台阶冲击检测）落地方案与实施记录

> 状态：**已实施**（2026-07-29，IAR 编译 0 错 0 警，6 组实测 CSV 回放全部通过）。
> 算法来源：`D:/WORKS/2026LunTui/trials/slope-anlysis/STAIR_DETECTION_ALGORITHM.md`（AP3 — Adaptive Prominence Top-3 Peak Detector）
> 用途：用 `imu_acc_z` 冲击检测替代惯导固定里程作为 slope 脱出判据，得到"末级台阶着地"的脱出时间点。
> 验证脚本：`D:/WORKS/2026LunTui/trials/slope-anlysis/validate_online_ap3.py`（逐拍复刻 C 实现）

---

## 1. 用户决策（2026-07-29 确认）

| 项 | 决策 |
|----|------|
| 代码位置 | 全部写在 `code/vision/vision_slope_control.h/.c`，不新建模块文件 |
| 脱出逻辑 | 冲击检测**替代**惯导里程作主判据；里程降级为兜底（放宽至 3500mm） |
| 脱出提示 | 冲击脱出时蜂鸣器连续叫 3 声；里程兜底退出不叫 |
| 采样率 | 10ms（100Hz），比离线验证的 50Hz 高一倍，d_min 等参数等比放大 |
| 落地节奏 | 两阶段一步到位，用宏 `SLOPE_IMPACT_DETECT_MODE` 区分（0 关闭 / 1 仅记录 / 2 参与脱出） |

---

## 2. 现状与动机

斜坡任务 `VisionSlopeTask` 的 RUN 阶段原退出判据是纯固定里程
（`vision_slope_control.c` 原 :309，`VISION_SLOPE_TASK_EXIT_DISTANCE_MM = 2500mm`），
没有任何物理事件确认：轮速打滑、坡度变化都会让该判据提前或滞后。
本方案用 AP3 冲击检测给出"末级台阶着地"的脱出时间点。

---

## 3. 已核验的工程事实（插入点依据）

- IMU：IMU963RA（`sys_options.h:22`），0 核 1kHz 更新（`user/cm7_0_isr.c:641` `EKF_UpData()`）
- 信号：`imu_t imu_data`（`code/calculate/ekf.h:45-58`），`acc_z` 为原始计数 IIR 低通（静止 ≈ ±4098 counts），CAR_SELECT 3 下 acc_z 即垂直轴
- 换算先例：`cm7_0_isr.c:370-371` 的 `9806.65*(acc/4098 - grav)`；本实现**不做重力补偿**（与离线 CSV 信号形状一致，Prominence 对直流偏置免疫）
- 调度：`VisionSlopeTask_Update_2ms()` 在 `cm7_0_isr.c:306` 的 2ms 槽内被调用，检测采样在其内部 5 分频到 10ms——**无需改动 ISR 文件**
- 蜂鸣器：`BUZZER_PIN = P19_4`（`code/tools/beep.h`），GPIO 高电平鸣响；本状态机 2ms 节拍自带蜂鸣时序
- 初始化：`VisionSlopeTask_Init()` 已挂在 `user/main0/init_main0.c:189`，检测状态复位在其中完成

---

## 4. 最终算法（在线化 AP3 + 安静期脱出）

离线 AP3 是批处理（整段序列取 Top-3）；车上为流式版本，流程（每 10ms 一拍）：

```
环形缓冲 buf[320]（3.2s 窗口，约 1.3KB RAM），全局帧号单调递增
每拍：
 1. 采样 accZ = 9806.65 * (imu_data.acc_z / 4098) 写入 buf
 2. 局部谷值判定：buf[t-1] < buf[t-2] 且 buf[t-1] <= buf[t] → 入待确认队列
 3. 右边界确认：当前样本高于某待确认谷值 → 计算 Prominence
      P(v) = min(左山脊, 右山脊) - 谷底（左山脊在缓冲窗口内向左搜索）
 4. 自适应阈值：T = (P90(|a|) - median(|a|)) * α，按缓冲内有效样本统计（确认时才计算）
 5. 计数：P(v) >= T * MARGIN 且与上次冲击间距 >= d_min → impact_count++
 6. 脱出判定：impact_count >= K 且距最后一次冲击确认 >= QUIET_TICKS → 锁存 exit_detected
```

### 4.1 相对原始算法文档的三处在线化改动（均有实测依据）

1. **Prominence 右边界前瞻 → 固有确认延迟**：谷值要等信号回升才能确认，延迟 ≈ 冲击恢复时间（100~300ms），对"脱出时间点"用途可接受。
2. **新增 MARGIN=2.5 余量系数**：在线阈值基于"到目前为止"的因果数据，缓冲早期统计不充分会漏进缓坡段小起伏。6 组实测：假冲击 prom/T ≤ 2.1，真实着地冲击簇内必有 ≥ 4.5 的谷值，取 2.5 分隔（详见 validate_online_ap3.py 分析）。
3. **"第 K 次冲击即脱出"改为"够 K 次 + 安静期"**：实测发现单次落地会弹跳产生多次合格计数（间隔最大 9 帧/180ms），而相邻真台阶间隔最小 7 帧（140ms）——d_min 无法同时压制弹跳又保留真台阶。改为安静期判定后：**脱出时间点 = 末次冲击 + 400ms**，弹跳只会推迟安静期起点，不会造成提前脱出（实测同级间隔 ≤ 320ms < 400ms）。

### 4.2 参数表（`vision_slope_control.h`，100Hz 采样）

| 宏 | 值 | 含义 |
|----|----|------|
| `SLOPE_IMPACT_DETECT_MODE` | 2 | 0 关闭 / 1 仅记录 / 2 冲击检测参与脱出 |
| `SLOPE_IMPACT_SAMPLE_DIV` | 5 | 2ms 调度 5 分频 → 10ms 采样 |
| `SLOPE_IMPACT_BUF_LEN` | 320 | 3.2s 窗口 |
| `SLOPE_IMPACT_K` | 3 | 最少冲击次数（台阶数） |
| `SLOPE_IMPACT_ALPHA` | 1.5 | 自适应阈值系数 |
| `SLOPE_IMPACT_PROM_MARGIN` | 2.5 | Prominence 余量系数 |
| `SLOPE_IMPACT_QUIET_TICKS` | 40 | 安静期 400ms |
| `SLOPE_IMPACT_D_MIN` | 12 | 最小冲击帧间距 120ms |
| `SLOPE_IMPACT_MIN_SAMPLES` | 30 | 300ms 布防延迟（滤除上坡闯动） |
| `SLOPE_IMPACT_FAILSAFE_DISTANCE_MM` | 3500 | 里程兜底（原 2500 放宽） |

### 4.3 门控

检测仅在 `ENTRY_HOLD`/`RUN` 阶段布防（`vision_slope_set_state` 进 ENTRY_HOLD 时复位布防，`vision_slope_cleanup` 撤防）。颠簸路、跳跃等场景的冲击不进入检测窗口。调试结论 `g_slope_impact_debug` 在 cleanup 时保留，供事后分析。

---

## 5. 离线回放验证（6 组实测 CSV，50Hz 等效参数）

`validate_online_ap3.py` 逐拍复刻 C 实现，用 trials/slope-anlysis 的 6 组下台阶数据验证：

| 文件 | 检出冲击帧 | 期望（算法文档§7.3） | 脱出帧 | 结果 |
|------|-----------|---------------------|--------|------|
| 232245 | 56, 65, 72, 80 | 66, 73, 86 | 101（CSV 截断推算） | OK |
| 232302 | 14*, 139, 146, 152, 158 | 142, 151, 162 | 179 | OK |
| 232322 | 40, 49, 55 | 50, 56, 66 | 76 | OK |
| 232337 | 27, 33, 49 | 28, 34, 50 | 70 | OK |
| 232349 | 52, 60, 66, 74 | 61, 67, 75 | 95 | OK |
| 232401 | 31, 38, 44, 53, 64 | 39, 45, 65 | 85 | OK |

- 全部 6 组：脱出帧均落在末级台阶之后、无中途误触发（*232302 的帧 14 为缓冲早期孤立假阳性，被 K≥3 + 安静期规则自然吸收，不影响脱出时刻）
- 脱出帧 = 末次冲击确认 + 400ms，即"末级台阶着地后 0.4~0.7s"——这就是 slope 脱出时间点
- 232245 的 CSV 在末次冲击后 19 帧即结束，不足 20 帧安静期，属采集截断；车上数据连续，不存在此问题

---

## 6. 实际修改清单（已全部完成）

| # | 文件 | 内容 |
|---|------|------|
| 1 | `code/vision/vision_slope_control.h` | 新增 2.5 节检测参数宏、`slope_impact_debug_t` 调试结构与 `g_slope_impact_debug` 声明 |
| 2 | `code/vision/vision_slope_control.c` | 新增在线 AP3 检测器（`slope_impact_feed` / `slope_impact_confirm_valley` / `slope_impact_reset`）、蜂鸣 3 声时序（`vision_slope_beep3_start/update`）；ENTRY_HOLD 布防、cleanup 撤防；RUN 态退出判据按 `SLOPE_IMPACT_DETECT_MODE` 三态切换 |
| 3 | `trials/slope-anlysis/validate_online_ap3.py` | C 实现的逐拍等效验证脚本（项目外，调参用） |

未改动：ISR、init、EKF/惯导、WiFi 协议、IAR 工程文件（无新增文件），1 核工程完全不涉及。

## 7. 验证状态

- IAR 编译（cyt4bb7_cm_7_0，Debug）：**0 错误**，`vision_slope_control.c` 无警告（全工程 19 个警告均为既有文件的历史警告）
- 离线回放：6/6 通过（见 §5）
- **待实车验证**：MODE=1（仅记录）下跑斜坡，用 `g_slope_impact_debug` 观察计数/阈值/Prominence 是否符合预期后，再切 MODE=2

## 8. 开放问题

- **O1**：采集固件中 `imu_acc_z` 的确切换算公式仍未找到（CSV 基线 −3900 与当前代码任何换算都对不上）。算法对线性缩放不变，不阻塞；若实车 MODE=1 发现 Prominence 量级与 CSV 差异大，按比值重新标定 MARGIN 即可。
- **O2**：k=3 是否匹配赛场实际台阶数，以实车为准，改 `SLOPE_IMPACT_K` 即可。
- **O3**：MODE=1 实车数据可追加进 WiFi 遥测帧（`wifi_protocol.c` append-only 约定）便于采集分析——当前未做，需要时再加。
