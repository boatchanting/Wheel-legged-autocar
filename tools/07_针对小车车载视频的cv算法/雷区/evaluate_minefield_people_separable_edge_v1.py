"""Evaluate the separable small-kernel edge detector on peoplev3."""

from __future__ import annotations

import json
import sys
from pathlib import Path

import cv2
import numpy as np
from PIL import Image, ImageDraw

CURRENT_DIR = Path(__file__).resolve().parent
if str(CURRENT_DIR) not in sys.path:
    sys.path.insert(0, str(CURRENT_DIR))

import detect_minefield_people_separable_edge_v1 as edge_detector
import evaluate_minefield_people_v3 as base_eval


PATCHED_NAMES = (
    "DATA_ROOT",
    "SCALE",
    "detect_best_candidate",
    "extract_prediction_segments",
    "load_ground_truth",
    "make_comparison_overlay",
    "make_gt_overlay",
    "make_prediction_overlay",
    "make_threshold_debug",
    "mask_scores",
    "quad_corner_error",
    "quad_to_list",
    "read_gray",
    "read_rgb",
    "render_segments_mask",
    "save_gray",
    "save_mask",
    "save_rgb",
    "should_suppress_inner_segments",
)

for name in PATCHED_NAMES:
    setattr(base_eval, name, getattr(edge_detector, name))


def make_prediction_edge_debug(
    edge_mask: np.ndarray,
    pred_outer_segments,
    pred_inner_segments,
    title: str,
) -> np.ndarray:
    """Render the selected gradient response and algorithm-only line output."""

    canvas = np.zeros((*edge_mask.shape, 3), dtype=np.uint8)
    canvas[edge_mask] = (255, 255, 255)
    canvas = cv2.resize(
        canvas,
        (edge_mask.shape[1] * edge_detector.SCALE, edge_mask.shape[0] * edge_detector.SCALE),
        interpolation=cv2.INTER_NEAREST,
    )
    image = Image.fromarray(canvas, mode="RGB")
    draw = ImageDraw.Draw(image)
    for segment in pred_outer_segments:
        draw.line(
            (segment.x1 * edge_detector.SCALE, segment.y1 * edge_detector.SCALE,
             segment.x2 * edge_detector.SCALE, segment.y2 * edge_detector.SCALE),
            fill=(255, 0, 0), width=max(1, edge_detector.SCALE // 3),
        )
    for segment in pred_inner_segments:
        draw.line(
            (segment.x1 * edge_detector.SCALE, segment.y1 * edge_detector.SCALE,
             segment.x2 * edge_detector.SCALE, segment.y2 * edge_detector.SCALE),
            fill=(0, 255, 0), width=max(1, edge_detector.SCALE // 3),
        )
    draw.rectangle([0, 0, image.width - 1, 18], fill=(0, 0, 0))
    draw.text((3, 3), title.replace("binary", "gradient"), fill=(255, 255, 255))
    return np.asarray(image)


base_eval.make_prediction_binary_debug = make_prediction_edge_debug
base_eval.DEFAULT_ANNOTATION_DIR = edge_detector.DATA_ROOT / "frames/雷区peoplev3"
base_eval.DEFAULT_OUTPUT_DIR = edge_detector.DATA_ROOT / "test/peoplev3_v30"


def main() -> None:
    base_eval.main()
    output_dir = base_eval.DEFAULT_OUTPUT_DIR
    if "--output-dir" in sys.argv:
        output_dir = Path(sys.argv[sys.argv.index("--output-dir") + 1])
    summary_path = output_dir / "summary.json"
    if summary_path.exists():
        summary = json.loads(summary_path.read_text(encoding="utf-8"))
        summary["threshold_mode"] = {
            "type": "gradient-response-percentile",
            "value": {
                "kernel": "[1,3,3,1]^T * [-1,0,0,1] and transpose",
                "percentile": edge_detector.EDGE_PERCENTILE,
            },
        }
        summary_path.write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
        summary_text = output_dir / "summary.txt"
        if summary_text.exists():
            lines = summary_text.read_text(encoding="utf-8").splitlines()
            lines = [
                "threshold_mode: gradient-response-percentile (not intensity binarization)"
                if line.startswith("threshold_mode:")
                else line
                for line in lines
            ]
            summary_text.write_text("\n".join(lines), encoding="utf-8")


if __name__ == "__main__":
    main()
