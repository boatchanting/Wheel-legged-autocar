#!/usr/bin/env python3
"""Convert exported nav marker CSV to a C route table header."""

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


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert nav marker CSV to C header")
    parser.add_argument("csv", nargs="?", help="Path to exported CSV")
    parser.add_argument(
        "--output",
        help="Output header path (default: code/navigation/nav_replay_route_table.h)",
    )
    parser.add_argument(
        "--max-points",
        type=int,
        default=MAX_POINTS_DEFAULT,
        help=f"Maximum route points to keep (default: {MAX_POINTS_DEFAULT})",
    )
    return parser.parse_args()


def normalize_key(key: str) -> str:
    return key.strip().lower().replace(" ", "")


def auto_find_latest_csv(script_dir: Path) -> Path:
    candidates = sorted(
        script_dir.glob("nav_mark_points_*.csv"),
        key=lambda p: p.stat().st_mtime,
        reverse=True,
    )
    if not candidates:
        raise FileNotFoundError("No nav_mark_points_*.csv found. Please pass CSV path explicitly.")
    return candidates[0]


def normalize_heading_deg(value: object) -> Optional[float]:
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
    return -math.degrees(math.atan2(y1 - y0, -(x1 - x0)))


def infer_target_yaws(points: List[RoutePoint]) -> None:
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
    for point in points:
        if point.heading_deg is None:
            point.heading_deg = 0.0


def read_points(csv_path: Path) -> Tuple[List[RoutePoint], Optional[float]]:
    with csv_path.open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None:
            raise ValueError("CSV has no header")

        key_map = {normalize_key(k): k for k in reader.fieldnames if k is not None}
        for required in ("x", "y", "point_type"):
            if required not in key_map:
                raise ValueError(f"Missing required column: {required}")

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
                raise ValueError(f"Invalid row {order + 1}: {exc}") from exc

    points.sort(key=lambda item: item.index)
    infer_target_yaws(points)
    fill_missing_heading(points)
    return points, start_heading


def format_header(points: List[RoutePoint], src_path: Path, start_heading: Optional[float]) -> str:
    now = dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    count = len(points)
    # heading_valid = 1 if start_heading is not None else 0
    heading_valid = 0
    heading_deg = 0.0 if start_heading is None else start_heading

    lines = [
        "#ifndef _NAV_REPLAY_ROUTE_TABLE_H_",
        "#define _NAV_REPLAY_ROUTE_TABLE_H_",
        "",
        "#include \"nav_ram.h\"",
        "",
        "// Auto-generated by tools/webview_nav_marker/csv_to_nav_table.py",
        f"// Source CSV: {src_path.as_posix()}",
        f"// Generated at: {now}",
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
        lines.append("    {0.0f, 0.0f, 0.0f, 0.0f, NAV_POINT_PATH},")
    else:
        for point in points:
            lines.append(
                "    "
                f"{{{point.x:.3f}f, {point.y:.3f}f, {point.target_yaw_deg:.3f}f, "
                f"{point.heading_deg:.3f}f, (uint8){point.point_type}}},"
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
        raise FileNotFoundError(f"CSV not found: {csv_path}")
    if args.max_points <= 0:
        raise ValueError("--max-points must be > 0")

    points, start_heading = read_points(csv_path)
    truncated = False
    if len(points) > args.max_points:
        points = points[: args.max_points]
        truncated = True

    out_text = format_header(points, csv_path, start_heading)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(out_text, encoding="utf-8")

    print(f"[OK] CSV: {csv_path}")
    print(f"[OK] Header generated: {out_path}")
    print(f"[OK] Route points: {len(points)}")
    if start_heading is None:
        print("[OK] Start heading: N/A")
    else:
        print(f"[OK] Start heading: {start_heading:.3f} deg")
    if truncated:
        print(f"[WARN] Truncated to max points: {args.max_points}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())