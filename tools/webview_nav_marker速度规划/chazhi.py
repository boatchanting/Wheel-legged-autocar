#!/usr/bin/env python3
"""轨迹平滑与离线速度规划脚本：读取路表、插值、规划速度并输出 6 字段头文件。"""

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
plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
# 插值后的目标点间距（mm）。
# 作用：决定原始轨迹在平滑后被重采样成多密的点列，后续航向、曲率、速度规划都基于这批点计算。
# 调参影响：
# 1. 该值更小：路径几何细节保留更多，急弯曲率估计更细，但点数会上升，文件更大，规划更容易放大局部毛刺。
# 2. 该值更大：路径更稀疏，计算更省，但弯道细节可能被“抹平”，导致曲率峰值被低估。
# 经验例子：
# 1. 普通室内赛道、点位较平滑时，20mm 往往够用。
# 2. 如果发现 U 型弯被采样得太粗，弯中限速不明显，可尝试降到 10~15mm。
# 3. 如果原始打点本身噪声较大，先不要盲目继续减小，否则容易把噪声当成真实弯折。
INTERPOLATE_DIST = 50.0


# 纵向速度上限（mm/s）。
# 作用：整条轨迹的绝对最高目标速度上限，哪怕直道再长、曲率再小，规划速度也不会超过它。
# 调参影响：
# 1. 调大后，长直道会更积极地提速，但也会提高制动压力和出弯后的再加速幅度。
# 2. 调小后，整体风格更保守，调试更稳，但会直接压低直道平均速度。
# 经验例子：
# 1. 设为 1800 时，理论上速度规划不会给出高于 1800mm/s 的目标速度。
# 2. 如果实车在直道末端总是来不及刹住，除了看减速度参数，也应先确认这个上限是否定得过高。
PATH_SPEED_MAX_MM_S = 5000.0


# 最大可用加速度（mm/s^2）。
# 作用：前向扫描阶段的“提速能力”约束，表示车辆沿路径从低速加到高速时，允许使用的最大纵向加速度。
# 调参影响：
# 1. 调大后，出弯和起步后的升速更快，速度曲线更激进。
# 2. 调小后，速度抬升会更平缓，更接近“保守油门”，适合先求稳。
# 经验例子：
# 1. 如果规划结果里长直道前半段升速太慢，明明车还能继续冲，可适当增大它。
# 2. 如果实车总在出弯后突然猛窜、驱动轮容易空转，可适当减小它。
MAX_ACCEL_MM_S2 = 2500.0


# 最大可用减速度（mm/s^2）。
# 作用：反向扫描阶段的“刹车能力”约束，表示车辆为了赶上下一个低速点，最多能以多大的减速度提前降速。
# 调参影响：
# 1. 调大后，规划会更敢于晚刹车，弯前或停止点前的减速区更短。
# 2. 调小后，规划会更早开始收速，停车和入弯更保守，但直道利用率会下降。
# 经验例子：
# 1. 如果圆环点前经常刹不住、停止点有明显前冲，优先检查这个值是不是过大。
# 2. 如果车明明刹得住，却在很早之前就开始“怂”下来，可以适当增大它。
MAX_DECEL_MM_S2 = 1500.0


# 最大横向加速度（mm/s^2）。
# 作用：弯道速度上限的核心参数。规划会按 v^2 / R 或等价的 v = sqrt(a_lat / |k|) 估算弯中安全速度。
# 调参影响：
# 1. 调大后，弯中允许速度更高，整条轨迹的曲率限速谷值会变浅。
# 2. 调小后，遇到急弯会更早、更明显地降速，打滑和外扩风险更低。
# 经验例子：
# 1. 如果 U 型弯里总推头、外扩或甩尾，通常应先减小它。
# 2. 如果车在弯中明显还很稳，但规划速度低得离谱，可能是这个值设得太保守。
MAX_LATERAL_ACCEL_MM_S2 = 1500.0


# 速度指令换算系数（rpm -> mm/s）。
# 作用：把规划内部使用的 mm/s 速度，换算成 C 端底盘控制实际使用的电机目标转速 rpm。
# 使用关系：写回头文件时采用 target_speed_cmd = -target_speed_mm_s / SPEED_TO_MM_S。
# 调参影响：
# 1. 该值偏大：同样的 mm/s 会被换算成更小的 rpm 指令，整车实际会偏慢。
# 2. 该值偏小：同样的 mm/s 会被换算成更大的 rpm 指令，整车实际会偏快。
# 经验例子：
# 1. 以当前 CAR_SELECT=3 为例，若目标速度是 1036.6mm/s，则会换算为约 -200rpm 的目标转速指令。
# 2. 如果规划曲线形状正确，但全程“整体快一档或慢一档”，优先检查这个换算系数是否匹配当前车型。
SPEED_TO_MM_S = 4.936


# 曲率计算分母保护项。
# 作用：当某一段路径几乎完全笔直、曲率极接近 0 时，用它避免除零和数值爆炸。
# 调参原则：
# 1. 它不是“性能参数”，而是数值稳定性保护项，通常保持很小即可。
# 2. 过大时会把本应接近 0 的曲率“垫高”，使直道限速被不必要地压低。
# 经验例子：
# 1. 在完全直线段中，真实曲率应非常接近 0；此时会用 max(|k|, CURVATURE_EPS) 保护分母。
# 2. 除非发现浮点异常或速度计算爆炸，一般不需要改它。


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
    1: ("圆环点", "#FF00FF", "o"),
    2: ("坡道点", "#FFA500", "^"),
    3: ("跳跃点", "#00FFFF", "v"),
    4: ("桥面点", "#8B4513", "s"),
    5: ("颠簸点", "#800080", "D"),
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
    """解析命令行参数。"""
    parser = argparse.ArgumentParser(description="轨迹插值并生成离线速度规划路表")
    parser.add_argument(
        "--input",
        help="输入路表头文件路径（默认：code/navigation/nav_replay_route_table.h）",
    )
    parser.add_argument(
        "--output",
        help="输出路表头文件路径（默认：覆盖输入文件）",
    )
    parser.add_argument(
        "--method",
        choices=["1", "2", "3", "4", "5", "6"],
        help="插值方法编号。未指定时：交互模式手选，非交互模式默认 4。",
    )
    parser.add_argument(
        "--no-plot",
        action="store_true",
        help="跳过插值对比预览窗口。",
    )
    return parser.parse_args()


def normalize_relative_yaw_deg(value: float) -> float:
    """将相对航向角归一化到 (-180, 180]。"""
    while value > 180.0:
        value -= 360.0
    while value <= -180.0:
        value += 360.0
    return value


def normalize_heading_deg(value: float) -> float:
    """将绝对航向角归一化到 [0, 360)。"""
    value = math.fmod(value, 360.0)
    if value < 0.0:
        value += 360.0
    return value


def calc_path_yaw_deg(x0: float, y0: float, x1: float, y1: float) -> float:
    """按项目坐标系定义计算路径切向角（deg）。"""
    return -math.degrees(math.atan2(y1 - y0, -(x1 - x0)))


def parse_point_type(token: str) -> int:
    """解析点类型标记（支持枚举名或数字）。"""
    token = token.strip()
    if token in POINT_TYPE_TOKENS:
        return POINT_TYPE_TOKENS[token]
    return int(token)


def infer_missing_angles(points: List[RoutePoint]) -> None:
    """补全缺失的 target_yaw_deg 与 heading_deg。"""
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
    """
    读取路表头文件，兼容 3/5/6 字段点格式。

    @return (轨迹点列表, 起跑航向有效标志, 起跑航向角)
    @note 调用位置：main() 主流程入口
    """
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
    """
    生成 6 字段导航路表头文件。

    @note 调用位置：main() 中完成插值与速度规划后调用
    """
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("#ifndef _NAV_REPLAY_ROUTE_TABLE_H_\n")
        f.write("#define _NAV_REPLAY_ROUTE_TABLE_H_\n\n")
        f.write('#include "nav_ram.h"\n\n')
        f.write("// 由 tools/webview_nav_marker/chazhi.py 自动生成\n")
        f.write(f"// 生成时间：{timestamp}\n")
        f.write(f"// 插值方法：{method_name}\n")
        f.write(f"// 插值间距：约 {INTERPOLATE_DIST}mm\n")
        f.write(
            f"// 速度规划: vmax={PATH_SPEED_MAX_MM_S:.1f}mm/s, "
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
                    f"{p.heading_deg:.3f}f, (uint8){p.point_type}, {p.target_speed:.3f}f}},\n"
                )
        else:
            f.write("    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, NAV_POINT_PATH},\n")
        f.write("};\n\n")
        f.write("#endif // _NAV_REPLAY_ROUTE_TABLE_H_\n")


def resample_path(x_fine: np.ndarray, y_fine: np.ndarray, target_dist: float) -> Tuple[np.ndarray, np.ndarray]:
    """按弧长重采样路径，使点间距接近 target_dist。"""
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
    """对折角进行圆角化，减小路径尖角突变。"""
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
    """使用 B 样条对路径进行平滑。"""
    tck, _ = splprep([x_orig, y_orig], s=smooth_factor, k=min(3, len(x_orig) - 1))
    u_fine = np.linspace(0.0, 1.0, 2000)
    x_spline, y_spline = splev(u_fine, tck)
    return np.array(x_spline), np.array(y_spline)


def tangent_yaws(x_vals: np.ndarray, y_vals: np.ndarray) -> np.ndarray:
    """根据相邻点切向计算每个点的 target_yaw。"""
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
    """构建多种插值方法候选，用于人工/脚本选择。"""
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
    """可视化不同插值方法效果，供人工对比。"""
    plt.rcParams["font.sans-serif"] = ["SimHei", "DejaVu Sans", "Arial Unicode MS"]
    plt.rcParams["axes.unicode_minus"] = False
    plt.ion()
    fig, axs = plt.subplots(2, 3, figsize=(16, 8))
    fig.canvas.manager.set_window_title("轨迹插值预览")
    axs = axs.flatten()

    x_orig = np.array([p.x for p in raw_points])
    y_orig = np.array([p.y for p in raw_points])
    special_points = [p for p in raw_points if p.point_type != 0]

    for ax, (key, (name, x_res, y_res)) in zip(axs, methods.items()):
        ax.plot(x_orig, y_orig, "ro-", label="原始点", markersize=4, alpha=0.5, zorder=3)
        ax.plot(x_res, y_res, "b.-", label="插值后", markersize=3, zorder=2)
        for p in special_points:
            label, color, marker = SPECIAL_POINTS_MAP.get(p.point_type, ("特殊点", "black", "X"))
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
    """
    构建最终轨迹点序列，并回填特殊点类型与姿态信息。

    @note 调用位置：main() 选择插值方法后调用
    """
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
    """准备样条输入点，必要时补充原点并返回需丢弃的前缀点数。"""
    if raw_points and math.isclose(raw_points[0].x, 0.0, abs_tol=1e-6) and math.isclose(raw_points[0].y, 0.0, abs_tol=1e-6):
        return list(raw_points), 0

    return [RoutePoint(0.0, 0.0, 0.0, 0.0, 0.0, 0)] + list(raw_points), 1


def cumulative_arc_length(points: List[RoutePoint]) -> np.ndarray:
    """计算每个轨迹点对应的累计弧长 s（mm）。"""
    if not points:
        return np.array([], dtype=float)
    x_vals = np.array([p.x for p in points], dtype=float)
    y_vals = np.array([p.y for p in points], dtype=float)
    diffs = np.sqrt(np.diff(x_vals) ** 2 + np.diff(y_vals) ** 2)
    return np.insert(np.cumsum(diffs), 0, 0.0)


def signed_curvature(points: List[RoutePoint]) -> np.ndarray:
    """离散估计路径有符号曲率 k（1/mm）。"""
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
    """
    对轨迹点执行离线速度规划并写入 target_speed。

    @note 规划流程：曲率限速包络 -> 停止点约束 -> 反向减速扫描 -> 正向加速扫描 -> 单位换算
    """
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
    """确定插值方法（命令行优先，否则交互/默认值）。"""
    if args.method in methods:
        return args.method

    if args.no_plot or not sys.stdin.isatty():
        return "4"

    print("\n" + "=" * 64)
    print("插值方法")
    print("[1] Cubic Spline")
    print("[2] PCHIP")
    print("[3] Akima")
    print("[4] Corner Fillet")
    print("[5] B-Spline")
    print("[6] Linear")
    print("=" * 64)

    choice = input("请选择方法 (1-6) [默认 4]: ").strip() or "4"
    if choice not in methods:
        print("输入无效，回退到 [4] Corner Fillet。")
        return "4"
    return choice


def main() -> int:
    """
    脚本主入口：读路表 -> 插值 -> 速度规划 -> 写回头文件。

    @return 0 成功，非 0 失败
    """
    args = parse_args()

    default_header = Path(__file__).resolve().parents[2] / "code" / "navigation" / "nav_replay_route_table.h"
    input_path = Path(args.input).resolve() if args.input else default_header
    output_path = Path(args.output).resolve() if args.output else input_path

    raw_points, start_heading_valid, start_heading_deg = read_route_header(str(input_path))
    if not raw_points:
        print("未从输入头文件读取到轨迹点。")
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

    print(f"已选择插值方法: {method_name}")
    print(f"已生成头文件: {output_path}")
    print(f"写入轨迹点数: {len(final_points)}")
    print(
        "速度规划默认参数: "
        f"vmax={PATH_SPEED_MAX_MM_S:.1f}mm/s, "
        f"a+={MAX_ACCEL_MM_S2:.1f}mm/s^2, "
        f"a-={MAX_DECEL_MM_S2:.1f}mm/s^2, "
        f"alat={MAX_LATERAL_ACCEL_MM_S2:.1f}mm/s^2"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
