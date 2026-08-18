#!/usr/bin/env python3
"""分析 Plan4 中颠簸路出口到首个雷区转圈入口的速度衔接。

默认读取 data/科目四日志08172257 的 0818 三份遥测以及同目录的 08181919
路线表。脚本会自动按 loop 回退切分一份 CSV 中拼接的多次起跑，仅保留
同时包含“颠簸结束”和“雷区转圈开始”的回放片段。

运行：
    .venv\\Scripts\\python.exe tools\\日志分析\\analyze_bumpy_to_minefield.py
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


ROOT = Path(__file__).resolve().parents[2]
DATA_DIR = ROOT / "data" / "科目四日志08172257"
OUTPUT_DIR = Path(__file__).resolve().parent
ROUTE_FILE = DATA_DIR / "nav_replay_route_table_08181919.h"
CSV_FILES = (
    "wifi_telemetry_20260818_175457_873_雷区未进.csv",
    "wifi_telemetry_20260818_182540_472.csv",
    "wifi_telemetry_20260818_182825_112.csv",
)
SPEED_TO_MM_S = 4.79
MINEFIELD_TRIGGER_SPEED_MM_S = 1500.0
MINEFIELD_CRAWL_SPEED_RATIO = 0.5
MINEFIELD_BRAKE_POLY_A = 0.00025
MINEFIELD_BRAKE_POLY_B = -0.2877
MINEFIELD_BRAKE_POLY_C = 887.0


@dataclass(frozen=True)
class TransitionRun:
    name: str
    source: str
    segment_index: int
    frame: pd.DataFrame
    bumpy_exit_row: int
    mine_spin_row: int
    bumpy_route_index: int
    mine_route_index: int

    @property
    def transition(self) -> pd.DataFrame:
        return self.frame.loc[self.bumpy_exit_row : self.mine_spin_row].copy()


def parse_route(path: Path) -> pd.DataFrame:
    """解析生成的 C 静态表，不依赖编译器或手工复制路线。"""
    field_pattern = re.compile(r"\{([^{}]+)\}")
    rows: list[dict[str, float | int]] = []
    for match in field_pattern.finditer(path.read_text(encoding="utf-8")):
        fields = [field.strip() for field in match.group(1).split(",")]
        if len(fields) != 7 or "uint8" not in fields[4]:
            continue
        rows.append(
            {
                "route_index": len(rows),
                "x": float(fields[0].removesuffix("f")),
                "y": float(fields[1].removesuffix("f")),
                "point_type": int(re.search(r"\)\s*(\d+)", fields[4]).group(1)),
                "target_speed": float(fields[5].removesuffix("f")),
                "curvature": float(fields[6].removesuffix("f")),
            }
        )
    route = pd.DataFrame(rows)
    if route.empty:
        raise ValueError(f"无法从路线表解析采样点: {path}")
    route["segment_mm"] = np.hypot(route.x.diff(), route.y.diff()).fillna(0.0)
    route["s_mm"] = route.segment_mm.cumsum()
    return route


def split_replay_segments(frame: pd.DataFrame) -> list[pd.DataFrame]:
    """loop 倒退代表板端重启/重新开始回放，不能把多次跑车拼成一段。"""
    frame = frame.loc[frame.g_replay_state.eq(1)].copy()
    if frame.empty:
        return []
    frame["run_segment"] = frame.loop.diff().lt(0).cumsum()
    return [part.reset_index(drop=True) for _, part in frame.groupby("run_segment", sort=True)]


def first_falling_edge(series: pd.Series) -> int | None:
    edges = np.flatnonzero(series.to_numpy()[1:] < series.to_numpy()[:-1])
    return int(edges[0] + 1) if len(edges) else None


def first_rising_edge_after(series: pd.Series, start: int) -> int | None:
    values = series.to_numpy()
    edges = np.flatnonzero(values[1:] > values[:-1]) + 1
    edges = edges[edges >= start]
    return int(edges[0]) if len(edges) else None


def route_index_for_type(route: pd.DataFrame, point_type: int) -> int:
    matched = route.loc[route.point_type.eq(point_type), "route_index"]
    if matched.empty:
        raise ValueError(f"路线表中缺少 point_type={point_type}")
    return int(matched.iloc[0])


def project_to_route(frame: pd.DataFrame, route_section: pd.DataFrame) -> pd.DataFrame:
    """用路线采样点最近邻估算沿线进度，仅用于可视化而非控制结论。"""
    route_xy = route_section[["x", "y"]].to_numpy()
    samples = frame[["nav_x", "nav_y"]].to_numpy()
    distances_sq = ((samples[:, None, :] - route_xy[None, :, :]) ** 2).sum(axis=2)
    nearest = distances_sq.argmin(axis=1)
    output = frame.copy()
    output["route_index_est"] = route_section.route_index.to_numpy()[nearest]
    output["s_from_bumpy_exit_mm"] = (
        route_section.s_mm.to_numpy()[nearest] - float(route_section.s_mm.iloc[0])
    )
    output["route_offset_mm"] = np.sqrt(distances_sq[np.arange(len(output)), nearest])
    return output


def find_transition_runs(route: pd.DataFrame) -> list[TransitionRun]:
    usecols = [
        "loop", "nav_x", "nav_y", "vx_body", "relative_yaw", "g_replay_state",
        "err_degree", "minefield_is_active", "g_special_action_trigger",
        "bumpy_road_is_active", "nav_replay_point_type", "target_speed_set",
        "speed_L", "speed_R", "slip_flag",
    ]
    bumpy_route_index = route_index_for_type(route, 50)
    mine_route_index = route_index_for_type(route, 1)
    route_section = route.loc[
        route.route_index.between(bumpy_route_index, mine_route_index)
    ].copy()
    mine_entry = route.loc[route.route_index.eq(mine_route_index)].iloc[0]
    transitions: list[TransitionRun] = []

    for filename in CSV_FILES:
        csv_path = DATA_DIR / filename
        if not csv_path.is_file():
            continue
        raw = pd.read_csv(csv_path, usecols=usecols)
        for segment_index, segment in enumerate(split_replay_segments(raw)):
            bumpy_exit = first_falling_edge(segment.bumpy_road_is_active)
            mine_spin = first_rising_edge_after(segment.minefield_is_active, bumpy_exit or 0)
            if bumpy_exit is None or mine_spin is None or mine_spin <= bumpy_exit:
                continue
            segment["time_s"] = (segment.loop - segment.loop.iloc[0]) / 1000.0
            segment["actual_forward_mm_s"] = -segment.vx_body
            segment["command_forward_mm_s"] = -segment.target_speed_set * SPEED_TO_MM_S
            segment = project_to_route(segment, route_section)
            segment["route_remaining_to_mine_mm"] = (
                float(route_section.s_mm.iloc[-1] - route_section.s_mm.iloc[0])
                - segment.s_from_bumpy_exit_mm
            )
            segment["straight_distance_to_mine_mm"] = np.hypot(
                segment.nav_x - float(mine_entry.x), segment.nav_y - float(mine_entry.y)
            )
            label = f"{csv_path.stem} / run {segment_index + 1}"
            transitions.append(
                TransitionRun(
                    name=label,
                    source=filename,
                    segment_index=segment_index,
                    frame=segment,
                    bumpy_exit_row=bumpy_exit,
                    mine_spin_row=mine_spin,
                    bumpy_route_index=bumpy_route_index,
                    mine_route_index=mine_route_index,
                )
            )
    return transitions


def summarize(run: TransitionRun) -> dict[str, float | int | str]:
    part = run.transition
    duration_s = float(part.time_s.iloc[-1] - part.time_s.iloc[0])
    dt_s = part.time_s.diff().fillna(0.0).clip(lower=0.0, upper=0.1)
    crawl_speed = MINEFIELD_TRIGGER_SPEED_MM_S * MINEFIELD_CRAWL_SPEED_RATIO
    stopped_s = float(dt_s[part.target_speed_set.abs().lt(1.0)].sum())
    yaw_blocked_s = float(
        dt_s[part.target_speed_set.abs().lt(1.0) & part.err_degree.abs().gt(35.0)].sum()
    )
    s_progress = float(part.s_from_bumpy_exit_mm.iloc[-1])
    initial_speed = float(part.actual_forward_mm_s.iloc[0])
    initial_brake_distance = (
        MINEFIELD_BRAKE_POLY_A * initial_speed * initial_speed
        + MINEFIELD_BRAKE_POLY_B * initial_speed
        + MINEFIELD_BRAKE_POLY_C
    )
    return {
        "run": run.name,
        "bumpy_exit_loop": int(part.loop.iloc[0]),
        "mine_spin_loop": int(part.loop.iloc[-1]),
        "transition_s": duration_s,
        "route_remaining_at_exit_mm": float(part.route_remaining_to_mine_mm.iloc[0]),
        "straight_distance_at_exit_mm": float(part.straight_distance_to_mine_mm.iloc[0]),
        "initial_actual_forward_mm_s": initial_speed,
        "brake_model_distance_at_exit_mm": initial_brake_distance,
        "estimated_progress_mm": s_progress,
        "mean_actual_forward_mm_s": float(part.actual_forward_mm_s.mean()),
        "max_actual_forward_mm_s": float(part.actual_forward_mm_s.max()),
        "mean_command_forward_mm_s": float(part.command_forward_mm_s.mean()),
        "zero_command_s": stopped_s,
        "crawl_command_s": float(dt_s[part.command_forward_mm_s.sub(crawl_speed).abs().lt(20.0)].sum()),
        "yaw_block_zero_s": yaw_blocked_s,
        "max_abs_err_deg": float(part.err_degree.abs().max()),
        "max_route_offset_mm": float(part.route_offset_mm.max()),
    }


def markdown_table(frame: pd.DataFrame) -> str:
    """避免依赖 pandas.to_markdown() 所需的可选 tabulate 包。"""
    rendered = frame.copy()
    for column in rendered.columns:
        if pd.api.types.is_float_dtype(rendered[column]):
            rendered[column] = rendered[column].map(lambda value: f"{value:.1f}")
    headers = [str(column) for column in rendered.columns]
    lines = [
        "| " + " | ".join(headers) + " |",
        "| " + " | ".join("---" for _ in headers) + " |",
    ]
    lines.extend(
        "| " + " | ".join(str(value) for value in row) + " |"
        for row in rendered.itertuples(index=False, name=None)
    )
    return "\n".join(lines)


def render(route: pd.DataFrame, runs: list[TransitionRun], summary: pd.DataFrame) -> None:
    if not runs:
        return
    primary = max(runs, key=lambda run: len(run.transition))
    part = primary.transition
    bumpy_exit = route.loc[route.route_index.eq(primary.bumpy_route_index)].iloc[0]
    mine_entry = route.loc[route.route_index.eq(primary.mine_route_index)].iloc[0]
    route_section = route.loc[
        route.route_index.between(primary.bumpy_route_index, primary.mine_route_index)
    ]

    plt.rcParams.update({"font.size": 9, "axes.unicode_minus": False})
    fig, axes = plt.subplots(2, 2, figsize=(15, 10), constrained_layout=True)
    ax = axes[0, 0]
    ax.plot(route_section.x, route_section.y, color="#6b7280", linewidth=2, label="route table")
    for run_index, run in enumerate(runs, start=1):
        phase = run.transition
        ax.plot(phase.nav_x, phase.nav_y, linewidth=1.4, alpha=0.8, label=f"run {run_index}")
    ax.scatter([bumpy_exit.x], [bumpy_exit.y], color="#d97706", s=65, zorder=3, label="bumpy exit (type 50)")
    ax.scatter([mine_entry.x], [mine_entry.y], color="#dc2626", s=65, zorder=3, label="mine entry (type 1)")
    ax.set_aspect("equal", adjustable="box")
    ax.set_title("Bumpy exit to mine entry: route and logged positions")
    ax.set_xlabel("nav x (mm)")
    ax.set_ylabel("nav y (mm)")
    ax.legend(fontsize=7, loc="best")
    ax.grid(alpha=0.25)

    ax = axes[0, 1]
    rel_time = part.time_s - part.time_s.iloc[0]
    ax.plot(rel_time, part.command_forward_mm_s, color="#2563eb", label="commanded forward speed")
    ax.plot(rel_time, part.actual_forward_mm_s, color="#16a34a", label="vx_body forward speed")
    ax.axvline(0.0, color="#d97706", linestyle="--", label="bumpy state ends")
    ax.axvline(rel_time.iloc[-1], color="#dc2626", linestyle="--", label="mine spin begins")
    ax.set_title("Speed transition: longest complete run")
    ax.set_xlabel("time after bumpy exit (s)")
    ax.set_ylabel("forward speed (mm/s)")
    ax.set_ylim(bottom=-300)
    ax.grid(alpha=0.25)
    ax.legend(fontsize=8, loc="best")

    ax = axes[1, 0]
    err_limit = max(40.0, float(part.err_degree.abs().max()) * 1.08)
    ax.plot(rel_time, part.err_degree, color="#7c3aed", label="err_degree")
    ax.fill_between(
        rel_time,
        -err_limit,
        err_limit,
        where=part.target_speed_set.abs().lt(1.0),
        color="#ef4444", alpha=0.15, label="zero speed command",
    )
    ax.axhline(35.0, color="#dc2626", linestyle=":", linewidth=1, label="mine yaw stop threshold")
    ax.axhline(-35.0, color="#dc2626", linestyle=":", linewidth=1)
    ax.set_ylim(-err_limit, err_limit)
    ax.set_title("Mine approach yaw gate and zero-command intervals")
    ax.set_xlabel("time after bumpy exit (s)")
    ax.set_ylabel("err_degree (deg)")
    ax.grid(alpha=0.25)
    ax.legend(fontsize=8, loc="best")

    ax = axes[1, 1]
    plotting = summary.sort_values("transition_s", ascending=True)
    y = np.arange(len(plotting))
    crawl = plotting.crawl_command_s.clip(lower=0.0)
    zero = plotting.zero_command_s.clip(lower=0.0)
    normal = (plotting.transition_s - crawl - zero).clip(lower=0.0)
    ax.barh(y, normal, color="#2563eb", label="normal moving command")
    ax.barh(y, crawl, left=normal, color="#f59e0b", label="750 mm/s crawl command")
    ax.barh(y, zero, left=normal + crawl, color="#ef4444", label="zero command")
    ax.set_yticks(y, [f"run {index + 1}" for index in range(len(plotting))])
    ax.set_xlabel("bumpy exit to mine spin (s)")
    ax.set_title("Delay breakdown across complete logged runs")
    ax.grid(axis="x", alpha=0.25)
    ax.legend(fontsize=8, loc="best")

    fig.savefig(OUTPUT_DIR / "bumpy_to_minefield_analysis.png", dpi=180)
    plt.close(fig)


def write_report(route: pd.DataFrame, runs: list[TransitionRun], summary: pd.DataFrame) -> None:
    bumpy_exit = route.loc[route.point_type.eq(50)].iloc[0]
    mine_entry = route.loc[route.point_type.eq(1)].iloc[0]
    planned_mm = float(mine_entry.s_mm - bumpy_exit.s_mm)
    route_section = route.loc[route.route_index.between(int(bumpy_exit.route_index), int(mine_entry.route_index))]
    route_peak = route_section.loc[route_section.target_speed.abs().idxmax()]
    crawl_speed = MINEFIELD_TRIGGER_SPEED_MM_S * MINEFIELD_CRAWL_SPEED_RATIO
    lines = [
        "# 颠簸路到雷区衔接日志分析",
        "",
        "输入：`nav_replay_route_table_08181919.h` 与 2026-08-18 三份 Wi-Fi 遥测。",
        "同一 CSV 中若 `loop` 回退，会拆成独立的回放片段；仅统计同时记录到颠簸退出和雷区开始转圈的片段。",
        "",
        "## 路线事实",
        "",
        f"- 颠簸出口 type=50：路表 index `{int(bumpy_exit.route_index)}`，坐标 `({bumpy_exit.x:.0f}, {bumpy_exit.y:.0f})`。",
        f"- 首个雷区 type=1：路表 index `{int(mine_entry.route_index)}`，坐标 `({mine_entry.x:.0f}, {mine_entry.y:.0f})`。",
        f"- 二者沿路表距离约 `{planned_mm:.0f} mm`；这段路线没有新的配对视觉任务，属于可提速的普通导航段。",
        f"- 离线速度表在该段峰值为 `{abs(route_peak.target_speed):.0f} rpm`（约 `{abs(route_peak.target_speed) * SPEED_TO_MM_S:.0f} mm/s`），位于出口后约 `{route_peak.s_mm - bumpy_exit.s_mm:.0f} mm`。",
        "",
        "## 实测汇总",
        "",
        markdown_table(summary),
        "",
        "## 根因",
        "",
        "- `BumpyRoad_Update_1ms()` 在视觉确认出口后仍执行 `BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM = 1500 mm`，期间继续独占速度控制。日志显示状态机真正结束时，按路线最近邻估计只剩约 2.5–3.9 m 到雷区；该距离不足以执行路表前半段的高速曲线。融合坐标有跳变，因此该数字用于量级判断而非定位精度。",
        "- `NavReplay_Process()` 只要下一个特殊点为 type=1，就无条件调用 `Plan4_ProcessMinefieldApproach()` 并返回，完全绕过普通 LQR 和离线路表速度。它不会等待距离缩短到专用的雷区接管门限。",
        f"- 雷区接近的实测首帧速度与当前刹车多项式对应的刹车距离见表。速度高于 `{MINEFIELD_TRIGGER_SPEED_MM_S:.0f} mm/s` 且距离已小于该模型值时，代码立即将 `target_speed_set` 置零；速度降到阈值后，改为固定 `{crawl_speed:.0f} mm/s` 蠕行直到 250 mm 执行圆。两份完整日志的 `crawl_command_s` 正是主要时间损失。",
        "- 红色阴影是零速度指令。航向门限 +/-35 度在一份日志中只贡献约 0.06 s，另一份没有触发；它不是本次 3.6–5.9 s 过渡的主因。20 个导航周期的交接斜坡同样不足以解释该量级。",
        "",
        "## 建议的优化顺序",
        "",
        "1. 先改颠簸退出时机，而非盲目提高雷区速度：把视觉出口后的 `BUMPY_ROAD_POST_CORRECTION_DISTANCE_MM` 逐档试为 1000、750、500 mm，并记录坐标锚定后的横向误差。目标是把至少 4–5 m 的普通路段交还 Plan4，同时保持退出重定位稳定。",
        "2. 增加雷区的 LQR 接管距离：type=1 在远距离时仍按路表跟踪；接近到由当前实测速度和刹车模型决定的安全距离时，再进入 `Plan4_ProcessMinefieldApproach()`。现有代码对 type=1 无条件提前接管，是离线速度曲线失效的直接原因。",
        "3. 将雷区接近改成连续的速度包络，而不是“零指令减到 1500 mm/s 后固定 750 mm/s”。例如以 `v^2 = 2*a*(distance-execute_radius)` 为主曲线，并对实测速度超出曲线的部分施加受限减速度；只有安全越界才使用零指令。这样能避免长距离蠕行，同时不削弱超速保护。",
        "4. 最后再改轨迹：保持 type=50 到 type=1 的直线末端切向圆心，避免最后 +/-35 度才原地对正。路线几何优化有价值，但在第 2 条之前不会提高实际速度，因为当前实现根本不消费这段路表。",
        "5. 用同轮胎、同电压、同场地重测刹车距离，再拟合 `PLAN4_MINEFIELD_BRAKE_POLY_*`。在未验证前不要只调低 `PLAN4_MINEFIELD_BRAKE_DIST_RATIO` 或调高触发速度，这会直接缩小转圈前的停车裕度。",
        "",
        "图表：`bumpy_to_minefield_analysis.png`。最近邻路线投影仅用于判断趋势；融合坐标会在视觉锚定时跳变，因此不用于控制精度结论。",
    ]
    (OUTPUT_DIR / "bumpy_to_minefield_analysis.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    route = parse_route(ROUTE_FILE)
    runs = find_transition_runs(route)
    if not runs:
        raise SystemExit("没有在指定日志中找到完整的‘颠簸结束 -> 雷区转圈开始’回放片段。")
    summary = pd.DataFrame([summarize(run) for run in runs])
    summary.to_csv(OUTPUT_DIR / "bumpy_to_minefield_summary.csv", index=False, encoding="utf-8-sig")
    render(route, runs, summary)
    write_report(route, runs, summary)
    print(summary.to_string(index=False))
    print(f"wrote: {OUTPUT_DIR / 'bumpy_to_minefield_analysis.png'}")


if __name__ == "__main__":
    main()
