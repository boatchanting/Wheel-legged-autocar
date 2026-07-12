# 多预设 PID 调用指南

## 1. 架构概述

当前 PID 控制采用 **ControlProfile（控制预设）** 机制：将所有 PID 环的参数、前馈参数、执行器限幅参数打包成一个结构体，预先定义多套，运行时按场景切换。

```
┌─────────────────────────────────────────────────────┐
│  上层（导航/复刻/遥控器）                              │
│    Control_Profile_RequestMode(CONTROL_MODE_ACCEL)   │
└──────────────────────┬──────────────────────────────┘
                       │ 1ms 平滑过渡
                       ▼
┌─────────────────────────────────────────────────────┐
│  g_control_profile_active（当前生效参数）              │
│  ┌─────────┬─────────┬─────────┬──────────────────┐ │
│  │NORMAL   │ACCEL    │BRAKE    │ 可扩展更多...     │ │
│  │(默认)   │(加速)   │(刹车)   │                   │ │
│  └─────────┴─────────┴─────────┴──────────────────┘ │
└──────────────────────┬──────────────────────────────┘
                       │ 每1ms写入各PID环
                       ▼
┌─────────────────────────────────────────────────────┐
│  6个PID环 + 前馈 + 执行器                            │
│  servo_speed → angle → gyro（平衡）                  │
│  turn_angle → turn_gyro（转向）                      │
│  roll（横滚）                                        │
│  brake_ff / accel_ff（前馈）                         │
│  servo_executor（舵机斜率限制）                       │
└─────────────────────────────────────────────────────┘
```

## 2. 三套预设定义

定义在 `code/calculate/pid-new.c` 中：

| 模式 | 枚举值 | 变量名 | 典型用途 |
|------|--------|--------|----------|
| `CONTROL_MODE_NORMAL` | 0 | `g_control_profile_normal` | 直道巡航、常规行驶（默认） |
| `CONTROL_MODE_ACCEL` | 1 | `g_control_profile_accel` | 起步、出弯提速、直道加速 |
| `CONTROL_MODE_BRAKE` | 2 | `g_control_profile_brake` | 弯前减速、停车、刹车场景 |

### 各预设主要差异（当前参数）

| 参数 | NORMAL | ACCEL | BRAKE | 说明 |
|------|--------|-------|-------|------|
| 舵机速度环 Kp | -4.5 | -5.4 | -4.8 | 加速时更激进，刹车时更保守 |
| 舵机速度环 Kd | -0.17 | -0.14 | -0.22 | 刹车时阻尼更大防后坐 |
| 舵机速度环 MaxO | 2000 | 2600 | 2300 | 加速时允许更大调节量 |
| 角度环 Kp | -12.0 | -13.2 | -10.8 | 加速时直立更硬 |
| 角度环 Kd | -13.33 | -11.8 | -15.0 | 刹车时阻尼更大 |
| 转向角速度环 MaxO | 8000 | 8000 | 7200 | 刹车时限制转向力矩 |
| 刹车前馈增益(轻/中/重) | 4/10/22 | 3.2/8.5/18 | 4.8/11.5/25 | 刹车模式更强制动力 |
| 加速前馈增益 | 10 | 13 | 6 | 加速模式更强推力 |
| 舵机执行器加速限幅 | 10 | 22 | 14 | 加速时舵机响应更快 |

> **注意**：ACCEL 和 BRAKE 的参数注释标注为"AI随便写的，需要调整"，实际使用前需根据实车标定。

## 3. API 说明

头文件：`code/calculate/pid-new.h`

### 3.1 请求切换（推荐）

```c
void Control_Profile_RequestMode(ControlMode_e mode);
```

- 设置目标预设，底层通过 `Control_Profile_Update1ms()` 以指数平滑方式逐步过渡
- **不会立即生效**，过渡时间取决于 alpha 系数（约 100~200ms 完成 95% 过渡）
- **线程安全**：只需在导航/复刻模块中写入，ISR 中读取

```c
// 示例：进入弯道前请求刹车预设
Control_Profile_RequestMode(CONTROL_MODE_BRAKE);
```

### 3.2 立即切换（特殊场景）

```c
void Control_Profile_ApplyNow(ControlMode_e mode);
```

- 跳过平滑过渡，直接将目标预设写入生效参数和所有 PID 环
- 适用于：上电初始化、倒地恢复后、需要立即切换的紧急场景

```c
// 示例：倒地恢复后立即重置为 NORMAL
Control_Profile_ApplyNow(CONTROL_MODE_NORMAL);
```

### 3.3 查询当前模式

```c
extern volatile ControlMode_e g_control_mode_requested;  // 请求的目标模式
extern volatile ControlMode_e g_control_mode_applied;    // 实际生效的模式（平滑后）
```

## 4. 调用位置与时机

### 4.1 当前已有的调用

| 位置 | 调用 | 说明 |
|------|------|------|
| `pid-new.c` `Control_Profile_Init()` | `ApplyNow(NORMAL)` | 上电初始化，默认 NORMAL |

### 4.2 建议的调用点

以下场景**尚未接入**，是框架预留的调用位置：

#### 场景 A：导航复刻中的加减速

```c
// code/navigation/nav_replay.c 或类似文件
// 在复刻状态机中根据路径段类型切换

void NavReplay_Process(void)
{
    // ... 现有逻辑 ...

    if (当前路段是直线加速段)
    {
        Control_Profile_RequestMode(CONTROL_MODE_ACCEL);
    }
    else if (当前路段是弯前减速段)
    {
        Control_Profile_RequestMode(CONTROL_MODE_BRAKE);
    }
    else
    {
        Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
    }
}
```

#### 场景 B：视觉任务触发

```c
// 例如视觉检测到弯道入口
void VisionTask_OnCurveDetected(void)
{
    Control_Profile_RequestMode(CONTROL_MODE_BRAKE);
}

// 出弯后恢复
void VisionTask_OnStraightDetected(void)
{
    Control_Profile_RequestMode(CONTROL_MODE_ACCEL);
}
```

#### 场景 C：遥控器手动切换

```c
// 在 pit0_ch1_isr() 的遥控器处理中
// 可映射某个通道到模式切换
if (robot_ctrl.channel_x > threshold)
{
    Control_Profile_RequestMode(CONTROL_MODE_ACCEL);
}
```

#### 场景 D：倒地恢复

```c
// cm7_0_isr.c 中已有倒地检测，可在此恢复
if (g_fallen && !g_fallen_last)  // 刚倒地
{
    // 已有：PID_Param_Init(), Brake_Feedforward_Reset()
    // 可加：确保恢复到 NORMAL
    Control_Profile_ApplyNow(CONTROL_MODE_NORMAL);
}
```

## 5. 扩展新预设

如需新增预设（如弯道专用、单边桥专用），按以下步骤：

### Step 1：新增枚举值

```c
// code/calculate/pid-new.h
typedef enum {
    CONTROL_MODE_NORMAL = 0U,
    CONTROL_MODE_ACCEL  = 1U,
    CONTROL_MODE_BRAKE  = 2U,
    CONTROL_MODE_CURVE  = 3U,   // ← 新增：弯道模式
} ControlMode_e;
```

### Step 2：定义预设常量

```c
// code/calculate/pid-new.c
static const ControlProfile_t g_control_profile_curve = {
    // 舵机速度环
    -5.0f, SERVO_SPEED_KI, -0.18f, 2400.0f, SERVO_SPEED_MAX_I, SERVO_SPEED_COMP,
    // 角度环
    -11.5f, ANG_KI, -14.0f, ANG_MAX_O, ANG_MAX_I, ANG_MECH_ZERO,
    // 角速度环
    GYR_KP, GYR_KI, GYR_KD, GYR_MAX_O, GYR_MAX_I, GYR_DEAD_ZONE,
    // 转向角度环
    -10.0f, TURN_ANG_KI, -2.0f, TURN_ANG_MAX_O, TURN_ANG_MAX_I, TURN_ANG_DEAD_ZONE,
    // 转向角速度环
    TURN_GYR_KP, TURN_GYR_KI, TURN_GYR_KD, 8500.0f, TURN_GYR_MAX_I, TURN_GYR_DEAD_ZONE,
    // 横滚环
    ROLL_KP, ROLL_KI, ROLL_KD, ROLL_MAX_O, ROLL_MAX_I, ROLL_MECH_ZERO,
    // 刹车前馈 gain_light/med/heavy, max_light/med/heavy, ramp_up_light/med/heavy, ramp_down
    4.5f, 11.0f, 24.0f,
    850.0f, 1700.0f, 3700.0f,
    140.0f, 340.0f, 850.0f, 850.0f,
    // 加速前馈 gain, max, ramp_up, ramp_down
    8.0f, 2400.0f, 600.0f, 750.0f,
    // 舵机执行器 acc_limit, dec_limit, boost_from_speed, boost_from_error, boost_max
    14.0f, 12.0f, 0.022f, 0.012f, 70.0f
};
```

### Step 3：注册到分发函数

```c
// code/calculate/pid-new.c → Control_Profile_GetPreset()
static const ControlProfile_t *Control_Profile_GetPreset(ControlMode_e mode)
{
    if (mode == CONTROL_MODE_ACCEL) return &g_control_profile_accel;
    if (mode == CONTROL_MODE_BRAKE) return &g_control_profile_brake;
    if (mode == CONTROL_MODE_CURVE) return &g_control_profile_curve;  // ← 新增
    return &g_control_profile_normal;
}
```

### Step 4：在合适的位置调用

```c
Control_Profile_RequestMode(CONTROL_MODE_CURVE);
```

## 6. 平滑过渡机制

`Control_Profile_Update1ms()` 每 1ms 在 ISR 中调用，使用指数平滑：

```
next = current + (target - current) * alpha
```

| 参数类型 | alpha | 收敛到 95% 约需 |
|----------|-------|-----------------|
| PID 增益 (kp/kd) | 0.12 | ~24ms |
| 前馈参数 (gain) | 0.10 | ~29ms |
| 限幅参数 (max) | 0.18 | ~16ms |

这意味着：
- 切换预设后，参数会在 **约 30ms 内基本过渡完成**
- 过渡期间车身行为是两套参数的混合，不会突变
- 如果在过渡完成前又切回，参数会平滑回退

## 7. 注意事项

1. **不要在多处同时写 `g_control_mode_requested`**：注释明确说"只在导航给就行，其他地方别写，防止冲突"
2. **ACCEL/BRAKE 参数未标定**：当前注释标注为"AI随便写的"，实车使用前必须重新调试
3. **切换不会重置 PID 运行时状态**：切换预设只改变 kp/kd/max 等参数，不清除积分项和误差历史。如需清积分，调用 `PID_Data_Reset()`
4. **刹车前馈和加速前馈是独立模块**：预设切换只改变它们的增益参数，不改变它们的使能/禁止状态，使能由 ISR 中的条件判断控制
5. **舵机执行器联动**：切换预设时会同步更新 `servo_executor` 的斜率限制参数，舵机响应速度会随模式变化
