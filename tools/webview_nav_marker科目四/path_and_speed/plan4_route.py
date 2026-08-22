#!/usr/bin/env python3
"""点表读取、状态机编排、连接预设选择和路径点保留策略。"""
from __future__ import annotations

import csv
import math
from dataclasses import replace
from pathlib import Path
from typing import Optional

import numpy as np

from .plan4_config import trajectory_config_key
from .plan4_models import (
    ENTRY_TYPES,
    EXIT_TO_ENTRY,
    PAIRED_ENTRY_TYPES,
    POINT_TO_POINT_TYPES,
    SPECIAL_EXIT_DISTANCE_OFFSETS_MM,
    TASK_SPEED_PROFILES,
    TYPE_LABEL,
    Marker,
    Node,
    PathSample,
    SpeedPlanningProfile,
    StateMachineSegment,
    TaskSpeedProfile,
    TrajectorySegment,
    TransitionPlan,
    TransitionPreset,
    TRANSITION_PRESET_LABEL,
)

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
        if marker.point_type in POINT_TO_POINT_TYPES:
            # type=1 只在驶入时触发雷区状态机；type=11 是不停点。
            # 两者都没有配对出口，入口点本身必须保留在路径上。
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
            # 必须使用调用方传入的当前路线档案，才能让专属 TOML 中的
            # task.* 速度和状态机进出长度真实参与后续规划。
            profile = task_profiles[marker.point_type]
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

    选择 1 时保留普通打点，并按 chazhi.py 的圆角平滑算法连接；选择 3/4 时将
    该对状态机之间的普通打点移除，分别构造近似平行 G2 换道或纯直线。
    非交互模式只用于批处理，统一采用预设 1，避免 CI 或脚本调用卡在 input()。
    """
    trajectories: list[TrajectorySegment] = []
    plans: dict[tuple[int, int], TransitionPlan] = {}
    preset_by_choice = {
        "1": TransitionPreset.INTERPOLATED,
        "2": TransitionPreset.G2_INTERPOLATED,
        "3": TransitionPreset.NEAR_PARALLEL,
        "4": TransitionPreset.PURE_LINE,
        "5": TransitionPreset.POINT_TO_LINE,
        "6": TransitionPreset.TURNAROUND_STAKE_FASTEST,
        "7": TransitionPreset.TURNAROUND_STAKE_SMOOTH,
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
            print("  1. 圆角平滑插值型：保留普通打点，按 chazhi.py 的 Corner Fillet 平滑连接")
            print("  2. G2 贝塞尔插值型：保留普通打点，使用原有逐段五次 Bezier 曲线")
            print("  3. 近似平行型：删除中间普通打点，用两端平行走廊之间的 G2 换道连接")
            print("  4. 纯直线型：删除中间普通打点，两个锚点之间直接连直线（常用于雷区到雷区）")
            print("  5. 点到线丝滑型：雷区自由离场，搜索低曲率方向后贴合下一状态机入口直线")
            print("  6. 带掉头桩丝滑型：忽略中间点与掉头桩标签，绕桩搜索最快的平滑曲线")
            print("  7. 带掉头桩低曲率丝滑型：使用与预设 6 相同输入，选择曲率更平缓的绕桩曲线")
            while True:
                try:
                    choice = input("  选择预设 [1]: ").strip() or "1"
                except EOFError:
                    # 无交互终端仍可安全执行，且行为与显式批处理保持一致。
                    choice = "1"
                    print("1（未检测到终端输入，使用轨迹插值型）")
                if choice in preset_by_choice:
                    break
                print("  输入无效，请输入 1、2、3、4、5、6 或 7。")
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


def build_transition_plans(
    trajectories: Iterable[TrajectorySegment], markers: list[Marker]
) -> dict[tuple[int, int], TransitionPlan]:
    """把轨迹段变为几何计划，并为绕桩预设绑定唯一的 type=7 掉头桩。

    type=7 不是状态机，不能参与 find_event_pairs；仅当人工选择绕桩预设时，
    它才必须位于该段两个状态机锚点之间且数量恰为一个。
    """
    plans: dict[tuple[int, int], TransitionPlan] = {}
    for trajectory in trajectories:
        if trajectory.source_exit_order is None or trajectory.target_entry_order is None:
            continue
        stake: Optional[Marker] = None
        must_pass_markers: tuple[Marker, ...] = ()
        marker_by_order = {marker.order: marker for marker in markers}
        if trajectory.preset == TransitionPreset.POINT_TO_LINE:
            source = marker_by_order.get(trajectory.source_exit_order)
            target = marker_by_order.get(trajectory.target_entry_order)
            if source is None or target is None:
                raise ValueError("点到线预设缺少对应的状态机锚点。")
            if source.point_type != 1 or target.point_type not in PAIRED_ENTRY_TYPES:
                raise ValueError("点到线预设仅支持雷区(point_type=1)出口连接到配对状态机入口。")
        if trajectory.preset in {
            TransitionPreset.TURNAROUND_STAKE_FASTEST,
            TransitionPreset.TURNAROUND_STAKE_SMOOTH,
        }:
            stakes = [
                marker for marker in markers
                if trajectory.source_exit_order < marker.order < trajectory.target_entry_order
                and marker.point_type == 7
            ]
            if len(stakes) != 1:
                raise ValueError(
                    "带掉头桩预设要求两个状态机之间恰好有一个 point_type=7 掉头桩。"
                )
            stake = stakes[0]
            if tuple(sorted(trajectory.must_pass_marker_orders)) != trajectory.must_pass_marker_orders:
                raise ValueError("掉头桩必经点必须按点表 index 的行驶顺序升序填写。")
            must_pass_markers = tuple(
                marker_by_order.get(order) for order in trajectory.must_pass_marker_orders
            )
            if any(marker is None for marker in must_pass_markers):
                raise ValueError("掉头桩必经点包含当前点表中不存在的 index。")
            if any(
                marker.point_type != 0
                or not trajectory.source_exit_order < marker.order < trajectory.target_entry_order
                for marker in must_pass_markers
            ):
                raise ValueError("掉头桩必经点必须是该连接段内的 point_type=0 普通点。")
        plans[(trajectory.source_exit_order, trajectory.target_entry_order)] = TransitionPlan(
            trajectory.source_exit_order,
            trajectory.target_entry_order,
            trajectory.preset,
            source_exit_speed_command=trajectory.source_exit_speed_command,
            target_entry_speed_command=trajectory.target_entry_speed_command,
            stake=stake,
            speed_profile=trajectory.speed_profile,
            must_pass_markers=must_pass_markers,
            must_pass_tolerance_mm=trajectory.must_pass_tolerance_mm,
        )
    return plans


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
    print("  顺序：最高速度,冲刺速度,启用冲刺(0/1),最大加速,最大减速,最大横向加速度,最大航向角速度,速度换算,曲率阈值,响应延迟秒")
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
            f"{profile.max_path_yaw_rate_rad_s:g},{profile.speed_to_mm_s:g},{profile.curvature_eps:g},"
            f"{profile.response_delay_s:g}]: "
        ).strip()
        if not raw:
            configured_trajectories.append(trajectory)
            continue
        values = [value.strip() for value in raw.split(",")]
        if len(values) != 10:
            raise ValueError("轨迹速度参数必须恰好包含 10 个逗号分隔值。")
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
                response_delay_s=float(values[9]),
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
            or configured.response_delay_s < 0.0
        ):
            raise ValueError("轨迹速度参数中的数值必须为正数，响应延迟不能为负数。")
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
        print(
            f"  -> 轨迹段[{trajectory_config_key(start_trajectory)} | "
            f"{TRANSITION_PRESET_LABEL[start_trajectory.preset]} | "
            f"最高速度={start_trajectory.speed_profile.path_speed_max_mm_s:.0f}]"
        )
    for state in state_segments:
        print(
            f"  -> {TYPE_LABEL[state.entry_type]}(v={describe_speed(state.entry_speed_command)})"
            f" -> 状态机 -> {TYPE_LABEL[state.exit_type]}(v={describe_speed(state.exit_speed_command)})"
        )
        trajectory = trajectory_by_source.get(state.exit_order)
        if trajectory is not None:
            print(
                f"  -> 轨迹段[{trajectory_config_key(trajectory)} | "
                f"{TRANSITION_PRESET_LABEL[trajectory.preset]} | "
                f"最高速度={trajectory.speed_profile.path_speed_max_mm_s:.0f}]"
                f" (v={describe_speed(trajectory.source_exit_speed_command)}"
                f" -> v={describe_speed(trajectory.target_entry_speed_command)})"
            )
    print("  -> 轨迹终点")


def apply_trajectory_marker_policy(
    markers: list[Marker], trajectories: Iterable[TrajectorySegment]
) -> tuple[list[Marker], int]:
    """根据预设决定普通打点是否参与两个状态机之间的几何插值。

    近似平行型、纯直线型和点到线丝滑型都直接从前一状态机的退出锚点连到后一状态机的
    入口锚点，因此中间只能有 type=0 普通点；遇到其他状态机标记时拒绝
    删除，防止人工选择破坏点表中的任务触发顺序。
    """
    index_by_order = {marker.order: index for index, marker in enumerate(markers)}
    # 掉头桩只描述障碍物位置，绝不是导航事件；无论选用何种预设都不能
    # 出现在生成的 path sample / C 路表中。绕桩预设已在 TransitionPlan 中保留它。
    remove_indices: set[int] = {
        index for index, marker in enumerate(markers) if marker.point_type == 7
    }
    for trajectory in trajectories:
        if (
            trajectory.preset in {
                TransitionPreset.INTERPOLATED,
                TransitionPreset.G2_INTERPOLATED,
            }
            or trajectory.source_exit_order is None
            or trajectory.target_entry_order is None
        ):
            continue
        source_index = index_by_order[trajectory.source_exit_order]
        target_index = index_by_order[trajectory.target_entry_order]
        if target_index <= source_index:
            raise ValueError("状态机连接顺序无效：目标入口必须位于源出口之后。")
        between = range(source_index + 1, target_index)
        allowed_types = {0}
        if trajectory.preset in {
            TransitionPreset.TURNAROUND_STAKE_FASTEST,
            TransitionPreset.TURNAROUND_STAKE_SMOOTH,
        }:
            allowed_types.add(7)
        non_ordinary = [markers[index] for index in between if markers[index].point_type not in allowed_types]
        if non_ordinary:
            labels = ", ".join(TYPE_LABEL[marker.point_type] for marker in non_ordinary)
            raise ValueError(f"{TRANSITION_PRESET_LABEL[trajectory.preset]} 不能跨越其他状态机标记: {labels}")
        remove_indices.update(between)
    return [marker for index, marker in enumerate(markers) if index not in remove_indices], len(remove_indices)


