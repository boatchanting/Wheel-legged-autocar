"""Minefield detector evaluated against `雷区peoplev2` JSON line annotations.

This version uses the corrected geometry:
- inner frame: 100 cm x 100 cm
- outer frame: 120 cm x 120 cm

Ground truth comes from `data/雷区室外偏振片/frames/雷区peoplev2`, where each
PNG has a same-name JSON file describing visible inner/outer line segments.
"""

from __future__ import annotations

import argparse
import json
import math
from dataclasses import dataclass
from itertools import combinations
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw

from minefield_scoring import measure_geometry, score_candidate as score_candidate_breakdown


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DATA_ROOT = PROJECT_ROOT / "data/雷区室外偏振片"
ANNOTATION_DIR = DATA_ROOT / "frames/雷区peoplev2"
OUTPUT_DIR = DATA_ROOT / "peoplev2_geometry_representative"

SCALE = 6
LINE_THICKNESS = 1
OUTER_SIDE_CM = 120.0
INNER_SIDE_CM = 100.0
TAPE_WIDTH_CM = (OUTER_SIDE_CM - INNER_SIDE_CM) / 2.0
VISIBLE_SEGMENT_SUPPORT_RADIUS = 1
VISIBLE_SEGMENT_MIN_VISIBLE_FRACTION = 0.20
INNER_SUPPRESS_FAR_VIEW_THRESHOLD = 0.38
INNER_DIRECT_AREA_RATIO_THRESHOLD = 0.45
INNER_DIRECT_LEFT_DELTA_THRESHOLD = 4.0
INNER_DIRECT_WIDTH_RATIO_THRESHOLD = 0.72
INNER_DIRECT_TOP_DELTA_MIN = -2.0
THRESHOLD_125_AREA_WEIGHT = 0.02
THRESHOLD_125_HEIGHT_WEIGHT = 0.03
FAR_SMALL_FALLBACK_THRESHOLD = 110
FAR_SMALL_FALLBACK_FAR_VIEW = 0.35
FAR_SMALL_FALLBACK_OUTER_AREA = 70.0
HOUGH_LINE_THRESHOLD = 10
HOUGH_MIN_LINE_LENGTH = 8
HOUGH_MAX_LINE_GAP = 4
HOUGH_HORIZONTAL_DY_BASE = 1.5
HOUGH_HORIZONTAL_DY_RATIO = 0.08
HOUGH_SLOPE_RATIO_MIN = 0.15
HOUGH_NEAR_VIEW_MIN_HEIGHT_RATIO = 0.45
CENTRAL_SCENE_MAX_FAR_VIEW = 0.20
CENTRAL_SCENE_MIN_WIDTH_RATIO = 0.90
CENTRAL_SCENE_MIN_HEIGHT_RATIO = 0.30
CENTRAL_SCENE_THIN_TOP_MIN_HEIGHT_RATIO = 0.22
CENTRAL_SCENE_LOW_SUPPORT_OUTER_MAX = 0.75
CENTRAL_SCENE_LOW_SUPPORT_INNER_MAX = 0.65
CENTRAL_SCENE_DARK_TOP_MAX_RATIO = 0.45
CENTRAL_SCENE_OUTER_TOP_RATIO = 0.28
CENTRAL_SCENE_INNER_TOP_RATIO = 0.35
CENTRAL_SCENE_INNER_BOTTOM_VISIBLE_RATIO = 0.70
THRESHOLD_110_RECOVERY_MIN_SCORE_GAIN = 0.006
THRESHOLD_110_RECOVERY_MIN_Y_GAIN = 5
THRESHOLD_110_RECOVERY_MIN_HEIGHT_GAIN = 5
THRESHOLD_110_RECOVERY_MAX_SCORE_DELTA = 0.05
THRESHOLD_110_RECOVERY_MAX_BEST_WIDTH_RATIO = 0.60
THRESHOLD_MID_PREFERENCE_MAX_SCORE_DELTA = 0.03
OBLIQUE_SCENE_MIN_WIDTH_RATIO = 0.90
OBLIQUE_SCENE_MIN_HEIGHT_RATIO = 0.18
OBLIQUE_SCENE_MAX_TOP_RATIO = 0.50
OBLIQUE_SCENE_MIN_OUTER_SLOPE = 0.12
OBLIQUE_BRANCH_MIN_SLOPE = 0.08
OBLIQUE_BRANCH_MIN_SPAN_RATIO = 0.20
OBLIQUE_BRANCH_BORDER_MAX_OFFSET_RATIO = 0.22
OBLIQUE_BRANCH_APEX_LEFT_MIN_OFFSET_RATIO = -0.18
OBLIQUE_BRANCH_APEX_LEFT_MAX_OFFSET_RATIO = 0.15
OBLIQUE_BRANCH_APEX_RIGHT_MIN_OFFSET_RATIO = -0.15
OBLIQUE_BRANCH_APEX_RIGHT_MAX_OFFSET_RATIO = 0.18
OBLIQUE_PROFILE_INNER_APEX_MAX_OFFSET_RATIO = 0.10
OBLIQUE_BRANCH_RIGHT_MIN_END_RATIO = 0.78
OBLIQUE_INNER_MIN_OUTER_GAP_PX = 1.5
OBLIQUE_PROFILE_SOURCE_BONUS = 0.02

REPRESENTATIVE_FRAMES: dict[str, tuple[int, ...]] = {
    "雷区阴天30固定曝光侧_20260713_180026": (3, 71, 159, 202, 302, 320),
    "雷区阴天30固定曝光斜_20260713_175949": (18, 175, 264, 303, 499),
    "雷区阴天30固定曝光直_20260713_180007": (19, 159, 237, 304),
}


@dataclass(frozen=True)
class Segment:
    x1: int
    y1: int
    x2: int
    y2: int


@dataclass(frozen=True)
class Sample:
    prefix: str
    frame_idx: int
    original_path: Path
    annotation_png_path: Path
    annotation_json_path: Path

    @property
    def frame_name(self) -> str:
        return f"{self.prefix}_frame{self.frame_idx:06d}.png"


@dataclass
class GroundTruth:
    outer_segments: list[Segment]
    inner_segments: list[Segment]
    outer_mask: np.ndarray
    inner_mask: np.ndarray
    outer_quad: np.ndarray | None
    inner_quad: np.ndarray | None
    mode: str


@dataclass
class Candidate:
    threshold: int
    score: float
    far_view_factor: float
    outer_area: float
    outer_bbox: tuple[int, int, int, int]
    child_area: float
    touches_border: bool
    outer_quad: np.ndarray
    inner_quad_direct: np.ndarray | None
    inner_quad_final: np.ndarray | None
    threshold_mask: np.ndarray
    geometry_score: float
    ring_mean: float
    inner_mean: float
    ring_on_ratio: float
    inner_on_ratio: float
    outer_support: float
    inner_support: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Detect minefield outer/inner boundaries and compare with 雷区peoplev2 JSON annotations."
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=OUTPUT_DIR,
        help="Directory for outputs under data/雷区室外偏振片.",
    )
    parser.add_argument(
        "--fixed-threshold",
        type=int,
        default=None,
        help="Use a single fixed grayscale threshold instead of the adaptive/Otsu-derived sweep.",
    )
    return parser.parse_args()


def read_rgb(path: Path) -> np.ndarray:
    data = np.fromfile(str(path), dtype=np.uint8)
    bgr = cv2.imdecode(data, cv2.IMREAD_COLOR)
    if bgr is None:
        raise FileNotFoundError(path)
    return cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)


def read_gray(path: Path) -> np.ndarray:
    data = np.fromfile(str(path), dtype=np.uint8)
    gray = cv2.imdecode(data, cv2.IMREAD_GRAYSCALE)
    if gray is None:
        raise FileNotFoundError(path)
    return gray


def save_rgb(path: Path, rgb: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(rgb, mode="RGB").save(path)


def save_gray(path: Path, gray: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(gray, mode="L").save(path)


def save_mask(path: Path, mask: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray((mask.astype(np.uint8) * 255), mode="L").save(path)


def build_samples() -> list[Sample]:
    samples: list[Sample] = []
    for prefix, frame_indices in REPRESENTATIVE_FRAMES.items():
        frame_dir = DATA_ROOT / "frames" / prefix
        for frame_idx in frame_indices:
            frame_name = f"{prefix}_frame{frame_idx:06d}.png"
            original_path = frame_dir / frame_name
            annotation_png_path = ANNOTATION_DIR / frame_name
            annotation_json_path = ANNOTATION_DIR / frame_name.replace(".png", ".json")
            if not original_path.exists():
                raise FileNotFoundError(original_path)
            if not annotation_png_path.exists():
                raise FileNotFoundError(annotation_png_path)
            if not annotation_json_path.exists():
                raise FileNotFoundError(annotation_json_path)
            samples.append(
                Sample(
                    prefix=prefix,
                    frame_idx=frame_idx,
                    original_path=original_path,
                    annotation_png_path=annotation_png_path,
                    annotation_json_path=annotation_json_path,
                )
            )
    return samples


def parse_segments(items: list[dict]) -> list[Segment]:
    return [
        Segment(
            x1=int(item["x1"]),
            y1=int(item["y1"]),
            x2=int(item["x2"]),
            y2=int(item["y2"]),
        )
        for item in items
    ]


def order_quad(points: np.ndarray) -> np.ndarray:
    pts = np.asarray(points, dtype=np.float32).reshape(-1, 2)
    centroid = pts.mean(axis=0)
    angles = np.arctan2(pts[:, 1] - centroid[1], pts[:, 0] - centroid[0])
    pts = pts[np.argsort(angles)]
    start = int(np.argmin(pts.sum(axis=1)))
    return np.roll(pts, -start, axis=0)


def polygon_area(quad: np.ndarray | None) -> float:
    if quad is None:
        return 0.0
    return float(abs(cv2.contourArea(np.asarray(quad, dtype=np.float32).reshape(-1, 1, 2))))


def reduce_polygon_to_quad(points: np.ndarray) -> np.ndarray | None:
    pts = np.asarray(points, dtype=np.float32).reshape(-1, 2)
    if len(pts) < 4:
        return None
    best_quad: np.ndarray | None = None
    best_area = -1.0
    for combo in combinations(range(len(pts)), 4):
        quad = order_quad(pts[list(combo)])
        area = polygon_area(quad)
        if area > best_area:
            best_area = area
            best_quad = quad
    return best_quad


def approx_quad_from_contour(contour: np.ndarray, allow_box_fallback: bool = True) -> np.ndarray | None:
    hull = cv2.convexHull(contour)
    if len(hull) < 4:
        return None

    perimeter = cv2.arcLength(hull, True)
    best_multi: np.ndarray | None = None
    for eps_ratio in np.linspace(0.008, 0.055, 11):
        approx = cv2.approxPolyDP(hull, eps_ratio * perimeter, True)
        if len(approx) == 4:
            return order_quad(approx[:, 0, :])
        if len(approx) > 4 and (best_multi is None or len(approx) < len(best_multi)):
            best_multi = approx[:, 0, :]

    if best_multi is not None:
        quad = reduce_polygon_to_quad(best_multi)
        if quad is not None:
            return order_quad(quad)

    if not allow_box_fallback:
        return None

    rect = cv2.boxPoints(cv2.minAreaRect(hull)).astype(np.float32)
    return order_quad(rect)


def render_segments_mask(shape: tuple[int, int], segments: list[Segment]) -> np.ndarray:
    mask = np.zeros(shape, dtype=np.uint8)
    for seg in segments:
        cv2.line(mask, (seg.x1, seg.y1), (seg.x2, seg.y2), 255, thickness=LINE_THICKNESS, lineType=cv2.LINE_8)
    return mask.astype(bool)


def sample_line_visibility(
    support_mask: np.ndarray,
    p0: np.ndarray,
    p1: np.ndarray,
    max_gap: int = 2,
) -> tuple[np.ndarray, np.ndarray]:
    length = float(np.linalg.norm(p1 - p0))
    sample_count = max(int(math.ceil(length * 2.0)), 8)
    ts = np.linspace(0.0, 1.0, sample_count)
    points = p0[None, :] + (p1 - p0)[None, :] * ts[:, None]
    xs = np.clip(np.round(points[:, 0]).astype(np.int32), 0, support_mask.shape[1] - 1)
    ys = np.clip(np.round(points[:, 1]).astype(np.int32), 0, support_mask.shape[0] - 1)
    visible = support_mask[ys, xs].astype(bool)

    start = None
    gap = 0
    for idx, value in enumerate(visible):
        if value:
            if start is None:
                start = idx
            gap = 0
        elif start is not None:
            gap += 1
            if gap <= max_gap:
                visible[idx] = True
            else:
                gap = 0
                start = None
    return ts, visible


def normalize_segment_points(p0: np.ndarray, p1: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    if p0[0] < p1[0]:
        return p0, p1
    if p0[0] > p1[0]:
        return p1, p0
    if p0[1] <= p1[1]:
        return p0, p1
    return p1, p0


def sample_side_ratios(
    source_mask: np.ndarray,
    p0: np.ndarray,
    p1: np.ndarray,
    offset: float = 1.5,
) -> tuple[float, float]:
    p0, p1 = normalize_segment_points(p0.astype(np.float32), p1.astype(np.float32))
    delta = p1 - p0
    length = float(np.linalg.norm(delta))
    if length < 1e-6:
        return 0.0, 0.0

    tangent = delta / length
    normal = np.array([-tangent[1], tangent[0]], dtype=np.float32)
    sample_count = max(int(math.ceil(length * 1.5)), 8)
    ts = np.linspace(0.0, 1.0, sample_count)
    points = p0[None, :] + delta[None, :] * ts[:, None]
    plus_points = points + normal[None, :] * offset
    minus_points = points - normal[None, :] * offset

    def points_ratio(points_array: np.ndarray) -> float:
        xs = np.clip(np.round(points_array[:, 0]).astype(np.int32), 0, source_mask.shape[1] - 1)
        ys = np.clip(np.round(points_array[:, 1]).astype(np.int32), 0, source_mask.shape[0] - 1)
        return float(source_mask[ys, xs].mean())

    return points_ratio(plus_points), points_ratio(minus_points)


def longest_visible_segment(
    quad: np.ndarray | None,
    support_mask: np.ndarray,
    support_radius: int = 1,
    min_visible_pixels: int = 4,
    min_visible_fraction: float = 0.18,
    min_segment_length_px: float = 6.0,
) -> list[Segment]:
    if quad is None:
        return []

    kernel = np.ones((support_radius * 2 + 1, support_radius * 2 + 1), dtype=np.uint8)
    support_mask = cv2.dilate(support_mask.astype(np.uint8), kernel, iterations=1).astype(bool)
    points = np.asarray(quad, dtype=np.float32)
    segments: list[Segment] = []
    for index in range(4):
        p0 = points[index]
        p1 = points[(index + 1) % 4]
        ts, visible = sample_line_visibility(support_mask, p0, p1)
        if not visible.any():
            continue

        best_start = best_end = -1
        current_start = None
        for pos, value in enumerate(visible):
            if value and current_start is None:
                current_start = pos
            if (not value or pos == len(visible) - 1) and current_start is not None:
                end_pos = pos if value and pos == len(visible) - 1 else pos - 1
                if end_pos - current_start > best_end - best_start:
                    best_start, best_end = current_start, end_pos
                current_start = None

        if best_start < 0 or best_end < best_start:
            continue

        run_length = best_end - best_start + 1
        if run_length < min_visible_pixels:
            continue
        if run_length / max(1, len(visible)) < min_visible_fraction:
            continue

        start_point = p0 + (p1 - p0) * ts[best_start]
        end_point = p0 + (p1 - p0) * ts[best_end]
        if float(np.linalg.norm(end_point - start_point)) < min_segment_length_px:
            continue

        segments.append(
            Segment(
                x1=int(round(float(start_point[0]))),
                y1=int(round(float(start_point[1]))),
                x2=int(round(float(end_point[0]))),
                y2=int(round(float(end_point[1]))),
            )
        )
    return segments


def should_suppress_inner_segments(candidate: Candidate | None) -> bool:
    if candidate is None:
        return True
    return candidate.far_view_factor >= INNER_SUPPRESS_FAR_VIEW_THRESHOLD


def build_edge_support_mask(gray: np.ndarray, threshold_mask: np.ndarray) -> np.ndarray:
    blurred = cv2.GaussianBlur(gray, (3, 3), 0)
    canny = cv2.Canny(blurred, 40, 100) > 0
    gradient = cv2.morphologyEx(
        threshold_mask.astype(np.uint8) * 255,
        cv2.MORPH_GRADIENT,
        np.ones((3, 3), dtype=np.uint8),
    ) > 0
    return canny | gradient


def segment_length(segment: Segment) -> float:
    return float(math.hypot(segment.x2 - segment.x1, segment.y2 - segment.y1))


def edge_length(p0: np.ndarray, p1: np.ndarray) -> float:
    return float(np.linalg.norm(p1 - p0))


def build_segment(p0: np.ndarray, p1: np.ndarray) -> Segment:
    return Segment(
        x1=int(round(float(p0[0]))),
        y1=int(round(float(p0[1]))),
        x2=int(round(float(p1[0]))),
        y2=int(round(float(p1[1]))),
    )


def normalize_segment(segment: Segment) -> Segment:
    if segment.x1 < segment.x2:
        return segment
    if segment.x1 == segment.x2 and segment.y1 <= segment.y2:
        return segment
    return Segment(segment.x2, segment.y2, segment.x1, segment.y1)


def segment_slope(segment: Segment) -> float:
    normalized = normalize_segment(segment)
    dx = float(normalized.x2 - normalized.x1)
    if abs(dx) < 1e-6:
        return float("inf") if normalized.y2 >= normalized.y1 else float("-inf")
    return float(normalized.y2 - normalized.y1) / dx


def segment_visibility_ratio(segment: Segment, support_mask: np.ndarray) -> float:
    normalized = normalize_segment(segment)
    p0 = np.array([normalized.x1, normalized.y1], dtype=np.float32)
    p1 = np.array([normalized.x2, normalized.y2], dtype=np.float32)
    _, visible = sample_line_visibility(support_mask, p0, p1, max_gap=2)
    if len(visible) == 0:
        return 0.0
    return float(visible.mean())


def line_y_at_x(segment: Segment, x: float) -> float:
    normalized = normalize_segment(segment)
    dx = float(normalized.x2 - normalized.x1)
    if abs(dx) < 1e-6:
        return float(normalized.y1)
    return float(normalized.y1 + (x - normalized.x1) * (normalized.y2 - normalized.y1) / dx)


def is_inner_branch_below_outer(
    inner_segment: Segment,
    outer_segment: Segment,
    min_gap_px: float = OBLIQUE_INNER_MIN_OUTER_GAP_PX,
) -> bool:
    inner = normalize_segment(inner_segment)
    outer = normalize_segment(outer_segment)
    for x, y in ((float(inner.x1), float(inner.y1)), (float(inner.x2), float(inner.y2))):
        if y < line_y_at_x(outer, x) + min_gap_px:
            return False
    return True


def dedupe_segments(segments: list[Segment]) -> list[Segment]:
    unique: list[Segment] = []
    seen: set[tuple[int, int, int, int]] = set()
    for segment in segments:
        endpoints = ((segment.x1, segment.y1), (segment.x2, segment.y2))
        ordered = tuple(endpoints if endpoints[0] <= endpoints[1] else (endpoints[1], endpoints[0]))
        key = (ordered[0][0], ordered[0][1], ordered[1][0], ordered[1][1])
        if key in seen:
            continue
        seen.add(key)
        unique.append(segment)
    return unique


def is_side_segment(segment: Segment) -> bool:
    dx = abs(segment.x2 - segment.x1)
    dy = abs(segment.y2 - segment.y1)
    return segment_length(segment) >= 6.0 and dy >= max(3.0, dx * 0.25)


def is_central_scene_candidate(candidate: Candidate, image_shape: tuple[int, int]) -> bool:
    height, width = image_shape
    width_ratio = candidate.outer_bbox[2] / max(float(width), 1.0)
    height_ratio = candidate.outer_bbox[3] / max(float(height), 1.0)
    return (
        candidate.far_view_factor <= CENTRAL_SCENE_MAX_FAR_VIEW
        and width_ratio >= CENTRAL_SCENE_MIN_WIDTH_RATIO
        and (
            height_ratio >= CENTRAL_SCENE_MIN_HEIGHT_RATIO
            or (
                candidate.outer_bbox[1] <= int(height * 0.2)
                and height_ratio >= CENTRAL_SCENE_THIN_TOP_MIN_HEIGHT_RATIO
            )
        )
    )


def find_matching_outer_contour(candidate: Candidate) -> np.ndarray | None:
    mask_u8 = candidate.threshold_mask.astype(np.uint8) * 255
    contours, hierarchy = cv2.findContours(mask_u8, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)
    if not contours or hierarchy is None:
        return None

    target_bbox = tuple(candidate.outer_bbox)
    best: np.ndarray | None = None
    best_score = float("inf")
    for idx, contour in enumerate(contours):
        if int(hierarchy[0, idx, 3]) != -1:
            continue

        area = float(cv2.contourArea(contour))
        bbox = tuple(cv2.boundingRect(contour))
        score = abs(area - candidate.outer_area) + sum(abs(a - b) for a, b in zip(bbox, target_bbox)) * 5.0
        if score < best_score:
            best = contour
            best_score = score
    return best


def pick_central_dark_component_contour(
    threshold_mask: np.ndarray,
) -> tuple[np.ndarray | None, tuple[int, int, int, int] | None]:
    inv = (~threshold_mask).astype(np.uint8) * 255
    num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(inv, 8)
    height, width = threshold_mask.shape

    best_idx = -1
    best_score = float("-inf")
    for idx in range(1, num_labels):
        x, y, bw, bh, area = (int(value) for value in stats[idx])
        if bw < int(width * 0.35) or bh < int(height * 0.2):
            continue

        cx, cy = centroids[idx]
        center_dx = abs(float(cx) - (width - 1) * 0.5) / max(width * 0.5, 1.0)
        center_dy = abs(float(cy) - (height - 1) * 0.5) / max(height * 0.5, 1.0)
        score = float(area) - 1200.0 * center_dx - 600.0 * center_dy - (200.0 if y == 0 else 0.0)
        if score > best_score:
            best_idx = idx
            best_score = score

    if best_idx < 0:
        return None, None

    component_mask = (labels == best_idx).astype(np.uint8) * 255
    contours, _ = cv2.findContours(component_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return None, None

    contour = max(contours, key=cv2.contourArea)
    x, y, bw, bh = cv2.boundingRect(contour)
    return contour, (int(x), int(y), int(bw), int(bh))


def build_top_band_profiles(
    threshold_mask: np.ndarray,
    bbox: tuple[int, int, int, int],
) -> tuple[np.ndarray, np.ndarray]:
    _, width = threshold_mask.shape
    x, _, bw, _ = bbox
    start_x = max(0, x)
    end_x = min(width - 1, x + bw - 1)
    top_profile = np.full(width, np.nan, dtype=np.float32)
    band_bottom_profile = np.full(width, np.nan, dtype=np.float32)
    for x_pos in range(start_x, end_x + 1):
        ys = np.flatnonzero(threshold_mask[:, x_pos])
        if len(ys) == 0:
            continue
        top_profile[x_pos] = float(ys[0])
        run_end = int(ys[0])
        for y_pos in ys[1:]:
            if int(y_pos) == run_end + 1:
                run_end = int(y_pos)
            else:
                break
        band_bottom_profile[x_pos] = float(run_end)
    return top_profile, band_bottom_profile


def smooth_sparse_profile(profile: np.ndarray, kernel_size: int = 5) -> np.ndarray:
    if kernel_size <= 1:
        return profile.copy()
    kernel = np.ones(kernel_size, dtype=np.float32)
    values = np.where(np.isnan(profile), 0.0, profile).astype(np.float32)
    weights = (~np.isnan(profile)).astype(np.float32)
    smoothed_values = np.convolve(values, kernel, mode="same")
    smoothed_weights = np.convolve(weights, kernel, mode="same")
    result = np.full(profile.shape, np.nan, dtype=np.float32)
    valid = smoothed_weights > 1e-6
    result[valid] = smoothed_values[valid] / smoothed_weights[valid]
    return result


def fit_profile_branch(
    profile: np.ndarray,
    start_x: int,
    end_x: int,
) -> tuple[Segment | None, float]:
    lo = min(start_x, end_x)
    hi = max(start_x, end_x)
    xs = np.arange(lo, hi + 1)
    xs = xs[~np.isnan(profile[lo:hi + 1])]
    if len(xs) < 6:
        return None, 0.0
    ys = profile[xs]
    slope, intercept = np.polyfit(xs.astype(np.float32), ys.astype(np.float32), deg=1)
    x1 = int(xs[0])
    x2 = int(xs[-1])
    y1 = int(round(float(slope * x1 + intercept)))
    y2 = int(round(float(slope * x2 + intercept)))
    return Segment(x1=x1, y1=y1, x2=x2, y2=y2), float(slope)


def extract_oblique_profile_roles(candidate: Candidate) -> dict:
    top_profile, band_bottom_profile = build_top_band_profiles(candidate.threshold_mask, candidate.outer_bbox)
    top_profile = smooth_sparse_profile(top_profile)
    band_bottom_profile = smooth_sparse_profile(band_bottom_profile)

    valid_top = np.flatnonzero(~np.isnan(top_profile))
    if len(valid_top) < max(12, int(round(candidate.outer_bbox[2] * 0.55))):
        return {}

    apex_x = int(valid_top[np.argmin(top_profile[valid_top])])
    roles: dict[str, object] = {
        "apex_x": apex_x,
        "slopes": {},
    }

    outer_left, outer_left_slope = fit_profile_branch(top_profile, int(valid_top[0]), apex_x)
    outer_right, outer_right_slope = fit_profile_branch(top_profile, apex_x, int(valid_top[-1]))
    if outer_left is not None:
        roles["outer_left"] = outer_left
    if outer_right is not None:
        roles["outer_right"] = outer_right
    roles["slopes"]["outer_left"] = outer_left_slope
    roles["slopes"]["outer_right"] = outer_right_slope

    valid_bottom = np.flatnonzero(~np.isnan(band_bottom_profile))
    if len(valid_bottom) >= max(12, int(round(candidate.outer_bbox[2] * 0.45))):
        bottom_apex_x = int(valid_bottom[np.argmin(band_bottom_profile[valid_bottom])])
        inner_left, inner_left_slope = fit_profile_branch(
            band_bottom_profile,
            int(valid_bottom[0]),
            bottom_apex_x,
        )
        inner_right, inner_right_slope = fit_profile_branch(
            band_bottom_profile,
            bottom_apex_x,
            int(valid_bottom[-1]),
        )
        if inner_left is not None:
            roles["inner_left"] = inner_left
        if inner_right is not None:
            roles["inner_right"] = inner_right
        roles["slopes"]["inner_left"] = inner_left_slope
        roles["slopes"]["inner_right"] = inner_right_slope
    return roles


def approx_ordered_contour_points(contour: np.ndarray, eps_ratio: float) -> np.ndarray:
    perimeter = cv2.arcLength(contour, True)
    approx = cv2.approxPolyDP(contour, eps_ratio * perimeter, True)
    return approx[:, 0, :].astype(np.float32)


def group_contour_edges(points: np.ndarray, predicate) -> list[list[int]]:
    point_count = len(points)
    if point_count == 0:
        return []

    active_edges = [
        bool(predicate(points[idx], points[(idx + 1) % point_count]))
        for idx in range(point_count)
    ]
    groups: list[list[int]] = []
    start_idx: int | None = None
    for edge_idx, is_active in enumerate(active_edges):
        if is_active and start_idx is None:
            start_idx = edge_idx
        elif (not is_active) and start_idx is not None:
            groups.append(list(range(start_idx, edge_idx)))
            start_idx = None
    if start_idx is not None:
        groups.append(list(range(start_idx, point_count)))

    if groups and active_edges[0] and active_edges[-1] and len(groups) >= 2:
        groups[0] = groups[-1] + groups[0]
        groups.pop()
    return groups


def extract_central_open_outer_segments(
    candidate: Candidate,
    image_shape: tuple[int, int],
) -> dict[str, list[Segment]]:
    contour = find_matching_outer_contour(candidate)
    if contour is None:
        return {}

    points = approx_ordered_contour_points(contour, eps_ratio=0.008)
    if len(points) < 4:
        return {}

    ys = points[:, 1]
    min_y = float(ys.min())
    max_y = float(ys.max())
    top_threshold = min_y + CENTRAL_SCENE_OUTER_TOP_RATIO * max(1.0, max_y - min_y)
    top_groups = group_contour_edges(
        points,
        lambda p0, p1: p0[1] <= top_threshold and p1[1] <= top_threshold and edge_length(p0, p1) >= 6.0,
    )
    if len(top_groups) < 2:
        return {}

    top_group = min(
        top_groups,
        key=lambda group: float(
            np.mean([(points[idx][1] + points[(idx + 1) % len(points)][1]) * 0.5 for idx in group])
        ),
    )

    roles: dict[str, list[Segment]] = {"outer_top": []}
    for edge_idx in top_group:
        roles["outer_top"].append(build_segment(points[edge_idx], points[(edge_idx + 1) % len(points)]))

    center_x = candidate.outer_bbox[0] + candidate.outer_bbox[2] * 0.5
    for edge_idx in ((top_group[0] - 1) % len(points), (top_group[-1] + 1) % len(points)):
        segment = build_segment(points[edge_idx], points[(edge_idx + 1) % len(points)])
        if not is_side_segment(segment):
            continue
        if min(segment.x1, segment.x2) <= 1 and max(segment.x1, segment.x2) <= 1:
            continue
        role_name = "outer_left" if (segment.x1 + segment.x2) * 0.5 <= center_x else "outer_right"
        roles.setdefault(role_name, []).append(segment)
    return roles


def extract_central_open_inner_segments(candidate: Candidate) -> dict[str, list[Segment]]:
    contour, dark_bbox = pick_central_dark_component_contour(candidate.threshold_mask)
    if contour is None or dark_bbox is None:
        return {}

    points = approx_ordered_contour_points(contour, eps_ratio=0.02)
    if len(points) < 4:
        return {}

    outer_y = candidate.outer_bbox[1]
    outer_h = candidate.outer_bbox[3]
    if dark_bbox[1] > int(round(outer_y + outer_h * CENTRAL_SCENE_DARK_TOP_MAX_RATIO)):
        return {}

    ys = points[:, 1]
    min_y = float(ys.min())
    max_y = float(ys.max())
    top_threshold = min_y + CENTRAL_SCENE_INNER_TOP_RATIO * max(1.0, max_y - min_y)
    top_groups = group_contour_edges(
        points,
        lambda p0, p1: p0[1] <= top_threshold and p1[1] <= top_threshold and edge_length(p0, p1) >= 6.0,
    )
    if not top_groups:
        return {}

    top_group = min(
        top_groups,
        key=lambda group: float(
            np.mean([(points[idx][1] + points[(idx + 1) % len(points)][1]) * 0.5 for idx in group])
        ),
    )
    roles: dict[str, list[Segment]] = {"inner_top": []}
    for edge_idx in top_group:
        roles["inner_top"].append(build_segment(points[edge_idx], points[(edge_idx + 1) % len(points)]))

    center_x = candidate.outer_bbox[0] + candidate.outer_bbox[2] * 0.5
    for edge_idx in ((top_group[0] - 1) % len(points), (top_group[-1] + 1) % len(points)):
        segment = build_segment(points[edge_idx], points[(edge_idx + 1) % len(points)])
        if not is_side_segment(segment):
            continue
        role_name = "inner_left" if (segment.x1 + segment.x2) * 0.5 <= center_x else "inner_right"
        roles.setdefault(role_name, []).append(segment)

    bottom_points = points[points[:, 1] >= (max_y - 0.18 * max(1.0, max_y - min_y))]
    if len(bottom_points) >= 2:
        left_point = bottom_points[np.argmin(bottom_points[:, 0])]
        right_point = bottom_points[np.argmax(bottom_points[:, 0])]
        if float(right_point[0] - left_point[0]) >= candidate.outer_bbox[2] * CENTRAL_SCENE_INNER_BOTTOM_VISIBLE_RATIO:
            roles["inner_bottom"] = [build_segment(left_point, right_point)]
    return roles


def is_oblique_scene_candidate(candidate: Candidate, image_shape: tuple[int, int], profile_roles: dict) -> bool:
    if not profile_roles:
        return False
    outer_left = profile_roles.get("outer_left")
    outer_right = profile_roles.get("outer_right")
    if outer_left is None or outer_right is None:
        return False

    height, width = image_shape
    width_ratio = candidate.outer_bbox[2] / max(float(width), 1.0)
    height_ratio = candidate.outer_bbox[3] / max(float(height), 1.0)
    if width_ratio < OBLIQUE_SCENE_MIN_WIDTH_RATIO or height_ratio < OBLIQUE_SCENE_MIN_HEIGHT_RATIO:
        return False
    if candidate.outer_bbox[1] > int(height * OBLIQUE_SCENE_MAX_TOP_RATIO):
        return False

    slopes = profile_roles.get("slopes", {})
    return (
        float(slopes.get("outer_left", 0.0)) <= -OBLIQUE_SCENE_MIN_OUTER_SLOPE
        and float(slopes.get("outer_right", 0.0)) >= OBLIQUE_SCENE_MIN_OUTER_SLOPE
    )


def is_valid_oblique_branch_segment(
    segment: Segment,
    side: str,
    apex_x: int,
    bbox: tuple[int, int, int, int],
    min_slope: float = OBLIQUE_BRANCH_MIN_SLOPE,
    strict_inner_profile: bool = False,
) -> bool:
    normalized = normalize_segment(segment)
    bbox_x, _, bbox_w, _ = bbox
    min_span = max(12.0, bbox_w * OBLIQUE_BRANCH_MIN_SPAN_RATIO)
    dx = float(normalized.x2 - normalized.x1)
    if dx < min_span:
        return False

    slope = segment_slope(normalized)
    if side == "left":
        if slope > -min_slope:
            return False
        if normalized.x1 > bbox_x + bbox_w * OBLIQUE_BRANCH_BORDER_MAX_OFFSET_RATIO:
            return False
        apex_max_ratio = (
            OBLIQUE_PROFILE_INNER_APEX_MAX_OFFSET_RATIO
            if strict_inner_profile
            else OBLIQUE_BRANCH_APEX_LEFT_MAX_OFFSET_RATIO
        )
        if normalized.x2 < apex_x + bbox_w * OBLIQUE_BRANCH_APEX_LEFT_MIN_OFFSET_RATIO:
            return False
        if normalized.x2 > apex_x + bbox_w * apex_max_ratio:
            return False
        return True

    if slope < min_slope:
        return False
    apex_min_ratio = (
        -OBLIQUE_PROFILE_INNER_APEX_MAX_OFFSET_RATIO
        if strict_inner_profile
        else OBLIQUE_BRANCH_APEX_RIGHT_MIN_OFFSET_RATIO
    )
    apex_max_ratio = (
        OBLIQUE_PROFILE_INNER_APEX_MAX_OFFSET_RATIO
        if strict_inner_profile
        else OBLIQUE_BRANCH_APEX_RIGHT_MAX_OFFSET_RATIO
    )
    if normalized.x1 < apex_x + bbox_w * apex_min_ratio:
        return False
    if normalized.x1 > apex_x + bbox_w * apex_max_ratio:
        return False
    if normalized.x2 < bbox_x + bbox_w * OBLIQUE_BRANCH_RIGHT_MIN_END_RATIO:
        return False
    return True


def score_oblique_branch_segment(
    segment: Segment,
    side: str,
    apex_x: int,
    bbox: tuple[int, int, int, int],
    edge_support_mask: np.ndarray,
    source: str,
) -> float:
    normalized = normalize_segment(segment)
    bbox_x, _, bbox_w, _ = bbox
    support = segment_visibility_ratio(normalized, edge_support_mask)
    length_ratio = min(segment_length(normalized) / max(float(bbox_w), 1.0), 1.0)
    apex_anchor_x = normalized.x2 if side == "left" else normalized.x1
    border_anchor_x = normalized.x1 if side == "left" else normalized.x2
    apex_error = abs(float(apex_anchor_x) - float(apex_x)) / max(float(bbox_w), 1.0)
    border_target_x = bbox_x if side == "left" else bbox_x + bbox_w - 1
    border_error = abs(float(border_anchor_x) - float(border_target_x)) / max(float(bbox_w), 1.0)
    score = (
        0.60 * support
        + 0.25 * length_ratio
        + 0.10 * (1.0 - min(apex_error / 0.15, 1.0))
        + 0.05 * (1.0 - min(border_error / 0.15, 1.0))
    )
    if source == "profile":
        score += OBLIQUE_PROFILE_SOURCE_BONUS
    return score


def has_strong_bottom_evidence(
    segments: list[Segment],
    bbox: tuple[int, int, int, int],
) -> bool:
    _, bbox_y, bbox_w, bbox_h = bbox
    for segment in segments:
        normalized = normalize_segment(segment)
        dx = float(normalized.x2 - normalized.x1)
        dy = abs(float(normalized.y2 - normalized.y1))
        mid_y = (float(normalized.y1) + float(normalized.y2)) * 0.5
        if (
            dx >= bbox_w * 0.55
            and dy <= max(2.0, dx * 0.08)
            and mid_y >= bbox_y + bbox_h * 0.45
        ):
            return True
    return False


def has_strong_lower_branch_evidence(
    segments: list[Segment],
    apex_x: int,
    bbox: tuple[int, int, int, int],
) -> bool:
    bbox_x, bbox_y, bbox_w, bbox_h = bbox
    for segment in segments:
        normalized = normalize_segment(segment)
        slope = segment_slope(normalized)
        dy = abs(float(normalized.y2 - normalized.y1))
        mid_y = (float(normalized.y1) + float(normalized.y2)) * 0.5
        left_like = (
            normalized.x1 <= bbox_x + bbox_w * 0.18
            and abs(float(normalized.x2) - float(apex_x)) <= bbox_w * 0.22
        )
        right_like = (
            abs(float(normalized.x1) - float(apex_x)) <= bbox_w * 0.22
            and normalized.x2 >= bbox_x + bbox_w * 0.82
        )
        if (
            (left_like or right_like)
            and abs(slope) >= 0.25
            and dy >= bbox_h * 0.45
            and mid_y >= bbox_y + bbox_h * 0.55
        ):
            return True
    return False


def select_best_oblique_branch_segment(
    candidates: list[tuple[str, Segment]],
    role_name: str,
    apex_x: int,
    bbox: tuple[int, int, int, int],
    edge_support_mask: np.ndarray,
    outer_reference: Segment | None = None,
) -> Segment | None:
    role_kind, side = role_name.split("_", 1)
    best_segment: Segment | None = None
    best_score = float("-inf")
    for source, segment in candidates:
        strict_inner_profile = role_kind == "inner" and source == "profile"
        min_slope = OBLIQUE_SCENE_MIN_OUTER_SLOPE if role_kind == "outer" else OBLIQUE_BRANCH_MIN_SLOPE
        if strict_inner_profile:
            min_slope = max(min_slope, OBLIQUE_SCENE_MIN_OUTER_SLOPE)
        if not is_valid_oblique_branch_segment(
            segment,
            side=side,
            apex_x=apex_x,
            bbox=bbox,
            min_slope=min_slope,
            strict_inner_profile=strict_inner_profile,
        ):
            continue
        normalized = normalize_segment(segment)
        if role_kind == "inner" and outer_reference is not None and not is_inner_branch_below_outer(
            normalized,
            outer_reference,
        ):
            continue
        score = score_oblique_branch_segment(
            normalized,
            side=side,
            apex_x=apex_x,
            bbox=bbox,
            edge_support_mask=edge_support_mask,
            source=source,
        )
        if score > best_score:
            best_score = score
            best_segment = normalized
    return best_segment


def build_oblique_rescue_segments(
    candidate: Candidate,
    edge_support_mask: np.ndarray,
    fallback_outer_segments: list[Segment],
    fallback_inner_segments: list[Segment],
    hough_merged_segments: dict[str, Segment],
) -> tuple[list[Segment], list[Segment]]:
    profile_roles = extract_oblique_profile_roles(candidate)
    if not is_oblique_scene_candidate(candidate, edge_support_mask.shape, profile_roles):
        return [], []

    apex_x = int(profile_roles["apex_x"])
    bbox = candidate.outer_bbox
    fallback_segments = fallback_outer_segments + fallback_inner_segments
    if has_strong_bottom_evidence(fallback_segments, bbox):
        return [], []
    if has_strong_lower_branch_evidence(fallback_segments, apex_x, bbox):
        return [], []

    outer_left_candidates: list[tuple[str, Segment]] = []
    outer_right_candidates: list[tuple[str, Segment]] = []
    inner_left_candidates: list[tuple[str, Segment]] = []
    inner_right_candidates: list[tuple[str, Segment]] = []

    for source, roles in (("profile", profile_roles), ("hough", hough_merged_segments)):
        left_segment = roles.get("outer_left")
        right_segment = roles.get("outer_right")
        if isinstance(left_segment, Segment):
            outer_left_candidates.append((source, left_segment))
        if isinstance(right_segment, Segment):
            outer_right_candidates.append((source, right_segment))

    for segment in fallback_outer_segments:
        outer_left_candidates.append(("fallback", segment))
        outer_right_candidates.append(("fallback", segment))

    rescue_outer_left = select_best_oblique_branch_segment(
        outer_left_candidates,
        role_name="outer_left",
        apex_x=apex_x,
        bbox=bbox,
        edge_support_mask=edge_support_mask,
    )
    rescue_outer_right = select_best_oblique_branch_segment(
        outer_right_candidates,
        role_name="outer_right",
        apex_x=apex_x,
        bbox=bbox,
        edge_support_mask=edge_support_mask,
    )
    if rescue_outer_left is None or rescue_outer_right is None:
        return [], []

    rescue_outer_segments = dedupe_segments([rescue_outer_left, rescue_outer_right])
    if should_suppress_inner_segments(candidate):
        return rescue_outer_segments, []

    for source, roles in (("profile", profile_roles), ("hough", hough_merged_segments)):
        left_segment = roles.get("inner_left")
        right_segment = roles.get("inner_right")
        if isinstance(left_segment, Segment):
            inner_left_candidates.append((source, left_segment))
        if isinstance(right_segment, Segment):
            inner_right_candidates.append((source, right_segment))

    for segment in fallback_inner_segments:
        inner_left_candidates.append(("fallback", segment))
        inner_right_candidates.append(("fallback", segment))

    rescue_inner_left = select_best_oblique_branch_segment(
        inner_left_candidates,
        role_name="inner_left",
        apex_x=apex_x,
        bbox=bbox,
        edge_support_mask=edge_support_mask,
        outer_reference=rescue_outer_left,
    )
    rescue_inner_right = select_best_oblique_branch_segment(
        inner_right_candidates,
        role_name="inner_right",
        apex_x=apex_x,
        bbox=bbox,
        edge_support_mask=edge_support_mask,
        outer_reference=rescue_outer_right,
    )

    rescue_inner_segments: list[Segment] = []
    if rescue_inner_left is not None and rescue_inner_right is not None:
        rescue_inner_segments = dedupe_segments([rescue_inner_left, rescue_inner_right])
    if len(rescue_inner_segments) < 2:
        return [], []
    return rescue_outer_segments, rescue_inner_segments


def classify_hough_segment(
    threshold_mask: np.ndarray,
    bbox_center_y: float,
    raw_line: np.ndarray,
) -> tuple[str, Segment, float] | None:
    p0 = np.asarray(raw_line[:2], dtype=np.float32)
    p1 = np.asarray(raw_line[2:], dtype=np.float32)
    p0, p1 = normalize_segment_points(p0, p1)

    dx = float(p1[0] - p0[0])
    dy = float(p1[1] - p0[1])
    length = float(np.hypot(dx, dy))
    if length < HOUGH_MIN_LINE_LENGTH:
        return None

    horizontal_limit = max(HOUGH_HORIZONTAL_DY_BASE, HOUGH_HORIZONTAL_DY_RATIO * max(abs(dx), 1.0))
    if abs(dy) <= horizontal_limit:
        orientation = "horizontal"
    elif dy <= -(HOUGH_SLOPE_RATIO_MIN * max(abs(dx), 1.0)):
        orientation = "neg"
    elif dy >= (HOUGH_SLOPE_RATIO_MIN * max(abs(dx), 1.0)):
        orientation = "pos"
    else:
        return None

    plus_ratio, minus_ratio = sample_side_ratios(threshold_mask, p0, p1)
    if abs(plus_ratio - minus_ratio) < 0.08:
        return None

    white_plus = plus_ratio > minus_ratio
    mid_y = float((p0[1] + p1[1]) * 0.5)
    if orientation == "horizontal":
        if white_plus:
            class_name = "outer_top" if mid_y <= bbox_center_y else "inner_bottom"
        else:
            class_name = "inner_top" if mid_y <= bbox_center_y else "outer_bottom"
    elif orientation == "neg":
        class_name = "outer_left" if white_plus else "inner_left"
    else:
        class_name = "inner_right" if white_plus else "outer_right"

    segment = Segment(
        x1=int(round(float(p0[0]))),
        y1=int(round(float(p0[1]))),
        x2=int(round(float(p1[0]))),
        y2=int(round(float(p1[1]))),
    )
    return class_name, segment, length


def merge_hough_segments(class_name: str, items: list[tuple[Segment, float]]) -> Segment | None:
    if not items:
        return None

    if len(items) == 1:
        return items[0][0]

    endpoints: list[tuple[float, float, float]] = []
    for segment, length in items:
        endpoints.append((float(segment.x1), float(segment.y1), length))
        endpoints.append((float(segment.x2), float(segment.y2), length))

    xs = np.asarray([item[0] for item in endpoints], dtype=np.float32)
    ys = np.asarray([item[1] for item in endpoints], dtype=np.float32)
    weights = np.asarray([item[2] for item in endpoints], dtype=np.float32)

    if class_name.endswith("top") or class_name.endswith("bottom"):
        x1 = int(round(float(xs.min())))
        x2 = int(round(float(xs.max())))
        y = int(round(float(np.average(ys, weights=weights))))
        return Segment(x1=x1, y1=y, x2=x2, y2=y)

    slope, intercept = np.polyfit(xs, ys, deg=1, w=weights)
    x1 = float(xs.min())
    x2 = float(xs.max())
    y1 = float(slope * x1 + intercept)
    y2 = float(slope * x2 + intercept)
    return Segment(
        x1=int(round(x1)),
        y1=int(round(y1)),
        x2=int(round(x2)),
        y2=int(round(y2)),
    )


def extract_hough_prediction_segments(
    gray: np.ndarray,
    candidate: Candidate,
    edge_support_mask: np.ndarray,
) -> tuple[list[Segment], list[Segment], set[str], dict[str, Segment]]:
    edge_mask_u8 = edge_support_mask.astype(np.uint8) * 255
    lines = cv2.HoughLinesP(
        edge_mask_u8,
        rho=1,
        theta=np.pi / 180.0,
        threshold=HOUGH_LINE_THRESHOLD,
        minLineLength=HOUGH_MIN_LINE_LENGTH,
        maxLineGap=HOUGH_MAX_LINE_GAP,
    )
    if lines is None:
        return [], [], set(), {}

    bbox_center_y = float(candidate.outer_bbox[1] + candidate.outer_bbox[3] * 0.5)
    grouped: dict[str, list[tuple[Segment, float]]] = {
        "outer_left": [],
        "outer_top": [],
        "outer_right": [],
        "outer_bottom": [],
        "inner_left": [],
        "inner_top": [],
        "inner_right": [],
        "inner_bottom": [],
    }
    for raw_line in lines[:, 0, :]:
        classified = classify_hough_segment(candidate.threshold_mask, bbox_center_y, raw_line)
        if classified is None:
            continue
        class_name, segment, length = classified
        grouped[class_name].append((segment, length))

    merged_segments: dict[str, Segment] = {}
    for class_name in (
        "outer_left",
        "outer_top",
        "outer_right",
        "outer_bottom",
        "inner_left",
        "inner_top",
        "inner_right",
        "inner_bottom",
    ):
        merged = merge_hough_segments(class_name, grouped[class_name])
        if merged is not None:
            merged_segments[class_name] = merged

    outer_segments: list[Segment] = []
    inner_segments: list[Segment] = []
    for class_name in ("outer_left", "outer_top", "outer_right", "outer_bottom"):
        merged = merged_segments.get(class_name)
        if merged is not None:
            outer_segments.append(merged)
    for class_name in ("inner_left", "inner_top", "inner_right", "inner_bottom"):
        merged = merged_segments.get(class_name)
        if merged is not None:
            inner_segments.append(merged)
    present_classes = {class_name for class_name, items in grouped.items() if items}
    return outer_segments, inner_segments, present_classes, merged_segments


def render_quad_mask(shape: tuple[int, int], quad: np.ndarray | None) -> np.ndarray:
    mask = np.zeros(shape, dtype=np.uint8)
    if quad is None:
        return mask.astype(bool)
    pts = np.round(quad).astype(np.int32).reshape(-1, 1, 2)
    cv2.polylines(mask, [pts], isClosed=True, color=255, thickness=LINE_THICKNESS, lineType=cv2.LINE_8)
    return mask.astype(bool)


def render_polygon_mask(shape: tuple[int, int], quad: np.ndarray | None) -> np.ndarray:
    mask = np.zeros(shape, dtype=np.uint8)
    if quad is None:
        return mask.astype(bool)
    pts = np.round(quad).astype(np.int32).reshape(-1, 1, 2)
    cv2.fillPoly(mask, [pts], color=255, lineType=cv2.LINE_8)
    return mask.astype(bool)


def extract_prediction_segments(
    gray: np.ndarray,
    candidate: Candidate | None,
) -> tuple[np.ndarray, list[Segment], list[Segment]]:
    if candidate is None:
        empty_mask = np.zeros_like(gray, dtype=bool)
        return empty_mask, [], []

    edge_support_mask = build_edge_support_mask(gray, candidate.threshold_mask)
    fallback_outer_segments = longest_visible_segment(
        candidate.outer_quad,
        edge_support_mask,
        support_radius=VISIBLE_SEGMENT_SUPPORT_RADIUS,
        min_visible_fraction=VISIBLE_SEGMENT_MIN_VISIBLE_FRACTION,
    )
    hough_outer_segments, hough_inner_segments, hough_present_classes, hough_merged_segments = extract_hough_prediction_segments(
        gray,
        candidate,
        edge_support_mask,
    )
    near_view_height_ratio = candidate.outer_bbox[3] / max(float(gray.shape[0]), 1.0)
    hough_has_bottom_band = (
        "outer_bottom" in hough_present_classes or "inner_bottom" in hough_present_classes
    )

    use_hough = (
        candidate.far_view_factor <= 0.2
        and candidate.outer_bbox[2] >= int(gray.shape[1] * 0.9)
        and candidate.outer_bbox[1] <= int(gray.shape[0] * 0.2)
        and near_view_height_ratio >= HOUGH_NEAR_VIEW_MIN_HEIGHT_RATIO
        and hough_has_bottom_band
        and (len(hough_outer_segments) + len(hough_inner_segments)) >= 5
    )
    use_hough_outer = use_hough and len(hough_outer_segments) >= 2
    pred_outer_segments = hough_outer_segments if use_hough_outer else fallback_outer_segments
    fallback_inner_segments: list[Segment] = []

    if should_suppress_inner_segments(candidate):
        pred_inner_segments: list[Segment] = []
    else:
        fallback_inner_segments = longest_visible_segment(
            candidate.inner_quad_final,
            edge_support_mask,
            support_radius=VISIBLE_SEGMENT_SUPPORT_RADIUS,
            min_visible_fraction=VISIBLE_SEGMENT_MIN_VISIBLE_FRACTION,
        )
        use_hough_inner = use_hough and len(hough_inner_segments) >= 2
        pred_inner_segments = hough_inner_segments if use_hough_inner else fallback_inner_segments

    should_try_central_rescue = (
        not should_suppress_inner_segments(candidate)
        and is_central_scene_candidate(candidate, gray.shape)
        and candidate.inner_quad_direct is None
        and candidate.outer_support <= CENTRAL_SCENE_LOW_SUPPORT_OUTER_MAX
        and candidate.inner_support <= CENTRAL_SCENE_LOW_SUPPORT_INNER_MAX
        and len(fallback_outer_segments) <= 2
        and len(fallback_inner_segments) <= 2
    )
    if should_try_central_rescue:
        contour_outer_roles = extract_central_open_outer_segments(candidate, gray.shape)
        contour_inner_roles = extract_central_open_inner_segments(candidate)
        rescue_outer_segments = dedupe_segments(
            contour_outer_roles.get("outer_top", [])
            + contour_outer_roles.get("outer_left", [])
            + contour_outer_roles.get("outer_right", [])
        )
        rescue_inner_segments = dedupe_segments(
            contour_inner_roles.get("inner_left", [])
            + contour_inner_roles.get("inner_top", [])
            + contour_inner_roles.get("inner_right", [])
            + contour_inner_roles.get("inner_bottom", [])
        )
        if len(rescue_outer_segments) >= 2 and len(rescue_inner_segments) >= 3:
            pred_outer_segments = rescue_outer_segments
            pred_inner_segments = rescue_inner_segments

    should_try_low_band_inner_rescue = (
        not should_suppress_inner_segments(candidate)
        and is_central_scene_candidate(candidate, gray.shape)
        and candidate.inner_quad_direct is None
        and candidate.threshold >= 120
        and candidate.outer_bbox[1] >= int(gray.shape[0] * 0.35)
        and len(pred_inner_segments) <= 2
    )
    if should_try_low_band_inner_rescue:
        contour_inner_roles = extract_central_open_inner_segments(candidate)
        rescue_inner_segments = dedupe_segments(
            contour_inner_roles.get("inner_left", [])
            + contour_inner_roles.get("inner_top", [])
            + contour_inner_roles.get("inner_right", [])
            + contour_inner_roles.get("inner_bottom", [])
        )
        if "inner_bottom" in contour_inner_roles and len(rescue_inner_segments) >= 4:
            pred_inner_segments = rescue_inner_segments

    oblique_outer_segments, oblique_inner_segments = build_oblique_rescue_segments(
        candidate,
        edge_support_mask,
        fallback_outer_segments,
        fallback_inner_segments,
        hough_merged_segments,
    )
    if len(oblique_outer_segments) >= 2:
        pred_outer_segments = oblique_outer_segments
    if not should_suppress_inner_segments(candidate) and len(oblique_inner_segments) >= 2:
        pred_inner_segments = oblique_inner_segments
    return edge_support_mask, pred_outer_segments, pred_inner_segments


def quad_from_segments(segments: list[Segment]) -> np.ndarray | None:
    if len(segments) < 3:
        return None
    endpoint_points: list[tuple[int, int]] = []
    for seg in segments:
        endpoint_points.append((seg.x1, seg.y1))
        endpoint_points.append((seg.x2, seg.y2))
    contour = np.asarray(endpoint_points, dtype=np.int32).reshape(-1, 1, 2)
    return approx_quad_from_contour(contour, allow_box_fallback=True)


def load_ground_truth(annotation_json_path: Path) -> GroundTruth:
    payload = json.loads(annotation_json_path.read_text(encoding="utf-8"))
    width = int(payload["image_width"])
    height = int(payload["image_height"])
    outer_segments = parse_segments(payload.get("outer_segments", []))
    inner_segments = parse_segments(payload.get("inner_segments", []))
    outer_mask = render_segments_mask((height, width), outer_segments)
    inner_mask = render_segments_mask((height, width), inner_segments)
    outer_quad = quad_from_segments(outer_segments)
    inner_quad = quad_from_segments(inner_segments)

    if outer_segments and inner_segments:
        mode = "outer+inner"
    elif outer_segments:
        mode = "outer-only"
    elif inner_segments:
        mode = "inner-only"
    else:
        mode = "empty"

    return GroundTruth(
        outer_segments=outer_segments,
        inner_segments=inner_segments,
        outer_mask=outer_mask,
        inner_mask=inner_mask,
        outer_quad=outer_quad,
        inner_quad=inner_quad,
        mode=mode,
    )


def infer_inner_quad(outer_quad: np.ndarray) -> np.ndarray | None:
    if outer_quad is None:
        return None

    outer_model = np.array(
        [
            [0.0, 0.0],
            [OUTER_SIDE_CM, 0.0],
            [OUTER_SIDE_CM, OUTER_SIDE_CM],
            [0.0, OUTER_SIDE_CM],
        ],
        dtype=np.float32,
    )
    inner_model = np.array(
        [
            [TAPE_WIDTH_CM, TAPE_WIDTH_CM],
            [TAPE_WIDTH_CM + INNER_SIDE_CM, TAPE_WIDTH_CM],
            [TAPE_WIDTH_CM + INNER_SIDE_CM, TAPE_WIDTH_CM + INNER_SIDE_CM],
            [TAPE_WIDTH_CM, TAPE_WIDTH_CM + INNER_SIDE_CM],
        ],
        dtype=np.float32,
    )
    homography = cv2.getPerspectiveTransform(outer_model, outer_quad.astype(np.float32))
    inner_quad = cv2.perspectiveTransform(inner_model.reshape(1, -1, 2), homography)[0]
    return order_quad(inner_quad)


def should_replace_direct_inner_quad(
    inner_quad_direct: np.ndarray | None,
    inferred_inner_quad: np.ndarray | None,
) -> bool:
    if inner_quad_direct is None or inferred_inner_quad is None:
        return False

    inferred_area = polygon_area(inferred_inner_quad)
    if inferred_area <= 1e-6:
        return False

    direct_area = polygon_area(inner_quad_direct)
    area_ratio = direct_area / inferred_area

    direct_pts = np.asarray(inner_quad_direct, dtype=np.float32)
    inferred_pts = np.asarray(inferred_inner_quad, dtype=np.float32)
    direct_width = float(direct_pts[:, 0].max() - direct_pts[:, 0].min())
    inferred_width = float(inferred_pts[:, 0].max() - inferred_pts[:, 0].min())
    width_ratio = direct_width / max(inferred_width, 1e-6)
    left_delta = float(direct_pts[:, 0].min() - inferred_pts[:, 0].min())
    top_delta = float(direct_pts[:, 1].min() - inferred_pts[:, 1].min())

    return (
        area_ratio < INNER_DIRECT_AREA_RATIO_THRESHOLD
        and left_delta > INNER_DIRECT_LEFT_DELTA_THRESHOLD
        and width_ratio < INNER_DIRECT_WIDTH_RATIO_THRESHOLD
        and top_delta > INNER_DIRECT_TOP_DELTA_MIN
    )


def choose_inner_quad(
    outer_quad: np.ndarray,
    inner_quad_direct: np.ndarray | None,
) -> np.ndarray | None:
    inferred_inner_quad = infer_inner_quad(outer_quad)
    if inner_quad_direct is None:
        return inferred_inner_quad
    if inferred_inner_quad is None:
        return inner_quad_direct
    if should_replace_direct_inner_quad(inner_quad_direct, inferred_inner_quad):
        return inferred_inner_quad
    return inner_quad_direct


def find_largest_child(contours: list[np.ndarray], hierarchy: np.ndarray, outer_idx: int) -> tuple[np.ndarray | None, float]:
    largest_child: np.ndarray | None = None
    largest_area = 0.0
    child_idx = int(hierarchy[0, outer_idx, 2])
    while child_idx != -1:
        area = float(cv2.contourArea(contours[child_idx]))
        if area > largest_area:
            largest_area = area
            largest_child = contours[child_idx]
        child_idx = int(hierarchy[0, child_idx, 0])
    return largest_child, largest_area


def detect_candidate(gray: np.ndarray, fixed_threshold: int | None = None) -> Candidate | None:
    height, width = gray.shape
    blurred = cv2.GaussianBlur(gray, (3, 3), 0)
    if fixed_threshold is not None:
        thresholds = {int(np.clip(fixed_threshold, 0, 255))}
    else:
        otsu_threshold, _ = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        thresholds = {
            int(np.clip(otsu_threshold + delta, 85, 185))
            for delta in (-12, -6, 0, 6, 12, 18, 24, 30, 36)
        }
        thresholds.update(range(90, 171, 10))

    best: Candidate | None = None
    for threshold in sorted(thresholds):
        _, mask_u8 = cv2.threshold(blurred, threshold, 255, cv2.THRESH_BINARY)
        threshold_mask = mask_u8.astype(bool)
        contours, hierarchy = cv2.findContours(mask_u8, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)
        if not contours or hierarchy is None:
            continue

        for outer_idx, contour in enumerate(contours):
            if int(hierarchy[0, outer_idx, 3]) != -1:
                continue

            outer_area = float(cv2.contourArea(contour))
            min_outer_area = 18.0 if fixed_threshold is not None else 35.0
            if outer_area < min_outer_area:
                continue

            x, y, bw, bh = cv2.boundingRect(contour)
            min_width = 6 if fixed_threshold is not None else 10
            min_height = 2 if fixed_threshold is not None else 3
            if bw < min_width or bh < min_height:
                continue

            outer_quad = approx_quad_from_contour(contour, allow_box_fallback=outer_area >= 85.0)
            if outer_quad is None:
                continue

            child_contour, child_area = find_largest_child(contours, hierarchy, outer_idx)
            inner_quad_direct = None
            if child_contour is not None and child_area >= 18.0:
                inner_quad_direct = approx_quad_from_contour(child_contour, allow_box_fallback=False)

            inner_quad_final = choose_inner_quad(outer_quad, inner_quad_direct)
            touches_border = x == 0 or y == 0 or (x + bw) >= width or (y + bh) >= height
            geom = measure_geometry(
                gray=gray,
                threshold_mask=threshold_mask,
                outer_quad=outer_quad,
                inner_quad=inner_quad_final,
                render_quad_mask=render_quad_mask,
                render_polygon_mask=render_polygon_mask,
            )
            score_breakdown = score_candidate_breakdown(
                bbox=(x, y, bw, bh),
                outer_area=outer_area,
                child_area=child_area,
                touches_border=touches_border,
                geometry=geom,
                image_height=height,
            )

            candidate = Candidate(
                threshold=threshold,
                score=score_breakdown.total_score,
                far_view_factor=score_breakdown.far_view_factor,
                outer_area=outer_area,
                outer_bbox=(x, y, bw, bh),
                child_area=child_area,
                touches_border=touches_border,
                outer_quad=outer_quad,
                inner_quad_direct=inner_quad_direct,
                inner_quad_final=inner_quad_final,
                threshold_mask=threshold_mask,
                geometry_score=geom.geometry_score,
                ring_mean=geom.ring_mean,
                inner_mean=geom.inner_mean,
                ring_on_ratio=geom.ring_on_ratio,
                inner_on_ratio=geom.inner_on_ratio,
                outer_support=geom.outer_support,
                inner_support=geom.inner_support,
            )
            if best is None or candidate.score > best.score:
                best = candidate

    if best is None:
        return None
    best_breakdown = score_candidate_breakdown(
        bbox=best.outer_bbox,
        outer_area=best.outer_area,
        child_area=best.child_area,
        touches_border=best.touches_border,
        geometry=measure_geometry(
            gray=gray,
            threshold_mask=best.threshold_mask,
            outer_quad=best.outer_quad,
            inner_quad=best.inner_quad_final,
            render_quad_mask=render_quad_mask,
            render_polygon_mask=render_polygon_mask,
        ),
        image_height=height,
    )
    if best.score < best_breakdown.decision_threshold:
        return None
    return best


def detect_best_candidate(
    gray: np.ndarray,
    fixed_threshold: int | None = None,
    fixed_thresholds: list[int] | tuple[int, ...] | None = None,
) -> Candidate | None:
    if fixed_thresholds:
        candidates_by_threshold: dict[int, Candidate] = {}
        for threshold in dict.fromkeys(int(value) for value in fixed_thresholds):
            candidate = detect_candidate(gray, fixed_threshold=threshold)
            if candidate is not None:
                candidates_by_threshold[threshold] = candidate

        best = max(candidates_by_threshold.values(), key=lambda item: item.score, default=None)
        candidate_120 = candidates_by_threshold.get(120)
        candidate_125 = candidates_by_threshold.get(125)
        if candidate_120 is not None and candidate_125 is not None:
            area_delta = (candidate_120.outer_area - candidate_125.outer_area) / max(
                candidate_120.outer_area,
                candidate_125.outer_area,
                1e-6,
            )
            height_delta = (candidate_120.outer_bbox[3] - candidate_125.outer_bbox[3]) / max(
                candidate_120.outer_bbox[3],
                candidate_125.outer_bbox[3],
                1.0,
            )
            choose_125 = (
                (candidate_125.score - candidate_120.score)
                + THRESHOLD_125_AREA_WEIGHT * area_delta
                + THRESHOLD_125_HEIGHT_WEIGHT * height_delta
            ) >= 0.0
            preferred_mid = candidate_125 if choose_125 else candidate_120
            if best is None or preferred_mid.score + THRESHOLD_MID_PREFERENCE_MAX_SCORE_DELTA >= best.score:
                best = preferred_mid
        elif best is None and (candidate_120 is not None or candidate_125 is not None):
            best = candidate_125 if candidate_125 is not None else candidate_120

        candidate_110 = candidates_by_threshold.get(110)
        image_height, image_width = gray.shape
        if candidate_110 is not None and best is not None:
            candidate_110_width_ratio = candidate_110.outer_bbox[2] / max(float(image_width), 1.0)
            best_width_ratio = best.outer_bbox[2] / max(float(image_width), 1.0)
            if (
                candidate_110.score >= best.score + THRESHOLD_110_RECOVERY_MIN_SCORE_GAIN
                and candidate_110_width_ratio >= CENTRAL_SCENE_MIN_WIDTH_RATIO
                and candidate_110.outer_bbox[1] <= best.outer_bbox[1] - THRESHOLD_110_RECOVERY_MIN_Y_GAIN
                and candidate_110.outer_bbox[3] >= best.outer_bbox[3] + THRESHOLD_110_RECOVERY_MIN_HEIGHT_GAIN
            ):
                best = candidate_110
            elif (
                candidate_110_width_ratio >= CENTRAL_SCENE_MIN_WIDTH_RATIO
                and (
                    best_width_ratio <= THRESHOLD_110_RECOVERY_MAX_BEST_WIDTH_RATIO
                    or best.outer_bbox[0] >= int(image_width * 0.5)
                )
                and candidate_110.score + THRESHOLD_110_RECOVERY_MAX_SCORE_DELTA >= best.score
            ):
                best = candidate_110

        fallback_candidate = candidates_by_threshold.get(FAR_SMALL_FALLBACK_THRESHOLD)
        if fallback_candidate is not None and (
            best is None
            or (
                best.far_view_factor >= FAR_SMALL_FALLBACK_FAR_VIEW
                and best.outer_area < FAR_SMALL_FALLBACK_OUTER_AREA
            )
        ):
            return fallback_candidate
        return best
    return detect_candidate(gray, fixed_threshold=fixed_threshold)


def mask_scores(pred_mask: np.ndarray, gt_mask: np.ndarray, radius: int = 1) -> dict[str, float]:
    kernel = np.ones((radius * 2 + 1, radius * 2 + 1), dtype=np.uint8)
    pred_d = cv2.dilate(pred_mask.astype(np.uint8), kernel, iterations=1).astype(bool)
    gt_d = cv2.dilate(gt_mask.astype(np.uint8), kernel, iterations=1).astype(bool)

    pred_pixels = int(pred_mask.sum())
    gt_pixels = int(gt_mask.sum())
    if pred_pixels == 0 and gt_pixels == 0:
        return {"precision": 1.0, "recall": 1.0, "f1": 1.0}
    if pred_pixels == 0:
        return {"precision": 0.0, "recall": 0.0, "f1": 0.0}
    if gt_pixels == 0:
        return {"precision": 0.0, "recall": 0.0, "f1": 0.0}

    precision = float((pred_mask & gt_d).sum() / max(1, pred_pixels))
    recall = float((gt_mask & pred_d).sum() / max(1, gt_pixels))
    if precision + recall == 0.0:
        f1 = 0.0
    else:
        f1 = 2.0 * precision * recall / (precision + recall)
    return {"precision": precision, "recall": recall, "f1": f1}


def align_quad(reference: np.ndarray, predicted: np.ndarray) -> np.ndarray:
    best = predicted
    best_score = float("inf")
    variants = []
    for reversed_order in (False, True):
        pts = predicted[::-1].copy() if reversed_order else predicted.copy()
        for shift in range(4):
            variants.append(np.roll(pts, -shift, axis=0))
    for variant in variants:
        score = float(np.mean(np.linalg.norm(variant - reference, axis=1)))
        if score < best_score:
            best_score = score
            best = variant
    return best


def quad_corner_error(pred_quad: np.ndarray | None, gt_quad: np.ndarray | None) -> dict[str, float] | None:
    if pred_quad is None or gt_quad is None:
        return None
    aligned = align_quad(gt_quad, pred_quad)
    distances = np.linalg.norm(aligned - gt_quad, axis=1)
    return {
        "mean_px": float(distances.mean()),
        "max_px": float(distances.max()),
    }


def make_threshold_debug(
    threshold_mask: np.ndarray,
    outer_segments: list[Segment],
    inner_segments: list[Segment],
) -> np.ndarray:
    canvas = np.zeros((threshold_mask.shape[0], threshold_mask.shape[1], 3), dtype=np.uint8)
    canvas[threshold_mask] = (255, 255, 255)
    for seg in outer_segments:
        cv2.line(canvas, (seg.x1, seg.y1), (seg.x2, seg.y2), (255, 180, 0), thickness=1, lineType=cv2.LINE_8)
    for seg in inner_segments:
        cv2.line(canvas, (seg.x1, seg.y1), (seg.x2, seg.y2), (0, 220, 255), thickness=1, lineType=cv2.LINE_8)
    canvas = cv2.resize(canvas, (canvas.shape[1] * SCALE, canvas.shape[0] * SCALE), interpolation=cv2.INTER_NEAREST)
    image = Image.fromarray(canvas, mode="RGB")
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), "binary + gt", fill=(255, 255, 255))
    return np.asarray(image)


def scaled_gray_rgb(gray: np.ndarray) -> np.ndarray:
    rgb = np.repeat(gray[:, :, None], 3, axis=2)
    return cv2.resize(rgb, (gray.shape[1] * SCALE, gray.shape[0] * SCALE), interpolation=cv2.INTER_NEAREST)


def draw_segments_original(rgb: np.ndarray, segments: list[Segment], color: tuple[int, int, int]) -> np.ndarray:
    canvas = rgb.copy()
    for seg in segments:
        cv2.line(canvas, (seg.x1, seg.y1), (seg.x2, seg.y2), color, thickness=1, lineType=cv2.LINE_8)
    return canvas


def add_title_to_scaled(rgb: np.ndarray, title: str) -> np.ndarray:
    scaled = cv2.resize(rgb, (rgb.shape[1] * SCALE, rgb.shape[0] * SCALE), interpolation=cv2.INTER_NEAREST)
    image = Image.fromarray(scaled, mode="RGB")
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), title, fill=(255, 255, 255))
    return np.asarray(image)


def make_gt_overlay(gray: np.ndarray, gt: GroundTruth, title: str) -> np.ndarray:
    base = np.repeat(gray[:, :, None], 3, axis=2)
    base = draw_segments_original(base, gt.outer_segments, (255, 180, 0))
    base = draw_segments_original(base, gt.inner_segments, (0, 220, 255))
    return add_title_to_scaled(base, title)


def make_prediction_overlay(
    gray: np.ndarray,
    pred_outer_segments: list[Segment],
    pred_inner_segments: list[Segment],
    title: str,
) -> np.ndarray:
    base = np.repeat(gray[:, :, None], 3, axis=2)
    base = draw_segments_original(base, pred_outer_segments, (255, 0, 0))
    base = draw_segments_original(base, pred_inner_segments, (0, 255, 0))
    return add_title_to_scaled(base, title)


def make_comparison_overlay(
    gray: np.ndarray,
    gt: GroundTruth,
    pred_outer_segments: list[Segment],
    pred_inner_segments: list[Segment],
    title: str,
) -> np.ndarray:
    base = np.repeat(gray[:, :, None], 3, axis=2)
    base = draw_segments_original(base, gt.outer_segments, (255, 180, 0))
    base = draw_segments_original(base, gt.inner_segments, (0, 220, 255))
    base = draw_segments_original(base, pred_outer_segments, (255, 0, 0))
    base = draw_segments_original(base, pred_inner_segments, (0, 255, 0))
    return add_title_to_scaled(base, title)


def quad_to_list(quad: np.ndarray | None) -> list[list[float]] | None:
    if quad is None:
        return None
    return np.round(quad, 2).tolist()


def evaluate_sample(sample: Sample, output_dir: Path, fixed_threshold: int | None = None) -> dict:
    gray = read_gray(sample.original_path)
    annotation_rgb = read_rgb(sample.annotation_png_path)
    gt = load_ground_truth(sample.annotation_json_path)
    candidate = detect_candidate(gray, fixed_threshold=fixed_threshold)

    if candidate is None:
        threshold_mask = np.zeros_like(gray, dtype=bool)
        pred_outer_segments = []
        pred_inner_segments = []
    else:
        threshold_mask = candidate.threshold_mask
    _, pred_outer_segments, pred_inner_segments = extract_prediction_segments(gray, candidate)

    pred_outer_mask = render_segments_mask(gray.shape, pred_outer_segments)
    pred_inner_mask = render_segments_mask(gray.shape, pred_inner_segments)
    outer_scores = mask_scores(pred_outer_mask, gt.outer_mask)
    inner_scores = mask_scores(pred_inner_mask, gt.inner_mask)

    outer_corner = quad_corner_error(None if candidate is None else candidate.outer_quad, gt.outer_quad)
    inner_corner = quad_corner_error(None if candidate is None else candidate.inner_quad_final, gt.inner_quad)

    sample_dir = output_dir / sample.prefix / f"frame_{sample.frame_idx:06d}"
    sample_dir.mkdir(parents=True, exist_ok=True)

    gt_overlay = make_gt_overlay(gray, gt, f"{sample.frame_name} gt {gt.mode}")
    pred_title = f"{sample.frame_name} pred score={candidate.score:.2f}" if candidate is not None else f"{sample.frame_name} pred score=NA"
    pred_overlay = make_prediction_overlay(gray, pred_outer_segments, pred_inner_segments, pred_title)
    cmp_overlay = make_comparison_overlay(
        gray,
        gt,
        pred_outer_segments,
        pred_inner_segments,
        f"{sample.frame_name} gt=orange/cyan pred=red/green",
    )
    threshold_debug = make_threshold_debug(threshold_mask, gt.outer_segments, gt.inner_segments)

    save_gray(sample_dir / "01_original.png", gray)
    save_rgb(sample_dir / "02_annotation_png.png", annotation_rgb)
    save_rgb(sample_dir / "03_gt_overlay.png", gt_overlay)
    save_rgb(sample_dir / "04_prediction_overlay.png", pred_overlay)
    save_rgb(sample_dir / "05_comparison_overlay.png", cmp_overlay)
    save_rgb(sample_dir / "06_threshold_debug.png", threshold_debug)
    save_mask(sample_dir / "07_gt_outer_mask.png", gt.outer_mask)
    save_mask(sample_dir / "08_gt_inner_mask.png", gt.inner_mask)
    save_mask(sample_dir / "09_pred_outer_mask.png", pred_outer_mask)
    save_mask(sample_dir / "10_pred_inner_mask.png", pred_inner_mask)

    strict_role_mean_f1 = (outer_scores["f1"] + inner_scores["f1"]) / 2.0
    annotated_role_values = [outer_scores["f1"]]
    if gt.inner_segments:
        annotated_role_values.append(inner_scores["f1"])
    annotated_role_mean_f1 = float(np.mean(annotated_role_values)) if annotated_role_values else 0.0
    result = {
        "frame_name": sample.frame_name,
        "prefix": sample.prefix,
        "frame_idx": sample.frame_idx,
        "annotation_mode": gt.mode,
        "paths": {
            "original": str(sample.original_path),
            "annotation_png": str(sample.annotation_png_path),
            "annotation_json": str(sample.annotation_json_path),
            "output_dir": str(sample_dir),
        },
        "threshold_mode": {
            "type": "fixed" if fixed_threshold is not None else "sweep",
            "value": fixed_threshold,
        },
        "ground_truth": {
            "outer_segment_count": len(gt.outer_segments),
            "inner_segment_count": len(gt.inner_segments),
            "outer_segments": [segment.__dict__ for segment in gt.outer_segments],
            "inner_segments": [segment.__dict__ for segment in gt.inner_segments],
            "outer_quad": quad_to_list(gt.outer_quad),
            "inner_quad": quad_to_list(gt.inner_quad),
        },
        "detection": None,
        "metrics": {
            "outer_line": outer_scores,
            "inner_line": inner_scores,
            "strict_role_mean_f1": strict_role_mean_f1,
            "annotated_role_mean_f1": annotated_role_mean_f1,
            "outer_corner_error_px": outer_corner,
            "inner_corner_error_px": inner_corner,
            "outer_present_gt": bool(gt.outer_segments),
            "inner_present_gt": bool(gt.inner_segments),
            "outer_present_pred": candidate is not None,
            "inner_present_pred": bool(pred_inner_segments),
        },
    }

    if candidate is not None:
        result["detection"] = {
            "threshold": candidate.threshold,
            "score": round(candidate.score, 4),
            "far_view_factor": round(candidate.far_view_factor, 4),
            "inner_suppressed": should_suppress_inner_segments(candidate),
            "outer_area": round(candidate.outer_area, 2),
            "outer_bbox": list(candidate.outer_bbox),
            "child_area": round(candidate.child_area, 2),
            "touches_border": candidate.touches_border,
            "outer_quad": quad_to_list(candidate.outer_quad),
            "inner_quad_direct": quad_to_list(candidate.inner_quad_direct),
            "inner_quad_final": quad_to_list(candidate.inner_quad_final),
            "geometry_score": round(candidate.geometry_score, 4),
            "ring_mean": round(candidate.ring_mean, 3),
            "inner_mean": round(candidate.inner_mean, 3),
            "ring_on_ratio": round(candidate.ring_on_ratio, 4),
            "inner_on_ratio": round(candidate.inner_on_ratio, 4),
            "outer_support": round(candidate.outer_support, 4),
            "inner_support": round(candidate.inner_support, 4),
            "pred_outer_segments": [segment.__dict__ for segment in pred_outer_segments],
            "pred_inner_segments": [segment.__dict__ for segment in pred_inner_segments],
        }

    (sample_dir / "result.json").write_text(
        json.dumps(result, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    return result


def build_contact_sheet(results: list[dict], output_path: Path) -> None:
    tiles: list[Image.Image] = []
    for result in results:
        sample_dir = Path(result["paths"]["output_dir"])
        comparison = Image.open(sample_dir / "05_comparison_overlay.png").convert("RGB")
        binary = Image.open(sample_dir / "06_threshold_debug.png").convert("RGB")
        tile = Image.new("RGB", (comparison.width, comparison.height + binary.height), (18, 18, 18))
        tile.paste(comparison, (0, 0))
        tile.paste(binary, (0, comparison.height))
        tiles.append(tile)

    if not tiles:
        return

    cols = 3
    rows = math.ceil(len(tiles) / cols)
    tile_w, tile_h = tiles[0].size
    sheet = Image.new("RGB", (cols * tile_w, rows * tile_h), (18, 18, 18))
    for idx, tile in enumerate(tiles):
        x = (idx % cols) * tile_w
        y = (idx // cols) * tile_h
        sheet.paste(tile, (x, y))
    output_path.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output_path)


def write_all_annotation_index(output_dir: Path) -> None:
    rows: list[dict] = []
    for json_path in sorted(ANNOTATION_DIR.glob("*.json")):
        payload = json.loads(json_path.read_text(encoding="utf-8"))
        outer_segments = parse_segments(payload.get("outer_segments", []))
        inner_segments = parse_segments(payload.get("inner_segments", []))
        rows.append(
            {
                "frame_name": payload["image_name"],
                "outer_segment_count": len(outer_segments),
                "inner_segment_count": len(inner_segments),
                "mode": (
                    "outer+inner"
                    if outer_segments and inner_segments
                    else "outer-only"
                    if outer_segments
                    else "inner-only"
                    if inner_segments
                    else "empty"
                ),
                "outer_segments": [segment.__dict__ for segment in outer_segments],
                "inner_segments": [segment.__dict__ for segment in inner_segments],
            }
        )
    (output_dir / "all_annotation_index.json").write_text(
        json.dumps(rows, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )


def write_summary(results: list[dict], output_dir: Path, fixed_threshold: int | None = None) -> None:
    outer_f1 = [result["metrics"]["outer_line"]["f1"] for result in results if result["metrics"]["outer_present_gt"]]
    inner_f1 = [result["metrics"]["inner_line"]["f1"] for result in results if result["metrics"]["inner_present_gt"]]
    strict_role_f1 = [result["metrics"]["strict_role_mean_f1"] for result in results]
    annotated_role_f1 = [result["metrics"]["annotated_role_mean_f1"] for result in results]

    outer_corner_errors = [
        result["metrics"]["outer_corner_error_px"]["mean_px"]
        for result in results
        if result["metrics"]["outer_corner_error_px"] is not None
    ]
    inner_corner_errors = [
        result["metrics"]["inner_corner_error_px"]["mean_px"]
        for result in results
        if result["metrics"]["inner_corner_error_px"] is not None
    ]
    modes: dict[str, int] = {}
    for result in results:
        modes[result["annotation_mode"]] = modes.get(result["annotation_mode"], 0) + 1

    summary = {
        "dataset": "雷区peoplev2 representative subset",
        "sample_count": len(results),
        "annotation_modes": modes,
        "mean_outer_f1": float(np.mean(outer_f1)) if outer_f1 else 0.0,
        "mean_inner_f1": float(np.mean(inner_f1)) if inner_f1 else 0.0,
        "mean_strict_role_f1": float(np.mean(strict_role_f1)) if strict_role_f1 else 0.0,
        "mean_annotated_role_f1": float(np.mean(annotated_role_f1)) if annotated_role_f1 else 0.0,
        "mean_outer_corner_error_px": float(np.mean(outer_corner_errors)) if outer_corner_errors else None,
        "mean_inner_corner_error_px": float(np.mean(inner_corner_errors)) if inner_corner_errors else None,
        "geometry_assumptions": {
            "inner_square_side_cm": INNER_SIDE_CM,
            "outer_square_side_cm": OUTER_SIDE_CM,
            "tape_width_cm": TAPE_WIDTH_CM,
            "inner_generation": "infer by projective inset from detected outer quad when direct inner contour is absent",
        },
        "threshold_mode": {
            "type": "fixed" if fixed_threshold is not None else "sweep",
            "value": fixed_threshold,
        },
        "results": results,
    }

    lines = [
        "雷区 peoplev2 代表样本结果",
        f"样本数: {len(results)}",
        f"标注模式统计: {modes}",
        f"外框线段平均 F1: {summary['mean_outer_f1']:.3f}",
        f"内框线段平均 F1: {summary['mean_inner_f1']:.3f}",
        f"只按已标注线段统计的平均 F1: {summary['mean_annotated_role_f1']:.3f}",
        f"严格口径角色平均 F1: {summary['mean_strict_role_f1']:.3f}",
        (
            f"阈值模式: 固定阈值 {fixed_threshold}"
            if fixed_threshold is not None
            else "阈值模式: Otsu 派生阈值扫描"
        ),
        (
            "外框角点平均误差(px): "
            f"{summary['mean_outer_corner_error_px']:.3f}"
            if summary["mean_outer_corner_error_px"] is not None
            else "外框角点平均误差(px): N/A"
        ),
        (
            "内框角点平均误差(px): "
            f"{summary['mean_inner_corner_error_px']:.3f}"
            if summary["mean_inner_corner_error_px"] is not None
            else "内框角点平均误差(px): N/A"
        ),
        "",
        "当前几何约束:",
        f"1. 内框按 {INNER_SIDE_CM:.0f}x{INNER_SIDE_CM:.0f} cm 处理。",
        f"2. 外框按 {OUTER_SIDE_CM:.0f}x{OUTER_SIDE_CM:.0f} cm 处理。",
        f"3. 胶带宽度按 {TAPE_WIDTH_CM:.0f} cm 处理。",
        "4. 近景若看得到内框，优先使用检测到的内轮廓；否则从外框按 projective inset 反推内框。",
        "",
        "文件说明:",
        "1. `03_gt_overlay.png` 只画 JSON 真值线段。",
        "2. `04_prediction_overlay.png` 只画算法预测线。",
        "3. `05_comparison_overlay.png` 叠加显示真值和预测，真值是橙/青，预测是红/绿。",
        "4. `06_threshold_debug.png` 中白色是阈值结果，橙/青是真值线段。",
    ]

    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    (output_dir / "summary.txt").write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    args = parse_args()
    samples = build_samples()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    results = [evaluate_sample(sample, args.output_dir, fixed_threshold=args.fixed_threshold) for sample in samples]
    build_contact_sheet(results, args.output_dir / "contact_sheet.png")
    write_summary(results, args.output_dir, fixed_threshold=args.fixed_threshold)
    write_all_annotation_index(args.output_dir)

    print(f"output_dir: {args.output_dir}")
    print(f"samples: {len(results)}")
    print(f"mean_annotated_role_f1: {np.mean([result['metrics']['annotated_role_mean_f1'] for result in results]):.4f}")


if __name__ == "__main__":
    main()
