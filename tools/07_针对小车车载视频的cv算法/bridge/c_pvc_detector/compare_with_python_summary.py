"""Compare C PVC detector output with the Python detector video summary."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_PY_SUMMARY = (
    PROJECT_ROOT
    / "data"
    / "bridge_white_pvc_detection_video_2026_04_17_21_18_39"
    / "video_summary.json"
)
DEFAULT_C_SUMMARY = PROJECT_ROOT / "data" / "bridge_white_pvc_c_run" / "pvc_c_summary.json"
DEFAULT_OUTPUT = PROJECT_ROOT / "data" / "bridge_white_pvc_c_run" / "compare_report.md"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def normalize_timeline(rows: list[dict]) -> dict[int, dict]:
    return {int(row["frame"]): row for row in rows}


def as_bbox(row: dict) -> tuple[int, int, int, int] | None:
    bbox = row.get("bbox")
    if bbox is None:
        return None
    return tuple(int(v) for v in bbox)


def compare(py_summary_path: Path, c_summary_path: Path, output_path: Path) -> dict:
    py = load_json(py_summary_path)
    c = load_json(c_summary_path)
    py_rows = normalize_timeline(py["timeline"])
    c_rows = normalize_timeline(c["timeline"])
    common = sorted(set(py_rows) & set(c_rows))

    detected_mismatch: list[int] = []
    score_diffs: list[float] = []
    bbox_mismatch: list[int] = []
    bottom_y_mismatch: list[int] = []
    area_mismatch: list[int] = []

    for frame in common:
        py_row = py_rows[frame]
        c_row = c_rows[frame]
        if bool(py_row["detected"]) != bool(c_row["detected"]):
            detected_mismatch.append(frame)

        score_diffs.append(abs(float(py_row["score"]) - float(c_row["score"])))

        if as_bbox(py_row) != as_bbox(c_row):
            bbox_mismatch.append(frame)
        if py_row.get("entry_bottom_y") != c_row.get("entry_bottom_y"):
            bottom_y_mismatch.append(frame)
        if int(py_row.get("area", 0)) != int(c_row.get("area", 0)):
            area_mismatch.append(frame)

    max_score_diff = max(score_diffs) if score_diffs else 0.0
    avg_score_diff = sum(score_diffs) / len(score_diffs) if score_diffs else 0.0

    report = {
        "py_summary": str(py_summary_path),
        "c_summary": str(c_summary_path),
        "common_frames": len(common),
        "py_frame_count": py["summary"]["frame_count"],
        "c_frame_count": c["summary"]["frame_count"],
        "py_detected_count": py["summary"]["detected_count"],
        "c_detected_count": c["summary"]["detected_count"],
        "py_first_detected": py["summary"]["first_detected_frame"],
        "c_first_detected": c["summary"]["first_detected_frame"],
        "py_last_detected": py["summary"]["last_detected_frame"],
        "c_last_detected": c["summary"]["last_detected_frame"],
        "detected_mismatch_count": len(detected_mismatch),
        "bbox_mismatch_count": len(bbox_mismatch),
        "bottom_y_mismatch_count": len(bottom_y_mismatch),
        "area_mismatch_count": len(area_mismatch),
        "avg_score_abs_diff": avg_score_diff,
        "max_score_abs_diff": max_score_diff,
        "detected_mismatch_first20": detected_mismatch[:20],
        "bbox_mismatch_first20": bbox_mismatch[:20],
        "bottom_y_mismatch_first20": bottom_y_mismatch[:20],
        "area_mismatch_first20": area_mismatch[:20],
        "c_timing": c["summary"].get("timing", {}),
    }

    lines = [
        "# PVC C vs Python Compare",
        "",
        f"- Python summary: `{py_summary_path}`",
        f"- C summary: `{c_summary_path}`",
        f"- common_frames: `{report['common_frames']}`",
        f"- detected_count: Python `{report['py_detected_count']}`, C `{report['c_detected_count']}`",
        f"- first_detected: Python `{report['py_first_detected']}`, C `{report['c_first_detected']}`",
        f"- last_detected: Python `{report['py_last_detected']}`, C `{report['c_last_detected']}`",
        f"- detected_mismatch_count: `{report['detected_mismatch_count']}`",
        f"- bbox_mismatch_count: `{report['bbox_mismatch_count']}`",
        f"- bottom_y_mismatch_count: `{report['bottom_y_mismatch_count']}`",
        f"- area_mismatch_count: `{report['area_mismatch_count']}`",
        f"- avg_score_abs_diff: `{avg_score_diff:.8f}`",
        f"- max_score_abs_diff: `{max_score_diff:.8f}`",
        "",
        "## C Timing",
        "",
    ]
    for key, value in report["c_timing"].items():
        lines.append(f"- `{key}`: `{value}`")

    lines += [
        "",
        "## First Mismatches",
        "",
        f"- detected: `{detected_mismatch[:20]}`",
        f"- bbox: `{bbox_mismatch[:20]}`",
        f"- bottom_y: `{bottom_y_mismatch[:20]}`",
        f"- area: `{area_mismatch[:20]}`",
    ]

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(lines), encoding="utf-8")
    (output_path.with_suffix(".json")).write_text(
        json.dumps(report, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    return report


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Compare C PVC detector summary with Python summary.")
    parser.add_argument("--python-summary", type=Path, default=DEFAULT_PY_SUMMARY)
    parser.add_argument("--c-summary", type=Path, default=DEFAULT_C_SUMMARY)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    report = compare(args.python_summary, args.c_summary, args.output)
    print(json.dumps(report, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
