"""First-pass minefield boundary detector for the annotated `雷区people` samples.

This script focuses on the three manually annotated fixed-exposure-30 groups:
- 侧视角
- 斜视角
- 直视角

For each representative frame it will:
1. read the original grayscale frame and the annotated overlay frame
2. extract red/green annotation masks from the overlay frame
3. sweep several thresholds to find a stable outer quadrilateral
4. reuse an inner contour when possible, otherwise infer the inner quad from
   the known 100 cm square + 10 cm tape width geometry
5. save visual diagnostics and a small JSON summary under
   `data/雷区室外偏振片/people_v1_representative`
"""

from __future__ import annotations

import argparse
import json
import math
from dataclasses import asdict, dataclass
from itertools import combinations
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DATA_ROOT = PROJECT_ROOT / "data/雷区室外偏振片"
ANNOTATED_DIR = DATA_ROOT / "frames/雷区people"
OUTPUT_DIR = DATA_ROOT / "people_v1_representative"

SCALE = 6
LINE_THICKNESS = 1
OUTER_SIDE_CM = 100.0
TAPE_WIDTH_CM = 10.0

REPRESENTATIVE_FRAMES: dict[str, tuple[int, ...]] = {
    "雷区阴天30固定曝光侧_20260713_180026": (3, 71, 159, 202, 302, 371),
    "雷区阴天30固定曝光斜_20260713_175949": (18, 175, 264, 303, 499),
    "雷区阴天30固定曝光直_20260713_180007": (19, 159, 237, 304),
}

RED_TARGET = np.array([237, 28, 36], dtype=np.int16)
GREEN_TARGET = np.array([34, 177, 76], dtype=np.int16)


@dataclass(frozen=True)
class Sample:
    prefix: str
    frame_idx: int
    annotated_path: Path
    original_path: Path
    split: str
    has_annotation: bool

    @property
    def frame_name(self) -> str:
        return f"{self.prefix}_frame{self.frame_idx:06d}.png"


@dataclass
class Candidate:
    threshold: int
    score: float
    outer_area: float
    outer_bbox: tuple[int, int, int, int]
    child_area: float
    touches_border: bool
    outer_quad: np.ndarray
    inner_quad: np.ndarray | None
    inferred_inner_quad: np.ndarray | None
    threshold_mask: np.ndarray


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Detect outer/inner minefield boundaries on representative 雷区people samples."
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=OUTPUT_DIR,
        help="Directory for all outputs under data/雷区室外偏振片",
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


def extract_annotation_masks(annotated_rgb: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    rgb16 = annotated_rgb.astype(np.int16)
    red_dist = np.linalg.norm(rgb16 - RED_TARGET, axis=2)
    green_dist = np.linalg.norm(rgb16 - GREEN_TARGET, axis=2)
    red_mask = (red_dist <= 48.0) & (red_dist + 10.0 < green_dist)
    green_mask = (green_dist <= 55.0) & (green_dist + 10.0 < red_dist)
    return red_mask, green_mask


def build_samples() -> list[Sample]:
    samples: list[Sample] = []
    for prefix, frame_indices in REPRESENTATIVE_FRAMES.items():
        frame_dir = DATA_ROOT / "frames" / prefix
        for frame_idx in frame_indices:
            frame_name = f"{prefix}_frame{frame_idx:06d}.png"
            annotated_path = ANNOTATED_DIR / frame_name
            original_path = frame_dir / frame_name
            if not annotated_path.exists():
                raise FileNotFoundError(annotated_path)
            if not original_path.exists():
                raise FileNotFoundError(original_path)
            annotated_rgb = read_rgb(annotated_path)
            red_mask, green_mask = extract_annotation_masks(annotated_rgb)
            has_annotation = bool(red_mask.any() or green_mask.any())
            samples.append(
                Sample(
                    prefix=prefix,
                    frame_idx=frame_idx,
                    annotated_path=annotated_path,
                    original_path=original_path,
                    split="positive" if has_annotation else "negative",
                    has_annotation=has_annotation,
                )
            )
    return samples


def order_quad(points: np.ndarray) -> np.ndarray:
    pts = np.asarray(points, dtype=np.float32).reshape(-1, 2)
    centroid = pts.mean(axis=0)
    angles = np.arctan2(pts[:, 1] - centroid[1], pts[:, 0] - centroid[0])
    pts = pts[np.argsort(angles)]
    start = int(np.argmin(pts.sum(axis=1)))
    pts = np.roll(pts, -start, axis=0)
    area = cv2.contourArea(pts.reshape(-1, 1, 2))
    if area < 0:
        pts = pts[[0, 3, 2, 1]]
    return pts


def reduce_polygon_to_quad(points: np.ndarray) -> np.ndarray | None:
    pts = np.asarray(points, dtype=np.float32).reshape(-1, 2)
    if len(pts) < 4:
        return None
    best_quad: np.ndarray | None = None
    best_area = -1.0
    for combo in combinations(range(len(pts)), 4):
        quad = pts[list(combo)]
        quad = order_quad(quad)
        area = float(abs(cv2.contourArea(quad.reshape(-1, 1, 2))))
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


def infer_inner_quad(outer_quad: np.ndarray) -> np.ndarray | None:
    if outer_quad is None:
        return None
    inner_offset = TAPE_WIDTH_CM
    inner_side = OUTER_SIDE_CM - 2.0 * inner_offset
    if inner_side <= 0:
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
            [inner_offset, inner_offset],
            [inner_offset + inner_side, inner_offset],
            [inner_offset + inner_side, inner_offset + inner_side],
            [inner_offset, inner_offset + inner_side],
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


def score_candidate(
    outer_area: float,
    bbox: tuple[int, int, int, int],
    child_area: float,
    touches_border: bool,
    outer_quad: np.ndarray | None,
    inner_quad: np.ndarray | None,
) -> float:
    x, y, w, h = bbox
    area_score = min(outer_area / 380.0, 1.0)
    width_score = min(w / 58.0, 1.0)
    height_score = min(h / 21.0, 1.0)
    child_score = min(child_area / 220.0, 1.0)
    quad_score = 1.0 if outer_quad is not None else 0.0
    inner_score = 1.0 if inner_quad is not None else 0.0

    score = (
        0.36 * area_score
        + 0.16 * width_score
        + 0.12 * height_score
        + 0.16 * quad_score
        + 0.14 * child_score
        + 0.06 * inner_score
    )

    if outer_area < 100.0 and child_area < 25.0:
        score -= 0.16
    if y < 7 and h < 8 and child_area < 25.0:
        score -= 0.08
    if touches_border and outer_area < 85.0:
        score -= 0.05
    return score


def detect_candidate(gray: np.ndarray) -> Candidate | None:
    h, w = gray.shape
    blurred = cv2.GaussianBlur(gray, (3, 3), 0)
    otsu_threshold, _ = cv2.threshold(blurred, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    thresholds = {
        int(np.clip(otsu_threshold + delta, 85, 185))
        for delta in (-12, -6, 0, 6, 12, 18, 24, 30, 36)
    }
    thresholds.update(range(90, 171, 10))

    best: Candidate | None = None
    for threshold in sorted(thresholds):
        _, mask_u8 = cv2.threshold(blurred, threshold, 255, cv2.THRESH_BINARY)
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
            inner_quad = None
            if child_contour is not None and child_area >= 18.0:
                inner_quad = approx_quad_from_contour(child_contour, allow_box_fallback=False)

            inferred_inner_quad = inner_quad if inner_quad is not None else infer_inner_quad(outer_quad)
            touches_border = x == 0 or y == 0 or (x + bw) >= w or (y + bh) >= h
            score = score_candidate(
                outer_area=outer_area,
                bbox=(x, y, bw, bh),
                child_area=child_area,
                touches_border=touches_border,
                outer_quad=outer_quad,
                inner_quad=inner_quad,
            )

            candidate = Candidate(
                threshold=threshold,
                score=score,
                outer_area=outer_area,
                outer_bbox=(x, y, bw, bh),
                child_area=child_area,
                touches_border=touches_border,
                outer_quad=outer_quad,
                inner_quad=inner_quad,
                inferred_inner_quad=inferred_inner_quad,
                threshold_mask=mask_u8.astype(bool),
            )

            if best is None or candidate.score > best.score:
                best = candidate
    if best is None or best.score < 0.36:
        return None
    return best


def render_line_mask(shape: tuple[int, int], quad: np.ndarray | None) -> np.ndarray:
    mask = np.zeros(shape, dtype=np.uint8)
    if quad is None:
        return mask.astype(bool)
    pts = np.round(quad).astype(np.int32).reshape(-1, 1, 2)
    cv2.polylines(mask, [pts], isClosed=True, color=255, thickness=LINE_THICKNESS, lineType=cv2.LINE_AA)
    return mask.astype(bool)


def mask_scores(pred_mask: np.ndarray, gt_mask: np.ndarray, radius: int = 1) -> dict[str, float]:
    kernel = np.ones((radius * 2 + 1, radius * 2 + 1), dtype=np.uint8)
    pred = pred_mask.astype(np.uint8)
    gt = gt_mask.astype(np.uint8)
    pred_d = cv2.dilate(pred, kernel, iterations=1).astype(bool)
    gt_d = cv2.dilate(gt, kernel, iterations=1).astype(bool)

    pred_pixels = int(pred.sum())
    gt_pixels = int(gt.sum())
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


def make_mask_debug(mask: np.ndarray, red_mask: np.ndarray, green_mask: np.ndarray) -> np.ndarray:
    canvas = np.zeros((mask.shape[0], mask.shape[1], 3), dtype=np.uint8)
    canvas[mask] = (255, 255, 255)
    canvas[red_mask] = (255, 0, 0)
    canvas[green_mask] = (0, 255, 0)
    return canvas


def scaled_gray_rgb(gray: np.ndarray) -> np.ndarray:
    rgb = np.repeat(gray[:, :, None], 3, axis=2)
    return cv2.resize(rgb, (gray.shape[1] * SCALE, gray.shape[0] * SCALE), interpolation=cv2.INTER_NEAREST)


def draw_quad(draw: ImageDraw.ImageDraw, quad: np.ndarray | None, color: tuple[int, int, int], width: int) -> None:
    if quad is None:
        return
    pts = [tuple((np.asarray(point) * SCALE).tolist()) for point in quad]
    pts.append(pts[0])
    draw.line(pts, fill=color, width=width)


def make_prediction_overlay(gray: np.ndarray, outer_quad: np.ndarray | None, inner_quad: np.ndarray | None, title: str) -> np.ndarray:
    image = Image.fromarray(scaled_gray_rgb(gray), mode="RGB")
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), title, fill=(255, 255, 255))
    draw_quad(draw, outer_quad, (255, 0, 0), 2)
    draw_quad(draw, inner_quad, (0, 255, 0), 2)
    return np.asarray(image)


def make_comparison_overlay(
    gray: np.ndarray,
    gt_outer_mask: np.ndarray,
    gt_inner_mask: np.ndarray,
    pred_outer: np.ndarray | None,
    pred_inner: np.ndarray | None,
    title: str,
) -> np.ndarray:
    base = scaled_gray_rgb(gray)
    gt_outer_up = cv2.resize(
        (gt_outer_mask.astype(np.uint8) * 255),
        (base.shape[1], base.shape[0]),
        interpolation=cv2.INTER_NEAREST,
    ).astype(bool)
    gt_inner_up = cv2.resize(
        (gt_inner_mask.astype(np.uint8) * 255),
        (base.shape[1], base.shape[0]),
        interpolation=cv2.INTER_NEAREST,
    ).astype(bool)

    overlay = base.copy()
    overlay[gt_outer_up] = (255, 180, 0)
    overlay[gt_inner_up] = (0, 220, 255)

    image = Image.fromarray(overlay, mode="RGB")
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), title, fill=(255, 255, 255))
    draw_quad(draw, pred_outer, (255, 0, 0), 2)
    draw_quad(draw, pred_inner, (0, 255, 0), 2)
    return np.asarray(image)


def evaluate_sample(sample: Sample, output_dir: Path) -> dict:
    gray = read_gray(sample.original_path)
    annotated_rgb = read_rgb(sample.annotated_path)
    gt_red_mask, gt_green_mask = extract_annotation_masks(annotated_rgb)
    candidate = detect_candidate(gray)

    pred_outer_quad = candidate.outer_quad if candidate is not None else None
    pred_inner_quad = candidate.inferred_inner_quad if candidate is not None else None
    pred_red_mask = render_line_mask(gray.shape, pred_outer_quad)
    pred_green_mask = render_line_mask(gray.shape, pred_inner_quad)

    sample_dir = output_dir / sample.prefix / f"frame_{sample.frame_idx:06d}"
    sample_dir.mkdir(parents=True, exist_ok=True)

    title = f"{sample.frame_name} score={candidate.score:.2f}" if candidate is not None else f"{sample.frame_name} score=NA"
    prediction_overlay = make_prediction_overlay(gray, pred_outer_quad, pred_inner_quad, title)
    comparison_overlay = make_comparison_overlay(
        gray,
        gt_red_mask,
        gt_green_mask,
        pred_outer_quad,
        pred_inner_quad,
        f"{sample.frame_name} gt=orange/cyan pred=red/green",
    )
    threshold_mask = candidate.threshold_mask if candidate is not None else np.zeros_like(gray, dtype=bool)
    debug_mask = make_mask_debug(threshold_mask, gt_red_mask, gt_green_mask)

    save_gray(sample_dir / "01_original.png", gray)
    save_rgb(sample_dir / "02_annotation.png", annotated_rgb)
    save_rgb(sample_dir / "03_prediction_overlay.png", prediction_overlay)
    save_rgb(sample_dir / "04_comparison_overlay.png", comparison_overlay)
    save_rgb(sample_dir / "05_threshold_and_gt.png", debug_mask)
    save_mask(sample_dir / "06_pred_outer_mask.png", pred_red_mask)
    save_mask(sample_dir / "07_pred_inner_mask.png", pred_green_mask)
    save_mask(sample_dir / "08_gt_outer_mask.png", gt_red_mask)
    save_mask(sample_dir / "09_gt_inner_mask.png", gt_green_mask)

    red_scores = mask_scores(pred_red_mask, gt_red_mask)
    green_scores = mask_scores(pred_green_mask, gt_green_mask)
    avg_f1 = (red_scores["f1"] + green_scores["f1"]) / 2.0

    result = {
        "frame_name": sample.frame_name,
        "prefix": sample.prefix,
        "frame_idx": sample.frame_idx,
        "split": sample.split,
        "has_annotation": sample.has_annotation,
        "paths": {
            "annotated": str(sample.annotated_path),
            "original": str(sample.original_path),
            "output_dir": str(sample_dir),
        },
        "detection": None,
        "metrics": {
            "outer": red_scores,
            "inner": green_scores,
            "avg_f1": avg_f1,
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
            "outer_quad": np.round(candidate.outer_quad, 2).tolist(),
            "inner_quad_direct": None if candidate.inner_quad is None else np.round(candidate.inner_quad, 2).tolist(),
            "inner_quad_final": None
            if candidate.inferred_inner_quad is None
            else np.round(candidate.inferred_inner_quad, 2).tolist(),
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
        tiles.append(Image.open(sample_dir / "04_comparison_overlay.png").convert("RGB"))

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


def write_summary(results: list[dict], output_dir: Path) -> None:
    total = len(results)
    positives = sum(1 for result in results if result["has_annotation"])
    negatives = total - positives
    detected = sum(1 for result in results if result["detection"] is not None)
    avg_outer_f1 = float(np.mean([result["metrics"]["outer"]["f1"] for result in results])) if results else 0.0
    avg_inner_f1 = float(np.mean([result["metrics"]["inner"]["f1"] for result in results])) if results else 0.0
    avg_f1 = float(np.mean([result["metrics"]["avg_f1"] for result in results])) if results else 0.0

    summary = {
        "dataset": "雷区people representative subset",
        "sample_count": total,
        "positive_count": positives,
        "negative_count": negatives,
        "detected_count": detected,
        "mean_outer_f1": avg_outer_f1,
        "mean_inner_f1": avg_inner_f1,
        "mean_avg_f1": avg_f1,
        "assumptions": {
            "outer_square_side_cm": OUTER_SIDE_CM,
            "tape_width_cm": TAPE_WIDTH_CM,
            "inner_quad_strategy": "use detected inner contour when available, otherwise infer by projective inset",
        },
        "results": results,
    }

    summary_lines = [
        "雷区 people 第一版代表样本结果",
        f"样本数: {total}",
        f"正样本: {positives}",
        f"负样本: {negatives}",
        f"给出检测结果的样本数: {detected}",
        f"外框平均 F1: {avg_outer_f1:.3f}",
        f"内框平均 F1: {avg_inner_f1:.3f}",
        f"总平均 F1: {avg_f1:.3f}",
        "",
        "当前假设:",
        f"1. 外框对应 100 cm 正方形。",
        f"2. 白色胶带宽度为 10 cm，因此当内框不稳定时，按 10 cm projective inset 反推内框。",
        "3. 第一版仅在 `雷区people` 的 30 固定曝光代表帧上验证，不扩展到 60 曝光和固定阈值视频。",
        "",
        "文件说明:",
        "1. `contact_sheet.png` 为代表帧总览，细线是真值，粗线是算法结果。",
        "2. 每个样本目录下 `03_prediction_overlay.png` 是纯算法输出，`04_comparison_overlay.png` 用于对照人工标注。",
        "3. `05_threshold_and_gt.png` 中白色是当前阈值二值结果，红/绿是真值标注掩膜。",
    ]

    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    (output_dir / "summary.txt").write_text("\n".join(summary_lines), encoding="utf-8")
    (output_dir / "subset_manifest.json").write_text(
        json.dumps(
            [
                {
                    "prefix": sample.prefix,
                    "frame_idx": sample.frame_idx,
                    "frame_name": sample.frame_name,
                    "split": sample.split,
                }
                for sample in build_samples()
            ],
            ensure_ascii=False,
            indent=2,
        ),
        encoding="utf-8",
    )


def main() -> None:
    args = parse_args()
    samples = build_samples()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    results = [evaluate_sample(sample, args.output_dir) for sample in samples]
    build_contact_sheet(results, args.output_dir / "contact_sheet.png")
    write_summary(results, args.output_dir)

    print(f"output_dir: {args.output_dir}")
    print(f"samples: {len(results)}")
    print(f"mean_avg_f1: {np.mean([result['metrics']['avg_f1'] for result in results]):.4f}")


if __name__ == "__main__":
    main()
