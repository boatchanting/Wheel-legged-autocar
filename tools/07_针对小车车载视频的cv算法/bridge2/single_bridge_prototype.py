from __future__ import annotations

import argparse
import csv
import io
import json
import struct
import time
from collections import Counter
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage
from scipy.spatial import ConvexHull, QhullError


DATA_DIR_NAME = "\u5355\u8fb9\u6865"
STATE_NONE = "\u65e0"
STATE_PREPARE_ENTER = "\u51c6\u5907\u8fdb\u5165"
STATE_ON_PVC = "\u5728PVC\u4e0a"
STATE_PREPARE_EXIT = "\u51c6\u5907\u9000\u51fa"

IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".bmp"}
MIN_VALID_SCORE = 350.0
DEFAULT_SOURCE_FPS = 50.0
JPEG_QUALITY = 90


def resolve_project_root() -> Path:
    here = Path(__file__).resolve()
    for parent in here.parents:
        if (parent / "data" / DATA_DIR_NAME).exists():
            return parent
    raise FileNotFoundError(f"Could not locate project root containing data/{DATA_DIR_NAME}.")


PROJECT_ROOT = resolve_project_root()
DATA_ROOT = PROJECT_ROOT / "data" / DATA_DIR_NAME
FRAMES_DIR = DATA_ROOT / "frames"
DEFAULT_SUBSET = DATA_ROOT / "single_bridge_subset.json"
DEFAULT_OUTPUT = DATA_ROOT / "python_results"


@dataclass
class LineFit:
    model: str
    slope: float
    intercept: float
    support_min: float
    support_max: float
    inlier_count: int
    span: float
    residual: float
    border_touch_ratio: float
    mean_value: float

    def x_at_y(self, y: float) -> float:
        if self.model != "x_from_y":
            raise ValueError("x_at_y is only valid for x_from_y lines.")
        return self.slope * y + self.intercept

    def y_at_x(self, x: float) -> float:
        if self.model != "y_from_x":
            raise ValueError("y_at_x is only valid for y_from_x lines.")
        return self.slope * x + self.intercept


@dataclass
class PvcCandidate:
    threshold: int
    score: float
    visible_mask: np.ndarray
    outer_mask: np.ndarray
    top_row: int
    start_row: int
    bottom_row: int
    max_width: int
    bottom_width: int
    area: int
    area_ratio: float
    center_x: float
    edge_contrast: float
    left_clip_ratio: float
    right_clip_ratio: float
    dual_clip_ratio: float
    border_monotonic: float


@dataclass
class BridgeResult:
    video: str
    frame: str
    tag: str
    expected_state: str | None
    state_match: bool | None
    threshold: int
    pvc_found: bool
    pvc_state: str
    pvc_area: int
    pvc_area_ratio: float
    pvc_top_row: int
    pvc_start_row: int
    pvc_bottom_row: int
    pvc_max_width: int
    pvc_bottom_width: int
    pvc_center_x: float | None
    edge_contrast: float
    left_clip_ratio: float
    right_clip_ratio: float
    dual_clip_ratio: float
    border_monotonic: float
    candidate_score: float
    left_line_visible: bool
    right_line_visible: bool
    pink_line_visible: bool
    yellow_line_visible: bool
    left_line_segment: list[int] | None
    right_line_segment: list[int] | None
    pink_line_segment: list[int] | None
    yellow_line_segment: list[int] | None


@dataclass
class FolderTimingStats:
    folder_name: str
    output_dir: str
    frame_count: int
    fps: float
    avg_recognition_ms: float
    median_recognition_ms: float
    p95_recognition_ms: float
    min_recognition_ms: float
    min_recognition_frame: str
    max_recognition_ms: float
    max_recognition_frame: str
    total_recognition_ms: float
    video_encode_ms: float
    total_wall_ms: float
    state_counts: dict[str, int]
    video_path: str


def otsu_threshold(image: np.ndarray, search_limit: int = 180) -> int:
    hist = np.bincount(image.ravel(), minlength=256).astype(np.float64)
    total = max(image.size, 1)
    prob = hist / total
    gray = np.arange(256, dtype=np.float64)

    cumulative_prob = np.cumsum(prob[: search_limit + 1])
    cumulative_mean = np.cumsum(prob[: search_limit + 1] * gray[: search_limit + 1])
    global_mean = cumulative_mean[-1]

    best_threshold = 0
    best_score = -1.0
    for threshold in range(search_limit):
        w0 = cumulative_prob[threshold]
        w1 = 1.0 - w0
        if w0 <= 1e-6 or w1 <= 1e-6:
            continue
        mean0 = cumulative_mean[threshold] / w0
        mean1 = (global_mean - cumulative_mean[threshold]) / w1
        score = w0 * w1 * (mean0 - mean1) ** 2
        if score > best_score:
            best_score = score
            best_threshold = threshold
    return max(best_threshold, 70)


def build_threshold_map(width: int, threshold: int) -> np.ndarray:
    threshold_map = np.full(width, threshold, dtype=np.int16)
    threshold_map[:19] -= 10
    threshold_map[76:] -= 10
    threshold_map[83:89] -= 10
    return threshold_map


def build_threshold_candidates(gray: np.ndarray) -> list[int]:
    base = otsu_threshold(gray)
    mean = float(gray.mean())
    std = float(gray.std())
    candidates = {
        base - 25,
        base - 15,
        base - 8,
        base,
        int(np.percentile(gray, 82)),
        int(np.percentile(gray, 86)),
        int(np.percentile(gray, 90)),
        int(np.percentile(gray, 92)),
        int(mean + 0.45 * std),
        int(mean + 0.75 * std),
        int(mean + 0.95 * std),
    }
    return sorted({int(np.clip(value, 90, 225)) for value in candidates})


def convex_hull_mask(mask: np.ndarray) -> np.ndarray:
    rows, cols = np.where(mask)
    if rows.size < 3:
        return np.zeros_like(mask, dtype=bool)
    points = np.column_stack([cols, rows])
    try:
        hull = ConvexHull(points)
    except QhullError:
        return np.zeros_like(mask, dtype=bool)
    hull_image = Image.new("1", (mask.shape[1], mask.shape[0]), 0)
    draw = ImageDraw.Draw(hull_image)
    polygon = [(int(points[index][0]), int(points[index][1])) for index in hull.vertices]
    draw.polygon(polygon, outline=1, fill=1)
    return np.array(hull_image, dtype=bool)


def extract_row_borders(mask: np.ndarray) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    height = mask.shape[0]
    left = np.full(height, -1, dtype=np.int16)
    right = np.full(height, -1, dtype=np.int16)
    width = np.zeros(height, dtype=np.int16)
    for row in range(height):
        cols = np.flatnonzero(mask[row])
        if cols.size:
            left[row] = int(cols[0])
            right[row] = int(cols[-1])
            width[row] = int(cols[-1] - cols[0] + 1)
    return left, right, width


def extract_column_borders(mask: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    width = mask.shape[1]
    top = np.full(width, -1, dtype=np.int16)
    bottom = np.full(width, -1, dtype=np.int16)
    for col in range(width):
        rows = np.flatnonzero(mask[:, col])
        if rows.size:
            top[col] = int(rows[0])
            bottom[col] = int(rows[-1])
    return top, bottom


def find_start_row(widths: np.ndarray, valid_rows: np.ndarray, max_width: int) -> int:
    min_width = max(6, int(round(max_width * 0.12)))
    for index, row in enumerate(valid_rows):
        if widths[row] < min_width:
            continue
        next_rows = valid_rows[index : index + 3]
        if next_rows.size >= 2 and np.all(widths[next_rows] >= max(4, min_width - 2)):
            return int(row)
    return int(valid_rows[0])


def compute_edge_contrast(
    gray: np.ndarray,
    left: np.ndarray,
    right: np.ndarray,
    rows: np.ndarray,
) -> float:
    inside_values: list[float] = []
    outside_values: list[float] = []
    image_width = gray.shape[1]

    for row in rows:
        left_x = int(left[row])
        right_x = int(right[row])
        if left_x < 0 or right_x < 0:
            continue

        outside_samples: list[float] = []
        if left_x >= 2:
            outside_samples.append(float(gray[row, left_x - 2]))
        elif left_x >= 1:
            outside_samples.append(float(gray[row, left_x - 1]))

        if right_x <= image_width - 3:
            outside_samples.append(float(gray[row, right_x + 2]))
        elif right_x <= image_width - 2:
            outside_samples.append(float(gray[row, right_x + 1]))

        if not outside_samples:
            continue

        inside_values.append((float(gray[row, left_x]) + float(gray[row, right_x])) * 0.5)
        outside_values.append(sum(outside_samples) / len(outside_samples))

    if not inside_values:
        return 0.0
    return float(np.mean(inside_values) - np.mean(outside_values))


def evaluate_component(gray: np.ndarray, component: np.ndarray, threshold: int) -> PvcCandidate | None:
    visible_mask = ndimage.binary_closing(component, structure=np.ones((3, 3), dtype=bool))
    visible_mask = ndimage.binary_opening(visible_mask, structure=np.ones((2, 2), dtype=bool))
    filled_mask = ndimage.binary_fill_holes(visible_mask)
    outer_mask = convex_hull_mask(filled_mask)
    if not outer_mask.any():
        outer_mask = filled_mask.astype(bool)

    left, right, widths = extract_row_borders(outer_mask)
    valid_rows = np.flatnonzero(widths > 0)
    if valid_rows.size < 10:
        return None

    top_row = int(valid_rows[0])
    bottom_row = int(valid_rows[-1])
    max_width = int(widths[valid_rows].max())
    start_row = find_start_row(widths, valid_rows, max_width)
    stable_rows = valid_rows[valid_rows >= start_row]
    if stable_rows.size < 10:
        return None

    area = int(outer_mask.sum())
    area_ratio = float(area / outer_mask.size)
    bottom_width = int(widths[bottom_row])
    start_width = int(widths[start_row])

    centers = (left[stable_rows] + right[stable_rows]) / 2.0
    center_x = float(np.average(centers, weights=np.maximum(widths[stable_rows], 1)))
    edge_contrast = compute_edge_contrast(gray, left, right, stable_rows)

    image_width = gray.shape[1]
    left_clip_ratio = float(np.mean(left[stable_rows] <= 1))
    right_clip_ratio = float(np.mean(right[stable_rows] >= image_width - 2))
    dual_clip_ratio = float(np.mean((left[stable_rows] <= 1) & (right[stable_rows] >= image_width - 2)))

    width_deltas = np.diff(widths[stable_rows].astype(np.int16))
    border_monotonic = float(np.mean(width_deltas >= -2)) if width_deltas.size else 1.0

    score = 0.0
    score += stable_rows.size * 9.0
    score += edge_contrast * 3.5
    score += max_width * 0.8
    score += max(bottom_row - start_row, 0) * 1.2
    score += max(max_width - start_width, 0) * 0.4
    score += border_monotonic * 60.0
    score += threshold * 0.25
    score -= abs(center_x - ((image_width - 1) / 2.0)) * 1.8
    score -= left_clip_ratio * 25.0
    score -= right_clip_ratio * 25.0
    score -= dual_clip_ratio * 120.0

    if edge_contrast < 15.0:
        score -= 1500.0
    if max_width >= image_width - 4 and dual_clip_ratio > 0.55:
        score -= 2200.0
    if top_row <= 4 and dual_clip_ratio > 0.45:
        score -= 1400.0
    if area_ratio > 0.72 and edge_contrast < 25.0:
        score -= 1200.0
    if max_width < 12:
        score -= 600.0

    return PvcCandidate(
        threshold=threshold,
        score=float(score),
        visible_mask=visible_mask.astype(bool),
        outer_mask=outer_mask.astype(bool),
        top_row=top_row,
        start_row=start_row,
        bottom_row=bottom_row,
        max_width=max_width,
        bottom_width=bottom_width,
        area=area,
        area_ratio=area_ratio,
        center_x=center_x,
        edge_contrast=edge_contrast,
        left_clip_ratio=left_clip_ratio,
        right_clip_ratio=right_clip_ratio,
        dual_clip_ratio=dual_clip_ratio,
        border_monotonic=border_monotonic,
    )


def detect_pvc_candidate(gray: np.ndarray) -> PvcCandidate | None:
    best_candidate: PvcCandidate | None = None

    for threshold in build_threshold_candidates(gray):
        threshold_map = build_threshold_map(gray.shape[1], threshold)
        mask = gray > threshold_map[np.newaxis, :]
        mask = ndimage.binary_closing(mask, structure=np.ones((3, 3), dtype=bool))
        mask = ndimage.binary_opening(mask, structure=np.ones((2, 2), dtype=bool))

        labels, count = ndimage.label(mask)
        for label in range(1, count + 1):
            component = labels == label
            if int(component.sum()) < 24:
                continue
            candidate = evaluate_component(gray, component, threshold)
            if candidate is None:
                continue
            if best_candidate is None or candidate.score > best_candidate.score:
                best_candidate = candidate

    return best_candidate


def fit_line_exhaustive(
    independent: np.ndarray,
    dependent: np.ndarray,
    *,
    model: str,
    slope_range: tuple[float, float],
    residual_threshold: float,
    min_inliers: int,
    min_span: float,
    border_limit: float,
    prefer: str,
) -> LineFit | None:
    if independent.size < min_inliers:
        return None

    independent = independent.astype(np.float64)
    dependent = dependent.astype(np.float64)

    best_score = -1e18
    best_inliers: np.ndarray | None = None

    for i in range(independent.size - 1):
        for j in range(i + 1, independent.size):
            delta = independent[j] - independent[i]
            if abs(delta) < 3.0:
                continue

            slope = (dependent[j] - dependent[i]) / delta
            if not (slope_range[0] <= slope <= slope_range[1]):
                continue

            intercept = dependent[i] - slope * independent[i]
            residuals = np.abs(dependent - (slope * independent + intercept))
            inliers = residuals <= residual_threshold
            inlier_count = int(inliers.sum())
            if inlier_count < min_inliers:
                continue

            span = float(independent[inliers].max() - independent[inliers].min())
            if span < min_span:
                continue

            mean_value = float(dependent[inliers].mean())
            mean_residual = float(residuals[inliers].mean())

            score = inlier_count * 12.0 + span * 2.0 - mean_residual * 6.0
            if prefer == "top":
                score -= mean_value * 0.7
            elif prefer == "bottom":
                score += mean_value * 0.7
            elif prefer == "left":
                score -= mean_value * 0.25
            elif prefer == "right":
                score += mean_value * 0.25

            if score > best_score:
                best_score = score
                best_inliers = inliers

    if best_inliers is None or int(best_inliers.sum()) < min_inliers:
        return None

    slope, intercept = np.polyfit(independent[best_inliers], dependent[best_inliers], 1)
    residuals = np.abs(dependent - (slope * independent + intercept))
    relaxed_threshold = max(residual_threshold, float(np.percentile(residuals[best_inliers], 80)) * 1.3)
    inliers = residuals <= relaxed_threshold
    if int(inliers.sum()) < min_inliers:
        inliers = best_inliers

    slope, intercept = np.polyfit(independent[inliers], dependent[inliers], 1)
    residuals = np.abs(dependent - (slope * independent + intercept))
    final_threshold = max(residual_threshold, float(np.percentile(residuals[inliers], 80)) * 1.2)
    inliers = residuals <= final_threshold
    if int(inliers.sum()) < min_inliers:
        inliers = best_inliers
        slope, intercept = np.polyfit(independent[inliers], dependent[inliers], 1)
        residuals = np.abs(dependent - (slope * independent + intercept))

    support_min = float(independent[inliers].min())
    support_max = float(independent[inliers].max())
    span = support_max - support_min
    mean_value = float(dependent[inliers].mean())
    mean_residual = float(residuals[inliers].mean())

    if prefer == "left":
        border_touch_ratio = float(np.mean(dependent[inliers] <= border_limit))
    elif prefer == "right":
        border_touch_ratio = float(np.mean(dependent[inliers] >= border_limit))
    elif prefer == "top":
        border_touch_ratio = float(np.mean(dependent[inliers] <= border_limit))
    else:
        border_touch_ratio = float(np.mean(dependent[inliers] >= border_limit))

    return LineFit(
        model=model,
        slope=float(slope),
        intercept=float(intercept),
        support_min=support_min,
        support_max=support_max,
        inlier_count=int(inliers.sum()),
        span=float(span),
        residual=mean_residual,
        border_touch_ratio=border_touch_ratio,
        mean_value=mean_value,
    )


def fit_one_side(mask: np.ndarray, side: str) -> LineFit | None:
    left, right, widths = extract_row_borders(mask)
    rows = np.flatnonzero(widths > 0)
    if rows.size < 8:
        return None

    if side == "left":
        dependent = left
        border_limit = 1.5
        unclipped_rows = rows[left[rows] > 1]
        prefer = "left"
    else:
        dependent = right
        border_limit = mask.shape[1] - 2.5
        unclipped_rows = rows[right[rows] < mask.shape[1] - 2]
        prefer = "right"

    use_rows = rows
    if unclipped_rows.size >= 6 and float(unclipped_rows.max() - unclipped_rows.min()) >= 10.0:
        use_rows = unclipped_rows

    return fit_line_exhaustive(
        use_rows,
        dependent[use_rows],
        model="x_from_y",
        slope_range=(-2.5, 2.5),
        residual_threshold=1.35,
        min_inliers=6,
        min_span=10.0,
        border_limit=border_limit,
        prefer=prefer,
    )


def score_side_fit(line: LineFit | None) -> float:
    if line is None:
        return -1e9
    return (
        line.inlier_count * 5.0
        + line.span * 1.8
        - line.residual * 10.0
        - line.border_touch_ratio * 30.0
        + abs(line.slope) * 10.0
        + abs(line.slope) * line.span * 3.0
    )


def fit_side_lines_from_masks(visible_mask: np.ndarray, outer_mask: np.ndarray) -> tuple[LineFit | None, LineFit | None]:
    visible_left = fit_one_side(visible_mask, "left")
    visible_right = fit_one_side(visible_mask, "right")
    outer_left = fit_one_side(outer_mask, "left")
    outer_right = fit_one_side(outer_mask, "right")

    left_fit = visible_left if score_side_fit(visible_left) >= score_side_fit(outer_left) else outer_left
    right_fit = visible_right if score_side_fit(visible_right) >= score_side_fit(outer_right) else outer_right
    return left_fit, right_fit


def fit_horizontal_lines(mask: np.ndarray) -> tuple[LineFit | None, LineFit | None]:
    top, bottom = extract_column_borders(mask)
    top_cols = np.flatnonzero(top >= 0)
    bottom_cols = np.flatnonzero(bottom >= 0)

    top_fit = fit_line_exhaustive(
        top_cols,
        top[top_cols],
        model="y_from_x",
        slope_range=(-0.32, 0.32),
        residual_threshold=1.2,
        min_inliers=6,
        min_span=8.0,
        border_limit=1.5,
        prefer="top",
    )
    bottom_fit = fit_line_exhaustive(
        bottom_cols,
        bottom[bottom_cols],
        model="y_from_x",
        slope_range=(-0.32, 0.32),
        residual_threshold=1.2,
        min_inliers=6,
        min_span=8.0,
        border_limit=mask.shape[0] - 2.5,
        prefer="bottom",
    )
    return top_fit, bottom_fit


def longest_contiguous_indices(values: np.ndarray) -> np.ndarray:
    if values.size == 0:
        return values
    best_start = 0
    best_end = 0
    start = 0
    for index in range(1, values.size):
        if values[index] != values[index - 1] + 1:
            if index - 1 - start > best_end - best_start:
                best_start = start
                best_end = index - 1
            start = index
    if values.size - 1 - start > best_end - best_start:
        best_start = start
        best_end = values.size - 1
    return values[best_start : best_end + 1]


def fit_pink_from_plateau(mask: np.ndarray) -> LineFit | None:
    top, _ = extract_column_borders(mask)
    cols = np.flatnonzero(top >= 0)
    if cols.size < 6:
        return None

    min_top = int(top[cols].min())
    for tolerance in (0, 1, 2, 3):
        candidates = cols[top[cols] <= min_top + tolerance]
        span = longest_contiguous_indices(candidates)
        if span.size < 6:
            continue
        fit = fit_line_exhaustive(
            span,
            top[span],
            model="y_from_x",
            slope_range=(-0.32, 0.32),
            residual_threshold=0.9,
            min_inliers=4,
            min_span=6.0,
            border_limit=1.5,
            prefer="top",
        )
        if fit is not None:
            return fit
    return None


def top_anchor_point(mask: np.ndarray, side: str) -> tuple[int, int] | None:
    top, _ = extract_column_borders(mask)
    cols = np.flatnonzero(top >= 0)
    if cols.size == 0:
        return None

    if side == "left":
        edge_cols = cols[: min(8, cols.size)]
        x = int(edge_cols[0])
    else:
        edge_cols = cols[max(0, cols.size - 8) :]
        x = int(edge_cols[-1])
    y = int(round(float(np.median(top[edge_cols]))))
    return x, y


def should_draw_left(line: LineFit | None) -> bool:
    if line is None or line.inlier_count < 6 or line.span < 10.0:
        return False
    return not (line.border_touch_ratio >= 0.75 and abs(line.slope) <= 0.25 and line.mean_value <= 1.6)


def should_draw_right(line: LineFit | None, image_width: int) -> bool:
    if line is None or line.inlier_count < 6 or line.span < 10.0:
        return False
    return not (
        line.border_touch_ratio >= 0.75
        and abs(line.slope) <= 0.25
        and line.mean_value >= image_width - 2.6
    )


def should_draw_pink(line: LineFit | None) -> bool:
    if line is None or line.inlier_count < 6 or line.span < 8.0:
        return False
    return not (line.border_touch_ratio >= 0.75 and line.mean_value <= 1.6)


def should_draw_yellow(line: LineFit | None, image_height: int) -> bool:
    if line is None or line.inlier_count < 6 or line.span < 8.0:
        return False
    return not (line.border_touch_ratio >= 0.55 and line.mean_value >= image_height - 2.6)


def intersect_side_with_horizontal(side: LineFit, horizontal: LineFit) -> tuple[float, float] | None:
    denominator = 1.0 - horizontal.slope * side.slope
    if abs(denominator) < 1e-6:
        return None
    y = (horizontal.slope * side.intercept + horizontal.intercept) / denominator
    x = side.x_at_y(y)
    if not np.isfinite(x) or not np.isfinite(y):
        return None
    return float(x), float(y)


def clip_point(x: float, y: float, image_width: int, image_height: int) -> tuple[int, int]:
    x = int(round(float(np.clip(x, 0, image_width - 1))))
    y = int(round(float(np.clip(y, 0, image_height - 1))))
    return x, y


def segment_from_side(
    line: LineFit | None,
    *,
    side: str,
    bottom_row: int,
    image_width: int,
    image_height: int,
) -> list[int] | None:
    if line is None:
        return None

    y0 = line.support_min
    y1 = line.support_max
    if bottom_row - y1 >= 8:
        x_bottom = line.x_at_y(bottom_row)
        if side == "left" and x_bottom > 2.5:
            y1 = float(bottom_row)
        if side == "right" and x_bottom < image_width - 3.5:
            y1 = float(bottom_row)

    x0, y0c = clip_point(line.x_at_y(y0), y0, image_width, image_height)
    x1, y1c = clip_point(line.x_at_y(y1), y1, image_width, image_height)
    return [x0, y0c, x1, y1c]


def segment_from_horizontal(
    line: LineFit | None,
    *,
    image_width: int,
    image_height: int,
    left_line: LineFit | None,
    right_line: LineFit | None,
    draw_left: bool,
    draw_right: bool,
) -> list[int] | None:
    if line is None:
        return None

    x0 = line.support_min
    x1 = line.support_max
    p0 = (x0, line.y_at_x(x0))
    p1 = (x1, line.y_at_x(x1))

    if draw_left and left_line is not None:
        hit = intersect_side_with_horizontal(left_line, line)
        if hit is not None and x0 - 8.0 <= hit[0] <= x0 + 8.0:
            p0 = hit
    if draw_right and right_line is not None:
        hit = intersect_side_with_horizontal(right_line, line)
        if hit is not None and x1 - 8.0 <= hit[0] <= x1 + 8.0:
            p1 = hit

    x0c, y0c = clip_point(p0[0], p0[1], image_width, image_height)
    x1c, y1c = clip_point(p1[0], p1[1], image_width, image_height)
    return [x0c, y0c, x1c, y1c]


def infer_pvc_state(candidate: PvcCandidate | None, image_height: int, yellow_visible: bool) -> tuple[bool, str]:
    if candidate is None:
        return False, STATE_NONE
    if candidate.score < MIN_VALID_SCORE or candidate.edge_contrast < 20.0:
        return False, STATE_NONE
    if yellow_visible or candidate.bottom_row <= image_height - 10:
        return True, STATE_PREPARE_ENTER

    exit_clip_ratio = max(candidate.left_clip_ratio, candidate.right_clip_ratio)
    if exit_clip_ratio >= 0.82:
        return True, STATE_PREPARE_EXIT
    if exit_clip_ratio >= 0.68 and candidate.start_row >= 18:
        return True, STATE_PREPARE_EXIT
    return True, STATE_ON_PVC


def analyze_frame(item: dict[str, Any]) -> tuple[BridgeResult, Image.Image, Image.Image]:
    if "image_path" in item:
        image_path = Path(item["image_path"])
    else:
        image_path = FRAMES_DIR / item["video"] / item["frame"]
    with Image.open(image_path) as image:
        gray = np.array(image.convert("L"), dtype=np.uint8)

    candidate = detect_pvc_candidate(gray)

    if candidate is not None:
        left_fit, right_fit = fit_side_lines_from_masks(candidate.visible_mask, candidate.outer_mask)
        top_fit, yellow_fit = fit_horizontal_lines(candidate.outer_mask)
        plateau_pink_fit = fit_pink_from_plateau(candidate.outer_mask)
        left_visible = should_draw_left(left_fit)
        right_visible = should_draw_right(right_fit, gray.shape[1])
        if left_visible and right_visible:
            pink_visible = plateau_pink_fit is not None or should_draw_pink(top_fit)
        else:
            pink_visible = should_draw_pink(top_fit)
        yellow_visible = should_draw_yellow(yellow_fit, gray.shape[0])
    else:
        left_fit = None
        right_fit = None
        top_fit = None
        plateau_pink_fit = None
        yellow_fit = None
        left_visible = False
        right_visible = False
        pink_visible = False
        yellow_visible = False

    pvc_found, pvc_state = infer_pvc_state(candidate, gray.shape[0], yellow_visible)

    if pvc_found and candidate is not None:
        left_segment = segment_from_side(
            left_fit if left_visible else None,
            side="left",
            bottom_row=candidate.bottom_row,
            image_width=gray.shape[1],
            image_height=gray.shape[0],
        )
        right_segment = segment_from_side(
            right_fit if right_visible else None,
            side="right",
            bottom_row=candidate.bottom_row,
            image_width=gray.shape[1],
            image_height=gray.shape[0],
        )

        pink_fit = plateau_pink_fit if plateau_pink_fit is not None else top_fit
        pink_segment = None
        if pink_visible:
            if left_segment is not None and right_segment is not None:
                left_top = (left_segment[0], left_segment[1])
                right_top = (right_segment[0], right_segment[1])
                if pink_fit is not None and left_fit is not None and right_fit is not None:
                    left_hit = intersect_side_with_horizontal(left_fit, pink_fit)
                    right_hit = intersect_side_with_horizontal(right_fit, pink_fit)
                    if left_hit is not None:
                        left_top = clip_point(left_hit[0], left_hit[1], gray.shape[1], gray.shape[0])
                    if right_hit is not None:
                        right_top = clip_point(right_hit[0], right_hit[1], gray.shape[1], gray.shape[0])
                pink_segment = [left_top[0], left_top[1], right_top[0], right_top[1]]
                left_segment[0], left_segment[1] = left_top
                right_segment[0], right_segment[1] = right_top
            else:
                if right_segment is not None:
                    anchor = top_anchor_point(candidate.outer_mask, "left")
                    if anchor is not None:
                        pink_segment = [anchor[0], anchor[1], right_segment[0], right_segment[1]]
                elif left_segment is not None:
                    anchor = top_anchor_point(candidate.outer_mask, "right")
                    if anchor is not None:
                        pink_segment = [left_segment[0], left_segment[1], anchor[0], anchor[1]]
                if pink_segment is None:
                    pink_segment = segment_from_horizontal(
                        pink_fit if pink_fit is not None else top_fit,
                        image_width=gray.shape[1],
                        image_height=gray.shape[0],
                        left_line=left_fit,
                        right_line=right_fit,
                        draw_left=left_visible,
                        draw_right=right_visible,
                    )

        yellow_segment = None
        if yellow_visible:
            if left_segment is not None and right_segment is not None and yellow_fit is not None:
                left_bottom = (left_segment[2], left_segment[3])
                right_bottom = (right_segment[2], right_segment[3])
                left_hit = intersect_side_with_horizontal(left_fit, yellow_fit) if left_fit is not None else None
                right_hit = intersect_side_with_horizontal(right_fit, yellow_fit) if right_fit is not None else None
                if left_hit is not None:
                    left_bottom = clip_point(left_hit[0], left_hit[1], gray.shape[1], gray.shape[0])
                if right_hit is not None:
                    right_bottom = clip_point(right_hit[0], right_hit[1], gray.shape[1], gray.shape[0])
                yellow_segment = [left_bottom[0], left_bottom[1], right_bottom[0], right_bottom[1]]
                left_segment[2], left_segment[3] = left_bottom
                right_segment[2], right_segment[3] = right_bottom
            else:
                yellow_segment = segment_from_horizontal(
                    yellow_fit,
                    image_width=gray.shape[1],
                    image_height=gray.shape[0],
                    left_line=left_fit,
                    right_line=right_fit,
                    draw_left=left_visible,
                    draw_right=right_visible,
                )

        threshold = candidate.threshold
        area = candidate.area
        area_ratio = round(candidate.area_ratio, 4)
        top_row = candidate.top_row
        start_row = candidate.start_row
        bottom_row = candidate.bottom_row
        max_width = candidate.max_width
        bottom_width = candidate.bottom_width
        pvc_center_x = round(candidate.center_x, 3)
        edge_contrast = round(candidate.edge_contrast, 3)
        left_clip_ratio = round(candidate.left_clip_ratio, 3)
        right_clip_ratio = round(candidate.right_clip_ratio, 3)
        dual_clip_ratio = round(candidate.dual_clip_ratio, 3)
        border_monotonic = round(candidate.border_monotonic, 3)
        candidate_score = round(candidate.score, 3)
        mask_image = Image.fromarray((candidate.outer_mask.astype(np.uint8) * 255), mode="L")
    else:
        left_segment = None
        right_segment = None
        pink_segment = None
        yellow_segment = None
        threshold = candidate.threshold if candidate is not None else -1
        area = candidate.area if candidate is not None else 0
        area_ratio = round(candidate.area_ratio, 4) if candidate is not None else 0.0
        top_row = candidate.top_row if candidate is not None else -1
        start_row = candidate.start_row if candidate is not None else -1
        bottom_row = candidate.bottom_row if candidate is not None else -1
        max_width = candidate.max_width if candidate is not None else 0
        bottom_width = candidate.bottom_width if candidate is not None else 0
        pvc_center_x = round(candidate.center_x, 3) if candidate is not None else None
        edge_contrast = round(candidate.edge_contrast, 3) if candidate is not None else 0.0
        left_clip_ratio = round(candidate.left_clip_ratio, 3) if candidate is not None else 0.0
        right_clip_ratio = round(candidate.right_clip_ratio, 3) if candidate is not None else 0.0
        dual_clip_ratio = round(candidate.dual_clip_ratio, 3) if candidate is not None else 0.0
        border_monotonic = round(candidate.border_monotonic, 3) if candidate is not None else 0.0
        candidate_score = round(candidate.score, 3) if candidate is not None else 0.0
        mask_image = Image.fromarray(np.zeros_like(gray, dtype=np.uint8), mode="L")
        left_visible = False
        right_visible = False
        pink_visible = False
        yellow_visible = False

    expected_state = item.get("expected_state")
    state_match = None if expected_state is None else bool(expected_state == pvc_state)

    result = BridgeResult(
        video=item["video"],
        frame=item["frame"],
        tag=item.get("tag", ""),
        expected_state=expected_state,
        state_match=state_match,
        threshold=threshold,
        pvc_found=pvc_found,
        pvc_state=pvc_state,
        pvc_area=area,
        pvc_area_ratio=area_ratio,
        pvc_top_row=top_row,
        pvc_start_row=start_row,
        pvc_bottom_row=bottom_row,
        pvc_max_width=max_width,
        pvc_bottom_width=bottom_width,
        pvc_center_x=pvc_center_x,
        edge_contrast=edge_contrast,
        left_clip_ratio=left_clip_ratio,
        right_clip_ratio=right_clip_ratio,
        dual_clip_ratio=dual_clip_ratio,
        border_monotonic=border_monotonic,
        candidate_score=candidate_score,
        left_line_visible=left_visible,
        right_line_visible=right_visible,
        pink_line_visible=pink_visible,
        yellow_line_visible=yellow_visible,
        left_line_segment=left_segment,
        right_line_segment=right_segment,
        pink_line_segment=pink_segment,
        yellow_line_segment=yellow_segment,
    )

    draw_image = render_draw(gray, result)
    return result, draw_image, mask_image


def render_draw(gray: np.ndarray, result: BridgeResult) -> Image.Image:
    rgb = np.stack([gray, gray, gray], axis=-1).astype(np.uint8)
    image = Image.fromarray(rgb, mode="RGB")
    draw = ImageDraw.Draw(image)

    for segment, color in [
        (result.left_line_segment, (255, 0, 0)),
        (result.right_line_segment, (0, 150, 255)),
        (result.pink_line_segment, (255, 105, 180)),
        (result.yellow_line_segment, (255, 220, 0)),
    ]:
        if segment is None:
            continue
        draw.line(tuple(segment), fill=color, width=1)

    return image


def build_contact_sheet(images: list[Image.Image], columns: int = 3, scale: int = 4) -> Image.Image:
    if not images:
        raise ValueError("No images to place on the contact sheet.")
    tile_w, tile_h = images[0].size
    tile_w *= scale
    tile_h *= scale
    rows = (len(images) + columns - 1) // columns
    sheet = Image.new("RGB", (columns * tile_w, rows * tile_h), color=(15, 15, 15))
    for index, image in enumerate(images):
        col = index % columns
        row = index // columns
        resized = image.resize((tile_w, tile_h), resample=Image.Resampling.NEAREST)
        sheet.paste(resized, (col * tile_w, row * tile_h))
    return sheet


def build_pair_contact_sheet(
    originals: list[Image.Image],
    draws: list[Image.Image],
    columns: int = 3,
    scale: int = 4,
) -> Image.Image:
    if not originals or len(originals) != len(draws):
        raise ValueError("Original and draw image lists must be non-empty and aligned.")

    tile_w, tile_h = originals[0].size
    tile_w *= scale
    tile_h *= scale
    rows = (len(originals) + columns - 1) // columns
    sheet = Image.new("RGB", (columns * tile_w * 2, rows * tile_h), color=(15, 15, 15))

    for index, (original, draw_image) in enumerate(zip(originals, draws)):
        col = index % columns
        row = index // columns
        x = col * tile_w * 2
        y = row * tile_h
        original_resized = original.resize((tile_w, tile_h), resample=Image.Resampling.NEAREST)
        draw_resized = draw_image.resize((tile_w, tile_h), resample=Image.Resampling.NEAREST)
        sheet.paste(original_resized, (x, y))
        sheet.paste(draw_resized, (x + tile_w, y))
    return sheet


def build_compare_image(original: Image.Image, draw_image: Image.Image, scale: int = 4) -> Image.Image:
    width, height = original.size
    canvas = Image.new("RGB", (width * scale * 2, height * scale), color=(15, 15, 15))
    original_resized = original.resize((width * scale, height * scale), resample=Image.Resampling.NEAREST)
    draw_resized = draw_image.resize((width * scale, height * scale), resample=Image.Resampling.NEAREST)
    canvas.paste(original_resized, (0, 0))
    canvas.paste(draw_resized, (width * scale, 0))
    return canvas


def write_summary_csv(path: Path, results: list[BridgeResult]) -> None:
    fieldnames = [
        "video",
        "frame",
        "tag",
        "expected_state",
        "state_match",
        "threshold",
        "pvc_found",
        "pvc_state",
        "pvc_area",
        "pvc_area_ratio",
        "pvc_top_row",
        "pvc_start_row",
        "pvc_bottom_row",
        "pvc_max_width",
        "pvc_bottom_width",
        "pvc_center_x",
        "edge_contrast",
        "left_clip_ratio",
        "right_clip_ratio",
        "dual_clip_ratio",
        "border_monotonic",
        "candidate_score",
        "left_line_visible",
        "right_line_visible",
        "pink_line_visible",
        "yellow_line_visible",
        "left_line_segment",
        "right_line_segment",
        "pink_line_segment",
        "yellow_line_segment",
    ]
    with path.open("w", encoding="utf-8-sig", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        for result in results:
            writer.writerow({name: getattr(result, name) for name in fieldnames})


def write_readme(output_dir: Path) -> None:
    readme = "\n".join(
        [
            "\u5355\u8fb9\u6865 PVC \u8bc6\u522b\u539f\u578b\u8f93\u51fa\u8bf4\u660e",
            "",
            "draw/: \u4ec5\u4fdd\u7559\u56db\u6761\u76f4\u7ebf\u6bb5\u7ed3\u679c\u56fe\u3002",
            "originals/: \u5bf9\u5e94\u539f\u56fe\u62f7\u8d1d\uff0c\u4fbf\u4e8e\u9010\u5f20\u5bf9\u6bd4\u3002",
            "masks/: PVC \u5916\u8f6e\u5ed3\u63a9\u7801\u3002",
            "compare/: \u6bcf\u5f20\u56fe\u7684 \u539f\u56fe | \u5de1\u7ebf\u56fe \u5e76\u6392\u5bf9\u7167\u56fe\u3002",
            "",
            "\u989c\u8272\u7ea6\u5b9a\uff1a",
            "\u7ea2\u8272 = \u5de6\u7ebf",
            "\u84dd\u8272 = \u53f3\u7ebf",
            "\u7c89\u8272 = \u4e0a\u8fb9\u7ebf",
            "\u9ec4\u8272 = \u4e0b\u8fb9\u7ebf",
            "",
            "\u6291\u5236\u89c4\u5219\uff1a",
            "\u9ec4\u7ebf\u8d34\u5e95\u5219\u4e0d\u753b\u3002",
            "\u7c89\u7ebf\u8d34\u9876\u5219\u4e0d\u753b\u3002",
            "\u5de6\u7ebf\u8d34\u5de6\u8fb9\u4e14\u8fd1\u4f3c\u7ad6\u76f4\u5219\u4e0d\u753b\u3002",
            "\u53f3\u7ebf\u8d34\u53f3\u8fb9\u4e14\u8fd1\u4f3c\u7ad6\u76f4\u5219\u4e0d\u753b\u3002",
        ]
    )
    (output_dir / "README.txt").write_text(readme, encoding="utf-8")
    (output_dir / "README.md").write_text(readme, encoding="utf-8")


def collect_frame_paths(frame_dir: Path) -> list[Path]:
    return sorted(path for path in frame_dir.iterdir() if path.is_file() and path.suffix.lower() in IMAGE_SUFFIXES)


def read_avi_fps(video_path: Path, default_fps: float = DEFAULT_SOURCE_FPS) -> float:
    if not video_path.exists():
        return default_fps
    data = video_path.read_bytes()
    index = data.find(b"avih")
    if index < 0 or index + 8 + 4 > len(data):
        return default_fps
    microseconds = struct.unpack_from("<I", data, index + 8)[0]
    if microseconds <= 0:
        return default_fps
    return 1_000_000.0 / float(microseconds)


def encode_jpeg(image: Image.Image, quality: int = JPEG_QUALITY) -> bytes:
    buffer = io.BytesIO()
    image.save(buffer, format="JPEG", quality=quality, optimize=False)
    return buffer.getvalue()


def write_chunk(file, fourcc: bytes, payload: bytes) -> None:
    file.write(fourcc)
    file.write(struct.pack("<I", len(payload)))
    file.write(payload)
    if len(payload) & 1:
        file.write(b"\x00")


def write_list(file, list_type: bytes, payload: bytes) -> None:
    file.write(b"LIST")
    file.write(struct.pack("<I", len(payload) + 4))
    file.write(list_type)
    file.write(payload)


def write_mjpeg_avi(images: list[Image.Image], video_path: Path, fps: float) -> None:
    if not images:
        raise ValueError("No images provided for video generation.")

    rgb_images = [image.convert("RGB") for image in images]
    width, height = rgb_images[0].size
    jpeg_frames = [encode_jpeg(image) for image in rgb_images]
    frame_count = len(jpeg_frames)
    max_frame_size = max(len(frame) for frame in jpeg_frames)
    average_bytes_per_second = int(sum(len(frame) for frame in jpeg_frames) * fps / max(frame_count, 1))
    microseconds_per_frame = int(round(1_000_000.0 / max(fps, 1e-6)))

    stream_scale = 1
    stream_rate = max(1, int(round(fps)))
    compression = int.from_bytes(b"MJPG", byteorder="little", signed=False)

    avih_payload = struct.pack(
        "<IIIIIIIIII4I",
        microseconds_per_frame,
        average_bytes_per_second,
        0,
        0x10,
        frame_count,
        0,
        1,
        max_frame_size,
        width,
        height,
        0,
        0,
        0,
        0,
    )

    strh_payload = struct.pack(
        "<4s4sIHHIIIIIIIIhhhh",
        b"vids",
        b"MJPG",
        0,
        0,
        0,
        0,
        stream_scale,
        stream_rate,
        0,
        frame_count,
        max_frame_size,
        0xFFFFFFFF,
        0,
        0,
        0,
        width,
        height,
    )

    strf_payload = struct.pack(
        "<IiiHHIIiiII",
        40,
        width,
        height,
        1,
        24,
        compression,
        width * height * 3,
        0,
        0,
        0,
        0,
    )

    strl_payload = b"".join(
        [
            b"strh",
            struct.pack("<I", len(strh_payload)),
            strh_payload,
            b"strf",
            struct.pack("<I", len(strf_payload)),
            strf_payload,
        ]
    )
    hdrl_payload = b"".join(
        [
            b"avih",
            struct.pack("<I", len(avih_payload)),
            avih_payload,
            b"LIST",
            struct.pack("<I", len(strl_payload) + 4),
            b"strl",
            strl_payload,
        ]
    )

    movi_buffer = io.BytesIO()
    idx_entries: list[bytes] = []
    chunk_offset = 4
    for frame in jpeg_frames:
        movi_buffer.write(b"00dc")
        movi_buffer.write(struct.pack("<I", len(frame)))
        movi_buffer.write(frame)
        if len(frame) & 1:
            movi_buffer.write(b"\x00")
        idx_entries.append(struct.pack("<4sIII", b"00dc", 0x10, chunk_offset, len(frame)))
        chunk_offset += 8 + len(frame) + (len(frame) & 1)

    movi_payload = movi_buffer.getvalue()
    idx_payload = b"".join(idx_entries)

    riff_size = (
        4
        + (8 + len(hdrl_payload) + 4)
        + (8 + len(movi_payload) + 4)
        + (8 + len(idx_payload))
    )

    with video_path.open("wb") as file:
        file.write(b"RIFF")
        file.write(struct.pack("<I", riff_size))
        file.write(b"AVI ")
        write_list(file, b"hdrl", hdrl_payload)
        write_list(file, b"movi", movi_payload)
        write_chunk(file, b"idx1", idx_payload)


def write_frame_timing_csv(
    path: Path,
    frame_names: list[str],
    analysis_times_ms: list[float],
    results: list[BridgeResult],
) -> None:
    with path.open("w", encoding="utf-8-sig", newline="") as file:
        writer = csv.DictWriter(
            file,
            fieldnames=[
                "frame",
                "analysis_ms",
                "pvc_state",
                "pvc_found",
                "left_line_visible",
                "right_line_visible",
                "pink_line_visible",
                "yellow_line_visible",
            ],
        )
        writer.writeheader()
        for frame_name, analysis_ms, result in zip(frame_names, analysis_times_ms, results):
            writer.writerow(
                {
                    "frame": frame_name,
                    "analysis_ms": round(analysis_ms, 6),
                    "pvc_state": result.pvc_state,
                    "pvc_found": result.pvc_found,
                    "left_line_visible": result.left_line_visible,
                    "right_line_visible": result.right_line_visible,
                    "pink_line_visible": result.pink_line_visible,
                    "yellow_line_visible": result.yellow_line_visible,
                }
            )


def summarize_folder_timings(
    folder_name: str,
    output_dir: Path,
    frame_names: list[str],
    analysis_times_ms: list[float],
    results: list[BridgeResult],
    fps: float,
    video_path: Path,
    video_encode_ms: float,
    total_wall_ms: float,
) -> FolderTimingStats:
    values = np.array(analysis_times_ms, dtype=np.float64)
    max_index = int(values.argmax())
    min_index = int(values.argmin())
    state_counts = dict(Counter(result.pvc_state for result in results))
    return FolderTimingStats(
        folder_name=folder_name,
        output_dir=str(output_dir),
        frame_count=len(frame_names),
        fps=round(float(fps), 6),
        avg_recognition_ms=round(float(values.mean()), 6),
        median_recognition_ms=round(float(np.median(values)), 6),
        p95_recognition_ms=round(float(np.percentile(values, 95)), 6),
        min_recognition_ms=round(float(values[min_index]), 6),
        min_recognition_frame=frame_names[min_index],
        max_recognition_ms=round(float(values[max_index]), 6),
        max_recognition_frame=frame_names[max_index],
        total_recognition_ms=round(float(values.sum()), 6),
        video_encode_ms=round(float(video_encode_ms), 6),
        total_wall_ms=round(float(total_wall_ms), 6),
        state_counts=state_counts,
        video_path=str(video_path),
    )


def write_timing_summary(output_dir: Path, stats: FolderTimingStats) -> None:
    summary_json = output_dir / "timing_summary.json"
    summary_txt = output_dir / "timing_summary.txt"
    summary_json.write_text(json.dumps(asdict(stats), ensure_ascii=False, indent=2), encoding="utf-8")

    lines = [
        f"folder: {stats.folder_name}",
        f"frame_count: {stats.frame_count}",
        f"fps: {stats.fps}",
        f"avg_recognition_ms: {stats.avg_recognition_ms}",
        f"median_recognition_ms: {stats.median_recognition_ms}",
        f"p95_recognition_ms: {stats.p95_recognition_ms}",
        f"min_recognition_ms: {stats.min_recognition_ms} ({stats.min_recognition_frame})",
        f"max_recognition_ms: {stats.max_recognition_ms} ({stats.max_recognition_frame})",
        f"total_recognition_ms: {stats.total_recognition_ms}",
        f"video_encode_ms: {stats.video_encode_ms}",
        f"total_wall_ms: {stats.total_wall_ms}",
        f"video_path: {stats.video_path}",
        f"state_counts: {json.dumps(stats.state_counts, ensure_ascii=False)}",
    ]
    summary_txt.write_text("\n".join(lines), encoding="utf-8")


def default_output_dir_for_frame_dir(frame_dir: Path) -> Path:
    return frame_dir.parent / f"{frame_dir.name}_output"


def resolve_frame_dir(input_dir: Path) -> Path:
    if input_dir.is_absolute():
        return input_dir
    candidate = input_dir
    if candidate.exists():
        return candidate.resolve()
    candidate = FRAMES_DIR / input_dir
    if candidate.exists():
        return candidate.resolve()
    raise FileNotFoundError(f"Frame directory not found: {input_dir}")


def process_frame_directory(frame_dir: Path, output_dir: Path) -> FolderTimingStats:
    frame_paths = collect_frame_paths(frame_dir)
    if not frame_paths:
        raise ValueError(f"No image frames found in {frame_dir}")

    draw_dir = output_dir / "draw"
    mask_dir = output_dir / "masks"
    draw_dir.mkdir(parents=True, exist_ok=True)
    mask_dir.mkdir(parents=True, exist_ok=True)

    results: list[BridgeResult] = []
    draw_images: list[Image.Image] = []
    frame_names: list[str] = []
    analysis_times_ms: list[float] = []

    folder_start = time.perf_counter()

    for frame_path in frame_paths:
        item = {
            "video": frame_dir.name,
            "frame": frame_path.name,
            "tag": "",
            "image_path": str(frame_path),
        }

        t0 = time.perf_counter()
        result, draw_image, mask_image = analyze_frame(item)
        analysis_ms = (time.perf_counter() - t0) * 1000.0

        results.append(result)
        frame_names.append(frame_path.name)
        analysis_times_ms.append(analysis_ms)
        draw_images.append(draw_image.copy())

        draw_image.save(draw_dir / frame_path.name, format="PNG")
        mask_image.save(mask_dir / frame_path.name, format="PNG")

    write_summary_csv(output_dir / "summary.csv", results)
    (output_dir / "summary.json").write_text(
        json.dumps([asdict(result) for result in results], ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    write_frame_timing_csv(output_dir / "frame_timing.csv", frame_names, analysis_times_ms, results)

    fps = read_avi_fps(DATA_ROOT / f"{frame_dir.name}.avi")
    video_path = output_dir / f"{frame_dir.name}_draw.avi"
    video_start = time.perf_counter()
    write_mjpeg_avi(draw_images, video_path, fps)
    video_encode_ms = (time.perf_counter() - video_start) * 1000.0

    total_wall_ms = (time.perf_counter() - folder_start) * 1000.0
    stats = summarize_folder_timings(
        folder_name=frame_dir.name,
        output_dir=output_dir,
        frame_names=frame_names,
        analysis_times_ms=analysis_times_ms,
        results=results,
        fps=fps,
        video_path=video_path,
        video_encode_ms=video_encode_ms,
        total_wall_ms=total_wall_ms,
    )
    write_timing_summary(output_dir, stats)
    return stats


def build_subset_outputs(output_dir: Path, subset: dict[str, Any]) -> None:
    draw_dir = output_dir / "draw"
    mask_dir = output_dir / "masks"
    original_dir = output_dir / "originals"
    compare_dir = output_dir / "compare"
    draw_dir.mkdir(parents=True, exist_ok=True)
    mask_dir.mkdir(parents=True, exist_ok=True)
    original_dir.mkdir(parents=True, exist_ok=True)
    compare_dir.mkdir(parents=True, exist_ok=True)

    results: list[BridgeResult] = []
    original_images: list[Image.Image] = []
    draw_images: list[Image.Image] = []
    mask_images: list[Image.Image] = []

    for item in subset["items"]:
        image_path = FRAMES_DIR / item["video"] / item["frame"]
        with Image.open(image_path) as image:
            original_image = image.convert("RGB")

        result, draw_image, mask_image = analyze_frame(item)
        results.append(result)
        original_images.append(original_image)
        draw_images.append(draw_image)
        mask_images.append(mask_image.convert("RGB"))

        output_name = f"{item['video']}__{item['frame']}"
        original_image.save(original_dir / output_name, format="PNG")
        draw_image.save(draw_dir / output_name, format="PNG")
        mask_image.save(mask_dir / output_name, format="PNG")
        build_compare_image(original_image, draw_image).save(compare_dir / output_name, format="PNG")

    write_summary_csv(output_dir / "summary.csv", results)
    (output_dir / "summary.json").write_text(
        json.dumps([asdict(result) for result in results], ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    (output_dir / "subset_used.json").write_text(
        json.dumps(subset, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    original_contact_sheet = build_contact_sheet(original_images)
    draw_contact_sheet = build_contact_sheet(draw_images)
    mask_contact_sheet = build_contact_sheet(mask_images)
    pair_contact_sheet = build_pair_contact_sheet(original_images, draw_images)

    original_contact_sheet.save(output_dir / "original_contact_sheet.png", format="PNG")
    draw_contact_sheet.save(output_dir / "draw_contact_sheet.png", format="PNG")
    draw_contact_sheet.save(output_dir / "contact_sheet.png", format="PNG")
    mask_contact_sheet.save(output_dir / "mask_contact_sheet.png", format="PNG")
    pair_contact_sheet.save(output_dir / "original_vs_draw_contact_sheet.png", format="PNG")
    pair_contact_sheet.save(output_dir / "original_vs_overlay_contact_sheet.png", format="PNG")
    write_readme(output_dir)


def write_batch_summary(stats_list: list[FolderTimingStats]) -> None:
    if not stats_list:
        return
    csv_path = DATA_ROOT / "frames_batch_summary.csv"
    json_path = DATA_ROOT / "frames_batch_summary.json"
    txt_path = DATA_ROOT / "frames_batch_summary.txt"

    fieldnames = [
        "folder_name",
        "output_dir",
        "frame_count",
        "fps",
        "avg_recognition_ms",
        "median_recognition_ms",
        "p95_recognition_ms",
        "min_recognition_ms",
        "min_recognition_frame",
        "max_recognition_ms",
        "max_recognition_frame",
        "total_recognition_ms",
        "video_encode_ms",
        "total_wall_ms",
        "video_path",
        "state_counts",
    ]
    with csv_path.open("w", encoding="utf-8-sig", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        for stats in stats_list:
            row = asdict(stats)
            row["state_counts"] = json.dumps(stats.state_counts, ensure_ascii=False)
            writer.writerow(row)

    json_path.write_text(
        json.dumps([asdict(stats) for stats in stats_list], ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    total_frames = sum(stats.frame_count for stats in stats_list)
    total_recognition_ms = sum(stats.total_recognition_ms for stats in stats_list)
    total_wall_ms = sum(stats.total_wall_ms for stats in stats_list)
    weighted_avg_ms = total_recognition_ms / max(total_frames, 1)
    max_stats = max(stats_list, key=lambda item: item.max_recognition_ms)
    aggregate_states: Counter[str] = Counter()
    for stats in stats_list:
        aggregate_states.update(stats.state_counts)

    lines = [
        f"folder_count: {len(stats_list)}",
        f"total_frames: {total_frames}",
        f"weighted_avg_recognition_ms: {round(weighted_avg_ms, 6)}",
        f"total_recognition_ms: {round(total_recognition_ms, 6)}",
        f"total_wall_ms: {round(total_wall_ms, 6)}",
        (
            "overall_max_recognition_ms: "
            f"{max_stats.max_recognition_ms} "
            f"({max_stats.folder_name} / {max_stats.max_recognition_frame})"
        ),
        f"state_counts: {json.dumps(dict(aggregate_states), ensure_ascii=False)}",
    ]
    txt_path.write_text("\n".join(lines), encoding="utf-8")


def load_timing_stats(summary_path: Path) -> FolderTimingStats:
    data = json.loads(summary_path.read_text(encoding="utf-8"))
    return FolderTimingStats(
        folder_name=str(data["folder_name"]),
        output_dir=str(data["output_dir"]),
        frame_count=int(data["frame_count"]),
        fps=float(data["fps"]),
        avg_recognition_ms=float(data["avg_recognition_ms"]),
        median_recognition_ms=float(data["median_recognition_ms"]),
        p95_recognition_ms=float(data["p95_recognition_ms"]),
        min_recognition_ms=float(data["min_recognition_ms"]),
        min_recognition_frame=str(data["min_recognition_frame"]),
        max_recognition_ms=float(data["max_recognition_ms"]),
        max_recognition_frame=str(data["max_recognition_frame"]),
        total_recognition_ms=float(data["total_recognition_ms"]),
        video_encode_ms=float(data["video_encode_ms"]),
        total_wall_ms=float(data["total_wall_ms"]),
        state_counts={str(key): int(value) for key, value in dict(data["state_counts"]).items()},
        video_path=str(data["video_path"]),
    )


def run_subset_mode(output_dir: Path, subset_path: Path) -> None:
    subset = json.loads(subset_path.read_text(encoding="utf-8"))
    build_subset_outputs(output_dir, subset)
    print(f"Processed {len(subset['items'])} subset frames.")
    print(f"Results written to: {output_dir}")


def run_single_directory(frame_dir: Path, output_dir: Path) -> None:
    stats = process_frame_directory(frame_dir, output_dir)
    print(f"Processed {stats.frame_count} frames from: {frame_dir.name}")
    print(f"Results written to: {output_dir}")
    print(
        "Timing ms: "
        f"avg={stats.avg_recognition_ms:.3f}, "
        f"median={stats.median_recognition_ms:.3f}, "
        f"p95={stats.p95_recognition_ms:.3f}, "
        f"max={stats.max_recognition_ms:.3f} ({stats.max_recognition_frame})"
    )
    print(f"Video written to: {stats.video_path}")


def run_all_frame_dirs() -> None:
    frame_dirs = sorted(
        path
        for path in FRAMES_DIR.iterdir()
        if path.is_dir() and not path.name.endswith("_output")
    )
    stats_list: list[FolderTimingStats] = []
    for frame_dir in frame_dirs:
        output_dir = default_output_dir_for_frame_dir(frame_dir)
        stats = process_frame_directory(frame_dir, output_dir)
        stats_list.append(stats)
        print(
            f"[{frame_dir.name}] frames={stats.frame_count} "
            f"avg_ms={stats.avg_recognition_ms:.3f} "
            f"max_ms={stats.max_recognition_ms:.3f}"
        )
    write_batch_summary(stats_list)
    print(f"Batch summary written to: {DATA_ROOT / 'frames_batch_summary.csv'}")


def run_existing_batch_summary() -> None:
    frame_dirs = sorted(
        path
        for path in FRAMES_DIR.iterdir()
        if path.is_dir() and not path.name.endswith("_output")
    )
    stats_list: list[FolderTimingStats] = []
    for frame_dir in frame_dirs:
        output_dir = default_output_dir_for_frame_dir(frame_dir)
        summary_path = output_dir / "timing_summary.json"
        if not summary_path.exists():
            raise FileNotFoundError(f"Missing timing summary: {summary_path}")
        stats_list.append(load_timing_stats(summary_path))
    write_batch_summary(stats_list)
    print(f"Batch summary written to: {DATA_ROOT / 'frames_batch_summary.csv'}")


def main() -> None:
    parser = argparse.ArgumentParser(description="PVC-only single-bridge prototype with straight-line fitting.")
    parser.add_argument("--subset", type=Path, default=DEFAULT_SUBSET, help="Subset JSON file.")
    parser.add_argument("--output-dir", type=Path, default=None, help="Directory for generated results.")
    parser.add_argument("--input-dir", type=Path, default=None, help="Process all frames from one frame directory.")
    parser.add_argument(
        "--all-frame-dirs",
        action="store_true",
        help="Process all subdirectories under data/单边桥/frames.",
    )
    parser.add_argument(
        "--summarize-existing-outputs",
        action="store_true",
        help="Build the batch summary from existing *_output folders without rerunning recognition.",
    )
    args = parser.parse_args()

    if args.summarize_existing_outputs:
        run_existing_batch_summary()
        return

    if args.all_frame_dirs:
        run_all_frame_dirs()
        return

    if args.input_dir is not None:
        frame_dir = resolve_frame_dir(args.input_dir)
        output_dir = args.output_dir if args.output_dir is not None else default_output_dir_for_frame_dir(frame_dir)
        run_single_directory(frame_dir, output_dir)
        return

    output_dir = args.output_dir if args.output_dir is not None else DEFAULT_OUTPUT
    run_subset_mode(output_dir, args.subset)


if __name__ == "__main__":
    main()
