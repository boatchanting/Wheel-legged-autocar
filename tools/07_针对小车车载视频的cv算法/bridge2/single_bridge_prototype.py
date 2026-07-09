from __future__ import annotations

import argparse
import csv
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

import cv2
import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage
from scipy.spatial import ConvexHull, QhullError


STATE_NONE = "无"
STATE_PREPARE_ENTER = "准备进入"
STATE_ON_PVC = "在PVC上"
STATE_PREPARE_EXIT = "准备退出"

MIN_VALID_SCORE = 350.0


def resolve_project_root() -> Path:
    here = Path(__file__).resolve()
    for parent in here.parents:
        if (parent / "data" / "单边桥").exists():
            return parent
    raise FileNotFoundError("Could not locate project root containing data/单边桥.")


PROJECT_ROOT = resolve_project_root()
DATA_ROOT = PROJECT_ROOT / "data" / "单边桥"
FRAMES_DIR = DATA_ROOT / "frames"
DEFAULT_SUBSET = DATA_ROOT / "single_bridge_subset.json"
DEFAULT_OUTPUT = DATA_ROOT / "python_results"
PEOPLE_DIR = DEFAULT_OUTPUT / "people"


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
    best_slope = 0.0
    best_intercept = 0.0

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
                best_slope = slope
                best_intercept = intercept

    if best_inliers is None or int(best_inliers.sum()) < min_inliers:
        return None

    slope, intercept = np.polyfit(independent[best_inliers], dependent[best_inliers], 1)
    residuals = np.abs(dependent - (slope * independent + intercept))
    inliers = residuals <= max(residual_threshold, float(np.percentile(residuals[best_inliers], 80)) * 1.3)
    if int(inliers.sum()) < min_inliers:
        inliers = best_inliers

    slope, intercept = np.polyfit(independent[inliers], dependent[inliers], 1)
    residuals = np.abs(dependent - (slope * independent + intercept))
    inliers = residuals <= max(residual_threshold, float(np.percentile(residuals[inliers], 80)) * 1.2)
    if int(inliers.sum()) < min_inliers:
        inliers = best_inliers

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


def fit_side_lines(mask: np.ndarray) -> tuple[LineFit | None, LineFit | None]:
    raise NotImplementedError("Use fit_side_lines_from_masks instead.")


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
        slope_range = (-2.5, 0.25)
    else:
        dependent = right
        border_limit = mask.shape[1] - 2.5
        unclipped_rows = rows[right[rows] < mask.shape[1] - 2]
        prefer = "right"
        slope_range = (0.15, 2.5)

    use_rows = rows
    if unclipped_rows.size >= 6 and float(unclipped_rows.max() - unclipped_rows.min()) >= 10.0:
        use_rows = unclipped_rows

    return fit_line_exhaustive(
        use_rows,
        dependent[use_rows],
        model="x_from_y",
        slope_range=slope_range,
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
    else:
        edge_cols = cols[max(0, cols.size - 8) :]

    x = int(edge_cols[0] if side == "left" else edge_cols[-1])
    y = int(round(float(np.median(top[edge_cols]))))
    return x, y


def build_annotation_mask(gray: np.ndarray, candidate: PvcCandidate) -> np.ndarray:
    annotation_mask = candidate.outer_mask.copy()
    threshold_map = build_threshold_map(gray.shape[1], candidate.threshold)
    mask = gray > threshold_map[np.newaxis, :]
    mask = ndimage.binary_closing(mask, structure=np.ones((3, 3), dtype=bool))
    mask = ndimage.binary_opening(mask, structure=np.ones((2, 2), dtype=bool))

    labels, count = ndimage.label(mask)
    rows, cols = np.where(candidate.outer_mask)
    if rows.size == 0:
        return annotation_mask

    min_x = int(cols.min())
    max_x = int(cols.max())
    top_row = int(rows.min())
    merged_cap = False

    for label in range(1, count + 1):
        component = labels == label
        area = int(component.sum())
        if area < 8 or area > 200:
            continue

        comp_rows, comp_cols = np.where(component)
        if comp_rows.size == 0:
            continue

        comp_top = int(comp_rows.min())
        comp_bottom = int(comp_rows.max())
        comp_left = int(comp_cols.min())
        comp_right = int(comp_cols.max())
        overlap = max(0, min(max_x + 4, comp_right) - max(min_x - 4, comp_left) + 1)

        if comp_top < top_row and comp_bottom <= top_row + 1 and top_row - comp_bottom <= 10 and overlap >= 5:
            annotation_mask |= component
            merged_cap = True

    if merged_cap:
        hull_mask = convex_hull_mask(annotation_mask)
        if hull_mask.any():
            annotation_mask = hull_mask
    return annotation_mask


def approximate_mask_polygon(mask: np.ndarray, epsilon_ratio: float = 0.003) -> np.ndarray | None:
    contour_mask = (mask.astype(np.uint8) * 255)
    contours, _ = cv2.findContours(contour_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
    if not contours:
        return None

    contour = max(contours, key=cv2.contourArea)
    perimeter = cv2.arcLength(contour, True)
    if perimeter <= 0.0:
        return None

    points = cv2.approxPolyDP(contour, epsilon_ratio * perimeter, True).reshape(-1, 2).astype(np.float64)
    if points.shape[0] < 4:
        return None
    return points


def find_top_chain(points: np.ndarray, y_tolerance: float = 2.0) -> list[int] | None:
    if points.shape[0] < 2:
        return None

    eligible = points[:, 1] <= float(points[:, 1].min() + y_tolerance)
    if int(eligible.sum()) < 2:
        eligible = points[:, 1] <= float(points[:, 1].min() + 3.0)
    if int(eligible.sum()) < 2:
        return None

    count = points.shape[0]
    best_chain: list[int] | None = None
    best_span = -1.0
    run_start: int | None = None

    for offset in range(count * 2):
        index = offset % count
        if eligible[index]:
            if run_start is None:
                run_start = offset
            continue

        if run_start is None:
            continue

        run_length = offset - run_start
        if 1 < run_length <= count:
            chain = [value % count for value in range(run_start, offset)]
            span = float(points[chain, 0].max() - points[chain, 0].min())
            if span > best_span:
                best_span = span
                best_chain = chain
        run_start = None

    if run_start is not None:
        run_length = count * 2 - run_start
        if 1 < run_length <= count:
            chain = [value % count for value in range(run_start, count * 2)]
            span = float(points[chain, 0].max() - points[chain, 0].min())
            if span > best_span:
                best_chain = chain

    return best_chain


def angle_between_vectors(previous: np.ndarray, current: np.ndarray) -> float:
    previous_norm = float(np.linalg.norm(previous))
    current_norm = float(np.linalg.norm(current))
    if previous_norm <= 1e-6 or current_norm <= 1e-6:
        return 0.0

    cosine = float(np.clip(np.dot(previous, current) / (previous_norm * current_norm), -1.0, 1.0))
    return float(np.degrees(np.arccos(cosine)))


def fit_x_from_y_points(points: np.ndarray) -> tuple[float, float] | None:
    if points.shape[0] < 2:
        return None

    ys = points[:, 1].astype(np.float64)
    xs = points[:, 0].astype(np.float64)
    slope, intercept = np.polyfit(ys, xs, 1)
    return float(slope), float(intercept)


def project_segment_to_frame(
    start: np.ndarray,
    slope: float,
    intercept: float,
    *,
    side: str,
    image_width: int,
    image_height: int,
    tiny_neighbor: np.ndarray | None = None,
) -> list[int]:
    start_x = float(start[0])
    start_y = float(start[1])

    if tiny_neighbor is not None:
        dx = float(tiny_neighbor[0] - start_x)
        dy = float(tiny_neighbor[1] - start_y)
        if side == "right" and start_x >= image_width - 7 and np.hypot(dx, dy) <= 4.5:
            x0, y0 = clip_point(start_x, start_y, image_width, image_height)
            x1, y1 = clip_point(tiny_neighbor[0], tiny_neighbor[1], image_width, image_height)
            return [x0, y0, x1, y1]

    if abs(slope) <= 1e-6:
        end_x = start_x
        end_y = float(image_height - 1)
    else:
        end_y = float(image_height - 1)
        end_x = slope * end_y + intercept

    if side == "left":
        if end_x >= 2.0:
            x0, y0 = clip_point(start_x, start_y, image_width, image_height)
            x1, y1 = clip_point(end_x, end_y, image_width, image_height)
            return [x0, y0, x1, y1]

        border_y = float((0.0 - intercept) / slope) if abs(slope) > 1e-6 else start_y
        x0, y0 = clip_point(start_x, start_y, image_width, image_height)
        x1, y1 = clip_point(0.0, border_y, image_width, image_height)
        return [x0, y0, x1, y1]

    if end_x <= image_width - 3.0:
        x0, y0 = clip_point(start_x, start_y, image_width, image_height)
        x1, y1 = clip_point(end_x, end_y, image_width, image_height)
        return [x0, y0, x1, y1]

    border_x = float(image_width - 1)
    border_y = float((border_x - intercept) / slope) if abs(slope) > 1e-6 else start_y
    x0, y0 = clip_point(start_x, start_y, image_width, image_height)
    x1, y1 = clip_point(border_x, border_y, image_width, image_height)
    return [x0, y0, x1, y1]


def collect_right_chain(
    points: np.ndarray,
    start_index: int,
    first_side_index: int,
    step: int,
) -> np.ndarray:
    count = points.shape[0]
    indices = [start_index]
    next_index = first_side_index % count
    if points[next_index, 1] < points[start_index, 1]:
        return points[indices].astype(np.float64)

    indices.append(next_index)
    max_x = float(points[:, 0].max())

    while len(indices) < 5:
        candidate_index = (indices[-1] + step) % count
        if candidate_index in indices:
            break

        current = points[indices[-1]]
        candidate = points[candidate_index]
        vector = candidate - current
        if vector[1] <= 0.5:
            break
        if vector[0] < -1.0:
            break

        previous = points[indices[-1]] - points[indices[-2]]
        angle = angle_between_vectors(previous, vector)
        if current[0] >= max_x - 1.0 and vector[0] <= 1.0 and vector[1] >= 4.0:
            break
        if angle > 65.0:
            break

        indices.append(candidate_index)

    return points[indices].astype(np.float64)


def polygon_guided_segments(
    mask: np.ndarray,
    *,
    image_width: int,
    image_height: int,
) -> tuple[list[int] | None, list[int] | None, list[int] | None]:
    points = approximate_mask_polygon(mask)
    if points is None:
        return None, None, None

    top_chain = find_top_chain(points)
    if top_chain is None:
        return None, None, None

    start_index = top_chain[0]
    end_index = top_chain[-1]
    if points[start_index, 0] <= points[end_index, 0]:
        left_top_index = start_index
        right_top_index = end_index
        left_step = -1
        right_step = 1
    else:
        left_top_index = end_index
        right_top_index = start_index
        left_step = 1
        right_step = -1

    left_top = points[left_top_index]
    right_top = points[right_top_index]
    pink_y = float(np.median(points[top_chain, 1]))
    pink_segment = [
        clip_point(left_top[0], pink_y, image_width, image_height)[0],
        clip_point(left_top[0], pink_y, image_width, image_height)[1],
        clip_point(right_top[0], pink_y, image_width, image_height)[0],
        clip_point(right_top[0], pink_y, image_width, image_height)[1],
    ]

    left_neighbor_index = (left_top_index + left_step) % points.shape[0]
    while left_neighbor_index in top_chain:
        left_neighbor_index = (left_neighbor_index + left_step) % points.shape[0]

    left_segment = None
    left_points = np.vstack([left_top, points[left_neighbor_index]])
    left_fit = fit_x_from_y_points(left_points)
    if left_fit is not None:
        left_segment = project_segment_to_frame(
            left_top,
            left_fit[0],
            left_fit[1],
            side="left",
            image_width=image_width,
            image_height=image_height,
        )

    right_segment = None
    right_neighbor_index = (right_top_index + right_step) % points.shape[0]
    while right_neighbor_index in top_chain:
        right_neighbor_index = (right_neighbor_index + right_step) % points.shape[0]

    right_chain = collect_right_chain(points, right_top_index, right_neighbor_index, right_step)
    right_fit = fit_x_from_y_points(right_chain)
    if right_fit is not None:
        tiny_neighbor = right_chain[1] if right_chain.shape[0] >= 2 else None
        right_segment = project_segment_to_frame(
            right_top,
            right_fit[0],
            right_fit[1],
            side="right",
            image_width=image_width,
            image_height=image_height,
            tiny_neighbor=tiny_neighbor,
        )

    return left_segment, right_segment, pink_segment


def should_draw_left(line: LineFit | None) -> bool:
    if line is None:
        return False
    if line.inlier_count < 6 or line.span < 10.0:
        return False
    return not (line.border_touch_ratio >= 0.75 and abs(line.slope) <= 0.25 and line.mean_value <= 1.6)


def should_draw_right(line: LineFit | None, image_width: int) -> bool:
    if line is None:
        return False
    if line.inlier_count < 6 or line.span < 10.0:
        return False
    return not (
        line.border_touch_ratio >= 0.75
        and abs(line.slope) <= 0.25
        and line.mean_value >= image_width - 2.6
    )


def should_draw_right_with_candidate(
    line: LineFit | None,
    image_width: int,
    candidate: PvcCandidate | None,
) -> bool:
    if not should_draw_right(line, image_width):
        return False
    if candidate is None:
        return False

    # Small right-leaning remnants near the border often produce a fake "right side" line.
    if candidate.max_width <= 36 and candidate.area_ratio <= 0.19 and line.mean_value >= image_width - 18.0:
        return False
    return True


def should_draw_pink(line: LineFit | None) -> bool:
    if line is None:
        return False
    if line.inlier_count < 6 or line.span < 8.0:
        return False
    return not (line.border_touch_ratio >= 0.75 and line.mean_value <= 1.6)


def should_draw_yellow(line: LineFit | None, image_height: int) -> bool:
    if line is None:
        return False
    if line.inlier_count < 6 or line.span < 8.0:
        return False
    return not (line.border_touch_ratio >= 0.55 and line.mean_value >= image_height - 2.6)


def should_draw_yellow_with_candidate(
    line: LineFit | None,
    image_height: int,
    candidate: PvcCandidate | None,
) -> bool:
    if not should_draw_yellow(line, image_height):
        return False
    if candidate is None:
        return False

    # A true start line needs a meaningful bottom span; pointed tips are usually false positives.
    min_bottom_width = max(16, int(round(candidate.max_width * 0.28)))
    return candidate.bottom_width >= min_bottom_width


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
    extend_to_bottom: bool = True,
) -> list[int] | None:
    if line is None:
        return None

    y0 = line.support_min
    y1 = line.support_max
    if extend_to_bottom and bottom_row - y1 >= 8:
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
    image_path = FRAMES_DIR / item["video"] / item["frame"]
    with Image.open(image_path) as image:
        gray = np.array(image.convert("L"), dtype=np.uint8)

    candidate = detect_pvc_candidate(gray)

    if candidate is not None:
        left_fit, right_fit = fit_side_lines_from_masks(candidate.visible_mask, candidate.outer_mask)
        top_fit, yellow_fit = fit_horizontal_lines(candidate.outer_mask)
        plateau_pink_fit = fit_pink_from_plateau(candidate.outer_mask)
        annotation_mask = build_annotation_mask(gray, candidate)
        polygon_left_segment, polygon_right_segment, polygon_pink_segment = polygon_guided_segments(
            annotation_mask,
            image_width=gray.shape[1],
            image_height=gray.shape[0],
        )
        left_visible = should_draw_left(left_fit)
        right_visible = should_draw_right_with_candidate(right_fit, gray.shape[1], candidate)
        if left_visible and right_visible:
            pink_visible = plateau_pink_fit is not None or should_draw_pink(top_fit)
        else:
            pink_visible = should_draw_pink(top_fit)
        if pink_visible and candidate.top_row <= 1:
            pink_visible = False
        yellow_visible = should_draw_yellow_with_candidate(yellow_fit, gray.shape[0], candidate)
    else:
        left_fit = None
        right_fit = None
        top_fit = None
        plateau_pink_fit = None
        yellow_fit = None
        polygon_left_segment = None
        polygon_right_segment = None
        polygon_pink_segment = None
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
            extend_to_bottom=yellow_visible
            or candidate.bottom_width <= max(6, int(round(candidate.max_width * 0.12))),
        )
        right_segment = segment_from_side(
            right_fit if right_visible else None,
            side="right",
            bottom_row=candidate.bottom_row,
            image_width=gray.shape[1],
            image_height=gray.shape[0],
            extend_to_bottom=False,
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

        if not yellow_visible:
            if left_visible and polygon_left_segment is not None:
                left_segment = polygon_left_segment
            if right_visible and polygon_right_segment is not None:
                right_segment = polygon_right_segment
            if pink_visible and polygon_pink_segment is not None:
                pink_segment = polygon_pink_segment
                if left_segment is not None:
                    left_segment[0], left_segment[1] = pink_segment[0], pink_segment[1]
                if right_segment is not None:
                    right_segment[0], right_segment[1] = pink_segment[2], pink_segment[3]

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
        top_fit = None
        plateau_pink_fit = None
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
        tag=item["tag"],
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
            "单边桥 PVC 识别原型输出说明",
            "",
            "draw/: 仅保留四条直线段结果图。",
            "originals/: 对应原图拷贝，便于逐张对比。",
            "masks/: PVC 外轮廓掩码。",
            "compare/: 每张图的 原图 | 巡线图 并排对照图。",
            "people/: 人工标注参考图，不会被脚本覆盖。",
            "",
            "颜色约定：",
            "红色 = 左线",
            "蓝色 = 右线",
            "粉色 = 上边线",
            "黄色 = 下边线",
            "",
            "抑制规则：",
            "黄色贴底则不画。",
            "粉色贴顶则不画。",
            "左线贴左边且近似竖直则不画。",
            "右线贴右边且近似竖直则不画。",
        ]
    )
    (output_dir / "README.txt").write_text(readme, encoding="utf-8")
    (output_dir / "README.md").write_text(readme, encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="PVC-only single-bridge prototype with straight-line fitting.")
    parser.add_argument("--subset", type=Path, default=DEFAULT_SUBSET, help="Subset JSON file.")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT, help="Directory for generated results.")
    args = parser.parse_args()

    output_dir = args.output_dir
    draw_dir = output_dir / "draw"
    mask_dir = output_dir / "masks"
    original_dir = output_dir / "originals"
    compare_dir = output_dir / "compare"
    draw_dir.mkdir(parents=True, exist_ok=True)
    mask_dir.mkdir(parents=True, exist_ok=True)
    original_dir.mkdir(parents=True, exist_ok=True)
    compare_dir.mkdir(parents=True, exist_ok=True)

    subset = json.loads(args.subset.read_text(encoding="utf-8"))
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

    print(f"Processed {len(results)} frames.")
    print(f"Results written to: {output_dir}")


if __name__ == "__main__":
    main()
