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


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DATA_ROOT = PROJECT_ROOT / "data/雷区室外偏振片"
ANNOTATION_DIR = DATA_ROOT / "frames/雷区peoplev2"
OUTPUT_DIR = DATA_ROOT / "peoplev2_geometry_representative"

SCALE = 6
LINE_THICKNESS = 1
OUTER_SIDE_CM = 120.0
INNER_SIDE_CM = 100.0
TAPE_WIDTH_CM = (OUTER_SIDE_CM - INNER_SIDE_CM) / 2.0

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


def clamp01(value: float) -> float:
    return max(0.0, min(1.0, value))


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


def line_support(source_mask: np.ndarray, line_mask: np.ndarray, radius: int = 1) -> float:
    if not line_mask.any():
        return 0.0
    kernel = np.ones((radius * 2 + 1, radius * 2 + 1), dtype=np.uint8)
    dilated = cv2.dilate(source_mask.astype(np.uint8), kernel, iterations=1).astype(bool)
    return float((line_mask & dilated).sum() / max(1, line_mask.sum()))


def geometry_features(
    gray: np.ndarray,
    threshold_mask: np.ndarray,
    outer_quad: np.ndarray,
    inner_quad: np.ndarray | None,
) -> dict[str, float]:
    if inner_quad is None:
        return {
            "geometry_score": 0.0,
            "ring_mean": 0.0,
            "inner_mean": 0.0,
            "ring_on_ratio": 0.0,
            "inner_on_ratio": 0.0,
            "outer_support": line_support(threshold_mask, render_quad_mask(gray.shape, outer_quad)),
            "inner_support": 0.0,
        }

    outer_fill = render_polygon_mask(gray.shape, outer_quad)
    inner_fill = render_polygon_mask(gray.shape, inner_quad)
    ring_fill = outer_fill & ~inner_fill

    ring_mean = float(gray[ring_fill].mean()) if ring_fill.any() else 0.0
    inner_mean = float(gray[inner_fill].mean()) if inner_fill.any() else 0.0
    ring_on_ratio = float(threshold_mask[ring_fill].mean()) if ring_fill.any() else 0.0
    inner_on_ratio = float(threshold_mask[inner_fill].mean()) if inner_fill.any() else 0.0

    outer_support = line_support(threshold_mask, render_quad_mask(gray.shape, outer_quad))
    inner_support = line_support(threshold_mask, render_quad_mask(gray.shape, inner_quad))
    support_score = 0.5 * (outer_support + inner_support)
    ring_score = clamp01(ring_on_ratio)
    hollow_score = clamp01((ring_on_ratio - inner_on_ratio + 0.25) / 0.7)
    contrast_score = clamp01((ring_mean - inner_mean + 6.0) / 45.0)
    geometry_score = (
        0.32 * support_score
        + 0.28 * ring_score
        + 0.22 * hollow_score
        + 0.18 * contrast_score
    )

    return {
        "geometry_score": geometry_score,
        "ring_mean": ring_mean,
        "inner_mean": inner_mean,
        "ring_on_ratio": ring_on_ratio,
        "inner_on_ratio": inner_on_ratio,
        "outer_support": outer_support,
        "inner_support": inner_support,
    }


def score_candidate(
    bbox: tuple[int, int, int, int],
    outer_area: float,
    child_area: float,
    touches_border: bool,
    geometry_score: float,
    ring_on_ratio: float,
    inner_on_ratio: float,
) -> float:
    x, y, w, h = bbox
    area_score = min(outer_area / 380.0, 1.0)
    width_score = min(w / 58.0, 1.0)
    height_score = min(h / 21.0, 1.0)
    child_score = min(child_area / 220.0, 1.0)
    size_score = (
        0.42 * area_score
        + 0.20 * width_score
        + 0.14 * height_score
        + 0.24 * child_score
    )

    score = 0.52 * size_score + 0.48 * geometry_score
    if outer_area < 90.0 and child_area < 20.0:
        score -= 0.12
    if y < 7 and h < 8 and geometry_score < 0.38:
        score -= 0.10
    if ring_on_ratio < 0.12 and inner_on_ratio < 0.12:
        score -= 0.08
    if touches_border and outer_area < 75.0:
        score -= 0.05
    if x == 0 and w < 16 and outer_area < 80.0:
        score -= 0.06
    return score


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
            if outer_area < 35.0:
                continue

            x, y, bw, bh = cv2.boundingRect(contour)
            if bw < 10 or bh < 3:
                continue

            outer_quad = approx_quad_from_contour(contour, allow_box_fallback=outer_area >= 85.0)
            if outer_quad is None:
                continue

            child_contour, child_area = find_largest_child(contours, hierarchy, outer_idx)
            inner_quad_direct = None
            if child_contour is not None and child_area >= 18.0:
                inner_quad_direct = approx_quad_from_contour(child_contour, allow_box_fallback=False)

            inner_quad_final = inner_quad_direct if inner_quad_direct is not None else infer_inner_quad(outer_quad)
            touches_border = x == 0 or y == 0 or (x + bw) >= width or (y + bh) >= height
            geom = geometry_features(
                gray=gray,
                threshold_mask=threshold_mask,
                outer_quad=outer_quad,
                inner_quad=inner_quad_final,
            )
            score = score_candidate(
                bbox=(x, y, bw, bh),
                outer_area=outer_area,
                child_area=child_area,
                touches_border=touches_border,
                geometry_score=geom["geometry_score"],
                ring_on_ratio=geom["ring_on_ratio"],
                inner_on_ratio=geom["inner_on_ratio"],
            )

            candidate = Candidate(
                threshold=threshold,
                score=score,
                outer_area=outer_area,
                outer_bbox=(x, y, bw, bh),
                child_area=child_area,
                touches_border=touches_border,
                outer_quad=outer_quad,
                inner_quad_direct=inner_quad_direct,
                inner_quad_final=inner_quad_final,
                threshold_mask=threshold_mask,
                geometry_score=geom["geometry_score"],
                ring_mean=geom["ring_mean"],
                inner_mean=geom["inner_mean"],
                ring_on_ratio=geom["ring_on_ratio"],
                inner_on_ratio=geom["inner_on_ratio"],
                outer_support=geom["outer_support"],
                inner_support=geom["inner_support"],
            )
            if best is None or candidate.score > best.score:
                best = candidate

    if best is None or best.score < 0.34:
        return None
    return best


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
    return canvas


def scaled_gray_rgb(gray: np.ndarray) -> np.ndarray:
    rgb = np.repeat(gray[:, :, None], 3, axis=2)
    return cv2.resize(rgb, (gray.shape[1] * SCALE, gray.shape[0] * SCALE), interpolation=cv2.INTER_NEAREST)


def draw_segments(draw: ImageDraw.ImageDraw, segments: list[Segment], color: tuple[int, int, int], width: int) -> None:
    for seg in segments:
        draw.line(
            [(seg.x1 * SCALE, seg.y1 * SCALE), (seg.x2 * SCALE, seg.y2 * SCALE)],
            fill=color,
            width=width,
        )


def draw_quad(draw: ImageDraw.ImageDraw, quad: np.ndarray | None, color: tuple[int, int, int], width: int) -> None:
    if quad is None:
        return
    pts = [tuple((np.asarray(point) * SCALE).tolist()) for point in quad]
    pts.append(pts[0])
    draw.line(pts, fill=color, width=width)


def make_gt_overlay(gray: np.ndarray, gt: GroundTruth, title: str) -> np.ndarray:
    image = Image.fromarray(scaled_gray_rgb(gray), mode="RGB")
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), title, fill=(255, 255, 255))
    draw_segments(draw, gt.outer_segments, (255, 180, 0), 2)
    draw_segments(draw, gt.inner_segments, (0, 220, 255), 2)
    return np.asarray(image)


def make_prediction_overlay(gray: np.ndarray, candidate: Candidate | None, title: str) -> np.ndarray:
    image = Image.fromarray(scaled_gray_rgb(gray), mode="RGB")
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), title, fill=(255, 255, 255))
    if candidate is not None:
        draw_quad(draw, candidate.outer_quad, (255, 0, 0), 2)
        draw_quad(draw, candidate.inner_quad_final, (0, 255, 0), 2)
    return np.asarray(image)


def make_comparison_overlay(gray: np.ndarray, gt: GroundTruth, candidate: Candidate | None, title: str) -> np.ndarray:
    image = Image.fromarray(scaled_gray_rgb(gray), mode="RGB")
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), title, fill=(255, 255, 255))
    draw_segments(draw, gt.outer_segments, (255, 180, 0), 1)
    draw_segments(draw, gt.inner_segments, (0, 220, 255), 1)
    if candidate is not None:
        draw_quad(draw, candidate.outer_quad, (255, 0, 0), 2)
        draw_quad(draw, candidate.inner_quad_final, (0, 255, 0), 2)
    return np.asarray(image)


def quad_to_list(quad: np.ndarray | None) -> list[list[float]] | None:
    if quad is None:
        return None
    return np.round(quad, 2).tolist()


def evaluate_sample(sample: Sample, output_dir: Path, fixed_threshold: int | None = None) -> dict:
    gray = read_gray(sample.original_path)
    annotation_rgb = read_rgb(sample.annotation_png_path)
    gt = load_ground_truth(sample.annotation_json_path)
    candidate = detect_candidate(gray, fixed_threshold=fixed_threshold)

    pred_outer_mask = render_quad_mask(gray.shape, None if candidate is None else candidate.outer_quad)
    pred_inner_mask = render_quad_mask(gray.shape, None if candidate is None else candidate.inner_quad_final)
    outer_scores = mask_scores(pred_outer_mask, gt.outer_mask)
    inner_scores = mask_scores(pred_inner_mask, gt.inner_mask)

    outer_corner = quad_corner_error(None if candidate is None else candidate.outer_quad, gt.outer_quad)
    inner_corner = quad_corner_error(None if candidate is None else candidate.inner_quad_final, gt.inner_quad)

    sample_dir = output_dir / sample.prefix / f"frame_{sample.frame_idx:06d}"
    sample_dir.mkdir(parents=True, exist_ok=True)

    gt_overlay = make_gt_overlay(gray, gt, f"{sample.frame_name} gt {gt.mode}")
    pred_title = f"{sample.frame_name} pred score={candidate.score:.2f}" if candidate is not None else f"{sample.frame_name} pred score=NA"
    pred_overlay = make_prediction_overlay(gray, candidate, pred_title)
    cmp_overlay = make_comparison_overlay(gray, gt, candidate, f"{sample.frame_name} gt=orange/cyan pred=red/green")
    threshold_mask = candidate.threshold_mask if candidate is not None else np.zeros_like(gray, dtype=bool)
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
            "inner_present_pred": candidate is not None and candidate.inner_quad_final is not None,
        },
    }

    if candidate is not None:
        result["detection"] = {
            "threshold": candidate.threshold,
            "score": round(candidate.score, 4),
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
        tiles.append(Image.open(sample_dir / "05_comparison_overlay.png").convert("RGB"))

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
