#!/usr/bin/env python3
"""G2 曲线、掉头桩绕行与按弧长重采样等路径几何算法。"""
from __future__ import annotations

import math

import numpy as np

from .plan4_models import (
    DENSE_SAMPLE_STEP_MM,
    LOCAL_CORNER_HANDLE_MAX_MM,
    LOCAL_CORNER_HANDLE_RATIO,
    LOCAL_CORNER_NEIGHBOR_RATIO,
    MIN_LINK_LENGTH_MM,
    NAV_ROUTE_MAX_POINTS,
    PARALLEL_TRANSITION_HANDLE_MM,
    POINT_TO_LINE_MAX_END_HANDLE_RATIO,
    POINT_TO_LINE_MAX_START_HANDLE_RATIO,
    POINT_TO_LINE_MIN_END_HANDLE_RATIO,
    POINT_TO_LINE_MIN_START_HANDLE_RATIO,
    SAMPLE_STEP_MM,
    SPEED_TO_MM_S,
    START_POINT_X_MM,
    START_POINT_Y_MM,
    TRANSITION_PRESET_LABEL,
    TYPE_LABEL,
    PathSample,
    Marker,
    Node,
    SpeedPlanningProfile,
    TaskSpeedProfile,
    TransitionPlan,
    TransitionPreset,
)
from .plan4_route import distance, find_event_pairs, unit
from .plan4_speed import (
    apply_longitudinal_speed_envelope,
    calculate_target_speed,
    calculate_yaw_and_curvature,
)

def add_sample(
    samples: list[PathSample], point: np.ndarray, point_type: int, forced: bool,
    segment: str, marker_order: Optional[int] = None,
) -> None:
    if samples and math.hypot(samples[-1].x - point[0], samples[-1].y - point[1]) < 0.01:
        # 若事件点与前一个几何点重合，保留事件标签。
        if point_type != 0:
            samples[-1].point_type = point_type
            samples[-1].marker_order = marker_order
        samples[-1].forced_straight |= forced
        return
    samples.append(PathSample(float(point[0]), float(point[1]), point_type, forced, segment, marker_order))


def append_line(samples: list[PathSample], start: Node, end: Node, segment: str, forced: bool, sample_step_mm: float) -> None:
    start_vec = np.array([start.x, start.y], dtype=float)
    end_vec = np.array([end.x, end.y], dtype=float)
    length = float(np.linalg.norm(end_vec - start_vec))
    count = max(1, int(math.ceil(length / sample_step_mm)))
    for i in range(count + 1):
        ratio = i / count
        is_start = i == 0
        is_end = i == count
        event_type = start.point_type if is_start else (end.point_type if is_end else 0)
        marker_order = start.marker_order if is_start else (end.marker_order if is_end else None)
        add_sample(samples, start_vec + ratio * (end_vec - start_vec), event_type, forced, segment, marker_order)


def bezier_quintic(control: np.ndarray, t: np.ndarray) -> np.ndarray:
    omt = 1.0 - t
    return (
        (omt**5)[:, None] * control[0]
        + (5.0 * omt**4 * t)[:, None] * control[1]
        + (10.0 * omt**3 * t**2)[:, None] * control[2]
        + (10.0 * omt**2 * t**3)[:, None] * control[3]
        + (5.0 * omt * t**4)[:, None] * control[4]
        + (t**5)[:, None] * control[5]
    )


def append_g2_link(samples: list[PathSample], start: Node, end: Node, segment: str, sample_step_mm: float) -> None:
    """Append a dense quintic G2 link; the whole route is resampled later."""
    start_vec = np.array([start.x, start.y], dtype=float)
    end_vec = np.array([end.x, end.y], dtype=float)
    chord = end_vec - start_vec
    chord_length = float(np.linalg.norm(chord))
    if chord_length < MIN_LINK_LENGTH_MM:
        add_sample(samples, end_vec, end.point_type, False, segment, end.marker_order)
        return

    tangent_start = unit(start.tangent if start.tangent is not None else chord)
    tangent_end = unit(end.tangent if end.tangent is not None else chord)
    # 两端控制柄独立限幅，等效于局部圆角，使曲线靠近相邻锚点，避免短边引起过度外扩。
    default_handle = min(LOCAL_CORNER_HANDLE_RATIO * chord_length, LOCAL_CORNER_HANDLE_MAX_MM)
    # 显式 corner_handle_mm 用于专用过渡段；普通节点仍使用 default_handle。
    start_handle = start.corner_handle_mm if start.corner_handle_mm is not None else default_handle
    end_handle = end.corner_handle_mm if end.corner_handle_mm is not None else default_handle
    # 控制柄不能达到整条弦长，否则五次曲线会出现回折。
    max_safe_handle = 0.45 * chord_length
    start_handle = min(start_handle, max_safe_handle)
    end_handle = min(end_handle, max_safe_handle)
    control = np.array([
        start_vec,
        start_vec + start_handle * tangent_start,
        start_vec + 2.0 * start_handle * tangent_start,
        end_vec - 2.0 * end_handle * tangent_end,
        end_vec - end_handle * tangent_end,
        end_vec,
    ])

    dense = bezier_quintic(control, np.linspace(0.0, 1.0, max(80, int(chord_length / sample_step_mm))))
    chord_lengths = np.linalg.norm(np.diff(dense, axis=0), axis=1)
    arclength = np.concatenate(([0.0], np.cumsum(chord_lengths)))
    sample_s = np.arange(0.0, arclength[-1], sample_step_mm)
    if not math.isclose(sample_s[-1] if len(sample_s) else 0.0, arclength[-1], abs_tol=0.01):
        sample_s = np.append(sample_s, arclength[-1])
    x = np.interp(sample_s, arclength, dense[:, 0])
    y = np.interp(sample_s, arclength, dense[:, 1])
    for index, (px, py) in enumerate(zip(x, y)):
        is_start = index == 0
        is_end = index == len(x) - 1
        point_type = start.point_type if is_start else (end.point_type if is_end else 0)
        marker_order = start.marker_order if is_start else (end.marker_order if is_end else None)
        add_sample(samples, np.array([px, py]), point_type, False, segment, marker_order)


def point_to_line_control(start: Node, end: Node) -> np.ndarray:
    """Choose a low-curvature quintic link from a free point to a directed line.

    The minefield exit has no valid outgoing heading.  Searching it locally is
    preferable to inheriting the previous state-machine segment's tangent,
    while the target task corridor remains an exact terminal heading constraint.
    """
    start_vec = np.array([start.x, start.y], dtype=float)
    end_vec = np.array([end.x, end.y], dtype=float)
    chord = end_vec - start_vec
    chord_length = float(np.linalg.norm(chord))
    if chord_length < MIN_LINK_LENGTH_MM:
        return np.repeat(end_vec[None, :], 6, axis=0)

    chord_direction = unit(chord)
    end_direction = unit(end.tangent if end.tangent is not None else chord)
    chord_angle = math.atan2(chord_direction[1], chord_direction[0])
    start_handle_ratios = np.linspace(
        POINT_TO_LINE_MIN_START_HANDLE_RATIO,
        POINT_TO_LINE_MAX_START_HANDLE_RATIO,
        5,
    )
    end_handle_ratios = np.linspace(
        POINT_TO_LINE_MIN_END_HANDLE_RATIO,
        POINT_TO_LINE_MAX_END_HANDLE_RATIO,
        5,
    )
    # 自由离场仍限定在朝向目标的半平面内，排除出点后先反向再折返的曲线。
    departure_offsets = np.linspace(-0.48 * math.pi, 0.48 * math.pi, 49)
    sample_t = np.linspace(0.0, 1.0, 121)
    best_control: np.ndarray | None = None
    best_score = math.inf

    for offset in departure_offsets:
        departure_direction = np.array(
            [math.cos(chord_angle + offset), math.sin(chord_angle + offset)], dtype=float
        )
        for start_ratio in start_handle_ratios:
            for end_ratio in end_handle_ratios:
                start_handle = start_ratio * chord_length
                end_handle = end_ratio * chord_length
                control = np.array([
                    start_vec,
                    start_vec + start_handle * departure_direction,
                    start_vec + 2.0 * start_handle * departure_direction,
                    end_vec - 2.0 * end_handle * end_direction,
                    end_vec - end_handle * end_direction,
                    end_vec,
                ])
                curve = bezier_quintic(control, sample_t)
                progress = curve @ chord_direction
                # 数值容差保留极小波动；超过 0.5% 弦长的后退必然形成不易跟踪的回环。
                if np.min(np.diff(progress)) < -0.005 * chord_length:
                    continue
                edges = np.diff(curve, axis=0)
                edge_length = np.linalg.norm(edges, axis=1)
                if np.any(edge_length < 1e-6):
                    continue
                heading = np.unwrap(np.arctan2(edges[:, 1], edges[:, 0]))
                curvature = np.diff(heading) / ((edge_length[:-1] + edge_length[1:]) * 0.5)
                normalized_curvature = curvature * chord_length
                length_ratio = float(np.sum(edge_length) / chord_length)
                # 峰值曲率优先，曲率能量用于抑制同样峰值下的局部急弯。
                score = (
                    float(np.max(np.abs(normalized_curvature)) ** 2)
                    + 0.35 * float(np.mean(normalized_curvature ** 2))
                    + 0.04 * (length_ratio - 1.0)
                )
                if score < best_score:
                    best_score = score
                    best_control = control

    if best_control is not None:
        return best_control
    # 极端的终端走廊方向可能与目标方向相背。此时仍返回确定性的退化解，
    # 后续速度规划会根据曲率限速，而不会留下空路径。
    fallback_handle = 0.22 * chord_length
    return np.array([
        start_vec,
        start_vec + fallback_handle * chord_direction,
        start_vec + 2.0 * fallback_handle * chord_direction,
        end_vec - 2.0 * fallback_handle * end_direction,
        end_vec - fallback_handle * end_direction,
        end_vec,
    ])


def append_point_to_line_g2(
    samples: list[PathSample], start: Node, end: Node, segment: str, sample_step_mm: float
) -> None:
    """Append a free-heading point-to-line G2 link without mutating either neighbor."""
    start_vec = np.array([start.x, start.y], dtype=float)
    end_vec = np.array([end.x, end.y], dtype=float)
    chord_length = float(np.linalg.norm(end_vec - start_vec))
    if chord_length < MIN_LINK_LENGTH_MM:
        add_sample(samples, end_vec, end.point_type, False, segment, end.marker_order)
        return
    control = point_to_line_control(start, end)
    dense = bezier_quintic(control, np.linspace(0.0, 1.0, max(120, int(chord_length / sample_step_mm))))
    chord_lengths = np.linalg.norm(np.diff(dense, axis=0), axis=1)
    arclength = np.concatenate(([0.0], np.cumsum(chord_lengths)))
    sample_s = np.arange(0.0, arclength[-1], sample_step_mm)
    if not math.isclose(sample_s[-1] if len(sample_s) else 0.0, arclength[-1], abs_tol=0.01):
        sample_s = np.append(sample_s, arclength[-1])
    x = np.interp(sample_s, arclength, dense[:, 0])
    y = np.interp(sample_s, arclength, dense[:, 1])
    for index, (px, py) in enumerate(zip(x, y)):
        is_start = index == 0
        is_end = index == len(x) - 1
        point_type = start.point_type if is_start else (end.point_type if is_end else 0)
        marker_order = start.marker_order if is_start else (end.marker_order if is_end else None)
        add_sample(samples, np.array([px, py]), point_type, False, segment, marker_order)


def append_turnaround_candidate(
    samples: list[PathSample],
    start: Node,
    end: Node,
    stake: Marker,
    safety_radius_mm: float,
    start_angle_rad: float,
    sweep_angle_rad: float,
    direction: int,
    segment_prefix: str,
    must_pass_markers: tuple[Marker, ...] = (),
    must_pass_tolerance_mm: float = 20.0,
    sample_step_mm: float = DENSE_SAMPLE_STEP_MM,
) -> None:
    """生成一条可检验的绕桩 G2 候选曲线，并精确经过指定普通点。

    必经点仍然是普通路径点（point_type=0），不会向 C 路表发出任务指令。
    它们只作为几何锚点参与曲线构造；每个锚点的切线取前后锚点的连线方向，
    因此相邻五次 Bezier 段在此处保持切线连续和零端点曲率。
    """
    if must_pass_tolerance_mm <= 0.0:
        raise ValueError("掉头桩必经点容差必须为正数。")
    center = np.array([stake.x, stake.y], dtype=float)

    def circular_node(angle: float, suffix: str) -> Node:
        radial = np.array([math.cos(angle), math.sin(angle)], dtype=float)
        # direction=1 为逆时针，-1 为顺时针；圆切线保证两段曲线的中间拼接平滑。
        tangent = direction * np.array([-radial[1], radial[0]], dtype=float)
        point = center + safety_radius_mm * radial
        return Node(float(point[0]), float(point[1]), 0, tangent, f"掉头桩绕行{suffix}")

    first = circular_node(start_angle_rad, "起点")
    second = circular_node(start_angle_rad + direction * sweep_angle_rad, "终点")
    before_stake: list[Node] = [start]
    must_positions = [np.array([marker.x, marker.y], dtype=float) for marker in must_pass_markers]
    # 必经点可能位于桩前或桩后。每个点依相邻锚点动态计算切线，使不同的
    # 绕桩候选角度仍能在这些点处平滑连接，而不是把路径折成折线。
    reference_positions = [np.array([start.x, start.y], dtype=float), *must_positions, np.array([first.x, first.y], dtype=float)]
    for index, marker in enumerate(must_pass_markers, start=1):
        tangent = unit(reference_positions[index + 1] - reference_positions[index - 1])
        before_stake.append(Node(
            marker.x,
            marker.y,
            0,
            tangent,
            f"掉头桩必经点 {marker.order}",
            marker_order=marker.order,
        ))
    before_stake.append(first)

    for index, (left, right) in enumerate(zip(before_stake, before_stake[1:])):
        label = "进入绕桩" if index == len(before_stake) - 2 else f"经过必经点 {must_pass_markers[index].order}"
        append_g2_link(samples, left, right, f"{segment_prefix}: {label}", sample_step_mm)
    append_g2_link(samples, first, second, f"{segment_prefix}: 绕桩", sample_step_mm)
    append_g2_link(samples, second, end, f"{segment_prefix}: 离开绕桩", sample_step_mm)


def choose_fastest_turnaround_curve(
    start: Node,
    end: Node,
    stake: Marker,
    speed_profile: SpeedPlanningProfile,
    stake_radius_mm: float,
    clearance_mm: float,
    source_exit_speed_command: Optional[float],
    target_entry_speed_command: Optional[float],
    must_pass_markers: tuple[Marker, ...] = (),
    must_pass_tolerance_mm: float = 20.0,
) -> tuple[float, float, int, float]:
    """搜索不碰桩且预计通过时间最短的 G2 绕桩曲线。

    最短几何曲线在掉头时往往产生极大曲率，实际反而更慢。这里对每个候选
    曲线计算曲率限速、纵向加减速包络与预计通过时间，以时间最小值作为目标。
    """
    safety_radius = stake_radius_mm + clearance_mm
    if stake_radius_mm <= 0.0 or clearance_mm < 0.0:
        raise ValueError("掉头桩半径必须为正，安全裕量不能为负。")

    # (安全圆半径, 进入角, 绕行方向, 绕行跨度, 预计通过时间)
    best: Optional[tuple[float, float, int, float, float]] = None
    center = np.array([stake.x, stake.y], dtype=float)
    # 半径、进入角和绕行跨度共同决定曲率和总路程。搜索两种绕行方向，
    # 而不是根据最短弦长直接连线，才能在“更长但可更快通过”的情况中取优。
    radius_candidates = [safety_radius + offset for offset in (0.0, 200.0, 400.0, 700.0)]
    angle_candidates = np.deg2rad(np.arange(0.0, 360.0, 30.0))
    sweep_candidates = np.deg2rad(np.arange(90.0, 301.0, 30.0))
    for radius in radius_candidates:
        for direction in (-1, 1):
            for start_angle in angle_candidates:
                for sweep in sweep_candidates:
                    candidate: list[PathSample] = []
                    append_turnaround_candidate(
                        candidate,
                        start,
                        end,
                        stake,
                        radius,
                        float(start_angle),
                        float(sweep),
                        direction,
                        "候选",
                        must_pass_markers,
                        must_pass_tolerance_mm,
                        20.0,
                    )
                    points = np.array([(sample.x, sample.y) for sample in candidate], dtype=float)
                    if len(points) < 3:
                        continue
                    minimum_distance = float(np.min(np.linalg.norm(points - center, axis=1)))
                    if minimum_distance + 1e-6 < safety_radius:
                        continue
                    if any(
                        np.min(np.linalg.norm(points - np.array([marker.x, marker.y]), axis=1))
                        > must_pass_tolerance_mm + 1e-6
                        for marker in must_pass_markers
                    ):
                        continue
                    candidate_s, _, candidate_curvature = calculate_yaw_and_curvature(candidate)
                    candidate_profiles = [speed_profile] * len(candidate)
                    candidate_speed = calculate_target_speed(
                        candidate, candidate_s, candidate_curvature, candidate_profiles
                    )
                    # 两端分别受前一状态机出口 v1、后一状态机入口 v2 约束；
                    # 重新做一次纵向扫描，让候选耗时反映真实可达的加减速过程。
                    if source_exit_speed_command is not None:
                        candidate_speed[0] = min(
                            candidate_speed[0], source_exit_speed_command * SPEED_TO_MM_S
                        )
                    if target_entry_speed_command is not None:
                        candidate_speed[-1] = min(
                            candidate_speed[-1], target_entry_speed_command * SPEED_TO_MM_S
                        )
                    candidate_speed = apply_longitudinal_speed_envelope(
                        candidate_speed,
                        candidate_s,
                        np.full(len(candidate), speed_profile.max_accel_mm_s2),
                        np.full(len(candidate), speed_profile.max_decel_mm_s2),
                    )
                    ds = np.diff(candidate_s)
                    average_speed = 0.5 * (candidate_speed[:-1] + candidate_speed[1:])
                    # 极低速度意味着候选无法可靠通过；它不应因数值除零而被误选。
                    if np.any(average_speed <= 1.0):
                        continue
                    travel_time = float(np.sum(ds / average_speed))
                    if best is None or travel_time < best[4]:
                        best = (radius, float(start_angle), direction, float(sweep), travel_time)
    if best is None:
        raise ValueError("没有找到满足掉头桩安全半径的平滑绕行曲线，请增大可用空间或减小安全裕量。")
    # 返回最佳半径、进入角、方向、跨度；通过时间只用于搜索比较，不写入路表。
    return best[0], best[1], best[2], best[3]


def append_fastest_turnaround_curve(
    samples: list[PathSample],
    start: Node,
    end: Node,
    plan: TransitionPlan,
    stake_radius_mm: float,
    clearance_mm: float,
) -> None:
    if plan.stake is None:
        raise ValueError("带掉头桩丝滑型缺少 point_type=7 掉头桩。")
    radius, start_angle, direction, sweep = choose_fastest_turnaround_curve(
        start,
        end,
        plan.stake,
        plan.speed_profile,
        stake_radius_mm,
        clearance_mm,
        plan.source_exit_speed_command,
        plan.target_entry_speed_command,
        plan.must_pass_markers,
        plan.must_pass_tolerance_mm,
    )
    append_turnaround_candidate(
        samples, start, end, plan.stake, radius, start_angle, sweep, direction,
        "掉头桩最速 G2", plan.must_pass_markers, plan.must_pass_tolerance_mm,
    )


def append_low_curvature_turnaround_candidate(
    samples: list[PathSample],
    start: Node,
    end: Node,
    stake: Marker,
    safety_radius_mm: float,
    guard_angle_rad: float,
    direction: int,
    segment_prefix: str,
    must_pass_markers: tuple[Marker, ...] = (),
    must_pass_tolerance_mm: float = 20.0,
    sample_step_mm: float = DENSE_SAMPLE_STEP_MM,
) -> None:
    """通过一个安全圆切向守卫点生成低曲率 G2 绕桩连接。

    与预设 4 的两个圆周控制点不同，预设 5 只使用一个守卫点约束整条
    曲线的外扩方向。这样两端状态机切线直接参与同一条宽弧连接，避免
    最速搜索中为缩短路程出现的多段急弯。
    """
    if must_pass_tolerance_mm <= 0.0:
        raise ValueError("掉头桩必经点容差必须为正数。")
    center = np.array([stake.x, stake.y], dtype=float)
    radial = np.array([math.cos(guard_angle_rad), math.sin(guard_angle_rad)], dtype=float)
    guard_tangent = direction * np.array([-radial[1], radial[0]], dtype=float)
    guard_position = center + safety_radius_mm * radial
    guard = Node(
        float(guard_position[0]),
        float(guard_position[1]),
        0,
        guard_tangent,
        "掉头桩低曲率守卫点",
    )

    before_guard: list[Node] = [start]
    must_positions = [np.array([marker.x, marker.y], dtype=float) for marker in must_pass_markers]
    reference_positions = [
        np.array([start.x, start.y], dtype=float),
        *must_positions,
        np.array([guard.x, guard.y], dtype=float),
    ]
    for index, marker in enumerate(must_pass_markers, start=1):
        tangent = unit(reference_positions[index + 1] - reference_positions[index - 1])
        before_guard.append(Node(
            marker.x,
            marker.y,
            0,
            tangent,
            f"掉头桩必经点 {marker.order}",
            marker_order=marker.order,
        ))
    before_guard.append(guard)
    for index, (left, right) in enumerate(zip(before_guard, before_guard[1:])):
        label = "进入低曲率绕桩" if index == len(before_guard) - 2 else f"经过必经点 {must_pass_markers[index].order}"
        append_g2_link(samples, left, right, f"{segment_prefix}: {label}", sample_step_mm)
    append_g2_link(samples, guard, end, f"{segment_prefix}: 离开低曲率绕桩", sample_step_mm)


def choose_smooth_turnaround_curve(
    start: Node,
    end: Node,
    stake: Marker,
    speed_profile: SpeedPlanningProfile,
    stake_radius_mm: float,
    clearance_mm: float,
    source_exit_speed_command: Optional[float],
    target_entry_speed_command: Optional[float],
    must_pass_markers: tuple[Marker, ...] = (),
    must_pass_tolerance_mm: float = 20.0,
) -> tuple[float, float, int]:
    """搜索低曲率的 G2 绕桩曲线，并用预计通过时间打破平局。

    预设 4 以通过时间最短为目标，可能为少量距离收益接受较大的峰值曲率。
    本预设复用完全相同的候选模型和输入，只改为优先降低峰值曲率、曲率
    能量和曲率变化量；最终速度仍由选中的实际轨迹统一规划。
    """
    safety_radius = stake_radius_mm + clearance_mm
    if stake_radius_mm <= 0.0 or clearance_mm < 0.0:
        raise ValueError("掉头桩半径必须为正，安全裕量不能为负。")
    # 搜索阶段额外预留一个稠密采样步长的余量，避免最终重采样后贴着安全圆。
    candidate_clearance_mm = safety_radius + 25.0

    # (峰值曲率, 曲率能量, 曲率变化量, 预计通过时间, 半径, 守卫点角度, 方向)
    best: Optional[tuple[float, float, float, float, float, float, int]] = None
    center = np.array([stake.x, stake.y], dtype=float)
    radius_candidates = [safety_radius + offset for offset in (0.0, 200.0, 400.0, 700.0)]
    angle_candidates = np.deg2rad(np.arange(0.0, 360.0, 15.0))
    for radius in radius_candidates:
        for direction in (-1, 1):
            for guard_angle in angle_candidates:
                candidate: list[PathSample] = []
                append_low_curvature_turnaround_candidate(
                    candidate,
                    start,
                    end,
                    stake,
                    radius,
                    float(guard_angle),
                    direction,
                    "候选",
                    must_pass_markers,
                    must_pass_tolerance_mm,
                    20.0,
                )
                points = np.array([(sample.x, sample.y) for sample in candidate], dtype=float)
                if len(points) < 3:
                    continue
                if float(np.min(np.linalg.norm(points - center, axis=1))) + 1e-6 < candidate_clearance_mm:
                    continue
                if any(
                    np.min(np.linalg.norm(points - np.array([marker.x, marker.y]), axis=1))
                    > must_pass_tolerance_mm + 1e-6
                    for marker in must_pass_markers
                ):
                    continue

                candidate_s, _, candidate_curvature = calculate_yaw_and_curvature(candidate)
                ds = np.diff(candidate_s)
                if not len(ds) or np.any(ds <= 0.0):
                    continue
                abs_curvature = np.abs(candidate_curvature)
                peak_curvature = float(np.max(abs_curvature))
                curvature_energy = float(
                    np.sum(0.5 * (candidate_curvature[:-1] ** 2 + candidate_curvature[1:] ** 2) * ds)
                )
                curvature_variation = float(np.sum(np.abs(np.diff(candidate_curvature))))

                candidate_profiles = [speed_profile] * len(candidate)
                candidate_speed = calculate_target_speed(
                    candidate, candidate_s, candidate_curvature, candidate_profiles
                )
                if source_exit_speed_command is not None:
                    candidate_speed[0] = min(
                        candidate_speed[0], source_exit_speed_command * SPEED_TO_MM_S
                    )
                if target_entry_speed_command is not None:
                    candidate_speed[-1] = min(
                        candidate_speed[-1], target_entry_speed_command * SPEED_TO_MM_S
                    )
                candidate_speed = apply_longitudinal_speed_envelope(
                    candidate_speed,
                    candidate_s,
                    np.full(len(candidate), speed_profile.max_accel_mm_s2),
                    np.full(len(candidate), speed_profile.max_decel_mm_s2),
                )
                average_speed = 0.5 * (candidate_speed[:-1] + candidate_speed[1:])
                if np.any(average_speed <= 1.0):
                    continue
                travel_time = float(np.sum(ds / average_speed))
                score = (peak_curvature, curvature_energy, curvature_variation, travel_time)
                if best is None or score < best[:4]:
                    best = (*score, radius, float(guard_angle), direction)
    if best is None:
        raise ValueError("没有找到满足掉头桩安全半径的低曲率绕行曲线，请增大可用空间或减小安全裕量。")
    return best[4], best[5], best[6]


def append_smooth_turnaround_curve(
    samples: list[PathSample],
    start: Node,
    end: Node,
    plan: TransitionPlan,
    stake_radius_mm: float,
    clearance_mm: float,
) -> None:
    if plan.stake is None:
        raise ValueError("带掉头桩低曲率丝滑型缺少 point_type=7 掉头桩。")
    radius, guard_angle, direction = choose_smooth_turnaround_curve(
        start,
        end,
        plan.stake,
        plan.speed_profile,
        stake_radius_mm,
        clearance_mm,
        plan.source_exit_speed_command,
        plan.target_entry_speed_command,
        plan.must_pass_markers,
        plan.must_pass_tolerance_mm,
    )
    append_low_curvature_turnaround_candidate(
        samples, start, end, plan.stake, radius, guard_angle, direction,
        "掉头桩低曲率 G2", plan.must_pass_markers, plan.must_pass_tolerance_mm,
    )


def make_nodes(
    markers: list[Marker],
    pairs: dict[int, int],
    transition_plans: dict[tuple[int, int], TransitionPlan],
    task_profiles: dict[int, TaskSpeedProfile],
) -> tuple[list[Node], dict[tuple[int, int], str], dict[tuple[int, int], TransitionPlan]]:
    """展开特殊标记对，并在开头补充未记录的车辆原点。"""
    # 录制是在车辆驶离 (0, 0) 原点之后才开始的。
    # 包含这个虚拟节点可以使得表中的第一条记录成为真正的路线起点。

    nodes: list[Node] = [Node(START_POINT_X_MM, START_POINT_Y_MM, 0, None, "origin (0, 0)")]
    # 仅针对所要求的0.5米驶入/驶出走廊，将值设为True。
    # 入口到出口的连接段是一个直接的任务区域占位符；故意的，
    # 它不被算作导航路径的强制直线段。

    # 只有状态机内部和进出走廊天生是直线；状态机之间的连接类型由
    # transition_plans 决定，未显式标记的普通路段一律走 G2 插值。
    link_modes: dict[tuple[int, int], str] = {}
    turnaround_links: dict[tuple[int, int], TransitionPlan] = {}
    entry_by_exit = {exit_index: entry_index for entry_index, exit_index in pairs.items()}
    exit_anchor_nodes: dict[int, int] = {}
    entry_anchor_nodes: dict[int, int] = {}
    marker_index = 0

    while marker_index < len(markers):
        marker = markers[marker_index]
        if marker_index in pairs:
            exit_index = pairs[marker_index]
            exit_marker = markers[exit_index]
            profile = task_profiles[marker.point_type]
            axis = unit(np.array([exit_marker.x - marker.x, exit_marker.y - marker.y], dtype=float))
            pre = Node(
                marker.x - profile.entry_corridor_mm * axis[0],
                marker.y - profile.entry_corridor_mm * axis[1],
                0, axis, f"{TYPE_LABEL[marker.point_type]}前直线起点",
            )
            entry = Node(
                marker.x, marker.y, marker.point_type, axis, TYPE_LABEL[marker.point_type],
                marker_order=marker.order,
            )
            exit_node = Node(
                exit_marker.x, exit_marker.y, exit_marker.point_type, axis, TYPE_LABEL[exit_marker.point_type],
                marker_order=exit_marker.order,
            )
            post = Node(
                exit_marker.x + profile.exit_corridor_mm * axis[0],
                exit_marker.y + profile.exit_corridor_mm * axis[1],
                0, axis, f"{TYPE_LABEL[exit_marker.point_type]}后直线终点",
            )
            if nodes and distance(nodes[-1], pre) < MIN_LINK_LENGTH_MM:
                raise ValueError(f"{entry.name} 前直线与上一锚点重叠，无法构造平滑连接。")
            nodes.extend((pre, entry, exit_node, post))
            base = len(nodes) - 4
            link_modes[(base, base + 1)] = "forced_line"
            link_modes[(base + 1, base + 2)] = "task_internal"
            link_modes[(base + 2, base + 3)] = "forced_line"
            entry_anchor_nodes[marker.order] = base
            exit_anchor_nodes[exit_marker.order] = base + 3
            marker_index = exit_index + 1
            continue
        if marker_index in entry_by_exit:
            marker_index += 1
            continue
        nodes.append(
            Node(marker.x, marker.y, marker.point_type, None, TYPE_LABEL[marker.point_type], marker_order=marker.order)
        )
        # 雷区/圆环只有一个入口事件点。该点同时充当连接段两端的锚点，
        # 使“雷区 -> 雷区”的纯直线预设能够复用同一套分段机制。
        if marker.point_type == 1:
            entry_anchor_nodes[marker.order] = len(nodes) - 1
            exit_anchor_nodes[marker.order] = len(nodes) - 1
        marker_index += 1

    if len(nodes) < 2:
        raise ValueError("展开后路径锚点不足。")

    for plan in transition_plans.values():
        source_node_index = exit_anchor_nodes.get(plan.source_exit_order)
        target_node_index = entry_anchor_nodes.get(plan.target_entry_order)
        if source_node_index is None or target_node_index is None:
            raise ValueError("状态机连接计划缺少对应的入口或出口锚点。")
        if plan.preset == TransitionPreset.INTERPOLATED:
            continue
        # 非插值预设会在前处理阶段删除中间普通点，因此两个锚点必须相邻。
        # 这里再次检查可防止点表或前处理逻辑变更后悄悄产生错误连线。
        if target_node_index != source_node_index + 1:
            raise ValueError(
                f"{TRANSITION_PRESET_LABEL[plan.preset]} 的两端锚点不相邻，无法直接连接。"
            )
        if plan.preset == TransitionPreset.NEAR_PARALLEL:
            nodes[source_node_index].corner_handle_mm = PARALLEL_TRANSITION_HANDLE_MM
            nodes[target_node_index].corner_handle_mm = PARALLEL_TRANSITION_HANDLE_MM
            link_modes[(source_node_index, target_node_index)] = "near_parallel_g2"
        elif plan.preset == TransitionPreset.PURE_LINE:
            link_modes[(source_node_index, target_node_index)] = "pure_line"
        elif plan.preset == TransitionPreset.POINT_TO_LINE:
            link_modes[(source_node_index, target_node_index)] = "point_to_line_g2"
        elif plan.preset == TransitionPreset.TURNAROUND_STAKE_FASTEST:
            link_modes[(source_node_index, target_node_index)] = "turnaround_stake_fastest"
            turnaround_links[(source_node_index, target_node_index)] = plan
        elif plan.preset == TransitionPreset.TURNAROUND_STAKE_SMOOTH:
            link_modes[(source_node_index, target_node_index)] = "turnaround_stake_smooth"
            turnaround_links[(source_node_index, target_node_index)] = plan

    # 无标签路点使用角平分线切向量，使相邻 G2 曲线满足 C1 连续；同时根据相邻边长计算局部控制柄。
    for index, node in enumerate(nodes):
        if node.tangent is not None:
            continue
        if index == 0:
            node.tangent = unit(np.array([nodes[1].x - node.x, nodes[1].y - node.y]))
        elif index == len(nodes) - 1:
            node.tangent = unit(np.array([node.x - nodes[index - 1].x, node.y - nodes[index - 1].y]))
        else:
            incoming = unit(np.array([node.x - nodes[index - 1].x, node.y - nodes[index - 1].y]))
            outgoing = unit(np.array([nodes[index + 1].x - node.x, nodes[index + 1].y - node.y]))
            try:
                node.tangent = unit(incoming + outgoing)
            except ValueError:
                node.tangent = outgoing
            incoming_length = distance(nodes[index - 1], node)
            outgoing_length = distance(node, nodes[index + 1])
            if node.corner_handle_mm is None:
                node.corner_handle_mm = min(
                    LOCAL_CORNER_HANDLE_MAX_MM,
                    LOCAL_CORNER_NEIGHBOR_RATIO * min(incoming_length, outgoing_length),
                )
    return nodes, link_modes, turnaround_links


def resample_path_by_arclength(dense_samples: list[PathSample], sample_step_mm: float) -> list[PathSample]:
    """Resample the complete dense route at one arc-length spacing.

    Special points and forced-straight transitions are inserted into the
    sampling grid before interpolation, then their original coordinates and
    event tags are written back exactly.  This mirrors chazhi.py while keeping
    Plan4's special-task corridor geometry intact.
    """
    if not dense_samples:
        return []
    if len(dense_samples) == 1:
        return list(dense_samples)

    points = np.array([(sample.x, sample.y) for sample in dense_samples], dtype=float)
    ds = np.linalg.norm(np.diff(points, axis=0), axis=1)
    s_dense = np.concatenate(([0.0], np.cumsum(ds)))
    total_length = float(s_dense[-1])
    if total_length < MIN_LINK_LENGTH_MM:
        return list(dense_samples)

    mandatory_indices = {0, len(dense_samples) - 1}
    for index, sample in enumerate(dense_samples):
        # 普通 type=0 的必经点也需要原样进入最终路表；否则 50 mm 重采样会
        # 使它偏离配置的 20 mm 容差。普通插值路径中的原始打点同样受益于此。
        if sample.point_type != 0 or sample.marker_order is not None:
            mandatory_indices.add(index)
        if index > 0 and sample.forced_straight != dense_samples[index - 1].forced_straight:
            mandatory_indices.add(index)

    base_s = np.arange(0.0, total_length, sample_step_mm, dtype=float)
    if len(base_s) == 0 or not math.isclose(float(base_s[-1]), total_length, abs_tol=0.01):
        base_s = np.append(base_s, total_length)
    target_s = np.unique(np.concatenate((base_s, s_dense[sorted(mandatory_indices)])))

    x = np.interp(target_s, s_dense, points[:, 0])
    y = np.interp(target_s, s_dense, points[:, 1])
    output: list[PathSample] = []
    for target_index, arc in enumerate(target_s):
        exact = int(np.argmin(np.abs(s_dense - arc)))
        if abs(float(s_dense[exact] - arc)) <= 0.01:
            source = dense_samples[exact]
            point = points[exact]
            point_type = source.point_type
            marker_order = source.marker_order
        else:
            source_index = int(np.searchsorted(s_dense, arc, side="right") - 1)
            source_index = max(0, min(source_index, len(dense_samples) - 1))
            source = dense_samples[source_index]
            point = np.array([x[target_index], y[target_index]], dtype=float)
            point_type = 0
            marker_order = None
        add_sample(output, point, point_type, source.forced_straight, source.segment, marker_order)
    return output



def generate_path(
    markers: list[Marker], sample_step_mm: float,
    transition_plans: dict[tuple[int, int], TransitionPlan],
    task_profiles: dict[int, TaskSpeedProfile],
    turnaround_stake_radius_mm: float,
    turnaround_stake_clearance_mm: float,
) -> list[PathSample]:
    pairs = find_event_pairs(markers)
    nodes, link_modes, turnaround_links = make_nodes(markers, pairs, transition_plans, task_profiles)
    dense_samples: list[PathSample] = []
    for index in range(len(nodes) - 1):
        mode = link_modes.get((index, index + 1), "g2")
        if mode == "turnaround_stake_fastest":
            append_fastest_turnaround_curve(
                dense_samples,
                nodes[index],
                nodes[index + 1],
                turnaround_links[(index, index + 1)],
                turnaround_stake_radius_mm,
                turnaround_stake_clearance_mm,
            )
        elif mode == "turnaround_stake_smooth":
            append_smooth_turnaround_curve(
                dense_samples,
                nodes[index],
                nodes[index + 1],
                turnaround_links[(index, index + 1)],
                turnaround_stake_radius_mm,
                turnaround_stake_clearance_mm,
            )
        elif mode == "point_to_line_g2":
            append_point_to_line_g2(
                dense_samples,
                nodes[index],
                nodes[index + 1],
                f"点到线 G2（自由离场）: {nodes[index].name} -> {nodes[index + 1].name}",
                DENSE_SAMPLE_STEP_MM,
            )
        elif mode in {"forced_line", "task_internal", "pure_line"}:
            label = {
                "forced_line": "强制直线",
                "task_internal": "任务区直连",
                "pure_line": "纯直线过渡",
            }[mode]
            append_line(
                dense_samples,
                nodes[index],
                nodes[index + 1],
                f"{label}: {nodes[index].name} -> {nodes[index + 1].name}",
                mode != "task_internal",
                DENSE_SAMPLE_STEP_MM,
            )
        else:
            label = "近似平行 G2" if mode == "near_parallel_g2" else "G2"
            append_g2_link(
                dense_samples,
                nodes[index],
                nodes[index + 1],
                f"{label}: {nodes[index].name} -> {nodes[index + 1].name}",
                DENSE_SAMPLE_STEP_MM,
            )
    return resample_path_by_arclength(dense_samples, sample_step_mm)


def generate_path_with_point_cap(
    markers: list[Marker],
    transition_plans: dict[tuple[int, int], TransitionPlan],
    task_profiles: dict[int, TaskSpeedProfile],
    turnaround_stake_radius_mm: float,
    turnaround_stake_clearance_mm: float,
) -> tuple[list[PathSample], float]:
    """Prefer 50 mm samples, while never generating a route that RAM truncates."""
    sample_step_mm = SAMPLE_STEP_MM
    for _ in range(8):
        samples = generate_path(
            markers,
            sample_step_mm,
            transition_plans,
            task_profiles,
            turnaround_stake_radius_mm,
            turnaround_stake_clearance_mm,
        )
        if len(samples) <= NAV_ROUTE_MAX_POINTS:
            return samples, sample_step_mm
        # 预留少量余量，避免各段 ceil() 后仍需再次调整采样间距。
        sample_step_mm *= max(1.05, len(samples) / (NAV_ROUTE_MAX_POINTS - 4))
    raise ValueError(f"路径即使用 {sample_step_mm:.1f} mm 采样仍超过 {NAV_ROUTE_MAX_POINTS} 点。")


