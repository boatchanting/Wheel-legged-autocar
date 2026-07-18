"""Minefield detection from separable small-kernel gradient responses.

The image is never thresholded by intensity.  The only convolutions are the
two rank-one 4x4 FIR filters:

    Gx = [1, 3, 3, 1]^T * [-1, -1, 1, 1]
    Gy = [-1, -1, 1, 1]^T * [1, 3, 3, 1]

Each kernel is separable and its largest dimension is four.  Their L1
gradient magnitude is then sparsified only for line fitting; that response
selection is not a brightness/binary-image segmentation step.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import cv2
import numpy as np

from detect_minefield_people_v2_json import (
    DATA_ROOT,
    SCALE,
    Segment,
    load_ground_truth,
    make_comparison_overlay,
    make_gt_overlay,
    make_prediction_overlay,
    mask_scores,
    quad_corner_error,
    quad_to_list,
    read_gray,
    read_rgb,
    render_segments_mask,
    save_gray,
    save_mask,
    save_rgb,
)


SMOOTH = np.array([1.0, 3.0, 3.0, 1.0], dtype=np.float32) / 8.0
DIFF = np.array([-1.0, -1.0, 1.0, 1.0], dtype=np.float32) / 4.0
EDGE_PERCENTILE = 90.0
MIN_EDGE_RESPONSE = 12.0
HOUGH_THRESHOLD = 9
HOUGH_MIN_LENGTH = 9
HOUGH_MAX_GAP = 4
SLOPE_HORIZONTAL = 0.12
CLUSTER_GAP_PX = 2.5


@dataclass
class EdgeCandidate:
    """Compatibility payload consumed by the existing peoplev3 evaluator."""

    threshold: int
    score: float
    far_view_factor: float
    outer_area: float
    outer_bbox: tuple[int, int, int, int]
    child_area: float
    touches_border: bool
    outer_quad: np.ndarray | None
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
    outer_segments: list[Segment]
    inner_segments: list[Segment]


@dataclass
class LineCluster:
    slope_kind: str
    y_at_center: float
    segment: Segment
    support: float


def gradient_edges(gray: np.ndarray) -> tuple[np.ndarray, np.ndarray, int]:
    """Return a Sobel-L1 response map and a response-selected edge map.

    ``sepFilter2D`` performs the horizontal and vertical FIR passes implied by
    the two outer products above.  The binomial 1x4 factor suppresses texture
    before the 1x4 box-difference factor; no Gaussian blur or brightness
    threshold is applied before the differentiators.
    """

    signal = gray.astype(np.float32)
    gx = cv2.sepFilter2D(signal, cv2.CV_32F, DIFF, SMOOTH, borderType=cv2.BORDER_REPLICATE)
    gy = cv2.sepFilter2D(signal, cv2.CV_32F, SMOOTH, DIFF, borderType=cv2.BORDER_REPLICATE)
    magnitude = np.abs(gx) + np.abs(gy)
    response_threshold = int(round(max(MIN_EDGE_RESPONSE, float(np.percentile(magnitude, EDGE_PERCENTILE)))))
    edges = magnitude >= response_threshold
    return magnitude, edges, response_threshold


def normalize_segment(p0: np.ndarray, p1: np.ndarray) -> Segment:
    if p0[0] > p1[0] or (p0[0] == p1[0] and p0[1] > p1[1]):
        p0, p1 = p1, p0
    return Segment(
        x1=int(round(float(p0[0]))),
        y1=int(round(float(p0[1]))),
        x2=int(round(float(p1[0]))),
        y2=int(round(float(p1[1]))),
    )


def segment_length(segment: Segment) -> float:
    return float(np.hypot(segment.x2 - segment.x1, segment.y2 - segment.y1))


def line_y_at_center(segment: Segment, center_x: float) -> float:
    dx = float(segment.x2 - segment.x1)
    if abs(dx) < 1e-5:
        return float((segment.y1 + segment.y2) * 0.5)
    return float(segment.y1 + (center_x - segment.x1) * (segment.y2 - segment.y1) / dx)


def classify_line(segment: Segment) -> str | None:
    dx = float(segment.x2 - segment.x1)
    dy = float(segment.y2 - segment.y1)
    if abs(dx) < 2.0:
        return None
    slope = dy / dx
    if abs(slope) <= SLOPE_HORIZONTAL:
        return "horizontal"
    return "negative" if slope < 0.0 else "positive"


def sample_support(magnitude: np.ndarray, segment: Segment) -> float:
    count = max(8, int(round(segment_length(segment) * 1.4)))
    xs = np.linspace(segment.x1, segment.x2, count)
    ys = np.linspace(segment.y1, segment.y2, count)
    xi = np.clip(np.round(xs).astype(np.int32), 0, magnitude.shape[1] - 1)
    yi = np.clip(np.round(ys).astype(np.int32), 0, magnitude.shape[0] - 1)
    return float(magnitude[yi, xi].mean())


def merge_cluster(items: list[tuple[Segment, float]], slope_kind: str, center_x: float) -> LineCluster:
    weights = np.asarray([max(1.0, segment_length(segment) * support) for segment, support in items], dtype=np.float32)
    points = np.asarray(
        [(segment.x1, segment.y1) for segment, _ in items] + [(segment.x2, segment.y2) for segment, _ in items],
        dtype=np.float32,
    )
    point_weights = np.repeat(weights, 2)
    x1, x2 = float(points[:, 0].min()), float(points[:, 0].max())
    if slope_kind == "horizontal":
        y = float(np.average(points[:, 1], weights=point_weights))
        merged = Segment(int(round(x1)), int(round(y)), int(round(x2)), int(round(y)))
    else:
        slope, intercept = np.polyfit(points[:, 0], points[:, 1], deg=1, w=point_weights)
        merged = Segment(
            int(round(x1)),
            int(round(slope * x1 + intercept)),
            int(round(x2)),
            int(round(slope * x2 + intercept)),
        )
    return LineCluster(
        slope_kind=slope_kind,
        y_at_center=line_y_at_center(merged, center_x),
        segment=merged,
        support=float(np.average([support for _, support in items], weights=weights)),
    )


def hough_clusters(magnitude: np.ndarray, edges: np.ndarray) -> dict[str, list[LineCluster]]:
    lines = cv2.HoughLinesP(
        (edges.astype(np.uint8) * 255),
        rho=1,
        theta=np.pi / 180.0,
        threshold=HOUGH_THRESHOLD,
        minLineLength=HOUGH_MIN_LENGTH,
        maxLineGap=HOUGH_MAX_GAP,
    )
    groups: dict[str, list[tuple[Segment, float]]] = {"horizontal": [], "negative": [], "positive": []}
    if lines is None:
        return {key: [] for key in groups}
    for raw in lines[:, 0, :]:
        segment = normalize_segment(raw[:2].astype(np.float32), raw[2:].astype(np.float32))
        kind = classify_line(segment)
        if kind is None or segment_length(segment) < HOUGH_MIN_LENGTH:
            continue
        groups[kind].append((segment, sample_support(magnitude, segment)))

    center_x = (magnitude.shape[1] - 1) * 0.5
    merged: dict[str, list[LineCluster]] = {}
    for kind, items in groups.items():
        items.sort(key=lambda item: line_y_at_center(item[0], center_x))
        buckets: list[list[tuple[Segment, float]]] = []
        for item in items:
            line_y = line_y_at_center(item[0], center_x)
            if not buckets:
                buckets.append([item])
                continue
            bucket_y = np.mean([line_y_at_center(previous[0], center_x) for previous in buckets[-1]])
            if abs(line_y - bucket_y) <= CLUSTER_GAP_PX:
                buckets[-1].append(item)
            else:
                buckets.append([item])
        merged[kind] = [merge_cluster(bucket, kind, center_x) for bucket in buckets]
    return merged


def select_role_lines(clusters: dict[str, list[LineCluster]]) -> tuple[list[Segment], list[Segment]]:
    """Assign lines by depth, without assuming a fronto-parallel square.

    For both oblique side families, a smaller y at the image centre is the
    outer edge and the next separated line is the corresponding inner edge.
    Horizontal lines are ordered outer-top, inner-top, inner-bottom,
    outer-bottom.  This remains valid for the central oblique V-shaped view.
    """

    outer: list[Segment] = []
    inner: list[Segment] = []
    for kind in ("negative", "positive"):
        family = clusters[kind]
        # A bright tape produces two closely parallel gradient ridges.  Their
        # midpoint is the physical tape centreline, which is what peoplev3
        # annotates.  The following more distant ridge is the next frame.
        if len(family) >= 3 and family[1].y_at_center - family[0].y_at_center <= 5.0:
            outer.append(midpoint_segment(family[0].segment, family[1].segment))
            inner.append(family[2].segment)
        else:
            if family:
                outer.append(family[0].segment)
            if len(family) >= 2:
                inner.append(family[1].segment)

    horizontal = clusters["horizontal"]
    if horizontal:
        outer.append(horizontal[0].segment)
    if len(horizontal) >= 2:
        if horizontal[1].y_at_center - horizontal[0].y_at_center <= 6.0:
            inner.append(midpoint_segment(horizontal[0].segment, horizontal[1].segment))
        else:
            inner.append(horizontal[1].segment)
    # The lowest Hough ridge can be a floor/background boundary.  When five
    # or more horizontal families exist, retain the preceding two frame lines.
    bottom_offset = 3 if len(horizontal) >= 5 else 2
    if len(horizontal) >= bottom_offset + 1:
        inner.append(horizontal[-bottom_offset].segment)
        outer.append(horizontal[-(bottom_offset - 1)].segment)
    return dedupe_segments(outer), dedupe_segments(inner)


def midpoint_segment(first: Segment, second: Segment) -> Segment:
    """Average matched gradient ridges into one physical paint/tape line."""

    return Segment(
        x1=int(round((first.x1 + second.x1) * 0.5)),
        y1=int(round((first.y1 + second.y1) * 0.5)),
        x2=int(round((first.x2 + second.x2) * 0.5)),
        y2=int(round((first.y2 + second.y2) * 0.5)),
    )


def dedupe_segments(segments: Iterable[Segment]) -> list[Segment]:
    unique: list[Segment] = []
    for segment in segments:
        if segment_length(segment) < HOUGH_MIN_LENGTH:
            continue
        if any(
            abs(segment.x1 - item.x1) <= 2
            and abs(segment.y1 - item.y1) <= 2
            and abs(segment.x2 - item.x2) <= 2
            and abs(segment.y2 - item.y2) <= 2
            for item in unique
        ):
            continue
        unique.append(segment)
    return unique


def bbox_from_segments(segments: list[Segment], shape: tuple[int, int]) -> tuple[int, int, int, int]:
    if not segments:
        return 0, 0, shape[1], shape[0]
    xs = [point for segment in segments for point in (segment.x1, segment.x2)]
    ys = [point for segment in segments for point in (segment.y1, segment.y2)]
    x1, x2 = max(0, min(xs)), min(shape[1] - 1, max(xs))
    y1, y2 = max(0, min(ys)), min(shape[0] - 1, max(ys))
    return x1, y1, max(1, x2 - x1 + 1), max(1, y2 - y1 + 1)


def estimate_far_view(bbox: tuple[int, int, int, int], shape: tuple[int, int]) -> float:
    _, y, width, height = bbox
    image_h, image_w = shape
    small = max(0.0, min(1.0, 1.0 - (width * height) / max(1.0, image_w * image_h * 0.14)))
    high = max(0.0, min(1.0, (image_h * 0.28 - y) / max(1.0, image_h * 0.28)))
    return 0.65 * small + 0.35 * high


def detect_best_candidate(
    gray: np.ndarray,
    fixed_threshold: int | None = None,
    fixed_thresholds: list[int] | tuple[int, ...] | None = None,
) -> EdgeCandidate | None:
    """Detect line families; threshold arguments are ignored for edge-only mode."""

    magnitude, edges, response_threshold = gradient_edges(gray)
    clusters = hough_clusters(magnitude, edges)
    outer_segments, inner_segments = select_role_lines(clusters)
    all_segments = outer_segments + inner_segments
    if len(all_segments) < 2:
        return None

    bbox = bbox_from_segments(all_segments, gray.shape)
    x, y, width, height = bbox
    line_count_score = min(1.0, len(all_segments) / 5.0)
    orientation_score = sum(bool(clusters[key]) for key in ("negative", "positive", "horizontal")) / 3.0
    support_values = [cluster.support for family in clusters.values() for cluster in family]
    response_score = min(1.0, float(np.mean(support_values)) / max(1.0, response_threshold * 1.8)) if support_values else 0.0
    score = 0.42 * line_count_score + 0.36 * orientation_score + 0.22 * response_score
    far_view = estimate_far_view(bbox, gray.shape)
    area = float(width * height)
    touches_border = x == 0 or y == 0 or x + width >= gray.shape[1] or y + height >= gray.shape[0]
    return EdgeCandidate(
        threshold=response_threshold,
        score=score,
        far_view_factor=far_view,
        outer_area=area,
        outer_bbox=bbox,
        child_area=float(len(inner_segments)),
        touches_border=touches_border,
        outer_quad=None,
        inner_quad_direct=None,
        inner_quad_final=None,
        threshold_mask=edges,
        geometry_score=orientation_score,
        ring_mean=0.0,
        inner_mean=0.0,
        ring_on_ratio=0.0,
        inner_on_ratio=0.0,
        outer_support=response_score,
        inner_support=response_score if inner_segments else 0.0,
        outer_segments=outer_segments,
        inner_segments=inner_segments,
    )


def should_suppress_inner_segments(candidate: EdgeCandidate | None) -> bool:
    return candidate is None or candidate.far_view_factor >= 0.52


def extract_prediction_segments(
    gray: np.ndarray,
    candidate: EdgeCandidate | None,
) -> tuple[np.ndarray, list[Segment], list[Segment]]:
    if candidate is None:
        return np.zeros_like(gray, dtype=bool), [], []
    inner = [] if should_suppress_inner_segments(candidate) else candidate.inner_segments
    return candidate.threshold_mask, candidate.outer_segments, inner


def make_threshold_debug(
    edge_mask: np.ndarray,
    outer_segments: list[Segment],
    inner_segments: list[Segment],
) -> np.ndarray:
    canvas = np.zeros((*edge_mask.shape, 3), dtype=np.uint8)
    canvas[edge_mask] = (255, 255, 255)
    for segment in outer_segments:
        cv2.line(canvas, (segment.x1, segment.y1), (segment.x2, segment.y2), (255, 0, 0), 1)
    for segment in inner_segments:
        cv2.line(canvas, (segment.x1, segment.y1), (segment.x2, segment.y2), (0, 255, 0), 1)
    return canvas
