"""Evaluate global perspective-pair recognition on the v30 preprocessor."""

from __future__ import annotations

import sys
from pathlib import Path

CURRENT_DIR = Path(__file__).resolve().parent
if str(CURRENT_DIR) not in sys.path:
    sys.path.insert(0, str(CURRENT_DIR))

import detect_minefield_people_geometry_v1 as detector
import evaluate_minefield_people_separable_edge_v1 as edge_eval


edge_eval.base_eval.DEFAULT_OUTPUT_DIR = detector.DATA_ROOT / "test/peoplev3_v39"
edge_eval.edge_detector = detector
for name in edge_eval.PATCHED_NAMES:
    setattr(edge_eval.base_eval, name, getattr(detector, name))


def main() -> None:
    edge_eval.main()


if __name__ == "__main__":
    main()
