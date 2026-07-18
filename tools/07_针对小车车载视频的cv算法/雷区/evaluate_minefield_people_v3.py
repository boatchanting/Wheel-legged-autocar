from __future__ import annotations

import argparse
import csv
import json
import math
import sys
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from PIL import Image

CURRENT_DIR = Path(__file__).resolve().parent
if str(CURRENT_DIR) not in sys.path:
    sys.path.insert(0, str(CURRENT_DIR))

from detect_minefield_people_v2_json import (  # noqa: E402
    DATA_ROOT,
    SCALE,
    detect_best_candidate,
    extract_prediction_segments,
    load_ground_truth,
    make_comparison_overlay,
    make_gt_overlay,
    make_prediction_overlay,
    make_threshold_debug,
    mask_scores,
    quad_corner_error,
    quad_to_list,
    read_gray,
    read_rgb,
    render_segments_mask,
    save_gray,
    save_mask,
    save_rgb,
    should_suppress_inner_segments,
)


DEFAULT_ANNOTATION_DIR = DATA_ROOT / "frames/雷区peoplev3"
DEFAULT_OUTPUT_DIR = DATA_ROOT / "test/peoplev3_v12"

CENTRAL_LARGE_MIN_WIDTH_RATIO = 0.80
CENTRAL_LARGE_MIN_HEIGHT_RATIO = 0.35
CENTRAL_LARGE_MAX_CENTER_DX = 0.18
CENTRAL_LARGE_MAX_CENTER_DY = 0.18
CENTRAL_LARGE_WEIGHT = 3.0


@dataclass(frozen=True)
class AnnotatedSample:
    frame_name: str
    prefix: str
    frame_idx: int
    original_path: Path
    annotation_png_path: Path
    annotation_json_path: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Evaluate the minefield detector against peoplev3 JSON annotations."
    )
    parser.add_argument(
        "--annotation-dir",
        type=Path,
        default=DEFAULT_ANNOTATION_DIR,
        help="Directory containing peoplev3 annotation PNG/JSON pairs.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help="Output directory for test artifacts.",
    )
    parser.add_argument(
        "--fixed-threshold",
        type=int,
        default=None,
        help="Optional fixed threshold. Default uses the detector sweep.",
    )
    parser.add_argument(
        "--fixed-threshold-candidates",
        type=int,
        nargs="+",
        default=None,
        help="Optional list of fixed thresholds. The detector keeps the highest-score candidate among them.",
    )
    parser.add_argument(
        "--max-samples",
        type=int,
        default=None,
        help="Optional cap for quick smoke runs.",
    )
    parser.add_argument(
        "--contact-cols",
        type=int,
        default=3,
        help="Columns per contact sheet page.",
    )
    parser.add_argument(
        "--contact-rows",
        type=int,
        default=4,
        help="Rows per contact sheet page.",
    )
    return parser.parse_args()


def parse_frame_idx(frame_name: str) -> int:
    stem = Path(frame_name).stem
    return int(stem.rsplit("_frame", 1)[1])


def build_samples(annotation_dir: Path, max_samples: int | None = None) -> list[AnnotatedSample]:
    samples: list[AnnotatedSample] = []
    json_paths = sorted(annotation_dir.glob("*.json"))
    if max_samples is not None:
        json_paths = json_paths[: max(0, max_samples)]

    for json_path in json_paths:
        frame_name = f"{json_path.stem}.png"
        prefix = json_path.stem.rsplit("_frame", 1)[0]
        original_path = DATA_ROOT / "frames" / prefix / frame_name
        annotation_png_path = annotation_dir / frame_name
        if not original_path.exists():
            raise FileNotFoundError(original_path)
        if not annotation_png_path.exists():
            raise FileNotFoundError(annotation_png_path)

        samples.append(
            AnnotatedSample(
                frame_name=frame_name,
                prefix=prefix,
                frame_idx=parse_frame_idx(frame_name),
                original_path=original_path,
                annotation_png_path=annotation_png_path,
                annotation_json_path=json_path,
            )
        )
    return samples


def scene_bbox_from_segments(segments) -> tuple[int, int, int, int] | None:
    points: list[tuple[int, int]] = []
    for segment in segments:
        points.append((segment.x1, segment.y1))
        points.append((segment.x2, segment.y2))
    if not points:
        return None

    xs = [point[0] for point in points]
    ys = [point[1] for point in points]
    min_x = min(xs)
    max_x = max(xs)
    min_y = min(ys)
    max_y = max(ys)
    return min_x, min_y, max_x - min_x + 1, max_y - min_y + 1


def describe_scene_layout(gt) -> dict:
    bbox = scene_bbox_from_segments(gt.outer_segments + gt.inner_segments)
    height, width = gt.outer_mask.shape
    if bbox is None:
        return {
            "gt_bbox": None,
            "width_ratio": 0.0,
            "height_ratio": 0.0,
            "area_ratio": 0.0,
            "center_dx": 1.0,
            "center_dy": 1.0,
            "central_large": False,
        }

    x, y, bw, bh = bbox
    center_x = x + (bw - 1) * 0.5
    center_y = y + (bh - 1) * 0.5
    width_ratio = bw / max(float(width), 1.0)
    height_ratio = bh / max(float(height), 1.0)
    area_ratio = (bw * bh) / max(float(width * height), 1.0)
    center_dx = abs(center_x - (width - 1) * 0.5) / max(width * 0.5, 1.0)
    center_dy = abs(center_y - (height - 1) * 0.5) / max(height * 0.5, 1.0)
    central_large = (
        width_ratio >= CENTRAL_LARGE_MIN_WIDTH_RATIO
        and height_ratio >= CENTRAL_LARGE_MIN_HEIGHT_RATIO
        and center_dx <= CENTRAL_LARGE_MAX_CENTER_DX
        and center_dy <= CENTRAL_LARGE_MAX_CENTER_DY
    )
    return {
        "gt_bbox": [x, y, bw, bh],
        "width_ratio": width_ratio,
        "height_ratio": height_ratio,
        "area_ratio": area_ratio,
        "center_dx": center_dx,
        "center_dy": center_dy,
        "central_large": central_large,
    }


def build_contact_sheets(
    results: list[dict],
    output_dir: Path,
    cols: int,
    rows: int,
) -> list[str]:
    page_size = cols * rows
    output_dir.mkdir(parents=True, exist_ok=True)
    page_paths: list[str] = []

    for page_index, start in enumerate(range(0, len(results), page_size), start=1):
        page_results = results[start:start + page_size]
        tiles: list[Image.Image] = []
        for result in page_results:
            sample_dir = Path(result["paths"]["output_dir"])
            comparison = Image.open(sample_dir / "05_comparison_overlay.png").convert("RGB")
            binary = Image.open(sample_dir / "06_threshold_debug.png").convert("RGB")
            tile = Image.new("RGB", (comparison.width, comparison.height + binary.height), (18, 18, 18))
            tile.paste(comparison, (0, 0))
            tile.paste(binary, (0, comparison.height))
            tiles.append(tile)

        if not tiles:
            continue

        tile_w, tile_h = tiles[0].size
        page = Image.new("RGB", (cols * tile_w, rows * tile_h), (18, 18, 18))
        for idx, tile in enumerate(tiles):
            x = (idx % cols) * tile_w
            y = (idx // cols) * tile_h
            page.paste(tile, (x, y))

        page_path = output_dir / f"contact_sheet_{page_index:03d}.png"
        page.save(page_path)
        page_paths.append(str(page_path))

    return page_paths


def evaluate_sample(
    sample: AnnotatedSample,
    output_dir: Path,
    fixed_threshold: int | None,
    fixed_threshold_candidates: list[int] | None,
) -> dict:
    gray = read_gray(sample.original_path)
    annotation_rgb = read_rgb(sample.annotation_png_path)
    gt = load_ground_truth(sample.annotation_json_path)
    scene_layout = describe_scene_layout(gt)
    scene_weight = CENTRAL_LARGE_WEIGHT if scene_layout["central_large"] else 1.0
    candidate = detect_best_candidate(
        gray,
        fixed_threshold=fixed_threshold,
        fixed_thresholds=fixed_threshold_candidates,
    )

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
    pred_score = "NA" if candidate is None else f"{candidate.score:.2f}"
    pred_overlay = make_prediction_overlay(gray, pred_outer_segments, pred_inner_segments, f"{sample.frame_name} pred score={pred_score}")
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
        "scene": {
            **scene_layout,
            "weight": scene_weight,
        },
        "paths": {
            "original": str(sample.original_path),
            "annotation_png": str(sample.annotation_png_path),
            "annotation_json": str(sample.annotation_json_path),
            "output_dir": str(sample_dir),
        },
        "threshold_mode": {
            "type": (
                "fixed-candidates"
                if fixed_threshold_candidates
                else "fixed"
                if fixed_threshold is not None
                else "sweep"
            ),
            "value": fixed_threshold_candidates if fixed_threshold_candidates else fixed_threshold,
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
            "outer_present_pred": bool(pred_outer_segments),
            "inner_present_pred": bool(pred_inner_segments),
            "scene_weight": scene_weight,
            "weighted_shortfall": (1.0 - annotated_role_mean_f1) * scene_weight,
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


def write_summary_csv(results: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "frame_name",
        "annotation_mode",
        "central_large",
        "scene_weight",
        "outer_f1",
        "inner_f1",
        "annotated_role_mean_f1",
        "threshold",
        "score",
        "far_view_factor",
        "inner_suppressed",
        "outer_area",
        "child_area",
    ]
    with output_path.open("w", encoding="utf-8-sig", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for result in results:
            detection = result["detection"] or {}
            writer.writerow(
                {
                    "frame_name": result["frame_name"],
                    "annotation_mode": result["annotation_mode"],
                    "central_large": result["scene"]["central_large"],
                    "scene_weight": result["metrics"]["scene_weight"],
                    "outer_f1": round(result["metrics"]["outer_line"]["f1"], 4),
                    "inner_f1": round(result["metrics"]["inner_line"]["f1"], 4),
                    "annotated_role_mean_f1": round(result["metrics"]["annotated_role_mean_f1"], 4),
                    "threshold": detection.get("threshold"),
                    "score": detection.get("score"),
                    "far_view_factor": detection.get("far_view_factor"),
                    "inner_suppressed": detection.get("inner_suppressed"),
                    "outer_area": detection.get("outer_area"),
                    "child_area": detection.get("child_area"),
                }
            )


def write_all_annotation_index(annotation_dir: Path, output_dir: Path) -> None:
    rows: list[dict] = []
    for json_path in sorted(annotation_dir.glob("*.json")):
        payload = json.loads(json_path.read_text(encoding="utf-8"))
        outer_segments = payload.get("outer_segments", [])
        inner_segments = payload.get("inner_segments", [])
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
                "outer_segments": outer_segments,
                "inner_segments": inner_segments,
            }
        )
    (output_dir / "all_annotation_index.json").write_text(
        json.dumps(rows, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )


def write_summary(
    results: list[dict],
    output_dir: Path,
    annotation_dir: Path,
    fixed_threshold: int | None,
    fixed_threshold_candidates: list[int] | None,
    contact_sheet_paths: list[str],
) -> None:
    outer_f1 = [result["metrics"]["outer_line"]["f1"] for result in results if result["metrics"]["outer_present_gt"]]
    inner_f1 = [result["metrics"]["inner_line"]["f1"] for result in results if result["metrics"]["inner_present_gt"]]
    strict_role_f1 = [result["metrics"]["strict_role_mean_f1"] for result in results]
    annotated_role_f1 = [result["metrics"]["annotated_role_mean_f1"] for result in results]
    central_large_results = [result for result in results if result["scene"]["central_large"]]
    central_large_annotated_role_f1 = [
        result["metrics"]["annotated_role_mean_f1"] for result in central_large_results
    ]
    total_weight = sum(result["metrics"]["scene_weight"] for result in results)
    weighted_annotated_role_f1 = (
        sum(
            result["metrics"]["annotated_role_mean_f1"] * result["metrics"]["scene_weight"]
            for result in results
        )
        / total_weight
        if total_weight > 0.0
        else 0.0
    )

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

    ranked_results = sorted(results, key=lambda item: item["metrics"]["weighted_shortfall"], reverse=True)
    summary = {
        "dataset": annotation_dir.name,
        "sample_count": len(results),
        "annotation_modes": modes,
        "mean_outer_f1": float(np.mean(outer_f1)) if outer_f1 else 0.0,
        "mean_inner_f1": float(np.mean(inner_f1)) if inner_f1 else 0.0,
        "mean_strict_role_f1": float(np.mean(strict_role_f1)) if strict_role_f1 else 0.0,
        "mean_annotated_role_f1": float(np.mean(annotated_role_f1)) if annotated_role_f1 else 0.0,
        "central_large_count": len(central_large_results),
        "mean_central_large_annotated_role_f1": (
            float(np.mean(central_large_annotated_role_f1)) if central_large_annotated_role_f1 else 0.0
        ),
        "weighted_mean_annotated_role_f1": weighted_annotated_role_f1,
        "central_large_weight": CENTRAL_LARGE_WEIGHT,
        "mean_outer_corner_error_px": float(np.mean(outer_corner_errors)) if outer_corner_errors else None,
        "mean_inner_corner_error_px": float(np.mean(inner_corner_errors)) if inner_corner_errors else None,
        "contact_sheet_paths": contact_sheet_paths,
        "threshold_mode": {
            "type": (
                "fixed-candidates"
                if fixed_threshold_candidates
                else "fixed"
                if fixed_threshold is not None
                else "sweep"
            ),
            "value": fixed_threshold_candidates if fixed_threshold_candidates else fixed_threshold,
        },
        "worst_15": ranked_results[:15],
        "results": results,
    }

    lines = [
        f"dataset: {annotation_dir.name}",
        f"sample_count: {len(results)}",
        f"annotation_modes: {modes}",
        f"mean_outer_f1: {summary['mean_outer_f1']:.4f}",
        f"mean_inner_f1: {summary['mean_inner_f1']:.4f}",
        f"mean_strict_role_f1: {summary['mean_strict_role_f1']:.4f}",
        f"mean_annotated_role_f1: {summary['mean_annotated_role_f1']:.4f}",
        f"central_large_count: {summary['central_large_count']}",
        f"mean_central_large_annotated_role_f1: {summary['mean_central_large_annotated_role_f1']:.4f}",
        f"weighted_mean_annotated_role_f1: {summary['weighted_mean_annotated_role_f1']:.4f}",
        (
            f"mean_outer_corner_error_px: {summary['mean_outer_corner_error_px']:.4f}"
            if summary["mean_outer_corner_error_px"] is not None
            else "mean_outer_corner_error_px: N/A"
        ),
        (
            f"mean_inner_corner_error_px: {summary['mean_inner_corner_error_px']:.4f}"
            if summary["mean_inner_corner_error_px"] is not None
            else "mean_inner_corner_error_px: N/A"
        ),
        (
            f"threshold_mode: fixed-candidates {fixed_threshold_candidates}"
            if fixed_threshold_candidates
            else f"threshold_mode: fixed {fixed_threshold}"
            if fixed_threshold is not None
            else "threshold_mode: sweep"
        ),
        f"contact_sheet_pages: {len(contact_sheet_paths)}",
    ]

    (output_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    (output_dir / "summary.txt").write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    args = parse_args()
    samples = build_samples(args.annotation_dir, max_samples=args.max_samples)
    args.output_dir.mkdir(parents=True, exist_ok=True)

    results = [
        evaluate_sample(
            sample,
            args.output_dir,
            fixed_threshold=args.fixed_threshold,
            fixed_threshold_candidates=args.fixed_threshold_candidates,
        )
        for sample in samples
    ]
    contact_sheet_paths = build_contact_sheets(results, args.output_dir / "contact_sheets", args.contact_cols, args.contact_rows)
    write_summary(
        results,
        args.output_dir,
        args.annotation_dir,
        args.fixed_threshold,
        args.fixed_threshold_candidates,
        contact_sheet_paths,
    )
    write_summary_csv(results, args.output_dir / "summary.csv")
    write_all_annotation_index(args.annotation_dir, args.output_dir)

    print(f"output_dir: {args.output_dir}")
    print(f"samples: {len(results)}")
    print(f"mean_annotated_role_f1: {np.mean([result['metrics']['annotated_role_mean_f1'] for result in results]):.4f}")


if __name__ == "__main__":
    main()
