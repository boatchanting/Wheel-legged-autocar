"""三级台阶离线识别原型。

算法面向 94x60 / 188x120 的车载低视角图像：
1. 多阈值提取一级白色顶面，按“居中的下宽上窄梯形”选择目标；
2. 对目标凸包逐行取左右边界，拟合一级台阶两侧边缘与中心线；
3. 在目标上方和内部搜索短横边，验证二、三级台阶的层级结构；
4. 依据可见面积和横边数量给出 FAR / FULL / NEAR 等状态。

输入和输出默认均在项目 data/三级台阶 下。仅依赖 numpy 与 opencv。
"""

from __future__ import annotations

import argparse
import csv
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

import cv2
import numpy as np


REPRESENTATIVE_FRAMES: dict[str, tuple[int, ...]] = {
    "2026_07_06_16_43_50_Video": (159, 238, 356),
    "2026_07_06_16_46_53_Video": (1, 326, 652),
    "2026_07_06_16_49_29_Video": (107, 426, 851),
    "2026_07_06_16_51_56_Video": (198, 394, 690),
    "2026_07_06_16_52_46_Video": (135, 336, 604),
    "realtime_record_20260706_171433": (1, 105, 314, 471),
}


@dataclass
class Candidate:
    threshold: int
    score: float
    mask: np.ndarray
    hull: np.ndarray
    bbox: tuple[int, int, int, int]
    area_ratio: float
    center_x: float
    top_width: float
    bottom_width: float
    expansion: float
    left_line: tuple[float, float] | None
    right_line: tuple[float, float] | None


@dataclass
class StairResult:
    source: str
    frame: str
    width: int
    height: int
    threshold: int | None
    state: str
    confidence: float
    target_found: bool
    area_ratio: float
    center_offset_px: float | None
    center_offset_norm: float | None
    top_width_px: float | None
    bottom_width_px: float | None
    expansion: float | None
    horizontal_edge_count: int
    horizontal_edges_y: list[int]
    blue_ratio: float


def project_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "data" / "三级台阶").is_dir():
            return parent
    raise FileNotFoundError("未找到项目根目录下的 data/三级台阶")


ROOT = project_root()
DATA_ROOT = ROOT / "data" / "三级台阶"
FRAMES_ROOT = DATA_ROOT / "frames"
DEFAULT_OUTPUT = DATA_ROOT / "python_results_3stages_v1"


def threshold_candidates(gray: np.ndarray) -> list[int]:
    """返回对曝光变化有容忍度的亮度阈值集合。"""
    otsu, _ = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    quantiles = [np.percentile(gray, q) for q in (67, 75, 82, 88, 93)]
    raw = [otsu - 20, otsu - 8, otsu, otsu + 12, *quantiles, 110, 135, 160, 185]
    return sorted({int(np.clip(value, 70, 240)) for value in raw})


def cleanup_mask(mask: np.ndarray, image_shape: tuple[int, int]) -> np.ndarray:
    height, width = image_shape
    kernel_size = 3 if min(height, width) < 100 else 5
    kernel = np.ones((kernel_size, kernel_size), np.uint8)
    closed = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=1)
    return cv2.morphologyEx(closed, cv2.MORPH_OPEN, np.ones((2, 2), np.uint8), iterations=1)


def hull_mask(component: np.ndarray) -> tuple[np.ndarray, np.ndarray] | None:
    contours, _ = cv2.findContours(component, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return None
    contour = max(contours, key=cv2.contourArea)
    if len(contour) < 3:
        return None
    hull = cv2.convexHull(contour)
    filled = np.zeros_like(component)
    cv2.fillConvexPoly(filled, hull, 255)
    return filled, hull


def boundary_widths(mask: np.ndarray) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    height = mask.shape[0]
    left = np.full(height, np.nan, dtype=np.float32)
    right = np.full(height, np.nan, dtype=np.float32)
    for y in range(height):
        xs = np.flatnonzero(mask[y])
        if xs.size:
            left[y], right[y] = xs[0], xs[-1]
    return left, right, right - left + 1


def fit_side(rows: np.ndarray, xs: np.ndarray) -> tuple[float, float] | None:
    if rows.size < 6:
        return None
    # x = a * y + b；一次残差剔除可避免少量背景亮点拉偏边线。
    a, b = np.polyfit(rows, xs, 1)
    residual = np.abs(xs - (a * rows + b))
    inliers = residual <= max(1.5, float(np.percentile(residual, 75)) * 1.5)
    if int(inliers.sum()) >= 5:
        a, b = np.polyfit(rows[inliers], xs[inliers], 1)
    return float(a), float(b)


def evaluate_component(component: np.ndarray, threshold: int, gray: np.ndarray) -> Candidate | None:
    height, width = component.shape
    converted = hull_mask(component)
    if converted is None:
        return None
    hull_mask_image, hull = converted
    x, y, bw, bh = cv2.boundingRect(hull)
    if bw < max(8, width * 0.10) or bh < max(4, height * 0.06):
        return None

    left, right, widths = boundary_widths(hull_mask_image > 0)
    rows = np.flatnonzero(np.isfinite(widths))
    if rows.size < 6:
        return None
    valid_widths = widths[rows]
    band = max(2, int(rows.size * 0.20))
    top_width = float(np.median(valid_widths[:band]))
    bottom_width = float(np.median(valid_widths[-band:]))
    expansion = bottom_width / max(top_width, 1.0)
    center_x = float((np.nanmedian(left[rows]) + np.nanmedian(right[rows])) * 0.5)
    area_ratio = float(np.count_nonzero(hull_mask_image) / (height * width))

    left_fit = fit_side(rows, left[rows])
    right_fit = fit_side(rows, right[rows])
    center_distance = abs(center_x - (width - 1) * 0.5) / max(width * 0.5, 1.0)
    lower_presence = (y + bh) / height
    # 一级顶面应大致位于画面中下部，且宽度通常向近端增大；仍保留偏航样本。
    score = 0.0
    score += min(rows.size, height) * 2.0
    score += min(bw, width) * 1.2
    score += min(bh, height) * 1.0
    score += min(area_ratio, 0.45) * 90.0
    score += min(expansion, 3.0) * 8.0
    score += lower_presence * 18.0
    score -= center_distance * 24.0
    # 低阈值下，亮地面会形成几乎铺满全图的矩形连通域；它既不居中也
    # 不呈近大远小的梯形，必须显著降分，给高阈值下的真实台阶让位。
    if expansion < 0.82:
        score -= 100.0
    elif expansion < 1.05:
        score -= 42.0
    score -= max(0.0, area_ratio - 0.72) * 450.0
    if y > height * 0.78:
        score -= 18.0
    if area_ratio < 0.008 or area_ratio > 0.92:
        score -= 80.0

    return Candidate(
        threshold=threshold,
        score=float(score),
        mask=component,
        hull=hull,
        bbox=(x, y, bw, bh),
        area_ratio=area_ratio,
        center_x=center_x,
        top_width=top_width,
        bottom_width=bottom_width,
        expansion=expansion,
        left_line=left_fit,
        right_line=right_fit,
    )


def detect_primary_surface(gray: np.ndarray) -> Candidate | None:
    """多阈值连通域筛选白色一级顶面；不把蓝色当作必要条件。"""
    best: Candidate | None = None
    height, width = gray.shape
    for threshold in threshold_candidates(gray):
        mask = cleanup_mask((gray >= threshold).astype(np.uint8) * 255, (height, width))
        count, labels, stats, _ = cv2.connectedComponentsWithStats(mask, connectivity=8)
        for label in range(1, count):
            area = int(stats[label, cv2.CC_STAT_AREA])
            if area < max(12, int(height * width * 0.006)):
                continue
            component = np.where(labels == label, 255, 0).astype(np.uint8)
            candidate = evaluate_component(component, threshold, gray)
            if candidate is not None and (best is None or candidate.score > best.score):
                best = candidate
    return best


def horizontal_edges(gray: np.ndarray, candidate: Candidate | None) -> list[tuple[int, int, int, float]]:
    """在一级顶面及其上方找横边，返回 (y, x1, x2, score)。"""
    if candidate is None:
        return []
    height, width = gray.shape
    scale = 4 if width < 120 else 2
    enlarged = cv2.resize(gray, (width * scale, height * scale), interpolation=cv2.INTER_CUBIC)
    edges = cv2.Canny(cv2.GaussianBlur(enlarged, (3, 3), 0), 28, 85)
    lines = cv2.HoughLinesP(
        edges, rho=1, theta=np.pi / 180, threshold=max(12, width // 2),
        minLineLength=max(9 * scale, int(width * scale * 0.12)), maxLineGap=4 * scale,
    )
    if lines is None:
        return []
    x, y, bw, bh = candidate.bbox
    y_min = max(0, y - int(height * 0.18))
    y_max = min(height - 1, y + bh)
    found: list[tuple[int, int, int, float]] = []
    for x1, yy1, x2, yy2 in lines.reshape(-1, 4):
        if abs(yy2 - yy1) > max(2 * scale, abs(x2 - x1) * 0.16):
            continue
        line_y = int(round((yy1 + yy2) / (2 * scale)))
        line_x1, line_x2 = sorted((int(round(x1 / scale)), int(round(x2 / scale))))
        length = line_x2 - line_x1
        if line_y < y_min or line_y > y_max or length < max(8, int(width * 0.12)):
            continue
        # 与一级梯形的横向范围应存在重叠；允许上方短横边更短。
        overlap = max(0, min(line_x2, x + bw) - max(line_x1, x))
        if overlap < max(4, int(length * 0.35)):
            continue
        y_lo, y_hi = sorted((yy1, yy2))
        x_lo, x_hi = sorted((x1, x2))
        contrast_region = edges[max(0, y_lo - 1):min(edges.shape[0], y_hi + 2), x_lo:x_hi + 1]
        contrast = float(np.mean(contrast_region)) if contrast_region.size else 0.0
        found.append((line_y, line_x1, line_x2, length + contrast / 255.0))

    # 相同物理横边在 Canny 的上下沿会重复，保留每组最强的一条。
    found.sort(key=lambda item: item[3], reverse=True)
    unique: list[tuple[int, int, int, float]] = []
    for item in found:
        if any(abs(item[0] - saved[0]) <= 2 for saved in unique):
            continue
        unique.append(item)
    return sorted(unique[:4], key=lambda item: item[0])


def blue_ratio(image: np.ndarray, candidate: Candidate | None) -> float:
    """彩色相机时为蓝色二级台阶提供辅助证据；灰度相机自然返回 0。"""
    if candidate is None or image.ndim != 3:
        return 0.0
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    blue = cv2.inRange(hsv, (92, 38, 25), (138, 255, 255)) > 0
    x, y, bw, bh = candidate.bbox
    roi = blue[max(0, y - bh // 3):min(image.shape[0], y + bh), x:x + bw]
    return float(np.count_nonzero(roi) / max(roi.size, 1))


def classify(candidate: Candidate | None, lines: list[tuple[int, int, int, float]], shape: tuple[int, int]) -> tuple[str, float]:
    if candidate is None or candidate.score < 18.0:
        return "NONE", 0.0
    height, width = shape
    x, y, bw, bh = candidate.bbox
    edge_count = len(lines)
    confidence = 35.0 + min(candidate.score, 120.0) * 0.45 + min(edge_count, 3) * 7.0
    confidence -= min(abs(candidate.center_x - (width - 1) / 2) / width, 0.5) * 20.0
    # 完整结构优先于面积判定：车体对正的中距离画面中一级顶面本来就会
    # 覆盖较大区域。只有横边已明显不足，或目标整体沉到画面底部时才退化。
    has_full_geometry = edge_count >= 2 and candidate.expansion >= 1.12
    near = ((y > height * 0.27 and y + bh >= height * 0.94) or
            (candidate.area_ratio > 0.78 and edge_count < 2) or
            (bh > height * 0.82 and y < height * 0.12 and edge_count < 2))
    far = candidate.area_ratio < 0.06 or bh < height * 0.18
    if has_full_geometry and not near:
        state = "FULL_STAIR"
    elif near:
        state = "NEAR_DEGRADED"
    elif far:
        state = "FAR_CANDIDATE"
    else:
        state = "APPROACHING"
    return state, float(np.clip(confidence, 0.0, 99.0))


def draw_result(image: np.ndarray, candidate: Candidate | None, lines: list[tuple[int, int, int, float]], state: str, confidence: float) -> np.ndarray:
    drawing = image.copy()
    if drawing.ndim == 2:
        drawing = cv2.cvtColor(drawing, cv2.COLOR_GRAY2BGR)
    height, width = drawing.shape[:2]
    cv2.line(drawing, (width // 2, 0), (width // 2, height - 1), (0, 220, 220), 1)
    if candidate is not None:
        cv2.polylines(drawing, [candidate.hull], True, (0, 255, 0), 1, cv2.LINE_AA)
        x, y, bw, bh = candidate.bbox
        cv2.rectangle(drawing, (x, y), (x + bw - 1, y + bh - 1), (0, 160, 255), 1)
        for fit, color in ((candidate.left_line, (255, 80, 80)), (candidate.right_line, (80, 80, 255))):
            if fit is None:
                continue
            a, b = fit
            p1 = (int(round(a * y + b)), y)
            p2 = (int(round(a * (y + bh - 1) + b)), y + bh - 1)
            cv2.line(drawing, p1, p2, color, 1, cv2.LINE_AA)
        center = int(round(candidate.center_x))
        cv2.line(drawing, (center, y), (center, min(height - 1, y + bh - 1)), (255, 255, 0), 1)
    for line_y, x1, x2, _ in lines:
        cv2.line(drawing, (x1, line_y), (x2, line_y), (255, 0, 255), 1, cv2.LINE_AA)
    cv2.putText(drawing, f"{state} {confidence:.0f}", (2, max(10, height - 3)), cv2.FONT_HERSHEY_SIMPLEX, 0.32 if width < 120 else 0.5, (0, 0, 255), 1, cv2.LINE_AA)
    return drawing


def make_debug_panel(image: np.ndarray, candidate: Candidate | None, annotated: np.ndarray) -> np.ndarray:
    if image.ndim == 2:
        original = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)
    else:
        original = image.copy()
    mask = np.zeros_like(original)
    if candidate is not None:
        mask[candidate.mask > 0] = (255, 255, 255)
        cv2.polylines(mask, [candidate.hull], True, (0, 255, 0), 1)
    # 低分辨率源图放大后再拼接，便于人工验收。
    scale = 6 if image.shape[1] < 120 else 3
    return np.hstack([cv2.resize(panel, None, fx=scale, fy=scale, interpolation=cv2.INTER_NEAREST) for panel in (original, mask, annotated)])


def process_one(path: Path, output_dir: Path) -> StairResult | None:
    image = cv2.imdecode(np.fromfile(str(path), dtype=np.uint8), cv2.IMREAD_COLOR)
    if image is None:
        print(f"跳过无法读取的图片: {path}")
        return None
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    candidate = detect_primary_surface(gray)
    lines = horizontal_edges(gray, candidate)
    state, confidence = classify(candidate, lines, gray.shape)
    annotated = draw_result(image, candidate, lines, state, confidence)
    debug = make_debug_panel(image, candidate, annotated)
    relative_parent = path.parent.name
    image_dir = output_dir / "annotated" / relative_parent
    debug_dir = output_dir / "debug" / relative_parent
    image_dir.mkdir(parents=True, exist_ok=True)
    debug_dir.mkdir(parents=True, exist_ok=True)
    cv2.imencode(".png", annotated)[1].tofile(str(image_dir / path.name))
    cv2.imencode(".png", debug)[1].tofile(str(debug_dir / path.name))

    if candidate is None:
        return StairResult(relative_parent, path.name, gray.shape[1], gray.shape[0], None, state, confidence, False, 0.0, None, None, None, None, None, 0, [], 0.0)
    offset = candidate.center_x - (gray.shape[1] - 1) * 0.5
    return StairResult(
        relative_parent, path.name, gray.shape[1], gray.shape[0], candidate.threshold, state, confidence, True,
        candidate.area_ratio, offset, offset / max(gray.shape[1] * 0.5, 1), candidate.top_width,
        candidate.bottom_width, candidate.expansion, len(lines), [line[0] for line in lines], blue_ratio(image, candidate),
    )


def representative_paths() -> list[Path]:
    paths: list[Path] = []
    for video, indices in REPRESENTATIVE_FRAMES.items():
        for index in indices:
            path = FRAMES_ROOT / video / f"frame_{index:06d}.png"
            if path.is_file():
                paths.append(path)
            else:
                print(f"代表帧不存在: {path}")
    return paths


def all_paths() -> Iterable[Path]:
    for folder in sorted(FRAMES_ROOT.iterdir()):
        if folder.is_dir():
            yield from sorted(folder.glob("frame_*.png"))


def write_contact_sheet(results: list[StairResult], output_dir: Path) -> None:
    tiles: list[np.ndarray] = []
    for result in results:
        path = output_dir / "annotated" / result.source / result.frame
        image = cv2.imdecode(np.fromfile(str(path), dtype=np.uint8), cv2.IMREAD_COLOR)
        if image is None:
            continue
        tile = cv2.resize(image, (282, 180), interpolation=cv2.INTER_NEAREST)
        cv2.putText(tile, f"{result.source[-8:]} {result.frame[6:12]}", (4, 16), cv2.FONT_HERSHEY_SIMPLEX, 0.38, (0, 0, 0), 2, cv2.LINE_AA)
        cv2.putText(tile, f"{result.source[-8:]} {result.frame[6:12]}", (4, 16), cv2.FONT_HERSHEY_SIMPLEX, 0.38, (255, 255, 255), 1, cv2.LINE_AA)
        tiles.append(tile)
    if not tiles:
        return
    columns = 4
    blank = np.full_like(tiles[0], 35)
    while len(tiles) % columns:
        tiles.append(blank.copy())
    rows = [np.hstack(tiles[i:i + columns]) for i in range(0, len(tiles), columns)]
    cv2.imencode(".png", np.vstack(rows))[1].tofile(str(output_dir / "representative_contact.png"))


def run(paths: Iterable[Path], output_dir: Path) -> list[StairResult]:
    output_dir.mkdir(parents=True, exist_ok=True)
    results = [result for path in paths if (result := process_one(path, output_dir)) is not None]
    with (output_dir / "results.json").open("w", encoding="utf-8") as handle:
        json.dump([asdict(result) for result in results], handle, ensure_ascii=False, indent=2)
    fields = list(StairResult.__dataclass_fields__)
    with (output_dir / "results.csv").open("w", newline="", encoding="utf-8-sig") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields)
        writer.writeheader()
        writer.writerows(asdict(result) for result in results)
    write_contact_sheet(results, output_dir)
    states: dict[str, int] = {}
    for result in results:
        states[result.state] = states.get(result.state, 0) + 1
    summary = {
        "algorithm": "multi-threshold bright trapezoid + side-line fit + horizontal-edge hierarchy",
        "processed_frames": len(results),
        "state_counts": states,
        "known_geometry_mm": {"width": 500, "stage1_length": 500, "stage1_height": 100, "stage2_length": 500, "stage2_height": 50, "stage3_length": 250, "stage3_height": 150, "ramp_angle_deg": 22, "ramp_hypotenuse": 404, "ramp_base": 375},
        "notes": ["样本是灰度成像，蓝色检测只作为彩色输入时的辅助证据。", "像素级距离未做标定，输出中心偏移和边宽均为像素/归一化坐标。"],
    }
    with (output_dir / "summary.json").open("w", encoding="utf-8") as handle:
        json.dump(summary, handle, ensure_ascii=False, indent=2)
    print(json.dumps({"output": str(output_dir), **summary}, ensure_ascii=False, indent=2))
    return results


def main() -> None:
    parser = argparse.ArgumentParser(description="三级台阶代表帧离线识别原型")
    parser.add_argument("--all", action="store_true", help="处理全部抽帧；默认只处理图片说明中的代表帧")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="结果目录（默认在 data/三级台阶 下）")
    parser.add_argument("--image", type=Path, action="append", help="额外指定图片；可重复传入")
    args = parser.parse_args()
    paths = list(all_paths()) if args.all else representative_paths()
    if args.image:
        paths.extend(args.image)
    if not paths:
        raise SystemExit("没有可处理的图片")
    run(paths, args.output)


if __name__ == "__main__":
    main()
