# 雷区转圈平滑交接设计

## 目标

避免雷区转圈结束时普通导航转向指令一次性接管而产生较大的反向转向 PWM；同时，转向输出必须始终为基础平衡环保留至少 2500 PWM 的余量。

## 范围和不变量

- 不改变现有雷区的触发、转圈角度判定、惯性缓冲或捕获策略。
- 雷区转圈和惯性缓冲期间，转向角速度环 `Turn_Gyro_Loop_Control()` 每个控制周期持续执行；不得通过将 `turn_gyro_loop_out` 清零来停环。
- 仅禁用舵机速度环 `Servo_Speed_Control()`，并清空其运行历史，防止其在禁用期间积累状态。
- 进入雷区时保存当前 `servo_height`，再把高度目标设为 `3.0`。结束后恢复保存的高度目标。

## 状态与数据流

在 `cm7_0_isr.c` 的转向控制路径维护以下状态：

1. `NORMAL`：普通导航角度环的完整输出送入转向角速度环。
2. `SPIN`：保留现有 `Minefield_Spin_Controller()` 指令，禁用舵机速度环，高度目标为 3.0。
3. `HANDOFF`：雷区动作完成后进入。恢复保存的高度和舵机速度环；普通导航角度环继续计算，但其输出乘以从 0 到 1 的 `handoff_ratio`。达到 1 后回到 `NORMAL`。

进入 `HANDOFF` 时重置普通导航转向角度环与转向角速度环的 PID 历史。随后转向角速度环立即以逐步增大的导航指令运行，避免继承旧误差或产生微分跳变。

## PWM 余量

当前轮毂电机混控为：

```
left  = balance + drive_feedforward + turn
right = -balance - drive_feedforward + turn
```

最终电机限幅为 `OUR_PWM_MAX_LIMIT = 8000`。雷区 `SPIN` 和 `HANDOFF` 阶段将 `turn_gyro_loop_out` 限制为：

```
abs(turn_gyro_loop_out) <= OUR_PWM_MAX_LIMIT - MINEFIELD_BALANCE_PWM_RESERVE
```

默认预留值 `MINEFIELD_BALANCE_PWM_RESERVE = 2500`，故当前最大绝对值为 5500 PWM。该限幅作用于 `Turn_Gyro_Loop_Control()` 的最终输出，而非只限制角速度指令。

## 可配置项

- `MINEFIELD_SPIN_HEIGHT_TARGET = 3.0f`
- `MINEFIELD_BALANCE_PWM_RESERVE = 2500.0f`
- `MINEFIELD_SPIN_HANDOFF_DURATION_MS`：默认交接总时长
- `MINEFIELD_SPIN_HANDOFF_RATIO_STEP`：默认每毫秒比例增量，由总时长推导并校验

## 验证

- 为状态与限幅逻辑添加/扩展主机侧测试，覆盖 `SPIN`、`HANDOFF` 和 `NORMAL` 的关键边界。
- 对变更文件执行现有测试或可用的语法检查。
- 人工检查：惯性缓冲阶段不再将 `turn_gyro_loop_out` 置零，舵机速度环仅在雷区活动时不调用。
