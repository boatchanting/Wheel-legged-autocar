#!/usr/bin/env python3
"""曲率限速、纵向速度包络、状态机恒速和响应延迟补偿。"""
from __future__ import annotations

import math

import numpy as np

from .plan4_models import (
    DEFAULT_TRAJECTORY_SPEED_PROFILE,
    EXIT_TO_ENTRY,
    SPEED_TO_MM_S,
    PathSample,
    SpeedPlanningProfile,
    TaskSpeedProfile,
    TrajectorySegment,
)

def calculate_yaw_and_curvature(samples: list[PathSample]) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    points = np.array([(sample.x, sample.y) for sample in samples], dtype=float)
    count = len(points)
    ds = np.zeros(count, dtype=float)
    ds[1:] = np.linalg.norm(np.diff(points, axis=0), axis=1)
    s = np.cumsum(ds)
    yaw = np.zeros(count, dtype=float)
    for index in range(count):
        before = max(0, index - 1)
        after = min(count - 1, index + 1)
        vector = points[after] - points[before]
        yaw[index] = -math.degrees(math.atan2(vector[1], -vector[0])) if np.linalg.norm(vector) > 1e-6 else 0.0

    curvature = np.zeros(count, dtype=float)
    for index in range(1, count - 1):
        a, b, c = points[index - 1], points[index], points[index + 1]
        ab, bc, ca = np.linalg.norm(b - a), np.linalg.norm(c - b), np.linalg.norm(a - c)
        denominator = ab * bc * ca
        if denominator > 1e-6:
            cross = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
            curvature[index] = 2.0 * cross / denominator
    if count > 2:
        curvature[0], curvature[-1] = curvature[1], curvature[-2]
    return s, yaw, curvature


def apply_longitudinal_speed_envelope(
    speed_limit: np.ndarray,
    s: np.ndarray,
    max_accel_mm_s2: np.ndarray,
    max_decel_mm_s2: np.ndarray,
) -> np.ndarray:
    """以每个采样点所属轨迹段的加减速能力生成可达速度包络。"""
    planned_speed = np.array(speed_limit, copy=True)
    for index in range(len(planned_speed) - 2, -1, -1):
        ds = s[index + 1] - s[index]
        decel = min(max_decel_mm_s2[index], max_decel_mm_s2[index + 1])
        planned_speed[index] = min(
            planned_speed[index],
            math.sqrt(max(0.0, planned_speed[index + 1] ** 2 + 2.0 * decel * ds)),
        )
    for index in range(1, len(planned_speed)):
        ds = s[index] - s[index - 1]
        accel = min(max_accel_mm_s2[index - 1], max_accel_mm_s2[index])
        planned_speed[index] = min(
            planned_speed[index],
            math.sqrt(max(0.0, planned_speed[index - 1] ** 2 + 2.0 * accel * ds)),
        )
    return planned_speed


def apply_response_delay_compensation(
    physical_speed: np.ndarray,
    s: np.ndarray,
    response_delay_s: float,
    preserve_mask: Optional[np.ndarray] = None,
) -> np.ndarray:
    """Advance only deceleration commands by the chassis response delay.

    The input is the physically safe speed profile in mm/s. A future lower
    speed is issued now when the vehicle will travel to it during the measured
    response delay.  Future acceleration is intentionally not advanced.
    """
    if len(physical_speed) == 0 or response_delay_s <= 0.0:
        return np.array(physical_speed, copy=True)

    desired_speed = np.array(physical_speed, copy=True)
    command_speed = np.array(physical_speed, copy=True)
    path_end_s = float(s[-1])

    for index, current_speed in enumerate(desired_speed):
        delayed_s = min(path_end_s, float(s[index]) + current_speed * response_delay_s)
        future_speed = float(np.interp(delayed_s, s, desired_speed))
        if preserve_mask is None or not preserve_mask[index]:
            command_speed[index] = min(current_speed, future_speed)

    return command_speed


def apply_fixed_speed_envelope(
    speed_limit: np.ndarray,
    s: np.ndarray,
    fixed_mask: np.ndarray,
    max_accel_mm_s2: np.ndarray,
    max_decel_mm_s2: np.ndarray,
) -> np.ndarray:
    """Apply longitudinal reachability while preserving hard task speeds.

    Task state machines require an exact command speed over their complete
    approach-to-exit interval.  The samples immediately before and after that
    interval are allowed to ramp under the normal acceleration/deceleration
    limits, so the route remains continuous without changing the task speed.
    """
    planned_speed = np.array(speed_limit, copy=True)
    for _ in range(2):
        for index in range(len(planned_speed) - 2, -1, -1):
            if fixed_mask[index]:
                continue
            ds = s[index + 1] - s[index]
            decel = min(max_decel_mm_s2[index], max_decel_mm_s2[index + 1])
            planned_speed[index] = min(
                planned_speed[index],
                math.sqrt(max(0.0, planned_speed[index + 1] ** 2 + 2.0 * decel * ds)),
            )
        for index in range(1, len(planned_speed)):
            if fixed_mask[index]:
                continue
            ds = s[index] - s[index - 1]
            accel = min(max_accel_mm_s2[index - 1], max_accel_mm_s2[index])
            planned_speed[index] = min(
                planned_speed[index],
                math.sqrt(max(0.0, planned_speed[index - 1] ** 2 + 2.0 * accel * ds)),
            )
    return planned_speed


def build_sample_speed_profiles(
    samples: list[PathSample], trajectories: Iterable[TrajectorySegment]
) -> list[SpeedPlanningProfile]:
    """把每个状态机间的轨迹档案映射到精确的点表事件锚点之间。

    marker_order 由稠密生成和弧长重采样共同保留，故此处不依赖相同
    point_type 的出现次数；连续两个雷区入口也能得到各自独立的速度参数。
    """
    profiles = [DEFAULT_TRAJECTORY_SPEED_PROFILE for _ in samples]
    index_by_order = {
        sample.marker_order: index
        for index, sample in enumerate(samples)
        if sample.marker_order is not None
    }
    for trajectory in trajectories:
        try:
            start = 0 if trajectory.source_exit_order is None else index_by_order[trajectory.source_exit_order]
            end = len(samples) - 1 if trajectory.target_entry_order is None else index_by_order[trajectory.target_entry_order]
        except KeyError as exc:
            raise ValueError("轨迹段缺少对应的点表事件锚点，无法应用速度参数。") from exc
        if end < start:
            raise ValueError("轨迹段速度参数的目标入口位于源出口之前。")
        for index in range(start, end + 1):
            profiles[index] = trajectory.speed_profile
    return profiles


def calculate_target_speed(
    samples: list[PathSample],
    s: np.ndarray,
    curvature: np.ndarray,
    sample_profiles: list[SpeedPlanningProfile],
) -> np.ndarray:
    """按所属轨迹段的速度参数生成物理速度（mm/s）包络。"""
    count = len(samples)
    if count == 0:
        return np.empty(0, dtype=float)
    if len(sample_profiles) != count:
        raise ValueError("速度档案数量与路径采样点数量不一致。")

    speed_limit = np.empty(count, dtype=float)
    max_accel = np.empty(count, dtype=float)
    max_decel = np.empty(count, dtype=float)
    for index, kappa in enumerate(curvature):
        profile = sample_profiles[index]
        abs_kappa = abs(float(kappa))
        curve_limit = profile.path_speed_max_mm_s
        yaw_rate_limit = profile.path_speed_max_mm_s
        if abs_kappa > profile.curvature_eps:
            curve_limit = math.sqrt(profile.max_lateral_accel_mm_s2 / abs_kappa)
            yaw_rate_limit = profile.max_path_yaw_rate_rad_s / abs_kappa
        speed_limit[index] = min(profile.path_speed_max_mm_s, curve_limit, yaw_rate_limit)
        max_accel[index] = profile.max_accel_mm_s2
        max_decel[index] = profile.max_decel_mm_s2

    # "冲刺"的作用域是每一段普通轨迹的末端，而非整条路线；这样一段的
    # 参数不会意外修改下一段的入场限速。相邻同配置段仍按各自边界处理。
    segment_start = 0
    while segment_start < count:
        profile = sample_profiles[segment_start]
        segment_end = segment_start + 1
        while segment_end < count and sample_profiles[segment_end] == profile:
            segment_end += 1
        if profile.enable_finish_sprint:
            local_curvature = curvature[segment_start:segment_end]
            curved = np.flatnonzero(np.abs(local_curvature) > profile.curvature_eps)
            if len(curved):
                sprint_start = segment_start + int(curved[-1]) + 5
                if sprint_start < segment_end:
                    speed_limit[sprint_start:segment_end] = np.minimum(
                        speed_limit[sprint_start:segment_end], profile.sprint_speed_mm_s
                    )
            speed_limit[segment_end - 1] = min(speed_limit[segment_end - 1], profile.sprint_speed_mm_s)
        segment_start = segment_end

    # 普通点和普通结束点都连续通过，不制造零速障碍；圆环动作仍允许明确停车。
    for index, sample in enumerate(samples):
        if sample.point_type == 1:  # NAV_POINT_CIRCLE 可能要求原地旋转。
            speed_limit[index] = 0.0

    return apply_longitudinal_speed_envelope(speed_limit, s, max_accel, max_decel)


def _task_speed_ranges(
    samples: list[PathSample],
    s: np.ndarray,
    entry_type: int,
    exit_type: int,
    approach_distance_mm: float,
    target_speed: float,
) -> list[tuple[float, float, float]]:
    """Return (approach start, state-machine exit, physical speed) ranges."""
    entries = [float(s[index]) for index, sample in enumerate(samples) if sample.point_type == entry_type]
    exits = [float(s[index]) for index, sample in enumerate(samples) if sample.point_type == exit_type]
    if len(entries) != len(exits):
        raise ValueError(
            f"任务 point_type={entry_type} 的入口/出口数量不匹配: {len(entries)} != {len(exits)}"
        )
    ranges: list[tuple[float, float, float]] = []
    for entry_s, exit_s in zip(entries, exits):
        start_s = max(0.0, entry_s - approach_distance_mm)
        if exit_s < start_s:
            raise ValueError(f"任务 point_type={entry_type} 的出口早于 approach 起点。")
        ranges.append((start_s, exit_s, target_speed * SPEED_TO_MM_S))
    return ranges


def apply_constant_task_speeds(
    samples: list[PathSample],
    s: np.ndarray,
    physical_speed: np.ndarray,
    task_ranges: Iterable[tuple[float, float, float]],
    sample_profiles: list[SpeedPlanningProfile],
) -> tuple[np.ndarray, np.ndarray]:
    """在轨迹物理速度上叠加状态机的硬速度，并保留逐段加减速约束。"""
    speed_limit = np.array(physical_speed, copy=True)
    fixed_mask = np.zeros(len(samples), dtype=bool)
    fixed_values = np.zeros(len(samples), dtype=float)
    for start_s, end_s, physical_speed in sorted(task_ranges, key=lambda item: item[0]):
        mask = (s >= start_s) & (s <= end_s)
        overlap = fixed_mask & mask & (np.abs(fixed_values - physical_speed) > 1e-6)
        if np.any(overlap):
            raise ValueError("不同状态机的恒速区间发生重叠。")
        fixed_mask[mask] = True
        fixed_values[mask] = physical_speed
    speed_limit[fixed_mask] = fixed_values[fixed_mask]
    max_accel = np.array([profile.max_accel_mm_s2 for profile in sample_profiles], dtype=float)
    max_decel = np.array([profile.max_decel_mm_s2 for profile in sample_profiles], dtype=float)
    planned_speed = apply_fixed_speed_envelope(speed_limit, s, fixed_mask, max_accel, max_decel)
    planned_speed[fixed_mask] = fixed_values[fixed_mask]
    return planned_speed, fixed_mask


def build_task_speed_ranges(
    samples: list[PathSample], s: np.ndarray, profiles: dict[int, TaskSpeedProfile]
) -> list[tuple[float, float, float]]:
    """把状态机速度档案展开为可供全局包络规划使用的固定速度区间。

    当前底盘状态机要求其进场到出口使用同一条固定速度命令，所以使用
    state_speed_command 作为硬约束。entry/exit_speed_command 同时保存在
    StateMachineSegment 和 TrajectorySegment 中，作为相邻轨迹段的速度合约；
    默认三者相同，兼容原实现，并为以后状态机支持变速进出预留接口。
    """
    ranges: list[tuple[float, float, float]] = []
    for entry_type, profile in profiles.items():
        exit_type = next(exit_value for exit_value, entry_value in EXIT_TO_ENTRY.items() if entry_value == entry_type)
        ranges.extend(
            _task_speed_ranges(
                samples,
                s,
                entry_type,
                exit_type,
                profile.approach_distance_mm,
                profile.state_speed_command,
            )
        )
    return ranges



