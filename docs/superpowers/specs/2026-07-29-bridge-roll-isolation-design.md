# 单边桥 rolling 与主动侧倾隔离设计

## 目标

进入单边桥时，转向角速度产生的主动侧倾不得影响腿部高度；单边桥专用的 rolling 单侧收腿补偿仍然保留。

## 问题根因

普通行驶中，5 ms 中断把转向角速度换算为 `roll_degree`，并通过 `Turn_Active_Roll_Duty_Update()` 写入四条腿的主动侧倾差动 PWM。

桥任务接管后，虽然会清空主动侧倾差动 PWM，但中断仍以进入前遗留的 `roll_degree` 调用 `Roll_Balance_Control()`。这会覆盖桥状态机锁存的横滚目标，把“转向主动侧倾”的旧目标伪装成桥内 rolling 补偿，导致一侧腿被收回，不能稳定到左右腿均为 6.0 cm 的入桥姿态。

## 行为设计

### 入桥与抬腿阶段

从桥任务开始到 `servo_height` 到达 `bridge_params.height_bridge`（当前为 6.0 cm）前：

- 强制清空 `Turn_Active_Roll_Target_Update()` 和 `Turn_Active_Roll_Duty_Update()` 产生的主动侧倾命令；
- 清除遗留的 `roll_degree`，不允许它作为桥内 rolling 目标；
- 关闭桥内被动 rolling 补偿；
- 仅执行公共高度命令，使左右四条腿同步升到 6.0 cm。

### 高姿态到位后的桥内补偿

当高度误差小于现有到位阈值时：

- 锁存当前 `euler_angle.roll` 到 `locked_roll_deg`；
- 启用桥内的 `Roll_Balance_Control(euler_angle.roll, locked_roll_deg)`；
- 只允许该桥状态机调用被动 rolling 补偿；
- 无论航向锁定产生多大的转向角速度，主动侧倾四腿差动 PWM 均保持为零。

被动 rolling 补偿可在桥面姿态偏离锁存横滚角时按原有“单侧收腿”逻辑修正。这与主动侧倾不同，且不会在抬腿阶段干扰两侧同步到达 6.0 cm。

### 退出与恢复

进入下桥阶段时清除桥内被动 rolling 输出和主动侧倾差动 PWM。桥任务结束后，5 ms 中断恢复其普通任务逻辑；主动侧倾是否启用仍遵循原有全局开关。

## 实现边界

- 修改 `user/cm7_0_isr.c`：将视觉单边桥与桥测试任务区分。视觉桥任务接管时，主动侧倾清零，且中断不再调用 `Roll_Balance_Control()`，防止覆盖桥状态机的 `locked_roll_deg`。
- 修改 `code/vision/vision_bridge_control.c`：入桥时清除遗留的主动侧倾目标；桥状态机继续作为视觉桥任务唯一的 rolling 补偿写入者。
- 不改变桥测试状态机和普通行驶的主动侧倾策略。

## 验收与回归测试

1. 以非零转向角速度、非零遗留 `roll_degree` 进入单边桥：主动侧倾四腿差动 PWM 必须立即归零。
2. 高姿态尚未到位时：桥内 rolling 输出为零，四腿只接收相同的 6.0 cm 高度目标。
3. 高姿态到位后施加横滚误差：桥内被动 rolling 补偿仍能产生单侧收腿输出；主动侧倾四腿差动 PWM 仍为零。
4. 下桥后：桥内 rolling 输出清零，普通任务的控制权恢复。
