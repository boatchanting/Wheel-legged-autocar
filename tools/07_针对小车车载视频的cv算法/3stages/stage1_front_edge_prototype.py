"""一级台阶前沿检测原型：输出控制所需的左边、右边和最近前沿。

人工真值 PNG 位于 data/三级台阶/people：
  红色=左侧边，蓝色=右侧边，黄色=最近一级台阶前沿。
本脚本不再把“完整台阶状态”作为最终结果，而是寻找下一条待跨越前沿，
输出其端点、中点、横向偏差、中心线方向和可用于距离融合的像素尺度。
"""

from __future__ import annotations

import argparse
import json
import math
from dataclasses import asdict, dataclass
from pathlib import Path

import cv2
import numpy as np


def project_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "data" / "三级台阶").is_dir():
            return parent
    raise FileNotFoundError("未找到项目根目录")


ROOT = project_root()
DATA_ROOT = ROOT / "data" / "三级台阶"
PEOPLE_ROOT = DATA_ROOT / "people"
FRAME_ROOT = DATA_ROOT / "frames" / "2026_07_06_16_43_50_Video"
DEFAULT_OUTPUT = DATA_ROOT / "stage1_front_edge_v1"


@dataclass
class ImageLine:
    """图像线 x = a*y+b，以及其原始支持端点。"""

    a: float
    b: float
    p1: tuple[float, float]
    p2: tuple[float, float]
    length: float

    def x_at(self, y: float) -> float:
        return self.a * y + self.b

    def y_at(self, x: float) -> float | None:
        if abs(self.a) < 1e-5:
            return None
        return (x - self.b) / self.a

    @property
    def y_min(self) -> float:
        return min(self.p1[1], self.p2[1])

    @property
    def y_max(self) -> float:
        return max(self.p1[1], self.p2[1])


@dataclass
class FrontTarget:
    left: ImageLine
    right: ImageLine
    front: ImageLine
    left_front: tuple[float, float]
    right_front: tuple[float, float]
    top_center: tuple[float, float]
    score: float

    @property
    def center(self) -> tuple[float, float]:
        return ((self.left_front[0] + self.right_front[0]) * 0.5, (self.left_front[1] + self.right_front[1]) * 0.5)

    @property
    def span(self) -> float:
        return math.dist(self.left_front, self.right_front)


def line_from_points(p1: tuple[float, float], p2: tuple[float, float]) -> ImageLine | None:
    dx, dy = p2[0] - p1[0], p2[1] - p1[1]
    length = math.hypot(dx, dy)
    if length < 1.0:
        return None
    if abs(dy) < 1e-3:
        return ImageLine(0.0, (p1[1] + p2[1]) * 0.5, p1, p2, length)
    a = dx / dy
    return ImageLine(a, p1[0] - a * p1[1], p1, p2, length)


def fit_colored_line(mask: np.ndarray) -> ImageLine | None:
    ys, xs = np.where(mask)
    if len(xs) < 5:
        return None
    points = np.column_stack((xs, ys)).astype(np.float32)
    vx, vy, x0, y0 = cv2.fitLine(points, cv2.DIST_L2, 0, 0.01, 0.01).flatten()
    projection = (points[:, 0] - x0) * vx + (points[:, 1] - y0) * vy
    p1 = (float(x0 + projection.min() * vx), float(y0 + projection.min() * vy))
    p2 = (float(x0 + projection.max() * vx), float(y0 + projection.max() * vy))
    return line_from_points(p1, p2)


def parse_ground_truth(image: np.ndarray) -> dict[str, ImageLine | None]:
    """从人工颜色线中恢复几何真值，允许 PNG 颜色因抗锯齿略有变化。"""
    b, g, r = cv2.split(image)
    return {
        "left": fit_colored_line((r > 180) & (g < 110) & (b < 110)),
        # 标注中的蓝色为 BGR 约 (232,162,0)，故保留青蓝抗锯齿范围。
        "right": fit_colored_line((b > 160) & (g > 70) & (g < 230) & (r < 90)),
        "front": fit_colored_line((r > 210) & (g > 170) & (b < 100)),
    }


def hough_lines(gray: np.ndarray) -> tuple[list[ImageLine], list[ImageLine], list[ImageLine]]:
    """返回左斜边、右斜边和横线候选；先放大以适配 94x60 的低分辨率。"""
    height, width = gray.shape
    scale = 4 if width < 120 else 2
    enlarged = cv2.resize(gray, (width * scale, height * scale), interpolation=cv2.INTER_CUBIC)
    edge = cv2.Canny(cv2.GaussianBlur(enlarged, (3, 3), 0), 25, 82)
    raw = cv2.HoughLinesP(
        edge, 1, np.pi / 180, threshold=max(16, width // 4),
        minLineLength=max(8 * scale, int(width * scale * 0.10)), maxLineGap=4 * scale,
    )
    left: list[ImageLine] = []
    right: list[ImageLine] = []
    horizontal: list[ImageLine] = []
    if raw is None:
        return left, right, horizontal
    for x1, y1, x2, y2 in raw.reshape(-1, 4):
        line = line_from_points((x1 / scale, y1 / scale), (x2 / scale, y2 / scale))
        if line is None:
            continue
        dx, dy = line.p2[0] - line.p1[0], line.p2[1] - line.p1[1]
        if abs(dy) <= max(1.4, abs(dx) * 0.12) and abs(dx) >= max(8.0, width * 0.10):
            if line.b >= height * 0.12:
                horizontal.append(line)
            continue
        if abs(dy) < 6.0:
            continue
        slope = dx / dy
        if not 0.30 <= abs(slope) <= 2.7:
            continue
        if slope < 0:
            left.append(line)
        else:
            right.append(line)
    return left, right, horizontal


def row_contrast(gray: np.ndarray, y: float, x1: float, x2: float) -> float:
    """台阶前沿应在横线两侧出现亮度变化；作为候选排序项而非硬阈值。"""
    height, width = gray.shape
    lo = max(0, int(math.floor(min(x1, x2))))
    hi = min(width, int(math.ceil(max(x1, x2))) + 1)
    yy = int(round(y))
    if hi - lo < 4 or yy < 2 or yy > height - 3:
        return 0.0
    upper = float(np.mean(gray[max(0, yy - 3):yy, lo:hi]))
    lower = float(np.mean(gray[yy + 1:min(height, yy + 4), lo:hi]))
    return abs(upper - lower)


def choose_front_target(gray: np.ndarray) -> FrontTarget | None:
    height, width = gray.shape
    lefts, rights, fronts = hough_lines(gray)
    best: FrontTarget | None = None
    for left in lefts:
        for right in rights:
            denominator = left.a - right.a
            if abs(denominator) < 0.08:
                continue
            top_y = (right.b - left.b) / denominator
            top_x = left.x_at(top_y)
            if not (-0.25 * height <= top_y < 0.68 * height and -0.30 * width <= top_x <= 1.30 * width):
                continue
            for front in fronts:
                y = front.b
                if y <= top_y + 5 or y < height * 0.22:
                    continue
                xl, xr = left.x_at(y), right.x_at(y)
                if xl > xr:
                    xl, xr = xr, xl
                span = xr - xl
                if not (width * 0.22 <= span <= width * 1.35):
                    continue
                # 黄线不一定完整（可能被高光打断），但应与两条斜边有较大重合。
                hxl, hxr = sorted((front.p1[0], front.p2[0]))
                overlap = max(0.0, min(xr, hxr) - max(xl, hxl))
                if overlap < min(7.0, span * 0.22):
                    continue
                endpoint_gap = abs(hxl - xl) + abs(hxr - xr)
                y_support_gap = abs(y - left.y_max) + abs(y - right.y_max)
                contrast = row_contrast(gray, y, xl, xr)
                center_error = abs((xl + xr) * 0.5 - (width - 1) * 0.5)
                score = 0.0
                score += left.length + right.length + front.length * 1.5
                score += overlap * 1.5 + contrast * 0.55
                score += min(y, height - 1) * 0.35
                score -= endpoint_gap * 0.55 + y_support_gap * 0.35 + center_error * 0.20
                candidate = FrontTarget(left, right, front, (xl, y), (xr, y), (top_x, top_y), score)
                if best is None or candidate.score > best.score:
                    best = candidate
    return best


def bright_trapezoid_fallback(gray: np.ndarray) -> FrontTarget | None:
    """远距离过曝时 Canny 可能只保留底边；用高亮连通域补出一级前沿。

    该分支只在三线组合失败时启用，避免中近距离把整块台面误作最近前沿。
    """
    height, width = gray.shape
    thresholds = sorted({int(np.percentile(gray, q)) for q in (94, 96, 98)} | {220, 235}, reverse=True)
    best: FrontTarget | None = None
    for threshold in thresholds:
        mask = (gray >= threshold).astype(np.uint8) * 255
        count, labels, stats, _ = cv2.connectedComponentsWithStats(mask, 8)
        for label in range(1, count):
            area = int(stats[label, cv2.CC_STAT_AREA])
            x = int(stats[label, cv2.CC_STAT_LEFT])
            y = int(stats[label, cv2.CC_STAT_TOP])
            bw = int(stats[label, cv2.CC_STAT_WIDTH])
            bh = int(stats[label, cv2.CC_STAT_HEIGHT])
            if area < 35 or bw < width * 0.20 or bh < height * 0.12:
                continue
            if y + bh < height * 0.20 or y > height * 0.55:
                continue
            component = np.where(labels == label, 255, 0).astype(np.uint8)
            contours, _ = cv2.findContours(component, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            if not contours:
                continue
            hull = cv2.convexHull(max(contours, key=cv2.contourArea))
            filled = np.zeros_like(component)
            cv2.fillConvexPoly(filled, hull, 255)
            bottom_y = max(row for row in range(height) if np.any(filled[row]))
            bottom_xs = np.flatnonzero(filled[bottom_y])
            if bottom_xs.size < 8:
                continue
            # 远距时高光会吞掉上沿，取下半部分两侧的拟合起点更接近人工标线。
            start_y = int(round(y + bh * 0.45))
            start_y = min(start_y, bottom_y - 4)
            start_xs = np.flatnonzero(filled[start_y])
            if start_xs.size < 5:
                continue
            left = line_from_points((float(start_xs[0]), float(start_y)), (float(bottom_xs[0]), float(bottom_y)))
            right = line_from_points((float(start_xs[-1]), float(start_y)), (float(bottom_xs[-1]), float(bottom_y)))
            front = line_from_points((float(bottom_xs[0]), float(bottom_y)), (float(bottom_xs[-1]), float(bottom_y)))
            if left is None or right is None or front is None:
                continue
            top_center = ((start_xs[0] + start_xs[-1]) * 0.5, float(start_y))
            target = FrontTarget(left, right, front, (float(bottom_xs[0]), float(bottom_y)), (float(bottom_xs[-1]), float(bottom_y)), top_center, float(area + bw * 2 + bh))
            if best is None or target.score > best.score:
                best = target
    return best


def point_line_distance(point: tuple[float, float], line: ImageLine) -> float:
    # 通用二维点到直线距离。ImageLine 的 x=a*y+b 表示不适用于水平线，
    # 因此这里必须使用两个端点而不是 a/b 参数。
    x1, y1 = line.p1
    x2, y2 = line.p2
    return abs((x2 - x1) * (y1 - point[1]) - (x1 - point[0]) * (y2 - y1)) / max(line.length, 1e-6)


def line_error(truth: ImageLine | None, predicted: ImageLine | None) -> float | None:
    if truth is None or predicted is None:
        return None
    return float((point_line_distance(truth.p1, predicted) + point_line_distance(truth.p2, predicted)) * 0.5)


def draw_line(image: np.ndarray, line: ImageLine | None, color: tuple[int, int, int], thickness: int = 1) -> None:
    if line is None:
        return
    cv2.line(image, tuple(np.round(line.p1).astype(int)), tuple(np.round(line.p2).astype(int)), color, thickness, cv2.LINE_AA)


def draw_target(image: np.ndarray, target: FrontTarget | None) -> None:
    if target is None:
        return
    draw_line(image, target.left, (0, 0, 255), 1)       # 左红
    draw_line(image, target.right, (232, 162, 0), 1)     # 右蓝，匹配人工标注色调
    draw_line(image, target.front, (0, 242, 255), 1)     # 最近前沿黄
    center = tuple(np.round(target.center).astype(int))
    cv2.circle(image, center, 2, (0, 255, 0), -1)
    cv2.line(image, tuple(np.round(target.top_center).astype(int)), center, (0, 255, 0), 1, cv2.LINE_AA)


def save_image(path: Path, image: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    cv2.imencode(".png", image)[1].tofile(str(path))


def annotated_paths() -> list[Path]:
    paths = []
    for path in sorted(PEOPLE_ROOT.glob("frame_*.png")):
        image = cv2.imdecode(np.fromfile(str(path), dtype=np.uint8), cv2.IMREAD_COLOR)
        if image is None:
            continue
        truth = parse_ground_truth(image)
        if all(truth[name] is not None for name in ("left", "right", "front")):
            paths.append(path)
    return paths


def run(paths: list[Path], output: Path) -> None:
    output.mkdir(parents=True, exist_ok=True)
    rows: list[dict[str, object]] = []
    panels: list[np.ndarray] = []
    for label_path in paths:
        original_path = FRAME_ROOT / label_path.name
        marked = cv2.imdecode(np.fromfile(str(label_path), dtype=np.uint8), cv2.IMREAD_COLOR)
        original = cv2.imdecode(np.fromfile(str(original_path), dtype=np.uint8), cv2.IMREAD_COLOR)
        if marked is None or original is None:
            print(f"跳过缺失配对: {label_path.name}")
            continue
        truth = parse_ground_truth(marked)
        gray = cv2.cvtColor(original, cv2.COLOR_BGR2GRAY)
        target = choose_front_target(gray)
        if target is None:
            target = bright_trapezoid_fallback(gray)
        predicted = None if target is None else {"left": target.left, "right": target.right, "front": target.front}
        output_image = original.copy()
        draw_target(output_image, target)
        # 左侧是人工标注，右侧是算法输出；两者均保留红蓝黄语义。
        scale = 6
        panel = np.hstack((
            cv2.resize(marked, None, fx=scale, fy=scale, interpolation=cv2.INTER_NEAREST),
            cv2.resize(output_image, None, fx=scale, fy=scale, interpolation=cv2.INTER_NEAREST),
        ))
        text = "GT (left) | prediction (right)"
        cv2.putText(panel, text, (4, 14), cv2.FONT_HERSHEY_SIMPLEX, 0.42, (0, 255, 0), 1, cv2.LINE_AA)
        save_image(output / "comparison" / label_path.name, panel)
        panels.append(panel)
        center = None if target is None else target.center
        rows.append({
            "frame": label_path.name,
            "found": target is not None,
            "left_line_error_px": line_error(truth["left"], None if target is None else target.left),
            "right_line_error_px": line_error(truth["right"], None if target is None else target.right),
            "front_line_error_px": line_error(truth["front"], None if target is None else target.front),
            "next_edge_left_px": None if target is None else [round(target.left_front[0], 2), round(target.left_front[1], 2)],
            "next_edge_right_px": None if target is None else [round(target.right_front[0], 2), round(target.right_front[1], 2)],
            "next_edge_center_px": None if center is None else [round(center[0], 2), round(center[1], 2)],
            "lateral_error_px": None if center is None else round(center[0] - (original.shape[1] - 1) * 0.5, 2),
            "front_span_px": None if target is None else round(target.span, 2),
            # 该角是图像中车辆到前沿中心线相对竖直方向的偏转；真实航向须经 IPM/标定换算。
            "centerline_angle_img_deg": None if target is None else round(math.degrees(math.atan2(center[0] - target.top_center[0], center[1] - target.top_center[1])), 2),
            "score": None if target is None else round(target.score, 2),
        })
    fields = ["left_line_error_px", "right_line_error_px", "front_line_error_px"]
    summary = {
        "labeled_frames": len(rows),
        "detected_frames": sum(bool(row["found"]) for row in rows),
        "mean_line_error_px": {field: round(float(np.mean([row[field] for row in rows if row[field] is not None])), 3) if any(row[field] is not None for row in rows) else None for field in fields},
        "control_contract": {"next_edge": "S1_FRONT", "distance": "需相机标定/编码器融合后提供毫米值", "available_now": ["next_edge_left_px", "next_edge_right_px", "next_edge_center_px", "lateral_error_px", "centerline_angle_img_deg", "front_span_px"]},
        "rows": rows,
    }
    (output / "front_edge_report.json").write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
    if panels:
        columns = 2
        blank = np.zeros_like(panels[0])
        while len(panels) % columns:
            panels.append(blank.copy())
        contact = np.vstack([np.hstack(panels[i:i + columns]) for i in range(0, len(panels), columns)])
        save_image(output / "comparison_contact.png", contact)
    print(json.dumps({key: value for key, value in summary.items() if key != "rows"}, ensure_ascii=False, indent=2))


def main() -> None:
    parser = argparse.ArgumentParser(description="一级台阶最近前沿检测与人工线标注对比")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--label", type=Path, action="append", help="指定 people 下的标注 PNG，可重复")
    args = parser.parse_args()
    paths = args.label or annotated_paths()
    if not paths:
        raise SystemExit("people 中没有找到包含红、蓝、黄三条线的标注 PNG")
    run(paths, args.output)


if __name__ == "__main__":
    main()
