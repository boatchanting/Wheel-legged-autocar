# Plan4 Speed Response Analysis

## Scope and method

- Input: six supplied Wi-Fi telemetry logs. Time is `loop * 1 ms`; host receive timestamps are not used for dynamics.
- Valid response samples: `g_replay_state == 1`, with minefield, bumpy-road, bridge, slope, three-stage, and special-action takeovers excluded.
- Controller feedback speed is reconstructed exactly as firmware does: `(speed_L - speed_R) / 2` rpm. `vx_body` is retained only as a sanity check because it is an inertial estimate.
- A response event is a normal-run target change of at least 75 rpm that remains approximately stable from 60 to 140 ms after the edge. Metrics are therefore evidence for step-like parts of the route, not every curved-path sample.

## Measured result

| Metric | Result |
|---|---:|
| Median normal-replay share of replay frames | 63.0% |
| Median p90 absolute wheel tracking error | 420.6 rpm (2014.5 mm/s) |
| Continuous-curve delay estimate (median) | 140.0 ms (correlation 0.59) |
| Qualified continuous acceleration windows | 51 |
| Coherent wheel-acceleration windows | 31 / 51 |
| Planned acceleration in coherent windows, median | 536.8 rpm/s (2571.3 mm/s^2) |
| Actual wheel acceleration in coherent windows, median | 478.7 rpm/s (2293.2 mm/s^2) |
| Actual/planned acceleration-rate ratio, median | 0.55 |
| Mean motor PWM in coherent acceleration windows, median | 1985.1 |
| Acceleration profile (`pid_mode = 1`) share | 0.0% |
| Qualified acceleration events | 0 |
| Median acceleration 63% time | n/a ms |
| Median acceleration 90% time | n/a ms |
| Qualified braking events | 0 |
| Median braking 90% time | n/a ms |

## Interpretation

1. Treat the event values as the effective closed-loop chassis delay. They include speed PID, leg actuator slew, balance dynamics, motor torque, and tyre-ground interaction. The continuous-curve estimate is only a provisional value when no plateaued steps are present. The planner's `SPEED_RESPONSE_DELAY_S` should use the conservative p75/p90 of a dedicated braking test, not a guessed value.
2. The 1 ms Plan4 target ramp (`110 rpm` up and `200 rpm` down per navigation update) is much faster than the measured chassis response and does not protect the plant. In the coherent rising windows, actual acceleration is only about 0.55 of planned acceleration. The other windows do not show a coherent wheel-speed rise despite a sustained rising command, which needs the longitudinal internal signals below before it is treated as a mechanical limit.
3. `vx_body` and encoder wheel speed are different signals. Tune the speed loop on encoder reconstruction, then use `vx_body` for path/odometry validation. A large body-wheel discrepancy in the CSV means slip or estimator filtering, not necessarily slow propulsion.
4. A normal-frame ratio below 100% is expected on this course: special-task state machines intentionally own the speed. Do not raise Plan4 speed or acceleration feed-forward using those frames.

## Firmware findings and optimisation order

1. **Enable the already-written acceleration path, but make it Plan4-safe.** The active configuration has `ACCEL_FF_ENABLE = 0U`, so `Accel_Feedforward_Update()` always returns zero. Also, Plan4 never requests `CONTROL_MODE_ACCEL`; the logs prove `pid_mode = 1` was never active. The first firmware change should enable the feature only for the normal Plan4 running state and request ACCEL only while the target is rising and there is a meaningful speed deficit; request NORMAL before braking, at a task handover, or at the route finish.
2. **Add a continuous-ramp trigger before enabling it.** The existing `ACCEL_FF_TARGET_STEP_MIN = 30 rpm` is tested every 9 ms, equivalent to roughly `3333 rpm/s`. The measured planned median is only about 486.3 rpm/s, so smooth Plan4 ramps rarely arm the 550 ms boost window. Keep the 30-rpm step trigger for launch, but add a separate Plan4 ramp threshold of roughly 2-3 rpm per navigation update (200-300 rpm/s) which refreshes the boost window only while target speed is genuinely rising.
3. **Instrument before raising force.** Add telemetry for `current_actual_speed`, `g_target_pwm_speed_adj`, `Accel_Feedforward_GetPwm()`, `Brake_Feedforward_GetPwm()`, `g_control_mode_applied`, and the four slew-limited servo target/current duties. Final motor PWM alone cannot distinguish PID saturation from servo slew saturation.
4. **Tune feed-forward before feedback gain.** With the new ramp trigger active, increase `ACCEL_FF_GAIN` in 10-15% steps. Increase `ACCEL_FF_RAMP_UP` only when the logged feed-forward takes too long to reach its target. Roll back on excessive pitch, wheel oscillation, or more than 10% acceleration overshoot.
5. **Then increase acceleration-mode posture authority.** The speed PID runs every 9 ms, but the leg command is slew limited at 1 ms. Only if the requested/final servo-duty log shows sustained slew error should `acc_limit`, acceleration-mode dynamic boost, or its boost maximum be raised. Brake-mode slew must remain separate and conservative.
6. **Make the path planner use measured limits.** Set `SPEED_RESPONSE_DELAY_S` from a dedicated braking p75/p90 test. For acceleration, use a route envelope at no more than 60-70% of the repeatable straight-line actual rate until the acceleration path above is tuned; the logs currently show only 0.55 response-rate tracking in coherent route windows.
7. **Validate with repeatable steps.** On a straight, no-special-task segment command 0 -> 320 -> 600 -> 800 rpm and reverse, hold each level 1.5 s, three repeats. Re-run this script and compare p50/p90 time, acceleration rate, peak pitch, PWM clipping fraction, and stopping distance. A speed increase is accepted only when those bounds remain safe.

## Code trace

- `user/cm7_0_isr.c`: navigation executes every 10 ms; speed feedback/PID/forward-feed executes every 9 ms; servo executor executes every 1 ms.
- `code/calculate/pid-new.c`: speed error produces leg-posture adjustment and acceleration/braking feed-forward is arbitrated before the final motor PWM.
- `code/servo/servo_executor.c`: the leg command is explicitly slew limited.
- `code/navigation/nav_replay/plan4/plan4_lqr_speed_planning.h`: Plan4 currently has target ramp constants of 110/200 rpm per navigation period.
- `tools/webview_nav_marker科目四/generate_plan4_smooth_path_考虑响应延迟_丝滑轨迹.py`: already accepts `--speed-response-delay-s`; feed it values identified by this analysis.
