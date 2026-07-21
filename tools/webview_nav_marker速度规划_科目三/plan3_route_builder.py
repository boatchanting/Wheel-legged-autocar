#!/usr/bin/env python3
"""科目三锚点补线路径工具：保留人工点，在相邻锚点之间自动补普通路径点。"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import math
from dataclasses import dataclass, replace
from pathlib import Path
from typing import Iterable, List, Optional, Sequence


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[1]

# ========================= 科目三补点关键调参区 =========================
# 补点间距（mm）：当前方法一是逐点“先转再走”。数值过小会增加寻点次数，
# 数值过大则会让急转弯的折线过于生硬。建议先在 200~300mm 范围内试车。
DEFAULT_FILL_SPACING_MM = 250.0

# 两个锚点距离小于该值时不插入自动点，防止手工重复点导致路线出现零长度段。
ZERO_LENGTH_EPS_MM = 1.0

# 单段补点数量保护：超过该数量通常意味着锚点遗漏或补点间距设置过小。
MAX_AUTO_POINTS_PER_SEGMENT = 1000

NAV_POINT_PATH = 0


@dataclass(frozen=True)
class RoutePoint:
    """与 NavRamPoint_t 对应的上位机路线点；source 只用于 CSV 回查。"""

    x: float
    y: float
    point_type: int
    target_yaw_deg: float
    heading_deg: float
    source: str = "manual"


def _finite_float(value: object, field_name: str) -> float:
    try:
        result = float(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{field_name} 不是有效数字: {value!r}") from exc
    if not math.isfinite(result):
        raise ValueError(f"{field_name} 必须是有限数字: {value!r}")
    return result


def _point_type(value: object) -> int:
    try:
        point_type = int(float(value))
    except (TypeError, ValueError) as exc:
        raise ValueError(f"point_type 不合法: {value!r}") from exc
    if point_type < 0 or point_type > 5:
        raise ValueError(f"point_type 必须在 0~5 之间，当前为 {point_type}")
    return point_type


def _normalize_csv_key(key: str) -> str:
    return key.strip().lower().replace(" ", "").replace("_", "")


def _read_value(row: dict, aliases: Sequence[str], default: Optional[object] = None) -> object:
    normalized = {_normalize_csv_key(str(key)): value for key, value in row.items()}
    for alias in aliases:
        value = normalized.get(_normalize_csv_key(alias))
        if value not in (None, ""):
            return value
    if default is not None:
        return default
    raise ValueError(f"CSV 缺少字段，支持字段名：{', '.join(aliases)}")


def read_anchor_csv(csv_path: Path) -> List[RoutePoint]:
    """读取上位机导出的锚点 CSV，保持用户的打点顺序。"""

    with csv_path.open("r", newline="", encoding="utf-8-sig") as file_obj:
        rows = list(csv.DictReader(file_obj))

    if not rows:
        raise ValueError("锚点 CSV 没有任何数据")

    anchors: List[RoutePoint] = []
    for row_index, row in enumerate(rows, start=2):
        try:
            anchors.append(
                RoutePoint(
                    x=_finite_float(_read_value(row, ("x",)), "x"),
                    y=_finite_float(_read_value(row, ("y",)), "y"),
                    point_type=_point_type(_read_value(row, ("point_type", "type"), NAV_POINT_PATH)),
                    target_yaw_deg=_finite_float(
                        _read_value(row, ("target_yaw_deg", "targetYaw", "target_yaw", "relative_yaw"), 0.0),
                        "target_yaw_deg",
                    ),
                    heading_deg=_finite_float(_read_value(row, ("heading_deg", "heading"), 0.0), "heading_deg"),
                    source="manual",
                )
            )
        except ValueError as exc:
            raise ValueError(f"CSV 第 {row_index} 行错误：{exc}") from exc

    return anchors


def read_start_heading(csv_path: Path) -> Optional[float]:
    """读取上位机写入的起跑绝对航向，供生成的路表继续使用。"""

    with csv_path.open("r", newline="", encoding="utf-8-sig") as file_obj:
        reader = csv.DictReader(file_obj)
        first_row = next(reader, None)

    if not first_row:
        return None

    normalized = {_normalize_csv_key(str(key)): value for key, value in first_row.items()}
    raw_heading = normalized.get(_normalize_csv_key("start_heading"))
    if raw_heading in (None, ""):
        return None
    return _finite_float(raw_heading, "start_heading")


def _auto_path_point(start: RoutePoint, end: RoutePoint, ratio: float) -> RoutePoint:
    """生成普通路径点；特殊点的类型与对角信息只保留在人工锚点上。"""

    return RoutePoint(
        x=start.x + (end.x - start.x) * ratio,
        y=start.y + (end.y - start.y) * ratio,
        point_type=NAV_POINT_PATH,
        target_yaw_deg=start.target_yaw_deg,
        heading_deg=start.heading_deg,
        source="auto_fill",
    )


def build_route_from_anchors(
    anchors: Iterable[RoutePoint],
    fill_spacing_mm: float = DEFAULT_FILL_SPACING_MM,
) -> List[RoutePoint]:
    """在相邻人工锚点之间补普通点，不平滑、不改写特殊锚点。"""

    anchor_list = list(anchors)
    if len(anchor_list) < 2:
        raise ValueError("科目三路线至少需要两个锚点")

    spacing = _finite_float(fill_spacing_mm, "fill_spacing_mm")
    if spacing <= ZERO_LENGTH_EPS_MM:
        raise ValueError(f"补点间距必须大于 {ZERO_LENGTH_EPS_MM}mm")

    route: List[RoutePoint] = [replace(anchor_list[0], source="manual")]
    for end_anchor in anchor_list[1:]:
        start_anchor = route[-1]
        dx = end_anchor.x - start_anchor.x
        dy = end_anchor.y - start_anchor.y
        distance = math.hypot(dx, dy)

        if distance <= ZERO_LENGTH_EPS_MM:
            # 同一坐标处优先保留后一个特殊锚点，避免普通点挡住元素触发点。
            if end_anchor.point_type != NAV_POINT_PATH:
                route[-1] = replace(end_anchor, source="manual")
            continue

        auto_count = int(math.floor((distance - ZERO_LENGTH_EPS_MM) / spacing))
        if auto_count > MAX_AUTO_POINTS_PER_SEGMENT:
            raise ValueError(
                f"锚点段 {len(route) - 1} 长度 {distance:.1f}mm 将生成 {auto_count} 个自动点，"
                "请增大补点间距或拆分检查路线。"
            )

        for point_index in range(1, auto_count + 1):
            offset_mm = point_index * spacing
            if offset_mm >= distance - ZERO_LENGTH_EPS_MM:
                break
            route.append(_auto_path_point(start_anchor, end_anchor, offset_mm / distance))

        route.append(replace(end_anchor, source="manual"))

    return route


def write_route_csv(route: Sequence[RoutePoint], output_path: Path) -> Path:
    """输出补点完成的 CSV，供复查或继续转为 C 路表。"""

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", newline="", encoding="utf-8-sig") as file_obj:
        writer = csv.DictWriter(
            file_obj,
            fieldnames=("index", "x", "y", "point_type", "target_yaw_deg", "heading_deg", "source"),
        )
        writer.writeheader()
        for index, point in enumerate(route):
            writer.writerow(
                {
                    "index": index,
                    "x": f"{point.x:.3f}",
                    "y": f"{point.y:.3f}",
                    "point_type": point.point_type,
                    "target_yaw_deg": f"{point.target_yaw_deg:.3f}",
                    "heading_deg": f"{point.heading_deg:.3f}",
                    "source": point.source,
                }
            )
    return output_path


def generate_route_artifacts(
    anchor_csv_path: Path,
    fill_spacing_mm: float = DEFAULT_FILL_SPACING_MM,
    output_csv_path: Optional[Path] = None,
    header_path: Optional[Path] = None,
) -> dict:
    """从锚点 CSV 生成完成路线 CSV 与 C 路表头文件。"""

    from csv_to_nav_table import format_header, read_points

    anchors = read_anchor_csv(anchor_csv_path)
    start_heading = read_start_heading(anchor_csv_path)
    route = build_route_from_anchors(anchors, fill_spacing_mm)
    timestamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    completed_csv = output_csv_path or SCRIPT_DIR / f"plan3_completed_route_{timestamp}.csv"
    write_route_csv(route, completed_csv)

    generated_points, _ = read_points(completed_csv)
    output_header = header_path or REPO_ROOT / "code" / "navigation" / "nav_replay_route_table.h"
    output_header.parent.mkdir(parents=True, exist_ok=True)
    output_header.write_text(format_header(generated_points, completed_csv, start_heading), encoding="utf-8")

    return {
        "anchor_count": len(anchors),
        "route_count": len(route),
        "completed_csv": str(completed_csv),
        "header_path": str(output_header),
        # 上位机预览直接使用与 CSV、C 路表同一批补点结果，避免再次解析文件产生不一致。
        "route_points": [
            {
                "index": index,
                "x": point.x,
                "y": point.y,
                "point_type": point.point_type,
                "target_yaw_deg": point.target_yaw_deg,
                "heading_deg": point.heading_deg,
                "source": point.source,
            }
            for index, point in enumerate(route)
        ],
    }


def _find_latest_anchor_csv() -> Path:
    candidates = sorted(SCRIPT_DIR.glob("nav_mark_points_*.csv"), key=lambda path: path.stat().st_mtime, reverse=True)
    if not candidates:
        raise FileNotFoundError("未找到 nav_mark_points_*.csv，请先从上位机导出锚点。")
    return candidates[0]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="科目三锚点补线并生成 C 路表")
    parser.add_argument("csv", nargs="?", type=Path, help="上位机导出的锚点 CSV；不填则使用目录中最新文件")
    parser.add_argument(
        "--spacing-mm",
        type=float,
        default=DEFAULT_FILL_SPACING_MM,
        help=f"自动补点间距（mm，默认 {DEFAULT_FILL_SPACING_MM:g}）",
    )
    parser.add_argument("--output-csv", type=Path, help="完成路线 CSV 输出路径")
    parser.add_argument("--header", type=Path, help="C 路表头文件输出路径")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    anchor_csv = args.csv or _find_latest_anchor_csv()
    result = generate_route_artifacts(anchor_csv, args.spacing_mm, args.output_csv, args.header)
    print(f"已读取锚点：{result['anchor_count']} 个")
    print(f"已生成路线：{result['route_count']} 点")
    print(f"完成路线 CSV：{result['completed_csv']}")
    print(f"C 路表头文件：{result['header_path']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
