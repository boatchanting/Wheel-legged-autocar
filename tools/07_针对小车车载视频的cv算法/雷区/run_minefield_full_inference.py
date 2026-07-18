from __future__ import annotations

import argparse
import csv
import json
import math
import sys
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw

CURRENT_DIR = Path(__file__).resolve().parent
if str(CURRENT_DIR) not in sys.path:
    sys.path.insert(0, str(CURRENT_DIR))

from detect_minefield_people_v2_json import (  # noqa: E402
    DATA_ROOT,
    SCALE,
    Candidate,
    Segment,
    add_title_to_scaled,
    detect_best_candidate,
    extract_prediction_segments,
    quad_to_list,
    read_gray,
    save_rgb,
)


DEFAULT_OUTPUT_ROOT = DATA_ROOT / "full_fixed128_runs"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run minefield inference on all frames in one or more frame directories."
    )
    parser.add_argument(
        "--frames-dir",
        type=Path,
        nargs="+",
        required=True,
        help="One or more frame directories that contain frame_*.png images.",
    )
    parser.add_argument(
        "--output-root",
        type=Path,
        default=DEFAULT_OUTPUT_ROOT,
        help="Root directory for batched outputs.",
    )
    parser.add_argument(
        "--fixed-threshold",
        type=int,
        default=128,
        help="Fixed grayscale threshold passed to the detector.",
    )
    parser.add_argument(
        "--fixed-threshold-candidates",
        type=int,
        nargs="+",
        default=None,
        help="Optional list of fixed thresholds. The detector keeps the highest-score candidate among them.",
    )
    parser.add_argument(
        "--contact-cols",
        type=int,
        default=3,
        help="Number of columns per contact sheet page.",
    )
    parser.add_argument(
        "--contact-rows",
        type=int,
        default=4,
        help="Number of rows per contact sheet page.",
    )
    parser.add_argument(
        "--max-frames",
        type=int,
        default=None,
        help="Optional cap for quick validation runs.",
    )
    return parser.parse_args()


def list_frame_paths(frames_dir: Path, max_frames: int | None = None) -> list[Path]:
    frame_paths = sorted(
        path
        for path in frames_dir.glob("*.png")
        if path.is_file() and "_frame" in path.stem
    )
    if max_frames is not None:
        return frame_paths[:max(0, max_frames)]
    return frame_paths


def segment_to_dict(segment: Segment) -> dict[str, int]:
    return {
        "x1": int(segment.x1),
        "y1": int(segment.y1),
        "x2": int(segment.x2),
        "y2": int(segment.y2),
    }


def make_binary_debug(
    threshold_mask: np.ndarray,
    pred_outer_segments: list[Segment],
    pred_inner_segments: list[Segment],
    title: str,
) -> np.ndarray:
    canvas = np.zeros((threshold_mask.shape[0], threshold_mask.shape[1], 3), dtype=np.uint8)
    canvas[threshold_mask] = (255, 255, 255)
    for seg in pred_outer_segments:
        cv2.line(canvas, (seg.x1, seg.y1), (seg.x2, seg.y2), (255, 0, 0), thickness=1, lineType=cv2.LINE_8)
    for seg in pred_inner_segments:
        cv2.line(canvas, (seg.x1, seg.y1), (seg.x2, seg.y2), (0, 255, 0), thickness=1, lineType=cv2.LINE_8)
    return add_title_to_scaled(canvas, title)


def make_prediction_label_image(
    gray: np.ndarray,
    pred_outer_segments: list[Segment],
    pred_inner_segments: list[Segment],
) -> np.ndarray:
    canvas = np.repeat(gray[:, :, None], 3, axis=2)
    for seg in pred_outer_segments:
        cv2.line(canvas, (seg.x1, seg.y1), (seg.x2, seg.y2), (255, 0, 0), thickness=1, lineType=cv2.LINE_8)
    for seg in pred_inner_segments:
        cv2.line(canvas, (seg.x1, seg.y1), (seg.x2, seg.y2), (0, 220, 0), thickness=1, lineType=cv2.LINE_8)
    return canvas


def make_labeler_payload(
    frame_path: Path,
    gray: np.ndarray,
    pred_outer_segments: list[Segment],
    pred_inner_segments: list[Segment],
) -> dict:
    return {
        "image_name": frame_path.name,
        "image_width": int(gray.shape[1]),
        "image_height": int(gray.shape[0]),
        "inner_segments": [segment_to_dict(segment) for segment in pred_inner_segments],
        "outer_segments": [segment_to_dict(segment) for segment in pred_outer_segments],
    }


def serialize_candidate(candidate: Candidate | None) -> dict | None:
    if candidate is None:
        return None
    return {
        "threshold": int(candidate.threshold),
        "score": round(float(candidate.score), 4),
        "far_view_factor": round(float(candidate.far_view_factor), 4),
        "inner_suppressed": bool(candidate.far_view_factor >= 0.45),
        "outer_area": round(float(candidate.outer_area), 2),
        "outer_bbox": [int(value) for value in candidate.outer_bbox],
        "child_area": round(float(candidate.child_area), 2),
        "touches_border": bool(candidate.touches_border),
        "outer_quad": quad_to_list(candidate.outer_quad),
        "inner_quad_direct": quad_to_list(candidate.inner_quad_direct),
        "inner_quad_final": quad_to_list(candidate.inner_quad_final),
        "geometry_score": round(float(candidate.geometry_score), 4),
        "ring_mean": round(float(candidate.ring_mean), 3),
        "inner_mean": round(float(candidate.inner_mean), 3),
        "ring_on_ratio": round(float(candidate.ring_on_ratio), 4),
        "inner_on_ratio": round(float(candidate.inner_on_ratio), 4),
        "outer_support": round(float(candidate.outer_support), 4),
        "inner_support": round(float(candidate.inner_support), 4),
    }


def frame_result(
    frame_path: Path,
    candidate: Candidate | None,
    pred_outer_segments: list[Segment],
    pred_inner_segments: list[Segment],
) -> dict:
    detection = serialize_candidate(candidate)
    if detection is not None:
        detection["pred_outer_segments"] = [segment_to_dict(segment) for segment in pred_outer_segments]
        detection["pred_inner_segments"] = [segment_to_dict(segment) for segment in pred_inner_segments]

    return {
        "frame_name": frame_path.name,
        "frame_stem": frame_path.stem,
        "frame_path": str(frame_path),
        "detected": candidate is not None,
        "pred_outer_segment_count": len(pred_outer_segments),
        "pred_inner_segment_count": len(pred_inner_segments),
        "detection": detection,
    }


def build_contact_sheets(
    records: list[dict],
    overlays_dir: Path,
    binary_dir: Path,
    output_dir: Path,
    cols: int,
    rows: int,
) -> list[str]:
    page_size = cols * rows
    output_dir.mkdir(parents=True, exist_ok=True)
    page_paths: list[str] = []
    for page_index, start in enumerate(range(0, len(records), page_size), start=1):
        page_records = records[start:start + page_size]
        tiles: list[Image.Image] = []
        for record in page_records:
            overlay = Image.open(overlays_dir / f"{record['frame_stem']}.png").convert("RGB")
            binary = Image.open(binary_dir / f"{record['frame_stem']}.png").convert("RGB")
            scaled_overlay = overlay.resize(
                (overlay.width * SCALE, overlay.height * SCALE),
                resample=Image.Resampling.NEAREST,
            )
            tile = Image.new("RGB", (scaled_overlay.width, scaled_overlay.height + binary.height), (18, 18, 18))
            tile.paste(scaled_overlay, (0, 0))
            tile.paste(binary, (0, scaled_overlay.height))
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


def write_summary_csv(records: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "frame_name",
        "detected",
        "pred_outer_segment_count",
        "pred_inner_segment_count",
        "threshold",
        "score",
        "outer_area",
        "child_area",
        "geometry_score",
        "outer_support",
        "inner_support",
    ]
    with output_path.open("w", encoding="utf-8-sig", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for record in records:
            detection = record["detection"] or {}
            writer.writerow(
                {
                    "frame_name": record["frame_name"],
                    "detected": record["detected"],
                    "pred_outer_segment_count": record["pred_outer_segment_count"],
                    "pred_inner_segment_count": record["pred_inner_segment_count"],
                    "threshold": detection.get("threshold"),
                    "score": detection.get("score"),
                    "outer_area": detection.get("outer_area"),
                    "child_area": detection.get("child_area"),
                    "geometry_score": detection.get("geometry_score"),
                    "outer_support": detection.get("outer_support"),
                    "inner_support": detection.get("inner_support"),
                }
            )


def process_directory(
    frames_dir: Path,
    output_root: Path,
    fixed_threshold: int,
    fixed_threshold_candidates: list[int] | None,
    contact_cols: int,
    contact_rows: int,
    max_frames: int | None,
) -> dict:
    frame_paths = list_frame_paths(frames_dir, max_frames=max_frames)
    run_dir = output_root / frames_dir.name
    overlays_dir = run_dir / "overlays"
    binary_dir = run_dir / "binary"
    json_dir = run_dir / "json"
    labeler_dir = run_dir / "labeler_ready"
    contact_dir = run_dir / "contact_sheets"

    overlays_dir.mkdir(parents=True, exist_ok=True)
    binary_dir.mkdir(parents=True, exist_ok=True)
    json_dir.mkdir(parents=True, exist_ok=True)
    labeler_dir.mkdir(parents=True, exist_ok=True)

    records: list[dict] = []
    for index, frame_path in enumerate(frame_paths, start=1):
        gray = read_gray(frame_path)
        candidate = detect_best_candidate(
            gray,
            fixed_threshold=fixed_threshold,
            fixed_thresholds=fixed_threshold_candidates,
        )

        if candidate is None:
            threshold_mask = np.zeros_like(gray, dtype=bool)
            score_text = "NA"
        else:
            threshold_mask = candidate.threshold_mask
            score_text = f"{candidate.score:.2f}"
        _, pred_outer_segments, pred_inner_segments = extract_prediction_segments(gray, candidate)

        overlay = make_prediction_label_image(gray, pred_outer_segments, pred_inner_segments)
        binary = make_binary_debug(
            threshold_mask,
            pred_outer_segments,
            pred_inner_segments,
            f"{frame_path.stem} binary pred=red/green",
        )
        labeler_payload = make_labeler_payload(frame_path, gray, pred_outer_segments, pred_inner_segments)

        save_rgb(overlays_dir / f"{frame_path.stem}.png", overlay)
        save_rgb(binary_dir / f"{frame_path.stem}.png", binary)
        save_rgb(labeler_dir / frame_path.name, overlay)
        (labeler_dir / f"{frame_path.stem}.json").write_text(
            json.dumps(labeler_payload, ensure_ascii=False, indent=2),
            encoding="utf-8",
        )

        record = frame_result(frame_path, candidate, pred_outer_segments, pred_inner_segments)
        record["artifacts"] = {
            "overlay": str(overlays_dir / f"{frame_path.stem}.png"),
            "binary": str(binary_dir / f"{frame_path.stem}.png"),
            "json": str(json_dir / f"{frame_path.stem}.json"),
            "labeler_png": str(labeler_dir / frame_path.name),
            "labeler_json": str(labeler_dir / f"{frame_path.stem}.json"),
        }
        record["labeler_annotation"] = labeler_payload
        (json_dir / f"{frame_path.stem}.json").write_text(
            json.dumps(record, ensure_ascii=False, indent=2),
            encoding="utf-8",
        )
        records.append(record)

        if index % 50 == 0 or index == len(frame_paths):
            print(f"[{frames_dir.name}] {index}/{len(frame_paths)}")

    contact_sheet_paths = build_contact_sheets(
        records,
        overlays_dir,
        binary_dir,
        contact_dir,
        cols=contact_cols,
        rows=contact_rows,
    )

    detected_records = [record for record in records if record["detected"]]
    scores = [
        float(record["detection"]["score"])
        for record in detected_records
        if record["detection"] and record["detection"].get("score") is not None
    ]
    summary = {
        "frames_dir": str(frames_dir),
        "frame_count": len(frame_paths),
        "detected_count": len(detected_records),
        "detected_ratio": (len(detected_records) / len(frame_paths)) if frame_paths else 0.0,
        "fixed_threshold": fixed_threshold,
        "fixed_threshold_candidates": fixed_threshold_candidates,
        "mean_detection_score": float(np.mean(scores)) if scores else None,
        "contact_sheet_page_count": len(contact_sheet_paths),
        "contact_sheet_paths": contact_sheet_paths,
        "artifacts": {
            "run_dir": str(run_dir),
            "overlays_dir": str(overlays_dir),
            "binary_dir": str(binary_dir),
            "json_dir": str(json_dir),
            "labeler_dir": str(labeler_dir),
            "summary_json": str(run_dir / "summary.json"),
            "summary_csv": str(run_dir / "summary.csv"),
        },
        "results": records,
    }

    (run_dir / "summary.json").write_text(
        json.dumps(summary, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    write_summary_csv(records, run_dir / "summary.csv")
    return summary


def main() -> None:
    args = parse_args()
    args.output_root.mkdir(parents=True, exist_ok=True)

    all_summaries = []
    for frames_dir in args.frames_dir:
        summary = process_directory(
            frames_dir=frames_dir,
            output_root=args.output_root,
            fixed_threshold=args.fixed_threshold,
            fixed_threshold_candidates=args.fixed_threshold_candidates,
            contact_cols=args.contact_cols,
            contact_rows=args.contact_rows,
            max_frames=args.max_frames,
        )
        all_summaries.append(summary)

    batch_summary = {
        "run_count": len(all_summaries),
        "fixed_threshold": args.fixed_threshold,
        "fixed_threshold_candidates": args.fixed_threshold_candidates,
        "output_root": str(args.output_root),
        "runs": all_summaries,
    }
    (args.output_root / "batch_summary.json").write_text(
        json.dumps(batch_summary, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    print(f"output_root: {args.output_root}")
    for summary in all_summaries:
        print(
            f"{Path(summary['frames_dir']).name}: "
            f"frames={summary['frame_count']} "
            f"detected={summary['detected_count']} "
            f"ratio={summary['detected_ratio']:.3f}"
        )


if __name__ == "__main__":
    main()
