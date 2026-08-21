#!/usr/bin/env python3
"""Analyze Plan4 free-tracking performance against a generated route table.

The analysis deliberately excludes state-machine-owned frames.  It reconstructs
the same local segment projection used by Plan4 and compares route curvature,
speed commands, actual body speed, yaw rate, and tracking error.

Run from the repository root:
    .venv\\Scripts\\python.exe tools\\日志分析\\plan4_tracking_log_analysis.py
"""
from __future__ import annotations

import re
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


ROOT = Path(__file__).resolve().parents[2]
OUTPUT_DIR = Path(__file__).resolve().parent
ROUTE_FILE = ROOT / "data" / "科目四日志08172257" / "nav_replay_route_table_08181919.h"
SPEED_TO_MM_S = 4.79
CURVATURE_ALERT_1_PER_MM = 0.0005
LOOKAHEAD_MM = 2500.0
SAFE_LATERAL_ACCEL_MM_S2 = 2000.0
MAX_PROJECTION_ERROR_MM = 1200.0

LOG_FILES = (
    ROOT / "data" / "科目四日志08172257" / "wifi_telemetry_20260818_182540_472.csv",
    ROOT / "data" / "科目四日志08172257" / "wifi_telemetry_20260818_182825_112.csv",
    ROOT / "data" / "科目四日志08172257" / "wifi_telemetry_20260818_175457_873_雷区未进.csv",
    ROOT / "tools" / "webview_nav_marker科目四" / "wifi_telemetry_20260819_145805_312_完赛.csv",
    ROOT / "tools" / "webview_nav_marker科目四" / "wifi_telemetry_20260819_145858_230_到颠簸.csv",
    ROOT / "tools" / "webview_nav_marker科目四" / "wifi_telemetry_20260819_145937_398_坡道未回来.csv",
)

plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DejaVu Sans"]
plt.rcParams["axes.unicode_minus"] = False


def parse_route(path: Path) -> pd.DataFrame:
    """Parse NavRamPoint_t rows from a generated static C route table."""
    rows: list[dict[str, float | int]] = []
    for match in re.finditer(r"\{([^{}]+)\}", path.read_text(encoding="utf-8")):
        fields = [field.strip() for field in match.group(1).split(",")]
        if len(fields) != 7 or "uint8" not in fields[4]:
            continue
        rows.append(
            {
                "route_index": len(rows),
                "x": float(fields[0].removesuffix("f")),
                "y": float(fields[1].removesuffix("f")),
                "yaw_deg": float(fields[2].removesuffix("f")),
                "point_type": int(re.search(r"\d+", fields[4]).group()),
                "target_speed_rpm": float(fields[5].removesuffix("f")),
                "curvature": float(fields[6].removesuffix("f")),
            }
        )
    route = pd.DataFrame(rows)
    if len(route) < 2:
        raise ValueError(f"Cannot parse route points from {path}")
    ds = np.hypot(np.diff(route.x), np.diff(route.y))
    route["s_mm"] = np.r_[0.0, np.cumsum(ds)]
    return route


def normalize_angle(angle_deg: np.ndarray) -> np.ndarray:
    return (angle_deg + 180.0) % 360.0 - 180.0


def replay_mask(frame: pd.DataFrame) -> pd.Series:
    """Keep only normal Plan4 LQR ownership, excluding all task state machines."""
    mask = frame["g_replay_state"].eq(1) & frame["g_special_action_trigger"].eq(0)
    for column in (
        "minefield_is_active",
        "bumpy_road_is_active",
        "vision_bridge_task_is_active",
        "vision_slope_task_is_active",
        "vision_three_stage_control_is_active",
    ):
        mask &= frame[column].eq(0)
    # Point type zero is the actual free-route LQR portion, excluding task entry anchors.
    mask &= frame["nav_replay_point_type"].eq(0)
    return mask


def split_continuous_runs(frame: pd.DataFrame) -> list[pd.DataFrame]:
    """Split filtered LQR samples at telemetry or ownership discontinuities."""
    if frame.empty:
        return []
    selected = frame.copy()
    selected["_source_row"] = selected.index
    new_run = (selected.loop.diff().fillna(10.0) > 50.0) | (selected.loop.diff().fillna(10.0) <= 0.0)
    # A state-machine interval disappears after filtering, so its source rows
    # are nonconsecutive even if its loop counter itself is continuous.
    new_run |= selected._source_row.diff().fillna(1).ne(1)
    selected["_run_id"] = new_run.cumsum()
    return [part.copy() for _, part in selected.groupby("_run_id") if len(part) >= 30]


def project_to_route(frame: pd.DataFrame, route: pd.DataFrame) -> pd.DataFrame:
    """Project every telemetry position to its closest route segment in batches."""
    result = frame.copy()
    ax = route.x.to_numpy()[:-1]
    ay = route.y.to_numpy()[:-1]
    bx = route.x.to_numpy()[1:]
    by = route.y.to_numpy()[1:]
    dx = bx - ax
    dy = by - ay
    len_sq = dx * dx + dy * dy
    seg_len = np.sqrt(len_sq)
    tangent_x = dx / np.maximum(seg_len, 1e-6)
    tangent_y = dy / np.maximum(seg_len, 1e-6)
    route_s = route.s_mm.to_numpy()
    route_kappa = route.curvature.to_numpy()
    route_speed = route.target_speed_rpm.to_numpy()
    route_yaw = -np.degrees(np.arctan2(dy, -dx))

    n = len(result)
    indices = np.empty(n, dtype=np.int32)
    projections = np.empty(n, dtype=float)
    for start in range(0, n, 256):
        end = min(n, start + 256)
        x = result.nav_x.to_numpy()[start:end, None]
        y = result.nav_y.to_numpy()[start:end, None]
        t = ((x - ax) * dx + (y - ay) * dy) / np.maximum(len_sq, 1e-6)
        t = np.clip(t, 0.0, 1.0)
        px = ax + t * dx
        py = ay + t * dy
        nearest = np.argmin((x - px) ** 2 + (y - py) ** 2, axis=1)
        indices[start:end] = nearest
        projections[start:end] = t[np.arange(end - start), nearest]

    px = ax[indices] + projections * dx[indices]
    py = ay[indices] + projections * dy[indices]
    local_yaw = route_yaw[indices]
    preview_offset = np.where(np.abs(route_kappa[indices]) >= 0.0015, 2, 5)
    preview_idx = np.minimum(indices + preview_offset, len(route) - 1)
    cross_track = tangent_y[indices] * (result.nav_x.to_numpy() - px) - tangent_x[indices] * (result.nav_y.to_numpy() - py)
    heading_error = normalize_angle(local_yaw - result.relative_yaw.to_numpy())

    result["route_index"] = indices
    result["route_s_mm"] = route_s[indices] + projections * seg_len[indices]
    result["proj_x"] = px
    result["proj_y"] = py
    result["cross_track_mm"] = cross_track
    result["route_heading_error_deg"] = heading_error
    result["route_speed_rpm"] = route_speed[indices]
    result["route_speed_mm_s"] = np.abs(route_speed[indices]) * SPEED_TO_MM_S
    result["preview_curvature"] = route_kappa[preview_idx]

    # Peak curvature in the next fixed path-distance lookahead.  It is the
    # quantity a proactive speed governor must react to before error grows.
    peak_kappa = np.empty(n, dtype=float)
    for row, s_now in enumerate(result.route_s_mm.to_numpy()):
        first = int(np.searchsorted(route_s, s_now, side="left"))
        last = int(np.searchsorted(route_s, s_now + LOOKAHEAD_MM, side="right"))
        peak_kappa[row] = np.abs(route_kappa[first:max(first + 1, last)]).max()
    result["ahead_peak_curvature"] = peak_kappa
    result["actual_speed_mm_s"] = np.abs(result.vx_body.to_numpy())
    result["command_speed_mm_s"] = np.abs(result.target_speed_set.to_numpy()) * SPEED_TO_MM_S
    # The logged imu_gyro_z is not a trustworthy normal-driving yaw-rate trace:
    # in these logs it remains near zero while relative_yaw visibly evolves.
    # Relative yaw is the controller's heading source, so use its wrapped
    # frame-to-frame derivative and a short median smoother instead.
    dt_s = result.loop.diff().to_numpy(dtype=float) / 1000.0
    yaw_delta = normalize_angle(np.diff(result.relative_yaw.to_numpy(), prepend=result.relative_yaw.iloc[0]))
    yaw_rate = yaw_delta / np.maximum(dt_s, 0.001)
    yaw_rate[0] = yaw_rate[1] if len(yaw_rate) > 1 else 0.0
    result["yaw_rate_deg_s"] = pd.Series(yaw_rate, index=result.index).rolling(5, center=True, min_periods=1).median()
    result["yaw_rate_rad_s"] = np.radians(result.yaw_rate_deg_s.to_numpy())
    result["lateral_accel_mm_s2"] = result.actual_speed_mm_s * np.abs(result.yaw_rate_rad_s)
    result["speed_ratio_to_route"] = result.command_speed_mm_s / np.maximum(result.route_speed_mm_s, 1.0)
    return result


def best_steer_yaw_lag(frame: pd.DataFrame) -> tuple[float, float]:
    """Return response lag and signed correlation between steer request and yaw rate."""
    valid = frame[(frame.actual_speed_mm_s > 800.0) & (np.abs(frame.cross_track_mm) < 600.0)]
    if len(valid) < 80:
        return float("nan"), float("nan")
    steer = valid.err_degree.to_numpy(dtype=float)
    yaw = valid.yaw_rate_deg_s.to_numpy(dtype=float)
    dt_ms = float(valid.loop.diff().median())
    best_lag = 0
    best_corr = 0.0
    for lag in range(-12, 13):
        if lag < 0:
            a, b = steer[-lag:], yaw[:lag]
        elif lag > 0:
            a, b = steer[:-lag], yaw[lag:]
        else:
            a, b = steer, yaw
        if np.std(a) < 1e-6 or np.std(b) < 1e-6:
            continue
        corr = float(np.corrcoef(a, b)[0, 1])
        if abs(corr) > abs(best_corr):
            best_lag, best_corr = lag, corr
    return best_lag * dt_ms, best_corr


def summarize_log(name: str, frame: pd.DataFrame) -> dict[str, float | int | str]:
    valid = frame[np.abs(frame.cross_track_mm) <= MAX_PROJECTION_ERROR_MM].copy()
    corner = valid[valid.ahead_peak_curvature >= CURVATURE_ALERT_1_PER_MM]
    straight = valid[valid.ahead_peak_curvature < CURVATURE_ALERT_1_PER_MM]
    lag_ms, steer_yaw_corr = best_steer_yaw_lag(valid)

    def quantile(column: str, q: float, source: pd.DataFrame = valid) -> float:
        return float(source[column].quantile(q)) if len(source) else float("nan")

    return {
        "log": name,
        "normal_frames": len(valid),
        "normal_duration_s": (valid.loop.iloc[-1] - valid.loop.iloc[0]) / 1000.0 if len(valid) > 1 else 0.0,
        "route_progress_m": (valid.route_s_mm.max() - valid.route_s_mm.min()) / 1000.0 if len(valid) else 0.0,
        "speed_p50_mm_s": quantile("actual_speed_mm_s", 0.50),
        "speed_p95_mm_s": quantile("actual_speed_mm_s", 0.95),
        "speed_max_mm_s": float(valid.actual_speed_mm_s.max()) if len(valid) else float("nan"),
        "cross_track_p50_abs_mm": quantile("cross_track_mm", 0.50, valid.assign(cross_track_mm=np.abs(valid.cross_track_mm))),
        "cross_track_p95_abs_mm": quantile("cross_track_mm", 0.95, valid.assign(cross_track_mm=np.abs(valid.cross_track_mm))),
        "heading_p95_abs_deg": quantile("route_heading_error_deg", 0.95, valid.assign(route_heading_error_deg=np.abs(valid.route_heading_error_deg))),
        "corner_frame_ratio": len(corner) / len(valid) if len(valid) else float("nan"),
        "corner_speed_p95_mm_s": quantile("actual_speed_mm_s", 0.95, corner),
        "corner_cross_p95_abs_mm": quantile("cross_track_mm", 0.95, corner.assign(cross_track_mm=np.abs(corner.cross_track_mm))),
        "corner_lat_accel_p95_mm_s2": quantile("lateral_accel_mm_s2", 0.95, corner),
        "corner_lat_accel_max_mm_s2": float(corner.lateral_accel_mm_s2.max()) if len(corner) else float("nan"),
        "runtime_limit_active_ratio": float((valid.speed_ratio_to_route < 0.90).mean()) if len(valid) else float("nan"),
        "runtime_limit_in_corner_ratio": float((corner.speed_ratio_to_route < 0.90).mean()) if len(corner) else float("nan"),
        "cross_gt_250_ratio": float((np.abs(valid.cross_track_mm) > 250.0).mean()) if len(valid) else float("nan"),
        "heading_gt_35_ratio": float((np.abs(valid.route_heading_error_deg) > 35.0).mean()) if len(valid) else float("nan"),
        "steer_to_yaw_lag_ms": lag_ms,
        "steer_to_yaw_correlation": steer_yaw_corr,
    }


def route_corner_zones(route: pd.DataFrame) -> list[tuple[int, int]]:
    flagged = np.abs(route.curvature.to_numpy()) >= CURVATURE_ALERT_1_PER_MM
    zones: list[tuple[int, int]] = []
    start: int | None = None
    for index, value in enumerate(flagged):
        if value and start is None:
            start = index
        if start is not None and (not value or index == len(flagged) - 1):
            end = index if value and index == len(flagged) - 1 else index - 1
            zones.append((start, end))
            start = None
    return zones


def build_corner_events(name: str, frame: pd.DataFrame, route: pd.DataFrame) -> list[dict[str, float | int | str]]:
    events: list[dict[str, float | int | str]] = []
    for zone_id, (first, last) in enumerate(route_corner_zones(route), start=1):
        start_s = float(route.s_mm.iloc[first])
        end_s = float(route.s_mm.iloc[last])
        # Include 1 m before the bend: this is where braking should already be visible.
        rows = frame[(frame.route_s_mm >= start_s - 1000.0) & (frame.route_s_mm <= end_s + 400.0)]
        if len(rows) < 5:
            continue
        in_curve = rows[(rows.route_s_mm >= start_s) & (rows.route_s_mm <= end_s)]
        events.append(
            {
                "log": name,
                "corner_id": zone_id,
                "route_start_m": start_s / 1000.0,
                "route_end_m": end_s / 1000.0,
                "peak_route_curvature_1_per_mm": float(np.abs(route.curvature.iloc[first:last + 1]).max()),
                "entry_speed_mm_s": float(rows.actual_speed_mm_s.iloc[0]),
                "curve_speed_max_mm_s": float(in_curve.actual_speed_mm_s.max()) if len(in_curve) else float("nan"),
                "curve_cross_p95_abs_mm": float(np.abs(in_curve.cross_track_mm).quantile(0.95)) if len(in_curve) else float("nan"),
                "curve_heading_p95_abs_deg": float(np.abs(in_curve.route_heading_error_deg).quantile(0.95)) if len(in_curve) else float("nan"),
                "curve_lat_accel_max_mm_s2": float(in_curve.lateral_accel_mm_s2.max()) if len(in_curve) else float("nan"),
                "runtime_limit_min_ratio": float(rows.speed_ratio_to_route.min()),
            }
        )
    return events


def plot_overview(route: pd.DataFrame, runs: dict[str, pd.DataFrame]) -> None:
    figure, axes = plt.subplots(2, 2, figsize=(18, 13))
    ax = axes[0, 0]
    ax.plot(route.x, route.y, color="black", linewidth=1.0, alpha=0.55, label="route table")
    scatter = None
    for name, frame in runs.items():
        sample = frame.iloc[::4]
        scatter = ax.scatter(sample.nav_x, sample.nav_y, c=sample.actual_speed_mm_s / 1000.0,
                             s=3, cmap="turbo", vmin=0.0, vmax=5.0, alpha=0.52)
    ax.set_title("普通回放轨迹叠加，颜色为实测速度 (m/s)")
    ax.set_aspect("equal", adjustable="box")
    ax.set_xlabel("nav_x (mm)")
    ax.set_ylabel("nav_y (mm)")
    if scatter is not None:
        figure.colorbar(scatter, ax=ax, label="actual speed (m/s)")

    ax = axes[0, 1]
    points = pd.concat(runs.values(), ignore_index=True)
    sample = points.iloc[::5]
    ax.scatter(sample.ahead_peak_curvature * 1000.0, sample.actual_speed_mm_s / 1000.0,
               c=np.abs(sample.cross_track_mm), s=4, alpha=0.45, cmap="magma", vmin=0, vmax=500)
    kappa = np.linspace(CURVATURE_ALERT_1_PER_MM, max(0.004, float(sample.ahead_peak_curvature.max())), 200)
    ax.plot(kappa * 1000.0, np.sqrt(SAFE_LATERAL_ACCEL_MM_S2 / kappa) / 1000.0,
            color="cyan", linewidth=2.0, label="a_lat = 2.0 m/s²")
    ax.set_xlabel("future peak curvature (1/m), next 2.5 m")
    ax.set_ylabel("actual speed (m/s)")
    ax.set_title("速度 vs 前方曲率：曲线以上意味着横向加速度风险更高")
    ax.legend()

    ax = axes[1, 0]
    ax.scatter(sample.actual_speed_mm_s / 1000.0, sample.lateral_accel_mm_s2 / 1000.0,
               c=sample.ahead_peak_curvature * 1000.0, s=4, alpha=0.45, cmap="viridis")
    ax.axhline(SAFE_LATERAL_ACCEL_MM_S2 / 1000.0, color="crimson", linestyle="--", label="proposed safe limit")
    ax.set_xlabel("actual speed (m/s)")
    ax.set_ylabel("|v * yaw_rate| (m/s²)")
    ax.set_title("由实测速度和陀螺计算的横向加速度代理量")
    ax.legend()

    ax = axes[1, 1]
    ax.scatter(np.abs(sample.cross_track_mm), sample.speed_ratio_to_route, c=np.abs(sample.route_heading_error_deg),
               s=4, alpha=0.45, cmap="plasma", vmin=0, vmax=70)
    ax.axvline(250.0, color="crimson", linestyle="--", label="current cross-track threshold")
    ax.axhline(0.90, color="gray", linestyle="--", label="runtime limit active")
    ax.set_xlim(0, 900)
    ax.set_ylim(0, 1.2)
    ax.set_xlabel("|cross-track error| (mm)")
    ax.set_ylabel("output command / route command")
    ax.set_title("当前运行时保护主要由已出现的误差触发")
    ax.legend()

    figure.tight_layout()
    figure.savefig(OUTPUT_DIR / "plan4_tracking_overview.png", dpi=180)
    plt.close(figure)


def plot_runs(route: pd.DataFrame, runs: dict[str, pd.DataFrame]) -> None:
    figure, axes = plt.subplots(len(runs), 3, figsize=(20, 4.3 * len(runs)))
    for row, (name, frame) in enumerate(runs.items()):
        time_s = (frame.loop - frame.loop.iloc[0]) / 1000.0
        route_slice = route[(route.s_mm >= frame.route_s_mm.min() - 500.0) &
                            (route.s_mm <= frame.route_s_mm.max() + 500.0)]
        ax = axes[row, 0]
        ax.plot(route_slice.x, route_slice.y, color="black", linewidth=1.0, alpha=0.55)
        color = ax.scatter(frame.nav_x, frame.nav_y, c=frame.actual_speed_mm_s / 1000.0,
                           s=5, cmap="turbo", vmin=0, vmax=5)
        ax.set_aspect("equal", adjustable="box")
        ax.set_title(f"{name}: 轨迹 / 速度")
        ax.set_xlabel("x (mm)")
        ax.set_ylabel("y (mm)")

        ax = axes[row, 1]
        ax.plot(time_s, frame.actual_speed_mm_s / 1000.0, label="actual vx", linewidth=1.2)
        ax.plot(time_s, frame.command_speed_mm_s / 1000.0, label="output target", linewidth=1.0)
        ax.plot(time_s, frame.route_speed_mm_s / 1000.0, label="route target", linewidth=0.9, alpha=0.75)
        ax2 = ax.twinx()
        ax2.fill_between(time_s, 0, frame.ahead_peak_curvature * 1000.0,
                         color="orange", alpha=0.18, label="ahead curvature")
        ax.set_title("速度命令、实测速度与前方曲率")
        ax.set_xlabel("time (s)")
        ax.set_ylabel("speed (m/s)")
        ax2.set_ylabel("peak curvature (1/m)")
        ax.legend(loc="upper left", fontsize=8)

        ax = axes[row, 2]
        ax.plot(time_s, frame.cross_track_mm, label="cross-track (mm)", linewidth=1.0)
        ax.plot(time_s, frame.route_heading_error_deg * 8.0, label="heading error x8", linewidth=0.9)
        ax.plot(time_s, frame.err_degree * 8.0, label="steer request x8", linewidth=0.9)
        ax.axhline(250.0, color="crimson", linestyle="--", linewidth=0.8)
        ax.axhline(-250.0, color="crimson", linestyle="--", linewidth=0.8)
        ax.set_ylim(-950, 950)
        ax.set_title("投影误差、航向误差与转向请求")
        ax.set_xlabel("time (s)")
        ax.set_ylabel("mm / scaled degrees")
        ax.legend(loc="upper right", fontsize=8)
    figure.tight_layout()
    figure.savefig(OUTPUT_DIR / "plan4_tracking_runs.png", dpi=170)
    plt.close(figure)


def write_report(summary: pd.DataFrame, events: pd.DataFrame, route: pd.DataFrame) -> None:
    peak_kappa = float(np.abs(route.curvature).max())
    route_limit_at_peak = np.sqrt(SAFE_LATERAL_ACCEL_MM_S2 / peak_kappa)
    stable = summary[(summary.normal_frames >= 150) & (summary.cross_track_p95_abs_mm < 150.0)]
    corner = stable.corner_lat_accel_p95_mm_s2.dropna()
    representative_kappa = 0.001
    representative_safe_speed = np.sqrt(SAFE_LATERAL_ACCEL_MM_S2 / representative_kappa)
    speed_at_four = 4000.0
    required_lat_accel = speed_at_four * speed_at_four * representative_kappa
    report = [
        "# Plan4 普通寻迹日志分析",
        "",
        "分析范围：`g_replay_state=REPLAY_RUNNING`、未被任何特殊任务接管，且 `nav_replay_point_type=0` 的帧。",
        "位置采用 C 路表线段投影重建；`heading` 与 `imu_gyro_z` 未使用。横向加速度由 `relative_yaw` 的平滑差分横摆率计算：`|vx_body * yaw_rate|`。它适合趋势比较，不能替代完整车辆动力学标定。",
        "",
        "## 关键结果",
        "",
        f"- 使用路表 `{ROUTE_FILE.name}`，共 {len(route)} 点、{route.s_mm.iloc[-1] / 1000.0:.1f} m；峰值曲率为 {peak_kappa * 1000.0:.2f} 1/m。若把 2.0 m/s² 作为保守横向加速度上限，该峰值对应速度仅 {route_limit_at_peak / 1000.0:.2f} m/s。",
        f"- 在横向误差 P95 小于 150 mm 的连续正常片段中，急弯横向加速度 P95 仍为 {corner.min() / 1000.0:.2f} 到 {corner.max() / 1000.0:.2f} m/s²。这表明贴线不代表横向载荷安全。",
        f"- 典型长弯曲率约 1.0 1/m；按 2.0 m/s² 的保守横向能力，安全稳态速度约 {representative_safe_speed / 1000.0:.2f} m/s。若以 4.0 m/s 通过，需要约 {required_lat_accel / 1000.0:.1f} m/s² 横向能力。",
        "- 当前 Plan4 运行时保护由已发生的横向/航向误差驱动，不能替代入弯前按曲率和制动距离降速。应保留它作为离轨兜底，而不是作为弯道速度规划。",
        "",
        "## 对速度与丝滑寻迹的建议",
        "",
        "1. 新增前方 2.5 到 5 m 曲率扫描的实时安全速度上限。每周期用实测 `vx_body` 和路径距离计算 `v_allow = sqrt(v_curve² + 2*a_brake*max(d - v_actual*T_delay - margin, 0))`，对前方所有点取最小值。初版只允许压低路表目标。",
        "2. 曲率限速用保守实测横向能力，而不是离线理想值。先以 1.8 到 2.2 m/s² 起步；速度越高或路面越滑，应再降低。",
        "3. 转向前馈 `Kff * v * kappa` 的 `v` 改为实时安全速度与实测速度的平滑最小值，避免路表高目标速度在急弯瞬间直接放大前馈。",
        "4. 为转向反馈加入小死区而非追零：建议先试横向 25 到 40 mm、航向 1 到 2 deg；死区外再渐进恢复增益。这样允许传感器噪声和正常的小偏差，不会持续反打方向。它不应被用来放宽曲率限速。",
        "5. 速度保护使用双层误差带：横向 80 到 120 mm、航向 8 到 12 deg 进入轻度限速；横向 250 mm、航向 35 deg 仍作为强保护。用滞回避免门限附近反复加减速。",
        "6. 急弯前瞻从硬切换改为随速度和曲率连续变化，并对前瞻曲率做路径窗口平均或低通。高速时不能因发现急弯直接从 5 点跳到 2 点，否则曲率前馈和转向请求容易台阶化。",
        "7. 先标定减速延迟 `T_delay`、实测制动能力 `a_brake`、安全横向加速度 `a_lat`，再提高逐段路表速度；不要用增大横向增益来掩盖入弯过快。",
        "",
        "## 文件说明",
        "",
        "- `plan4_tracking_summary.csv`：逐日志汇总指标。",
        "- `plan4_tracking_corner_events.csv`：每次经过高曲率区域的入弯速度、误差和横向加速度。",
        "- `plan4_tracking_overview.png`：总览散点和风险边界。",
        "- `plan4_tracking_runs.png`：每份日志的路线、速度、误差时序。",
    ]
    (OUTPUT_DIR / "plan4_tracking_analysis.md").write_text("\n".join(report) + "\n", encoding="utf-8")


def main() -> None:
    route = parse_route(ROUTE_FILE)
    summary_rows: list[dict[str, float | int | str]] = []
    event_rows: list[dict[str, float | int | str]] = []
    runs: dict[str, pd.DataFrame] = {}
    for path in LOG_FILES:
        if not path.is_file():
            print(f"skip missing log: {path}")
            continue
        raw = pd.read_csv(path)
        selected = raw.loc[replay_mask(raw)].copy()
        if selected.empty:
            print(f"skip no normal replay frame: {path.name}")
            continue
        for segment_id, segment in enumerate(split_continuous_runs(selected), start=1):
            projected = project_to_route(segment.reset_index(drop=True), route)
            projected = projected[np.abs(projected.cross_track_mm) <= MAX_PROJECTION_ERROR_MM].reset_index(drop=True)
            if projected.empty:
                continue
            name = f"{path.name}#{segment_id}"
            runs[name] = projected
            summary_rows.append(summarize_log(name, projected))
            event_rows.extend(build_corner_events(name, projected, route))

    summary = pd.DataFrame(summary_rows)
    events = pd.DataFrame(event_rows)
    summary.to_csv(OUTPUT_DIR / "plan4_tracking_summary.csv", index=False, encoding="utf-8-sig")
    events.to_csv(OUTPUT_DIR / "plan4_tracking_corner_events.csv", index=False, encoding="utf-8-sig")
    # Keep every reconstructed sample auditable, including the source-log and
    # segment identity needed to inspect individual sharp-turn episodes.
    pd.concat(
        [frame.assign(log=name) for name, frame in runs.items()], ignore_index=True
    ).to_csv(OUTPUT_DIR / "plan4_tracking_projected_samples.csv", index=False, encoding="utf-8-sig")
    plot_overview(route, runs)
    plot_runs(route, runs)
    write_report(summary, events, route)
    print(f"analyzed {len(runs)} logs; outputs written to {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
