"""Run adaptive-threshold white PVC detection on a full frame sequence.

This script keeps the same component filtering logic as the fixed-threshold
version, but computes a per-frame threshold from image statistics with
temporal smoothing.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

from detect_white_pvc_samples import (
    FRAME_DIR as DEFAULT_FRAME_DIR,
    MIN_DECISION_SCORE,
    filter_candidates,
    find_components,
)
from detect_white_pvc_video import DibAviWriter


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_VIDEO_OUTPUT_DIR = PROJECT_ROOT / "data/bridge_white_pvc_detection_video_adaptive"

FPS = 50
SCALE = 3


def _clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def estimate_threshold(
    gray: np.ndarray,
    prev_threshold: float | None,
    min_threshold: float,
    max_threshold: float,
    smooth_alpha: float,
) -> tuple[float, float]:
    """Estimate a white threshold for this frame.

    Returns (smoothed_threshold, raw_candidate_threshold).
    """
    pixels = gray.astype(np.float32).reshape(-1)
    mean = float(pixels.mean())
    std = float(pixels.std())
    p97 = float(np.percentile(pixels, 97.0))
    p995 = float(np.percentile(pixels, 99.5))

    # Tail model: focus on the bright-end histogram slope.
    tail_candidate = p97 + 0.55 * max(0.0, p995 - p97)
    # Sigma model: avoid dropping too low in overall darker frames.
    sigma_candidate = mean + 2.10 * std

    candidate = _clamp(max(tail_candidate, sigma_candidate), min_threshold, max_threshold)
    if prev_threshold is None:
        smoothed = candidate
    else:
        smoothed = smooth_alpha * prev_threshold + (1.0 - smooth_alpha) * candidate
        smoothed = _clamp(smoothed, min_threshold, max_threshold)

    return smoothed, candidate


def detect_white_pvc(
    rgb: np.ndarray,
    prev_threshold: float | None,
    min_threshold: float,
    max_threshold: float,
    smooth_alpha: float,
) -> dict:
    gray = np.asarray(Image.fromarray(rgb).convert("L"))
    threshold, threshold_candidate = estimate_threshold(
        gray,
        prev_threshold=prev_threshold,
        min_threshold=min_threshold,
        max_threshold=max_threshold,
        smooth_alpha=smooth_alpha,
    )
    threshold_int = int(round(threshold))

    raw_mask = gray >= threshold_int
    components = find_components(raw_mask, gray)
    candidates = filter_candidates(components, gray.size)
    best = candidates[0] if candidates else None
    detected = best is not None and best.score >= MIN_DECISION_SCORE

    return {
        "gray": gray,
        "raw_mask": raw_mask,
        "components": components,
        "best": best,
        "detected": detected,
        "threshold": threshold,
        "threshold_candidate": threshold_candidate,
        "threshold_int": threshold_int,
    }


def make_overlay(rgb: np.ndarray, frame_index: int, detection: dict) -> Image.Image:
    base = Image.fromarray(rgb, mode="RGB").resize(
        (rgb.shape[1] * SCALE, rgb.shape[0] * SCALE),
        Image.Resampling.NEAREST,
    )
    draw = ImageDraw.Draw(base)

    best = detection["best"]
    detected = detection["detected"]
    mask = detection["raw_mask"]

    alpha = Image.fromarray((mask.astype(np.uint8) * 90), mode="L").resize(base.size, Image.Resampling.NEAREST)
    red = Image.new("RGB", base.size, (255, 0, 0))
    base = Image.composite(red, base, alpha)
    draw = ImageDraw.Draw(base)

    for component in detection["components"][:4]:
        color = "yellow"
        width = 1
        if best is component and detected:
            color = "lime"
            width = 2
        draw.rectangle(
            [
                component.xmin * SCALE,
                component.ymin * SCALE,
                (component.xmax + 1) * SCALE - 1,
                (component.ymax + 1) * SCALE - 1,
            ],
            outline=color,
            width=width,
        )

    status = "PVC" if detected else "NO_PVC"
    score = best.score if best is not None else 0.0
    area = best.area if best is not None else 0
    bottom_y = best.ymax if best is not None else -1
    threshold_int = detection["threshold_int"]
    threshold_raw = detection["threshold"]
    threshold_candidate = detection["threshold_candidate"]

    draw.rectangle([0, 0, base.width - 1, 18], fill=(0, 0, 0))
    draw.text(
        (3, 3),
        (
            f"frame={frame_index:06d} {status} thr={threshold_int} "
            f"(raw={threshold_candidate:.1f},smooth={threshold_raw:.1f}) "
            f"score={score:.2f} area={area} bottom_y={bottom_y}"
        ),
        fill="white",
    )
    return base


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Detect white PVC with an adaptive threshold over a full frame sequence.")
    parser.add_argument("--frames", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_VIDEO_OUTPUT_DIR)
    parser.add_argument("--output-name", default="white_pvc_overlay_adaptive.avi")
    parser.add_argument("--min-threshold", type=float, default=235.0)
    parser.add_argument("--max-threshold", type=float, default=252.0)
    parser.add_argument("--smooth-alpha", type=float, default=0.82)
    parser.add_argument("--initial-threshold", type=float, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    frame_dir = args.frames
    output_dir = args.output_dir
    output_video = output_dir / args.output_name
    output_summary = output_dir / "video_summary_adaptive.txt"
    output_json = output_dir / "video_summary_adaptive.json"

    output_dir.mkdir(parents=True, exist_ok=True)
    frame_paths = sorted(frame_dir.glob("frame_*.png"))
    if not frame_paths:
        raise FileNotFoundError(f"no frames found in {frame_dir}")

    first = Image.open(frame_paths[0]).convert("RGB")
    out_width = first.width * SCALE
    out_height = first.height * SCALE

    detected_count = 0
    first_detected = None
    last_detected = None
    timeline: list[dict] = []
    thresholds: list[float] = []
    prev_threshold = args.initial_threshold

    with DibAviWriter(output_video, out_width, out_height, FPS) as writer:
        for idx, frame_path in enumerate(frame_paths, start=1):
            rgb = np.asarray(Image.open(frame_path).convert("RGB"))
            detection = detect_white_pvc(
                rgb,
                prev_threshold=prev_threshold,
                min_threshold=args.min_threshold,
                max_threshold=args.max_threshold,
                smooth_alpha=args.smooth_alpha,
            )
            prev_threshold = float(detection["threshold"])
            thresholds.append(prev_threshold)

            overlay = make_overlay(rgb, idx, detection)
            writer.write_frame(overlay)

            best = detection["best"]
            detected = detection["detected"]
            if detected:
                detected_count += 1
                if first_detected is None:
                    first_detected = idx
                last_detected = idx

            timeline.append(
                {
                    "frame": idx,
                    "detected": detected,
                    "score": round(best.score, 4) if best is not None else 0.0,
                    "area": best.area if best is not None else 0,
                    "bbox": [best.xmin, best.ymin, best.xmax, best.ymax] if best is not None else None,
                    "entry_bottom_y": best.ymax if best is not None else None,
                    "threshold": round(float(detection["threshold"]), 3),
                    "threshold_candidate": round(float(detection["threshold_candidate"]), 3),
                }
            )

    summary = {
        "frame_dir": str(frame_dir),
        "output_video": str(output_video),
        "fps": FPS,
        "scale": SCALE,
        "frame_count": len(frame_paths),
        "detected_count": detected_count,
        "first_detected_frame": first_detected,
        "last_detected_frame": last_detected,
        "decision_score": MIN_DECISION_SCORE,
        "adaptive_threshold": {
            "min_threshold": args.min_threshold,
            "max_threshold": args.max_threshold,
            "smooth_alpha": args.smooth_alpha,
            "initial_threshold": args.initial_threshold,
            "used_min": round(float(min(thresholds)), 3),
            "used_max": round(float(max(thresholds)), 3),
            "used_mean": round(float(np.mean(thresholds)), 3),
        },
    }

    output_json.write_text(
        json.dumps({"summary": summary, "timeline": timeline}, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    lines = [
        "Adaptive white PVC full-video detection summary",
        f"Input frame dir: {frame_dir}",
        f"Output video: {output_video}",
        f"Output size: {out_width}x{out_height}",
        f"FPS: {FPS}",
        f"Total frames: {len(frame_paths)}",
        f"Detected PVC frames: {detected_count}",
        f"First detected frame: {first_detected}",
        f"Last detected frame: {last_detected}",
        (
            "Adaptive threshold params: "
            f"min={args.min_threshold}, max={args.max_threshold}, "
            f"smooth_alpha={args.smooth_alpha}, initial={args.initial_threshold}"
        ),
        (
            "Adaptive threshold stats: "
            f"used_min={min(thresholds):.2f}, used_mean={np.mean(thresholds):.2f}, used_max={max(thresholds):.2f}"
        ),
        f"Decision rule: score >= {MIN_DECISION_SCORE}",
    ]
    output_summary.write_text("\n".join(lines), encoding="utf-8")

    print(json.dumps(summary, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
