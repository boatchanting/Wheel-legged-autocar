#!/usr/bin/env python3
"""生成并渲染一条基于标记的进出点、具备G2连续性的Plan3路径。

标记CSV文件描述了路线事件。一个特殊事件由一个入口标记（1..5）和其匹配的出口标记（10..50）表示。生成器会精确保留这两个标记，并预留两条*直*线走廊：

* 入口标记前方的500毫米；
* 出口标记后方的500毫米。

所有剩余的连接段均为五次贝塞尔曲线，且其两端的曲率为零。因此，相邻的连接段与直线走廊共享切线和曲率（即G2连续性）。生成的路径旨在作为连续跟踪的路线输入；它在普通采样点处不会发出停止指令。

示例：
    .venv\\Scripts\\python.exe tools/webview_nav_marker科目四/generate_plan4_smooth_path_考虑响应延迟_丝滑轨迹.py

默认的CSV文件特意指定为项目要求的最新Plan3记录。如果需要基于其他记录生成路径，请传入 ``--input`` 参数。
"""

from __future__ import annotations

import argparse
import csv
import math
import sys
from dataclasses import dataclass, replace
from enum import Enum
from pathlib import Path
from typing import Iterable, Optional

import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent.parent
DEFAULT_HEADER = PROJECT_ROOT / "code" / "navigation" / "nav_replay_route_table.h"
SAMPLE_STEP_MM = 50.0
# 先以更密的几何点生成路径，再统一按弧长重采样，避免最终路表点距影响曲率估计。
DENSE_SAMPLE_STEP_MM = 5.0
MIN_LINK_LENGTH_MM = 5.0
NAV_ROUTE_MAX_POINTS = 5000
START_POINT_X_MM = 0.0
START_POINT_Y_MM = 0.0

# 与 tools/webview_nav_marker速度规划/caculate_path.py 保持一致的离线路径速度约束。
# 状态机的固定速度仍使用这一条底盘标定换算。普通轨迹段可在自己的
# SpeedPlanningProfile 中使用不同的换算值，最终会逐点转换为 target_speed。
SPEED_TO_MM_S = 4.79

# Advance deceleration commands by the measured chassis response delay.
# Keep acceleration unshifted so a future higher speed never violates the
# current curve or special-task speed ceiling.
SPEED_RESPONSE_DELAY_S = 0.0

# 局部圆角控制柄同时受相邻边长限制，避免稀疏或急转标记使 Bezier 曲线偏离局部走廊。
LOCAL_CORNER_HANDLE_RATIO = 0.18
LOCAL_CORNER_NEIGHBOR_RATIO = 0.35
LOCAL_CORNER_HANDLE_MAX_MM = 600.0
# 近似平行型过渡使用独立的控制柄。它比普通局部圆角更长，适合两条
# 方向接近、但存在明显横向错位的状态机通道（例如三级跳 -> 单边桥）。
PARALLEL_TRANSITION_HANDLE_MM = 900.0

# 以下任务速度使用写入 NavRamPoint_t 的 target_speed 指令单位。每个状态机
# 显式区分“进入速度 / 状态机运行速度 / 退出速度”，即使当前默认三者相同。
# 这样一条完整路线可表达为：
#   轨迹1 -> 状态机1进入(v1) -> 状态机1 -> 状态机1退出(v2) -> 轨迹2 -> ...
# 将来某一状态机需要以不同速度进出时，只需改对应配置，不必重写连接逻辑。

# 与 csv_to_nav_table.py 保持一致：每个值都从记录的出口点朝对应入口点测量，
# 使视觉状态机出口锚定在车辆实际离开任务的位置。
# 特殊状态机结束点的沿线修正距离（单位：CSV 坐标单位；当前导航坐标单位为毫米 mm）。
# 30：三级台阶结束点，对应状态机进入点类型 3（3 -> 30），单位：mm。
# 40：单边桥结束点，对应状态机进入点类型 4（4 -> 40），单位：mm。
# 50：颠簸路段结束点，对应状态机进入点类型 5（5 -> 50），单位：mm。
# 正值：结束点沿“结束点 -> 对应进入点”的连线靠近进入点。
# 负值：结束点沿同一连线的反方向远离进入点。
# 如果发现少跑了，应该增大对应距离；多跑了，减少对应距离
SPECIAL_EXIT_DISTANCE_OFFSETS_MM = {30: 0.0, 40: 750.0, 50: 1350.0}


# 雷区（1）只有驶入点，不记录/匹配驶出点；其余视觉任务仍为入口/出口成对点。
ENTRY_TYPES = {1, 2, 3, 4, 5}
PAIRED_ENTRY_TYPES = {2, 3, 4, 5}
EXIT_TO_ENTRY = {20: 2, 30: 3, 40: 4, 50: 5}
TYPE_LABEL = {
    0: "普通路径",
    1: "圆环进入",
    2: "坡道进入",
    3: "三级跳进入",
    4: "单边桥进入",
    5: "颠簸路进入",
    10: "圆环退出",
    20: "坡道退出",
    30: "三级跳退出",
    40: "单边桥退出",
    50: "颠簸路退出",
}
TYPE_COLOR = {
    0: "#3b82f6", 1: "#db2777",
    # 坡道绿色、三级跳红色、单边桥紫色、颠簸路黄色；进入/退出保持同色。
    2: "#16a34a", 3: "#dc2626", 4: "#9333ea", 5: "#eab308",
    10: "#db2777", 20: "#16a34a", 30: "#dc2626", 40: "#9333ea", 50: "#eab308",
}


@dataclass(frozen=True)
class Marker:
    order: int
    x: float
    y: float
    point_type: int
    heading: Optional[float]
    relative_yaw: Optional[float]


@dataclass
class Node:
    x: float
    y: float
    point_type: int = 0
    tangent: Optional[np.ndarray] = None
    name: str = ""
    corner_handle_mm: Optional[float] = None
    marker_order: Optional[int] = None


@dataclass
class PathSample:
    x: float
    y: float
    point_type: int
    forced_straight: bool
    segment: str
    marker_order: Optional[int] = None


class TransitionPreset(str, Enum):
    """两段状态机之间的人工可选轨迹连接策略。"""

    INTERPOLATED = "interpolated"
    NEAR_PARALLEL = "near_parallel"
    PURE_LINE = "pure_line"


TRANSITION_PRESET_LABEL = {
    TransitionPreset.INTERPOLATED: "轨迹插值型",
    TransitionPreset.NEAR_PARALLEL: "近似平行型",
    TransitionPreset.PURE_LINE: "纯直线型",
}


@dataclass(frozen=True)
class TaskSpeedProfile:
    """单个状态机的入口、运行及出口速度约束。"""

    approach_distance_mm: float
    entry_speed_command: float
    state_speed_command: float
    exit_speed_command: float
    entry_corridor_mm: float
    exit_corridor_mm: float


# 每种有配对出口的状态机都使用一个独立速度档案。现有任务的进入、运行、
# 退出速度保持一致，以完全保留原来的控制行为；后续可以按任务单独拆开。
TASK_SPEED_PROFILES = {
    # 参数依次为：进场恒速距离、入口速度、状态机速度、出口速度、入口前直线、出口后直线。
    3: TaskSpeedProfile(4000.0, 320.0, 320.0, 320.0, 600.0, 600.0),  # 三级跳
    4: TaskSpeedProfile(500.0, 300.0, 300.0, 300.0, 600.0, 600.0),    # 单边桥
    5: TaskSpeedProfile(500.0, 500.0, 500.0, 500.0, 600.0, 600.0),    # 颠簸路
    2: TaskSpeedProfile(500.0, 500.0, 500.0, 500.0, 600.0, 600.0),    # 坡道
}


@dataclass(frozen=True)
class SpeedPlanningProfile:
    """一段普通轨迹的完整速度规划参数，单位与旧全局常量保持一致。"""

    path_speed_max_mm_s: float = 8000.0
    sprint_speed_mm_s: float = 8000.0
    enable_finish_sprint: bool = True
    max_accel_mm_s2: float = 10500.0
    max_decel_mm_s2: float = 10500.0
    max_lateral_accel_mm_s2: float = 4000.0
    max_path_yaw_rate_rad_s: float = 2.8
    speed_to_mm_s: float = SPEED_TO_MM_S
    curvature_eps: float = 1e-6


# 未被某个 TrajectorySegment 覆盖的样本（状态机内部、路线首尾）使用原来的默认值。
DEFAULT_TRAJECTORY_SPEED_PROFILE = SpeedPlanningProfile()


@dataclass(frozen=True)
class StateMachineSegment:
    """点表中一个完整状态机段：入口(v_in) -> 状态机 -> 出口(v_out)。"""

    entry_order: int
    exit_order: int
    entry_type: int
    exit_type: int
    entry_speed_command: Optional[float]
    exit_speed_command: Optional[float]


@dataclass(frozen=True)
class TrajectorySegment:
    """普通轨迹段：包括路线首段、状态机间段和路线末段。"""

    source_exit_order: Optional[int]
    target_entry_order: Optional[int]
    preset: TransitionPreset
    source_exit_speed_command: Optional[float]
    target_entry_speed_command: Optional[float]
    speed_profile: SpeedPlanningProfile = DEFAULT_TRAJECTORY_SPEED_PROFILE


@dataclass(frozen=True)
class TransitionPlan:
    """供几何生成使用的状态机间连接计划。"""

    source_exit_order: int
    target_entry_order: int
    preset: TransitionPreset


def normalize_key(value: str) -> str:
    return value.strip().lower().replace(" ", "")


def unit(vector: np.ndarray) -> np.ndarray:
    length = float(np.linalg.norm(vector))
    if length < 1e-6:
        raise ValueError("Two consecutive path anchors are coincident.")
    return vector / length


def distance(a: Node | Marker | np.ndarray, b: Node | Marker | np.ndarray) -> float:
    ax, ay = (a.x, a.y) if hasattr(a, "x") else a
    bx, by = (b.x, b.y) if hasattr(b, "x") else b
    return math.hypot(bx - ax, by - ay)


def find_latest_marker_csv(script_dir: Path) -> Path:
    candidates = [
        path for path in script_dir.glob("nav_mark_points_*.csv")
        if not path.stem.endswith("_planned")
        and len(path.stem[len("nav_mark_points_"):]) == 15  # 新增：严格校验时间戳部分长度为15
    ]
    if not candidates:
        raise FileNotFoundError(f"{script_dir} 中没有原始 nav_mark_points_*.csv。")
    return max(candidates, key=lambda path: path.stat().st_mtime)


def read_markers(csv_path: Path) -> tuple[list[Marker], Optional[float]]:
    with csv_path.open("r", encoding="utf-8-sig", newline="") as csv_file:
        reader = csv.DictReader(csv_file)
        if not reader.fieldnames:
            raise ValueError("CSV 缺少表头。")
        fields = {normalize_key(name): name for name in reader.fieldnames if name}
        for required in ("x", "y", "point_type"):
            if required not in fields:
                raise ValueError(f"CSV 缺少必需列: {required}")

        index_field = fields.get("index")
        relative_yaw_field = fields.get("relative_yaw") or fields.get("target_yaw_deg")
        heading_field = fields.get("heading")
        start_heading_field = fields.get("start_heading")
        markers: list[Marker] = []
        start_heading: Optional[float] = None
        for row_number, row in enumerate(reader, start=2):
            try:
                order = int(row[index_field]) if index_field and row[index_field] else len(markers)
                relative_yaw = float(row[relative_yaw_field]) if relative_yaw_field and row[relative_yaw_field] else None
                heading = float(row[heading_field]) if heading_field and row[heading_field] else None
                if start_heading is None and start_heading_field and row[start_heading_field]:
                    start_heading = float(row[start_heading_field])
                point_type = int(float(row[fields["point_type"]]))
                if point_type not in TYPE_LABEL:
                    raise ValueError(f"不支持的 point_type={point_type}")
                markers.append(Marker(
                    order,
                    float(row[fields["x"]]),
                    float(row[fields["y"]]),
                    point_type,
                    heading,
                    relative_yaw,
                ))
            except (TypeError, ValueError) as exc:
                raise ValueError(f"CSV 第 {row_number} 行无效: {exc}") from exc

    markers.sort(key=lambda item: item.order)
    if len(markers) < 2:
        raise ValueError("至少需要两个标记点。")
    return markers, start_heading


def find_event_pairs(markers: list[Marker]) -> dict[int, int]:
    """Return entry-index -> exit-index and reject ambiguous marker ordering."""
    pairs: dict[int, int] = {}
    pending: dict[int, int] = {}
    for index, marker in enumerate(markers):
        if marker.point_type == 1:
            # 雷区只在驶入时触发状态机，入口点本身保留在路径上，不能要求 type=10 出口。
            continue
        if marker.point_type in PAIRED_ENTRY_TYPES:
            if marker.point_type in pending:
                raise ValueError(f"第 {index} 个进入点前，类型 {marker.point_type} 的上一个任务尚未退出。")
            pending[marker.point_type] = index
        elif marker.point_type == 10:
            raise ValueError("雷区 point_type=1 仅支持驶入点，不应存在 point_type=10 的驶出点。")
        elif marker.point_type in EXIT_TO_ENTRY:
            entry_type = EXIT_TO_ENTRY[marker.point_type]
            if entry_type not in pending:
                raise ValueError(f"第 {index} 个退出点 type={marker.point_type} 没有对应进入点。")
            pairs[pending.pop(entry_type)] = index

    if pending:
        names = ", ".join(str(value) for value in sorted(pending))
        raise ValueError(f"缺少对应退出点的进入类型: {names}")
    return pairs


def apply_special_exit_corrections(markers: list[Marker], pairs: dict[int, int]) -> list[Marker]:
    """Move 30/40/50 exit anchors to the state machine's calibrated exit point."""
    corrected = list(markers)
    for entry_index, exit_index in pairs.items():
        entry = corrected[entry_index]
        exit_marker = corrected[exit_index]
        offset = SPECIAL_EXIT_DISTANCE_OFFSETS_MM.get(exit_marker.point_type, 0.0)
        # 0 表示不修正；正负值都有效，方向语义由配置注释定义。
        if math.isclose(offset, 0.0, abs_tol=1e-9):
            continue
        direction = unit(np.array([entry.x - exit_marker.x, entry.y - exit_marker.y], dtype=float))
        corrected[exit_index] = replace(
            exit_marker,
            x=exit_marker.x + offset * direction[0],
            y=exit_marker.y + offset * direction[1],
        )
    return corrected


def build_state_machine_segments(
    markers: list[Marker], pairs: dict[int, int], task_profiles: dict[int, TaskSpeedProfile]
) -> list[StateMachineSegment]:
    """按点表顺序提取状态机段，供交互选择和速度规划共同使用。

    配对任务（2/3/4/5）有明确入口和出口。圆环/雷区（1）只有触发入口，
    因而把该点同时作为它的入口与退出锚点：这样“雷区 -> 雷区”同样可以
    选择纯直线型，而不会伪造一个不存在的 type=10 出口点。
    """
    segments: list[StateMachineSegment] = []
    for entry_index, marker in enumerate(markers):
        if entry_index in pairs:
            exit_marker = markers[pairs[entry_index]]
            profile = TASK_SPEED_PROFILES[marker.point_type]
            segments.append(
                StateMachineSegment(
                    marker.order,
                    exit_marker.order,
                    marker.point_type,
                    exit_marker.point_type,
                    profile.entry_speed_command,
                    profile.exit_speed_command,
                )
            )
        elif marker.point_type == 1:
            segments.append(
                StateMachineSegment(marker.order, marker.order, 1, 1, None, None)
            )
    return segments


def describe_speed(speed: Optional[float]) -> str:
    return "由状态机控制" if speed is None else f"{speed:.0f}"


def choose_trajectory_segments(
    state_segments: list[StateMachineSegment], interactive: bool
) -> tuple[list[TrajectorySegment], dict[tuple[int, int], TransitionPlan]]:
    """在终端依点表顺序选择每两个状态机之间的连接预设。

    选择 1 时保留普通打点，并按原有 G2 平滑插值连接；选择 2/3 时将
    该对状态机之间的普通打点移除，分别构造近似平行 G2 换道或纯直线。
    非交互模式只用于批处理，统一采用预设 1，避免 CI 或脚本调用卡在 input()。
    """
    trajectories: list[TrajectorySegment] = []
    plans: dict[tuple[int, int], TransitionPlan] = {}
    preset_by_choice = {
        "1": TransitionPreset.INTERPOLATED,
        "2": TransitionPreset.NEAR_PARALLEL,
        "3": TransitionPreset.PURE_LINE,
    }

    # 起点到第一状态机入口同样是一段可调速度的普通轨迹，只是不需要选择
    # 状态机间连接预设，所以固定为插值型且不生成 TransitionPlan。
    if state_segments:
        first = state_segments[0]
        trajectories.append(
            TrajectorySegment(None, first.entry_order, TransitionPreset.INTERPOLATED, None, first.entry_speed_command)
        )

    for position, (source, target) in enumerate(zip(state_segments, state_segments[1:]), start=1):
        source_name = TYPE_LABEL[source.exit_type]
        target_name = TYPE_LABEL[target.entry_type]
        if interactive:
            print(
                f"\n[{position}/{len(state_segments) - 1}] {source_name} (v={describe_speed(source.exit_speed_command)}) "
                f"-> {target_name} (v={describe_speed(target.entry_speed_command)})"
            )
            print("  1. 轨迹插值型：保留普通打点，按原有 G2 曲线逐段平滑连接")
            print("  2. 近似平行型：删除中间普通打点，用两端平行走廊之间的 G2 换道连接")
            print("  3. 纯直线型：删除中间普通打点，两个锚点之间直接连直线（常用于雷区到雷区）")
            while True:
                try:
                    choice = input("  选择预设 [1]: ").strip() or "1"
                except EOFError:
                    # 无交互终端仍可安全执行，且行为与显式批处理保持一致。
                    choice = "1"
                    print("1（未检测到终端输入，使用轨迹插值型）")
                if choice in preset_by_choice:
                    break
                print("  输入无效，请输入 1、2 或 3。")
        else:
            choice = "1"

        preset = preset_by_choice[choice]
        trajectory = TrajectorySegment(
            source.exit_order,
            target.entry_order,
            preset,
            source.exit_speed_command,
            target.entry_speed_command,
        )
        trajectories.append(trajectory)
        plans[(trajectory.source_exit_order, trajectory.target_entry_order)] = TransitionPlan(
            trajectory.source_exit_order,
            trajectory.target_entry_order,
            preset,
        )
    if state_segments:
        last = state_segments[-1]
        trajectories.append(
            TrajectorySegment(last.exit_order, None, TransitionPreset.INTERPOLATED, last.exit_speed_command, None)
        )
    return trajectories, plans


def configure_segment_parameters(
    state_segments: list[StateMachineSegment],
    trajectories: list[TrajectorySegment],
    task_profiles: dict[int, TaskSpeedProfile],
) -> tuple[dict[int, TaskSpeedProfile], list[TrajectorySegment]]:
    """可选的终端配置入口；直接回车保留每段的默认参数。

    速度参数以一行 CSV 输入，字段顺序与 SpeedPlanningProfile 完全对应。
    这种形式既支持逐段设置，也方便复制既有调参数据，避免为每段弹出九个问题。
    """
    configured_tasks = dict(task_profiles)
    seen_types: set[int] = set()
    print("\n状态机进出直线长度（直接回车保留默认值）：")
    for state in state_segments:
        if state.entry_type not in configured_tasks or state.entry_type in seen_types:
            continue
        seen_types.add(state.entry_type)
        profile = configured_tasks[state.entry_type]
        raw = input(
            f"  {TYPE_LABEL[state.entry_type]} 入口前/出口后 mm "
            f"[{profile.entry_corridor_mm:.0f},{profile.exit_corridor_mm:.0f}]: "
        ).strip()
        if not raw:
            continue
        try:
            entry_length, exit_length = (float(value.strip()) for value in raw.split(","))
        except ValueError as exc:
            raise ValueError("直线长度格式应为：入口前mm,出口后mm") from exc
        if entry_length < 0.0 or exit_length < 0.0:
            raise ValueError("状态机前后的直线长度不能为负数。")
        configured_tasks[state.entry_type] = replace(
            profile, entry_corridor_mm=entry_length, exit_corridor_mm=exit_length
        )

    print("\n轨迹段速度参数（直接回车保留默认值）：")
    print("  顺序：最高速度,冲刺速度,启用冲刺(0/1),最大加速,最大减速,最大横向加速度,最大航向角速度,速度换算,曲率阈值")
    configured_trajectories: list[TrajectorySegment] = []
    for trajectory in trajectories:
        profile = trajectory.speed_profile
        source_name = "轨迹起点" if trajectory.source_exit_order is None else str(trajectory.source_exit_order)
        target_name = "轨迹终点" if trajectory.target_entry_order is None else str(trajectory.target_entry_order)
        raw = input(
            f"  {source_name} -> {target_name} "
            f"[{profile.path_speed_max_mm_s:.0f},{profile.sprint_speed_mm_s:.0f},"
            f"{int(profile.enable_finish_sprint)},{profile.max_accel_mm_s2:.0f},"
            f"{profile.max_decel_mm_s2:.0f},{profile.max_lateral_accel_mm_s2:.0f},"
            f"{profile.max_path_yaw_rate_rad_s:g},{profile.speed_to_mm_s:g},{profile.curvature_eps:g}]: "
        ).strip()
        if not raw:
            configured_trajectories.append(trajectory)
            continue
        values = [value.strip() for value in raw.split(",")]
        if len(values) != 9:
            raise ValueError("轨迹速度参数必须恰好包含 9 个逗号分隔值。")
        if values[2] not in {"0", "1"}:
            raise ValueError("启用冲刺只能输入 0 或 1。")
        try:
            configured = SpeedPlanningProfile(
                path_speed_max_mm_s=float(values[0]),
                sprint_speed_mm_s=float(values[1]),
                enable_finish_sprint=bool(int(values[2])),
                max_accel_mm_s2=float(values[3]),
                max_decel_mm_s2=float(values[4]),
                max_lateral_accel_mm_s2=float(values[5]),
                max_path_yaw_rate_rad_s=float(values[6]),
                speed_to_mm_s=float(values[7]),
                curvature_eps=float(values[8]),
            )
        except ValueError as exc:
            raise ValueError("轨迹速度参数格式无效；冲刺开关只能输入 0 或 1。") from exc
        if (
            configured.path_speed_max_mm_s <= 0.0
            or configured.sprint_speed_mm_s <= 0.0
            or configured.max_accel_mm_s2 <= 0.0
            or configured.max_decel_mm_s2 <= 0.0
            or configured.max_lateral_accel_mm_s2 <= 0.0
            or configured.max_path_yaw_rate_rad_s <= 0.0
            or configured.speed_to_mm_s <= 0.0
            or configured.curvature_eps <= 0.0
        ):
            raise ValueError("轨迹速度参数中的数值必须为正数。")
        configured_trajectories.append(replace(trajectory, speed_profile=configured))
    return configured_tasks, configured_trajectories


def print_segment_plan(
    state_segments: list[StateMachineSegment], trajectories: list[TrajectorySegment]
) -> None:
    """显示最终的交替分段计划，便于把终端选择和点表顺序逐项核对。"""
    if not state_segments:
        return
    trajectory_by_source = {segment.source_exit_order: segment for segment in trajectories}
    print("\n最终分段规划：")
    print("  轨迹起点")
    start_trajectory = trajectory_by_source.get(None)
    if start_trajectory is not None:
        print(f"  -> 轨迹段[{TRANSITION_PRESET_LABEL[start_trajectory.preset]}]")
    for state in state_segments:
        print(
            f"  -> {TYPE_LABEL[state.entry_type]}(v={describe_speed(state.entry_speed_command)})"
            f" -> 状态机 -> {TYPE_LABEL[state.exit_type]}(v={describe_speed(state.exit_speed_command)})"
        )
        trajectory = trajectory_by_source.get(state.exit_order)
        if trajectory is not None:
            print(
                f"  -> 轨迹段[{TRANSITION_PRESET_LABEL[trajectory.preset]}]"
                f" (v={describe_speed(trajectory.source_exit_speed_command)}"
                f" -> v={describe_speed(trajectory.target_entry_speed_command)})"
            )
    print("  -> 轨迹终点")


def apply_trajectory_marker_policy(
    markers: list[Marker], trajectories: Iterable[TrajectorySegment]
) -> tuple[list[Marker], int]:
    """根据预设决定普通打点是否参与两个状态机之间的几何插值。

    近似平行型和纯直线型都直接从前一状态机的退出锚点连到后一状态机的
    入口锚点，因此中间只能有 type=0 普通点；遇到其他状态机标记时拒绝
    删除，防止人工选择破坏点表中的任务触发顺序。
    """
    index_by_order = {marker.order: index for index, marker in enumerate(markers)}
    remove_indices: set[int] = set()
    for trajectory in trajectories:
        if (
            trajectory.preset == TransitionPreset.INTERPOLATED
            or trajectory.source_exit_order is None
            or trajectory.target_entry_order is None
        ):
            continue
        source_index = index_by_order[trajectory.source_exit_order]
        target_index = index_by_order[trajectory.target_entry_order]
        if target_index <= source_index:
            raise ValueError("状态机连接顺序无效：目标入口必须位于源出口之后。")
        between = range(source_index + 1, target_index)
        non_ordinary = [markers[index] for index in between if markers[index].point_type != 0]
        if non_ordinary:
            labels = ", ".join(TYPE_LABEL[marker.point_type] for marker in non_ordinary)
            raise ValueError(f"{TRANSITION_PRESET_LABEL[trajectory.preset]} 不能跨越其他状态机标记: {labels}")
        remove_indices.update(between)
    return [marker for index, marker in enumerate(markers) if index not in remove_indices], len(remove_indices)


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


def make_nodes(
    markers: list[Marker],
    pairs: dict[int, int],
    transition_plans: dict[tuple[int, int], TransitionPlan],
    task_profiles: dict[int, TaskSpeedProfile],
) -> tuple[list[Node], dict[tuple[int, int], str]]:
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
    return nodes, link_modes


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
        if sample.point_type != 0:
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



def generate_path(
    markers: list[Marker], sample_step_mm: float,
    transition_plans: dict[tuple[int, int], TransitionPlan],
    task_profiles: dict[int, TaskSpeedProfile],
) -> list[PathSample]:
    pairs = find_event_pairs(markers)
    nodes, link_modes = make_nodes(markers, pairs, transition_plans, task_profiles)
    dense_samples: list[PathSample] = []
    for index in range(len(nodes) - 1):
        mode = link_modes.get((index, index + 1), "g2")
        if mode in {"forced_line", "task_internal", "pure_line"}:
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
) -> tuple[list[PathSample], float]:
    """Prefer 50 mm samples, while never generating a route that RAM truncates."""
    sample_step_mm = SAMPLE_STEP_MM
    for _ in range(8):
        samples = generate_path(markers, sample_step_mm, transition_plans, task_profiles)
        if len(samples) <= NAV_ROUTE_MAX_POINTS:
            return samples, sample_step_mm
        # 预留少量余量，避免各段 ceil() 后仍需再次调整采样间距。
        sample_step_mm *= max(1.05, len(samples) / (NAV_ROUTE_MAX_POINTS - 4))
    raise ValueError(f"路径即使用 {sample_step_mm:.1f} mm 采样仍超过 {NAV_ROUTE_MAX_POINTS} 点。")


def write_csv(output: Path, samples: list[PathSample], yaw: np.ndarray, curvature: np.ndarray, target_speed: np.ndarray) -> None:
    with output.open("w", encoding="utf-8", newline="") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["index", "x", "y", "target_yaw_deg", "heading", "point_type", "target_speed", "curvature", "forced_straight", "segment"])
        for index, (sample, sample_yaw, sample_curvature, speed) in enumerate(zip(samples, yaw, curvature, target_speed)):
            writer.writerow([index, f"{sample.x:.3f}", f"{sample.y:.3f}", f"{sample_yaw:.3f}", "0.000", sample.point_type, f"{speed:.3f}", f"{sample_curvature:.8f}", int(sample.forced_straight), sample.segment])


def write_c_header(output: Path, samples: list[PathSample], yaw: np.ndarray, curvature: np.ndarray, target_speed: np.ndarray, source: Path, start_heading: Optional[float]) -> None:
    # 获取当前的生成时间，格式为 年-月-日 时:分:秒
    generation_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    lines = [
        "#ifndef _NAV_REPLAY_ROUTE_TABLE_H_",
        "#define _NAV_REPLAY_ROUTE_TABLE_H_",
        "",
        "#include \"nav_ram.h\"",
        "",
        f"// Generated by {Path(__file__).name} from {source.name}",
        f"// Generated at {generation_time}",
        "// Exit anchors 30/40/50 include the calibrated state-machine correction.",
        "#define NAV_REPLAY_START_HEADING_VALID 0",
        f"#define NAV_REPLAY_START_HEADING_DEG {0.0 if start_heading is None else start_heading:.3f}f",
        f"#define NAV_REPLAY_STATIC_ROUTE_COUNT {len(samples)}",
        "",
        f"static const NavRamPoint_t nav_replay_static_route_points[{len(samples)}] = {{",
    ]
    for sample, sample_yaw, sample_curvature, speed in zip(samples, yaw, curvature, target_speed):
        # NavRamPoint_t 字段顺序：x、y、目标航向、记录航向、点类型、目标速度、曲率。
        lines.append(f"    {{{sample.x:.3f}f, {sample.y:.3f}f, {sample_yaw:.3f}f, 0.0f, (uint8){sample.point_type}, {speed:.3f}f, {sample_curvature:.8f}f}},")
    lines.extend(["};", "", "#endif // _NAV_REPLAY_ROUTE_TABLE_H_", ""])
    output.write_text("\n".join(lines), encoding="utf-8")


def render(output: Path, markers: Iterable[Marker], samples: list[PathSample], s: np.ndarray, curvature: np.ndarray, target_speed: np.ndarray) -> None:
    plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DejaVu Sans"]
    plt.rcParams["axes.unicode_minus"] = False
    points = np.array([(sample.x, sample.y) for sample in samples], dtype=float)
    marker_list = list(markers)
    # 箭头与标记点保持可见间隔，并根据地图尺度自适应长度。
    map_span_mm = max(float(np.ptp(points[:, 0])), float(np.ptp(points[:, 1])), 1.0)
    arrow_length_mm = min(1200.0, max(350.0, 0.035 * map_span_mm))
    forced = np.array([sample.forced_straight for sample in samples], dtype=bool)
    figure, (axis_path, axis_speed, axis_curve) = plt.subplots(1, 3, figsize=(20, 7), gridspec_kw={"width_ratios": [1.35, 1, 1]})
    axis_path.plot(points[:, 0], points[:, 1], color="#1d4ed8", linewidth=2.0, label="G2 平滑路径")
    if forced.any():
        axis_path.scatter(points[forced, 0], points[forced, 1], s=9, color="#f97316", label="强制直线段", zorder=3)
    for marker in marker_list:
        color = TYPE_COLOR[marker.point_type]
        axis_path.scatter(marker.x, marker.y, s=75, marker="o" if marker.point_type in (0, *ENTRY_TYPES) else "s", color=color, edgecolor="black", linewidth=0.6, zorder=5)
        # 普通路径点只保留位置标记，避免地图被大量文字遮挡。
        if marker.point_type != 0:
            axis_path.annotate(f"{marker.order}: {TYPE_LABEL[marker.point_type]}", (marker.x, marker.y), xytext=(5, 5), textcoords="offset points", fontsize=8)
        if marker.point_type in ENTRY_TYPES and marker.relative_yaw is not None:
            # 与 calc_path_yaw_deg() 相反变换：relative_yaw=0° 指向 X 负方向。
            yaw_rad = math.radians(marker.relative_yaw)
            direction = np.array([-math.cos(yaw_rad), -math.sin(yaw_rad)])
            arrow_start = np.array([marker.x, marker.y]) + 0.35 * arrow_length_mm * direction
            arrow_end = np.array([marker.x, marker.y]) + 1.35 * arrow_length_mm * direction
            axis_path.annotate(
                "",
                xy=arrow_end,
                xytext=arrow_start,
                arrowprops={"arrowstyle": "-|>", "color": color, "lw": 2.4, "mutation_scale": 18},
                zorder=6,
            )
            yaw_label = np.array([marker.x, marker.y]) + 1.55 * arrow_length_mm * direction
            axis_path.annotate(
                f"relative_yaw={marker.relative_yaw:.1f}°",
                yaw_label,
                ha="center",
                va="center",
                fontsize=8,
                fontweight="bold",
                color=color,
                zorder=6,
            )
    axis_path.set_title("Plan3 生成路径（橙色为入口前/出口后直线）")
    axis_path.set_xlabel("X (mm)")
    axis_path.set_ylabel("Y (mm)")
    axis_path.axis("equal")
    axis_path.grid(True, alpha=0.25)
    axis_path.legend(loc="best")

    axis_speed.plot(s, -target_speed * SPEED_TO_MM_S, color="#dc2626", linewidth=1.5)
    axis_speed.set_title("离线速度规划")
    axis_speed.set_xlabel("累计路径长度 (mm)")
    axis_speed.set_ylabel("目标速度 (mm/s)")
    axis_speed.grid(True, alpha=0.25)

    axis_curve.plot(s, curvature * 1000.0, color="#0f766e", linewidth=1.5)
    axis_curve.axhline(0.0, color="black", linewidth=0.8)
    axis_curve.set_title("路径曲率")
    axis_curve.set_xlabel("累计路径长度 (mm)")
    axis_curve.set_ylabel("曲率 (1/m)")
    axis_curve.grid(True, alpha=0.25)
    figure.suptitle(
        "请仔细检查地图（1.状态机的种类2.坡道前的打点需要拉一下）",
        fontsize=21,
        fontweight="bold",
    )
    figure.tight_layout(rect=(0, 0, 1, 0.92))
    figure.savefig(output, dpi=180)
    plt.show()
    plt.close(figure)


def render_speed_heatmap(output: Path, samples: list[PathSample], target_speed: np.ndarray) -> None:
    """单独绘制路径速度热力图，颜色表示物理目标速度。"""
    plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DejaVu Sans"]
    plt.rcParams["axes.unicode_minus"] = False
    points = np.array([(sample.x, sample.y) for sample in samples], dtype=float)
    speed_mm_s = -target_speed * SPEED_TO_MM_S
    figure, axis = plt.subplots(figsize=(10, 8))
    heatmap = axis.scatter(
        points[:, 0],
        points[:, 1],
        c=speed_mm_s,
        cmap="turbo",
        s=18,
        linewidths=0.0,
    )
    axis.plot(points[:, 0], points[:, 1], color="#334155", linewidth=0.8, alpha=0.45, zorder=0)
    axis.set_title("Plan4 路径速度热力图")
    axis.set_xlabel("X (mm)")
    axis.set_ylabel("Y (mm)")
    axis.axis("equal")
    axis.grid(True, alpha=0.25)
    colorbar = figure.colorbar(heatmap, ax=axis, pad=0.02)
    colorbar.set_label("目标速度 (mm/s)")
    figure.tight_layout()
    figure.savefig(output, dpi=180)
    plt.show()
    plt.close(figure)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="生成带可配置状态机进出直线和分段速度参数的 Plan4 平滑路径。")
    parser.add_argument("--input", type=Path, help="输入标记 CSV（默认自动选择本目录最新原始 CSV）")
    parser.add_argument("--output-csv", type=Path, help="输出路径 CSV，默认与输入同目录并追加 _planned")
    parser.add_argument("--render", type=Path, help="输出 PNG，默认与输入同目录并追加 _planned")
    parser.add_argument("--header", type=Path, default=DEFAULT_HEADER, help="C 路表输出位置（默认 code/navigation/nav_replay_route_table.h）")
    parser.add_argument(
        "--stairs-approach-distance-mm",
        type=float,
        default=TASK_SPEED_PROFILES[3].approach_distance_mm,
        help="three-step approach speed-cap distance in mm (default: 4000)",
    )
    parser.add_argument(
        "--non-interactive-transitions",
        action="store_true",
        help="不在终端询问连接预设；所有状态机间轨迹统一采用预设1（轨迹插值型）",
    )
    parser.add_argument(
        "--configure-segment-parameters",
        action="store_true",
        help="在选择连接预设后，逐段编辑状态机进出直线长度和轨迹速度参数",
    )
    parser.add_argument(
        "--speed-response-delay-s",
        type=float,
        default=SPEED_RESPONSE_DELAY_S,
        help="chassis response delay used to advance deceleration commands (default: 0.0)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.stairs_approach_distance_mm < 0.0:
        raise ValueError("Three-step approach distance must not be negative.")
    if args.speed_response_delay_s < 0.0:
        raise ValueError("Speed response delay must not be negative.")
    source = args.input.resolve() if args.input else find_latest_marker_csv(SCRIPT_DIR)
    if not source.is_file():
        raise FileNotFoundError(f"找不到默认输入 CSV: {source}。请导出该文件或使用 --input 指定 CSV。")
    csv_output = args.output_csv or source.with_name(f"{source.stem}_planned.csv")
    render_output = args.render or source.with_name(f"{source.stem}_planned.png")

    # 每次运行从默认档案复制，交互调参只影响本次生成，不会修改脚本源码。
    task_speed_profiles = dict(TASK_SPEED_PROFILES)
    # 兼容旧命令行参数，同时使三级跳进场距离真正属于它的状态机速度档案。
    task_speed_profiles[3] = replace(
        task_speed_profiles[3], approach_distance_mm=args.stairs_approach_distance_mm
    )

    markers, start_heading = read_markers(source)
    pairs = find_event_pairs(markers)
    markers = apply_special_exit_corrections(markers, pairs)
    # 先从原始点表建立交替的“状态机段 / 轨迹段”计划，再按人工选择决定
    # 哪些普通点参与几何生成。Marker.order 在过滤后保持不变，可作为稳定键。
    state_segments = build_state_machine_segments(markers, pairs, task_speed_profiles)
    trajectory_segments, transition_plans = choose_trajectory_segments(
        state_segments, interactive=not args.non_interactive_transitions
    )
    if args.configure_segment_parameters:
        task_speed_profiles, trajectory_segments = configure_segment_parameters(
            state_segments, trajectory_segments, task_speed_profiles
        )
        # 状态机段也持有入口/出口速度与几何档案，配置后重新建立以保持显示一致。
        state_segments = build_state_machine_segments(markers, pairs, task_speed_profiles)
    print_segment_plan(state_segments, trajectory_segments)
    markers, removed_marker_count = apply_trajectory_marker_policy(markers, trajectory_segments)
    samples, effective_sample_step_mm = generate_path_with_point_cap(
        markers, transition_plans, task_speed_profiles
    )
    s, yaw, curvature = calculate_yaw_and_curvature(samples)
    sample_profiles = build_sample_speed_profiles(samples, trajectory_segments)
    physical_speed = calculate_target_speed(samples, s, curvature, sample_profiles)
    task_ranges = build_task_speed_ranges(samples, s, task_speed_profiles)
    physical_speed, task_speed_mask = apply_constant_task_speeds(
        samples, s, physical_speed, task_ranges, sample_profiles
    )
    physical_speed = apply_response_delay_compensation(
        physical_speed,
        s,
        args.speed_response_delay_s,
        preserve_mask=task_speed_mask,
    )
    # 延时补偿只提前减速命令；补偿后的边界仍重新检查可达性，避免在
    # 状态机恒速段出口产生不可实现的速度跳变。
    max_accel = np.array([profile.max_accel_mm_s2 for profile in sample_profiles], dtype=float)
    max_decel = np.array([profile.max_decel_mm_s2 for profile in sample_profiles], dtype=float)
    physical_speed = apply_fixed_speed_envelope(
        physical_speed, s, task_speed_mask, max_accel, max_decel
    )
    # 状态机固定区必须保留原始 target_speed 指令单位；普通轨迹段则按它自己的
    # speed_to_mm_s 逐点换算，满足不同轨迹段使用不同底盘标定的需求。
    speed_to_mm_s = np.array([profile.speed_to_mm_s for profile in sample_profiles], dtype=float)
    speed_to_mm_s[task_speed_mask] = SPEED_TO_MM_S
    output_target_speed = -physical_speed / speed_to_mm_s
    speed_heatmap_output = render_output.with_name(f"{render_output.stem}_speed_heatmap{render_output.suffix}")
    # 如需导出路径 CSV，取消下一行注释。
    # write_csv(csv_output, samples, yaw, curvature, output_target_speed)
    render(render_output, markers, samples, s, curvature, output_target_speed)
    render_speed_heatmap(speed_heatmap_output, samples, output_target_speed)
    write_c_header(args.header, samples, yaw, curvature, output_target_speed, source, start_heading)
    print(f"速度热力图: {speed_heatmap_output}")

    # 即使 50 mm 采样点恰好落在边界上，几何长度仍按任务锚点精确计算，不从可视标签估算。
    final_pairs = find_event_pairs(markers)
    forced_length = sum(
        task_speed_profiles[markers[entry_index].point_type].entry_corridor_mm
        + task_speed_profiles[markers[entry_index].point_type].exit_corridor_mm
        for entry_index in final_pairs
    )
    print(f"输入: {source}")
    print(f"输出路径: {csv_output} ({len(samples)} 点, 总长 {s[-1]:.1f} mm)")
    print(f"采样间距: {effective_sample_step_mm:.1f} mm（上限 {NAV_ROUTE_MAX_POINTS} 点）")
    print(f"目标速度范围: {output_target_speed.min():.1f} ~ {output_target_speed.max():.1f}（负数为前进指令）")
    print(f"渲染图: {render_output}")
    print(f"强制直线累计长度: {forced_length:.1f} mm")
    print(f"非插值状态机过渡忽略普通路径点: {removed_marker_count} 个")
    print(f"C 路表: {args.header}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, ValueError) as exc:
        print(f"错误: {exc}", file=sys.stderr)
        raise SystemExit(2) from exc
