"""Scoring helpers for minefield candidate ranking.

This module is intentionally separate from the detector so we can iterate on
ranking logic without mixing it into contour extraction and geometry fitting.
"""

from __future__ import annotations

from dataclasses import dataclass

import cv2
import numpy as np


def clamp01(value: float) -> float:
    return max(0.0, min(1.0, value))


@dataclass(frozen=True)
class GeometryMeasurements:
    geometry_score: float
    ring_mean: float
    inner_mean: float
    ring_on_ratio: float
    inner_on_ratio: float
    outer_support: float
    inner_support: float


@dataclass(frozen=True)
class ScoreBreakdown:
    total_score: float
    far_view_factor: float
    decision_threshold: float
    area_score: float
    width_score: float
    height_score: float
    child_score: float
    size_score: float
    geometry_score: float


def line_support(source_mask: np.ndarray, line_mask: np.ndarray, radius: int = 1) -> float:
    if not line_mask.any():
        return 0.0
    kernel = np.ones((radius * 2 + 1, radius * 2 + 1), dtype=np.uint8)
    dilated = cv2.dilate(source_mask.astype(np.uint8), kernel, iterations=1).astype(bool)
    return float((line_mask & dilated).sum() / max(1, line_mask.sum()))


def measure_geometry(
    gray: np.ndarray,
    threshold_mask: np.ndarray,
    outer_quad: np.ndarray,
    inner_quad: np.ndarray | None,
    render_quad_mask,
    render_polygon_mask,
) -> GeometryMeasurements:
    if inner_quad is None:
        return GeometryMeasurements(
            geometry_score=0.0,
            ring_mean=0.0,
            inner_mean=0.0,
            ring_on_ratio=0.0,
            inner_on_ratio=0.0,
            outer_support=line_support(threshold_mask, render_quad_mask(gray.shape, outer_quad)),
            inner_support=0.0,
        )

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

    return GeometryMeasurements(
        geometry_score=geometry_score,
        ring_mean=ring_mean,
        inner_mean=inner_mean,
        ring_on_ratio=ring_on_ratio,
        inner_on_ratio=inner_on_ratio,
        outer_support=outer_support,
        inner_support=inner_support,
    )


def estimate_far_view_factor(
    bbox: tuple[int, int, int, int],
    outer_area: float,
    image_height: int,
) -> float:
    _, y, _, h = bbox
    topness = clamp01((12.0 - float(y)) / 12.0)
    smallness = clamp01((70.0 - outer_area) / 70.0)
    thinness = clamp01((6.0 - float(h)) / 6.0)
    vertical_position = clamp01((0.35 * image_height - (y + h)) / max(1.0, 0.35 * image_height))
    return clamp01(0.35 * topness + 0.30 * smallness + 0.20 * thinness + 0.15 * vertical_position)


def score_candidate(
    bbox: tuple[int, int, int, int],
    outer_area: float,
    child_area: float,
    touches_border: bool,
    geometry: GeometryMeasurements,
    image_height: int,
) -> ScoreBreakdown:
    x, y, w, h = bbox
    far_view_factor = estimate_far_view_factor(bbox, outer_area, image_height)

    area_ref = (1.0 - far_view_factor) * 380.0 + far_view_factor * 85.0
    width_ref = (1.0 - far_view_factor) * 58.0 + far_view_factor * 24.0
    height_ref = (1.0 - far_view_factor) * 21.0 + far_view_factor * 5.0
    child_ref = (1.0 - far_view_factor) * 220.0 + far_view_factor * 50.0

    area_score = min(outer_area / area_ref, 1.0)
    width_score = min(w / width_ref, 1.0)
    height_score = min(h / height_ref, 1.0)
    child_score = min(child_area / child_ref, 1.0)

    size_score = (
        0.42 * area_score
        + 0.20 * width_score
        + 0.14 * height_score
        + 0.24 * child_score
    )

    total_score = 0.52 * size_score + 0.48 * geometry.geometry_score
    total_score += 0.06 * far_view_factor

    if outer_area < ((1.0 - far_view_factor) * 90.0 + far_view_factor * 18.0) and child_area < 20.0:
        total_score -= (1.0 - far_view_factor) * 0.12
    if y < 7 and h < 8 and geometry.geometry_score < 0.38:
        total_score -= (1.0 - far_view_factor) * 0.10
    if geometry.ring_on_ratio < 0.12 and geometry.inner_on_ratio < 0.12:
        total_score -= (1.0 - far_view_factor) * 0.08
    if touches_border and outer_area < ((1.0 - far_view_factor) * 75.0 + far_view_factor * 16.0):
        total_score -= (1.0 - far_view_factor) * 0.05
    if x == 0 and w < 16 and outer_area < ((1.0 - far_view_factor) * 80.0 + far_view_factor * 14.0):
        total_score -= (1.0 - far_view_factor) * 0.06

    decision_threshold = 0.34 - 0.14 * far_view_factor
    return ScoreBreakdown(
        total_score=total_score,
        far_view_factor=far_view_factor,
        decision_threshold=decision_threshold,
        area_score=area_score,
        width_score=width_score,
        height_score=height_score,
        child_score=child_score,
        size_score=size_score,
        geometry_score=geometry.geometry_score,
    )
