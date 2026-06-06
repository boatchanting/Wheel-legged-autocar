#!/usr/bin/env python3
"""
自动轨迹解算脚本：根据稀疏的关键障碍物坐标（起点、掉头点、桩桶点），
利用刚体几何膨胀和 B 样条算法，自动解算并生成一条专供双轮平衡越野车行驶的、
曲率 C^2 连续的全局最优路径。

下游管线（曲率计算、速度规划、头文件生成）与 chazhi.py 保持完全一致。
"""

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
from scipy.interpolate import splprep, splev

plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False

# ============================================================
# 掉头点识别模式开关
# 0 = 掉头点为列表的第 1 个点（索引 1）
# 1 = 遍历所有点，找出距离起点直线距离最远的点，判定为掉头顶点
# ============================================================
U_TURN_DETECT_MODE = 0

# ============================================================
# 全局物理参数（与 chazhi.py 保持一致）
# ============================================================
INTERPOLATE_DIST = 50.0
PATH_SPEED_MAX_MM_S = 5000.0
MAX_ACCEL_MM_S2 = 2500.0
MAX_DECEL_MM_S2 = 1500.0
MAX_LATERAL_ACCEL_MM_S2 = 2500.0
SPEED_TO_MM_S = 4.936
CURVATURE_EPS = 1e-6
FLOAT_RE = r"[+-]?(?:\d+(?:\.\d*)?|\.\d+)"

# ============================================================
# 自动轨迹解算专属参数
# ============================================================
CAR_HALF_WIDTH_MM = 135.0      # 车身半宽
CONE_RADIUS_MM = 140.0         # 桩桶物理半径
SAFE_MARGIN_MM = 50.0          # 绕桩防撞安全余量（考虑平衡车侧倾和打滑冗余）
CONE_OFFSET_MM = CAR_HALF_WIDTH_MM + CONE_RADIUS_MM + SAFE_MARGIN_MM  # 桩桶横向偏置距离
U_TURN_RADIUS_MM = 1000.0      # 掉头弯的期望回转半径
U_TURN_CONTROL_POINTS = 24     # 掉头弯半圆控制点数量
B_SPLINE_SMOOTH_FACTOR = 0.5   # B 样条平滑因子（越小越逼近控制点）

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
    curvature: float = 0.0


# ============================================================
# 通用工具函数（与 chazhi.py 保持一致）
# ============================================================

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


# ============================================================
# CSV / 头文件 读写（与 chazhi.py 保持一致）
# ============================================================

def read_route_header(file_path: str) -> Tuple[List[RoutePoint], int, float]:
    """
    读取路表头文件，兼容 3/5/6/7 字段点格式。

    @return (轨迹点列表, 起跑航向有效标志, 起跑航向角)
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

    pattern_v7 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*,\s*({FLOAT_RE})f\s*\}}"
    )
    pattern_v6 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*\}}"
    )
    pattern_v5 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*\}}"
    )
    pattern_v3 = re.compile(
        rf"\{{\s*({FLOAT_RE})f\s*,\s*({FLOAT_RE})f\s*,\s*(?:\(uint8\))?\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*\}}"
    )

    for match in pattern_v7.finditer(body):
        points.append(
            RoutePoint(
                x=float(match.group(1)),
                y=float(match.group(2)),
                target_yaw_deg=normalize_relative_yaw_deg(float(match.group(3))),
                heading_deg=normalize_heading_deg(float(match.group(4))),
                target_speed=float(match.group(5)),
                point_type=parse_point_type(match.group(6)),
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


def read_csv_points(csv_path: str) -> List[RoutePoint]:
    """
    读取打点导出的 CSV 文件，返回稀疏轨迹点列表。

    @note CSV 格式：index, x, y, point_type, (可选) relative_yaw, heading
    """
    import csv as csv_mod

    if not os.path.exists(csv_path):
        raise FileNotFoundError(f"CSV 文件不存在：{csv_path}")

    points: List[RoutePoint] = []
    with open(csv_path, "r", encoding="utf-8-sig", newline="") as f:
        reader = csv_mod.DictReader(f)
        if reader.fieldnames is None:
            raise ValueError("CSV 缺少表头")

        # 规范化列名
        key_map = {}
        for k in reader.fieldnames:
            if k is not None:
                key_map[k.strip().lower().replace(" ", "")] = k

        for required in ("x", "y"):
            if required not in key_map:
                raise ValueError(f"CSV 缺少必需列：{required}")

        point_type_col = key_map.get("point_type")

        for row in reader:
            try:
                x = float(row[key_map["x"]])
                y = float(row[key_map["y"]])
                point_type = 0
                if point_type_col and row.get(point_type_col, "").strip():
                    point_type = int(float(row[point_type_col]))
                    point_type = max(0, min(5, point_type))

                points.append(
                    RoutePoint(
                        x=x,
                        y=y,
                        target_yaw_deg=None,
                        heading_deg=None,
                        target_speed=0.0,
                        point_type=point_type,
                    )
                )
            except Exception as exc:
                raise ValueError(f"CSV 行数据非法：{exc}") from exc

    infer_missing_angles(points)
    return points


# ============================================================
# 核心解算逻辑
# ============================================================

def classify_points(raw_points: List[RoutePoint]) -> Tuple[RoutePoint, RoutePoint, List[RoutePoint]]:
    """
    拓扑识别与点位分类。

    @param raw_points 按打点时间排序的稀疏坐标列表
    @return (起点, 掉头点, 绕桩点列表)
    """
    if len(raw_points) < 2:
        raise ValueError("至少需要 2 个点（起点 + 掉头点）")

    start = raw_points[0]

    if U_TURN_DETECT_MODE == 0:
        # 模式 0：掉头点为列表的第 1 个点
        u_turn = raw_points[1]
        cones = raw_points[2:]
    else:
        # 模式 1：遍历所有点，找出距离起点直线距离最远的点
        max_dist = -1.0
        u_turn_idx = 1
        for i in range(1, len(raw_points)):
            dx = raw_points[i].x - start.x
            dy = raw_points[i].y - start.y
            dist = math.sqrt(dx * dx + dy * dy)
            if dist > max_dist:
                max_dist = dist
                u_turn_idx = i
        u_turn = raw_points[u_turn_idx]
        cones = raw_points[u_turn_idx + 1:]

    return start, u_turn, cones


def compute_base_direction(u_turn: RoutePoint, cones: List[RoutePoint]) -> Tuple[float, float]:
    """
    计算返程的总体基准方向向量（从掉头点指向桩桶区终点）。

    @return (dx, dy) 单位方向向量
    """
    if not cones:
        # 没有桩桶点时，使用从起点到掉头点的方向反转
        return 0.0, 1.0

    last_cone = cones[-1]
    dx = last_cone.x - u_turn.x
    dy = last_cone.y - u_turn.y
    length = math.sqrt(dx * dx + dy * dy)
    if length < 1e-6:
        return 0.0, 1.0
    return dx / length, dy / length


def generate_start_straight(start: RoutePoint, u_turn: RoutePoint, n_points: int = 10) -> List[Tuple[float, float]]:
    """
    生成起步直道控制点：从起点到掉头点前一段距离。

    @return 控制点坐标列表 [(x, y), ...]
    """
    # 直道从起点延伸到掉头点附近（保留 U_TURN_RADIUS_MM 的距离给弯道）
    dx = u_turn.x - start.x
    dy = u_turn.y - start.y
    dist = math.sqrt(dx * dx + dy * dy)

    if dist < 1e-6:
        return [(start.x, start.y)]

    # 直道终点距离掉头点 U_TURN_RADIUS_MM 处
    straight_ratio = max(0.1, 1.0 - U_TURN_RADIUS_MM / dist) if dist > U_TURN_RADIUS_MM else 0.3
    end_x = start.x + dx * straight_ratio
    end_y = start.y + dy * straight_ratio

    points = []
    for i in range(n_points):
        t = i / max(n_points - 1, 1)
        px = start.x + (end_x - start.x) * t
        py = start.y + (end_y - start.y) * t
        points.append((px, py))

    return points


def generate_u_turn_arc(
    entry_point: Tuple[float, float],
    u_turn: RoutePoint,
    base_dx: float,
    base_dy: float,
    n_points: int = U_TURN_CONTROL_POINTS,
) -> List[Tuple[float, float]]:
    """
    生成掉头弯控制点：以掉头点为顶点，生成平滑的 U 型弧线。

    使用半圆弧实现，圆心在掉头点附近，半径为 U_TURN_RADIUS_MM。
    车子从直道进入，画一个平滑的 U 型弧线后，对准返程的桩桶区。

    @param entry_point 直道终点（进入弯道的起始点）
    @param u_turn 掉头点坐标
    @param base_dx, base_dy 返程基准方向
    @return 控制点坐标列表
    """
    ex, ey = entry_point

    # 入口方向（从起点指向掉头点）
    in_dx = u_turn.x - ex
    in_dy = u_turn.y - ey
    in_len = math.sqrt(in_dx * in_dx + in_dy * in_dy)
    if in_len < 1e-6:
        in_dx, in_dy = 1.0, 0.0
    else:
        in_dx /= in_len
        in_dy /= in_len

    # 出口方向（返程方向的反向，因为 U 型弯掉头）
    out_dx = -base_dx
    out_dy = -base_dy

    # U 型弯的圆心：在掉头点的法线方向偏移 R
    # 入口方向的法线（左转方向）
    normal_x = -in_dy
    normal_y = in_dx

    # 判断 U 型弯的转向方向：选择使圆心更靠近路径内侧的方向
    # 使用入口方向和出口方向的叉积判断
    cross = in_dx * out_dy - in_dy * out_dx
    if cross < 0:
        normal_x = in_dy
        normal_y = -in_dx

    # 圆心位置
    cx = u_turn.x + normal_x * U_TURN_RADIUS_MM
    cy = u_turn.y + normal_y * U_TURN_RADIUS_MM

    # 计算入口和出口角度
    start_angle = math.atan2(ey - cy, ex - cx)
    # 出口角度：从圆心指向返程方向
    end_angle = math.atan2(-base_dy, -base_dx)

    # 确保弧线是 U 型（约 180 度）
    # 调整 end_angle 使得从 start_angle 到 end_angle 的弧度接近 pi
    while end_angle - start_angle > math.pi:
        end_angle -= 2.0 * math.pi
    while end_angle - start_angle < -math.pi:
        end_angle += 2.0 * math.pi

    # 如果弧度太小（不是 U 型），强制设为半圆
    if abs(end_angle - start_angle) < math.pi * 0.5:
        # 用叉积确定旋转方向
        if cross >= 0:
            end_angle = start_angle + math.pi
        else:
            end_angle = start_angle - math.pi

    points = []
    for i in range(n_points):
        t = i / max(n_points - 1, 1)
        angle = start_angle + (end_angle - start_angle) * t
        px = cx + U_TURN_RADIUS_MM * math.cos(angle)
        py = cy + U_TURN_RADIUS_MM * math.sin(angle)
        points.append((px, py))

    return points


def generate_slalom_apex_points(
    cones: List[RoutePoint],
    u_turn: RoutePoint,
    base_dx: float,
    base_dy: float,
) -> List[Tuple[float, float]]:
    """
    绕桩走线生成 (Apex Offset)。

    对于每个桩桶点，沿法向量向外平移 CONE_OFFSET_MM 距离，生成期望通车点（Apex 点）。
    平移方向交替进行（第一个桩向左偏，第二个向右偏……），形成 S 型走线引导点。

    @return Apex 控制点坐标列表
    """
    if not cones:
        return []

    # 法向量（垂直于基准方向）
    normal_x = -base_dy
    normal_y = base_dx

    apex_points = []
    for i, cone in enumerate(cones):
        # 交替偏置方向
        sign = 1.0 if (i % 2 == 0) else -1.0
        offset_x = cone.x + normal_x * CONE_OFFSET_MM * sign
        offset_y = cone.y + normal_y * CONE_OFFSET_MM * sign
        apex_points.append((offset_x, offset_y))

    return apex_points


def generate_control_points(
    start: RoutePoint,
    u_turn: RoutePoint,
    cones: List[RoutePoint],
) -> Tuple[List[Tuple[float, float]], List[Tuple[float, float]]]:
    """
    生成用于 B 样条拟合的全部控制点。

    @return (全部控制点, 仅 Apex 控制点用于可视化)
    """
    # 计算返程基准方向
    base_dx, base_dy = compute_base_direction(u_turn, cones)

    # 1. 起步直道控制点
    straight_pts = generate_start_straight(start, u_turn, n_points=10)

    # 2. 掉头弯控制点
    entry_point = straight_pts[-1] if straight_pts else (start.x, start.y)
    u_turn_pts = generate_u_turn_arc(entry_point, u_turn, base_dx, base_dy)

    # 3. 绕桩 Apex 控制点
    apex_pts = generate_slalom_apex_points(cones, u_turn, base_dx, base_dy)

    # 合并所有控制点（去重：避免直道终点和弯道起点重合）
    all_control = []
    all_control.extend(straight_pts)
    # 跳过 U 型弯第一个点（与直道终点可能重合）
    if u_turn_pts and straight_pts:
        dx = u_turn_pts[0][0] - straight_pts[-1][0]
        dy = u_turn_pts[0][1] - straight_pts[-1][1]
        if math.sqrt(dx * dx + dy * dy) > 1.0:
            pass  # 不重合，保留
        else:
            u_turn_pts = u_turn_pts[1:]
    all_control.extend(u_turn_pts)
    all_control.extend(apex_pts)

    return all_control, apex_pts


def bspline_smooth(control_points: List[Tuple[float, float]]) -> Tuple[np.ndarray, np.ndarray]:
    """
    使用三次 B 样条对控制点进行平滑拟合，并等距重采样。

    @return (x_fine, y_fine) 密集轨迹坐标数组
    """
    if len(control_points) < 4:
        # 控制点不足，直接返回
        x = np.array([p[0] for p in control_points])
        y = np.array([p[1] for p in control_points])
        return x, y

    x_ctrl = np.array([p[0] for p in control_points])
    y_ctrl = np.array([p[1] for p in control_points])

    # 三次 B 样条拟合
    k = min(3, len(control_points) - 1)
    tck, _ = splprep([x_ctrl, y_ctrl], s=B_SPLINE_SMOOTH_FACTOR, k=k)

    # 密集采样
    u_fine = np.linspace(0.0, 1.0, 5000)
    x_spline, y_spline = splev(u_fine, tck)

    return np.array(x_spline), np.array(y_spline)


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


def generate_calculated_path(raw_points: List[RoutePoint]) -> Tuple[List[Tuple[float, float]], np.ndarray, np.ndarray]:
    """
    核心解算函数：从稀疏关键点生成平滑轨迹。

    @param raw_points 稀疏关键点列表（起点、掉头点、桩桶点）
    @return (控制点列表, 重采样后 x 数组, 重采样后 y 数组)
    """
    # 1. 拓扑识别与点位分类
    start, u_turn, cones = classify_points(raw_points)

    print(f"[分类] 起点: ({start.x:.1f}, {start.y:.1f})")
    print(f"[分类] 掉头点: ({u_turn.x:.1f}, {u_turn.y:.1f})")
    print(f"[分类] 桩桶数量: {len(cones)}")
    for i, c in enumerate(cones):
        print(f"  桩桶 {i + 1}: ({c.x:.1f}, {c.y:.1f})")

    # 2. 控制点生成
    control_points, apex_pts = generate_control_points(start, u_turn, cones)
    print(f"[控制点] 总数: {len(control_points)}")

    # 3. B 样条平滑
    x_fine, y_fine = bspline_smooth(control_points)

    # 4. 等距重采样
    x_resampled, y_resampled = resample_path(x_fine, y_fine, INTERPOLATE_DIST)
    print(f"[轨迹] 重采样后点数: {len(x_resampled)}")

    return control_points, x_resampled, y_resampled


# ============================================================
# 下游管线（与 chazhi.py 保持完全一致）
# ============================================================

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
        points[i].curvature = float(kappa)

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


def build_final_points(
    raw_points: List[RoutePoint],
    sel_x: np.ndarray,
    sel_y: np.ndarray,
) -> List[RoutePoint]:
    """
    构建最终轨迹点序列，并回填特殊点类型与姿态信息。

    @note 对于桩桶点附近的最近轨迹点，继承原稀疏点的 point_type
    """
    final_x = np.array(sel_x, dtype=float)
    final_y = np.array(sel_y, dtype=float)
    final_type = np.zeros(len(final_x), dtype=int)
    final_heading = np.zeros(len(final_x), dtype=float)

    # 回填特殊点类型：在最终轨迹中找到离原始特殊点最近的点
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

    return [
        RoutePoint(
            x=float(final_x[i]),
            y=float(final_y[i]),
            target_yaw_deg=normalize_relative_yaw_deg(float(final_yaw[i])),
            heading_deg=normalize_heading_deg(float(final_heading[i])),
            target_speed=0.0,
            point_type=int(final_type[i]),
        )
        for i in range(len(final_x))
    ]


def generate_header(
    points: List[RoutePoint],
    method_name: str,
    output_path: str,
    start_heading_valid: int,
    start_heading_deg: float,
) -> None:
    """
    生成 7 字段导航路表头文件。

    @note 与 chazhi.py 的 generate_header 格式完全一致
    """
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("#ifndef _NAV_REPLAY_ROUTE_TABLE_H_\n")
        f.write("#define _NAV_REPLAY_ROUTE_TABLE_H_\n\n")
        f.write('#include "nav_ram.h"\n\n')
        f.write("// 由 tools/webview_nav_marker速度规划/calculate_path.py 自动生成\n")
        f.write(f"// 生成时间：{timestamp}\n")
        f.write(f"// 生成方法：{method_name}\n")
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
                    f"{p.heading_deg:.3f}f, (uint8){p.point_type}, {p.target_speed:.3f}f, {p.curvature:.6f}f}},\n"
                )
        else:
            f.write("    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, NAV_POINT_PATH, 0.0f},\n")
        f.write("};\n\n")
        f.write("#endif // _NAV_REPLAY_ROUTE_TABLE_H_\n")


# ============================================================
# 可视化
# ============================================================

def plot_result(
    raw_points: List[RoutePoint],
    control_points: List[Tuple[float, float]],
    final_points: List[RoutePoint],
) -> None:
    """
    绘制结果预览：原始打点（桩桶）、生成的虚拟控制点、最终 B 样条平滑轨迹。
    """
    fig, ax = plt.subplots(1, 1, figsize=(14, 10))
    fig.canvas.manager.set_window_title("自动轨迹解算预览")

    # 原始打点
    raw_x = [p.x for p in raw_points]
    raw_y = [p.y for p in raw_points]
    ax.plot(raw_x, raw_y, "ro", markersize=10, label="原始打点", zorder=5, alpha=0.7)

    # 标注原始点类型
    for i, p in enumerate(raw_points):
        label = f"P{i}"
        if p.point_type != 0:
            type_name, _, _ = SPECIAL_POINTS_MAP.get(p.point_type, ("特殊点", "black", "X"))
            label = f"P{i}({type_name})"
        ax.annotate(label, (p.x, p.y), textcoords="offset points",
                    xytext=(8, 8), fontsize=9, color="red", fontweight="bold")

    # 控制点
    if control_points:
        ctrl_x = [p[0] for p in control_points]
        ctrl_y = [p[1] for p in control_points]
        ax.plot(ctrl_x, ctrl_y, "g+", markersize=6, label="虚拟控制点", zorder=4, alpha=0.5)

    # 最终轨迹
    final_x = [p.x for p in final_points]
    final_y = [p.y for p in final_points]
    ax.plot(final_x, final_y, "b-", linewidth=1.5, label="B 样条平滑轨迹", zorder=3)

    # 标注特殊点
    for p in final_points:
        if p.point_type != 0:
            type_name, color, marker = SPECIAL_POINTS_MAP.get(p.point_type, ("特殊点", "black", "X"))
            ax.scatter([p.x], [p.y], c=color, marker=marker, s=120,
                       edgecolors="black", label=type_name, zorder=6)

    # 绘制桩桶安全区域（可视化防撞余量）
    cones = [p for p in raw_points if p.point_type == 0 and p != raw_points[0]]
    # 掉头点之后的普通点视为桩桶
    if len(raw_points) >= 3:
        u_turn_idx = 1 if U_TURN_DETECT_MODE == 0 else None
        if u_turn_idx is not None:
            cone_points = raw_points[u_turn_idx + 1:]
        else:
            # 找最远点
            start = raw_points[0]
            max_dist = -1.0
            max_idx = 1
            for i in range(1, len(raw_points)):
                d = math.sqrt((raw_points[i].x - start.x) ** 2 + (raw_points[i].y - start.y) ** 2)
                if d > max_dist:
                    max_dist = d
                    max_idx = i
            cone_points = raw_points[max_idx + 1:]

        for cp in cone_points:
            circle = plt.Circle(
                (cp.x, cp.y), CONE_RADIUS_MM + SAFE_MARGIN_MM,
                color="orange", alpha=0.2, linestyle="--", linewidth=1
            )
            ax.add_patch(circle)

    ax.set_xlabel("X (mm)")
    ax.set_ylabel("Y (mm)")
    ax.set_title("自动轨迹解算结果")
    ax.axis("equal")
    ax.grid(True, linestyle="--", alpha=0.6)
    ax.legend(fontsize=10, loc="best")

    plt.tight_layout()
    plt.show()


def plot_speed_profile(final_points: List[RoutePoint]) -> None:
    """绘制速度规划曲线。"""
    if not final_points:
        return

    s_vals = cumulative_arc_length(final_points)
    speeds = [-p.target_speed * SPEED_TO_MM_S for p in final_points]  # 转换回 mm/s

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), sharex=True)
    fig.canvas.manager.set_window_title("速度规划预览")

    ax1.plot(s_vals, speeds, "b-", linewidth=1.5)
    ax1.set_ylabel("目标速度 (mm/s)")
    ax1.set_title("纵向速度规划")
    ax1.grid(True, linestyle="--", alpha=0.6)
    ax1.axhline(y=PATH_SPEED_MAX_MM_S, color="r", linestyle="--", alpha=0.5, label=f"vmax={PATH_SPEED_MAX_MM_S}")
    ax1.legend()

    curvatures = [p.curvature for p in final_points]
    ax2.plot(s_vals, curvatures, "g-", linewidth=1.0)
    ax2.set_ylabel("曲率 (1/mm)")
    ax2.set_xlabel("弧长 (mm)")
    ax2.set_title("路径曲率")
    ax2.grid(True, linestyle="--", alpha=0.6)

    plt.tight_layout()
    plt.show()


# ============================================================
# 命令行入口
# ============================================================

def parse_args() -> argparse.Namespace:
    """解析命令行参数。"""
    parser = argparse.ArgumentParser(description="自动轨迹解算：稀疏关键点 -> B 样条平滑轨迹 + 速度规划")
    parser.add_argument(
        "input",
        nargs="?",
        help="输入 CSV 文件路径或头文件路径（默认：自动查找最新 CSV）",
    )
    parser.add_argument(
        "--output",
        help="输出头文件路径（默认：code/navigation/nav_replay_route_table.h）",
    )
    parser.add_argument(
        "--no-plot",
        action="store_true",
        help="跳过预览窗口。",
    )
    parser.add_argument(
        "--u-turn-mode",
        choices=["0", "1"],
        help="掉头点识别模式：0=第1个点，1=最远点（覆盖脚本内 U_TURN_DETECT_MODE）",
    )
    return parser.parse_args()


def auto_find_latest_csv(script_dir: Path) -> Path:
    """自动查找脚本目录下最新的打点 CSV 文件。"""
    candidates = sorted(
        script_dir.glob("nav_mark_points_*.csv"),
        key=lambda p: p.stat().st_mtime,
        reverse=True,
    )
    if not candidates:
        raise FileNotFoundError("未找到 nav_mark_points_*.csv，请显式传入 CSV 路径。")
    return candidates[0]


def main() -> int:
    """
    脚本主入口：读取稀疏关键点 -> 自动解算轨迹 -> 速度规划 -> 生成头文件。

    @return 0 成功，非 0 失败
    """
    global U_TURN_DETECT_MODE

    args = parse_args()

    # 覆盖掉头点识别模式
    if args.u_turn_mode is not None:
        U_TURN_DETECT_MODE = int(args.u_turn_mode)

    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent.parent

    # 确定输入文件
    if args.input:
        input_path = Path(args.input).resolve()
    else:
        input_path = auto_find_latest_csv(script_dir)

    # 确定输出路径
    default_output = project_root / "code" / "navigation" / "nav_replay_route_table.h"
    output_path = Path(args.output).resolve() if args.output else default_output

    # 读取输入
    input_str = str(input_path)
    if input_str.endswith(".csv"):
        raw_points = read_csv_points(input_str)
        start_heading_valid = 0
        start_heading_deg = 0.0
    else:
        raw_points, start_heading_valid, start_heading_deg = read_route_header(input_str)

    if not raw_points:
        print("未读取到轨迹点。")
        return 1

    print(f"[输入] 文件: {input_path}")
    print(f"[输入] 原始点数: {len(raw_points)}")
    print(f"[参数] 车身半宽: {CAR_HALF_WIDTH_MM}mm")
    print(f"[参数] 桩桶半径: {CONE_RADIUS_MM}mm")
    print(f"[参数] 安全余量: {SAFE_MARGIN_MM}mm")
    print(f"[参数] 桩桶偏置: {CONE_OFFSET_MM}mm")
    print(f"[参数] 掉头半径: {U_TURN_RADIUS_MM}mm")
    print(f"[参数] 掉头模式: {U_TURN_DETECT_MODE}")
    print()

    # 核心解算
    control_points, x_resampled, y_resampled = generate_calculated_path(raw_points)

    # 构建最终点序列
    final_points = build_final_points(raw_points, x_resampled, y_resampled)

    # 离线速度规划
    apply_speed_plan(final_points)

    # 生成头文件
    generate_header(
        final_points,
        "AutoPath B-Spline",
        str(output_path),
        start_heading_valid,
        start_heading_deg,
    )

    print()
    print(f"[输出] 头文件: {output_path}")
    print(f"[输出] 轨迹点数: {len(final_points)}")
    print(
        "[输出] 速度规划参数: "
        f"vmax={PATH_SPEED_MAX_MM_S:.1f}mm/s, "
        f"a+={MAX_ACCEL_MM_S2:.1f}mm/s^2, "
        f"a-={MAX_DECEL_MM_S2:.1f}mm/s^2, "
        f"alat={MAX_LATERAL_ACCEL_MM_S2:.1f}mm/s^2"
    )

    # 可视化
    if not args.no_plot:
        plot_result(raw_points, control_points, final_points)
        plot_speed_profile(final_points)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
