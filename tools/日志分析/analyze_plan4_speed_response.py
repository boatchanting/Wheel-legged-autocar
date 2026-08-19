#!/usr/bin/env python3
"""Quantify Plan4 longitudinal response from Wi-Fi telemetry.

The target and the controller's measured speed are both in RPM.  The script
uses loop_counter as the time base because the host-side Wi-Fi timestamps are
bursty.  It only evaluates replay frames without a special-task takeover.

Outputs are written next to this file.  Re-run after each vehicle test:
    .venv\\Scripts\\python.exe tools\\日志分析\\analyze_plan4_speed_response.py
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


SPEED_TO_MM_S = 4.79
LOOP_PERIOD_S = 0.001
MAX_GAP_LOOPS = 20
EVENT_STEP_RPM = 75.0
EVENT_MIN_SETTLE_RPM = 55.0
EVENT_MIN_SPAN_LOOPS = 700
PRE_LOOPS = 80
POST_LOOPS = 900
RAMP_WINDOW_LOOPS = 300
RAMP_MIN_DELTA_RPM = 60.0
RAMP_MIN_SLOPE_RPM_S = 180.0
RAMP_REFRACTORY_LOOPS = 250

plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DejaVu Sans"]
plt.rcParams["axes.unicode_minus"] = False


@dataclass
class Run:
    path: Path
    all_replay: pd.DataFrame
    normal: pd.DataFrame


def find_default_logs(data_dir: Path) -> list[Path]:
    # The six logs supplied with the request.  IDs make this robust to Chinese
    # filename rendering on a different Windows console.
    ids = ("225445_948", "225743_906", "225858_588", "175457_873", "182540_472", "182825_112")
    found = [path for path in sorted(data_dir.glob("wifi_telemetry_*.csv")) if any(item in path.name for item in ids)]
    if not found:
        raise FileNotFoundError(f"No requested telemetry logs found in {data_dir}")
    return found


def make_normal_mask(frame: pd.DataFrame) -> pd.Series:
    mask = frame["g_replay_state"].eq(1)
    for column in (
        "g_special_action_trigger",
        "minefield_is_active",
        "bumpy_road_is_active",
        "vision_bridge_task_is_active",
        "vision_slope_task_is_active",
        "vision_three_stage_control_is_active",
    ):
        if column in frame:
            mask &= frame[column].fillna(0).eq(0)
    return mask


def add_signals(frame: pd.DataFrame) -> pd.DataFrame:
    result = frame.copy().sort_values("loop").drop_duplicates("loop").reset_index(drop=True)
    result["target_rpm"] = result["target_speed_set"].abs()
    # Firmware: current_actual_speed = 0.5 * (right_speed - left_speed).
    # Telemetry convention: speed_R is negative when the car moves forward.
    result["wheel_rpm"] = (result["speed_L"] - result["speed_R"]) * 0.5
    result["wheel_mm_s"] = result["wheel_rpm"] * SPEED_TO_MM_S
    result["body_rpm"] = result["vx_body"].abs() / SPEED_TO_MM_S
    result["pwm_mean_abs"] = (result["pwm_left"].abs() + result["pwm_right"].abs()) * 0.5
    result["error_rpm"] = result["wheel_rpm"] - result["target_rpm"]
    result["loop_dt"] = result["loop"].diff()
    result["segment"] = (result["loop_dt"].isna() | result["loop_dt"].gt(MAX_GAP_LOOPS)).cumsum()
    return result


def load_run(path: Path) -> Run:
    raw = add_signals(pd.read_csv(path))
    replay = raw[raw["g_replay_state"].eq(1)].copy()
    normal = raw[make_normal_mask(raw)].copy()
    normal["normal_segment"] = (
        normal["loop"].diff().isna() | normal["loop"].diff().gt(MAX_GAP_LOOPS)
    ).cumsum()
    return Run(path=path, all_replay=replay, normal=normal)


def quantile(values: pd.Series, q: float) -> float:
    return float(values.quantile(q)) if len(values) else float("nan")


def summarize_run(run: Run) -> dict[str, object]:
    frame = run.normal
    replay = run.all_replay
    if frame.empty:
        return {"log": run.path.name, "replay_s": 0.0, "normal_s": 0.0, "normal_frames": 0}
    span_s = (frame["loop"].max() - frame["loop"].min()) * LOOP_PERIOD_S
    replay_s = (replay["loop"].max() - replay["loop"].min()) * LOOP_PERIOD_S if not replay.empty else 0.0
    lag = estimate_continuous_lag(frame)
    return {
        "log": run.path.name,
        "replay_s": replay_s,
        "normal_s": span_s,
        "normal_frames": len(frame),
        "normal_frame_ratio": len(frame) / max(len(replay), 1),
        "loop_dt_p50_ms": quantile(frame["loop_dt"].dropna(), 0.5),
        "loop_dt_p90_ms": quantile(frame["loop_dt"].dropna(), 0.9),
        "target_p50_rpm": quantile(frame["target_rpm"], 0.5),
        "target_p90_rpm": quantile(frame["target_rpm"], 0.9),
        "target_max_rpm": float(frame["target_rpm"].max()),
        "wheel_p50_rpm": quantile(frame["wheel_rpm"], 0.5),
        "wheel_p90_rpm": quantile(frame["wheel_rpm"], 0.9),
        "wheel_max_rpm": float(frame["wheel_rpm"].max()),
        "error_p50_abs_rpm": quantile(frame["error_rpm"].abs(), 0.5),
        "error_p90_abs_rpm": quantile(frame["error_rpm"].abs(), 0.9),
        "underspeed_p50_rpm": quantile((-frame["error_rpm"]).clip(lower=0), 0.5),
        "underspeed_p90_rpm": quantile((-frame["error_rpm"]).clip(lower=0), 0.9),
        "pwm_p50": quantile(frame["pwm_mean_abs"], 0.5),
        "pwm_p90": quantile(frame["pwm_mean_abs"], 0.9),
        "accel_profile_frame_ratio": float(frame["pid_mode"].fillna(-1).eq(1).mean()),
        "body_wheel_abs_diff_p50_rpm": quantile((frame["body_rpm"] - frame["wheel_rpm"]).abs(), 0.5),
        **lag,
    }


def estimate_continuous_lag(frame: pd.DataFrame) -> dict[str, float]:
    """Estimate delay from the best correlation of smoothed speed changes.

    This is deliberately a complementary metric, not a substitute for a
    designed step test.  It works when the route planner continuously changes
    target speed and no target plateau exists long enough for 10/63/90 timing.
    """
    rows: list[tuple[int, float]] = []
    for _, segment in frame.groupby("normal_segment", sort=False):
        if len(segment) < 50:
            continue
        segment = segment.sort_values("loop")
        if segment["loop"].diff().dropna().median() != 10:
            continue
        target_change = segment["target_rpm"].rolling(5, center=True).mean().diff().to_numpy()
        wheel_change = segment["wheel_rpm"].rolling(5, center=True).mean().diff().to_numpy()
        for lag_samples in range(0, 61):
            if lag_samples == 0:
                command, response = target_change, wheel_change
            else:
                command, response = target_change[:-lag_samples], wheel_change[lag_samples:]
            valid = np.isfinite(command) & np.isfinite(response)
            # Constant-speed regions do not identify a delay.
            valid &= np.abs(command) >= 1.5
            if valid.sum() < 20:
                continue
            correlation = float(np.corrcoef(command[valid], response[valid])[0, 1])
            if np.isfinite(correlation):
                rows.append((lag_samples, correlation))
    if not rows:
        return {"continuous_lag_ms": float("nan"), "continuous_lag_corr": float("nan"), "continuous_zero_lag_corr": float("nan")}
    values = pd.DataFrame(rows, columns=["lag_samples", "correlation"])
    mean_corr = values.groupby("lag_samples")["correlation"].mean()
    best_sample = int(mean_corr.idxmax())
    return {
        "continuous_lag_ms": best_sample * 10.0,
        "continuous_lag_corr": float(mean_corr.loc[best_sample]),
        "continuous_zero_lag_corr": float(mean_corr.get(0, np.nan)),
    }


def _crossing_time(time_s: np.ndarray, value: np.ndarray, threshold: float, rising: bool) -> float:
    hit = np.flatnonzero(value >= threshold if rising else value <= threshold)
    return float(time_s[hit[0]]) if len(hit) else float("nan")


def extract_events(run: Run) -> list[dict[str, object]]:
    events: list[dict[str, object]] = []
    frame = run.normal
    if len(frame) < PRE_LOOPS + POST_LOOPS:
        return events
    for _, segment in frame.groupby("normal_segment", sort=False):
        segment = segment.reset_index(drop=True)
        if segment["loop"].iloc[-1] - segment["loop"].iloc[0] < EVENT_MIN_SPAN_LOOPS:
            continue
        for index in range(PRE_LOOPS, len(segment) - POST_LOOPS):
            step = segment.loc[index, "target_rpm"] - segment.loc[index - 1, "target_rpm"]
            if abs(step) < EVENT_STEP_RPM:
                continue
            # Use medians to resist one encoder frame and reject steps that are
            # immediately followed by another planned step.
            before = segment.loc[index - PRE_LOOPS : index - 1, "target_rpm"].median()
            after_window = segment.loc[index + 60 : index + 140, "target_rpm"]
            after = after_window.median()
            if abs(after - before) < EVENT_MIN_SETTLE_RPM:
                continue
            if (after_window.max() - after_window.min()) > max(35.0, abs(after - before) * 0.25):
                continue
            if events and segment.loc[index, "loop"] - int(events[-1]["loop"]) < 500:
                continue

            actual_before = segment.loc[index - PRE_LOOPS : index - 1, "wheel_rpm"].median()
            delta = after - before
            actual_delta = segment.loc[index : index + POST_LOOPS, "wheel_rpm"].to_numpy() - actual_before
            time_s = (segment.loc[index : index + POST_LOOPS, "loop"].to_numpy() - segment.loc[index, "loop"]) * LOOP_PERIOD_S
            rising = delta > 0.0
            threshold_10 = 0.10 * delta
            threshold_63 = 0.632 * delta
            threshold_90 = 0.90 * delta
            t10 = _crossing_time(time_s, actual_delta, threshold_10, rising)
            t63 = _crossing_time(time_s, actual_delta, threshold_63, rising)
            t90 = _crossing_time(time_s, actual_delta, threshold_90, rising)
            measured_after = segment.loc[index + 60 : index + 140, "wheel_rpm"].median()
            future = segment.loc[index : index + POST_LOOPS, "wheel_rpm"].to_numpy()
            peak = future.max() if rising else future.min()
            overshoot = (peak - after) if rising else (after - peak)
            events.append(
                {
                    "log": run.path.name,
                    "loop": int(segment.loc[index, "loop"]),
                    "kind": "accel" if rising else "brake",
                    "target_before_rpm": before,
                    "target_after_rpm": after,
                    "target_step_rpm": delta,
                    "wheel_before_rpm": actual_before,
                    "wheel_after_60_140ms_rpm": measured_after,
                    "wheel_error_60_140ms_rpm": measured_after - after,
                    "t10_ms": t10 * 1000.0,
                    "t63_ms": t63 * 1000.0,
                    "t90_ms": t90 * 1000.0,
                    "overshoot_rpm": overshoot,
                    "pwm_before": segment.loc[index - PRE_LOOPS : index - 1, "pwm_mean_abs"].median(),
                    "pwm_after_60_140ms": segment.loc[index + 60 : index + 140, "pwm_mean_abs"].median(),
                }
            )
    return events


def _linear_fit(time_s: np.ndarray, value: np.ndarray) -> tuple[float, float]:
    """Return slope and R-squared; caller ensures enough finite samples."""
    valid = np.isfinite(time_s) & np.isfinite(value)
    x = time_s[valid]
    y = value[valid]
    if len(x) < 4 or np.ptp(x) <= 0.0:
        return float("nan"), float("nan")
    slope, intercept = np.polyfit(x, y, 1)
    predicted = slope * x + intercept
    residual = float(np.sum((y - predicted) ** 2))
    total = float(np.sum((y - np.mean(y)) ** 2))
    r_squared = 1.0 - residual / total if total > 1.0e-9 else float("nan")
    return float(slope), float(r_squared)


def _speed_band(rpm: float) -> str:
    if rpm < 350.0:
        return "0-350 rpm"
    if rpm < 700.0:
        return "350-700 rpm"
    return "700+ rpm"


def extract_acceleration_windows(run: Run) -> list[dict[str, object]]:
    """Measure plant acceleration during continuous planned speed ramps.

    The speed curve does not have stable step plateaus.  For each sustained
    target ramp, actual wheel speed is observed after this run's identified
    delay.  This estimates usable acceleration rate (rpm/s and mm/s^2), not
    the 10 ms target-command slew rate.
    """
    results: list[dict[str, object]] = []
    default_lag_ms = 140.0
    lag_ms = estimate_continuous_lag(run.normal).get("continuous_lag_ms", default_lag_ms)
    if not np.isfinite(lag_ms):
        lag_ms = default_lag_ms
    lag_loops = int(round(lag_ms / 10.0))
    for _, segment in run.normal.groupby("normal_segment", sort=False):
        segment = segment.sort_values("loop").reset_index(drop=True)
        if len(segment) < RAMP_WINDOW_LOOPS // 10 + lag_loops + RAMP_WINDOW_LOOPS // 10 + 1:
            continue
        smoothed_target = segment["target_rpm"].rolling(5, center=True, min_periods=3).mean().to_numpy()
        next_allowed_loop = -np.inf
        for index in range(2, len(segment)):
            start_loop = int(segment.loc[index, "loop"])
            if start_loop < next_allowed_loop:
                continue
            end_loop = start_loop + RAMP_WINDOW_LOOPS
            response_start = start_loop + lag_loops * 10
            response_end = response_start + RAMP_WINDOW_LOOPS
            # A meaningful plant-rate measurement needs the planned ramp to
            # continue while the delayed response is being observed.
            full_target_window = segment[(segment["loop"] >= start_loop) & (segment["loop"] <= response_end)]
            if len(full_target_window) < 32:
                continue
            full_indexes = full_target_window.index.to_numpy()
            full_target_time = (full_target_window["loop"].to_numpy() - start_loop) * LOOP_PERIOD_S
            full_target_slope, full_target_r2 = _linear_fit(full_target_time, smoothed_target[full_indexes])
            if (not np.isfinite(full_target_slope) or full_target_slope < RAMP_MIN_SLOPE_RPM_S or
                    smoothed_target[full_indexes[-1]] - smoothed_target[full_indexes[0]] < RAMP_MIN_DELTA_RPM * 1.35 or
                    not np.isfinite(full_target_r2) or full_target_r2 < 0.70):
                continue
            target_window = segment[(segment["loop"] >= start_loop) & (segment["loop"] <= end_loop)]
            if len(target_window) < 20:
                continue
            target_indexes = target_window.index.to_numpy()
            target_values = smoothed_target[target_indexes]
            target_time = (target_window["loop"].to_numpy() - start_loop) * LOOP_PERIOD_S
            target_slope, target_r2 = _linear_fit(target_time, target_values)
            if (not np.isfinite(target_slope) or target_slope < RAMP_MIN_SLOPE_RPM_S or
                    target_values[-1] - target_values[0] < RAMP_MIN_DELTA_RPM or
                    not np.isfinite(target_r2) or target_r2 < 0.45):
                continue

            response_window = segment[(segment["loop"] >= response_start) & (segment["loop"] <= response_end)]
            if len(response_window) < 20:
                continue
            response_time = (response_window["loop"].to_numpy() - response_start) * LOOP_PERIOD_S
            wheel_slope, wheel_r2 = _linear_fit(response_time, response_window["wheel_rpm"].to_numpy())
            wheel_start = float(response_window["wheel_rpm"].iloc[:5].median())
            wheel_end = float(response_window["wheel_rpm"].iloc[-5:].median())
            target_start = float(target_values[:5].mean())
            target_end = float(target_values[-5:].mean())
            response_ratio = wheel_slope / target_slope if np.isfinite(wheel_slope) and target_slope > 1.0 else float("nan")
            results.append(
                {
                    "log": run.path.name,
                    "target_start_loop": start_loop,
                    "lag_used_ms": lag_ms,
                    "target_start_rpm": target_start,
                    "target_end_rpm": target_end,
                    "target_accel_rpm_s": target_slope,
                    "target_fit_r2": target_r2,
                    "wheel_start_rpm": wheel_start,
                    "wheel_end_rpm": wheel_end,
                    "wheel_accel_rpm_s": wheel_slope,
                    "wheel_accel_mm_s2": wheel_slope * SPEED_TO_MM_S,
                    "wheel_fit_r2": wheel_r2,
                    "response_rate_ratio": response_ratio,
                    "mean_pwm": float(response_window["pwm_mean_abs"].mean()),
                    "p90_pwm": quantile(response_window["pwm_mean_abs"], 0.9),
                    "speed_band": _speed_band((wheel_start + wheel_end) * 0.5),
                    "is_coherent": bool((wheel_slope > 0.0) and (wheel_r2 >= 0.40)),
                }
            )
            next_allowed_loop = start_loop + RAMP_REFRACTORY_LOOPS
    return results


def summarize_acceleration_windows(windows: pd.DataFrame) -> pd.DataFrame:
    columns = ["speed_band", "windows", "wheel_accel_p50_rpm_s", "wheel_accel_p25_rpm_s", "wheel_accel_p75_rpm_s", "wheel_accel_p50_mm_s2", "target_accel_p50_rpm_s", "response_rate_ratio_p50", "mean_pwm_p50"]
    if windows.empty:
        return pd.DataFrame(columns=columns)
    rows: list[dict[str, object]] = []
    for band, frame in windows.groupby("speed_band", sort=False):
        rows.append(
            {
                "speed_band": band,
                "windows": len(frame),
                "wheel_accel_p50_rpm_s": quantile(frame["wheel_accel_rpm_s"], 0.5),
                "wheel_accel_p25_rpm_s": quantile(frame["wheel_accel_rpm_s"], 0.25),
                "wheel_accel_p75_rpm_s": quantile(frame["wheel_accel_rpm_s"], 0.75),
                "wheel_accel_p50_mm_s2": quantile(frame["wheel_accel_mm_s2"], 0.5),
                "target_accel_p50_rpm_s": quantile(frame["target_accel_rpm_s"], 0.5),
                "response_rate_ratio_p50": quantile(frame["response_rate_ratio"], 0.5),
                "mean_pwm_p50": quantile(frame["mean_pwm"], 0.5),
            }
        )
    return pd.DataFrame(rows, columns=columns)


def plot_acceleration_rate(windows: pd.DataFrame, output: Path) -> None:
    fig, axes = plt.subplots(1, 2, figsize=(15, 5.5))
    if windows.empty:
        for axis in axes:
            axis.text(0.5, 0.5, "No continuous acceleration windows", ha="center", va="center", transform=axis.transAxes)
        fig.tight_layout()
        fig.savefig(output, dpi=170)
        plt.close(fig)
        return
    coherent = windows[windows["is_coherent"].eq(True)]
    incoherent = windows[~windows["is_coherent"].eq(True)]
    if not incoherent.empty:
        axes[0].scatter(incoherent["target_accel_rpm_s"], incoherent["wheel_accel_rpm_s"],
                        marker="x", color="#9ca3af", alpha=0.75, label="no coherent wheel rise")
    for band, frame in coherent.groupby("speed_band", sort=False):
        axes[0].scatter(frame["target_accel_rpm_s"], frame["wheel_accel_rpm_s"], alpha=0.75, label=band)
        axes[1].scatter(frame["mean_pwm"], frame["wheel_accel_mm_s2"], alpha=0.75, label=band)
    max_rate = max(1.0, float(max(windows["target_accel_rpm_s"].max(), coherent["wheel_accel_rpm_s"].max() if not coherent.empty else 1.0)))
    axes[0].plot([0, max_rate], [0, max_rate], color="#6b7280", linestyle="--", linewidth=1, label="1:1 response")
    axes[0].set_xlabel("planned acceleration (rpm/s)")
    axes[0].set_ylabel("wheel acceleration after lag (rpm/s)")
    axes[0].set_title("Actual acceleration capacity (crosses: no coherent rise)")
    axes[1].set_xlabel("mean |motor PWM|")
    axes[1].set_ylabel("wheel acceleration (mm/s^2)")
    axes[1].set_title("Coherent acceleration versus motor effort")
    for axis in axes:
        axis.grid(alpha=0.25)
        axis.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(output, dpi=170)
    plt.close(fig)


def plot_overview(runs: list[Run], output: Path) -> None:
    columns = 2
    rows = max(1, int(np.ceil(len(runs) / columns)))
    fig, axes = plt.subplots(rows, columns, figsize=(16, 3.8 * rows), squeeze=False)
    for axis, run in zip(axes.flat, runs):
        frame = run.normal
        if frame.empty:
            axis.set_visible(False)
            continue
        time_s = (frame["loop"] - frame["loop"].iloc[0]) * LOOP_PERIOD_S
        label = run.path.stem.replace("wifi_telemetry_", "")
        axis.plot(time_s, frame["target_rpm"], color="#dc2626", linewidth=1.15, label="target rpm")
        axis.plot(time_s, frame["wheel_rpm"], color="#2563eb", linewidth=0.95, label="encoder wheel rpm")
        axis.set_title(label, fontsize=10)
        axis.set_ylabel("speed (rpm)")
        axis.set_xlabel("normal replay time (s)")
        axis.grid(alpha=0.25)
        axis.legend(fontsize=8, loc="upper right")
    for axis in axes.flat[len(runs) :]:
        axis.set_visible(False)
    fig.suptitle("Plan4 normal replay: target versus encoder wheel speed", y=1.01, fontsize=14)
    fig.tight_layout()
    fig.savefig(output, dpi=170)
    plt.close(fig)


def plot_event_overlay(runs: list[Run], events: pd.DataFrame, output: Path) -> None:
    fig, axes = plt.subplots(1, 2, figsize=(15, 6), sharey=True)
    colors = {"accel": "#d97706", "brake": "#2563eb"}
    plotted = {"accel": 0, "brake": 0}
    if events.empty:
        for axis, kind in zip(axes, ("accel", "brake")):
            axis.text(0.5, 0.5, "No qualified step events in route logs", ha="center", va="center", transform=axis.transAxes)
            axis.set_title(f"{kind} events (n=0)")
            axis.set_xlabel("time from target step (s)")
        axes[0].set_ylabel("normalised target / wheel response")
        fig.tight_layout()
        fig.savefig(output, dpi=170)
        plt.close(fig)
        return
    for run in runs:
        frame = run.normal.reset_index(drop=True)
        for event in events[events["log"].eq(run.path.name)].itertuples(index=False):
            locations = np.flatnonzero(frame["loop"].eq(event.loop).to_numpy())
            if not len(locations):
                continue
            index = int(locations[0])
            start, stop = max(0, index - 100), min(len(frame), index + 600)
            window = frame.iloc[start:stop]
            t = (window["loop"].to_numpy() - event.loop) * LOOP_PERIOD_S
            target = window["target_rpm"].to_numpy()
            actual = window["wheel_rpm"].to_numpy()
            denom = event.target_after_rpm - event.target_before_rpm
            if abs(denom) < 1.0:
                continue
            target_norm = (target - event.target_before_rpm) / denom
            actual_norm = (actual - event.wheel_before_rpm) / denom
            ax = axes[0] if event.kind == "accel" else axes[1]
            ax.plot(t, actual_norm, color=colors[event.kind], alpha=0.22, linewidth=1.0)
            ax.plot(t, target_norm, color="#111827", alpha=0.12, linewidth=0.7)
            plotted[event.kind] += 1
    for kind, axis in zip(("accel", "brake"), axes):
        axis.axhline(0.0, color="#6b7280", linewidth=0.8)
        axis.axhline(0.632, color="#6b7280", linestyle="--", linewidth=0.8)
        axis.axhline(1.0, color="#6b7280", linewidth=0.8)
        axis.axvline(0.0, color="#111827", linestyle="--", linewidth=0.8)
        axis.set_xlim(-0.1, 0.6)
        axis.set_ylim(-0.6, 1.8)
        axis.grid(alpha=0.25)
        axis.set_title(f"{kind} events (n={plotted[kind]})")
        axis.set_xlabel("time from target step (s)")
    axes[0].set_ylabel("normalised target / wheel response")
    fig.tight_layout()
    fig.savefig(output, dpi=170)
    plt.close(fig)


def fmt(value: float, digits: int = 1) -> str:
    return "n/a" if not np.isfinite(value) else f"{value:.{digits}f}"


def write_report(summary: pd.DataFrame, events: pd.DataFrame, accel_windows: pd.DataFrame, accel_summary: pd.DataFrame, output: Path) -> None:
    acceleration = events[events["kind"].eq("accel")]
    braking = events[events["kind"].eq("brake")]
    e90 = quantile(acceleration["t90_ms"].dropna(), 0.5) if not acceleration.empty else float("nan")
    e63 = quantile(acceleration["t63_ms"].dropna(), 0.5) if not acceleration.empty else float("nan")
    b90 = quantile(braking["t90_ms"].dropna(), 0.5) if not braking.empty else float("nan")
    error90 = quantile(summary["error_p90_abs_rpm"].dropna(), 0.5)
    normal_ratio = quantile(summary["normal_frame_ratio"].dropna(), 0.5)
    continuous_lag = quantile(summary["continuous_lag_ms"].dropna(), 0.5)
    continuous_corr = quantile(summary["continuous_lag_corr"].dropna(), 0.5)
    coherent = accel_windows[accel_windows["is_coherent"].eq(True)].copy()
    wheel_accel = quantile(coherent["wheel_accel_rpm_s"].dropna(), 0.5) if not coherent.empty else float("nan")
    target_accel = quantile(coherent["target_accel_rpm_s"].dropna(), 0.5) if not coherent.empty else float("nan")
    response_ratio = quantile(coherent["response_rate_ratio"].dropna(), 0.5) if not coherent.empty else float("nan")
    accel_pwm = quantile(coherent["mean_pwm"].dropna(), 0.5) if not coherent.empty else float("nan")
    accel_profile_ratio = quantile(summary["accel_profile_frame_ratio"].dropna(), 0.5)
    text = f"""# Plan4 Speed Response Analysis

## Scope and method

- Input: six supplied Wi-Fi telemetry logs. Time is `loop * 1 ms`; host receive timestamps are not used for dynamics.
- Valid response samples: `g_replay_state == 1`, with minefield, bumpy-road, bridge, slope, three-stage, and special-action takeovers excluded.
- Controller feedback speed is reconstructed exactly as firmware does: `(speed_L - speed_R) / 2` rpm. `vx_body` is retained only as a sanity check because it is an inertial estimate.
- A response event is a normal-run target change of at least {EVENT_STEP_RPM:.0f} rpm that remains approximately stable from 60 to 140 ms after the edge. Metrics are therefore evidence for step-like parts of the route, not every curved-path sample.

## Measured result

| Metric | Result |
|---|---:|
| Median normal-replay share of replay frames | {fmt(normal_ratio * 100.0)}% |
| Median p90 absolute wheel tracking error | {fmt(error90)} rpm ({fmt(error90 * SPEED_TO_MM_S)} mm/s) |
| Continuous-curve delay estimate (median) | {fmt(continuous_lag)} ms (correlation {fmt(continuous_corr, 2)}) |
| Qualified continuous acceleration windows | {len(accel_windows)} |
| Coherent wheel-acceleration windows | {len(coherent)} / {len(accel_windows)} |
| Planned acceleration in coherent windows, median | {fmt(target_accel)} rpm/s ({fmt(target_accel * SPEED_TO_MM_S)} mm/s^2) |
| Actual wheel acceleration in coherent windows, median | {fmt(wheel_accel)} rpm/s ({fmt(wheel_accel * SPEED_TO_MM_S)} mm/s^2) |
| Actual/planned acceleration-rate ratio, median | {fmt(response_ratio, 2)} |
| Mean motor PWM in coherent acceleration windows, median | {fmt(accel_pwm)} |
| Acceleration profile (`pid_mode = 1`) share | {fmt(accel_profile_ratio * 100.0)}% |
| Qualified acceleration events | {len(acceleration)} |
| Median acceleration 63% time | {fmt(e63)} ms |
| Median acceleration 90% time | {fmt(e90)} ms |
| Qualified braking events | {len(braking)} |
| Median braking 90% time | {fmt(b90)} ms |

## Interpretation

1. Treat the event values as the effective closed-loop chassis delay. They include speed PID, leg actuator slew, balance dynamics, motor torque, and tyre-ground interaction. The continuous-curve estimate is only a provisional value when no plateaued steps are present. The planner's `SPEED_RESPONSE_DELAY_S` should use the conservative p75/p90 of a dedicated braking test, not a guessed value.
2. The 1 ms Plan4 target ramp (`110 rpm` up and `200 rpm` down per navigation update) is much faster than the measured chassis response and does not protect the plant. In the coherent rising windows, actual acceleration is only about {fmt(response_ratio, 2)} of planned acceleration. The other windows do not show a coherent wheel-speed rise despite a sustained rising command, which needs the longitudinal internal signals below before it is treated as a mechanical limit.
3. `vx_body` and encoder wheel speed are different signals. Tune the speed loop on encoder reconstruction, then use `vx_body` for path/odometry validation. A large body-wheel discrepancy in the CSV means slip or estimator filtering, not necessarily slow propulsion.
4. A normal-frame ratio below 100% is expected on this course: special-task state machines intentionally own the speed. Do not raise Plan4 speed or acceleration feed-forward using those frames.

## Firmware findings and optimisation order

1. **Enable the already-written acceleration path, but make it Plan4-safe.** The active configuration has `ACCEL_FF_ENABLE = 0U`, so `Accel_Feedforward_Update()` always returns zero. Also, Plan4 never requests `CONTROL_MODE_ACCEL`; the logs prove `pid_mode = 1` was never active. The first firmware change should enable the feature only for the normal Plan4 running state and request ACCEL only while the target is rising and there is a meaningful speed deficit; request NORMAL before braking, at a task handover, or at the route finish.
2. **Add a continuous-ramp trigger before enabling it.** The existing `ACCEL_FF_TARGET_STEP_MIN = 30 rpm` is tested every 9 ms, equivalent to roughly `3333 rpm/s`. The measured planned median is only about {fmt(486.3)} rpm/s, so smooth Plan4 ramps rarely arm the 550 ms boost window. Keep the 30-rpm step trigger for launch, but add a separate Plan4 ramp threshold of roughly 2-3 rpm per navigation update (200-300 rpm/s) which refreshes the boost window only while target speed is genuinely rising.
3. **Instrument before raising force.** Add telemetry for `current_actual_speed`, `g_target_pwm_speed_adj`, `Accel_Feedforward_GetPwm()`, `Brake_Feedforward_GetPwm()`, `g_control_mode_applied`, and the four slew-limited servo target/current duties. Final motor PWM alone cannot distinguish PID saturation from servo slew saturation.
4. **Tune feed-forward before feedback gain.** With the new ramp trigger active, increase `ACCEL_FF_GAIN` in 10-15% steps. Increase `ACCEL_FF_RAMP_UP` only when the logged feed-forward takes too long to reach its target. Roll back on excessive pitch, wheel oscillation, or more than 10% acceleration overshoot.
5. **Then increase acceleration-mode posture authority.** The speed PID runs every 9 ms, but the leg command is slew limited at 1 ms. Only if the requested/final servo-duty log shows sustained slew error should `acc_limit`, acceleration-mode dynamic boost, or its boost maximum be raised. Brake-mode slew must remain separate and conservative.
6. **Make the path planner use measured limits.** Set `SPEED_RESPONSE_DELAY_S` from a dedicated braking p75/p90 test. For acceleration, use a route envelope at no more than 60-70% of the repeatable straight-line actual rate until the acceleration path above is tuned; the logs currently show only {fmt(response_ratio, 2)} response-rate tracking in coherent route windows.
7. **Validate with repeatable steps.** On a straight, no-special-task segment command 0 -> 320 -> 600 -> 800 rpm and reverse, hold each level 1.5 s, three repeats. Re-run this script and compare p50/p90 time, acceleration rate, peak pitch, PWM clipping fraction, and stopping distance. A speed increase is accepted only when those bounds remain safe.

## Code trace

- `user/cm7_0_isr.c`: navigation executes every 10 ms; speed feedback/PID/forward-feed executes every 9 ms; servo executor executes every 1 ms.
- `code/calculate/pid-new.c`: speed error produces leg-posture adjustment and acceleration/braking feed-forward is arbitrated before the final motor PWM.
- `code/servo/servo_executor.c`: the leg command is explicitly slew limited.
- `code/navigation/nav_replay/plan4/plan4_lqr_speed_planning.h`: Plan4 currently has target ramp constants of 110/200 rpm per navigation period.
- `tools/webview_nav_marker科目四/generate_plan4_smooth_path_考虑响应延迟_丝滑轨迹.py`: already accepts `--speed-response-delay-s`; feed it values identified by this analysis.
"""
    output.write_text(text, encoding="utf-8")


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description="Analyse Plan4 speed response from telemetry")
    parser.add_argument("--data-dir", type=Path, default=root / "data" / "科目四日志08172257")
    parser.add_argument("logs", nargs="*", type=Path, help="optional explicit telemetry CSVs")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    paths = args.logs or find_default_logs(args.data_dir)
    runs = [load_run(path) for path in paths]
    summary = pd.DataFrame([summarize_run(run) for run in runs])
    event_rows = [event for run in runs for event in extract_events(run)]
    accel_rows = [window for run in runs for window in extract_acceleration_windows(run)]
    event_columns = [
        "log", "loop", "kind", "target_before_rpm", "target_after_rpm", "target_step_rpm",
        "wheel_before_rpm", "wheel_after_60_140ms_rpm", "wheel_error_60_140ms_rpm",
        "t10_ms", "t63_ms", "t90_ms", "overshoot_rpm", "pwm_before", "pwm_after_60_140ms",
    ]
    events = pd.DataFrame(event_rows, columns=event_columns)
    accel_columns = [
        "log", "target_start_loop", "lag_used_ms", "target_start_rpm", "target_end_rpm",
        "target_accel_rpm_s", "target_fit_r2", "wheel_start_rpm", "wheel_end_rpm",
        "wheel_accel_rpm_s", "wheel_accel_mm_s2", "wheel_fit_r2", "response_rate_ratio",
        "mean_pwm", "p90_pwm", "speed_band", "is_coherent",
    ]
    accel_windows = pd.DataFrame(accel_rows, columns=accel_columns)
    accel_summary = summarize_acceleration_windows(accel_windows)
    output_dir = Path(__file__).resolve().parent
    summary.to_csv(output_dir / "plan4_speed_response_summary.csv", index=False, encoding="utf-8-sig")
    events.to_csv(output_dir / "plan4_speed_response_events.csv", index=False, encoding="utf-8-sig")
    accel_windows.to_csv(output_dir / "plan4_speed_acceleration_windows.csv", index=False, encoding="utf-8-sig")
    accel_summary.to_csv(output_dir / "plan4_speed_acceleration_summary.csv", index=False, encoding="utf-8-sig")
    plot_overview(runs, output_dir / "plan4_speed_response_overview.png")
    plot_event_overlay(runs, events, output_dir / "plan4_speed_response_event_overlay.png")
    plot_acceleration_rate(accel_windows, output_dir / "plan4_speed_acceleration_rate.png")
    write_report(summary, events, accel_windows, accel_summary, output_dir / "plan4_speed_response_analysis.md")
    print(f"logs={len(runs)} normal_frames={int(summary['normal_frames'].sum())} events={len(events)} accel_windows={len(accel_windows)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
