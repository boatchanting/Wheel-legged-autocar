from __future__ import annotations

import sys
from pathlib import Path

CURRENT_DIR = Path(__file__).resolve().parent
if str(CURRENT_DIR) not in sys.path:
    sys.path.insert(0, str(CURRENT_DIR))

import detect_minefield_people_edge_json as edge_detector
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

base_eval.DEFAULT_ANNOTATION_DIR = edge_detector.DATA_ROOT / "frames/雷区peoplev3"
base_eval.DEFAULT_OUTPUT_DIR = edge_detector.DATA_ROOT / "test/peoplev3_v17"


def main() -> None:
    base_eval.main()


if __name__ == "__main__":
    main()
