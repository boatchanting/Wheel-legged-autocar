"""Compare the desktop C bumpy-road detector with the Python timeline output."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_PY_SUMMARY = PROJECT_ROOT / "data" / "bumpy_road_line_detection_video" / "full_line_detection_timeline.json"
DEFAULT_C_SUMMARY = PROJECT_ROOT / "data" / "bumpy_road_c_run" / "bumpy_c_summary.json"
DEFAULT_OUTPUT = PROJECT_ROOT / "data" / "bumpy_road_c_run" / "compare_report.md"


def load_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def normalize_timeline(rows: list[dict]) -> dict[int, dict]:
    return {int(row["frame"]): row for row in rows}


def summary_block(payload: dict) -> dict:
    if "summary" in payload:
        return payload["summary"]
    return payload


def compare(py_summary_path: Path, c_summary_path: Path, output_path: Path) -> dict:
    py = load_json(py_summary_path)
    c = load_json(c_summary_path)
    py_summary = summary_block(py)
    c_summary = summary_block(c)

    py_rows = normalize_timeline(py["timeline"])
    c_rows = normalize_timeline(c["timeline"])
    common = sorted(set(py_rows) & set(c_rows))

    phase_mismatch: list[int] = []
    mode_mismatch: list[int] = []
    rib_count_mismatch: list[int] = []
    target_x_diffs: list[float] = []
    steer_diffs: list[float] = []
    white_threshold_diffs: list[float] = []

    for frame in common:
        py_row = py_rows[frame]
        c_row = c_rows[frame]

        if py_row["phase"] != c_row["phase"]:
            phase_mismatch.append(frame)
        if py_row["mode"] != c_row["mode"]:
            mode_mismatch.append(frame)
        if int(py_row["rib_count"]) != int(c_row["rib_count"]):
            rib_count_mismatch.append(frame)

        target_x_diffs.append(abs(float(py_row["target_x"]) - float(c_row["target_x"])))
        steer_diffs.append(abs(float(py_row["steer_error_px"]) - float(c_row["steer_error_px"])))
        white_threshold_diffs.append(abs(float(py_row["white_threshold"]) - float(c_row["white_threshold"])))

    report = {
        "python_summary": str(py_summary_path),
        "c_summary": str(c_summary_path),
        "common_frames": len(common),
        "py_frame_count": py_summary["frame_count"],
        "c_frame_count": c_summary["frame_count"],
        "py_phase_counts": py_summary["phase_counts"],
        "c_phase_counts": c_summary["phase_counts"],
        "py_first_inside_frame": py_summary["first_inside_frame"],
        "c_first_inside_frame": c_summary["first_inside_frame"],
        "py_first_exit_frame": py_summary["first_exit_frame"],
        "c_first_exit_frame": c_summary["first_exit_frame"],
        "phase_mismatch_count": len(phase_mismatch),
        "mode_mismatch_count": len(mode_mismatch),
        "rib_count_mismatch_count": len(rib_count_mismatch),
        "avg_target_x_abs_diff": sum(target_x_diffs) / len(target_x_diffs) if target_x_diffs else 0.0,
        "max_target_x_abs_diff": max(target_x_diffs) if target_x_diffs else 0.0,
        "avg_steer_abs_diff": sum(steer_diffs) / len(steer_diffs) if steer_diffs else 0.0,
        "max_steer_abs_diff": max(steer_diffs) if steer_diffs else 0.0,
        "avg_white_threshold_abs_diff": sum(white_threshold_diffs) / len(white_threshold_diffs) if white_threshold_diffs else 0.0,
        "max_white_threshold_abs_diff": max(white_threshold_diffs) if white_threshold_diffs else 0.0,
        "phase_mismatch_first20": phase_mismatch[:20],
        "mode_mismatch_first20": mode_mismatch[:20],
        "rib_count_mismatch_first20": rib_count_mismatch[:20],
        "c_timing": c_summary.get("timing", {}),
    }

    lines = [
        "# Bumpy C vs Python Compare",
        "",
        f"- Python summary: `{py_summary_path}`",
        f"- C summary: `{c_summary_path}`",
        f"- common_frames: `{report['common_frames']}`",
        f"- py_phase_counts: `{report['py_phase_counts']}`",
        f"- c_phase_counts: `{report['c_phase_counts']}`",
        f"- first_inside_frame: Python `{report['py_first_inside_frame']}`, C `{report['c_first_inside_frame']}`",
        f"- first_exit_frame: Python `{report['py_first_exit_frame']}`, C `{report['c_first_exit_frame']}`",
        f"- phase_mismatch_count: `{report['phase_mismatch_count']}`",
        f"- mode_mismatch_count: `{report['mode_mismatch_count']}`",
        f"- rib_count_mismatch_count: `{report['rib_count_mismatch_count']}`",
        f"- avg_target_x_abs_diff: `{report['avg_target_x_abs_diff']:.6f}`",
        f"- max_target_x_abs_diff: `{report['max_target_x_abs_diff']:.6f}`",
        f"- avg_steer_abs_diff: `{report['avg_steer_abs_diff']:.6f}`",
        f"- max_steer_abs_diff: `{report['max_steer_abs_diff']:.6f}`",
        f"- avg_white_threshold_abs_diff: `{report['avg_white_threshold_abs_diff']:.6f}`",
        f"- max_white_threshold_abs_diff: `{report['max_white_threshold_abs_diff']:.6f}`",
        "",
        "## C Timing",
        "",
    ]
    for key, value in report["c_timing"].items():
        lines.append(f"- `{key}`: `{value}`")

    lines.extend(
        [
            "",
            "## First Mismatches",
            "",
            f"- phase: `{report['phase_mismatch_first20']}`",
            f"- mode: `{report['mode_mismatch_first20']}`",
            f"- rib_count: `{report['rib_count_mismatch_first20']}`",
        ]
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(lines), encoding="utf-8")
    output_path.with_suffix(".json").write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")
    return report


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Compare C bumpy-road output with Python output.")
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
