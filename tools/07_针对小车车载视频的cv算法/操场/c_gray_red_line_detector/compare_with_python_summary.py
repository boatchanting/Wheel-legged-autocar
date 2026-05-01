"""Compare C gray-red line summary with Python summary.csv."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_PY_SUMMARY = PROJECT_ROOT / "data" / "runway_line_from11s_track_gray_red_brightcore" / "summary.csv"
DEFAULT_C_SUMMARY = PROJECT_ROOT / "data" / "line_gray_red_c_run" / "line_c_summary.json"
DEFAULT_OUTPUT = PROJECT_ROOT / "data" / "line_gray_red_c_run" / "compare_report.md"


def load_py_rows(path: Path) -> dict[int, dict]:
    rows: dict[int, dict] = {}
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            frame = int(row["frame_index"])
            rows[frame] = row
    return rows


def load_c_rows(path: Path) -> tuple[dict, dict[int, dict]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    rows = {int(row["frame"]): row for row in data["timeline"]}
    return data, rows


def to_bool(s: str | bool) -> bool:
    if isinstance(s, bool):
        return s
    return str(s).strip().lower() in {"1", "true", "yes"}


def compare(py_summary: Path, c_summary: Path, output: Path) -> dict:
    py_rows = load_py_rows(py_summary)
    c_data, c_rows = load_c_rows(c_summary)
    common = sorted(set(py_rows) & set(c_rows))

    detected_mismatch = []
    lookahead_diffs = []
    yaw_diffs = []
    err_diffs = []

    for frame in common:
        py = py_rows[frame]
        c = c_rows[frame]
        py_det = to_bool(py["detected"])
        c_det = to_bool(c["detected"])
        if py_det != c_det:
            detected_mismatch.append(frame)

        py_lk = float(py["lookahead_x"]) if py["lookahead_x"] else 0.0
        c_lk = float(c["line_x_lookahead"])
        lookahead_diffs.append(abs(py_lk - c_lk))

        py_yaw = float(py["yaw_deg"]) if py["yaw_deg"] else 0.0
        c_yaw = float(c["line_yaw_deg"])
        yaw_diffs.append(abs(py_yaw - c_yaw))

        py_err = float(py["lateral_error_px"]) if py["lateral_error_px"] else 0.0
        c_err = float(c["lateral_error_px"])
        err_diffs.append(abs(py_err - c_err))

    report = {
        "python_summary_csv": str(py_summary),
        "c_summary_json": str(c_summary),
        "common_frames": len(common),
        "py_detected_count": sum(1 for v in py_rows.values() if to_bool(v["detected"])),
        "c_detected_count": int(c_data["summary"]["detected_count"]),
        "detected_mismatch_count": len(detected_mismatch),
        "detected_mismatch_first20": detected_mismatch[:20],
        "lookahead_abs_diff_avg": sum(lookahead_diffs) / len(lookahead_diffs) if lookahead_diffs else 0.0,
        "lookahead_abs_diff_max": max(lookahead_diffs) if lookahead_diffs else 0.0,
        "yaw_abs_diff_avg": sum(yaw_diffs) / len(yaw_diffs) if yaw_diffs else 0.0,
        "yaw_abs_diff_max": max(yaw_diffs) if yaw_diffs else 0.0,
        "err_abs_diff_avg": sum(err_diffs) / len(err_diffs) if err_diffs else 0.0,
        "err_abs_diff_max": max(err_diffs) if err_diffs else 0.0,
        "c_timing": c_data["summary"].get("timing", {}),
    }

    lines = [
        "# Gray-Red Line C vs Python Compare",
        "",
        f"- Python summary: `{py_summary}`",
        f"- C summary: `{c_summary}`",
        f"- common_frames: `{report['common_frames']}`",
        f"- detected_count: Python `{report['py_detected_count']}`, C `{report['c_detected_count']}`",
        f"- detected_mismatch_count: `{report['detected_mismatch_count']}`",
        f"- lookahead_abs_diff_avg: `{report['lookahead_abs_diff_avg']:.4f}`",
        f"- lookahead_abs_diff_max: `{report['lookahead_abs_diff_max']:.4f}`",
        f"- yaw_abs_diff_avg: `{report['yaw_abs_diff_avg']:.4f}`",
        f"- yaw_abs_diff_max: `{report['yaw_abs_diff_max']:.4f}`",
        f"- err_abs_diff_avg: `{report['err_abs_diff_avg']:.4f}`",
        f"- err_abs_diff_max: `{report['err_abs_diff_max']:.4f}`",
        "",
        "## C Timing",
        "",
    ]
    for k, v in report["c_timing"].items():
        lines.append(f"- `{k}`: `{v}`")
    lines += ["", "## Mismatch Frames", "", f"- detected mismatch first20: `{report['detected_mismatch_first20']}`"]

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines), encoding="utf-8")
    output.with_suffix(".json").write_text(json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8")
    return report


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Compare C line detector with Python summary.")
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
