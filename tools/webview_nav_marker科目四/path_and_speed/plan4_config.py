#!/usr/bin/env python3
"""通用 TOML 与路线专属 TOML 的读取、校验及模板生成。"""
from __future__ import annotations

import hashlib
import tomllib
from dataclasses import replace
from pathlib import Path

from .plan4_models import (
    DEFAULT_TRAJECTORY_SPEED_PROFILE,
    NAV_TOML_DIR,
    TASK_SPEED_PROFILES,
    TURNAROUND_STAKE_CLEARANCE_MM,
    TURNAROUND_STAKE_RADIUS_MM,
    PlanningConfiguration,
    SpeedPlanningProfile,
    StateMachineSegment,
    TaskSpeedProfile,
    TrajectorySegment,
    TransitionPreset,
)

def trajectory_config_key(trajectory: TrajectorySegment) -> str:
    """生成 TOML 中稳定、易读的轨迹段键，例如 ``8->20`` 或 ``start->0``。"""
    source = "start" if trajectory.source_exit_order is None else str(trajectory.source_exit_order)
    target = "end" if trajectory.target_entry_order is None else str(trajectory.target_entry_order)
    return f"{source}->{target}"


def validate_speed_profile(profile: SpeedPlanningProfile, context: str) -> None:
    if (
        profile.path_speed_max_mm_s <= 0.0
        or profile.sprint_speed_mm_s <= 0.0
        or profile.max_accel_mm_s2 <= 0.0
        or profile.max_decel_mm_s2 <= 0.0
        or profile.max_lateral_accel_mm_s2 <= 0.0
        or profile.max_path_yaw_rate_rad_s <= 0.0
        or profile.speed_to_mm_s <= 0.0
        or profile.curvature_eps <= 0.0
        or profile.response_delay_s < 0.0
    ):
        raise ValueError(f"{context} 中的速度规划数值必须为正数，response_delay_s 不能为负数。")


def overlay_speed_profile(
    base: SpeedPlanningProfile, values: dict[str, object], context: str
) -> SpeedPlanningProfile:
    allowed = set(SpeedPlanningProfile.__dataclass_fields__)
    unknown = set(values) - allowed
    if unknown:
        raise ValueError(f"{context} 包含不支持的速度参数: {', '.join(sorted(unknown))}")
    try:
        profile = replace(base, **values)
    except TypeError as exc:
        raise ValueError(f"{context} 的速度参数类型无效。") from exc
    if not isinstance(profile.enable_finish_sprint, bool):
        raise ValueError(f"{context}.enable_finish_sprint 必须为 true 或 false。")
    validate_speed_profile(profile, context)
    return profile


def overlay_task_profile(
    base: TaskSpeedProfile, values: dict[str, object], context: str
) -> TaskSpeedProfile:
    allowed = set(TaskSpeedProfile.__dataclass_fields__)
    unknown = set(values) - allowed
    if unknown:
        raise ValueError(f"{context} 包含不支持的状态机参数: {', '.join(sorted(unknown))}")
    try:
        profile = replace(base, **values)
    except TypeError as exc:
        raise ValueError(f"{context} 的状态机参数类型无效。") from exc
    if any(float(getattr(profile, name)) <= 0.0 for name in TaskSpeedProfile.__dataclass_fields__):
        raise ValueError(f"{context} 中的状态机参数必须为正数。")
    return profile


def load_planning_configuration(config_path: Path) -> PlanningConfiguration:
    """读取通用 TOML；每种连接预设拥有独立的默认速度档案。"""
    if not config_path.is_file():
        raise FileNotFoundError(f"找不到速度规划配置文件: {config_path}")
    try:
        with config_path.open("rb") as config_file:
            raw = tomllib.load(config_file)
    except tomllib.TOMLDecodeError as exc:
        raise ValueError(f"速度规划配置 TOML 格式错误: {exc}") from exc

    task_profiles = dict(TASK_SPEED_PROFILES)
    raw_tasks = raw.get("task", {})
    if not isinstance(raw_tasks, dict):
        raise ValueError("[task] 必须是 TOML 表。")
    for key, values in raw_tasks.items():
        try:
            point_type = int(key)
        except ValueError as exc:
            raise ValueError(f"[task] 的键必须是状态机入口类型，例如 '3'。") from exc
        if point_type not in task_profiles or not isinstance(values, dict):
            raise ValueError(f"[task.{key}] 不是可配置的配对状态机。")
        task_profiles[point_type] = overlay_task_profile(
            task_profiles[point_type], values, f"[task.{key}]"
        )

    raw_trajectory = raw.get("trajectory", {})
    if not isinstance(raw_trajectory, dict):
        raise ValueError("[trajectory] 必须是 TOML 表。")
    expected_preset_names = {preset.value for preset in TransitionPreset}
    unknown_preset_names = set(raw_trajectory) - expected_preset_names
    if unknown_preset_names:
        raise ValueError(
            "[trajectory] 只允许按连接预设配置速度参数，"
            f"不支持: {', '.join(sorted(unknown_preset_names))}。"
        )
    preset_profiles: dict[TransitionPreset, SpeedPlanningProfile] = {}
    for preset in TransitionPreset:
        values = raw_trajectory.get(preset.value)
        if not isinstance(values, dict):
            raise ValueError(
                f"通用 TOML 缺少 [trajectory.{preset.value}]，"
                "每一种连接预设都必须有独立速度参数。"
            )
        preset_profiles[preset] = overlay_speed_profile(
            DEFAULT_TRAJECTORY_SPEED_PROFILE,
            values,
            f"[trajectory.{preset.value}]",
        )

    raw_turnaround = raw.get("turnaround_stake", {})
    if not isinstance(raw_turnaround, dict):
        raise ValueError("[turnaround_stake] 必须是 TOML 表。")
    allowed_turnaround = {"radius_mm", "clearance_mm"}
    unknown_turnaround = set(raw_turnaround) - allowed_turnaround
    if unknown_turnaround:
        raise ValueError(f"[turnaround_stake] 包含不支持的参数: {', '.join(sorted(unknown_turnaround))}")
    radius = float(raw_turnaround.get("radius_mm", TURNAROUND_STAKE_RADIUS_MM))
    clearance = float(raw_turnaround.get("clearance_mm", TURNAROUND_STAKE_CLEARANCE_MM))
    if radius <= 0.0 or clearance < 0.0:
        raise ValueError("[turnaround_stake] radius_mm 必须为正，clearance_mm 不能为负。")
    return PlanningConfiguration(task_profiles, preset_profiles, radius, clearance)


def apply_configuration_to_trajectories(
    trajectories: list[TrajectorySegment], configuration: PlanningConfiguration
) -> list[TrajectorySegment]:
    """按每段选择的连接预设，应用通用 TOML 中对应的速度档案。"""
    return [
        replace(
            trajectory,
            speed_profile=configuration.preset_speed_profiles[trajectory.preset],
        )
        for trajectory in trajectories
    ]


def suggest_initial_trajectory_presets(
    trajectories: list[TrajectorySegment], markers: list[Marker]
) -> list[TrajectorySegment]:
    """为首次专属模板提供安全默认值，不覆盖用户已保存的路线配置。

    唯一可无歧义自动识别的情形是：两个状态机之间恰有一个 type=7 掉头桩。
    此时普通插值会忽略桩桶并可能切入禁区，所以模板默认写入预设 4；
    其他所有连接仍保持 interpolated，等待用户根据现场路线选择。
    """
    suggested: list[TrajectorySegment] = []
    for trajectory in trajectories:
        if trajectory.source_exit_order is None or trajectory.target_entry_order is None:
            suggested.append(trajectory)
            continue
        stake_count = sum(
            marker.point_type == 7
            and trajectory.source_exit_order < marker.order < trajectory.target_entry_order
            for marker in markers
        )
        preset = (
            TransitionPreset.TURNAROUND_STAKE_FASTEST
            if stake_count == 1
            else trajectory.preset
        )
        suggested.append(replace(trajectory, preset=preset))
    return suggested


def nav_toml_path_for_source(source: Path) -> Path:
    """每个原始点表拥有独立的专属 TOML，避免不同赛道相互污染。"""
    return NAV_TOML_DIR / f"{source.stem}.toml"


def state_machine_identifiers(state_segments: list[StateMachineSegment]) -> list[str]:
    """给同类型状态机附加出现次数，例如 4#1、1#2。"""
    counts: dict[int, int] = {}
    identifiers: list[str] = []
    for state in state_segments:
        counts[state.entry_type] = counts.get(state.entry_type, 0) + 1
        identifiers.append(f"{state.entry_type}#{counts[state.entry_type]}")
    return identifiers


def route_state_signature(state_segments: list[StateMachineSegment]) -> str:
    """仅由状态机拓扑构造签名，用于阻止旧专属配置套用到新点表顺序。"""
    payload = "|".join(
        f"{state.entry_order}:{state.entry_type}>{state.exit_order}:{state.exit_type}"
        for state in state_segments
    )
    return hashlib.sha256(payload.encode("utf-8")).hexdigest()[:16]


def toml_speed_profile_lines(profile: SpeedPlanningProfile) -> list[str]:
    """生成完整段级速度参数；专属 TOML 使用户无需回查通用配置。"""
    return [
        f"path_speed_max_mm_s = {profile.path_speed_max_mm_s:.6g}",
        f"sprint_speed_mm_s = {profile.sprint_speed_mm_s:.6g}",
        f"enable_finish_sprint = {'true' if profile.enable_finish_sprint else 'false'}",
        f"max_accel_mm_s2 = {profile.max_accel_mm_s2:.6g}",
        f"max_decel_mm_s2 = {profile.max_decel_mm_s2:.6g}",
        f"max_lateral_accel_mm_s2 = {profile.max_lateral_accel_mm_s2:.6g}",
        f"max_path_yaw_rate_rad_s = {profile.max_path_yaw_rate_rad_s:.6g}",
        f"speed_to_mm_s = {profile.speed_to_mm_s:.6g}",
        f"curvature_eps = {profile.curvature_eps:.6g}",
        f"response_delay_s = {profile.response_delay_s:.6g}",
    ]


def toml_task_profile_lines(profile: TaskSpeedProfile) -> list[str]:
    """将状态机档案完整写入路线专属 TOML，避免继续依赖通用文件。"""
    return [
        f"approach_distance_mm = {profile.approach_distance_mm:.6g}",
        f"entry_speed_command = {profile.entry_speed_command:.6g}",
        f"state_speed_command = {profile.state_speed_command:.6g}",
        f"exit_speed_command = {profile.exit_speed_command:.6g}",
        f"entry_corridor_mm = {profile.entry_corridor_mm:.6g}",
        f"exit_corridor_mm = {profile.exit_corridor_mm:.6g}",
    ]


def write_nav_toml_template(
    output: Path,
    source: Path,
    state_segments: list[StateMachineSegment],
    trajectories: list[TrajectorySegment],
    configuration: PlanningConfiguration,
) -> None:
    """首次发现点表时写入可直接编辑的路线专属 TOML 模板。"""
    identifiers = state_machine_identifiers(state_segments)
    lines = [
        "# Plan4 路线专属轨迹规划配置（自动生成）",
        "#",
        "# 本文件是生成时对通用配置的完整复制，仅作用于当前点表。",
        "# 修改本文件的状态机、掉头桩或逐段速度参数，不会改动 ../plan4_speed_planning.toml。",
        "# 修改本文件后重新运行脚本，脚本会自动读取并生成轨迹和渲染图。",
        "# 可用 preset：interpolated、near_parallel、pure_line、turnaround_stake_fastest、turnaround_stake_smooth。",
        "# 两种 turnaround_stake 预设都要求该段点表中有且仅有一个 point_type=7 掉头桩。",
        "",
        "[route]",
        f"source_csv = \"{source.name}\"",
        f"state_signature = \"{route_state_signature(state_segments)}\"",
        "",
        "# 掉头桩的禁入半径 = radius_mm + clearance_mm；仅预设 4/5 使用此参数。",
        "[turnaround_stake]",
        f"radius_mm = {configuration.turnaround_stake_radius_mm:.6g}",
        f"clearance_mm = {configuration.turnaround_stake_clearance_mm:.6g}",
        "",
        "# 每类状态机的完整参数复制自通用 TOML。即使本点表暂未出现某类状态机，也保留",
        "# 该段，以便点表后续小幅调整时可继续在本文件内完成调参。",
    ]
    for point_type in sorted(configuration.task_profiles):
        lines.extend([
            "",
            f"[task.\"{point_type}\"]",
            *toml_task_profile_lines(configuration.task_profiles[point_type]),
        ])

    lines.extend([
        "",
        "# 以下状态机段由点表自动识别，仅用于人工核对。",
    ])
    for identifier, state in zip(identifiers, state_segments):
        lines.extend([
            "",
            f"[state_machine.\"{identifier}\"]",
            f"entry_order = {state.entry_order}",
            f"entry_type = {state.entry_type}",
            f"exit_order = {state.exit_order}",
            f"exit_type = {state.exit_type}",
        ])

    lines.extend([
        "",
        "# 每个普通轨迹段的详细参数。键采用点表事件序号，首尾以 start/end 标识。",
        "# 速度参数已从该段 preset 对应的通用默认段完整复制；在这里可逐项修改。",
        "# 修改 preset 后请同步检查下面的速度参数，它们不会再随通用 TOML 自动改变。",
        "# response_delay_s 只提前当前轨迹段内的减速指令，不会跨越该段的起止锚点。",
    ])
    for trajectory in trajectories:
        key = trajectory_config_key(trajectory)
        lines.extend([
            "",
            f"[trajectory.\"{key}\"]",
            f"preset = \"{trajectory.preset.value}\"",
            *toml_speed_profile_lines(trajectory.speed_profile),
        ])
        if trajectory.preset in {
            TransitionPreset.TURNAROUND_STAKE_FASTEST,
            TransitionPreset.TURNAROUND_STAKE_SMOOTH,
        }:
            lines.extend([
                "# 必经普通点的点表 index。绕桩曲线会以 G2 连续方式精确经过这些点。",
                "# 例如单边桥出口 8 到颠簸路入口 20 可设为 [9, 10]。",
                f"must_pass_marker_orders = {list(trajectory.must_pass_marker_orders)}",
                "# 必经点几何校验容差（mm）；当前实现精确过点，实际误差通常为 0。",
                f"must_pass_tolerance_mm = {trajectory.must_pass_tolerance_mm:.6g}",
            ])
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def overlay_route_shared_configuration(
    raw: dict[str, object], base: PlanningConfiguration
) -> PlanningConfiguration:
    """用路线 TOML 的共享段覆盖通用配置，路线修改不会写回通用 TOML。"""
    task_profiles = dict(base.task_profiles)
    raw_tasks = raw.get("task", {})
    if not isinstance(raw_tasks, dict):
        raise ValueError("路线专属 TOML 的 [task] 必须是 TOML 表。")
    for key, values in raw_tasks.items():
        try:
            point_type = int(key)
        except ValueError as exc:
            raise ValueError(f"路线专属 TOML 的 [task] 键必须是状态机入口类型，例如 '3'。") from exc
        if point_type not in task_profiles or not isinstance(values, dict):
            raise ValueError(f"路线专属 TOML 的 [task.{key}] 不是可配置的配对状态机。")
        task_profiles[point_type] = overlay_task_profile(
            task_profiles[point_type], values, f"路线专属 [task.{key}]"
        )

    raw_turnaround = raw.get("turnaround_stake", {})
    if not isinstance(raw_turnaround, dict):
        raise ValueError("路线专属 TOML 的 [turnaround_stake] 必须是 TOML 表。")
    allowed_turnaround = {"radius_mm", "clearance_mm"}
    unknown_turnaround = set(raw_turnaround) - allowed_turnaround
    if unknown_turnaround:
        raise ValueError(
            "路线专属 [turnaround_stake] 包含不支持的参数: "
            f"{', '.join(sorted(unknown_turnaround))}"
        )
    radius = float(raw_turnaround.get("radius_mm", base.turnaround_stake_radius_mm))
    clearance = float(raw_turnaround.get("clearance_mm", base.turnaround_stake_clearance_mm))
    if radius <= 0.0 or clearance < 0.0:
        raise ValueError("路线专属 [turnaround_stake] radius_mm 必须为正，clearance_mm 不能为负。")
    return PlanningConfiguration(
        task_profiles, base.preset_speed_profiles, radius, clearance
    )


def load_nav_toml_configuration(
    config_path: Path,
    source: Path,
    state_segments: list[StateMachineSegment],
    trajectories: list[TrajectorySegment],
    base_configuration: PlanningConfiguration,
) -> tuple[PlanningConfiguration, list[TrajectorySegment]]:
    """读取专属 TOML，将共享参数、连接预设和逐段速度应用到当前路线。"""
    try:
        with config_path.open("rb") as config_file:
            raw = tomllib.load(config_file)
    except tomllib.TOMLDecodeError as exc:
        raise ValueError(f"路线专属 TOML 格式错误: {exc}") from exc
    route = raw.get("route")
    if not isinstance(route, dict):
        raise ValueError("专属 TOML 缺少 [route]。")
    if route.get("source_csv") != source.name:
        raise ValueError("专属 TOML 对应的 source_csv 与当前点表不一致。")
    if route.get("state_signature") != route_state_signature(state_segments):
        raise ValueError(
            "当前点表的状态机数量或顺序与专属 TOML 不一致。请删除该专属 TOML 后重新运行以生成新模板。"
        )
    route_configuration = overlay_route_shared_configuration(raw, base_configuration)
    raw_trajectory = raw.get("trajectory")
    if not isinstance(raw_trajectory, dict):
        raise ValueError("专属 TOML 缺少 [trajectory]。")
    expected = {trajectory_config_key(trajectory) for trajectory in trajectories}
    supplied = set(raw_trajectory)
    missing = expected - supplied
    extra = supplied - expected
    if missing or extra:
        details = []
        if missing:
            details.append(f"缺少: {', '.join(sorted(missing))}")
        if extra:
            details.append(f"多余: {', '.join(sorted(extra))}")
        raise ValueError("专属 TOML 的轨迹段与当前点表不一致（" + "；".join(details) + "）。")

    updated: list[TrajectorySegment] = []
    allowed = set(SpeedPlanningProfile.__dataclass_fields__) | {
        "preset",
        "must_pass_marker_orders",
        "must_pass_tolerance_mm",
    }
    for trajectory in trajectories:
        key = trajectory_config_key(trajectory)
        values = raw_trajectory[key]
        if not isinstance(values, dict):
            raise ValueError(f"[trajectory.{key}] 必须是 TOML 表。")
        unknown = set(values) - allowed
        if unknown:
            raise ValueError(f"[trajectory.{key}] 包含不支持的参数: {', '.join(sorted(unknown))}")
        preset_value = values.get("preset")
        if not isinstance(preset_value, str):
            raise ValueError(f"[trajectory.{key}].preset 必须是字符串。")
        try:
            preset = TransitionPreset(preset_value)
        except ValueError as exc:
            valid = ", ".join(item.value for item in TransitionPreset)
            raise ValueError(f"[trajectory.{key}].preset 不支持 '{preset_value}'，可用值: {valid}") from exc
        raw_must_pass_orders = values.get("must_pass_marker_orders", [])
        if not isinstance(raw_must_pass_orders, list) or any(
            isinstance(order, bool) or not isinstance(order, int)
            for order in raw_must_pass_orders
        ):
            raise ValueError(
                f"[trajectory.{key}].must_pass_marker_orders 必须是整数点表 index 数组。"
            )
        must_pass_orders = tuple(raw_must_pass_orders)
        if len(set(must_pass_orders)) != len(must_pass_orders):
            raise ValueError(f"[trajectory.{key}].must_pass_marker_orders 不能重复。")
        tolerance = float(values.get("must_pass_tolerance_mm", 20.0))
        if tolerance <= 0.0:
            raise ValueError(f"[trajectory.{key}].must_pass_tolerance_mm 必须为正数。")
        if preset not in {
            TransitionPreset.TURNAROUND_STAKE_FASTEST,
            TransitionPreset.TURNAROUND_STAKE_SMOOTH,
        } and must_pass_orders:
            raise ValueError(
                f"[trajectory.{key}] 只有 turnaround_stake 预设可以配置必经点。"
            )
        speed_values = {
            name: value for name, value in values.items()
            if name not in {"preset", "must_pass_marker_orders", "must_pass_tolerance_mm"}
        }
        speed_profile = overlay_speed_profile(trajectory.speed_profile, speed_values, f"[trajectory.{key}]")
        updated.append(replace(
            trajectory,
            preset=preset,
            speed_profile=speed_profile,
            must_pass_marker_orders=must_pass_orders,
            must_pass_tolerance_mm=tolerance,
        ))
    return route_configuration, updated


