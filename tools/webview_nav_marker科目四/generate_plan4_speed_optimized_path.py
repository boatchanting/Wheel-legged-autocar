#!/usr/bin/env python3
from __future__ import annotations
"""
这个代码对比求path来说进行了路径优化，允许路径进行少量偏移以弥补打点所造成的不准确，是一次成功的尝试，但是方案尚且不成熟
"""
"""
prompt:
一个可能的猜测，通过速度热力图发现速度控制存在严重损失，应该可以允许规划器对我打的路径点(type为0的点)做适度的偏移（离下个状态机入口越近的点允许的偏移值越小）或者拟合以适应最佳的速度曲线，毕竟人手打的点是存在问题的
code\navigation\nav_replay_route_table.h
tools\webview_nav_marker科目四\nav_mark_points_{time}.csv
"""
"""
使用的示例方法：
PS generate_plan4_speed_optimized_path.py --step-sizes-mm 300,150,75,35 --sweeps-per-step 3
"""
"""对 Plan4 普通锚点做受约束优化，生成更平顺、更高速度的路径。

录制的 ``type=0`` 点是软约束；所有特殊任务锚点及其生成的直线走廊均为
硬约束。每一个可移动普通点都有位移预算，且越接近下一个状态机入口，
预算越小，直至锁定为零。

评分直接复用 ``generate_plan4_smooth_path.py`` 的五次 G2 几何和限速模型：
降低由曲率造成的速度损失，同时惩罚曲率尖峰和不必要的大幅位移。由于本
工具没有赛道边界或障碍物地图，导出的路线必须先审图、再进行低速实车验证。
"""



import argparse
import csv
import math
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Sequence

import matplotlib.pyplot as plt
import numpy as np

import generate_plan4_smooth_path as base


SCRIPT_DIR = Path(__file__).resolve().parent

# 普通点到下一个任务入口的距离超过 FULL 时，可以使用完整位移预算；
# 在 LOCK 与 FULL 之间，位移预算线性增大。
DEFAULT_MAX_OFFSET_MM = 600.0
DEFAULT_ENTRY_LOCK_DISTANCE_MM = 800.0
DEFAULT_ENTRY_FULL_OFFSET_DISTANCE_MM = 3000.0

# 从粗到细的坐标下降步长。八方向搜索既避免偏向坐标轴，也能保证结果可复现。
DEFAULT_STEP_SIZES_MM = (260.0, 130.0, 65.0)
DEFAULT_SWEEPS_PER_STEP = 2
EVALUATION_SAMPLE_STEP_MM = 75.0
# 候选搜索只需保证评分相对一致，无需使用最终路表的 5 mm 稠密几何。
# 优化结束后仍会恢复原脚本的稠密步长，重新生成最终路径。
EVALUATION_DENSE_SAMPLE_STEP_MM = 50.0

# 评分配置。原生成器的横向加速度和角速度限速都直接受曲率影响；
# 曲率峰值项防止很短的尖弯被全路线平均速度掩盖。
SPEED_LOSS_WEIGHT = 1.0
PEAK_CURVATURE_WEIGHT = 0.18
OFFSET_WEIGHT = 0.045
CURVATURE_REFERENCE_PER_MM = 0.0015


@dataclass(frozen=True)
class OptimizationConfig:
    max_offset_mm: float
    entry_lock_distance_mm: float
    entry_full_offset_distance_mm: float
    step_sizes_mm: tuple[float, ...]
    sweeps_per_step: int


@dataclass(frozen=True)
class RouteMetrics:
    score: float
    mean_speed_mm_s: float
    p05_speed_mm_s: float
    max_abs_curvature: float
    speed_loss: float
    peak_curvature_penalty: float
    offset_penalty: float


@dataclass(frozen=True)
class IterationRecord:
    """一次普通点坐标搜索后的结果，供 CSV 日志回放和调参。"""

    step_mm: float
    sweep: int
    marker_index: int
    previous_x: float
    previous_y: float
    selected_x: float
    selected_y: float
    accepted: bool
    metrics: RouteMetrics


def find_latest_marker_csv() -> Path:
    candidates = [
        path for path in SCRIPT_DIR.glob("nav_mark_points_*.csv")
        if "_planned" not in path.stem and "_optimized" not in path.stem
    ]
    if not candidates:
        raise FileNotFoundError("No source nav_mark_points_*.csv was found.")
    return max(candidates, key=lambda path: path.stat().st_mtime)


def marker_distance_to_next_entry(markers: Sequence[base.Marker]) -> list[float]:
    """计算每个原始锚点沿人工折线到下一个入口锚点的距离。"""
    result = [math.inf] * len(markers)
    next_entry: int | None = None
    accumulated = 0.0
    for index in range(len(markers) - 1, -1, -1):
        if markers[index].point_type in base.ENTRY_TYPES:
            next_entry = index
            accumulated = 0.0
            result[index] = 0.0
        elif next_entry is not None:
            accumulated += base.distance(markers[index], markers[index + 1])
            result[index] = accumulated
    return result


def allowed_offsets(markers: Sequence[base.Marker], config: OptimizationConfig) -> list[float]:
    distances = marker_distance_to_next_entry(markers)
    allowed: list[float] = []
    span = config.entry_full_offset_distance_mm - config.entry_lock_distance_mm
    if span <= 0.0:
        raise ValueError("entry full-offset distance must exceed lock distance")
    for marker, distance_to_entry in zip(markers, distances):
        if marker.point_type != 0:
            allowed.append(0.0)
            continue
        if math.isinf(distance_to_entry):
            allowed.append(config.max_offset_mm)
            continue
        ratio = (distance_to_entry - config.entry_lock_distance_mm) / span
        allowed.append(config.max_offset_mm * max(0.0, min(1.0, ratio)))
    return allowed


def clamp_to_offset_budget(
    candidate: base.Marker,
    original: base.Marker,
    max_offset_mm: float,
) -> base.Marker:
    dx = candidate.x - original.x
    dy = candidate.y - original.y
    distance = math.hypot(dx, dy)
    if distance <= max_offset_mm or distance <= 1e-9:
        return candidate
    scale = max_offset_mm / distance
    return replace(candidate, x=original.x + dx * scale, y=original.y + dy * scale)


def evaluate_route(
    markers: Sequence[base.Marker],
    originals: Sequence[base.Marker],
    budgets: Sequence[float],
) -> tuple[RouteMetrics, list[base.PathSample], np.ndarray, np.ndarray, np.ndarray]:
    """生成路径并按纯几何速度评分，刻意不计入任务本身的限速。

    台阶/单边桥限速与雷区停车是设计要求，不应被误判为普通人工点导致的
    速度损失。因此评分时仅将样本点类型临时清零以计算速度，路径几何仍然
    保留特殊任务锚点及其不可移动的直线走廊。
    """
    corrected = base.apply_special_exit_corrections(list(markers), base.find_event_pairs(list(markers)))
    original_dense_step = base.DENSE_SAMPLE_STEP_MM
    try:
        base.DENSE_SAMPLE_STEP_MM = EVALUATION_DENSE_SAMPLE_STEP_MM
        samples = base.generate_path(corrected, EVALUATION_SAMPLE_STEP_MM)
    finally:
        base.DENSE_SAMPLE_STEP_MM = original_dense_step
    s, yaw, curvature = base.calculate_yaw_and_curvature(samples)
    geometry_only = [replace(sample, point_type=0) for sample in samples]
    target_speed = base.calculate_target_speed(geometry_only, s, curvature)
    speeds = np.abs(target_speed) * base.SPEED_TO_MM_S

    speed_loss = float(np.mean(((base.PATH_SPEED_MAX_MM_S - speeds) / base.PATH_SPEED_MAX_MM_S) ** 2))
    max_abs_curvature = float(np.max(np.abs(curvature))) if len(curvature) else 0.0
    peak_curvature_penalty = (max_abs_curvature / CURVATURE_REFERENCE_PER_MM) ** 2

    ratios = []
    for marker, original, budget in zip(markers, originals, budgets):
        if budget > 1e-6:
            ratios.append(((marker.x - original.x) ** 2 + (marker.y - original.y) ** 2) / (budget * budget))
    offset_penalty = float(np.mean(ratios)) if ratios else 0.0
    score = (SPEED_LOSS_WEIGHT * speed_loss +
             PEAK_CURVATURE_WEIGHT * peak_curvature_penalty +
             OFFSET_WEIGHT * offset_penalty)
    metrics = RouteMetrics(
        score=score,
        mean_speed_mm_s=float(np.mean(speeds)),
        p05_speed_mm_s=float(np.percentile(speeds, 5.0)),
        max_abs_curvature=max_abs_curvature,
        speed_loss=speed_loss,
        peak_curvature_penalty=peak_curvature_penalty,
        offset_penalty=offset_penalty,
    )
    return metrics, samples, s, yaw, curvature


def optimize_markers(
    originals: Sequence[base.Marker],
    config: OptimizationConfig,
) -> tuple[list[base.Marker], list[float], RouteMetrics, list[IterationRecord]]:
    """在普通锚点上执行有界、确定性的坐标下降搜索。"""
    markers = list(originals)
    budgets = allowed_offsets(originals, config)
    movable_indices = [
        index for index, budget in enumerate(budgets)
        if originals[index].point_type == 0 and budget > 1e-6
    ]
    if not movable_indices:
        raise ValueError("No type=0 marker has a non-zero optimization budget.")

    directions = [(0.0, 0.0)]
    directions.extend((math.cos(math.radians(degree)), math.sin(math.radians(degree)))
                      for degree in range(0, 360, 45))
    best_metrics, _, _, _, _ = evaluate_route(markers, originals, budgets)
    records: list[IterationRecord] = []

    for step in config.step_sizes_mm:
        for sweep in range(1, config.sweeps_per_step + 1):
            any_improved = False
            for index in movable_indices:
                current = markers[index]
                selected = current
                selected_metrics = best_metrics
                for direction_x, direction_y in directions:
                    candidate = replace(
                        current,
                        x=current.x + step * direction_x,
                        y=current.y + step * direction_y,
                    )
                    candidate = clamp_to_offset_budget(candidate, originals[index], budgets[index])
                    trial = list(markers)
                    trial[index] = candidate
                    trial_metrics, _, _, _, _ = evaluate_route(trial, originals, budgets)
                    if trial_metrics.score + 1e-9 < selected_metrics.score:
                        selected = candidate
                        selected_metrics = trial_metrics
                if selected != current:
                    markers[index] = selected
                    best_metrics = selected_metrics
                    any_improved = True
                records.append(IterationRecord(
                    step_mm=step,
                    sweep=sweep,
                    marker_index=originals[index].order,
                    previous_x=current.x,
                    previous_y=current.y,
                    selected_x=selected.x,
                    selected_y=selected.y,
                    accepted=(selected != current),
                    metrics=selected_metrics,
                ))
            if not any_improved:
                break

    final_metrics, _, _, _, _ = evaluate_route(markers, originals, budgets)
    return markers, budgets, final_metrics, records


def write_optimized_markers(
    output: Path,
    original: Sequence[base.Marker],
    optimized: Sequence[base.Marker],
    budgets: Sequence[float],
) -> None:
    with output.open("w", encoding="utf-8", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow([
            "index", "x", "y", "point_type", "relative_yaw", "heading",
            "original_x", "original_y", "offset_mm", "max_offset_mm",
        ])
        for old, new, budget in zip(original, optimized, budgets):
            offset = math.hypot(new.x - old.x, new.y - old.y)
            writer.writerow([
                new.order, f"{new.x:.3f}", f"{new.y:.3f}", new.point_type,
                "" if new.relative_yaw is None else f"{new.relative_yaw:.3f}",
                "" if new.heading is None else f"{new.heading:.3f}",
                f"{old.x:.3f}", f"{old.y:.3f}", f"{offset:.3f}", f"{budget:.3f}",
            ])


def write_iteration_log(output: Path, records: Sequence[IterationRecord]) -> None:
    """导出每次普通点搜索后的选点结果，不记录大量被淘汰的八方向候选。"""
    with output.open("w", encoding="utf-8", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow([
            "step_mm", "sweep", "marker_index", "previous_x", "previous_y",
            "selected_x", "selected_y", "accepted", "score", "mean_speed_mm_s",
            "p05_speed_mm_s", "max_abs_curvature_per_mm",
        ])
        for record in records:
            writer.writerow([
                f"{record.step_mm:.3f}", record.sweep, record.marker_index,
                f"{record.previous_x:.3f}", f"{record.previous_y:.3f}",
                f"{record.selected_x:.3f}", f"{record.selected_y:.3f}",
                int(record.accepted), f"{record.metrics.score:.8f}",
                f"{record.metrics.mean_speed_mm_s:.3f}",
                f"{record.metrics.p05_speed_mm_s:.3f}",
                f"{record.metrics.max_abs_curvature:.8f}",
            ])


def render_comparison(
    output: Path,
    original: Sequence[base.Marker],
    optimized: Sequence[base.Marker],
    original_samples: Sequence[base.PathSample],
    optimized_samples: Sequence[base.PathSample],
    budgets: Sequence[float],
) -> None:
    figure, axis = plt.subplots(figsize=(12, 8))
    old_path = np.array([(point.x, point.y) for point in original_samples])
    new_path = np.array([(point.x, point.y) for point in optimized_samples])
    axis.plot(old_path[:, 0], old_path[:, 1], color="#94a3b8", linewidth=1.5, label="recorded-anchor route")
    axis.plot(new_path[:, 0], new_path[:, 1], color="#0f766e", linewidth=2.3, label="optimized route")

    for old, new, budget in zip(original, optimized, budgets):
        if old.point_type == 0:
            axis.scatter(old.x, old.y, color="#64748b", s=22, zorder=3)
            if budget > 0.0:
                circle = plt.Circle((old.x, old.y), budget, color="#0f766e", alpha=0.06, linewidth=0)
                axis.add_patch(circle)
            if math.hypot(new.x - old.x, new.y - old.y) > 1.0:
                axis.plot((old.x, new.x), (old.y, new.y), color="#14b8a6", linewidth=0.9, alpha=0.75)
                axis.scatter(new.x, new.y, color="#0f766e", s=25, zorder=4)
        else:
            axis.scatter(old.x, old.y, color="#dc2626", marker="s", s=58, edgecolor="black", linewidth=0.5, zorder=5)
            axis.annotate(f"type {old.point_type}", (old.x, old.y), xytext=(5, 5), textcoords="offset points", fontsize=8)

    axis.set_aspect("equal", adjustable="box")
    axis.set_xlabel("X (mm)")
    axis.set_ylabel("Y (mm)")
    axis.set_title("Plan4 constrained ordinary-anchor optimization")
    axis.grid(True, alpha=0.25)
    axis.legend(loc="best")
    figure.tight_layout()
    figure.savefig(output, dpi=180)
    plt.close(figure)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, help="输入标记 CSV；默认选择目录中最新源文件")
    parser.add_argument("--max-offset-mm", type=float, default=DEFAULT_MAX_OFFSET_MM,
                        help="远离任务入口时 type=0 点允许的最大偏移，默认 600")
    parser.add_argument("--entry-lock-distance-mm", type=float, default=DEFAULT_ENTRY_LOCK_DISTANCE_MM,
                        help="距下一个任务入口不超过此值时锁死普通点，默认 800")
    parser.add_argument("--entry-full-offset-distance-mm", type=float,
                        default=DEFAULT_ENTRY_FULL_OFFSET_DISTANCE_MM,
                        help="距下一个任务入口达到此值时使用完整偏移预算，默认 3000")
    parser.add_argument("--step-sizes-mm", default=",".join(str(value) for value in DEFAULT_STEP_SIZES_MM),
                        help="从粗到细的搜索步长列表，逗号分隔，默认 260,130,65")
    parser.add_argument("--sweeps-per-step", type=int, default=DEFAULT_SWEEPS_PER_STEP,
                        help="每个步长对所有可移动点完整扫描的次数，默认 2")
    parser.add_argument("--output-markers", type=Path, help="优化后锚点 CSV 输出路径")
    parser.add_argument("--output-csv", type=Path, help="优化后路径 CSV 输出路径")
    parser.add_argument("--render", type=Path, help="原始/优化路径对比图输出路径")
    parser.add_argument("--speed-heatmap", type=Path, help="最终路径速度热力图输出路径")
    parser.add_argument("--iteration-log", type=Path, help="迭代记录 CSV 输出路径")
    parser.add_argument(
        "--header",
        type=Path,
        help="C 路表输出路径；审图验证后才传 code/navigation/nav_replay_route_table.h 覆盖运行表",
    )
    return parser.parse_args()


def parse_step_sizes(value: str) -> tuple[float, ...]:
    """解析命令行步长列表，并拒绝零值、负值及空列表。"""
    try:
        steps = tuple(float(item.strip()) for item in value.split(",") if item.strip())
    except ValueError as exc:
        raise ValueError("step-sizes-mm 必须是逗号分隔的正数") from exc
    if not steps or any(step <= 0.0 for step in steps):
        raise ValueError("step-sizes-mm 至少包含一个正数")
    return steps


def main() -> int:
    args = parse_args()
    if args.max_offset_mm < 0.0 or args.sweeps_per_step < 1:
        raise ValueError("最大偏移不能为负数，扫描次数必须为正数")
    source = args.input.resolve() if args.input else find_latest_marker_csv()
    original, start_heading = base.read_markers(source)
    config = OptimizationConfig(
        max_offset_mm=args.max_offset_mm,
        entry_lock_distance_mm=args.entry_lock_distance_mm,
        entry_full_offset_distance_mm=args.entry_full_offset_distance_mm,
        step_sizes_mm=parse_step_sizes(args.step_sizes_mm),
        sweeps_per_step=args.sweeps_per_step,
    )

    original_metrics, original_samples, _, _, _ = evaluate_route(
        original, original, allowed_offsets(original, config))
    optimized, budgets, optimized_metrics, records = optimize_markers(original, config)
    final_markers = base.apply_special_exit_corrections(optimized, base.find_event_pairs(optimized))
    samples, sample_step = base.generate_path_with_point_cap(final_markers)
    s, yaw, curvature = base.calculate_yaw_and_curvature(samples)
    target_speed = base.calculate_target_speed(samples, s, curvature)
    target_speed = base.limit_stairs_approach_output_speed(samples, s, target_speed, base.STAIRS_APPROACH_DISTANCE_MM)
    target_speed = base.limit_bridge_approach_output_speed(
        samples, s, target_speed, base.BRIDGE_APPROACH_DISTANCE_MM, base.BRIDGE_APPROACH_TARGET_SPEED_MAX)

    prefix = source.with_name(f"{source.stem}_speed_optimized")
    output_markers = args.output_markers or prefix.with_name(f"{prefix.name}_markers.csv")
    output_csv = args.output_csv or prefix.with_suffix(".csv")
    render_output = args.render or prefix.with_suffix(".png")
    heatmap_output = args.speed_heatmap or prefix.with_name(f"{prefix.name}_heatmap.png")
    iteration_log_output = args.iteration_log or prefix.with_name(f"{prefix.name}_iteration_log.csv")
    output_header = args.header or prefix.with_name(f"{prefix.name}_route_table.h")
    write_optimized_markers(output_markers, original, optimized, budgets)
    write_iteration_log(iteration_log_output, records)
    base.write_csv(output_csv, samples, yaw, curvature, target_speed)
    base.write_c_header(output_header, samples, yaw, curvature, target_speed, source, start_heading)
    render_comparison(render_output, original, optimized, original_samples, samples, budgets)
    base.render_speed_heatmap(heatmap_output, samples, target_speed)

    moved = [math.hypot(new.x - old.x, new.y - old.y) for old, new in zip(original, optimized)]
    print(f"输入: {source}")
    print(f"评分: {original_metrics.score:.6f} -> {optimized_metrics.score:.6f}")
    print(f"几何速度均值: {original_metrics.mean_speed_mm_s:.0f} -> {optimized_metrics.mean_speed_mm_s:.0f} mm/s")
    print(f"几何速度 P05: {original_metrics.p05_speed_mm_s:.0f} -> {optimized_metrics.p05_speed_mm_s:.0f} mm/s")
    print(f"最大 |曲率|: {original_metrics.max_abs_curvature:.6f} -> {optimized_metrics.max_abs_curvature:.6f} 1/mm")
    print(f"最大锚点偏移: {max(moved):.1f} mm；最终采样间距: {sample_step:.1f} mm")
    print(f"优化锚点: {output_markers}")
    print(f"迭代记录: {iteration_log_output}（共 {len(records)} 行）")
    print(f"路径 CSV: {output_csv}")
    print(f"对比图: {render_output}")
    print(f"速度热力图: {heatmap_output}")
    print(f"C 路表: {output_header}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, ValueError) as exc:
        print(f"错误: {exc}")
        raise SystemExit(2) from exc
