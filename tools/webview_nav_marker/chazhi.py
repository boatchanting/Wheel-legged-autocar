#!/usr/bin/env python3
"""Smooth route points and generate a speed-planned nav route table."""

from __future__ import annotations

import argparse
import math
import os
import re
import sys
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple

import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import Akima1DInterpolator, PchipInterpolator, interp1d, splprep, splev

INTERPOLATE_DIST = 20.0
PATH_SPEED_MAX_MM_S = 1800.0
MAX_ACCEL_MM_S2 = 1200.0
MAX_DECEL_MM_S2 = 1800.0
MAX_LATERAL_ACCEL_MM_S2 = 1800.0
SPEED_TO_MM_S = 5.183
CURVATURE_EPS = 1e-6
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
    1: ("Circle", "#FF00FF", "o"),
    2: ("Slope", "#FFA500", "^"),
    3: ("Jump", "#00FFFF", "v"),
    4: ("Bridge", "#8B4513", "s"),
    5: ("Bump", "#800080", "D"),
}


@dataclass
class RoutePoint:
    x: float
    y: float
    target_yaw_deg: Optional[float]
    heading_deg: Optional[float]
    target_speed: float
    point_type: int


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Interpolate route points and plan target speed")
    parser.add_argument(
        "--input",
        help="Input route-table header path (default: code/navigation/nav_replay_route_table.h)",
    )
    parser.add_argument(
        "--output",
        help="Output route-table header path (default: overwrite input file)",
    )
    parser.add_argument(
        "--method",
        choices=["1", "2", "3", "4", "5", "6"],
        help="Interpolation method id. Defaults to interactive selection or [4] in non-interactive mode.",
    )
    parser.add_argument(
        "--no-plot",
        action="store_true",
        help="Skip the interpolation preview window.",
    )
    return parser.parse_args()


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
    return int(token)


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


def read_route_header(file_path: str) -> Tuple[List[RoutePoint], int, float]:
    if not os.path.exists(file_path):
        raise FileNotFoundError(file_path)

    with open(file_path, "r", encoding="utf-8") as f:
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

    pattern_v6 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*\}}"
    )
    pattern_v5 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*\}}"
    )
    pattern_v3 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*\}}"
    )

    for match in pattern_v6.finditer(body):
        points.append(
            RoutePoint(
                x=float(match.group(1)),
                y=float(match.group(2)),
                target_yaw_deg=normalize_relative_yaw_deg(float(match.group(3))),
                heading_deg=normalize_heading_deg(float(match.group(4))),
                target_speed=float(match.group(5)),
                point_type=parse_point_type(match.group(6)),
            )
        )

    if not points:
        for match in pattern_v5.finditer(body):
            points.append(
                RoutePoint(
                    x=float(match.group(1)),
                    y=float(match.group(2)),
                    target_yaw_deg=normalize_relative_yaw_deg(float(match.group(3))),
                    heading_deg=normalize_heading_deg(float(match.group(4))),
                    target_speed=0.0,
                    point_type=parse_point_type(match.group(5)),
                )
            )

    if not points:
        for match in pattern_v3.finditer(body):
            points.append(
                RoutePoint(
                    x=float(match.group(1)),
                    y=float(match.group(2)),
                    target_yaw_deg=None,
                    heading_deg=None,
                    target_speed=0.0,
                    point_type=parse_point_type(match.group(3)),
                )
            )

    infer_missing_angles(points)
    return points, start_heading_valid, start_heading_deg


def generate_header(
    points: List[RoutePoint],
    method_name: str,
    output_path: str,
    start_heading_valid: int,
    start_heading_deg: float,
) -> None:
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("#ifndef _NAV_REPLAY_ROUTE_TABLE_H_\n")
        f.write("#define _NAV_REPLAY_ROUTE_TABLE_H_\n\n")
        f.write('#include "nav_ram.h"\n\n')
        f.write("// Auto-generated by tools/webview_nav_marker/chazhi.py\n")
        f.write(f"// Generated at: {timestamp}\n")
        f.write(f"// Interpolation Method: {method_name}\n")
        f.write(f"// Interpolation interval: ~{INTERPOLATE_DIST}mm\n")
        f.write(
            f"// Speed plan: vmax={PATH_SPEED_MAX_MM_S:.1f}mm/s, "
            f"a+={MAX_ACCEL_MM_S2:.1f}mm/s^2, "
            f"a-={MAX_DECEL_MM_S2:.1f}mm/s^2, "
            f"alat={MAX_LATERAL_ACCEL_MM_S2:.1f}mm/s^2, "
            f"SPEED_TO_MM_S={SPEED_TO_MM_S:.3f}\n\n"
        )
        f.write(f"#define NAV_REPLAY_START_HEADING_VALID {start_heading_valid}\n")
        f.write(f"#define NAV_REPLAY_START_HEADING_DEG {start_heading_deg:.3f}f\n\n")
        f.write(f"#define NAV_REPLAY_STATIC_ROUTE_COUNT {len(points)}\n\n")
        f.write(f"static const NavRamPoint_t nav_replay_static_route_points[{max(len(points), 1)}] = {{\n")
        if points:
            for p in points:
                f.write(
                    f"    {{{p.x:.3f}f, {p.y:.3f}f, {p.target_yaw_deg:.3f}f, "
                    f"{p.heading_deg:.3f}f, {p.target_speed:.3f}f, (uint8){p.point_type}}},\n"
                )
        else:
            f.write("    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, NAV_POINT_PATH},\n")
        f.write("};\n\n")
        f.write("#endif // _NAV_REPLAY_ROUTE_TABLE_H_\n")


def resample_path(x_fine: np.ndarray, y_fine: np.ndarray, target_dist: float) -> Tuple[np.ndarray, np.ndarray]:
    diffs = np.sqrt(np.diff(x_fine) ** 2 + np.diff(y_fine) ** 2)
    cum_dist = np.insert(np.cumsum(diffs), 0, 0.0)
    total_length = cum_dist[-1]

    if total_length == 0.0:
        return x_fine, y_fine

    num_points = max(int(total_length / target_dist), 2)
    target_dists = np.linspace(0.0, total_length, num_points)
    x_new = np.interp(target_dists, cum_dist, x_fine)
    y_new = np.interp(target_dists, cum_dist, y_fine)
    return x_new, y_new


def corner_fillet_path(
    x_orig: np.ndarray,
    y_orig: np.ndarray,
    fillet_radius: float = 500.0,
) -> Tuple[np.ndarray, np.ndarray]:
    pts = np.vstack((x_orig, y_orig)).T
    new_pts = [pts[0]]

    for i in range(1, len(pts) - 1):
        p0, p1, p2 = pts[i - 1], pts[i], pts[i + 1]
        v1 = p0 - p1
        v2 = p2 - p1
        l1 = np.linalg.norm(v1)
        l2 = np.linalg.norm(v2)

        if l1 < 1e-3 or l2 < 1e-3:
            new_pts.append(p1)
            continue

        v1_norm = v1 / l1
        v2_norm = v2 / l2
        cut_len = min(fillet_radius, l1 * 0.45, l2 * 0.45)
        p_start = p1 + v1_norm * cut_len
        p_end = p1 + v2_norm * cut_len

        t = np.linspace(0.0, 1.0, 20)
        bezier_x = (1.0 - t) ** 2 * p_start[0] + 2.0 * (1.0 - t) * t * p1[0] + t ** 2 * p_end[0]
        bezier_y = (1.0 - t) ** 2 * p_start[1] + 2.0 * (1.0 - t) * t * p1[1] + t ** 2 * p_end[1]
        new_pts.extend(np.vstack((bezier_x, bezier_y)).T)

    new_pts.append(pts[-1])
    new_pts = np.array(new_pts)
    return new_pts[:, 0], new_pts[:, 1]


def b_spline_path(x_orig: np.ndarray, y_orig: np.ndarray, smooth_factor: float = 5.0) -> Tuple[np.ndarray, np.ndarray]:
    tck, _ = splprep([x_orig, y_orig], s=smooth_factor, k=min(3, len(x_orig) - 1))
    u_fine = np.linspace(0.0, 1.0, 2000)
    x_spline, y_spline = splev(u_fine, tck)
    return np.array(x_spline), np.array(y_spline)


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


def build_methods(x_orig: np.ndarray, y_orig: np.ndarray) -> Dict[str, Tuple[str, np.ndarray, np.ndarray]]:
    diffs = np.sqrt(np.diff(x_orig) ** 2 + np.diff(y_orig) ** 2)
    t_orig = np.insert(np.cumsum(diffs), 0, 0.0)
    if t_orig[-1] == 0.0:
        t_orig[-1] = 1.0
    t_orig = t_orig / t_orig[-1]
    u_fine = np.linspace(0.0, 1.0, 2000)

    methods: Dict[str, Tuple[str, np.ndarray, np.ndarray]] = {}

    tck_cubic, _ = splprep([x_orig, y_orig], s=0, k=min(3, len(x_orig) - 1))
    x_c, y_c = splev(u_fine, tck_cubic)
    methods["1"] = ("Cubic Spline", *resample_path(np.array(x_c), np.array(y_c), INTERPOLATE_DIST))

    fx_pchip = PchipInterpolator(t_orig, x_orig)
    fy_pchip = PchipInterpolator(t_orig, y_orig)
    methods["2"] = ("PCHIP", *resample_path(fx_pchip(u_fine), fy_pchip(u_fine), INTERPOLATE_DIST))

    fx_akima = Akima1DInterpolator(t_orig, x_orig)
    fy_akima = Akima1DInterpolator(t_orig, y_orig)
    methods["3"] = ("Akima", *resample_path(fx_akima(u_fine), fy_akima(u_fine), INTERPOLATE_DIST))

    avg_dist = float(np.mean(diffs)) if len(diffs) else INTERPOLATE_DIST
    x_fillet, y_fillet = corner_fillet_path(x_orig, y_orig, fillet_radius=avg_dist * 0.5)
    methods["4"] = ("Corner Fillet", *resample_path(x_fillet, y_fillet, INTERPOLATE_DIST))

    x_bs, y_bs = b_spline_path(x_orig, y_orig, smooth_factor=max(len(x_orig) * 10, 1))
    methods["5"] = ("B-Spline", *resample_path(x_bs, y_bs, INTERPOLATE_DIST))

    fx_lin = interp1d(t_orig, x_orig, kind="linear")
    fy_lin = interp1d(t_orig, y_orig, kind="linear")
    methods["6"] = ("Linear", *resample_path(fx_lin(u_fine), fy_lin(u_fine), INTERPOLATE_DIST))

    return methods


def show_methods(raw_points: List[RoutePoint], methods: Dict[str, Tuple[str, np.ndarray, np.ndarray]]) -> None:
    plt.rcParams["font.sans-serif"] = ["SimHei", "DejaVu Sans", "Arial Unicode MS"]
    plt.rcParams["axes.unicode_minus"] = False
    plt.ion()
    fig, axs = plt.subplots(2, 3, figsize=(16, 8))
    fig.canvas.manager.set_window_title("Route interpolation preview")
    axs = axs.flatten()

    x_orig = np.array([p.x for p in raw_points])
    y_orig = np.array([p.y for p in raw_points])
    special_points = [p for p in raw_points if p.point_type != 0]

    for ax, (key, (name, x_res, y_res)) in zip(axs, methods.items()):
        ax.plot(x_orig, y_orig, "ro-", label="Raw points", markersize=4, alpha=0.5, zorder=3)
        ax.plot(x_res, y_res, "b.-", markersize=3, label="Interpolated", zorder=2)
        for p in special_points:
            label, color, marker = SPECIAL_POINTS_MAP.get(p.point_type, ("Special", "black", "X"))
            ax.scatter([p.x], [p.y], c=color, marker=marker, s=80, edgecolors="black", label=label, zorder=5)
        ax.set_title(f"[{key}] {name}")
        ax.axis("equal")
        ax.grid(True, linestyle="--", alpha=0.6)
        ax.legend(fontsize=8, loc="best")

    plt.tight_layout()
    plt.pause(0.5)


def build_final_points(
    raw_points: List[RoutePoint],
    sel_x: np.ndarray,
    sel_y: np.ndarray,
    drop_first_count: int,
) -> List[RoutePoint]:
    final_x = np.array(sel_x, dtype=float)
    final_y = np.array(sel_y, dtype=float)
    final_type = np.zeros(len(final_x), dtype=int)
    final_heading = np.zeros(len(final_x), dtype=float)

    for idx, original in enumerate(raw_points):
        if original.point_type != 0 or idx == len(raw_points) - 1:
            dists_sq = (final_x - original.x) ** 2 + (final_y - original.y) ** 2
            closest_idx = int(np.argmin(dists_sq))
            final_x[closest_idx] = original.x
            final_y[closest_idx] = original.y
            final_type[closest_idx] = original.point_type
            final_heading[closest_idx] = float(original.heading_deg or 0.0)

    final_yaw = tangent_yaws(final_x, final_y)

    for idx, original in enumerate(raw_points):
        if original.point_type != 0 or idx == len(raw_points) - 1:
            dists_sq = (final_x - original.x) ** 2 + (final_y - original.y) ** 2
            closest_idx = int(np.argmin(dists_sq))
            final_yaw[closest_idx] = float(original.target_yaw_deg or 0.0)
            final_heading[closest_idx] = float(original.heading_deg or 0.0)

    start_idx = drop_first_count if len(final_x) > drop_first_count else 0
    return [
        RoutePoint(
            x=float(final_x[i]),
            y=float(final_y[i]),
            target_yaw_deg=normalize_relative_yaw_deg(float(final_yaw[i])),
            heading_deg=normalize_heading_deg(float(final_heading[i])),
            target_speed=0.0,
            point_type=int(final_type[i]),
        )
        for i in range(start_idx, len(final_x))
    ]


def prepare_spline_input_points(raw_points: List[RoutePoint]) -> Tuple[List[RoutePoint], int]:
    if raw_points and math.isclose(raw_points[0].x, 0.0, abs_tol=1e-6) and math.isclose(raw_points[0].y, 0.0, abs_tol=1e-6):
        return list(raw_points), 0

    return [RoutePoint(0.0, 0.0, 0.0, 0.0, 0.0, 0)] + list(raw_points), 1


def cumulative_arc_length(points: List[RoutePoint]) -> np.ndarray:
    if not points:
        return np.array([], dtype=float)
    x_vals = np.array([p.x for p in points], dtype=float)
    y_vals = np.array([p.y for p in points], dtype=float)
    diffs = np.sqrt(np.diff(x_vals) ** 2 + np.diff(y_vals) ** 2)
    return np.insert(np.cumsum(diffs), 0, 0.0)


def signed_curvature(points: List[RoutePoint]) -> np.ndarray:
    count = len(points)
    curvature = np.zeros(count, dtype=float)
    if count < 3:
        return curvature

    x_vals = np.array([p.x for p in points], dtype=float)
    y_vals = np.array([p.y for p in points], dtype=float)

    for i in range(1, count - 1):
        ax = x_vals[i] - x_vals[i - 1]
        ay = y_vals[i] - y_vals[i - 1]
        bx = x_vals[i + 1] - x_vals[i]
        by = y_vals[i + 1] - y_vals[i]
        a = math.hypot(ax, ay)
        b = math.hypot(bx, by)
        c = math.hypot(x_vals[i + 1] - x_vals[i - 1], y_vals[i + 1] - y_vals[i - 1])
        denom = a * b * c
        if denom <= CURVATURE_EPS:
            curvature[i] = 0.0
            continue
        cross = ax * by - ay * bx
        curvature[i] = 2.0 * cross / denom

    curvature[0] = curvature[1]
    curvature[-1] = curvature[-2]
    return curvature


def apply_speed_plan(points: List[RoutePoint]) -> None:
    if not points:
        return

    s_vals = cumulative_arc_length(points)
    curvature = signed_curvature(points)
    speed_limit = np.full(len(points), PATH_SPEED_MAX_MM_S, dtype=float)

    for i, kappa in enumerate(curvature):
        curve_limit = PATH_SPEED_MAX_MM_S
        if abs(kappa) > CURVATURE_EPS:
            curve_limit = math.sqrt(MAX_LATERAL_ACCEL_MM_S2 / max(abs(kappa), CURVATURE_EPS))
        speed_limit[i] = min(PATH_SPEED_MAX_MM_S, curve_limit)

    speed_limit[-1] = 0.0

    for i, point in enumerate(points):
        if point.point_type == POINT_TYPE_TOKENS["NAV_POINT_CIRCLE"]:
            speed_limit[i] = 0.0

    planned_speed = np.array(speed_limit, copy=True)

    for i in range(len(points) - 2, -1, -1):
        ds = s_vals[i + 1] - s_vals[i]
        max_entry = math.sqrt(max(0.0, planned_speed[i + 1] ** 2 + 2.0 * MAX_DECEL_MM_S2 * ds))
        planned_speed[i] = min(planned_speed[i], max_entry)

    for i in range(1, len(points)):
        ds = s_vals[i] - s_vals[i - 1]
        max_exit = math.sqrt(max(0.0, planned_speed[i - 1] ** 2 + 2.0 * MAX_ACCEL_MM_S2 * ds))
        planned_speed[i] = min(planned_speed[i], max_exit)

    target_speed_cmd = -planned_speed / SPEED_TO_MM_S
    for point, speed_cmd in zip(points, target_speed_cmd):
        point.target_speed = float(speed_cmd)


def choose_method(
    args: argparse.Namespace,
    methods: Dict[str, Tuple[str, np.ndarray, np.ndarray]],
) -> str:
    if args.method in methods:
        return args.method

    if args.no_plot or not sys.stdin.isatty():
        return "4"

    print("\n" + "=" * 64)
    print("Interpolation methods")
    print("[1] Cubic Spline")
    print("[2] PCHIP")
    print("[3] Akima")
    print("[4] Corner Fillet")
    print("[5] B-Spline")
    print("[6] Linear")
    print("=" * 64)

    choice = input("Select method (1-6) [default 4]: ").strip() or "4"
    if choice not in methods:
        print("Invalid input, falling back to [4] Corner Fillet.")
        return "4"
    return choice


def main() -> int:
    args = parse_args()

    default_header = Path(__file__).resolve().parents[2] / "code" / "navigation" / "nav_replay_route_table.h"
    input_path = Path(args.input).resolve() if args.input else default_header
    output_path = Path(args.output).resolve() if args.output else input_path

    raw_points, start_heading_valid, start_heading_deg = read_route_header(str(input_path))
    if not raw_points:
        print("No route points were loaded from the input header.")
        return 1

    spline_input_points, drop_first_count = prepare_spline_input_points(raw_points)
    x_orig = np.array([p.x for p in spline_input_points], dtype=float)
    y_orig = np.array([p.y for p in spline_input_points], dtype=float)
    methods = build_methods(x_orig, y_orig)

    if not args.no_plot:
        show_methods(spline_input_points, methods)

    choice = choose_method(args, methods)
    plt.close("all")

    method_name, sel_x, sel_y = methods[choice]
    final_points = build_final_points(spline_input_points, sel_x, sel_y, drop_first_count)
    apply_speed_plan(final_points)
    generate_header(final_points, method_name, str(output_path), start_heading_valid, start_heading_deg)

    print(f"Selected interpolation: {method_name}")
    print(f"Generated header: {output_path}")
    print(f"Route points written: {len(final_points)}")
    print(
        "Speed plan defaults: "
        f"vmax={PATH_SPEED_MAX_MM_S:.1f}mm/s, "
        f"a+={MAX_ACCEL_MM_S2:.1f}mm/s^2, "
        f"a-={MAX_DECEL_MM_S2:.1f}mm/s^2, "
        f"alat={MAX_LATERAL_ACCEL_MM_S2:.1f}mm/s^2"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
