# tools/sbus 模块文档

## 1. 模块作用

`sbus` 模块负责解析枪遥控器输入并输出小车控制意图（速度、转向、模式、触发信号）。  
对应源码：`code/tools/sbus.h`、`code/tools/sbus.c`。

## 2. 核心数据结构 `robot_ctrl_t`

- `target_angle`：期望转向角（增量积分后）
- `target_speed`：期望速度（增量积分后）
- `mark_trigger`：打点触发沿
- `motor_enable`：电机总使能（急停开关）
- `point_type`：导航打点类型

全局实例：`extern robot_ctrl_t robot_ctrl;`

## 3. 关键接口

- `Remote_Control_Init()`：遥控逻辑初始化
- `Remote_Control_Process()`：遥控数据处理任务

## 4. 推荐调用方式

- 系统初始化时调用一次 `Remote_Control_Init()`。
- 在定时任务中以 10ms 或 20ms 周期调用 `Remote_Control_Process()`。
- 控制层从 `robot_ctrl` 读取目标值与开关状态。

## 5. 注意事项

- `mark_trigger` 是触发沿语义（瞬时位），使用后需按上层逻辑清零。
- `motor_enable` 建议直接作为安全门控量参与驱动输出。
