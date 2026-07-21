#!/usr/bin/env python3
"""Optimized offline path and speed planner for plan1 slalom.

Input point semantics:
    point 0      : pseudo cone at the u-turn area; also marks the u-turn line
    point 1..n-2 : cone positions
    point n-1    : one marker point on the finish line, not a waypoint

The vehicle still starts from (0, 0).  The planner treats point 0..n-2 as
clearance obstacles/control cones, chooses a smooth track around them, crosses
the finish line reference, speed-plans the result, and exports the same
NavRamPoint_t table shape used by the original tool.
"""

from __future__ import annotations

import argparse
import csv as csv_mod
import math
import os
import re
import warnings
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Callable, Dict, Iterable, List, Optional, Sequence, Tuple

import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import splev, splprep


plt.rcParams["font.sans-serif"] = ["SimHei", "Microsoft YaHei", "Arial Unicode MS"]
plt.rcParams["axes.unicode_minus"] = False


INTERPOLATE_DIST = 50.0
PATH_DENSE_SAMPLE_MM = 10.0
SEGMENT_CHECK_STEP_MM = 35.0

MIN_CONE_CLEARANCE_MM = 400.0
CONE_RADIUS_MM = 140.0
CAR_HALF_WIDTH_MM = 135.0

PRE_UTURN_SPEED_MAX_MM_S = 5000.0
PATH_SPEED_MAX_MM_S = 6500.0
SPRINT_SPEED_MM_S = 3000.0
ENABLE_FINISH_SPRINT = True
MAX_ACCEL_MM_S2 = 1500.0
MAX_DECEL_MM_S2 = 1500.0
MAX_LATERAL_ACCEL_MM_S2 = 3500.0
MAX_PATH_YAW_RATE_RAD_S = 2.8
MAX_PATH_YAW_ACCEL_RAD_S2 = 8.0
SPEED_TO_MM_S = 4.936
CURVATURE_EPS = 1e-9

UTURN_OVER_LINE_MM = 400.0
UTURN_APPROACH_DISTANCES_MM = (1200.0, 2000.0, 3200.0, 4800.0, 6500.0)
UTURN_APPROACH_LATERAL_OFFSETS_MM = (-700.0, -350.0, 0.0, 350.0, 700.0)
UTURN_OVERLINE_DISTANCES_MM = (800.0, 1200.0, 1800.0, 2600.0, 3600.0)
UTURN_GUARD_RADIUS_OFFSETS_MM = (800.0, 1200.0, 1600.0, 2200.0, 3000.0)
FINISH_OVER_LINE_MM = 700.0
LINE_SAMPLE_COUNT = 11
GAP_SAMPLE_COUNT = 9
BEAM_WIDTH = 96

TURN_COST_WEIGHT = 900.0
CLEARANCE_COST_WEIGHT = 90000.0
CLEARANCE_SOFT_MARGIN_MM = 260.0
FIRST_GUARD_RADIUS_OFFSETS_MM = (1000.0, 2000.0, 3200.0, 4600.0)
FIRST_GUARD_ANGLE_OFFSETS_DEG = (-75.0, -55.0, -38.0, -22.0, 0.0, 22.0, 38.0, 55.0, 75.0)
GUARD_RADIUS_OFFSETS_MM = (220.0, 400.0, 650.0, 900.0)
GUARD_ANGLE_OFFSETS_DEG = (-50.0, -32.0, -16.0, 0.0, 16.0, 32.0, 50.0)
MAX_FINAL_CANDIDATES = 96
# A small positive smoothing factor keeps the path rounder without widening clearance.
SMOOTH_TENSIONS = (7.75,)
START_APPROACH_ANCHOR_FRACS = (0.2, 0.4, 0.6, 0.8)
FINISH_EXIT_ANCHOR_FRACS = (0.33, 0.66)
SPRINT_ENTRY_AFTER_LAST_CONE_MM = (500.0, 800.0, 1100.0, 1450.0, 1800.0, 2200.0, 2700.0)
SPRINT_ENTRY_DISTANCE_FRACS = (0.35, 0.50, 0.65, 0.78)
SPRINT_ENTRY_MIN_LINE_APPROACH_MM = 450.0
SPRINT_ENTRY_LATERAL_OFFSETS_MM = (0.0, 560.0, 760.0, 980.0, 1250.0, 1550.0)
SPRINT_TAIL_EXCESS_WEIGHT = 3.5
SPRINT_TAIL_CHORD_WEIGHT = 0.18
SPRINT_TAIL_CURVATURE_WEIGHT = 3.0e6
SPRINT_TAIL_CURVATURE_RATE_WEIGHT = 1.6e9

FLOAT_RE = r"[+-]?(?:\d+(?:\.\d*)?|\.\d+)"


POINT_TYPE_TOKENS: Dict[str, int] = {
    "NAV_POINT_PATH": 0,
    "NAV_POINT_CIRCLE": 1,
    "NAV_POINT_SLOPE": 2,
    "NAV_POINT_JUMP": 3,
    "NAV_POINT_BRIDGE": 4,
    "NAV_POINT_BUMP": 5,
}

SPECIAL_POINTS_MAP = {
    1: ("圆环点", "#D946EF", "o"),
    2: ("坡道点", "#F59E0B", "^"),
    3: ("掉头动作点", "#06B6D4", "v"),
    4: ("桥面点", "#92400E", "s"),
    5: ("颠簸点", "#7C3AED", "D"),
}


@dataclass
class RoutePoint:
    x: float
    y: float
    target_yaw_deg: Optional[float]
    heading_deg: Optional[float]
    target_speed: float
    point_type: int
    curvature: float = 0.0


@dataclass(frozen=True)
class CandidatePoint:
    x: float
    y: float
    label: str


@dataclass
class OptimizedPathResult:
    raw_route: List[Tuple[float, float]]
    final_points: List[RoutePoint]
    action_xy: Tuple[float, float]
    min_clearance_mm: float
    score: float


class PathConstraintError(ValueError):
    """Raised when no path can satisfy the geometric constraints."""


def normalize_relative_yaw_deg(value: float) -> float:
    while value > 180.0:
        value -= 360.0
    while value <= -180.0:
        value += 360.0
    return value


def normalize_heading_deg(value: float) -> float:
    value = math.fmod(value, 360.0)
    if value < 0.0:
        value += 360.0
    return value


def calc_path_yaw_deg(x0: float, y0: float, x1: float, y1: float) -> float:
    return -math.degrees(math.atan2(y1 - y0, -(x1 - x0)))


def parse_point_type(token: str) -> int:
    token = token.strip()
    if token in POINT_TYPE_TOKENS:
        return POINT_TYPE_TOKENS[token]
    return int(float(token))


def _unit_vec(dx: float, dy: float, fallback: Tuple[float, float] = (1.0, 0.0)) -> Tuple[float, float]:
    length = math.hypot(dx, dy)
    if length < 1.0e-9:
        return fallback
    return dx / length, dy / length


def _point_xy(point: RoutePoint) -> Tuple[float, float]:
    return float(point.x), float(point.y)


def tangent_yaws(x_vals: np.ndarray, y_vals: np.ndarray) -> np.ndarray:
    yaws = np.zeros(len(x_vals), dtype=float)
    for i in range(len(x_vals)):
        if i + 1 < len(x_vals):
            x0, y0, x1, y1 = x_vals[i], y_vals[i], x_vals[i + 1], y_vals[i + 1]
        elif i > 0:
            x0, y0, x1, y1 = x_vals[i - 1], y_vals[i - 1], x_vals[i], y_vals[i]
        else:
            x0 = x1 = x_vals[i]
            y0 = y1 = y_vals[i]
        yaws[i] = normalize_relative_yaw_deg(calc_path_yaw_deg(x0, y0, x1, y1))
    return yaws


def infer_missing_angles(points: List[RoutePoint]) -> None:
    for idx, point in enumerate(points):
        if point.target_yaw_deg is None:
            yaw = None
            if idx + 1 < len(points):
                nxt = points[idx + 1]
                if not math.isclose(point.x, nxt.x) or not math.isclose(point.y, nxt.y):
                    yaw = calc_path_yaw_deg(point.x, point.y, nxt.x, nxt.y)
            if yaw is None and idx > 0:
                prev = points[idx - 1]
                if not math.isclose(prev.x, point.x) or not math.isclose(prev.y, point.y):
                    yaw = calc_path_yaw_deg(prev.x, prev.y, point.x, point.y)
            point.target_yaw_deg = normalize_relative_yaw_deg(0.0 if yaw is None else yaw)
        if point.heading_deg is None:
            point.heading_deg = 0.0


def read_csv_points(csv_path: str) -> Tuple[List[RoutePoint], int, float]:
    if not os.path.exists(csv_path):
        raise FileNotFoundError(csv_path)

    points: List[RoutePoint] = []
    start_heading_deg = 0.0
    start_heading_valid = 0

    with open(csv_path, "r", encoding="utf-8-sig", newline="") as f:
        reader = csv_mod.DictReader(f)
        if reader.fieldnames is None:
            raise ValueError("CSV is missing a header row.")

        key_map: Dict[str, str] = {}
        for key in reader.fieldnames:
            if key is not None:
                key_map[key.strip().lower().replace(" ", "")] = key

        for required in ("x", "y"):
            if required not in key_map:
                raise ValueError(f"CSV is missing required column: {required}")

        point_type_col = key_map.get("point_type")
        yaw_col = key_map.get("relative_yaw") or key_map.get("target_yaw_deg")
        heading_col = key_map.get("heading")
        start_heading_col = key_map.get("start_heading")

        for row in reader:
            x = float(row[key_map["x"]])
            y = float(row[key_map["y"]])

            point_type = 0
            if point_type_col and row.get(point_type_col, "").strip():
                point_type = max(0, min(5, int(float(row[point_type_col]))))

            target_yaw = None
            if yaw_col and row.get(yaw_col, "").strip():
                target_yaw = normalize_relative_yaw_deg(float(row[yaw_col]))

            heading = None
            if heading_col and row.get(heading_col, "").strip():
                heading = normalize_heading_deg(float(row[heading_col]))

            if start_heading_col and row.get(start_heading_col, "").strip():
                start_heading_deg = float(row[start_heading_col])

            points.append(
                RoutePoint(
                    x=x,
                    y=y,
                    target_yaw_deg=target_yaw,
                    heading_deg=heading,
                    target_speed=0.0,
                    point_type=point_type,
                )
            )

    infer_missing_angles(points)
    return points, start_heading_valid, start_heading_deg


def read_route_header(file_path: str) -> Tuple[List[RoutePoint], int, float]:
    if not os.path.exists(file_path):
        raise FileNotFoundError(file_path)

    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        text = f.read()

    start_heading_valid = 0
    match_valid = re.search(r"#define\s+NAV_REPLAY_START_HEADING_VALID\s+(\d+)", text)
    if match_valid:
        start_heading_valid = int(match_valid.group(1))

    start_heading_deg = 0.0
    match_heading = re.search(r"#define\s+NAV_REPLAY_START_HEADING_DEG\s+([+-]?[\d\.]+)f", text)
    if match_heading:
        start_heading_deg = float(match_heading.group(1))

    body_match = re.search(
        r"static const NavRamPoint_t nav_replay_static_route_points\[.*?\]\s*=\s*\{(.*?)\};",
        text,
        re.DOTALL,
    )
    if not body_match:
        return [], start_heading_valid, start_heading_deg

    body = body_match.group(1)
    points: List[RoutePoint] = []
    pattern_v7 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*"
        rf"({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*,\s*"
        rf"({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*\}}"
    )
    pattern_v6 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*"
        rf"({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*,\s*"
        rf"({FLOAT_RE})f\s*\}}"
    )

    for match in pattern_v7.finditer(body):
        points.append(
            RoutePoint(
                x=float(match.group(1)),
                y=float(match.group(2)),
                target_yaw_deg=normalize_relative_yaw_deg(float(match.group(3))),
                heading_deg=normalize_heading_deg(float(match.group(4))),
                point_type=parse_point_type(match.group(5)),
                target_speed=float(match.group(6)),
                curvature=float(match.group(7)),
            )
        )

    if not points:
        for match in pattern_v6.finditer(body):
            points.append(
                RoutePoint(
                    x=float(match.group(1)),
                    y=float(match.group(2)),
                    target_yaw_deg=normalize_relative_yaw_deg(float(match.group(3))),
                    heading_deg=normalize_heading_deg(float(match.group(4))),
                    point_type=parse_point_type(match.group(5)),
                    target_speed=float(match.group(6)),
                )
            )

    infer_missing_angles(points)
    return points, start_heading_valid, start_heading_deg


def _course_axis_from_markers_and_cones(
    uturn_marker: RoutePoint,
    cones: Sequence[RoutePoint],
    finish_marker: RoutePoint,
) -> Tuple[Tuple[float, float], Tuple[float, float]]:
    marker_dx = finish_marker.x - uturn_marker.x
    axis_x = 1.0 if marker_dx >= 0.0 else -1.0
    return (axis_x, 0.0), (0.0, axis_x)


def _to_sl(
    point_xy: Tuple[float, float],
    origin_xy: Tuple[float, float],
    axis: Tuple[float, float],
    lateral: Tuple[float, float],
) -> Tuple[float, float]:
    dx = point_xy[0] - origin_xy[0]
    dy = point_xy[1] - origin_xy[1]
    return dx * axis[0] + dy * axis[1], dx * lateral[0] + dy * lateral[1]


def _from_sl(
    s_val: float,
    l_val: float,
    origin_xy: Tuple[float, float],
    axis: Tuple[float, float],
    lateral: Tuple[float, float],
) -> Tuple[float, float]:
    return (
        origin_xy[0] + axis[0] * s_val + lateral[0] * l_val,
        origin_xy[1] + axis[1] * s_val + lateral[1] * l_val,
    )


def _unique_sorted(values: Iterable[float], min_gap: float = 1.0) -> List[float]:
    out: List[float] = []
    for value in sorted(float(v) for v in values):
        if not out or abs(value - out[-1]) >= min_gap:
            out.append(value)
    return out


def _line_lateral_candidates(key_values: Sequence[float], count: int = LINE_SAMPLE_COUNT) -> List[float]:
    if not key_values:
        key_values = [0.0]
    low = min(key_values) - MIN_CONE_CLEARANCE_MM * 1.6
    high = max(key_values) + MIN_CONE_CLEARANCE_MM * 1.6
    if high - low < 1200.0:
        mid = (low + high) * 0.5
        low = mid - 600.0
        high = mid + 600.0

    values = list(np.linspace(low, high, count))
    values.extend(key_values)
    return _unique_sorted(values, min_gap=25.0)


def _terminal_lateral_candidates(key_values: Sequence[float], count: int = LINE_SAMPLE_COUNT) -> List[float]:
    if not key_values:
        key_values = [0.0]
    low = min(key_values) - MIN_CONE_CLEARANCE_MM * 0.7
    high = max(key_values) + MIN_CONE_CLEARANCE_MM * 0.7
    if high - low < 1000.0:
        mid = (low + high) * 0.5
        low = mid - 500.0
        high = mid + 500.0

    values = list(np.linspace(low, high, count))
    values.extend(key_values)
    return _unique_sorted(values, min_gap=25.0)


def _dedupe_candidate_points(candidates: Sequence[CandidatePoint]) -> List[CandidatePoint]:
    unique: Dict[Tuple[int, int, str], CandidatePoint] = {}
    for cand in candidates:
        unique[(round(cand.x), round(cand.y), cand.label)] = cand
    return list(unique.values())


def _same_xy(a: RoutePoint, b: RoutePoint, tol_mm: float = 1.0e-3) -> bool:
    return math.hypot(a.x - b.x, a.y - b.y) <= tol_mm


def _signed_side(value: float, fallback: float = 1.0) -> float:
    if abs(value) < 1.0e-6:
        return 1.0 if fallback >= 0.0 else -1.0
    return 1.0 if value >= 0.0 else -1.0


def _pseudo_uturn_approach_candidates(
    start_s: float,
    start_l: float,
    origin_xy: Tuple[float, float],
    axis: Tuple[float, float],
    lateral: Tuple[float, float],
) -> List[CandidatePoint]:
    start_side = _signed_side(start_s)
    max_distance = max(MIN_CONE_CLEARANCE_MM + 250.0, abs(start_s) - MIN_CONE_CLEARANCE_MM)
    distances = [d for d in UTURN_APPROACH_DISTANCES_MM if d < max_distance]
    if not distances:
        distances = [max(MIN_CONE_CLEARANCE_MM + 250.0, abs(start_s) * 0.45)]

    candidates: List[CandidatePoint] = []
    for dist_s in distances:
        s_val = start_side * dist_s
        for offset_l in UTURN_APPROACH_LATERAL_OFFSETS_MM:
            l_val = start_l + offset_l
            if math.hypot(s_val, l_val) < MIN_CONE_CLEARANCE_MM + 120.0:
                continue
            x, y = _from_sl(s_val, l_val, origin_xy, axis, lateral)
            candidates.append(CandidatePoint(float(x), float(y), "uturn_approach"))

    if not candidates:
        raise PathConstraintError("cannot build approach candidates before pseudo u-turn cone")
    return _dedupe_candidate_points(candidates)


def _pseudo_uturn_overline_candidates(
    start_s: float,
    lateral_reference_l: float,
    origin_xy: Tuple[float, float],
    axis: Tuple[float, float],
    lateral: Tuple[float, float],
    label: str = "uturn_overline",
) -> List[CandidatePoint]:
    start_side = _signed_side(start_s)
    beyond_side = -start_side
    wrap_side = _signed_side(lateral_reference_l, fallback=1.0)
    candidates: List[CandidatePoint] = []

    for radius_offset in UTURN_GUARD_RADIUS_OFFSETS_MM:
        radius = MIN_CONE_CLEARANCE_MM + radius_offset
        for over_s in UTURN_OVERLINE_DISTANCES_MM:
            if over_s >= radius - 20.0:
                continue
            l_abs = math.sqrt(max(0.0, radius * radius - over_s * over_s))
            s_val = beyond_side * over_s
            l_val = wrap_side * l_abs
            x, y = _from_sl(s_val, l_val, origin_xy, axis, lateral)
            candidates.append(CandidatePoint(float(x), float(y), label))

    if not candidates:
        raise PathConstraintError("cannot build over-line candidates around pseudo u-turn cone")
    return _dedupe_candidate_points(candidates)


def _gap_candidates(
    cone_a: RoutePoint,
    cone_b: RoutePoint,
    gap_index: int,
) -> List[CandidatePoint]:
    ax, ay = cone_a.x, cone_a.y
    bx, by = cone_b.x, cone_b.y
    dist = math.hypot(bx - ax, by - ay)
    if dist <= 2.0 * MIN_CONE_CLEARANCE_MM:
        raise PathConstraintError(
            f"cone gap {gap_index + 1} is too narrow: {dist:.1f}mm, "
            f"requires > {2.0 * MIN_CONE_CLEARANCE_MM:.1f}mm"
        )

    hard_min_t = MIN_CONE_CLEARANCE_MM / dist
    hard_max_t = 1.0 - hard_min_t
    soft_clearance = min(
        MIN_CONE_CLEARANCE_MM + CLEARANCE_SOFT_MARGIN_MM,
        dist * 0.5 - 5.0,
    )
    soft_min_t = soft_clearance / dist
    soft_max_t = 1.0 - soft_min_t

    if soft_min_t <= soft_max_t:
        low_t, high_t = soft_min_t, soft_max_t
    else:
        low_t, high_t = hard_min_t, hard_max_t

    t_values = np.linspace(low_t, high_t, GAP_SAMPLE_COUNT)
    t_values = np.concatenate(
        [
            t_values,
            np.array([0.5, hard_min_t, hard_max_t], dtype=float),
        ]
    )

    candidates: List[CandidatePoint] = []
    for t in _unique_sorted(np.clip(t_values, hard_min_t, hard_max_t), min_gap=0.01):
        x = ax + (bx - ax) * t
        y = ay + (by - ay) * t
        candidates.append(CandidatePoint(float(x), float(y), f"gap{gap_index + 1}"))
    return candidates


def _sprint_entry_candidates(
    last_cone: RoutePoint,
    finish_s: float,
    finish_l: float,
    origin_xy: Tuple[float, float],
    axis: Tuple[float, float],
    lateral: Tuple[float, float],
) -> Tuple[List[CandidatePoint], List[float]]:
    last_s, last_l = _to_sl(_point_xy(last_cone), origin_xy, axis, lateral)
    finish_target_s = finish_s + abs(FINISH_OVER_LINE_MM)
    max_entry_s = min(finish_s - SPRINT_ENTRY_MIN_LINE_APPROACH_MM, finish_target_s - 150.0)
    min_entry_s = last_s + MIN_CONE_CLEARANCE_MM + 80.0
    if max_entry_s <= min_entry_s:
        max_entry_s = min_entry_s + 1.0

    s_candidates: List[float] = []
    for distance in SPRINT_ENTRY_AFTER_LAST_CONE_MM:
        s_candidates.append(last_s + distance)

    usable_span = max(0.0, finish_s - last_s)
    for frac in SPRINT_ENTRY_DISTANCE_FRACS:
        s_candidates.append(last_s + usable_span * frac)

    s_values = _unique_sorted(
        [min(max(s_val, min_entry_s), max_entry_s) for s_val in s_candidates],
        min_gap=80.0,
    )
    if len(s_values) < 3 and max_entry_s - min_entry_s > 240.0:
        s_values = _unique_sorted(
            list(s_values) + list(np.linspace(min_entry_s, max_entry_s, 3)),
            min_gap=80.0,
        )

    lateral_keys: List[float] = [last_l, finish_l]
    for base_l in (last_l, finish_l):
        for offset in SPRINT_ENTRY_LATERAL_OFFSETS_MM:
            lateral_keys.append(base_l + offset)
            if offset > 0.0:
                lateral_keys.append(base_l - offset)

    l_values = _terminal_lateral_candidates(lateral_keys, count=LINE_SAMPLE_COUNT)

    candidates: List[CandidatePoint] = []
    for s_val in s_values:
        label = f"sprint_entry_{int(round(s_val - last_s))}"
        for l_val in l_values:
            x, y = _from_sl(s_val, l_val, origin_xy, axis, lateral)
            candidates.append(CandidatePoint(float(x), float(y), label))

    if not candidates:
        raise PathConstraintError("cannot build finish sprint entry candidates")
    return _dedupe_candidate_points(candidates), l_values


def _rotate_vec(x_val: float, y_val: float, angle_deg: float) -> Tuple[float, float]:
    rad = math.radians(angle_deg)
    c = math.cos(rad)
    s = math.sin(rad)
    return x_val * c - y_val * s, x_val * s + y_val * c


def _cone_guard_candidates(
    prev_cone: RoutePoint,
    cone: RoutePoint,
    next_cone: RoutePoint,
    guard_index: int,
    radius_offsets_mm: Sequence[float] = GUARD_RADIUS_OFFSETS_MM,
    angle_offsets_deg: Sequence[float] = GUARD_ANGLE_OFFSETS_DEG,
) -> List[CandidatePoint]:
    tx, ty = _unit_vec(next_cone.x - prev_cone.x, next_cone.y - prev_cone.y)
    nx, ny = -ty, tx
    candidates: List[CandidatePoint] = []

    for side in (-1.0, 1.0):
        base_x = nx * side
        base_y = ny * side
        for radius_offset in radius_offsets_mm:
            radius = MIN_CONE_CLEARANCE_MM + radius_offset
            for angle_deg in angle_offsets_deg:
                vx, vy = _rotate_vec(base_x, base_y, angle_deg)
                x = cone.x + vx * radius
                y = cone.y + vy * radius
                if (
                    math.hypot(x - prev_cone.x, y - prev_cone.y) < MIN_CONE_CLEARANCE_MM
                    or math.hypot(x - cone.x, y - cone.y) < MIN_CONE_CLEARANCE_MM
                    or math.hypot(x - next_cone.x, y - next_cone.y) < MIN_CONE_CLEARANCE_MM
                ):
                    continue
                candidates.append(CandidatePoint(float(x), float(y), f"guard{guard_index + 1}"))

    if not candidates:
        raise PathConstraintError(f"cannot build guard candidates around cone {guard_index + 1}")

    return _dedupe_candidate_points(candidates)


def _segment_min_clearance(
    a: Tuple[float, float],
    b: Tuple[float, float],
    cones: Sequence[RoutePoint],
) -> float:
    if not cones:
        return float("inf")
    length = math.hypot(b[0] - a[0], b[1] - a[1])
    steps = max(1, int(math.ceil(length / SEGMENT_CHECK_STEP_MM)))
    min_clearance = float("inf")
    for i in range(steps + 1):
        t = i / steps
        x = a[0] + (b[0] - a[0]) * t
        y = a[1] + (b[1] - a[1]) * t
        for cone in cones:
            min_clearance = min(min_clearance, math.hypot(x - cone.x, y - cone.y))
    return min_clearance


def _turn_angle_rad(
    a: Tuple[float, float],
    b: Tuple[float, float],
    c: Tuple[float, float],
) -> float:
    ux, uy = b[0] - a[0], b[1] - a[1]
    vx, vy = c[0] - b[0], c[1] - b[1]
    ul = math.hypot(ux, uy)
    vl = math.hypot(vx, vy)
    if ul < 1e-6 or vl < 1e-6:
        return 0.0
    dot = max(-1.0, min(1.0, (ux * vx + uy * vy) / (ul * vl)))
    return math.acos(dot)


def _clearance_cost(clearance_mm: float) -> float:
    if clearance_mm < MIN_CONE_CLEARANCE_MM:
        return float("inf")
    margin = clearance_mm - MIN_CONE_CLEARANCE_MM
    return CLEARANCE_COST_WEIGHT / ((margin + 30.0) ** 1.3)


def _is_sprint_entry_stage(stage: Sequence[CandidatePoint]) -> bool:
    return bool(stage) and all(cand.label.startswith("sprint_entry") for cand in stage)


def _select_diverse_search_states(
    states: Sequence[Tuple[float, List[Tuple[float, float]], List[str]]],
    limit: int,
    group_key: Callable[[Tuple[float, List[Tuple[float, float]], List[str]]], str],
) -> List[Tuple[float, List[Tuple[float, float]], List[str]]]:
    if len(states) <= limit:
        return list(states)

    groups = {group_key(state) for state in states}
    quota = max(2, limit // max(1, len(groups)))
    counts: Dict[str, int] = {}
    selected: List[Tuple[float, List[Tuple[float, float]], List[str]]] = []

    for state in states:
        key = group_key(state)
        if counts.get(key, 0) >= quota:
            continue
        selected.append(state)
        counts[key] = counts.get(key, 0) + 1
        if len(selected) >= limit:
            return selected

    if len(selected) < limit:
        used = {id(state) for state in selected}
        for state in states:
            if id(state) in used:
                continue
            selected.append(state)
            if len(selected) >= limit:
                break

    return selected


def _beam_search_routes(
    start_xy: Tuple[float, float],
    stages: Sequence[Sequence[CandidatePoint]],
    cones: Sequence[RoutePoint],
) -> List[Tuple[float, List[Tuple[float, float]]]]:
    routes: List[Tuple[float, List[Tuple[float, float]], List[str]]] = [(0.0, [start_xy], ["start"])]

    for stage_idx, stage in enumerate(stages):
        next_stage = stages[stage_idx + 1] if stage_idx + 1 < len(stages) else None
        expanded: List[Tuple[float, List[Tuple[float, float]], List[str]]] = []
        for current_cost, route, labels in routes:
            prev_xy = route[-1]
            prev_prev_xy = route[-2] if len(route) >= 2 else None
            for cand in stage:
                cand_xy = (cand.x, cand.y)
                clearance = _segment_min_clearance(prev_xy, cand_xy, cones)
                if clearance < MIN_CONE_CLEARANCE_MM:
                    continue
                if next_stage is not None and not any(
                    _segment_min_clearance(cand_xy, (next_cand.x, next_cand.y), cones) >= MIN_CONE_CLEARANCE_MM
                    for next_cand in next_stage
                ):
                    continue
                length_cost = math.hypot(cand.x - prev_xy[0], cand.y - prev_xy[1])
                turn_cost = 0.0
                if prev_prev_xy is not None:
                    angle = _turn_angle_rad(prev_prev_xy, prev_xy, cand_xy)
                    turn_cost = TURN_COST_WEIGHT * angle * angle
                total = current_cost + length_cost + turn_cost + _clearance_cost(clearance)
                expanded.append((total, route + [cand_xy], labels + [cand.label]))

        if not expanded:
            raise PathConstraintError("no candidate route can satisfy segment clearance")
        expanded.sort(key=lambda item: item[0])
        if _is_sprint_entry_stage(stage):
            routes = _select_diverse_search_states(expanded, BEAM_WIDTH, lambda item: item[2][-1])
        elif (
            stage
            and all(cand.label == "finish" for cand in stage)
            and stage_idx > 0
            and _is_sprint_entry_stage(stages[stage_idx - 1])
        ):
            routes = _select_diverse_search_states(expanded, BEAM_WIDTH, lambda item: item[2][-2])
        else:
            routes = expanded[:BEAM_WIDTH]

    return [(cost, route) for cost, route, _labels in routes[:MAX_FINAL_CANDIDATES]]


def _sample_hermite_path(
    waypoints: Sequence[Tuple[float, float]],
    tension: float,
    sample_step_mm: float,
) -> Tuple[np.ndarray, np.ndarray]:
    if len(waypoints) < 2:
        raise PathConstraintError("not enough waypoints")

    cleaned: List[Tuple[float, float]] = []
    for point in waypoints:
        candidate = (float(point[0]), float(point[1]))
        if not cleaned or math.hypot(candidate[0] - cleaned[-1][0], candidate[1] - cleaned[-1][1]) > 1.0e-6:
            cleaned.append(candidate)

    if len(cleaned) < 2:
        raise PathConstraintError("not enough unique waypoints")
    if len(cleaned) == 2:
        p0, p1 = cleaned
        chord = math.hypot(p1[0] - p0[0], p1[1] - p0[1])
        steps = max(int(math.ceil(chord / max(sample_step_mm, 1.0))), 4)
        xs = np.array([p0[0] + (p1[0] - p0[0]) * (step / steps) for step in range(steps + 1)], dtype=float)
        ys = np.array([p0[1] + (p1[1] - p0[1]) * (step / steps) for step in range(steps + 1)], dtype=float)
        return xs, ys

    try:
        pts = np.array(cleaned, dtype=float)
        seg = np.sqrt(np.diff(pts[:, 0]) ** 2 + np.diff(pts[:, 1]) ** 2)
        chord_s = np.insert(np.cumsum(seg), 0, 0.0)
        total = float(chord_s[-1])
        if total <= 1.0e-6:
            raise PathConstraintError("degenerate waypoint chain")

        u_vals = chord_s / total
        k = min(3, len(cleaned) - 1)
        with warnings.catch_warnings():
            warnings.simplefilter("error", RuntimeWarning)
            tck, _ = splprep([pts[:, 0], pts[:, 1]], u=u_vals, s=max(0.0, float(tension)), k=k)
        dense_count = max(
            900,
            len(cleaned) * 80,
            int(math.ceil(total / max(sample_step_mm * 4.0, 1.0))),
        )
        sample_u = np.linspace(0.0, 1.0, dense_count)
        xs, ys = splev(sample_u, tck)
        return np.array(xs, dtype=float), np.array(ys, dtype=float)
    except Exception:
        tangents: List[Tuple[float, float]] = []
        for i, point in enumerate(cleaned):
            if i == 0:
                dx = cleaned[1][0] - point[0]
                dy = cleaned[1][1] - point[1]
            elif i == len(cleaned) - 1:
                dx = point[0] - cleaned[i - 1][0]
                dy = point[1] - cleaned[i - 1][1]
            else:
                dx = cleaned[i + 1][0] - cleaned[i - 1][0]
                dy = cleaned[i + 1][1] - cleaned[i - 1][1]
            tangents.append((dx * tension, dy * tension))

        xs: List[float] = []
        ys: List[float] = []
        for i in range(len(cleaned) - 1):
            p0 = cleaned[i]
            p1 = cleaned[i + 1]
            m0 = tangents[i]
            m1 = tangents[i + 1]
            chord = math.hypot(p1[0] - p0[0], p1[1] - p0[1])
            steps = max(int(math.ceil(chord / max(sample_step_mm, 1.0))), 4)

            for step in range(steps + 1):
                if i > 0 and step == 0:
                    continue
                t = step / steps
                t2 = t * t
                t3 = t2 * t
                h00 = 2.0 * t3 - 3.0 * t2 + 1.0
                h10 = t3 - 2.0 * t2 + t
                h01 = -2.0 * t3 + 3.0 * t2
                h11 = t3 - t2
                xs.append(h00 * p0[0] + h10 * m0[0] + h01 * p1[0] + h11 * m1[0])
                ys.append(h00 * p0[1] + h10 * m0[1] + h01 * p1[1] + h11 * m1[1])

        return np.array(xs, dtype=float), np.array(ys, dtype=float)


def _add_terminal_anchor_points(route: Sequence[Tuple[float, float]]) -> List[Tuple[float, float]]:
    if len(route) < 2:
        return list(route)

    anchored: List[Tuple[float, float]] = []
    for i in range(len(route) - 1):
        ax, ay = route[i]
        bx, by = route[i + 1]
        anchored.append((float(ax), float(ay)))

        fractions: Tuple[float, ...] = ()
        if i == 0:
            fractions = START_APPROACH_ANCHOR_FRACS
        elif i == len(route) - 2:
            fractions = FINISH_EXIT_ANCHOR_FRACS

        for frac in fractions:
            x = ax + (bx - ax) * frac
            y = ay + (by - ay) * frac
            anchored.append((float(x), float(y)))

    anchored.append((float(route[-1][0]), float(route[-1][1])))
    return anchored


def resample_path(
    x_fine: np.ndarray,
    y_fine: np.ndarray,
    target_dist: float,
) -> Tuple[np.ndarray, np.ndarray]:
    if len(x_fine) < 2:
        return x_fine, y_fine

    dx = np.diff(x_fine)
    dy = np.diff(y_fine)
    seg = np.sqrt(dx * dx + dy * dy)
    s = np.insert(np.cumsum(seg), 0, 0.0)
    total = float(s[-1])
    if total <= 1e-6:
        return x_fine[:1], y_fine[:1]

    sample_s = np.arange(0.0, total, target_dist)
    if len(sample_s) == 0 or not math.isclose(float(sample_s[-1]), total):
        sample_s = np.append(sample_s, total)
    return np.interp(sample_s, s, x_fine), np.interp(sample_s, s, y_fine)


def _route_points_from_xy(x_vals: np.ndarray, y_vals: np.ndarray) -> List[RoutePoint]:
    yaws = tangent_yaws(x_vals, y_vals)
    return [
        RoutePoint(
            x=float(x_vals[i]),
            y=float(y_vals[i]),
            target_yaw_deg=normalize_relative_yaw_deg(float(yaws[i])),
            heading_deg=0.0,
            target_speed=0.0,
            point_type=0,
        )
        for i in range(len(x_vals))
    ]


def cumulative_arc_length(points: Sequence[RoutePoint]) -> np.ndarray:
    if not points:
        return np.array([], dtype=float)
    xs = np.array([p.x for p in points], dtype=float)
    ys = np.array([p.y for p in points], dtype=float)
    diffs = np.sqrt(np.diff(xs) ** 2 + np.diff(ys) ** 2)
    return np.insert(np.cumsum(diffs), 0, 0.0)


def signed_curvature(points: Sequence[RoutePoint]) -> np.ndarray:
    count = len(points)
    curvature = np.zeros(count, dtype=float)
    if count < 3:
        return curvature

    xs = np.array([p.x for p in points], dtype=float)
    ys = np.array([p.y for p in points], dtype=float)
    for i in range(1, count - 1):
        ax = xs[i] - xs[i - 1]
        ay = ys[i] - ys[i - 1]
        bx = xs[i + 1] - xs[i]
        by = ys[i + 1] - ys[i]
        a = math.hypot(ax, ay)
        b = math.hypot(bx, by)
        c = math.hypot(xs[i + 1] - xs[i - 1], ys[i + 1] - ys[i - 1])
        denom = a * b * c
        if denom <= CURVATURE_EPS:
            continue
        cross = ax * by - ay * bx
        curvature[i] = 2.0 * cross / denom

    curvature[0] = curvature[1]
    curvature[-1] = curvature[-2]
    return curvature


def curvature_rate_per_mm2(s_vals: np.ndarray, curvature: np.ndarray) -> np.ndarray:
    rate = np.zeros(len(curvature), dtype=float)
    if len(curvature) < 2:
        return rate
    ds = np.diff(s_vals)
    diff = np.diff(curvature)
    valid = ds > 1e-6
    rate[1:][valid] = np.abs(diff[valid] / ds[valid])
    rate[0] = rate[1]
    return rate


def minimum_path_cone_clearance(points: Sequence[RoutePoint], cones: Sequence[RoutePoint]) -> float:
    if not cones:
        return float("inf")
    min_clearance = float("inf")
    for point in points:
        for cone in cones:
            min_clearance = min(
                min_clearance,
                math.hypot(point.x - cone.x, point.y - cone.y),
            )
    return min_clearance


def validate_final_path(points: Sequence[RoutePoint], cones: Sequence[RoutePoint]) -> float:
    clearance = minimum_path_cone_clearance(points, cones)
    if clearance < MIN_CONE_CLEARANCE_MM - 1.0e-6:
        raise PathConstraintError(
            f"path clearance {clearance:.1f}mm is below required {MIN_CONE_CLEARANCE_MM:.1f}mm"
        )
    return clearance


def validate_uturn_line_crossing(
    points: Sequence[RoutePoint],
    uturn_marker: RoutePoint,
    real_cones: Sequence[RoutePoint],
    finish_marker: RoutePoint,
) -> None:
    if not points:
        raise PathConstraintError("path is empty")

    axis, lateral = _course_axis_from_markers_and_cones(uturn_marker, real_cones, finish_marker)
    origin_xy = _point_xy(uturn_marker)
    finish_s, _finish_l = _to_sl(_point_xy(finish_marker), origin_xy, axis, lateral)
    if finish_s <= 0.0:
        axis = (-axis[0], -axis[1])
        lateral = (-lateral[0], -lateral[1])

    start_s, _start_l = _to_sl((0.0, 0.0), origin_xy, axis, lateral)
    start_side = _signed_side(start_s)
    signed_progress = [
        start_side * _to_sl(_point_xy(point), origin_xy, axis, lateral)[0]
        for point in points
    ]
    beyond_indices = [
        idx
        for idx, progress in enumerate(signed_progress)
        if progress < -UTURN_OVER_LINE_MM
    ]
    if not beyond_indices:
        raise PathConstraintError("path never crosses beyond the pseudo u-turn line")

    first_beyond_idx = beyond_indices[0]
    if not any(progress > UTURN_OVER_LINE_MM for progress in signed_progress[first_beyond_idx + 1 :]):
        raise PathConstraintError("path crosses the u-turn line but never returns to the slalom side")


def _score_final_path(
    points: Sequence[RoutePoint],
    cones: Sequence[RoutePoint],
    route_cost: float,
    sprint_entry_xy: Optional[Tuple[float, float]] = None,
) -> Tuple[float, float]:
    s_vals = cumulative_arc_length(points)
    curvature = signed_curvature(points)
    rate = curvature_rate_per_mm2(s_vals, curvature)
    clearance = validate_final_path(points, cones)
    length = float(s_vals[-1]) if len(s_vals) else 0.0
    max_abs_k = float(np.max(np.abs(curvature))) if len(curvature) else 0.0
    max_abs_rate = float(np.max(np.abs(rate))) if len(rate) else 0.0
    clearance_margin = max(0.0, clearance - MIN_CONE_CLEARANCE_MM)
    score = (
        length
        + route_cost * 0.08
        + 5.0e6 * max_abs_k
        + 4.0e9 * max_abs_rate
        + 40000.0 / (clearance_margin + 80.0)
    )

    if sprint_entry_xy is not None and len(points) >= 4:
        entry_idx = min(
            range(len(points)),
            key=lambda i: math.hypot(points[i].x - sprint_entry_xy[0], points[i].y - sprint_entry_xy[1]),
        )
        if entry_idx < len(points) - 3:
            tail_length = float(s_vals[-1] - s_vals[entry_idx])
            tail_chord = math.hypot(points[-1].x - points[entry_idx].x, points[-1].y - points[entry_idx].y)
            tail_excess = max(0.0, tail_length - tail_chord)
            tail_k = float(np.max(np.abs(curvature[entry_idx:]))) if len(curvature) else 0.0
            tail_rate = float(np.max(np.abs(rate[entry_idx:]))) if len(rate) else 0.0
            score += (
                SPRINT_TAIL_EXCESS_WEIGHT * tail_excess
                + SPRINT_TAIL_CHORD_WEIGHT * tail_chord
                + SPRINT_TAIL_CURVATURE_WEIGHT * tail_k
                + SPRINT_TAIL_CURVATURE_RATE_WEIGHT * tail_rate
            )
    return score, clearance


def build_candidate_stages(
    uturn_marker: RoutePoint,
    cones: Sequence[RoutePoint],
    finish_marker: RoutePoint,
) -> Tuple[List[List[CandidatePoint]], List[RoutePoint], Tuple[float, float]]:
    obstacle_cones = list(cones)
    real_cones = [cone for cone in obstacle_cones if not _same_xy(cone, uturn_marker)]
    if len(real_cones) < 2:
        raise PathConstraintError("needs at least two real cones after the pseudo u-turn cone")

    axis, lateral = _course_axis_from_markers_and_cones(uturn_marker, real_cones, finish_marker)
    origin_xy = _point_xy(uturn_marker)

    cone_with_s = []
    for cone in real_cones:
        s_val, l_val = _to_sl(_point_xy(cone), origin_xy, axis, lateral)
        cone_with_s.append((s_val, l_val, cone))
    cone_with_s.sort(key=lambda item: item[0])
    sorted_real_cones = [item[2] for item in cone_with_s]

    finish_s, finish_l = _to_sl(_point_xy(finish_marker), origin_xy, axis, lateral)
    if finish_s <= 0.0:
        axis = (-axis[0], -axis[1])
        lateral = (-lateral[0], -lateral[1])
        cone_with_s = []
        for cone in real_cones:
            s_val, l_val = _to_sl(_point_xy(cone), origin_xy, axis, lateral)
            cone_with_s.append((s_val, l_val, cone))
        cone_with_s.sort(key=lambda item: item[0])
        sorted_real_cones = [item[2] for item in cone_with_s]
        finish_s, finish_l = _to_sl(_point_xy(finish_marker), origin_xy, axis, lateral)

    start_xy = (0.0, 0.0)
    start_s, start_l = _to_sl(start_xy, origin_xy, axis, lateral)
    _first_cone_s, first_cone_l = _to_sl(_point_xy(sorted_real_cones[0]), origin_xy, axis, lateral)
    entry_l = start_l
    if _signed_side(entry_l, fallback=-first_cone_l) == _signed_side(first_cone_l, fallback=1.0):
        entry_l = -first_cone_l

    traversal_stages: List[List[CandidatePoint]] = [
        _pseudo_uturn_approach_candidates(start_s, start_l, origin_xy, axis, lateral),
        _pseudo_uturn_overline_candidates(
            start_s,
            entry_l,
            origin_xy,
            axis,
            lateral,
            label="uturn_entry",
        ),
        _pseudo_uturn_overline_candidates(
            start_s,
            first_cone_l,
            origin_xy,
            axis,
            lateral,
            label="uturn_exit",
        ),
    ]

    if len(sorted_real_cones) >= 2:
        traversal_stages.append(
            _cone_guard_candidates(
                uturn_marker,
                sorted_real_cones[0],
                sorted_real_cones[1],
                1,
                radius_offsets_mm=FIRST_GUARD_RADIUS_OFFSETS_MM,
                angle_offsets_deg=FIRST_GUARD_ANGLE_OFFSETS_DEG,
            )
        )

    for i in range(max(0, len(sorted_real_cones) - 1)):
        traversal_stages.append(_gap_candidates(sorted_real_cones[i], sorted_real_cones[i + 1], i))
        if i + 2 < len(sorted_real_cones):
            traversal_stages.append(
                _cone_guard_candidates(
                    sorted_real_cones[i],
                    sorted_real_cones[i + 1],
                    sorted_real_cones[i + 2],
                    i + 2,
                )
            )

    gap_l_values: List[float] = []
    for stage in traversal_stages:
        for cand in stage:
            _s, l_val = _to_sl((cand.x, cand.y), origin_xy, axis, lateral)
            gap_l_values.append(l_val)

    sprint_entry_stage, sprint_entry_l_values = _sprint_entry_candidates(
        sorted_real_cones[-1],
        finish_s,
        finish_l,
        origin_xy,
        axis,
        lateral,
    )

    finish_target_s = finish_s + abs(FINISH_OVER_LINE_MM)
    terminal_lateral_keys = sprint_entry_l_values + [finish_l]
    finish_l_candidates = _terminal_lateral_candidates(terminal_lateral_keys)

    finish_stage = []
    for l_val in finish_l_candidates:
        x, y = _from_sl(finish_target_s, l_val, origin_xy, axis, lateral)
        finish_stage.append(CandidatePoint(x, y, "finish"))

    return traversal_stages + [sprint_entry_stage, finish_stage], obstacle_cones, axis


def generate_optimized_path(raw_points: Sequence[RoutePoint]) -> OptimizedPathResult:
    if len(raw_points) < 4:
        raise ValueError("plan1 optimized planner needs at least: pseudo u-turn cone, two cones, finish marker")

    uturn_marker = raw_points[0]
    cones = list(raw_points[:-1])
    real_cones = list(raw_points[1:-1])
    finish_marker = raw_points[-1]
    start_xy = (0.0, 0.0)

    stages, sorted_cones, _axis = build_candidate_stages(uturn_marker, cones, finish_marker)
    route_candidates = _beam_search_routes(start_xy, stages, sorted_cones)

    best: Optional[OptimizedPathResult] = None
    for route_cost, route in route_candidates:
        sample_route = _add_terminal_anchor_points(route)
        for tension in SMOOTH_TENSIONS:
            try:
                dense_x, dense_y = _sample_hermite_path(sample_route, tension, PATH_DENSE_SAMPLE_MM)
                out_x, out_y = resample_path(dense_x, dense_y, INTERPOLATE_DIST)
                points = _route_points_from_xy(out_x, out_y)
                validate_uturn_line_crossing(points, uturn_marker, real_cones, finish_marker)
                sprint_entry_xy = route[-2] if len(route) >= 2 else None
                score, clearance = _score_final_path(points, sorted_cones, route_cost, sprint_entry_xy)
            except PathConstraintError:
                continue

            action_xy = route[2] if len(route) > 2 else route[-1]
            result = OptimizedPathResult(
                raw_route=list(route),
                final_points=points,
                action_xy=action_xy,
                min_clearance_mm=clearance,
                score=score,
            )
            if best is None or result.score < best.score:
                best = result

    if best is None:
        raise PathConstraintError("no smoothed candidate path satisfies cone clearance")

    return best


def yaw_accel_speed_limit_mm_s(curvature_rate: float) -> float:
    if curvature_rate <= CURVATURE_EPS:
        return PATH_SPEED_MAX_MM_S
    return math.sqrt(MAX_PATH_YAW_ACCEL_RAD_S2 / curvature_rate)


def apply_speed_plan(points: List[RoutePoint], slalom_start_idx: Optional[int] = None) -> None:
    if not points:
        return

    s_vals = cumulative_arc_length(points)
    curvature = signed_curvature(points)
    rate = curvature_rate_per_mm2(s_vals, curvature)
    speed_limit = np.full(len(points), PRE_UTURN_SPEED_MAX_MM_S, dtype=float)

    for i, kappa in enumerate(curvature):
        local_speed_max = (
            PATH_SPEED_MAX_MM_S
            if slalom_start_idx is not None and i >= int(slalom_start_idx)
            else PRE_UTURN_SPEED_MAX_MM_S
        )
        curve_limit = local_speed_max
        yaw_rate_limit = local_speed_max
        yaw_accel_limit = yaw_accel_speed_limit_mm_s(float(rate[i]))
        if abs(kappa) > CURVATURE_EPS:
            curve_limit = math.sqrt(MAX_LATERAL_ACCEL_MM_S2 / max(abs(kappa), CURVATURE_EPS))
            yaw_rate_limit = MAX_PATH_YAW_RATE_RAD_S / abs(kappa)
        speed_limit[i] = min(local_speed_max, curve_limit, yaw_rate_limit, yaw_accel_limit)
        points[i].curvature = float(kappa)

    # The finish marker is a crossing target, not a stop line.
    if ENABLE_FINISH_SPRINT:
        speed_limit[-1] = min(speed_limit[-1], SPRINT_SPEED_MM_S)
    else:
        speed_limit[-1] = 0.0

    if ENABLE_FINISH_SPRINT and len(points) > 8:
        last_curve_idx = -1
        for i in range(len(curvature) - 1, -1, -1):
            if abs(curvature[i]) > 0.00045:
                last_curve_idx = i
                break
        if last_curve_idx != -1 and last_curve_idx < len(points) - 8:
            for i in range(last_curve_idx + 6, len(points)):
                speed_limit[i] = min(speed_limit[i], SPRINT_SPEED_MM_S)

    for i, point in enumerate(points):
        if point.point_type == POINT_TYPE_TOKENS["NAV_POINT_CIRCLE"]:
            speed_limit[i] = 0.0

    planned = np.array(speed_limit, copy=True)

    for i in range(len(points) - 2, -1, -1):
        ds = s_vals[i + 1] - s_vals[i]
        max_entry = math.sqrt(max(0.0, planned[i + 1] ** 2 + 2.0 * MAX_DECEL_MM_S2 * ds))
        planned[i] = min(planned[i], max_entry)

    for i in range(1, len(points)):
        ds = s_vals[i] - s_vals[i - 1]
        max_exit = math.sqrt(max(0.0, planned[i - 1] ** 2 + 2.0 * MAX_ACCEL_MM_S2 * ds))
        planned[i] = min(planned[i], max_exit)

    target_speed_cmd = -planned / SPEED_TO_MM_S
    for point, speed_cmd in zip(points, target_speed_cmd):
        point.target_speed = float(speed_cmd)


def generate_route_plan(raw_points: List[RoutePoint]) -> Tuple[List[Tuple[float, float]], List[RoutePoint], str]:
    result = generate_optimized_path(raw_points)
    final_points = result.final_points

    if final_points and math.hypot(final_points[0].x, final_points[0].y) < 1.0e-6:
        final_points = final_points[1:]

    slalom_start_idx: Optional[int] = None
    if len(result.raw_route) >= 5 and final_points:
        slalom_xy = result.raw_route[4]
        slalom_start_idx = min(
            range(len(final_points)),
            key=lambda i: math.hypot(final_points[i].x - slalom_xy[0], final_points[i].y - slalom_xy[1]),
        )

    apply_speed_plan(final_points, slalom_start_idx=slalom_start_idx)
    control_points = list(result.raw_route)
    method = (
        "Plan1 Optimized 平滑避障绕桩 "
        f"(clearance={result.min_clearance_mm:.1f}mm, score={result.score:.1f})"
    )
    return control_points, final_points, method


def _finish_line_config(raw_points: Optional[Sequence[RoutePoint]]) -> Optional[Tuple[float, float, float, float]]:
    if raw_points is None or len(raw_points) < 4:
        return None

    uturn_marker = raw_points[0]
    real_cones = raw_points[1:-1]
    finish_marker = raw_points[-1]
    axis, lateral = _course_axis_from_markers_and_cones(uturn_marker, real_cones, finish_marker)
    origin_xy = _point_xy(uturn_marker)
    finish_s, _finish_l = _to_sl(_point_xy(finish_marker), origin_xy, axis, lateral)
    if finish_s <= 0.0:
        axis = (-axis[0], -axis[1])

    return (
        float(finish_marker.x),
        float(finish_marker.y),
        float(axis[0]),
        float(axis[1]),
    )


def generate_header(
    points: Sequence[RoutePoint],
    method_name: str,
    output_path: str,
    start_heading_valid: int,
    start_heading_deg: float,
    raw_points: Optional[Sequence[RoutePoint]] = None,
) -> None:
    output_dir = os.path.dirname(output_path)
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)

    finish_line = _finish_line_config(raw_points)
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("#ifndef _NAV_REPLAY_ROUTE_TABLE_H_\n")
        f.write("#define _NAV_REPLAY_ROUTE_TABLE_H_\n\n")
        f.write('#include "nav_ram.h"\n\n')
        f.write("// Generated by tools/webview_nav_marker_speed_planning/caculate2.0.py\n")
        f.write(f"// Generated at: {timestamp}\n")
        f.write(f"// Method: {method_name}\n")
        f.write(f"// Point spacing: about {INTERPOLATE_DIST:.1f}mm\n")
        f.write(
            f"// Speed plan: pre_uturn_vmax={PRE_UTURN_SPEED_MAX_MM_S:.1f}mm/s, "
            f"slalom_vmax={PATH_SPEED_MAX_MM_S:.1f}mm/s, "
            f"accel={MAX_ACCEL_MM_S2:.1f}mm/s^2, "
            f"decel={MAX_DECEL_MM_S2:.1f}mm/s^2, "
            f"alat={MAX_LATERAL_ACCEL_MM_S2:.1f}mm/s^2, "
            f"forward speed command is negative\n\n"
        )
        f.write(f"#define NAV_REPLAY_START_HEADING_VALID {int(start_heading_valid)}\n")
        f.write(f"#define NAV_REPLAY_START_HEADING_DEG {float(start_heading_deg):.3f}f\n\n")
        if finish_line is not None:
            finish_x, finish_y, finish_nx, finish_ny = finish_line
            f.write("#define NAV_REPLAY_FINISH_LINE_VALID 1\n")
            f.write(f"#define NAV_REPLAY_FINISH_LINE_X_MM {finish_x:.3f}f\n")
            f.write(f"#define NAV_REPLAY_FINISH_LINE_Y_MM {finish_y:.3f}f\n")
            f.write(f"#define NAV_REPLAY_FINISH_LINE_NX {finish_nx:.8f}f\n")
            f.write(f"#define NAV_REPLAY_FINISH_LINE_NY {finish_ny:.8f}f\n\n")
        else:
            f.write("#define NAV_REPLAY_FINISH_LINE_VALID 0\n")
            f.write("#define NAV_REPLAY_FINISH_LINE_X_MM 0.0f\n")
            f.write("#define NAV_REPLAY_FINISH_LINE_Y_MM 0.0f\n")
            f.write("#define NAV_REPLAY_FINISH_LINE_NX 0.0f\n")
            f.write("#define NAV_REPLAY_FINISH_LINE_NY 0.0f\n\n")
        f.write(f"#define NAV_REPLAY_STATIC_ROUTE_COUNT {len(points)}\n\n")
        f.write(f"static const NavRamPoint_t nav_replay_static_route_points[{max(len(points), 1)}] = {{\n")
        if points:
            for p in points:
                f.write(
                    f"    {{{p.x:.3f}f, {p.y:.3f}f, {float(p.target_yaw_deg or 0.0):.3f}f, "
                    f"{float(p.heading_deg or 0.0):.3f}f, (uint8){int(p.point_type)}, "
                    f"{p.target_speed:.3f}f, {p.curvature:.6f}f}},\n"
                )
        else:
            f.write("    {0.0f, 0.0f, 0.0f, 0.0f, (uint8)NAV_POINT_PATH, 0.0f, 0.0f},\n")
        f.write("};\n\n")
        f.write("#endif // _NAV_REPLAY_ROUTE_TABLE_H_\n")


def plot_result(
    raw_points: Sequence[RoutePoint],
    control_points: Sequence[Tuple[float, float]],
    final_points: Sequence[RoutePoint],
) -> None:
    fig, ax = plt.subplots(1, 1, figsize=(14, 10))
    fig.canvas.manager.set_window_title("Plan1 平滑路径预览")
    fig.patch.set_facecolor("#F8FAFC")
    ax.set_facecolor("#FCFCFD")

    if raw_points:
        finish_marker = raw_points[-1]
        cones = raw_points[:-1]
        ax.scatter(
            [0.0],
            [0.0],
            c="#111827",
            marker="*",
            s=150,
            label="起点",
            zorder=8,
        )
        ax.scatter(
            [finish_marker.x],
            [finish_marker.y],
            c="#7C3AED",
            marker="s",
            s=120,
            label="终点线标记",
            edgecolors="white",
            linewidths=0.9,
            zorder=7,
        )
        if cones:
            ax.scatter(
                [p.x for p in cones[1:]],
                [p.y for p in cones[1:]],
                c="#DC2626",
                marker="o",
                s=82,
                label="桩筒",
                edgecolors="white",
                linewidths=0.8,
                zorder=6,
            )
            ax.scatter(
                [cones[0].x],
                [cones[0].y],
                c="#B91C1C",
                marker="D",
                s=132,
                label="掉头线伪桩筒",
                edgecolors="white",
                linewidths=0.9,
                zorder=7,
            )
            for idx, cone in enumerate(cones):
                label = "掉头桩" if idx == 0 else f"桩{idx}"
                ax.annotate(
                    label,
                    (cone.x, cone.y),
                    textcoords="offset points",
                    xytext=(7, 7),
                    fontsize=9,
                    color="#7F1D1D",
                    fontweight="bold",
                )
                circle = plt.Circle(
                    (cone.x, cone.y),
                    MIN_CONE_CLEARANCE_MM,
                    color="#F97316",
                    alpha=0.16,
                    linestyle="--",
                    linewidth=1.0,
                )
                ax.add_patch(circle)
            if len(cones) >= 3:
                axis, lateral = _course_axis_from_markers_and_cones(cones[0], cones[1:], finish_marker)
                origin_xy = _point_xy(cones[0])
                l_values = [
                    _to_sl((p.x, p.y), origin_xy, axis, lateral)[1]
                    for p in list(raw_points) + list(final_points)
                ]
                if control_points:
                    l_values.extend(_to_sl(p, origin_xy, axis, lateral)[1] for p in control_points)
                low_l = min(l_values) - 600.0
                high_l = max(l_values) + 600.0
                line_a = _from_sl(0.0, low_l, origin_xy, axis, lateral)
                line_b = _from_sl(0.0, high_l, origin_xy, axis, lateral)
                ax.plot(
                    [line_a[0], line_b[0]],
                    [line_a[1], line_b[1]],
                    color="#475569",
                    linestyle=(0, (5, 5)),
                    linewidth=1.6,
                    label="掉头线",
                    zorder=2,
                )
                finish_s, _finish_l = _to_sl(_point_xy(finish_marker), origin_xy, axis, lateral)
                finish_line_a = _from_sl(finish_s, low_l, origin_xy, axis, lateral)
                finish_line_b = _from_sl(finish_s, high_l, origin_xy, axis, lateral)
                ax.plot(
                    [finish_line_a[0], finish_line_b[0]],
                    [finish_line_a[1], finish_line_b[1]],
                    color="#7C3AED",
                    linestyle=(0, (4, 4)),
                    linewidth=1.5,
                    label="终点线",
                    zorder=2,
                )

    if control_points:
        ctrl_x = [p[0] for p in control_points]
        ctrl_y = [p[1] for p in control_points]
        ax.plot(
            ctrl_x,
            ctrl_y,
            color="#16A34A",
            marker="+",
            markersize=8,
            linewidth=1.0,
            linestyle="--",
            label="优化控制点",
            alpha=0.72,
            zorder=4,
        )

    if final_points:
        final_x = [p.x for p in final_points]
        final_y = [p.y for p in final_points]
        ax.plot(
            final_x,
            final_y,
            color="#2563EB",
            linewidth=2.25,
            label="最终平滑路径",
            zorder=5,
        )
        for p in final_points:
            if p.point_type != 0:
                name, color, marker = SPECIAL_POINTS_MAP.get(p.point_type, ("特殊点", "black", "X"))
                ax.scatter([p.x], [p.y], c=color, marker=marker, s=120, edgecolors="black", label=name)

        max_k_idx = max(range(len(final_points)), key=lambda i: abs(final_points[i].curvature))
        mk = final_points[max_k_idx]
        ax.plot(mk.x, mk.y, marker="D", color="#F43F5E", markersize=7)
        ax.annotate(
            f"最大曲率={mk.curvature:.6f}",
            (mk.x, mk.y),
            textcoords="offset points",
            xytext=(8, -12),
            fontsize=8,
            color="#BE123C",
        )

    ax.set_xlabel("X (mm)")
    ax.set_ylabel("Y (mm)")
    ax.set_title("科目一 Plan1 平滑避障路径", fontsize=14, fontweight="bold")
    ax.axis("equal")
    ax.grid(True, linestyle="--", alpha=0.36, color="#94A3B8")
    for spine in ax.spines.values():
        spine.set_color("#CBD5E1")
    handles, labels = ax.get_legend_handles_labels()
    dedup: Dict[str, object] = {}
    for handle, label in zip(handles, labels):
        dedup[label] = handle
    ax.legend(
        dedup.values(),
        dedup.keys(),
        fontsize=9,
        loc="best",
        frameon=True,
        framealpha=0.92,
        facecolor="white",
        edgecolor="#E2E8F0",
    )
    plt.tight_layout()
    plt.show()


def plot_speed_profile(final_points: Sequence[RoutePoint], raw_points: Optional[Sequence[RoutePoint]] = None) -> None:
    if not final_points:
        return

    s_vals = cumulative_arc_length(final_points)
    speeds = [-p.target_speed * SPEED_TO_MM_S for p in final_points]
    curvatures = [p.curvature for p in final_points]

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), sharex=True)
    fig.canvas.manager.set_window_title("Plan1 速度规划预览")
    fig.patch.set_facecolor("#F8FAFC")
    for ax in (ax1, ax2):
        ax.set_facecolor("#FCFCFD")
        ax.grid(True, linestyle="--", alpha=0.36, color="#94A3B8")
        for spine in ax.spines.values():
            spine.set_color("#CBD5E1")

    ax1.plot(s_vals, speeds, color="#2563EB", linewidth=1.7)
    ax1.axhline(PRE_UTURN_SPEED_MAX_MM_S, color="#64748B", linestyle="--", alpha=0.48, label="掉头前速度上限")
    ax1.axhline(PATH_SPEED_MAX_MM_S, color="#DC2626", linestyle="--", alpha=0.42, label="绕桩速度上限")
    ax1.set_ylabel("目标速度 (mm/s)")
    ax1.set_title("纵向速度规划", fontsize=13, fontweight="bold")
    ax1.legend()

    ax2.plot(s_vals, curvatures, color="#16A34A", linewidth=1.15)
    ax2.set_ylabel("曲率 (1/mm)")
    ax2.set_xlabel("弧长 (mm)")
    ax2.set_title("路径曲率", fontsize=13, fontweight="bold")

    if raw_points:
        fx = np.array([p.x for p in final_points])
        fy = np.array([p.y for p in final_points])
        for i, rp in enumerate(raw_points):
            dists_sq = (fx - rp.x) ** 2 + (fy - rp.y) ** 2
            s_mark = s_vals[int(np.argmin(dists_sq))]
            ax1.axvline(x=s_mark, color="gray", linestyle=":", alpha=0.35)
            ax2.axvline(x=s_mark, color="gray", linestyle=":", alpha=0.35)
            ax2.text(s_mark, ax2.get_ylim()[1], f"点{i}", ha="center", va="top", fontsize=8, color="#64748B")

    plt.tight_layout()
    plt.show()


def auto_find_latest_csv(script_dir: Path) -> Path:
    candidates = sorted(
        script_dir.glob("nav_mark_points_*.csv"),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )
    if not candidates:
        raise FileNotFoundError("no nav_mark_points_*.csv found")
    return candidates[0]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Optimized plan1 offline planner: pseudo u-turn cone + cone points -> "
            "smooth clearance-safe route + speed plan"
        )
    )
    parser.add_argument("input", nargs="?", help="input CSV/header path; defaults to latest nav_mark_points_*.csv")
    parser.add_argument("--output", help="output header path; defaults to code/navigation/nav_replay_route_table.h")
    parser.add_argument("--no-plot", action="store_true", help="skip preview windows")
    parser.add_argument("--clearance-mm", type=float, help="minimum cone clearance, default 400mm")
    parser.add_argument("--no-finish-sprint", action="store_true", help="disable finish sprint section")
    return parser.parse_args()


def main() -> int:
    global MIN_CONE_CLEARANCE_MM, ENABLE_FINISH_SPRINT

    args = parse_args()
    if args.clearance_mm is not None:
        if args.clearance_mm <= 0.0:
            raise ValueError("--clearance-mm must be positive")
        MIN_CONE_CLEARANCE_MM = float(args.clearance_mm)
    if args.no_finish_sprint:
        ENABLE_FINISH_SPRINT = False

    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent.parent

    input_path = Path(args.input).resolve() if args.input else auto_find_latest_csv(script_dir)
    default_output = project_root / "code" / "navigation" / "nav_replay_route_table.h"
    output_path = Path(args.output).resolve() if args.output else default_output

    if str(input_path).lower().endswith(".csv"):
        raw_points, start_heading_valid, start_heading_deg = read_csv_points(str(input_path))
    else:
        raw_points, start_heading_valid, start_heading_deg = read_route_header(str(input_path))

    if len(raw_points) < 4:
        print("[error] need at least: u-turn marker + two cones + finish marker")
        return 1

    print(f"[input] file: {input_path}")
    print(f"[input] raw points: {len(raw_points)}")
    print(f"[plan1] pseudo u-turn cone: ({raw_points[0].x:.1f}, {raw_points[0].y:.1f})")
    print(f"[plan1] cones including pseudo: {len(raw_points) - 1}")
    print(f"[plan1] finish marker: ({raw_points[-1].x:.1f}, {raw_points[-1].y:.1f})")
    print(f"[params] min cone clearance: {MIN_CONE_CLEARANCE_MM:.1f}mm")

    control_points, final_points, method_name = generate_route_plan(raw_points)
    generate_header(
        final_points,
        method_name,
        str(output_path),
        start_heading_valid,
        start_heading_deg,
        raw_points=raw_points,
    )

    print(f"[method] {method_name}")
    print(f"[output] header: {output_path}")
    print(f"[output] route points: {len(final_points)}")
    print(
        "[output] speed plan: "
        f"pre_uturn_vmax={PRE_UTURN_SPEED_MAX_MM_S:.1f}mm/s, "
        f"slalom_vmax={PATH_SPEED_MAX_MM_S:.1f}mm/s, "
        f"accel={MAX_ACCEL_MM_S2:.1f}mm/s^2, "
        f"decel={MAX_DECEL_MM_S2:.1f}mm/s^2, "
        "forward target_speed is negative"
    )

    if not args.no_plot:
        plot_result(raw_points, control_points, final_points)
        plot_speed_profile(final_points, raw_points)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
