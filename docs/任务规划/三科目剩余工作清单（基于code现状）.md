# 三科目开发任务表（基于 `code/` 目录现状）

> 更新说明：按最新进度同步——**科目一已完成、科目二已完成、科目三已完成各元素控制但尚未完成识别链路**。
> 本文重点给出「**每个文件具体改哪些函数**」的可执行任务表。

---

## 0. 当前完成状态（用于统一认知）

- 科目一（行进绕桩）：**已完成**（可运行链路具备）。
- 科目二（定点排雷）：**已完成**（转圈控制链路具备）。
- 科目三（颠簸路段）：**元素控制已完成**，但**元素识别/触发未完成**，目前缺少“识别 -> 触发 -> 控制”的完整闭环。

---

## 1. 本轮开发目标（只聚焦科目三识别闭环）

### 目标A：补齐识别输入
把“单边桥/颠簸/草地/台阶坡道”的识别结果，统一转换为可调度的事件数据。

### 目标B：补齐调度编排
将识别事件接入 `plan` 状态机，避免仅靠固定距离或手动触发。

### 目标C：补齐双向台阶流程
保证台阶坡道“正反方向各一次”的流程可被识别信息驱动。

---

## 2. 每个文件具体改哪些函数（开发任务表）

> 说明：下表按“文件 -> 函数级任务 -> 产出/验收点”给出，直接可分配给开发同学。

## 2.1 `code/plan/bridge.c`

### 需要修改/新增的函数
1. **修改 `Bridge_Trigger(float distance_to_bridge)`**
   - 现状：偏向距离触发。
   - 改造：支持“识别触发优先，距离触发兜底”。
   - 建议新增入参版本：
     - `Bridge_TriggerEx(float distance_to_bridge, uint8 detect_valid, float detect_confidence)`。

2. **修改 `Bridge_Update(void)`**
   - 在 `BRIDGE_STATE_BRAKE/READY/CLIMB` 增加识别稳定性判断（连续 N 帧有效才进入下一状态）。
   - 增加“识别丢失后的退化策略”（回退到低速 + 距离兜底）。

3. **新增 `Bridge_SetDetectStatus(...)`（建议）**
   - 统一缓存视觉识别结果（有效位、置信度、左右侧、相对距离）。
   - 避免在 ISR 中直接写状态机内部变量。

### 验收点
- 识别存在时优先按识别触发；识别丢失时不死锁，能平滑降级通过。

---

## 2.2 `code/plan/bridge.h`

### 需要修改/新增的接口
1. 新增桥识别数据结构（建议）：
   - `BridgeDetectInfo_t`（`valid/confidence/side/distance_mm`）。
2. 新增接口声明：
   - `void Bridge_SetDetectStatus(const BridgeDetectInfo_t* info);`
   - `void Bridge_TriggerEx(float distance_to_bridge, uint8 detect_valid, float detect_confidence);`
3. 补充状态查询接口（建议）：
   - `BridgeState_e Bridge_GetState(void);`

### 验收点
- 上层调度不用访问 `bridge.c` 内部静态变量，即可完成识别注入和状态读取。

---

## 2.3 `code/navigation/nav_replay.c`

### 需要修改/新增的函数
1. **修改 `NavReplay_Process(void)`**
   - 当前到特殊点后仅设置 `g_special_action_trigger`。
   - 改造为：触发后根据“识别结果 + 当前点类型”选择对应元素控制器入口。

2. **新增 `NavReplay_HandleSpecialPoint(uint8 point_type)`（建议）**
   - 将特殊点处理从主流程拆出，减轻 `NavReplay_Process` 复杂度。
   - 统一处理：桥/颠簸/草地/台阶。

3. **新增 `NavReplay_CanExitSpecialMode(void)`（建议）**
   - 用于判定元素任务何时完成并恢复主导航。

### 验收点
- 特殊点不再“触发后卡住”，能自动进入元素流程并自动退出回主路径。

---

## 2.4 `code/navigation/nav_replay.h`

### 需要修改/新增内容
1. 新增特殊任务类型枚举（如 `NAV_SPECIAL_BRIDGE/BUMP/GRASS/STAGE`）。
2. 声明新增接口：
   - `void NavReplay_HandleSpecialPoint(uint8 point_type);`
   - `uint8 NavReplay_CanExitSpecialMode(void);`

### 验收点
- 导航层头文件可完整表达“路径控制 + 特殊任务切换”的外部能力。

---

## 2.5 `code/plan/`（新增文件）

> 当前仅看到 `bridge` 与 `minefield`；科目三还需要把“已完成的元素控制”沉淀成独立模块并提供统一接口。

### 建议新增
1. `bump.c/.h`
   - `Bump_Init()`
   - `Bump_Trigger(...)`
   - `Bump_Update()`
   - `Bump_IsFinished()`

2. `grass.c/.h`
   - `Grass_Init()`
   - `Grass_Trigger(...)`
   - `Grass_Update()`
   - `Grass_IsFinished()`

3. `stage.c/.h`
   - `Stage_Init()`
   - `Stage_Trigger(uint8 direction)` // 正向/反向
   - `Stage_Update()`
   - `Stage_IsFinished()`
   - `Stage_GetPassCount()` // 统计双向完成次数

### 验收点
- 每个元素均有统一四件套：Init/Trigger/Update/IsFinished。
- 上层调度无需知道元素内部细节。

---

## 2.6 `user/cm7_0_isr.c`

### 需要修改的调用点
1. 在 10ms/20ms 调度区增加识别结果输入处理：
   - 从视觉/识别模块读取最新结果（建议只读“快照”结构）。
2. 将测试调用替换为正式调用：
   - 若当前仍在用 `Bridge_Test_Smooth_PID()`，改为任务态下调用 `Bridge_Update()`。
3. 增加元素控制统一调度入口（建议）：
   - `Plan_ElementScheduler_10ms()` 或同类函数。

### 验收点
- ISR 中不再是“单模块测试调用”，而是“识别输入 + 任务调度 + 控制输出”的正式链路。

---

## 2.7 `code/config/`（参数配置文件）

### 需要新增配置（建议新建 `plan_subject3_config.h`）
1. 识别稳定阈值：连续帧数、置信度阈值。
2. 各元素触发阈值：距离、速度、姿态容许范围。
3. 元素超时阈值：用于失败保护和状态机退出。

### 验收点
- 科目三参数不散落在多个 `.c` 文件中，可集中调参。

---

## 3. 建议实施顺序（按依赖）

1. 先补 `plan` 层统一接口（bridge/bump/grass/stage 四件套统一）。
2. 再改 `nav_replay` 特殊点处理（触发与退出规则）。
3. 再改 `cm7_0_isr.c` 接入识别快照和统一调度。
4. 最后做参数收敛与联调（`config` 集中化）。

---

## 4. 里程碑与DoD

### M1：识别可驱动单边桥（1-2天）
- DoD：桥元素可由识别触发，丢识别可降级通过。

### M2：识别可驱动全部科目三元素（2-4天）
- DoD：颠簸/草地/台阶均可由识别触发并自动退出。

### M3：双向台阶流程闭环（1-2天）
- DoD：正向一次 + 反向一次均完成，计数正确。

### M4：全流程稳定性回归（1-2天）
- DoD：连续多轮运行无死锁、无任务卡死、无异常重复触发。

---

如果需要，我下一步可以直接给出「按人分工版」任务单（算法/嵌入式/视觉各自负责到函数级别）。
