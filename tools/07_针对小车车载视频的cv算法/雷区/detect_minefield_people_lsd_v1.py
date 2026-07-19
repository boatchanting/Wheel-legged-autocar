"""LSD-based geometric recognition on top of the v30 separable gradients."""

from __future__ import annotations

import cv2
import numpy as np

import detect_minefield_people_separable_edge_v1 as base
from detect_minefield_people_separable_edge_v1 import *  # noqa: F401,F403


LSD_MIN_LENGTH = 7.0
LSD_CLUSTER_GAP_PX = 2.5


def lsd_clusters(
    gx: np.ndarray,
    gy: np.ndarray,
    magnitude: np.ndarray,
) -> dict[str, list[base.LineCluster]]:
    """Extract sub-pixel segments from the gradient response, not the image."""

    response = cv2.normalize(magnitude, None, 0, 255, cv2.NORM_MINMAX).astype(np.uint8)
    detector = cv2.createLineSegmentDetector(cv2.LSD_REFINE_ADV, scale=0.8, sigma_scale=0.6)
    detected = detector.detect(response)[0]
    groups: dict[str, list[tuple[base.Segment, float, float, float]]] = {
        "horizontal": [],
        "negative": [],
        "positive": [],
    }
    if detected is None:
        return {key: [] for key in groups}

    for raw in detected[:, 0, :]:
        segment = base.normalize_segment(raw[:2], raw[2:])
        kind = base.classify_line(segment)
        if kind is None or base.segment_length(segment) < LSD_MIN_LENGTH:
            continue
        support, alignment, polarity = base.sample_gradient(gx, gy, magnitude, segment)
        if alignment < 0.55:
            continue
        groups[kind].append((segment, support, alignment, polarity))

    center_x = (magnitude.shape[1] - 1) * 0.5
    merged: dict[str, list[base.LineCluster]] = {}
    for kind, items in groups.items():
        items.sort(key=lambda item: base.line_y_at_center(item[0], center_x))
        buckets: list[list[tuple[base.Segment, float, float, float]]] = []
        for item in items:
            line_y = base.line_y_at_center(item[0], center_x)
            if not buckets:
                buckets.append([item])
                continue
            bucket_y = np.mean([base.line_y_at_center(previous[0], center_x) for previous in buckets[-1]])
            if abs(line_y - bucket_y) <= LSD_CLUSTER_GAP_PX:
                buckets[-1].append(item)
            else:
                buckets.append([item])
        merged[kind] = [base.merge_cluster(bucket, kind, center_x) for bucket in buckets]
    return merged


def detect_best_candidate(
    gray: np.ndarray,
    fixed_threshold: int | None = None,
    fixed_thresholds: list[int] | tuple[int, ...] | None = None,
) -> base.EdgeCandidate | None:
    """Keep v30 preprocessing but replace probabilistic Hough with LSD."""

    gx, gy, magnitude, edges, response_threshold = base.gradient_edges(gray)
    clusters = lsd_clusters(gx, gy, magnitude)
    outer_segments, inner_segments = base.select_role_lines(clusters)
    all_segments = outer_segments + inner_segments
    if len(all_segments) < 2:
        return None

    bbox = base.bbox_from_segments(all_segments, gray.shape)
    x, y, width, height = bbox
    line_count_score = min(1.0, len(all_segments) / 5.0)
    orientation_score = sum(bool(clusters[key]) for key in ("negative", "positive", "horizontal")) / 3.0
    support_values = [cluster.support for family in clusters.values() for cluster in family]
    alignment_values = [cluster.normal_alignment for family in clusters.values() for cluster in family]
    response_score = min(1.0, float(np.mean(support_values)) / max(1.0, response_threshold * 1.8)) if support_values else 0.0
    alignment_score = float(np.mean(alignment_values)) if alignment_values else 0.0
    score = 0.38 * line_count_score + 0.30 * orientation_score + 0.20 * response_score + 0.12 * alignment_score
    far_view = base.estimate_far_view(bbox, gray.shape)
    area = float(width * height)
    touches_border = x == 0 or y == 0 or x + width >= gray.shape[1] or y + height >= gray.shape[0]
    return base.EdgeCandidate(
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
