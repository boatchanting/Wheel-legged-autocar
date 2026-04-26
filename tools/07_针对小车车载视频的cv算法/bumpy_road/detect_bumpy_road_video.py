"""Run bumpy-road detection over a full video frame sequence.

The detector reuses the sample-frame logic in detect_bumpy_road_samples.py.
It writes an annotated MJPG AVI and a per-frame timeline under data/.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw

from detect_bumpy_road_samples import (
    PROJECT_ROOT,
    ROI_X0,
    ROI_X1,
    ROI_Y0,
    ROI_Y1,
    classify_frame,
    detect_pvc,
    estimate_forward_mm_from_row,
    estimate_lateral_mm_from_x,
    find_default_frame_dir,
    find_rib_bands,
    score_bumpy,
    white_supported_dark_mask,
)


DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data/bumpy_road_detection_video"
DEFAULT_VIDEO = PROJECT_ROOT / "data/2026_04_17_21_44_42_Video颠簸.avi"

FPS = 50.0
SCALE = 4
MISSING_FRAME_TEXT = "NO_FRAME"


def frame_dir_from_video(video_path: Path) -> Path:
    frame_dir = PROJECT_ROOT / "data/frames" / video_path.stem
    if frame_dir.exists():
        return frame_dir

    # Windows console encoding can make non-ASCII path literals awkward, so
    # fall back to glob matching by timestamp prefix when the exact stem fails.
    prefix = video_path.stem.split("_Video")[0]
    matches = sorted((PROJECT_ROOT / "data/frames").glob(f"{prefix}*"))
    if matches:
        return matches[0]

    return find_default_frame_dir()


def open_writer(path: Path, width: int, height: int, fps: float) -> cv2.VideoWriter:
    path.parent.mkdir(parents=True, exist_ok=True)
    suffix = path.suffix.lower()
    if suffix == ".mp4":
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    else:
        fourcc = cv2.VideoWriter_fourcc(*"MJPG")
    writer = cv2.VideoWriter(str(path), fourcc, fps, (width, height))
    if not writer.isOpened():
        raise RuntimeError(f"failed to open video writer: {path}")
    return writer


def make_overlay(rgb: np.ndarray, frame_no: int, detection: dict) -> Image.Image:
    base = Image.fromarray(rgb, mode="RGB").resize(
        (rgb.shape[1] * SCALE, rgb.shape[0] * SCALE),
        Image.Resampling.NEAREST,
    )
    draw = ImageDraw.Draw(base)

    pvc = detection["best_pvc"]
    bands = detection["rib_bands"]
    classification = detection["classification"]
    score = detection["bumpy_score"]

    draw.rectangle(
        [ROI_X0 * SCALE, ROI_Y0 * SCALE, ROI_X1 * SCALE, ROI_Y1 * SCALE],
        outline="cyan",
        width=1,
    )

    if pvc is not None:
        color = "lime" if detection["pvc_detected"] else "yellow"
        draw.rectangle(
            [
                pvc.xmin * SCALE,
                pvc.ymin * SCALE,
                (pvc.xmax + 1) * SCALE - 1,
                (pvc.ymax + 1) * SCALE - 1,
            ],
            outline=color,
            width=2,
        )
        draw.ellipse(
            [
                pvc.centroid_x * SCALE - 3,
                pvc.centroid_y * SCALE - 3,
                pvc.centroid_x * SCALE + 3,
                pvc.centroid_y * SCALE + 3,
            ],
            fill="lime",
        )

    for idx, band in enumerate(bands[:5], start=1):
        draw.rectangle(
            [
                band.xmin * SCALE,
                band.ymin * SCALE,
                (band.xmax + 1) * SCALE - 1,
                (band.ymax + 1) * SCALE - 1,
            ],
            outline="red",
            width=2,
        )
        draw.text((band.xmin * SCALE + 2, band.ymin * SCALE + 1), str(idx), fill="red")

    first_forward = detection["first_rib_forward_mm"]
    first_forward_text = "--" if first_forward is None else f"{first_forward:.0f}mm"
    status_color = {
        "bumpy_visible": (40, 120, 40),
        "pvc_entry_or_clear": (80, 80, 80),
        "unknown": (120, 40, 40),
    }.get(classification, (80, 80, 80))

    draw.rectangle([0, 0, base.width - 1, 28], fill=(0, 0, 0))
    draw.rectangle([0, 28, base.width - 1, 34], fill=status_color)
    draw.text(
        (4, 4),
        (
            f"frame={frame_no:06d} {classification} "
            f"pvc={detection['pvc_detected']} ribs={len(bands)} "
            f"score={score:.2f} first={first_forward_text}"
        ),
        fill="white",
    )
    return base


def detect_frame(rgb: np.ndarray) -> dict:
    gray = np.asarray(Image.fromarray(rgb).convert("L"))
    white_mask, _components, best_pvc, pvc_detected = detect_pvc(gray)
    rib_mask, dark_threshold = white_supported_dark_mask(gray, white_mask)
    rib_bands = find_rib_bands(gray, rib_mask)
    bumpy_score = score_bumpy(pvc_detected, rib_bands)
    classification = classify_frame(pvc_detected, rib_bands, bumpy_score)

    first_band = rib_bands[0] if rib_bands else None
    return {
        "gray": gray,
        "best_pvc": best_pvc,
        "pvc_detected": pvc_detected,
        "rib_bands": rib_bands,
        "bumpy_score": bumpy_score,
        "classification": classification,
        "dark_threshold": dark_threshold,
        "first_rib_forward_mm": (
            estimate_forward_mm_from_row(first_band.center_y) if first_band is not None else None
        ),
    }


def timeline_entry(frame_no: int, detection: dict, frame_name: str) -> dict:
    pvc = detection["best_pvc"]
    bands = detection["rib_bands"]
    first_band = bands[0] if bands else None
    return {
        "frame": frame_no,
        "frame_name": frame_name,
        "classification": detection["classification"],
        "pvc_detected": detection["pvc_detected"],
        "bumpy_score": round(float(detection["bumpy_score"]), 4),
        "dark_threshold": round(float(detection["dark_threshold"]), 2),
        "rib_count": len(bands),
        "first_rib_forward_mm": (
            round(float(estimate_forward_mm_from_row(first_band.center_y)), 1)
            if first_band is not None
            else None
        ),
        "pvc_centroid": (
            [round(float(pvc.centroid_x), 2), round(float(pvc.centroid_y), 2)]
            if pvc is not None
            else None
        ),
        "pvc_lateral_mm": (
            round(float(estimate_lateral_mm_from_x(pvc.centroid_x)), 1)
            if pvc is not None
            else None
        ),
        "rib_bands": [
            {
                "bbox": [band.xmin, band.ymin, band.xmax, band.ymax],
                "center_y": round(float(band.center_y), 2),
                "approx_forward_mm": round(float(estimate_forward_mm_from_row(band.center_y)), 1),
            }
            for band in bands
        ],
    }


def summarize_timeline(timeline: list[dict]) -> dict:
    classifications: dict[str, int] = {}
    for row in timeline:
        classifications[row["classification"]] = classifications.get(row["classification"], 0) + 1

    bumpy_frames = [row["frame"] for row in timeline if row["classification"] == "bumpy_visible"]
    pvc_frames = [row["frame"] for row in timeline if row["pvc_detected"]]
    return {
        "frame_count": len(timeline),
        "classification_counts": classifications,
        "first_pvc_frame": pvc_frames[0] if pvc_frames else None,
        "last_pvc_frame": pvc_frames[-1] if pvc_frames else None,
        "first_bumpy_frame": bumpy_frames[0] if bumpy_frames else None,
        "last_bumpy_frame": bumpy_frames[-1] if bumpy_frames else None,
    }


def write_outputs(
    output_dir: Path,
    video_path: Path,
    frame_dir: Path,
    output_video: Path,
    timeline: list[dict],
) -> None:
    summary = summarize_timeline(timeline)
    payload = {
        "video": str(video_path),
        "frame_dir": str(frame_dir),
        "output_video": str(output_video),
        **summary,
        "timeline": timeline,
    }
    (output_dir / "timeline.json").write_text(
        json.dumps(payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    lines: list[str] = []
    lines.append("# Bumpy Road Video Detection")
    lines.append("")
    lines.append(f"video: `{video_path}`")
    lines.append(f"frame_dir: `{frame_dir}`")
    lines.append(f"output_video: `{output_video}`")
    lines.append("")
    lines.append(f"frame_count: `{summary['frame_count']}`")
    lines.append(f"first_pvc_frame: `{summary['first_pvc_frame']}`")
    lines.append(f"first_bumpy_frame: `{summary['first_bumpy_frame']}`")
    lines.append(f"last_bumpy_frame: `{summary['last_bumpy_frame']}`")
    lines.append("")
    lines.append("classification_counts:")
    for key, value in summary["classification_counts"].items():
        lines.append(f"- `{key}`: `{value}`")
    lines.append("")
    lines.append("control note:")
    lines.append("Use continuous 3 to 5 `bumpy_visible` frames before triggering the bumpy-road state machine.")
    lines.append("Use `pvc_entry_or_clear` with no ribs as the current entrance/exit candidate.")

    (output_dir / "summary.md").write_text("\n".join(lines), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Export annotated bumpy-road detection video.")
    parser.add_argument("--video", type=Path, default=DEFAULT_VIDEO)
    parser.add_argument("--frames", type=Path, default=None)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--output-name", default=None)
    parser.add_argument("--max-frames", type=int, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    video_path = args.video
    frame_dir = args.frames or frame_dir_from_video(video_path)
    output_dir = args.output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    frame_paths = sorted(frame_dir.glob("frame_*.png"))
    if not frame_paths:
        raise FileNotFoundError(f"no frame_*.png files found in {frame_dir}")
    if args.max_frames is not None:
        frame_paths = frame_paths[: args.max_frames]

    first_rgb = np.asarray(Image.open(frame_paths[0]).convert("RGB"))
    out_size = (first_rgb.shape[1] * SCALE, first_rgb.shape[0] * SCALE)
    output_name = args.output_name or f"{video_path.stem}_bumpy_overlay.avi"
    output_video = output_dir / output_name
    writer = open_writer(output_video, out_size[0], out_size[1], FPS)

    timeline: list[dict] = []
    try:
        for frame_no, frame_path in enumerate(frame_paths, start=1):
            rgb = np.asarray(Image.open(frame_path).convert("RGB"))
            detection = detect_frame(rgb)
            overlay = make_overlay(rgb, frame_no, detection)
            bgr = cv2.cvtColor(np.asarray(overlay), cv2.COLOR_RGB2BGR)
            writer.write(bgr)
            timeline.append(timeline_entry(frame_no, detection, frame_path.name))
    finally:
        writer.release()

    write_outputs(output_dir, video_path, frame_dir, output_video, timeline)
    summary = summarize_timeline(timeline)
    print(f"video: {video_path}")
    print(f"frame_dir: {frame_dir}")
    print(f"output_video: {output_video}")
    print(f"frame_count: {summary['frame_count']}")
    print(f"classification_counts: {summary['classification_counts']}")
    print(f"first_bumpy_frame: {summary['first_bumpy_frame']}")
    print(f"last_bumpy_frame: {summary['last_bumpy_frame']}")


if __name__ == "__main__":
    main()
