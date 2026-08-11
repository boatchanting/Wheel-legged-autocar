#!/usr/bin/env python3
"""生成并渲染一条基于标记的进出点、具备G2连续性的Plan3路径。

标记CSV文件描述了路线事件。一个特殊事件由一个入口标记（1..5）和其匹配的出口标记（10..50）表示。生成器会精确保留这两个标记，并预留两条*直*线走廊：

* 入口标记前方的500毫米；
* 出口标记后方的500毫米。

所有剩余的连接段均为五次贝塞尔曲线，且其两端的曲率为零。因此，相邻的连接段与直线走廊共享切线和曲率（即G2连续性）。生成的路径旨在作为连续跟踪的路线输入；它在普通采样点处不会发出停止指令。

示例：
    .venv\\Scripts\\python.exe tools\\webview_nav_marker科目三\\generate_plan3_smooth_path.py

默认的CSV文件特意指定为项目要求的最新Plan3记录。如果需要基于其他记录生成路径，请传入 ``--input`` 参数。
"""

from __future__ import annotations

import argparse
import csv
import math
import sys
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Iterable, Optional

import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent.parent
DEFAULT_HEADER = PROJECT_ROOT / "code" / "navigation" / "nav_replay_route_table.h"
STRAIGHT_LENGTH_MM = 600.0
SAMPLE_STEP_MM = 50.0
# 先以更密的几何点生成路径，再统一按弧长重采样，避免最终路表点距影响曲率估计。
DENSE_SAMPLE_STEP_MM = 5.0
MIN_LINK_LENGTH_MM = 5.0
NAV_ROUTE_MAX_POINTS = 5000
START_POINT_X_MM = 0.0
START_POINT_Y_MM = 0.0

# 与 tools/webview_nav_marker速度规划/caculate_path.py 保持一致的离线路径速度约束。
PATH_SPEED_MAX_MM_S = 4000.0
SPRINT_SPEED_MM_S = 4000.0
ENABLE_FINISH_SPRINT = True
MAX_ACCEL_MM_S2 = 1500.0
MAX_DECEL_MM_S2 = 1500.0
MAX_LATERAL_ACCEL_MM_S2 = 3500.0
MAX_PATH_YAW_RATE_RAD_S = 2.8
SPEED_TO_MM_S = 4.79
CURVATURE_EPS = 1e-6

# 局部圆角控制柄同时受相邻边长限制，避免稀疏或急转标记使 Bezier 曲线偏离局部走廊。
LOCAL_CORNER_HANDLE_RATIO = 0.18
LOCAL_CORNER_NEIGHBOR_RATIO = 0.35
LOCAL_CORNER_HANDLE_MAX_MM = 600.0

# 以下限速值使用写入 NavRamPoint_t 的 target_speed 指令单位。
STAIRS_APPROACH_DISTANCE_MM = 4000.0
STAIRS_APPROACH_TARGET_SPEED_MAX = 220.0
BRIDGE_APPROACH_DISTANCE_MM = 2000.0
BRIDGE_APPROACH_TARGET_SPEED_MAX = 300.0

# 与 csv_to_nav_table.py 保持一致：每个值都从记录的出口点朝对应入口点测量，
# 使视觉状态机出口锚定在车辆实际离开任务的位置。
# 特殊状态机结束点的沿线修正距离（单位：CSV 坐标单位；当前导航坐标单位为毫米 mm）。
# 30：三级台阶结束点，对应状态机进入点类型 3（3 -> 30），单位：mm。
# 40：单边桥结束点，对应状态机进入点类型 4（4 -> 40），单位：mm。
# 50：颠簸路段结束点，对应状态机进入点类型 5（5 -> 50），单位：mm。
# 正值：结束点沿“结束点 -> 对应进入点”的连线靠近进入点。
# 负值：结束点沿同一连线的反方向远离进入点。
# 如果发现少跑了，应该增大对应距离；多跑了，减少对应距离
SPECIAL_EXIT_DISTANCE_OFFSETS_MM = {30: -150.0, 40: 750.0, 50: 850.0}


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


@dataclass
class PathSample:
    x: float
    y: float
    point_type: int
    forced_straight: bool
    segment: str


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
        if offset <= 0.0:
            continue
        direction = unit(np.array([entry.x - exit_marker.x, entry.y - exit_marker.y], dtype=float))
        corrected[exit_index] = replace(
            exit_marker,
            x=exit_marker.x + offset * direction[0],
            y=exit_marker.y + offset * direction[1],
        )
    return corrected


def add_sample(samples: list[PathSample], point: np.ndarray, point_type: int, forced: bool, segment: str) -> None:
    if samples and math.hypot(samples[-1].x - point[0], samples[-1].y - point[1]) < 0.01:
        # 若事件点与前一个几何点重合，保留事件标签。
        if point_type != 0:
            samples[-1].point_type = point_type
        samples[-1].forced_straight |= forced
        return
    samples.append(PathSample(float(point[0]), float(point[1]), point_type, forced, segment))


def append_line(samples: list[PathSample], start: Node, end: Node, segment: str, forced: bool, sample_step_mm: float) -> None:
    start_vec = np.array([start.x, start.y], dtype=float)
    end_vec = np.array([end.x, end.y], dtype=float)
    length = float(np.linalg.norm(end_vec - start_vec))
    count = max(1, int(math.ceil(length / sample_step_mm)))
    for i in range(count + 1):
        ratio = i / count
        event_type = start.point_type if i == 0 else (end.point_type if i == count else 0)
        add_sample(samples, start_vec + ratio * (end_vec - start_vec), event_type, forced, segment)


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
        add_sample(samples, end_vec, end.point_type, False, segment)
        return

    tangent_start = unit(start.tangent if start.tangent is not None else chord)
    tangent_end = unit(end.tangent if end.tangent is not None else chord)
    # 两端控制柄独立限幅，等效于局部圆角，使曲线靠近相邻锚点，避免短边引起过度外扩。
    default_handle = min(LOCAL_CORNER_HANDLE_RATIO * chord_length, LOCAL_CORNER_HANDLE_MAX_MM)
    start_handle = min(default_handle, start.corner_handle_mm or default_handle)
    end_handle = min(default_handle, end.corner_handle_mm or default_handle)
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
        add_sample(samples, np.array([px, py]), start.point_type if index == 0 else (end.point_type if index == len(x) - 1 else 0), False, segment)


def make_nodes(markers: list[Marker], pairs: dict[int, int]) -> tuple[list[Node], dict[tuple[int, int], bool]]:
    """展开特殊标记对，并在开头补充未记录的车辆原点。"""
    # 录制是在车辆驶离 (0, 0) 原点之后才开始的。
    # 包含这个虚拟节点可以使得表中的第一条记录成为真正的路线起点。

    nodes: list[Node] = [Node(START_POINT_X_MM, START_POINT_Y_MM, 0, None, "origin (0, 0)")]
    # 仅针对所要求的0.5米驶入/驶出走廊，将值设为True。
    # 入口到出口的连接段是一个直接的任务区域占位符；故意的，
    # 它不被算作导航路径的强制直线段。

    straight_links: dict[tuple[int, int], bool] = {}
    entry_by_exit = {exit_index: entry_index for entry_index, exit_index in pairs.items()}
    marker_index = 0

    while marker_index < len(markers):
        marker = markers[marker_index]
        if marker_index in pairs:
            exit_index = pairs[marker_index]
            exit_marker = markers[exit_index]
            axis = unit(np.array([exit_marker.x - marker.x, exit_marker.y - marker.y], dtype=float))
            pre = Node(marker.x - STRAIGHT_LENGTH_MM * axis[0], marker.y - STRAIGHT_LENGTH_MM * axis[1], 0, axis, f"{TYPE_LABEL[marker.point_type]}前直线起点")
            entry = Node(marker.x, marker.y, marker.point_type, axis, TYPE_LABEL[marker.point_type])
            exit_node = Node(exit_marker.x, exit_marker.y, exit_marker.point_type, axis, TYPE_LABEL[exit_marker.point_type])
            post = Node(exit_marker.x + STRAIGHT_LENGTH_MM * axis[0], exit_marker.y + STRAIGHT_LENGTH_MM * axis[1], 0, axis, f"{TYPE_LABEL[exit_marker.point_type]}后直线终点")
            if nodes and distance(nodes[-1], pre) < MIN_LINK_LENGTH_MM:
                raise ValueError(f"{entry.name} 前 0.5m 直线与上一锚点重叠，无法构造平滑连接。")
            nodes.extend((pre, entry, exit_node, post))
            base = len(nodes) - 4
            straight_links[(base, base + 1)] = True
            straight_links[(base + 1, base + 2)] = False
            straight_links[(base + 2, base + 3)] = True
            marker_index = exit_index + 1
            continue
        if marker_index in entry_by_exit:
            marker_index += 1
            continue
        nodes.append(Node(marker.x, marker.y, marker.point_type, None, TYPE_LABEL[marker.point_type]))
        marker_index += 1

    if len(nodes) < 2:
        raise ValueError("展开后路径锚点不足。")

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
            node.corner_handle_mm = min(
                LOCAL_CORNER_HANDLE_MAX_MM,
                LOCAL_CORNER_NEIGHBOR_RATIO * min(incoming_length, outgoing_length),
            )
    return nodes, straight_links


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
        else:
            source_index = int(np.searchsorted(s_dense, arc, side="right") - 1)
            source_index = max(0, min(source_index, len(dense_samples) - 1))
            source = dense_samples[source_index]
            point = np.array([x[target_index], y[target_index]], dtype=float)
            point_type = 0
        add_sample(output, point, point_type, source.forced_straight, source.segment)
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


def apply_longitudinal_speed_envelope(speed_limit: np.ndarray, s: np.ndarray) -> np.ndarray:
    """Make a speed ceiling physically reachable in both travel directions."""
    planned_speed = np.array(speed_limit, copy=True)
    for index in range(len(planned_speed) - 2, -1, -1):
        ds = s[index + 1] - s[index]
        planned_speed[index] = min(
            planned_speed[index],
            math.sqrt(max(0.0, planned_speed[index + 1] ** 2 + 2.0 * MAX_DECEL_MM_S2 * ds)),
        )
    for index in range(1, len(planned_speed)):
        ds = s[index] - s[index - 1]
        planned_speed[index] = min(
            planned_speed[index],
            math.sqrt(max(0.0, planned_speed[index - 1] ** 2 + 2.0 * MAX_ACCEL_MM_S2 * ds)),
        )
    return planned_speed


def calculate_target_speed(
    samples: list[PathSample],
    s: np.ndarray,
    curvature: np.ndarray,
) -> np.ndarray:
    """曲率包络限速，再执行与 caculate_path.py 相同的前后向加减速扫描。"""
    count = len(samples)
    if count == 0:
        return np.empty(0, dtype=float)

    speed_limit = np.full(count, PATH_SPEED_MAX_MM_S, dtype=float)
    for index, kappa in enumerate(curvature):
        abs_kappa = abs(float(kappa))
        curve_limit = PATH_SPEED_MAX_MM_S
        yaw_rate_limit = PATH_SPEED_MAX_MM_S
        if abs_kappa > CURVATURE_EPS:
            curve_limit = math.sqrt(MAX_LATERAL_ACCEL_MM_S2 / abs_kappa)
            yaw_rate_limit = MAX_PATH_YAW_RATE_RAD_S / abs_kappa
        speed_limit[index] = min(PATH_SPEED_MAX_MM_S, curve_limit, yaw_rate_limit)

    # 普通点和普通结束点都连续通过，不制造零速障碍；圆环动作仍允许明确停车。
    if ENABLE_FINISH_SPRINT:
        last_curve_index = max((i for i, value in enumerate(curvature) if abs(value) > 0.0005), default=-1)
        if 0 <= last_curve_index < count - 5:
            speed_limit[last_curve_index + 5:] = np.minimum(speed_limit[last_curve_index + 5:], SPRINT_SPEED_MM_S)
        speed_limit[-1] = min(speed_limit[-1], SPRINT_SPEED_MM_S)
    for index, sample in enumerate(samples):
        if sample.point_type == 1:  # NAV_POINT_CIRCLE 可能要求原地旋转。
            speed_limit[index] = 0.0

    return -apply_longitudinal_speed_envelope(speed_limit, s) / SPEED_TO_MM_S


def limit_stairs_approach_output_speed(
    samples: list[PathSample],
    s: np.ndarray,
    target_speed: np.ndarray,
    approach_distance_mm: float,
) -> np.ndarray:
    """Cap the table output for points within the three-step approach and the entire stairs segment."""
    # 先转换为物理速度单位，使任务限速插入后仍可应用与基础规划相同的加减速约束。
    speed_limit = np.abs(target_speed) * SPEED_TO_MM_S
    stairs_entry_s = [s[index] for index, sample in enumerate(samples) if sample.point_type == 3]
    stairs_exit_s = [s[index] for index, sample in enumerate(samples) if sample.point_type == 30]
    
    # 将入口和出口配对，确保完整覆盖整个stairs段
    for entry_s, exit_s in zip(stairs_entry_s, stairs_exit_s):
        # 限制范围：从 (入口 - approach_distance_mm) 到 (出口 + approach_distance_mm)
        full_stairs_range = (s >= entry_s - approach_distance_mm) & (s <= exit_s + approach_distance_mm)
        speed_limit[full_stairs_range] = np.minimum(
            speed_limit[full_stairs_range],
            STAIRS_APPROACH_TARGET_SPEED_MAX * SPEED_TO_MM_S,
        )
    return -apply_longitudinal_speed_envelope(speed_limit, s) / SPEED_TO_MM_S


def limit_bridge_approach_output_speed(
    samples: list[PathSample],
    s: np.ndarray,
    target_speed: np.ndarray,
    approach_distance_mm: float,
    target_speed_max: float,
) -> np.ndarray:
    """限制单边桥状态机入口前的速度，并重新建立纵向速度包络。"""
    speed_limit = np.abs(target_speed) * SPEED_TO_MM_S
    bridge_entry_s = [s[index] for index, sample in enumerate(samples) if sample.point_type == 4]
    for current_entry_s in bridge_entry_s:
        approach_range = (s >= current_entry_s - approach_distance_mm) & (s <= current_entry_s)
        speed_limit[approach_range] = np.minimum(speed_limit[approach_range], target_speed_max * SPEED_TO_MM_S)
    return -apply_longitudinal_speed_envelope(speed_limit, s) / SPEED_TO_MM_S



def generate_path(markers: list[Marker], sample_step_mm: float) -> list[PathSample]:
    pairs = find_event_pairs(markers)
    nodes, straight_links = make_nodes(markers, pairs)
    dense_samples: list[PathSample] = []
    for index in range(len(nodes) - 1):
        if (index, index + 1) in straight_links:
            is_forced_corridor = straight_links[(index, index + 1)]
            label = "强制直线" if is_forced_corridor else "任务区直连"
            append_line(dense_samples, nodes[index], nodes[index + 1], f"{label}: {nodes[index].name} -> {nodes[index + 1].name}", is_forced_corridor, DENSE_SAMPLE_STEP_MM)
        else:
            append_g2_link(dense_samples, nodes[index], nodes[index + 1], f"G2: {nodes[index].name} -> {nodes[index + 1].name}", DENSE_SAMPLE_STEP_MM)
    return resample_path_by_arclength(dense_samples, sample_step_mm)


def generate_path_with_point_cap(markers: list[Marker]) -> tuple[list[PathSample], float]:
    """Prefer 50 mm samples, while never generating a route that RAM truncates."""
    sample_step_mm = SAMPLE_STEP_MM
    for _ in range(8):
        samples = generate_path(markers, sample_step_mm)
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
    parser = argparse.ArgumentParser(description="生成带 0.5m 进出直线约束的 Plan3 平滑路径。")
    parser.add_argument("--input", type=Path, help="输入标记 CSV（默认自动选择本目录最新原始 CSV）")
    parser.add_argument("--output-csv", type=Path, help="输出路径 CSV，默认与输入同目录并追加 _planned")
    parser.add_argument("--render", type=Path, help="输出 PNG，默认与输入同目录并追加 _planned")
    parser.add_argument("--header", type=Path, default=DEFAULT_HEADER, help="C 路表输出位置（默认 code/navigation/nav_replay_route_table.h）")
    parser.add_argument(
        "--stairs-approach-distance-mm",
        type=float,
        default=STAIRS_APPROACH_DISTANCE_MM,
        help="three-step approach speed-cap distance in mm (default: 4000)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.stairs_approach_distance_mm < 0.0:
        raise ValueError("Three-step approach distance must not be negative.")
    source = args.input.resolve() if args.input else find_latest_marker_csv(SCRIPT_DIR)
    if not source.is_file():
        raise FileNotFoundError(f"找不到默认输入 CSV: {source}。请导出该文件或使用 --input 指定 CSV。")
    csv_output = args.output_csv or source.with_name(f"{source.stem}_planned.csv")
    render_output = args.render or source.with_name(f"{source.stem}_planned.png")

    markers, start_heading = read_markers(source)
    pairs = find_event_pairs(markers)
    markers = apply_special_exit_corrections(markers, pairs)
    samples, effective_sample_step_mm = generate_path_with_point_cap(markers)
    s, yaw, curvature = calculate_yaw_and_curvature(samples)
    target_speed = calculate_target_speed(samples, s, curvature)
    output_target_speed = limit_stairs_approach_output_speed(
        samples,
        s,
        target_speed,
        args.stairs_approach_distance_mm,
    )
    output_target_speed = limit_bridge_approach_output_speed(
        samples,
        s,
        output_target_speed,
        BRIDGE_APPROACH_DISTANCE_MM,
        BRIDGE_APPROACH_TARGET_SPEED_MAX,
    )
    speed_heatmap_output = render_output.with_name(f"{render_output.stem}_speed_heatmap{render_output.suffix}")
    # 如需导出路径 CSV，取消下一行注释。
    # write_csv(csv_output, samples, yaw, curvature, output_target_speed)
    render(render_output, markers, samples, s, curvature, output_target_speed)
    render_speed_heatmap(speed_heatmap_output, samples, output_target_speed)
    write_c_header(args.header, samples, yaw, curvature, output_target_speed, source, start_heading)
    print(f"速度热力图: {speed_heatmap_output}")

    # 即使 50 mm 采样点恰好落在边界上，几何长度仍按任务锚点精确计算，不从可视标签估算。
    forced_length = len(pairs) * 2.0 * STRAIGHT_LENGTH_MM
    print(f"输入: {source}")
    print(f"输出路径: {csv_output} ({len(samples)} 点, 总长 {s[-1]:.1f} mm)")
    print(f"采样间距: {effective_sample_step_mm:.1f} mm（上限 {NAV_ROUTE_MAX_POINTS} 点）")
    print(f"目标速度范围: {output_target_speed.min():.1f} ~ {output_target_speed.max():.1f}（负数为前进指令）")
    print(f"渲染图: {render_output}")
    print(f"强制直线累计长度: {forced_length:.1f} mm")
    print(f"C 路表: {args.header}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (FileNotFoundError, ValueError) as exc:
        print(f"错误: {exc}", file=sys.stderr)
        raise SystemExit(2) from exc
