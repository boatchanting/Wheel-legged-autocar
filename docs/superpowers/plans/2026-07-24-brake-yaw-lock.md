# Accel/Brake Yaw Lock Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep heading locked for the whole accel/brake host speed-test state machine so wheel speed asymmetry does not turn the vehicle.

**Architecture:** Add a latched yaw reference in `user/cm7_0_isr.c` that is armed when `WifiHostSpeedTest` becomes active and released when it ends. While that host state is active, feed the yaw error into the normal turn angle loop and turn gyro loop so yaw hold is enforced through accel, cruise, and brake.

**Tech Stack:** C, existing PID helpers in `code/calculate/pid-new.c`, existing ISR control flow in `user/cm7_0_isr.c`, CSV log verification via the accel/brake collector.

---

### Task 1: Add host yaw-hold state

**Files:**
- Modify: `user/cm7_0_isr.c`

- [ ] **Step 1: Add a latched yaw reference and active flag near the existing control-state globals.**

```c
static float wifi_host_yaw_hold_ref = 0.0f;
static uint8 wifi_host_yaw_hold_active = 0U;
```

- [ ] **Step 2: Add a small helper that normalizes yaw error into `[-180, 180]`.**

```c
static float WifiHostYawHold_WrapErrorDeg(float yaw_error)
{
    yaw_error = fmodf(yaw_error, 360.0f);
    if (yaw_error > 180.0f) yaw_error -= 360.0f;
    if (yaw_error < -180.0f) yaw_error += 360.0f;
    return yaw_error;
}
```

### Task 2: Route host yaw hold into turn control

**Files:**
- Modify: `user/cm7_0_isr.c`

- [ ] **Step 1: Latch current yaw when host speed test becomes active, clear the latch otherwise.**

```c
if ((WifiHostSpeedTest_IsActive() != 0U) && (g_yaw_initialized != 0U))
{
    if (wifi_host_yaw_hold_active == 0U)
    {
        wifi_host_yaw_hold_ref = euler_angle.yaw;
    }
    wifi_host_yaw_hold_active = 1U;
}
else
{
    wifi_host_yaw_hold_active = 0U;
}
```

- [ ] **Step 2: Feed the held yaw error into the turn outer loop whenever host speed test is active.**

```c
if ((wifi_host_yaw_hold_active != 0U) && (g_yaw_initialized != 0U))
{
    float yaw_error = WifiHostYawHold_WrapErrorDeg(wifi_host_yaw_hold_ref - euler_angle.yaw);
    err_degree = yaw_error;
    turn_angle_loop_out = Turn_Angle_Loop_Control(yaw_error);
}
else if (WifiHostSpeedTest_IsActive() != 0U)
{
    err_degree = 0.0f;
    turn_angle_loop_out = 0.0f;
}
```

- [ ] **Step 3: Keep the turn gyro loop alive while host yaw hold is active.**

```c
if ((wifi_host_yaw_hold_active != 0U) && (g_yaw_initialized != 0U))
{
    turn_gyro_loop_out = Turn_Gyro_Loop_Control(turn_angle_loop_out, filtered_gyro_z);
}
else if (WifiHostSpeedTest_IsActive() != 0U)
{
    turn_gyro_loop_out = 0.0f;
}
```

### Task 3: Verify with logs

**Files:**
- None

- [ ] **Step 1: Run the existing accel/brake collector again with the same test cases.**
- [ ] **Step 2: Compare host-active `att_yaw`, `relative_yaw`, `speed_L`, `speed_R`, `theoretical_yaw_rate`, and `actual_yaw_rate` against the previous logs.**
- [ ] **Step 3: Confirm yaw drift is smaller through both accel and brake, and that left/right wheel speed divergence is reduced.**

