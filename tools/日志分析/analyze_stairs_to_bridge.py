#!/usr/bin/env python3
"""Analyze the open route segment from the three-step exit to bridge entry.

The script uses only replay frames (g_replay_state == 1), detects the two
vision-state transitions, and writes auditable CSV/Markdown summaries plus
PNG plots next to this file.  The candidate speed curve is an offline what-if:
it is not written to the firmware route table.
"""

from __future__ import annotations

import argparse
import csv
import re
from dataclasses import dataclass
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DejaVu Sans"]
plt.rcParams["axes.unicode_minus"] = False

SPEED_TO_MM_S = 4.79
MAX_LATERAL_ACCEL_MM_S2 = 2000.0
MAX_YAW_RATE_RAD_S = 2.2
PATH_SPEED_MAX_MM_S = 5000.0
CURRENT_BRIDGE_APPROACH_MM = 2500.0
CANDIDATE_BRIDGE_APPROACH_MM = 700.0
CANDIDATE_SPEED_CMD = 500.0
FUSION_JUMP_MM = 300.0
MAX_ACCEL_MM_S2 = 4500.0
MAX_DECEL_MM_S2 = 2500.0


@dataclass
class RunSegment:
    name: str
    frame: pd.DataFrame
    stairs_on_loop: int
    stairs_off_loop: int
    bridge_on_loop: int


def find_logs(input_dir: Path) -> list[Path]:
    return sorted(input_dir.glob("wifi_telemetry_*.csv"))


def _first_index(mask: pd.Series) -> int | None:
    values = np.flatnonzero(mask.to_numpy(dtype=bool))
    return int(values[0]) if len(values) else None


def load_segment(path: Path) -> RunSegment | None:
    frame = pd.read_csv(path)
    frame = frame[frame["g_replay_state"].astype(int) == 1].copy().reset_index(drop=True)
    if frame.empty:
        return None

    stairs = frame["vision_three_stage_control_is_active"].fillna(0).astype(int)
    bridge = frame["vision_bridge_task_is_active"].fillna(0).astype(int)
    stairs_on = _first_index((stairs == 1) & (stairs.shift(fill_value=0) == 0))
    stairs_off = _first_index((stairs == 0) & (stairs.shift(fill_value=0) == 1))
    bridge_on = _first_index((bridge == 1) & (bridge.shift(fill_value=0) == 0))
    if stairs_on is None or stairs_off is None or bridge_on is None or not stairs_on < stairs_off < bridge_on:
        return None

    segment = frame.iloc[stairs_off : bridge_on + 1].copy().reset_index(drop=True)
    dt_s = segment["loop"].diff().fillna(10.0).astype(float).clip(lower=1.0, upper=200.0) / 1000.0
    segment["time_s"] = (segment["loop"] - segment["loop"].iloc[0]) / 1000.0
    segment["target_rpm_abs"] = segment["target_speed_set"].abs()
    segment["target_mm_s"] = segment["target_rpm_abs"] * SPEED_TO_MM_S
    segment["actual_mm_s"] = segment["vx_body"].abs()
    # speed_R is negative for forward motion in the telemetry convention.
    segment["wheel_mm_s"] = (segment["speed_L"] - segment["speed_R"]) * SPEED_TO_MM_S / 2.0
    segment["pwm_mean_abs"] = (segment["pwm_left"].abs() + segment["pwm_right"].abs()) / 2.0
    segment["distance_mm"] = (segment["actual_mm_s"] * dt_s).cumsum()
    nav_dx = segment["nav_x"].diff()
    nav_dy = segment["nav_y"].diff()
    segment["nav_step_mm"] = np.hypot(nav_dx, nav_dy).fillna(0.0)
    segment["fusion_jump"] = segment["nav_step_mm"] > FUSION_JUMP_MM
    return RunSegment(path.name, segment, int(frame.loc[stairs_on, "loop"]), int(frame.loc[stairs_off, "loop"]), int(frame.loc[bridge_on, "loop"]))


def quantile(frame: pd.DataFrame, column: str, q: float) -> float:
    return float(frame[column].quantile(q))


def summarize(segment: RunSegment) -> dict[str, float | str | int]:
    frame = segment.frame
    target = frame["target_rpm_abs"]
    actual = frame["actual_mm_s"]
    tracking_error = (frame["actual_mm_s"] - frame["target_mm_s"]).abs()
    duration = float(frame["time_s"].iloc[-1])
    return {
        "log": segment.name,
        "stairs_on_loop": segment.stairs_on_loop,
        "stairs_exit_loop": segment.stairs_off_loop,
        "bridge_entry_loop": segment.bridge_on_loop,
        "duration_s": duration,
        "integrated_distance_mm": float(frame["distance_mm"].iloc[-1]),
        "target_rpm_p10": quantile(frame, "target_rpm_abs", 0.10),
        "target_rpm_p50": quantile(frame, "target_rpm_abs", 0.50),
        "target_rpm_p90": quantile(frame, "target_rpm_abs", 0.90),
        "target_rpm_max": float(target.max()),
        "actual_mm_s_p10": quantile(frame, "actual_mm_s", 0.10),
        "actual_mm_s_p50": quantile(frame, "actual_mm_s", 0.50),
        "actual_mm_s_p90": quantile(frame, "actual_mm_s", 0.90),
        "actual_mm_s_max": float(actual.max()),
        "target_le_220_fraction": float((target <= 220.0).mean()),
        "target_ge_300_fraction": float((target >= 300.0).mean()),
        "actual_ge_1500_fraction": float((actual >= 1500.0).mean()),
        "tracking_abs_error_mm_s_p50": float(tracking_error.quantile(0.50)),
        "tracking_abs_error_mm_s_p90": float(tracking_error.quantile(0.90)),
        "err_degree_abs_p50": quantile(frame.assign(err_abs=frame["err_degree"].abs()), "err_abs", 0.50),
        "err_degree_abs_p90": quantile(frame.assign(err_abs=frame["err_degree"].abs()), "err_abs", 0.90),
        "pwm_mean_abs_p50": quantile(frame, "pwm_mean_abs", 0.50),
        "pwm_mean_abs_p90": quantile(frame, "pwm_mean_abs", 0.90),
        "fusion_jump_count": int(frame["fusion_jump"].sum()),
    }


def parse_route_header(path: Path) -> pd.DataFrame:
    point_re = re.compile(
        r"\{([-+0-9.eE]+)f,\s*([-+0-9.eE]+)f,.*?\(uint8\)(\d+),\s*([-+0-9.eE]+)f,\s*([-+0-9.eE]+)f"
    )
    points: list[tuple[float, float, int, float, float]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        match = point_re.search(line)
        if match:
            points.append(tuple([float(match.group(1)), float(match.group(2)), int(match.group(3)), float(match.group(4)), float(match.group(5))]))
    if len(points) < 2:
        raise ValueError(f"路表没有解析到足够的点: {path}")
    values = pd.DataFrame(points, columns=["x", "y", "point_type", "target_rpm", "curvature"])
    values["s_mm"] = np.r_[0.0, np.cumsum(np.hypot(np.diff(values["x"]), np.diff(values["y"])))]
    values["target_rpm_abs"] = values["target_rpm"].abs()
    curvature_abs = values["curvature"].abs().replace(0.0, np.nan)
    curve_limit = np.sqrt(MAX_LATERAL_ACCEL_MM_S2 / curvature_abs)
    yaw_limit = MAX_YAW_RATE_RAD_S / curvature_abs
    values["curvature_ceiling_rpm"] = (pd.concat([curve_limit, yaw_limit], axis=1).min(axis=1).fillna(PATH_SPEED_MAX_MM_S).clip(upper=PATH_SPEED_MAX_MM_S) / SPEED_TO_MM_S)
    return values


def route_summary(route: pd.DataFrame) -> dict[str, float | int]:
    stairs_idx = int(route.index[route["point_type"] == 30][0])
    bridge_idx = int(route.index[route["point_type"] == 4][0])
    segment = route.iloc[stairs_idx : bridge_idx + 1].copy()
    ds = segment["s_mm"].diff().fillna(0.0)
    low = segment["target_rpm_abs"] <= 220.0
    candidate = apply_candidate(route).iloc[stairs_idx : bridge_idx + 1].copy()
    current_time_s = float((ds / (segment["target_rpm_abs"] * SPEED_TO_MM_S).clip(lower=1.0)).sum())
    candidate_time_s = float((ds / (candidate["candidate_target_rpm"] * SPEED_TO_MM_S).clip(lower=1.0)).sum())
    return {
        "route_stairs_exit_index": stairs_idx,
        "route_bridge_entry_index": bridge_idx,
        "route_distance_mm": float(route.loc[bridge_idx, "s_mm"] - route.loc[stairs_idx, "s_mm"]),
        "route_low_target_distance_mm": float(ds[low].sum()),
        "route_low_target_fraction": float(ds[low].sum() / max(ds.sum(), 1.0)),
        "route_target_rpm_p50": float(segment["target_rpm_abs"].quantile(0.5)),
        "route_target_rpm_p90": float(segment["target_rpm_abs"].quantile(0.9)),
        "route_curvature_ceiling_rpm_p10": float(segment["curvature_ceiling_rpm"].quantile(0.1)),
        "route_curvature_ceiling_rpm_p50": float(segment["curvature_ceiling_rpm"].quantile(0.5)),
        "route_curvature_abs_max": float(segment["curvature"].abs().max()),
        "route_curvature_ceiling_rpm_min": float(segment["curvature_ceiling_rpm"].min()),
        "route_current_nominal_time_s": current_time_s,
        "route_candidate_nominal_time_s": candidate_time_s,
    }


def apply_candidate(route: pd.DataFrame) -> pd.DataFrame:
    stairs_idx = int(route.index[route["point_type"] == 30][0])
    bridge_idx = int(route.index[route["point_type"] == 4][0])
    result = route.copy()
    mask = (result.index > stairs_idx) & (result["s_mm"] < result.loc[bridge_idx, "s_mm"] - CANDIDATE_BRIDGE_APPROACH_MM)
    result.loc[mask, "candidate_target_rpm"] = np.minimum(result.loc[mask, "curvature_ceiling_rpm"], CANDIDATE_SPEED_CMD)
    result.loc[~mask, "candidate_target_rpm"] = result.loc[~mask, "target_rpm_abs"]
    # The target above is a ceiling.  Apply the same forward/backward envelope
    # used by the route generator so the comparison has no speed steps.
    speed_mm_s = result["candidate_target_rpm"].to_numpy() * SPEED_TO_MM_S
    s_mm = result["s_mm"].to_numpy()
    for index in range(bridge_idx - 1, stairs_idx - 1, -1):
        ds = s_mm[index + 1] - s_mm[index]
        speed_mm_s[index] = min(speed_mm_s[index], np.sqrt(max(0.0, speed_mm_s[index + 1] ** 2 + 2.0 * MAX_DECEL_MM_S2 * ds)))
    for index in range(stairs_idx + 1, bridge_idx + 1):
        ds = s_mm[index] - s_mm[index - 1]
        speed_mm_s[index] = min(speed_mm_s[index], np.sqrt(max(0.0, speed_mm_s[index - 1] ** 2 + 2.0 * MAX_ACCEL_MM_S2 * ds)))
    result["candidate_target_rpm"] = speed_mm_s / SPEED_TO_MM_S
    # Preserve the actual task speed at the bridge approach while exposing the
    # open section to the curvature ceiling.  This is a comparison curve only.
    return result


def plot_run(segment: RunSegment, output: Path) -> None:
    frame = segment.frame
    fig, axes = plt.subplots(3, 1, figsize=(14, 11), sharex=True)
    t = frame["time_s"]
    axes[0].plot(t, frame["actual_mm_s"], label="|vx_body| actual (mm/s)", color="#1d4ed8")
    axes[0].plot(t, frame["target_mm_s"], label="target_speed_set x 4.79 (mm/s)", color="#dc2626", linewidth=1.4)
    axes[0].plot(t, frame["wheel_mm_s"].abs(), label="wheel average (mm/s)", color="#059669", alpha=0.65)
    axes[0].set_ylabel("speed (mm/s)")
    axes[0].legend(loc="upper right")
    axes[0].grid(alpha=0.25)

    axes[1].plot(t, frame["target_rpm_abs"], label="target rpm", color="#dc2626")
    axes[1].plot(t, frame["pwm_mean_abs"], label="mean |PWM|", color="#7c3aed", alpha=0.75)
    axes[1].set_ylabel("rpm / PWM")
    axes[1].legend(loc="upper right")
    axes[1].grid(alpha=0.25)

    axes[2].plot(t, frame["err_degree"], label="err_degree", color="#ea580c")
    axes[2].plot(t, frame["relative_yaw"], label="relative_yaw", color="#0891b2", alpha=0.75)
    axes[2].set_ylabel("angle (deg)")
    axes[2].set_xlabel("time after stairs exit (s)")
    axes[2].legend(loc="upper right")
    axes[2].grid(alpha=0.25)

    for ax in axes:
        ax.axvline(0.0, color="black", linestyle="--", linewidth=0.8)
        bridge_t = float(frame["time_s"].iloc[-1])
        ax.axvline(bridge_t, color="#16a34a", linestyle="--", linewidth=0.8)
    axes[0].set_title("stairs exit -> bridge entry telemetry")
    fig.tight_layout()
    fig.savefig(output, dpi=170)
    plt.close(fig)


def plot_overlay(segments: list[RunSegment], output: Path) -> None:
    fig, axes = plt.subplots(2, 1, figsize=(14, 10), gridspec_kw={"height_ratios": [1.2, 1]})
    for segment in segments:
        frame = segment.frame
        label = segment.name[:24]
        axes[0].plot(frame["distance_mm"] / 1000.0, frame["actual_mm_s"], label=label)
        axes[1].plot(frame["distance_mm"] / 1000.0, frame["target_rpm_abs"], label=label)
    axes[0].set_ylabel("|vx_body| (mm/s)")
    axes[1].set_ylabel("target |rpm|")
    axes[1].set_xlabel("integrated distance after stairs exit (m)")
    for ax in axes:
        ax.grid(alpha=0.25)
        ax.legend(loc="upper right", fontsize=8)
    axes[0].set_title("Three runs aligned at stairs exit")
    fig.tight_layout()
    fig.savefig(output, dpi=170)
    plt.close(fig)


def plot_route(route: pd.DataFrame, output: Path) -> None:
    stairs_idx = int(route.index[route["point_type"] == 30][0])
    bridge_idx = int(route.index[route["point_type"] == 4][0])
    segment = route.iloc[stairs_idx : bridge_idx + 1].copy()
    candidate = apply_candidate(route).iloc[stairs_idx : bridge_idx + 1]
    fig, axes = plt.subplots(1, 2, figsize=(16, 7))
    points = segment[["x", "y"]].to_numpy()
    axes[0].plot(points[:, 0], points[:, 1], color="#94a3b8", linewidth=1.0, label="current path")
    axes[0].scatter(segment["x"], segment["y"], c=segment["target_rpm_abs"], cmap="turbo", s=12)
    axes[0].scatter([segment.iloc[0]["x"], segment.iloc[-1]["x"]], [segment.iloc[0]["y"], segment.iloc[-1]["y"]], c=["#dc2626", "#16a34a"], s=70, zorder=3)
    axes[0].set_title("route geometry colored by target rpm")
    axes[0].set_xlabel("nav x (mm)")
    axes[0].set_ylabel("nav y (mm)")
    axes[0].axis("equal")
    axes[0].grid(alpha=0.25)

    axes[1].plot(segment["s_mm"] - segment["s_mm"].iloc[0], segment["target_rpm_abs"], label="current target", color="#dc2626")
    axes[1].plot(segment["s_mm"] - segment["s_mm"].iloc[0], candidate["candidate_target_rpm"], label="candidate: 700 mm bridge approach", color="#16a34a")
    axes[1].plot(segment["s_mm"] - segment["s_mm"].iloc[0], segment["curvature_ceiling_rpm"], label="curvature ceiling", color="#2563eb", alpha=0.65)
    axes[1].set_title("current vs open-section candidate")
    axes[1].set_xlabel("distance from stairs exit (mm)")
    axes[1].set_ylabel("speed command |rpm|")
    axes[1].grid(alpha=0.25)
    axes[1].legend(loc="upper left")
    fig.tight_layout()
    fig.savefig(output, dpi=170)
    plt.close(fig)


def write_report(output_dir: Path, summaries: list[dict[str, float | str | int]], route_metrics: dict[str, float | int]) -> None:
    summary_path = output_dir / "stairs_to_bridge_summary.csv"
    pd.DataFrame(summaries).to_csv(summary_path, index=False, encoding="utf-8-sig")
    lines = [
        "# 台阶出口到单边桥入口日志分析",
        "",
        "筛选条件：`g_replay_state == 1`；区间起点为三级台阶状态机从 active=1 变为 0，终点为单边桥状态机从 0 变为 1。",
        "",
        "## 关键结论",
        "",
        f"- 三次运行的该区间实际耗时约 {np.mean([x['duration_s'] for x in summaries]):.2f} s，积分距离约 {np.mean([x['integrated_distance_mm'] for x in summaries]) / 1000:.2f} m。",
        f"- 实际 `|vx_body|` 中位数约 {np.mean([x['actual_mm_s_p50'] for x in summaries]):.0f} mm/s；目标转速中位数约 {np.mean([x['target_rpm_p50'] for x in summaries]):.0f} rpm，即约 {np.mean([x['target_rpm_p50'] for x in summaries]) * SPEED_TO_MM_S:.0f} mm/s。",
        f"- 目标转速不超过 220 rpm 的时间比例约 {np.mean([x['target_le_220_fraction'] for x in summaries]) * 100:.1f}%，说明这段开放道路大部分时间受 `-200/-220` 级别目标速度限制。",
        f"- PWM 中位数约 {np.mean([x['pwm_mean_abs_p50'] for x in summaries]):.0f}，没有表现出长期 PWM 饱和；首要瓶颈是目标速度和状态机接管，不是电机已经饱和。",
        f"- 当前路表台阶出口到单边桥入口约 {route_metrics['route_distance_mm'] / 1000:.2f} m，其中目标速度不超过 220 rpm 的路段约占 {route_metrics['route_low_target_fraction'] * 100:.1f}%。",
        f"- 路表在这一段的最大曲率为 {route_metrics['route_curvature_abs_max']:.6f} 1/mm，局部曲率上限最低只有 {route_metrics['route_curvature_ceiling_rpm_min']:.0f} rpm；该尖峰会造成台阶出口后的短时降速和 PWM 波动。",
        f"- 仅按离线路表速度积分，当前曲线通过这一段约需 {route_metrics['route_current_nominal_time_s']:.2f} s；候选曲线约为 {route_metrics['route_candidate_nominal_time_s']:.2f} s。该值不包含台阶/桥状态机时间，不能直接当作实车成绩。",
        "",
        "## 建议的优化顺序",
        "",
        "1. 先消掉台阶出口后的局部曲率尖峰：忽略手打普通点，直接以台阶出口走廊和单边桥入口走廊构造较长控制柄的 G2 曲线。只提高速度而保留这个尖峰，会把上限再次压回约 82 rpm。",
        "2. 将桥前固定 `-200` 的 approach 距离从 2500 mm 缩短到 600~700 mm，与 C 侧 `PLAN4_SPECIAL_ALIGN_DISTANCE_MM=600` 的实际对准窗口一致；开放段按曲率上限规划。",
        "3. 检查台阶状态机结束后的 Plan4 handoff：日志中状态机结束后目标速度会落到约 300 rpm 附近，建议把出口再接管窗口从 600 mm 缩短或把 `PLAN4_EXIT_REJOIN_MAX_SPEED_CMD` 提到 450~500，并确认融合坐标重定位后的横向误差仍小于 100 mm。",
        "4. 保留最后 600~700 mm 的桥前低速和桥状态机自身的 `-200`，不要在桥上直接追求开放段速度；用状态机 active 信号作为硬边界。",
        "5. 先用候选曲线图做离线检查，再单独生成候选路表进行一次低风险试跑；本目录中的候选曲线没有写入固件路表。",
        "",
        "## 输出文件",
        "",
        "- `stairs_to_bridge_summary.csv`: 三次运行的量化指标。",
        "- `stairs_to_bridge_speed_<run>.png`: 每次运行的目标/实际速度、PWM、误差和时间轴。",
        "- `stairs_to_bridge_runs_overlay.png`: 三次运行按台阶出口对齐的对比。",
        "- `stairs_to_bridge_route_candidate.png`: 当前路表和缩短桥前 approach 的候选速度曲线。",
    ]
    (output_dir / "stairs_to_bridge_analysis.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-dir", type=Path, default=Path("data/科目四日志08172257"))
    parser.add_argument(
        "--route-header",
        type=Path,
        default=Path("data/科目四日志08172257/nav_replay_route_table_08172257.h"),
        help="Route-table snapshot captured with the telemetry logs.",
    )
    parser.add_argument("--output-dir", type=Path, default=Path("tools/日志分析"))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    segments = [segment for path in find_logs(args.input_dir) if (segment := load_segment(path)) is not None]
    if not segments:
        raise SystemExit("没有找到包含完整台阶出口和单边桥入口状态切换的 replay 日志。")
    summaries = [summarize(segment) for segment in segments]
    route = parse_route_header(args.route_header)
    route_metrics = route_summary(route)
    for segment in segments:
        safe_name = re.sub(r"[^A-Za-z0-9_.-]+", "_", segment.name)
        plot_run(segment, args.output_dir / f"stairs_to_bridge_speed_{safe_name}.png")
    plot_overlay(segments, args.output_dir / "stairs_to_bridge_runs_overlay.png")
    plot_route(route, args.output_dir / "stairs_to_bridge_route_candidate.png")
    write_report(args.output_dir, summaries, route_metrics)
    print(f"分析日志: {len(segments)} 份")
    print(f"结果目录: {args.output_dir.resolve()}")
    print(f"平均区间耗时: {np.mean([x['duration_s'] for x in summaries]):.2f} s")
    print(f"平均实际速度中位数: {np.mean([x['actual_mm_s_p50'] for x in summaries]):.1f} mm/s")
    print(f"平均目标转速中位数: {np.mean([x['target_rpm_p50'] for x in summaries]):.1f} rpm")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
