#!/usr/bin/env python3
"""将科目二打点导出的 CSV 转换为 C 路表头文件（6 字段，速度先占位 0）。"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import math
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional, Tuple

MAX_POINTS_DEFAULT = 500


@dataclass
class RoutePoint:
    index: int
    x: float
    y: float
    point_type: int
    target_yaw_deg: Optional[float]
    heading_deg: Optional[float]
    target_speed: float = 0.0
    curvature: float = 0.0


def parse_args() -> argparse.Namespace:
    """解析命令行参数。"""
    parser = argparse.ArgumentParser(description="将导航打点 CSV 转为 C 路表头文件")
    parser.add_argument("csv", nargs="?", help="导出的 CSV 文件路径")
    parser.add_argument(
        "--output",
        help="输出头文件路径（默认：code/navigation/nav_replay_route_table.h）",
    )
    parser.add_argument(
        "--max-points",
        type=int,
        default=MAX_POINTS_DEFAULT,
        help=f"最多保留的轨迹点数（默认：{MAX_POINTS_DEFAULT}）",
    )
    return parser.parse_args()


def normalize_key(key: str) -> str:
    """规范化 CSV 列名，便于兼容大小写和空格差异。"""
    return key.strip().lower().replace(" ", "")


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


def normalize_heading_deg(value: object) -> Optional[float]:
    """将绝对航向角归一化到 [0, 360)。"""
    try:
        heading = float(value)
    except (TypeError, ValueError):
        return None
    if not math.isfinite(heading):
        return None
    heading = math.fmod(heading, 360.0)
    if heading < 0.0:
        heading += 360.0
    return heading


def normalize_relative_yaw_deg(value: object) -> Optional[float]:
    """将相对航向角归一化到 (-180, 180]。"""
    try:
        yaw = float(value)
    except (TypeError, ValueError):
        return None
    if not math.isfinite(yaw):
        return None
    while yaw > 180.0:
        yaw -= 360.0
    while yaw <= -180.0:
        yaw += 360.0
    return yaw


def calc_path_yaw_deg(x0: float, y0: float, x1: float, y1: float) -> float:
    """按项目坐标系计算路径切向角（deg）。"""
    return -math.degrees(math.atan2(y1 - y0, -(x1 - x0)))


def infer_target_yaws(points: List[RoutePoint]) -> None:
    """在 CSV 缺失 target_yaw 时，用相邻点几何关系补齐。"""
    count = len(points)
    for idx, point in enumerate(points):
        if point.target_yaw_deg is not None:
            continue

        yaw = None
        if idx + 1 < count:
            nxt = points[idx + 1]
            if not math.isclose(point.x, nxt.x) or not math.isclose(point.y, nxt.y):
                yaw = calc_path_yaw_deg(point.x, point.y, nxt.x, nxt.y)
        if yaw is None and idx > 0:
            prev = points[idx - 1]
            if not math.isclose(prev.x, point.x) or not math.isclose(prev.y, point.y):
                yaw = calc_path_yaw_deg(prev.x, prev.y, point.x, point.y)

        point.target_yaw_deg = normalize_relative_yaw_deg(0.0 if yaw is None else yaw)


def fill_missing_heading(points: List[RoutePoint]) -> None:
    """补齐缺失 heading，默认回填 0。"""
    for point in points:
        if point.heading_deg is None:
            point.heading_deg = 0.0


def read_points(csv_path: Path) -> Tuple[List[RoutePoint], Optional[float]]:
    """
    读取 CSV 并转换为轨迹点列表。

    @return (轨迹点列表, 起跑航向角或 None)
    @note 调用位置：main() 主流程入口
    """
    with csv_path.open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None:
            raise ValueError("CSV 缺少表头")

        key_map = {normalize_key(k): k for k in reader.fieldnames if k is not None}
        for required in ("x", "y", "point_type"):
            if required not in key_map:
                raise ValueError(f"缺少必需列：{required}")

        idx_col = key_map.get("index")
        start_heading_col = key_map.get("start_heading")
        yaw_col = key_map.get("relative_yaw") or key_map.get("target_yaw_deg")
        heading_col = key_map.get("heading")

        points: List[RoutePoint] = []
        start_heading: Optional[float] = None

        for order, row in enumerate(reader):
            try:
                if start_heading_col and start_heading is None:
                    raw_start_heading = row.get(start_heading_col, "").strip()
                    if raw_start_heading:
                        start_heading = normalize_heading_deg(raw_start_heading)

                idx = int(row[idx_col]) if idx_col and row.get(idx_col, "") != "" else order
                x = float(row[key_map["x"]])
                y = float(row[key_map["y"]])
                point_type = int(float(row[key_map["point_type"]]))
                point_type = max(0, min(5, point_type))

                target_yaw_deg = None
                if yaw_col and row.get(yaw_col, "").strip() != "":
                    target_yaw_deg = normalize_relative_yaw_deg(row[yaw_col])

                heading_deg = None
                if heading_col and row.get(heading_col, "").strip() != "":
                    heading_deg = normalize_heading_deg(row[heading_col])

                points.append(
                    RoutePoint(
                        index=idx,
                        x=x,
                        y=y,
                        point_type=point_type,
                        target_yaw_deg=target_yaw_deg,
                        heading_deg=heading_deg,
                    )
                )
            except Exception as exc:
                raise ValueError(f"第 {order + 1} 行数据非法：{exc}") from exc

    points.sort(key=lambda item: item.index)
    infer_target_yaws(points)
    fill_missing_heading(points)
    return points, start_heading


def format_header(points: List[RoutePoint], src_path: Path, start_heading: Optional[float]) -> str:
    """
    生成 C 头文件文本（6 字段 NavRamPoint_t 格式）。

    @note 调用位置：main() 在读取并裁剪点数据后调用
    """
    now = dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    count = len(points)
    # heading_valid = 1 if start_heading is not None else 0
    heading_valid = 0 #默认设置不使用heading校准
    heading_deg = 0.0 if start_heading is None else start_heading

    lines = [
        "#ifndef _NAV_REPLAY_ROUTE_TABLE_H_",
        "#define _NAV_REPLAY_ROUTE_TABLE_H_",
        "",
        "#include \"nav_ram.h\"",
        "",
        "// 由 tools/webview_nav_marker速度规划_科目二/csv_to_nav_table.py 自动生成",
        f"// 源 CSV：{src_path.as_posix()}",
        f"// 生成时间：{now}",
        "",
        f"#define NAV_REPLAY_START_HEADING_VALID {heading_valid}",
        f"#define NAV_REPLAY_START_HEADING_DEG {heading_deg:.3f}f",
        "",
        f"#define NAV_REPLAY_STATIC_ROUTE_COUNT {count}",
        "",
    ]

    arr_size = count if count > 0 else 1
    lines.append(f"static const NavRamPoint_t nav_replay_static_route_points[{arr_size}] = {{")

    if count == 0:
        lines.append("    {0.0f, 0.0f, 0.0f, 0.0f, NAV_POINT_PATH, 0.0f, 0.0f},")
    else:
        for point in points:
            lines.append(
                "    "
                f"{{{point.x:.3f}f, {point.y:.3f}f, {point.target_yaw_deg:.3f}f, "
                f"{point.heading_deg:.3f}f, (uint8){point.point_type}, {point.target_speed:.3f}f, {point.curvature:.6f}f}},"
            )

    lines.extend(
        [
            "};",
            "",
            "#endif // _NAV_REPLAY_ROUTE_TABLE_H_",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    """
    脚本主入口：读取 CSV -> 构建头文件文本 -> 写入目标路径。

    @return 0 成功，非 0 失败
    """
    args = parse_args()

    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent.parent

    csv_path = Path(args.csv).resolve() if args.csv else auto_find_latest_csv(script_dir)
    out_path = (
        Path(args.output).resolve()
        if args.output
        else (project_root / "code" / "navigation" / "nav_replay_route_table.h")
    )

    if not csv_path.exists():
        raise FileNotFoundError(f"CSV 文件不存在：{csv_path}")
    if args.max_points <= 0:
        raise ValueError("--max-points 必须大于 0")

    points, start_heading = read_points(csv_path)
    truncated = False
    if len(points) > args.max_points:
        points = points[: args.max_points]
        truncated = True

    out_text = format_header(points, csv_path, start_heading)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(out_text, encoding="utf-8")

    print(f"[OK] CSV：{csv_path}")
    print(f"[OK] 头文件输出：{out_path}")
    print(f"[OK] 轨迹点数量：{len(points)}")
    if start_heading is None:
        print("[OK] 起跑航向：未提供")
    else:
        print(f"[OK] 起跑航向：{start_heading:.3f} deg")
    if truncated:
        print(f"[WARN] Truncated to max points: {args.max_points}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
