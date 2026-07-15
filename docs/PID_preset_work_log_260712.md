# 多套 PID 预设调用情况及近期工作总结 (2026-07-12)

本文档记录了关于平衡车导航循迹过程中“多套 PID 预设动态切换”功能的工作成果、代码机制以及在实车测试中发现的底层控制特性，方便后续接手该工程的开发者快速理解。

## 一、 工作背景与目标
平衡车在执行上位机规划的路径（路表）时，由于统一使用了一套 PID 参数，导致在直线加速和急弯减速时，无法兼顾平顺性和极限性能。
**本次工作的目标是：** 在不改变上层路径规划路表的前提下，修改底层循迹算法，使其能够根据**实际车速与规划车速的差值**，动态切换 PID 预设参数（加速 `ACCEL`、正常 `NORMAL`、刹车 `BRAKE`），以获得更好的加减速性能，同时确保该机制不影响遥控器直控模式。

## 二、 工作成果清单

### 1. 动态 PID 切换逻辑植入
在底层的多套循迹算法（`plan1_lqr_tracking.c`, `plan1_pure_pursuit.c`, `plan1_pure_pursuit_speed_planning.c`）的核心速度更新函数 `NavReplay_SpeedSlew_Update` 中，植入了基于实际速度的模式判断逻辑：
*   **判断依据**：提取目标规划速度 `raw_speed` 和 实际车速 `current_actual_speed`。
*   **模式判定**：
    *   实际速度落后于目标速度较多时：请求 `CONTROL_MODE_ACCEL`（加速模式）。
    *   实际速度超前于目标速度较多时：请求 `CONTROL_MODE_BRAKE`（刹车模式）。
    *   速度匹配时：请求 `CONTROL_MODE_NORMAL`（正常模式）。
*   **防抖机制**：加入了 300ms (30 ticks) 的切换冷却时间 `s_mode_cooldown`，防止模式频繁跳变导致电机抖动。同时为 `CONTROL_MODE_BRAKE` 开启了紧急豁免权，随时可以无视冷却立即切入。
*   **解耦遥控**：该判定逻辑仅在 `NavReplay_SpeedSlew_Update` 内部生效，仅在循迹任务开启时介入，完全不影响遥控器模式下的操控逻辑。

### 2. 上位机与通信协议可视化升级
为了直观地在上位机界面上验证 PID 切换是否按预期工作，我们打通了下位机到上位机的状态回传链路：
*   **下位机 (`tools/wifi_protocol.c`)**：在 WiFi 数据包的末尾动态追加了 1 字节的 `g_control_mode_applied` (当前生效的 PID 模式)。
*   **Python 解析端 (`nav_marker_host.py`)**：考虑到其他开发分支（如纯 GPS 分支）可能没有这 1 字节的数据，解析脚本采用了向后兼容的末尾提取法 `pid_mode = payload_bytes[-1]`，不破坏原有定长结构。
*   **HTML 前端 (`nav_marker.html`)**：通过修改前端 `canvas` 的 `strokeStyle`，实现了轨迹按 PID 模式变色：
    *   🟥 **红色段**：代表小车正处于 `ACCEL` (加速) 模式。
    *   🟦 **蓝色段**：代表小车正处于 `NORMAL` (正常) 模式。
    *   🟨 **黄色段**：代表小车正处于 `BRAKE` (刹车) 模式。

### 3. 代码修复与同步
*   修复了 `current_actual_speed` 变量在跨文件调用时因 `volatile` 关键字修饰不一致导致的 IAR 编译报错问题。
*   将 PID 切换逻辑全面同步到了所有 plan1 方案中，确保无论系统宏定义 `#define NAV_PLAN1_METHOD` 切换为 LQR 还是 Pure Pursuit，该功能均能正常生效。

## 三、 核心问题深度剖析 (重要交接内容)

在完成可视化后，实车测试暴露出一个极其重要的现象：
**“小车加速极快（轨迹短红），但减速极慢（轨迹呈现连绵不断的漫长黄色），并最终导致在急弯桩桶处因降速不及而打滑。”**

经过对源码的深度追溯，我们明确了造成该现象的根本原因：**循迹斜率限制器与底层刹车前馈机制的脱节。**

#### 1. 为什么加速像火箭？
在 `NavReplay_SpeedSlew_Update` 中，加速段存在一个**“绿色通道”**：
```c
// 加速段直接给目标速度，保留目标速度台阶，避免把加速前馈的触发条件抹平。
if (((raw_speed * s_prev_speed_set) >= 0.0f) && (abs_raw > (abs_prev + NAV_SPEED_SLEW_EPS))) {
    return raw_speed; // 直接 Bypass 限制
}
```
当路表要求加速时，目标速度会瞬间跳变。巨大的瞬时速度差 (`diff`) 瞬间唤醒了底层 PID 的“加速前馈机制”，电机瞬间输出极大扭矩，起步极快。

#### 2. 为什么减速像没刹车？
在同样的函数中，减速段没有“绿色通道”，而是被斜率限制器（如 `NAV_SPEED_SLEW_DOWN_FAST`，当前为 95.0f/10ms）死死卡住：
```c
else if ((abs_raw + NAV_SPEED_SLEW_EPS) < abs_prev) {
    step_limit = (abs_prev > NAV_SPEED_SLEW_FAST_DECEL_TH) ? NAV_SPEED_SLEW_DOWN_FAST : NAV_SPEED_SLEW_DOWN_NORMAL;
}
return s_prev_speed_set + Float_Constrain(diff, -step_limit, step_limit);
```
*   **后果**：目标减速指令被“磨平”成了一个极缓的下坡。
*   **连锁反应**：小车的实际速度很容易跟上这个极缓的目标速度，导致它俩之间的瞬时偏差 (`diff = actual - target`) 永远很小（例如只有 20~30）。
*   **底层失忆**：底层刹车前馈的唤醒阈值是 `BRAKE_ERR_MIN` (40.0f) 或 `BRAKE_ERR_HEAVY_MIN` (150.0f)。因为瞬时偏差被上层斜率限制器抹平了，**强大的底层刹车前馈永远处于沉睡状态**。小车只能依靠极弱的普通速度环比例项 (P) 软绵绵地减速。
*   **黄线漫长**：由于一直达不到目标速度，判定条件持续将其锁定在 `BRAKE`（黄色）模式，直到车子拖着过高的车速滑出弯道。

## 四、 下一步优化建议

未来的接手者若要解决“循迹急刹弱”的问题，可以从以下两个方向入手（二选一）：

1.  **方案 A（暴力解法，修改宏定义）**：
    在相应的头文件（如 `plan1_lqr_tracking.h`）中，大幅调高减速斜率限制：
    ```c
    #define NAV_SPEED_SLEW_DOWN_NORMAL         200.0f  // 甚至更高
    #define NAV_SPEED_SLEW_DOWN_FAST           400.0f  // 甚至更高
    ```
    通过放行更陡峭的目标速度跌落，人为制造足够大的瞬时 `diff` 误差，强行唤醒底层的刹车前馈机制。

2.  **方案 B（机制解耦，修改架构）**：
    不依赖于瞬间速度差（`diff`）来被动触发刹车前馈。
    既然上层 `NavReplay_SpeedSlew_Update` 已经明确判定当前处于 `CONTROL_MODE_BRAKE`，可以将此状态下发到底层 PID。在底层的 Brake Presets 中，赋予更强的速度环比例增益（P），或者启用一套专属的、不依赖大 `diff` 也能激活的循迹刹车补偿，使系统做到“只要见黄线，制动必给力”。

---
*文档编制日期：2026-07-12*
