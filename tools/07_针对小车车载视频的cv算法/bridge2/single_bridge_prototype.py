from __future__ import annotations

import argparse
import csv
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

import numpy as np
from PIL import Image, ImageDraw
from scipy import ndimage
from scipy.spatial import ConvexHull, QhullError


ROOT = Path(__file__).resolve().parent
FRAMES_DIR = ROOT / "frames"
DEFAULT_SUBSET = ROOT / "single_bridge_subset.json"
DEFAULT_OUTPUT = ROOT / "python_results"

STATE_NONE = "无"
STATE_PREPARE_ENTER = "准备进入"
STATE_ON_PVC = "在PVC上"
STATE_PREPARE_EXIT = "准备退出"

MIN_VALID_SCORE = 350.0


@dataclass
class PvcCandidate:
    threshold: int
    score: float
    mask: np.ndarray
    left: np.ndarray
    right: np.ndarray
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
    pvc_left_border: list[int]
    pvc_right_border: list[int]


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


def extract_borders(mask: np.ndarray) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
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


def smooth_borders(
    left: np.ndarray,
    right: np.ndarray,
    start_row: int,
    end_row: int,
    image_width: int,
) -> tuple[np.ndarray, np.ndarray]:
    smoothed_left = np.full(left.shape[0], -1, dtype=np.int16)
    smoothed_right = np.full(right.shape[0], -1, dtype=np.int16)
    if start_row < 0 or end_row < start_row:
        return smoothed_left, smoothed_right

    kernel = np.array([1.0, 2.0, 3.0, 2.0, 1.0], dtype=np.float64)
    kernel /= kernel.sum()

    left_segment = left[start_row : end_row + 1].astype(np.float64)
    right_segment = right[start_row : end_row + 1].astype(np.float64)

    if left_segment.size >= kernel.size:
        left_segment = np.convolve(np.pad(left_segment, (2, 2), mode="edge"), kernel, mode="valid")
        right_segment = np.convolve(np.pad(right_segment, (2, 2), mode="edge"), kernel, mode="valid")

    left_segment = np.rint(left_segment).astype(np.int16)
    right_segment = np.rint(right_segment).astype(np.int16)

    for index in range(left_segment.size):
        if right_segment[index] <= left_segment[index]:
            center = int(round((int(left_segment[index]) + int(right_segment[index])) * 0.5))
            left_segment[index] = max(center - 1, 0)
            right_segment[index] = min(center + 1, image_width - 1)

    smoothed_left[start_row : end_row + 1] = left_segment
    smoothed_right[start_row : end_row + 1] = right_segment
    return smoothed_left, smoothed_right


def evaluate_component(gray: np.ndarray, component: np.ndarray, threshold: int) -> PvcCandidate | None:
    filled = ndimage.binary_fill_holes(component)
    filled = ndimage.binary_closing(filled, structure=np.ones((3, 3), dtype=bool))
    outer_mask = convex_hull_mask(filled)
    if not outer_mask.any():
        outer_mask = filled.astype(bool)

    left, right, widths = extract_borders(outer_mask)
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
        mask=outer_mask.astype(bool),
        left=left,
        right=right,
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


def infer_pvc_state(candidate: PvcCandidate | None, image_height: int) -> tuple[bool, str]:
    if candidate is None:
        return False, STATE_NONE
    if candidate.score < MIN_VALID_SCORE or candidate.edge_contrast < 20.0:
        return False, STATE_NONE

    exit_clip_ratio = max(candidate.left_clip_ratio, candidate.right_clip_ratio)
    if candidate.bottom_row <= image_height - 10:
        return True, STATE_PREPARE_ENTER
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
    pvc_found, pvc_state = infer_pvc_state(candidate, gray.shape[0])

    if pvc_found and candidate is not None:
        left_border, right_border = smooth_borders(
            candidate.left,
            candidate.right,
            candidate.start_row,
            candidate.bottom_row,
            gray.shape[1],
        )
        pvc_center_x = round(candidate.center_x, 3)
        threshold = candidate.threshold
        area = candidate.area
        area_ratio = round(candidate.area_ratio, 4)
        top_row = candidate.top_row
        start_row = candidate.start_row
        bottom_row = candidate.bottom_row
        max_width = candidate.max_width
        bottom_width = candidate.bottom_width
        edge_contrast = round(candidate.edge_contrast, 3)
        left_clip_ratio = round(candidate.left_clip_ratio, 3)
        right_clip_ratio = round(candidate.right_clip_ratio, 3)
        dual_clip_ratio = round(candidate.dual_clip_ratio, 3)
        border_monotonic = round(candidate.border_monotonic, 3)
        candidate_score = round(candidate.score, 3)
        draw_mask = candidate.mask
    else:
        left_border = np.full(gray.shape[0], -1, dtype=np.int16)
        right_border = np.full(gray.shape[0], -1, dtype=np.int16)
        pvc_center_x = None
        threshold = candidate.threshold if candidate is not None else -1
        area = candidate.area if candidate is not None else 0
        area_ratio = round(candidate.area_ratio, 4) if candidate is not None else 0.0
        top_row = candidate.top_row if candidate is not None else -1
        start_row = candidate.start_row if candidate is not None else -1
        bottom_row = candidate.bottom_row if candidate is not None else -1
        max_width = candidate.max_width if candidate is not None else 0
        bottom_width = candidate.bottom_width if candidate is not None else 0
        edge_contrast = round(candidate.edge_contrast, 3) if candidate is not None else 0.0
        left_clip_ratio = round(candidate.left_clip_ratio, 3) if candidate is not None else 0.0
        right_clip_ratio = round(candidate.right_clip_ratio, 3) if candidate is not None else 0.0
        dual_clip_ratio = round(candidate.dual_clip_ratio, 3) if candidate is not None else 0.0
        border_monotonic = round(candidate.border_monotonic, 3) if candidate is not None else 0.0
        candidate_score = round(candidate.score, 3) if candidate is not None else 0.0
        draw_mask = np.zeros_like(gray, dtype=bool)

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
        pvc_left_border=left_border.astype(int).tolist(),
        pvc_right_border=right_border.astype(int).tolist(),
    )

    draw_image = render_draw(gray, result)
    mask_image = Image.fromarray((draw_mask.astype(np.uint8) * 255), mode="L")
    return result, draw_image, mask_image


def render_draw(gray: np.ndarray, result: BridgeResult) -> Image.Image:
    rgb = np.stack([gray, gray, gray], axis=-1).astype(np.uint8)
    image = Image.fromarray(rgb, mode="RGB")
    draw = ImageDraw.Draw(image)

    rows = [row for row, left_x in enumerate(result.pvc_left_border) if left_x >= 0 and result.pvc_right_border[row] >= 0]
    if rows:
        left_points = [(int(result.pvc_left_border[row]), int(row)) for row in rows]
        right_points = [(int(result.pvc_right_border[row]), int(row)) for row in rows]
        if len(left_points) >= 2:
            draw.line(left_points, fill=(255, 0, 0), width=1)
        else:
            draw.point(left_points[0], fill=(255, 0, 0))

        if len(right_points) >= 2:
            draw.line(right_points, fill=(0, 150, 255), width=1)
        else:
            draw.point(right_points[0], fill=(0, 150, 255))

        start_row = result.pvc_start_row
        end_row = result.pvc_bottom_row
        if start_row >= 0 and result.pvc_left_border[start_row] >= 0 and result.pvc_right_border[start_row] >= 0:
            draw.line(
                (
                    int(result.pvc_left_border[start_row]),
                    int(start_row),
                    int(result.pvc_right_border[start_row]),
                    int(start_row),
                ),
                fill=(255, 105, 180),
                width=1,
            )
        if end_row >= 0 and result.pvc_left_border[end_row] >= 0 and result.pvc_right_border[end_row] >= 0:
            draw.line(
                (
                    int(result.pvc_left_border[end_row]),
                    int(end_row),
                    int(result.pvc_right_border[end_row]),
                    int(end_row),
                ),
                fill=(255, 220, 0),
                width=1,
            )

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
            "draw/: 巡线结果图，只画四类线。",
            "originals/: 对应原图拷贝，方便逐张对比。",
            "masks/: 恢复出的 PVC 外轮廓掩码。",
            "compare/: 每张图的 原图 | 巡线图 并排对照图。",
            "",
            "线条颜色：",
            "红色 = 左边线",
            "蓝色 = 右边线",
            "粉色 = 起始线",
            "黄色 = 终止线",
            "",
            "summary.csv / summary.json 中只保留 PVC 四状态：无、准备进入、在PVC上、准备退出。",
        ]
    )
    (output_dir / "README.txt").write_text(readme, encoding="utf-8")
    (output_dir / "README.md").write_text(readme, encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="PVC-only single-bridge prototype for representative frames.")
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
