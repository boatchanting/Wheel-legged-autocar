# 循迹控制算法升级执行文档 (纯追踪 $\rightarrow$ Stanley)

> **执行者请注意（核心业务背景）**：
> 本项目车辆为**两轮平衡车**，无实体转向舵机！
> 最终输出的 `err_degree` 实际上是作为**目标偏航角/偏航角速度的代理指令**，交由底层的**差速PID环**（通过左右轮转速差）来实现转向。
> 因此，本次算法升级的目标是：计算出更精确的 `err_degree`，让差速环控制车身严格贴合参考轨迹，消除纯追踪算法带来的“切弯”现象。

## 1. 修改目标
将循迹主循环（`NavReplay_Process`）中第6阶段的 **“纯追踪前瞻计算”** 替换为 **“Stanley 算法（高阶PID）”**。
*   **保留**：阶段1~5（特殊动作、接管恢复、最近点搜索、停止屏障、近停点航向锁死）。
*   **删除**：阶段6a（前瞻距离计算）、阶段6b（前瞻目标查找）。
*   **重构**：阶段6c（利用 Stanley 算法计算横向偏差与航向偏差）。
*   **保留**：阶段6.5（曲率前馈）、阶段7（速度规划）。

---

## 2. 代码框架准备

为了不破坏原有逻辑，请通过宏定义路由新增一个方案：

1. 打开 `code/navigation/nav_replay/nav_options.h`，新增宏定义：
   ```c
   #define PLAN1_STANLEY_TRACKING 4  // 新增：Stanley循迹 + 离线速度规划
   ```
2. 复制 `plan1_pure_pursuit_speed_planning.c` 和 `.h`，重命名为 `plan1_stanley_tracking.c/.h`。
3. 在 `nav_replay.h/.c` 中做好相应的 `#elif` 包含配置。
4. 后续所有修改均在 `plan1_stanley_tracking.c` 中进行。

---

## 3. 核心修改步骤 (修改主循环流水线)

定位到 `plan1_stanley_tracking.c` 中的 `NavReplay_Process` 函数的主循环8阶段流水线。

### 🔪 步骤 3.1：删除无用逻辑 (阶段6a & 6b)
在原代码中，找到并**完全删除**以下逻辑及相关变量：
1. `lookahead_min`、`ahead_kappa`、`shrink_factor`、`lookahead_dist` 的计算逻辑（原阶段6a）。
2. `NavReplay_FindLookaheadTarget(...)` 函数调用及其相关变量 `tx`, `ty`, `out_idx`（原阶段6b）。

### 🛠️ 步骤 3.2：重写控制核心 (阶段6c)
在原阶段6c的位置，插入以下 Stanley 算法实现。

```c
// ==================== 阶段 6c: Stanley 核心计算 ====================
float raw_err_degree = 0.0f;

// 1. 航向锁死分支 (停车专用)
if (s_stop_lock_active) {
    err_degree = NormalizeAngle(s_stop_lock_yaw_deg - inertial_nav.relative_yaw);
} 
// 2. Stanley 正常分支
else {
    // 提取当前最近点(Pi)和下一个点(Pi+1)构成的路径线段
    int current_idx = g_target_idx;
    int next_idx = current_idx + 1;
    if (next_idx > last_idx) { // 防止终点越界
        next_idx = last_idx;
    }

    // 坐标准备
    float x1 = nav_ram_data.points[current_idx].x;
    float y1 = nav_ram_data.points[current_idx].y;
    float x2 = nav_ram_data.points[next_idx].x;
    float y2 = nav_ram_data.points[next_idx].y;
    float cx = inertial_nav.x;
    float cy = inertial_nav.y;

    // --- A. 计算横向偏差 (Cross-Track Error, CTE) ---
    float dx_path = x2 - x1;
    float dy_path = y2 - y1;
    float dx_car = cx - x1;
    float dy_car = cy - y1;
    
    float path_len = sqrtf(dx_path * dx_path + dy_path * dy_path);
    float cross_track_error = 0.0f;
    if (path_len > 0.001f) {
        // 叉乘计算距离，自带左右正负号特征
        cross_track_error = (dx_car * dy_path - dy_car * dx_path) / path_len;
    }

    // --- B. 计算航向偏差 (Heading Error) ---
    // 注意：这里的 target_yaw 必须是当前点的切线方向
    float target_yaw = nav_ram_data.points[current_idx].target_yaw_deg;
    float heading_error = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // --- C. Stanley 综合误差 (作为差速系统的虚拟航向目标) ---
    // 限制最小车速防除零，同时引入软化常数避免极低速时抖动
    float v_car = fmaxf(fabsf(inertial_nav.vx_body), 10.0f); 
    
    // Stanley 公式：航向误差 + 横向修正角度
    // （如果实车测试时发现越走越偏，请将 cross_track_error 前加负号）
    float cte_correction_rad = atan2f(STANLEY_K * cross_track_error, v_car);
    float cte_correction_deg = cte_correction_rad * 57.29578f; 

    raw_err_degree = NormalizeAngle(heading_error + cte_correction_deg);

    // --- D. 差速指令滤波 (防止平衡车因为突变指令发生倒伏或剧烈抖动) ---
    // 这里取前瞻曲率用于判断是否处于弯道 (借用下方计算好的ff_idx)
    int bypass_idx = current_idx + 15;
    if (bypass_idx > last_idx) bypass_idx = last_idx;
    float bypass_kappa = nav_ram_data.points[bypass_idx].curvature;
    
    if (fabsf(bypass_kappa) > KAPPA_CURVE_BYPASS_THRESH) {
        err_degree = raw_err_degree; // 弯道旁路低通，释放敏捷性
    } else {
        // Slew Rate 限幅 + 低通滤波
        float diff = raw_err_degree - s_prev_err_degree;
        if (diff > SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree + SLEW_RATE_ANGLE;
        else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree - SLEW_RATE_ANGLE;
        
        err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * s_prev_err_degree;
    }
}

```

### 🤝 步骤 3.3：无缝对接阶段 6.5 (曲率前馈)
这一步**完全保留原逻辑**，但因为移除了前瞻，曲率数据来源从 `ahead_kappa` 改为直接取当前最近点曲率：

```c
// ==================== 阶段 6.5: 曲率前馈 (已修正前瞻丢失问题) ====================
// 【注意】曲率必须用前瞻点！否则平衡车入弯必定滞后。
// 恢复原方案中的固定前瞻点（默认+15点，即约前瞻750mm），这是为了抵消差速底盘的转动惯量延迟
int ff_idx = g_target_idx + 15; 
if (ff_idx > last_idx) ff_idx = last_idx;
float target_kappa = nav_ram_data.points[ff_idx].curvature; 

// 叠加前馈（直接加在滤波后的 err_degree 上）
float feedforward_angle = target_kappa * fabsf(s_prev_speed_set) * K_FF_CURVATURE;
err_degree = err_degree + feedforward_angle;
```
*(阶段7速度规划无需任何改动，直接保留)*

---

## 4. 头文件 (.h) 新增参数

在 `plan1_stanley_tracking.h` 中，删除与 `PP_LD_`（前瞻相关）的宏，新增 Stanley 参数宏：

```c
// 删除以下宏：
// PP_LD_MIN_CURVE, PP_LD_MIN_STRAIGHT, PP_LD_SPEED_GAIN, PP_LD_KAPPA_SHRINK_GAIN, PP_LD_KAPPA_SHRINK_MIN

// ================= Stanley 算法专用参数 =================
// Stanley 横向误差增益系数。
// 越大越紧贴轨迹（但过大会导致高频差速振荡）；越小越平滑但会轻微切弯。
#define STANLEY_K 2.5f 
```

---

## 5. 调试指南 (交给负责下车调参的人)

因为是两轮平衡车（差速转向），调参步骤与普通舵机车有细微区别，请按以下顺序整定：

1. **确定符号极性 (核心第一步)**：
   * 屏蔽横向修正：将 `STANLEY_K` 设为 `0.0f`。
   * 屏蔽前馈：将 `K_FF_CURVATURE` 设为 `0.0f`。
   * 让车跑起来，此时系统退化为**纯航向角跟踪**。如果车能够大致顺着路跑（虽然偏离轨道但车头方向是对的），说明航向符号正确。
   * **如果车原地打转，说明 `heading_error` 的符号反了，请在代码中取负。**
2. **整定横向偏差极性**：
   * 将 `STANLEY_K` 设为 `1.0f`。
   * 观察车如果偏离轨迹，是**向轨道靠拢**还是**加速远离轨道**。
   * **如果加速远离（发散），说明 CTE 叉乘算出来的符号与你的差速系定义反了，请将 `cross_track_error` 的计算结果前面加一个负号。**
3. **推高增益 (STANLEY_K)**：
   * 逐渐增大 `STANLEY_K`（如 1.5 $\rightarrow$ 2.5 $\rightarrow$ 3.5）。
   * 直到平衡车在直道出现轻微的左右高频扭动（差速画龙），然后回调 20%。
4. **加入曲率前馈 (K_FF_CURVATURE)**：
   * 恢复原有的 `K_FF_CURVATURE` 值（原来是 100）。
   * 观察过弯。如果有“出弯甩尾”或“入弯滞后”，微调该参数。前馈负责“预判”差速，Stanley负责“兜底”修正。