#!/usr/bin/env python3
"""Plan4 路径规划命令行入口。具体能力按职责拆分至 plan4_*.py 模块。"""
from __future__ import annotations

import argparse
import sys
from dataclasses import replace
from pathlib import Path

import numpy as np

from path_and_speed.plan4_config import (
    apply_configuration_to_trajectories,
    load_nav_toml_configuration,
    load_planning_configuration,
    nav_toml_path_for_source,
    suggest_initial_trajectory_presets,
    write_nav_toml_template,
)
from path_and_speed.plan4_geometry import generate_path_with_point_cap
from path_and_speed.plan4_models import (
    DEFAULT_HEADER,
    DEFAULT_PLANNING_CONFIG,
    NAV_ROUTE_MAX_POINTS,
    SCRIPT_DIR,
    SPEED_RESPONSE_DELAY_S,
    SPEED_TO_MM_S,
)
from path_and_speed.plan4_output import render, render_speed_heatmap, write_c_header
from path_and_speed.plan4_route import (
    apply_special_exit_corrections,
    apply_trajectory_marker_policy,
    build_state_machine_segments,
    build_transition_plans,
    choose_trajectory_segments,
    configure_segment_parameters,
    find_event_pairs,
    find_latest_marker_csv,
    print_segment_plan,
    read_markers,
)
from path_and_speed.plan4_speed import (
    apply_constant_task_speeds,
    apply_fixed_speed_envelope,
    apply_response_delay_compensation,
    build_sample_speed_profiles,
    build_task_speed_ranges,
    calculate_target_speed,
    calculate_yaw_and_curvature,
)

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="生成带可配置状态机进出直线和分段速度参数的 Plan4 平滑路径。")
    parser.add_argument("--input", type=Path, help="输入标记 CSV（默认自动选择本目录最新原始 CSV）")
    parser.add_argument("--output-csv", type=Path, help="输出路径 CSV，默认与输入同目录并追加 _planned")
    parser.add_argument("--render", type=Path, help="输出 PNG，默认与输入同目录并追加 _planned")
    parser.add_argument("--header", type=Path, default=DEFAULT_HEADER, help="C 路表输出位置（默认 code/navigation/nav_replay_route_table.h）")
    parser.add_argument(
        "--config",
        type=Path,
        default=DEFAULT_PLANNING_CONFIG,
        help="速度规划 TOML 配置文件（默认 tools/webview_nav_marker科目四/速度规划config/plan4_speed_planning.toml）",
    )
    parser.add_argument(
        "--stairs-approach-distance-mm",
        type=float,
        default=None,
        help="可选：覆盖配置文件中 task.3.approach_distance_mm",
    )
    parser.add_argument(
        "--non-interactive-transitions",
        action="store_true",
        help="兼容旧命令行参数；连接预设现由路线专属 TOML 决定",
    )
    parser.add_argument(
        "--configure-segment-parameters",
        action="store_true",
        help="在选择连接预设后，逐段编辑状态机进出直线长度和轨迹速度参数",
    )
    parser.add_argument(
        "--turnaround-stake-radius-mm",
        type=float,
        default=None,
        help="可选：覆盖配置文件中 turnaround_stake.radius_mm",
    )
    parser.add_argument(
        "--turnaround-stake-clearance-mm",
        type=float,
        default=None,
        help="可选：覆盖配置文件中 turnaround_stake.clearance_mm",
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
    if args.stairs_approach_distance_mm is not None and args.stairs_approach_distance_mm < 0.0:
        raise ValueError("Three-step approach distance must not be negative.")
    if args.speed_response_delay_s < 0.0:
        raise ValueError("Speed response delay must not be negative.")
    if args.turnaround_stake_radius_mm is not None and args.turnaround_stake_radius_mm <= 0.0:
        raise ValueError("掉头桩半径必须为正数。")
    if args.turnaround_stake_clearance_mm is not None and args.turnaround_stake_clearance_mm < 0.0:
        raise ValueError("掉头桩安全裕量不能为负数。")
    source = args.input.resolve() if args.input else find_latest_marker_csv(SCRIPT_DIR)
    if not source.is_file():
        raise FileNotFoundError(f"找不到默认输入 CSV: {source}。请导出该文件或使用 --input 指定 CSV。")
    csv_output = args.output_csv or source.with_name(f"{source.stem}_planned.csv")
    render_output = args.render or source.with_name(f"{source.stem}_planned.png")

    configuration_path = args.config.resolve()
    configuration = load_planning_configuration(configuration_path)
    # 配置文件是默认来源；命令行参数仅在用户显式提供时覆盖相应配置。
    task_speed_profiles = dict(configuration.task_profiles)
    if args.stairs_approach_distance_mm is not None:
        task_speed_profiles[3] = replace(
            task_speed_profiles[3], approach_distance_mm=args.stairs_approach_distance_mm
        )
    turnaround_stake_radius_mm = (
        args.turnaround_stake_radius_mm
        if args.turnaround_stake_radius_mm is not None
        else configuration.turnaround_stake_radius_mm
    )
    turnaround_stake_clearance_mm = (
        args.turnaround_stake_clearance_mm
        if args.turnaround_stake_clearance_mm is not None
        else configuration.turnaround_stake_clearance_mm
    )

    markers, start_heading = read_markers(source)
    pairs = find_event_pairs(markers)
    markers = apply_special_exit_corrections(markers, pairs)
    # 先从原始点表建立交替的“状态机段 / 轨迹段”计划。首次运行只生成
    # 可编辑的路线专属 TOML，不输出任何路表。
    state_segments = build_state_machine_segments(markers, pairs, task_speed_profiles)
    trajectory_segments, _ = choose_trajectory_segments(state_segments, interactive=False)
    nav_configuration_path = nav_toml_path_for_source(source)
    if not nav_configuration_path.is_file():
        trajectory_segments = suggest_initial_trajectory_presets(trajectory_segments, markers)
        # 预设 4 等自动建议必须先完成，再复制相应预设的默认速度参数到专属 TOML。
        trajectory_segments = apply_configuration_to_trajectories(trajectory_segments, configuration)
        write_nav_toml_template(
            nav_configuration_path, source, state_segments, trajectory_segments, configuration
        )
        print(f"已生成路线专属配置: {nav_configuration_path}")
        print("请修改其中的 [task.*]、[turnaround_stake]、每段 [trajectory.*] 的 preset 和速度参数，")
        print("然后重新运行本脚本生成轨迹与渲染图。上述修改只影响这一份点表。")
        return 0
    # 专属配置控制该点表的状态机、掉头桩、连接预设和逐段速度；拓扑签名
    # 不匹配时会停止，防止状态机数目或顺序改变后意外套用旧路线参数。
    route_configuration, _ = load_nav_toml_configuration(
        nav_configuration_path, source, state_segments, trajectory_segments, configuration
    )
    task_speed_profiles = dict(route_configuration.task_profiles)
    if args.stairs_approach_distance_mm is not None:
        task_speed_profiles[3] = replace(
            task_speed_profiles[3], approach_distance_mm=args.stairs_approach_distance_mm
        )
    turnaround_stake_radius_mm = (
        args.turnaround_stake_radius_mm
        if args.turnaround_stake_radius_mm is not None
        else route_configuration.turnaround_stake_radius_mm
    )
    turnaround_stake_clearance_mm = (
        args.turnaround_stake_clearance_mm
        if args.turnaround_stake_clearance_mm is not None
        else route_configuration.turnaround_stake_clearance_mm
    )
    # task.* 可以改入口/出口走廊与速度，因此读取专属配置后必须重新建立
    # 状态机段和它们相邻的轨迹段，才能把本路线的局部参数传入几何与速度规划。
    state_segments = build_state_machine_segments(markers, pairs, task_speed_profiles)
    trajectory_segments, _ = choose_trajectory_segments(state_segments, interactive=False)
    route_configuration, trajectory_segments = load_nav_toml_configuration(
        nav_configuration_path, source, state_segments, trajectory_segments, configuration
    )
    if args.configure_segment_parameters:
        task_speed_profiles, trajectory_segments = configure_segment_parameters(
            state_segments, trajectory_segments, task_speed_profiles
        )
        # 状态机段也持有入口/出口速度与几何档案，配置后重新建立以保持显示一致。
        state_segments = build_state_machine_segments(markers, pairs, task_speed_profiles)
    print_segment_plan(state_segments, trajectory_segments)
    transition_plans = build_transition_plans(trajectory_segments, markers)
    markers, removed_marker_count = apply_trajectory_marker_policy(markers, trajectory_segments)
    samples, effective_sample_step_mm = generate_path_with_point_cap(
        markers,
        transition_plans,
        task_speed_profiles,
        turnaround_stake_radius_mm,
        turnaround_stake_clearance_mm,
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
    print(f"速度规划配置: {configuration_path}")
    print(f"路线专属配置: {nav_configuration_path}")
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
