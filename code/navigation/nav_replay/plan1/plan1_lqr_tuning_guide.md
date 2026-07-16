# Plan1 LQR 路径跟踪调参说明

这个方案不是在线 Riccati LQR。它保留项目原来的 `err_degree` 和 `target_speed_set` 接口，但反馈目标改成“贴住最近局部线段”：

```text
u = u_ff(kappa_preview, speed)
  + K_y * e_y
  + K_psi * e_psi
  + K_r * (r_ref - r_actual)
```

## 和纯追踪的差异

纯追踪会找前方预瞄点，车会天然朝远处目标点切过去，绕桩或急弯时容易切弯。现在的 LQR 只用最近路径线段的投影点和局部切线做横向/航向反馈，预瞄点只提供曲率前馈，不参与 `e_y` 和 `e_psi`。

局部投影反馈更贴线的原因很简单：车辆偏离哪一段，就按哪一段的法向距离回线；车头偏离哪一段，就按哪一段的切线对齐。它不会因为前方预瞄点已经拐进弯里，就提前把反馈目标拉到弯内。

## 参数和符号

- `LQR_SIGN`：总方向符号。实车左右整体反了，优先只改这个。
- `LQR_K_LATERAL`：横向误差增益，单位近似 deg/mm。本项目 `x` 向后为正、`y` 向右为正；沿路径前进时，车在路径右侧定义为 `e_y > 0`，正 `err_degree` 负责向左修回线。大偏差回线不够，就先加它。
- `LQR_K_HEADING`：局部切线航向误差增益。车头对不准线，就调它。
- `LQR_K_YAW_RATE_FF`：曲率前馈增益，输入是 `r_ref = abs(target_speed) * LQR_SPEED_TO_MM_S * kappa_preview`。
- `LQR_K_YAW_RATE`：实际 yaw-rate 反馈增益，默认 0。确认 `inertial_nav.actual_yaw_rate` 符号和噪声可靠后，再打开 `LQR_USE_ACTUAL_YAW_RATE_FB`。
- `LQR_FORWARD_SPEED_IS_NEGATIVE`：本项目默认负 `target_speed` 表示前进。
- `LQR_LOW/HIGH_SPEED_*`：速度相关限幅、slew、低通和横向加速度/yaw-rate 包络。

## 推荐调参顺序

1. 低速，把 `LQR_K_YAW_RATE_FF` 暂时调小或置 0，只看反馈方向。
2. 车偏左/偏右时如果修反，改 `LQR_SIGN`；不要在多个公式里分散加负号。
3. 调 `LQR_K_HEADING`，先让车头能顺着局部路径切线。
4. 调 `LQR_K_LATERAL`，让车能回到路径中心线。
5. 恢复并调 `LQR_K_YAW_RATE_FF`，让入弯不滞后，但不能把车提前拉进弯内。
6. 最后再加速度。速度上来后优先收紧高速限幅、横向加速度和 yaw 加速度约束。

## 低速和高速分别看什么

低速、大横向误差：优先调 `LQR_K_LATERAL`、`LQR_LOW_SPEED_ERR_MAX_DEG`、`LQR_LOW_SPEED_ERR_SLEW_DEG`。低速需要有足够纠偏幅度和变化率，否则只会慢慢漂回线。

高速、绕桩抽搐：优先降 `LQR_HIGH_SPEED_ERR_SLEW_DEG` 或 `LQR_HIGH_SPEED_FILTER_ALPHA`，再看 `LQR_HIGH_SPEED_YAW_RATE_LIMIT_RAD_S`、`LQR_HIGH_SPEED_YAW_ACCEL_LIMIT_RAD_S2`、`LQR_HIGH_SPEED_LATERAL_ACCEL_MM_S2`。高速不要靠固定小角度硬砍，而要按机体姿态、横向加速度和 yaw 动态能力收住。

急弯切弯：先确认路径曲率和安全距离没有问题，再调 `LQR_K_YAW_RATE_FF` 和 `LQR_PREVIEW_POINTS`。记住预瞄只负责提前建立转向，不改变贴局部线的反馈目标。
