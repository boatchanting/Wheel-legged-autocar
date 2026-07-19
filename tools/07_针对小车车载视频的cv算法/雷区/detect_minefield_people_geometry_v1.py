"""Globally paired perspective-line recognition on the v30 gradient map."""

from __future__ import annotations

import numpy as np

import detect_minefield_people_separable_edge_v1 as base
from detect_minefield_people_separable_edge_v1 import *  # noqa: F401,F403


def line_intersection(first: base.Segment, second: base.Segment) -> np.ndarray | None:
    p = np.array([first.x1, first.y1], dtype=np.float32)
    r = np.array([first.x2 - first.x1, first.y2 - first.y1], dtype=np.float32)
    q = np.array([second.x1, second.y1], dtype=np.float32)
    s = np.array([second.x2 - second.x1, second.y2 - second.y1], dtype=np.float32)
    cross = float(r[0] * s[1] - r[1] * s[0])
    if abs(cross) < 1e-4:
        return None
    delta = q - p
    t = float((delta[0] * s[1] - delta[1] * s[0]) / cross)
    return p + t * r


def chevron_pairs(
    negative: list[base.LineCluster],
    positive: list[base.LineCluster],
    shape: tuple[int, int],
) -> list[tuple[float, float, np.ndarray, base.LineCluster, base.LineCluster]]:
    """Form perspective side-pairs before assigning frame roles."""

    height, width = shape
    pairs: list[tuple[float, float, np.ndarray, base.LineCluster, base.LineCluster]] = []
    for left in negative:
        for right in positive:
            apex = line_intersection(left.segment, right.segment)
            if apex is None:
                continue
            if not (-0.25 * width <= apex[0] <= 1.25 * width and -0.45 * height <= apex[1] <= 0.85 * height):
                continue
            level = (left.y_at_center + right.y_at_center) * 0.5
            span_score = min(1.0, (base.segment_length(left.segment) + base.segment_length(right.segment)) / width)
            support_score = min(1.0, (left.support + right.support) / 80.0)
            apex_score = 1.0 - min(1.0, abs(float(apex[0]) - (width - 1) * 0.5) / (width * 0.65))
            score = 0.48 * span_score + 0.34 * support_score + 0.18 * apex_score
            pairs.append((level, score, apex, left, right))
    return pairs


def select_geometric_role_lines(
    clusters: dict[str, list[base.LineCluster]],
    shape: tuple[int, int],
) -> tuple[list[base.Segment], list[base.Segment]]:
    """Choose nested perspective side-pairs, with v30 as the safe fallback."""

    fallback_outer, fallback_inner = base.select_role_lines(clusters)
    pairs = chevron_pairs(clusters["negative"], clusters["positive"], shape)
    horizontal_only = select_horizontal_only_roles(clusters, shape)
    if horizontal_only is not None:
        return horizontal_only
    side_view = select_single_direction_side_roles(clusters, shape)
    if side_view is not None:
        return side_view
    low_band = select_low_band_roles(clusters, pairs, shape, fallback_outer, fallback_inner)
    if low_band is not None:
        return low_band
    clipped_bottom = select_clipped_outer_bottom_roles(clusters, shape, fallback_outer, fallback_inner)
    if clipped_bottom is not None:
        return clipped_bottom
    if len(pairs) < 2:
        return fallback_outer, fallback_inner

    pairs.sort(key=lambda item: item[0])
    height, width = shape
    outer_candidates = pairs[: min(4, len(pairs))]
    outer = max(outer_candidates, key=lambda item: item[1])
    inner_candidates = [
        item
        for item in pairs
        if item[0] >= outer[0] + 2.0
        and item[0] <= outer[0] + max(18.0, height * 0.35)
        and item[3] is not outer[3]
        and item[4] is not outer[4]
        and float(np.linalg.norm(item[2] - outer[2])) <= max(16.0, width * 0.25)
    ]
    if not inner_candidates:
        return fallback_outer, fallback_inner
    inner = max(inner_candidates, key=lambda item: item[1] - 0.012 * (item[0] - outer[0]))

    # Apply the paired geometry only to the oblique/no-horizontal regime.  A
    # rich horizontal family is handled better by the v30 trapezoid logic.
    if len(clusters["horizontal"]) >= 2:
        return fallback_outer, fallback_inner

    outer_segments = base.dedupe_segments([outer[3].segment, outer[4].segment])
    inner_segments = base.dedupe_segments([inner[3].segment, inner[4].segment])
    if len(outer_segments) < 2 or len(inner_segments) < 2:
        return fallback_outer, fallback_inner
    return outer_segments, inner_segments


def select_low_band_roles(
    clusters: dict[str, list[base.LineCluster]],
    pairs: list[tuple[float, float, np.ndarray, base.LineCluster, base.LineCluster]],
    shape: tuple[int, int],
    fallback_outer: list[base.Segment],
    fallback_inner: list[base.Segment],
) -> tuple[list[base.Segment], list[base.Segment]] | None:
    """Handle near views where only the two lower frame bands are visible."""

    horizontal = clusters["horizontal"]
    if len(horizontal) != 2 or not pairs:
        return None
    height, width = shape
    upper_band, lower_band = horizontal
    if (
        base.segment_length(upper_band.segment) < width * 0.62
        or base.segment_length(lower_band.segment) < width * 0.62
    ):
        return None
    apex = max(pairs, key=lambda item: item[1])[2]
    if upper_band.y_at_center <= float(apex[1]) + max(14.0, height * 0.22):
        return None

    outer_sides = [segment for segment in fallback_outer if base.classify_line(segment) != "horizontal"]
    inner_sides = [segment for segment in fallback_inner if base.classify_line(segment) != "horizontal"]
    # The nearer/lower band is the outer bottom edge; the upper band is the
    # inner bottom edge.  The standard ordering assumes visible top bands.
    outer = base.dedupe_segments(outer_sides + [lower_band.segment])
    inner = base.dedupe_segments(inner_sides + [upper_band.segment])
    return outer, inner


def select_single_direction_side_roles(
    clusters: dict[str, list[base.LineCluster]],
    shape: tuple[int, int],
) -> tuple[list[base.Segment], list[base.Segment]] | None:
    """Recognize the truncated parallel bands in a near side-view image."""

    if clusters["horizontal"] or clusters["negative"] or len(clusters["positive"]) < 3:
        return None
    height, width = shape
    family = clusters["positive"]
    max_support = max(line.support for line in family)
    strong = [line for line in family if line.support >= max_support * 0.55]
    if len(strong) < 2:
        return None

    outer_first = strong[0]
    outer_segment = outer_first.segment
    if len(strong) >= 2:
        outer_second = strong[1]
        first_length = base.segment_length(outer_first.segment)
        second_length = base.segment_length(outer_second.segment)
        lengths_match = min(first_length, second_length) / max(first_length, second_length) >= 0.75
        close_bands = outer_second.y_at_center - outer_first.y_at_center <= 4.0
        if lengths_match and close_bands:
            outer_segment = base.midpoint_segment(outer_first.segment, outer_second.segment)

    outer_y0 = min(outer_segment.y1, outer_segment.y2)
    inner_candidates = [
        line
        for line in strong[1:]
        if max(line.segment.x1, line.segment.x2) <= width * 0.36
        and min(line.segment.y1, line.segment.y2) >= outer_y0 + max(8.0, height * 0.16)
    ]
    inner = [max(inner_candidates, key=lambda line: line.support).segment] if inner_candidates else []
    return [outer_segment], inner


def select_horizontal_only_roles(
    clusters: dict[str, list[base.LineCluster]],
    shape: tuple[int, int],
) -> tuple[list[base.Segment], list[base.Segment]] | None:
    """Assign the two visible parallel bands by depth in a frontal low view."""

    if clusters["negative"] or clusters["positive"]:
        return None
    _, width = shape
    long_lines = [line for line in clusters["horizontal"] if base.segment_length(line.segment) >= width * 0.72]
    if len(long_lines) != 2:
        return None
    return [long_lines[0].segment], [long_lines[1].segment]


def select_clipped_outer_bottom_roles(
    clusters: dict[str, list[base.LineCluster]],
    shape: tuple[int, int],
    fallback_outer: list[base.Segment],
    fallback_inner: list[base.Segment],
) -> tuple[list[base.Segment], list[base.Segment]] | None:
    """Recover the outer lower edge when it is clipped below the image."""

    height, width = shape
    horizontal = clusters["horizontal"]
    long_lines = [line for line in horizontal if base.segment_length(line.segment) >= width * 0.54]
    if len(long_lines) != 2:
        return None
    outer_top, inner_bottom = long_lines
    if inner_bottom.y_at_center >= height * 0.78 or inner_bottom.y_at_center <= outer_top.y_at_center + height * 0.28:
        return None

    outer_sides = [segment for segment in fallback_outer if base.classify_line(segment) != "horizontal"]
    if len(outer_sides) < 2:
        return None
    inner_non_horizontal = [segment for segment in fallback_inner if base.classify_line(segment) != "horizontal"]
    inner_top = [
        segment
        for segment in fallback_inner
        if base.classify_line(segment) == "horizontal" and (segment.y1 + segment.y2) * 0.5 < inner_bottom.y_at_center - 6.0
    ]
    outer_bottom = base.Segment(x1=0, y1=height - 1, x2=width - 1, y2=height - 1)
    outer = base.dedupe_segments(outer_sides + [outer_top.segment, outer_bottom])
    inner = base.dedupe_segments(inner_non_horizontal + inner_top + [inner_bottom.segment])
    return outer, inner


def detect_best_candidate(
    gray: np.ndarray,
    fixed_threshold: int | None = None,
    fixed_thresholds: list[int] | tuple[int, ...] | None = None,
) -> base.EdgeCandidate | None:
    gx, gy, magnitude, edges, response_threshold = base.gradient_edges(gray)
    clusters = base.hough_clusters(gx, gy, magnitude, edges)
    outer_segments, inner_segments = select_geometric_role_lines(clusters, gray.shape)
    all_segments = outer_segments + inner_segments
    if not all_segments:
        return None
    if len(all_segments) == 1 and base.segment_length(all_segments[0]) < gray.shape[1] * 0.25:
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
