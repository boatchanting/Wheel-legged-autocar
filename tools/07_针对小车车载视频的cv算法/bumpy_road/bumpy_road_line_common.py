from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import numpy as np
from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_FRAME_DIR = PROJECT_ROOT / "data" / "frames" / "\u98a0\u7c38\u8def\u6bb5"
DEFAULT_OUTPUT_ROOT = PROJECT_ROOT / "data" / "\u98a0\u7c38\u8def\u6bb5"
DEFAULT_VIDEO_PATH = PROJECT_ROOT / "data" / "\u98a0\u7c38\u8def\u6bb5.avi"

ROI_X0 = 0
ROI_X1 = 93
ROI_Y0 = 6
ROI_Y1 = 59

MIN_COMPONENT_AREA = 150
MIN_COMPONENT_WIDTH = 10
MIN_COMPONENT_HEIGHT = 4

MIN_ROW_RUN_WIDTH = 12
MAX_ROW_GAP = 2
BOTTOM_TARGET_ROWS = 14

MIN_RIB_ROW_PIXELS = 18
MIN_RIB_WIDTH = 24
MIN_RIB_HEIGHT = 2

IMAGE_CENTER_X = 46.5


@dataclass
class Component:
    area: int
    xmin: int
    ymin: int
    xmax: int
    ymax: int
    centroid_x: float
    centroid_y: float
    fill_ratio: float
    touches_border: bool
    mean_gray: float
    score: float = 0.0

    @property
    def width(self) -> int:
        return self.xmax - self.xmin + 1

    @property
    def height(self) -> int:
        return self.ymax - self.ymin + 1


@dataclass(frozen=True)
class WhiteRun:
    y: int
    xmin: int
    xmax: int
    threshold: int

    @property
    def width(self) -> int:
        return self.xmax - self.xmin + 1

    @property
    def center_x(self) -> float:
        return 0.5 * (self.xmin + self.xmax)


@dataclass(frozen=True)
class RibBand:
    ymin: int
    ymax: int
    xmin: int
    xmax: int
    area: int
    max_row_pixels: int
    mean_gray: float

    @property
    def width(self) -> int:
        return self.xmax - self.xmin + 1

    @property
    def height(self) -> int:
        return self.ymax - self.ymin + 1

    @property
    def center_y(self) -> float:
        return 0.5 * (self.ymin + self.ymax)


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def gray_from_rgb(rgb: np.ndarray) -> np.ndarray:
    return np.asarray(Image.fromarray(rgb).convert("L"))


def find_components(mask: np.ndarray, gray: np.ndarray) -> list[Component]:
    h, w = mask.shape
    visited = np.zeros_like(mask, dtype=bool)
    components: list[Component] = []

    for y0 in range(h):
        for x0 in range(w):
            if visited[y0, x0] or not mask[y0, x0]:
                continue

            stack = [(y0, x0)]
            visited[y0, x0] = True
            xs: list[int] = []
            ys: list[int] = []

            while stack:
                y, x = stack.pop()
                xs.append(x)
                ys.append(y)
                for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    ny = y + dy
                    nx = x + dx
                    if (
                        0 <= ny < h
                        and 0 <= nx < w
                        and mask[ny, nx]
                        and not visited[ny, nx]
                    ):
                        visited[ny, nx] = True
                        stack.append((ny, nx))

            xs_arr = np.asarray(xs)
            ys_arr = np.asarray(ys)
            xmin = int(xs_arr.min())
            xmax = int(xs_arr.max())
            ymin = int(ys_arr.min())
            ymax = int(ys_arr.max())
            area = int(xs_arr.size)
            bbox_area = max(1, (xmax - xmin + 1) * (ymax - ymin + 1))
            touches_border = xmin == 0 or ymin == 0 or xmax == (w - 1) or ymax == (h - 1)
            mean_gray = float(gray[ys_arr, xs_arr].mean())

            components.append(
                Component(
                    area=area,
                    xmin=xmin,
                    ymin=ymin,
                    xmax=xmax,
                    ymax=ymax,
                    centroid_x=float(xs_arr.mean()),
                    centroid_y=float(ys_arr.mean()),
                    fill_ratio=float(area / bbox_area),
                    touches_border=touches_border,
                    mean_gray=mean_gray,
                )
            )

    return sorted(components, key=lambda c: c.area, reverse=True)


def score_white_component(component: Component) -> float:
    area_score = min(component.area / 2600.0, 1.0)
    width_score = min(component.width / 82.0, 1.0)
    height_score = min(component.height / 34.0, 1.0)
    fill_score = min(component.fill_ratio / 0.82, 1.0)
    brightness_score = min(max((component.mean_gray - 210.0) / 45.0, 0.0), 1.0)
    border_score = 1.0 if component.touches_border else 0.0

    return (
        0.30 * area_score
        + 0.18 * width_score
        + 0.15 * height_score
        + 0.14 * fill_score
        + 0.13 * brightness_score
        + 0.10 * border_score
    )


def estimate_white_threshold(
    gray: np.ndarray,
    prev_threshold: float | None = None,
    smooth_alpha: float = 0.72,
) -> tuple[float, float]:
    roi = gray[ROI_Y0 : ROI_Y1 + 1, ROI_X0 : ROI_X1 + 1].astype(np.float32)
    pixels = roi.reshape(-1)
    mean = float(pixels.mean())
    std = float(pixels.std())
    p75 = float(np.percentile(pixels, 75.0))
    p85 = float(np.percentile(pixels, 85.0))
    p95 = float(np.percentile(pixels, 95.0))
    p99 = float(np.percentile(pixels, 99.0))

    tail_candidate = p75 + 0.32 * max(0.0, p99 - p75)
    shoulder_candidate = p85 - 6.0
    sigma_candidate = mean + 0.58 * std
    highlight_candidate = p95 - 10.0

    candidate = clamp(
        max(tail_candidate, shoulder_candidate, sigma_candidate, highlight_candidate),
        200.0,
        248.0,
    )
    if prev_threshold is None:
        return candidate, candidate

    smoothed = smooth_alpha * prev_threshold + (1.0 - smooth_alpha) * candidate
    return clamp(smoothed, 200.0, 248.0), candidate


def estimate_row_white_threshold(row: np.ndarray, global_threshold: float) -> int:
    rowf = row.astype(np.float32)
    p60 = float(np.percentile(rowf, 60.0))
    p75 = float(np.percentile(rowf, 75.0))
    p90 = float(np.percentile(rowf, 90.0))
    candidate = max(p75, p60 + 0.35 * max(0.0, p90 - p60), global_threshold - 20.0)
    return int(round(clamp(candidate, 185.0, min(250.0, global_threshold + 6.0))))


def close_small_gaps(mask: np.ndarray, max_gap: int = MAX_ROW_GAP) -> np.ndarray:
    out = mask.copy()
    x = 0
    length = out.size
    while x < length:
        if out[x]:
            x += 1
            continue
        start = x
        while x < length and not out[x]:
            x += 1
        end = x - 1
        gap = end - start + 1
        left_on = start > 0 and out[start - 1]
        right_on = x < length and out[x]
        if left_on and right_on and gap <= max_gap:
            out[start : end + 1] = True
    return out


def extract_row_runs(row_mask: np.ndarray, y: int, threshold: int) -> list[WhiteRun]:
    runs: list[WhiteRun] = []
    x = 0
    while x < row_mask.size:
        if not row_mask[x]:
            x += 1
            continue
        start = x
        while x < row_mask.size and row_mask[x]:
            x += 1
        end = x - 1
        if end - start + 1 >= MIN_ROW_RUN_WIDTH:
            runs.append(WhiteRun(y=y, xmin=ROI_X0 + start, xmax=ROI_X0 + end, threshold=threshold))
    return runs


def choose_best_run(runs: list[WhiteRun], anchor_x: float) -> WhiteRun | None:
    if not runs:
        return None

    best_run = None
    best_score = None
    for run in runs:
        width_bonus = run.width
        center_penalty = 1.35 * abs(run.center_x - anchor_x)
        edge_bonus = 4.0 if run.xmin <= ROI_X0 + 1 or run.xmax >= ROI_X1 - 1 else 0.0
        score = width_bonus + edge_bonus - center_penalty
        if best_score is None or score > best_score:
            best_score = score
            best_run = run
    return best_run


def build_white_scan_mask(gray: np.ndarray, global_threshold: float) -> tuple[np.ndarray, list[WhiteRun]]:
    mask = np.zeros_like(gray, dtype=bool)
    runs: list[WhiteRun] = []
    anchor_x = IMAGE_CENTER_X

    for y in range(ROI_Y1, ROI_Y0 - 1, -1):
        row = gray[y, ROI_X0 : ROI_X1 + 1]
        row_threshold = estimate_row_white_threshold(row, global_threshold)
        row_mask = close_small_gaps(row >= row_threshold)
        row_runs = extract_row_runs(row_mask, y, row_threshold)
        best_run = choose_best_run(row_runs, anchor_x)
        if best_run is None:
            continue

        mask[y, best_run.xmin : best_run.xmax + 1] = True
        runs.append(best_run)
        anchor_x = 0.72 * anchor_x + 0.28 * best_run.center_x

    runs.sort(key=lambda item: item.y)
    return mask, runs


def estimate_dark_threshold(gray: np.ndarray) -> float:
    roi = gray[ROI_Y0 : ROI_Y1 + 1, ROI_X0 : ROI_X1 + 1].astype(np.float32).reshape(-1)
    p10 = float(np.percentile(roi, 10.0))
    p20 = float(np.percentile(roi, 20.0))
    p25 = float(np.percentile(roi, 25.0))
    candidate = min(p25 - 6.0, p10 + 18.0, p20 + 10.0)
    return clamp(candidate, 95.0, 185.0)


def build_supported_dark_mask(gray: np.ndarray, white_mask: np.ndarray, dark_threshold: float) -> np.ndarray:
    dark_mask = gray <= int(round(dark_threshold))
    support = np.zeros_like(dark_mask, dtype=bool)
    for offset in range(2, 7):
        above = np.zeros_like(white_mask, dtype=bool)
        below = np.zeros_like(white_mask, dtype=bool)
        above[offset:, :] = white_mask[:-offset, :]
        below[:-offset, :] = white_mask[offset:, :]
        support |= above & below
    return dark_mask & support


def group_rows(rows: np.ndarray) -> list[tuple[int, int]]:
    if rows.size == 0:
        return []

    groups: list[tuple[int, int]] = []
    start = prev = int(rows[0])
    for raw_y in rows[1:]:
        y = int(raw_y)
        if y <= prev + 1:
            prev = y
        else:
            groups.append((start, prev))
            start = prev = y
    groups.append((start, prev))
    return groups


def find_rib_bands(gray: np.ndarray, rib_mask: np.ndarray) -> list[RibBand]:
    roi = rib_mask[ROI_Y0 : ROI_Y1 + 1, ROI_X0 : ROI_X1 + 1]
    row_pixels = roi.sum(axis=1)
    rows = np.where(row_pixels >= MIN_RIB_ROW_PIXELS)[0] + ROI_Y0
    bands: list[RibBand] = []

    for ymin, ymax in group_rows(rows):
        if ymax - ymin + 1 < MIN_RIB_HEIGHT:
            continue

        band_mask = rib_mask[ymin : ymax + 1, ROI_X0 : ROI_X1 + 1]
        col_pixels = band_mask.sum(axis=0)
        cols = np.where(col_pixels > 0)[0]
        if cols.size == 0:
            continue

        xmin = int(cols.min()) + ROI_X0
        xmax = int(cols.max()) + ROI_X0
        width = xmax - xmin + 1
        if width < MIN_RIB_WIDTH:
            continue

        area = int(band_mask[:, (xmin - ROI_X0) : (xmax - ROI_X0 + 1)].sum())
        ys, xs = np.where(rib_mask[ymin : ymax + 1, xmin : xmax + 1])
        mean_gray = float(gray[ymin : ymax + 1, xmin : xmax + 1][ys, xs].mean()) if ys.size else 0.0
        bands.append(
            RibBand(
                ymin=ymin,
                ymax=ymax,
                xmin=xmin,
                xmax=xmax,
                area=area,
                max_row_pixels=int(row_pixels[(ymin - ROI_Y0) : (ymax - ROI_Y0 + 1)].max()),
                mean_gray=mean_gray,
            )
        )

    return sorted(bands, key=lambda item: item.center_y)


def detect_white_component(gray: np.ndarray, white_threshold_int: int) -> tuple[np.ndarray, list[Component], Component | None]:
    raw_mask = gray >= white_threshold_int
    components = find_components(raw_mask, gray)
    candidates: list[Component] = []
    for component in components:
        component.score = score_white_component(component)
        if component.area < MIN_COMPONENT_AREA:
            continue
        if component.width < MIN_COMPONENT_WIDTH or component.height < MIN_COMPONENT_HEIGHT:
            continue
        if component.fill_ratio < 0.22:
            continue
        candidates.append(component)
    candidates.sort(key=lambda item: item.score, reverse=True)
    return raw_mask, components, candidates[0] if candidates else None


def summarize_centerline(runs: list[WhiteRun], fallback_x: float | None) -> dict:
    if not runs:
        target_x = fallback_x if fallback_x is not None else IMAGE_CENTER_X
        return {
            "target_x": float(target_x),
            "steer_error_px": float(target_x - IMAGE_CENTER_X),
            "row_count": 0,
            "bottom_row_count": 0,
            "top_y": None,
            "bottom_y": None,
            "mean_width": 0.0,
        }

    bottom_runs = [run for run in runs if run.y >= ROI_Y1 - BOTTOM_TARGET_ROWS + 1]
    target_source = bottom_runs if bottom_runs else runs[-min(len(runs), 8) :]
    weighted_sum = 0.0
    weight_total = 0.0
    for run in target_source:
        weight = 1.0 + 0.08 * max(0, run.y - ROI_Y0)
        weighted_sum += weight * run.center_x
        weight_total += weight
    target_x = weighted_sum / max(weight_total, 1e-6)

    return {
        "target_x": float(target_x),
        "steer_error_px": float(target_x - IMAGE_CENTER_X),
        "row_count": len(runs),
        "bottom_row_count": len(bottom_runs),
        "top_y": runs[0].y,
        "bottom_y": runs[-1].y,
        "mean_width": float(np.mean([run.width for run in runs])),
    }


def classify_phase(component: Component | None, centerline: dict, rib_bands: list[RibBand]) -> str:
    row_count = centerline["row_count"]
    bottom_row_count = centerline["bottom_row_count"]
    top_y = centerline["top_y"]
    bottom_y = centerline["bottom_y"]

    if rib_bands:
        return "inside_bumpy"

    if component is None and bottom_y is not None and bottom_y >= 56 and bottom_row_count >= 3:
        return "exit_bumpy"

    if component is None and row_count < 4:
        return "uncertain"

    if top_y is not None and top_y <= 18 and bottom_row_count <= 5:
        return "approach_bumpy"

    if bottom_y is not None and bottom_y >= 54 and top_y is not None and top_y >= 20:
        return "exit_bumpy"

    if row_count >= 8 and bottom_row_count >= 4:
        return "white_surface_only"

    if component is not None:
        return "approach_bumpy"

    return "uncertain"


def controller_mode_from_phase(phase: str) -> str:
    if phase == "approach_bumpy":
        return "seek_bumpy_entrance"
    if phase == "inside_bumpy":
        return "follow_bumpy_centerline"
    if phase == "exit_bumpy":
        return "hold_exit_line"
    if phase == "white_surface_only":
        return "hold_white_surface"
    return "fallback_search"


def detect_bumpy_road_frame(
    rgb: np.ndarray,
    prev_white_threshold: float | None = None,
) -> dict:
    gray = gray_from_rgb(rgb)
    white_threshold, white_candidate = estimate_white_threshold(gray, prev_white_threshold)
    white_threshold_int = int(round(white_threshold))

    global_white_mask, white_components, best_component = detect_white_component(gray, white_threshold_int)
    scan_white_mask, centerline_runs = build_white_scan_mask(gray, white_threshold)
    white_mask = global_white_mask | scan_white_mask

    fallback_x = best_component.centroid_x if best_component is not None else None
    centerline = summarize_centerline(centerline_runs, fallback_x=fallback_x)

    dark_threshold = estimate_dark_threshold(gray)
    rib_mask = build_supported_dark_mask(gray, white_mask, dark_threshold)
    rib_bands = find_rib_bands(gray, rib_mask)

    phase = classify_phase(best_component, centerline, rib_bands)
    controller_mode = controller_mode_from_phase(phase)

    return {
        "gray": gray,
        "white_threshold": float(white_threshold),
        "white_threshold_candidate": float(white_candidate),
        "white_threshold_int": white_threshold_int,
        "dark_threshold": float(dark_threshold),
        "global_white_mask": global_white_mask,
        "scan_white_mask": scan_white_mask,
        "white_mask": white_mask,
        "rib_mask": rib_mask,
        "white_components": white_components,
        "best_component": best_component,
        "centerline_runs": centerline_runs,
        "centerline": centerline,
        "rib_bands": rib_bands,
        "phase": phase,
        "controller_mode": controller_mode,
    }
