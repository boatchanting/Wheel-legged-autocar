#!/usr/bin/env python3
"""Plan4 的共享数据模型、路线常量和默认参数。"""
from __future__ import annotations

import math
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Optional

import numpy as np

# 实现模块位于 path_and_speed 子目录；点表、配置和入口脚本仍在其父目录。
SCRIPT_DIR = Path(__file__).resolve().parent.parent
GENERATOR_SCRIPT_NAME = "generate_plan4_smooth_path_考虑响应延迟_丝滑轨迹.py"
PROJECT_ROOT = SCRIPT_DIR.parent.parent
DEFAULT_HEADER = PROJECT_ROOT / "code" / "navigation" / "nav_replay_route_table.h"
DEFAULT_PLANNING_CONFIG = SCRIPT_DIR / "速度规划config" / "plan4_speed_planning.toml"
NAV_TOML_DIR = DEFAULT_PLANNING_CONFIG.parent / "nav_toml"
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

# 每个普通轨迹段的响应延迟默认值。延迟补偿只提前该段内的减速指令，
# 不会读取或改写相邻轨迹段的速度规划结果。
SPEED_RESPONSE_DELAY_S = 0.0

# 局部圆角控制柄同时受相邻边长限制，避免稀疏或急转标记使 Bezier 曲线偏离局部走廊。
LOCAL_CORNER_HANDLE_RATIO = 0.18
LOCAL_CORNER_NEIGHBOR_RATIO = 0.35
LOCAL_CORNER_HANDLE_MAX_MM = 600.0
# 近似平行型过渡使用独立的控制柄。它比普通局部圆角更长，适合两条
# 方向接近、但存在明显横向错位的状态机通道（例如三级跳 -> 单边桥）。
PARALLEL_TRANSITION_HANDLE_MM = 900.0
# 掉头桩的安全圆半径 = 桩桶半径 + 车辆通过时额外预留的安全裕量。
# 两个值可通过命令行覆盖，单位均为 mm。
TURNAROUND_STAKE_RADIUS_MM = 400.0
TURNAROUND_STAKE_CLEARANCE_MM = 250.0

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
ENTRY_TYPES = {1, 2, 3, 4, 5, 11}
# These points are navigated by the Plan4 point-to-point controller. Type 1
# starts the minefield state machine; type 11 is a pass-through target only.
POINT_TO_POINT_TYPES = {1, 11}
PAIRED_ENTRY_TYPES = {2, 3, 4, 5}
EXIT_TO_ENTRY = {20: 2, 30: 3, 40: 4, 50: 5}
TYPE_LABEL = {
    0: "普通路径",
    1: "圆环进入",
    2: "坡道进入",
    3: "三级跳进入",
    4: "单边桥进入",
    5: "颠簸路进入",
    11: "雷区不停点",
    7: "掉头桩",
    10: "圆环退出",
    20: "坡道退出",
    30: "三级跳退出",
    40: "单边桥退出",
    50: "颠簸路退出",
}
TYPE_COLOR = {
    0: "#3b82f6", 1: "#db2777",
    # 坡道绿色、三级跳红色、单边桥紫色、颠簸路黄色；进入/退出保持同色。
    2: "#16a34a", 3: "#dc2626", 4: "#9333ea", 5: "#eab308", 11: "#f97316",
    7: "#475569",
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
    TURNAROUND_STAKE_FASTEST = "turnaround_stake_fastest"
    TURNAROUND_STAKE_SMOOTH = "turnaround_stake_smooth"


TRANSITION_PRESET_LABEL = {
    TransitionPreset.INTERPOLATED: "轨迹插值型",
    TransitionPreset.NEAR_PARALLEL: "近似平行型",
    TransitionPreset.PURE_LINE: "纯直线型",
    TransitionPreset.TURNAROUND_STAKE_FASTEST: "带掉头桩丝滑型",
    TransitionPreset.TURNAROUND_STAKE_SMOOTH: "带掉头桩低曲率丝滑型",
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
    response_delay_s: float = SPEED_RESPONSE_DELAY_S


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
    # 仅预设 4 使用：在绕桩前/后必须平滑通过的普通点表序号。
    must_pass_marker_orders: tuple[int, ...] = ()
    # 必经点允许的最大几何偏差；当前锚点实现会精确经过，因此是更严格的约束。
    must_pass_tolerance_mm: float = 20.0


@dataclass(frozen=True)
class TransitionPlan:
    """供几何生成使用的状态机间连接计划。"""

    source_exit_order: int
    target_entry_order: int
    preset: TransitionPreset
    source_exit_speed_command: Optional[float] = None
    target_entry_speed_command: Optional[float] = None
    stake: Optional[Marker] = None
    speed_profile: SpeedPlanningProfile = DEFAULT_TRAJECTORY_SPEED_PROFILE
    must_pass_markers: tuple[Marker, ...] = ()
    must_pass_tolerance_mm: float = 20.0


@dataclass(frozen=True)
class PlanningConfiguration:
    """从 TOML 文件读取的、仅作用于本次生成的完整规划配置。"""

    task_profiles: dict[int, TaskSpeedProfile]
    # 通用 TOML 按连接预设保存速度档案。首次创建路线专属 TOML 时，
    # 对应预设的档案会完整复制到每个 [trajectory."起点->终点"] 中。
    preset_speed_profiles: dict[TransitionPreset, SpeedPlanningProfile]
    turnaround_stake_radius_mm: float
    turnaround_stake_clearance_mm: float


