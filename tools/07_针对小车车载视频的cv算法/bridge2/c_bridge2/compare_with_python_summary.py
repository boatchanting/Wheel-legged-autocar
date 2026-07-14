"""Compare the C detector CSV with single_bridge_prototype_with_middle.py output."""

from __future__ import annotations

import argparse
import ast
import csv
import json
import math
import re
from pathlib import Path


STATE_CODE = {
    "无": 0,
    "准备进入": 1,
    "在PVC上": 2,
    "准备退出": 3,
}
FRAME_RE = re.compile(r"frame_(\d+)")


def load_rows(path: Path) -> dict[int, dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        rows = list(csv.DictReader(stream))
    result: dict[int, dict[str, str]] = {}
    for row in rows:
        match = FRAME_RE.search(row["frame"])
        if match:
            result[int(match.group(1))] = row
    return result


def as_bool(value: str) -> bool:
    return value.strip().lower() == "true"


def python_segment(value: str) -> list[int] | None:
    return [int(item) for item in ast.literal_eval(value)] if value else None


def c_segment(value: str) -> list[int] | None:
    return [int(item) for item in value.split(";")] if value else None


def compare(python_csv: Path, c_csv: Path, timing_json: Path, output: Path) -> dict:
    py = load_rows(python_csv)
    c = load_rows(c_csv)
    common = sorted(py.keys() & c.keys())
    exact_fields = [
        ("threshold", "threshold"),
        ("pvc_area", "bridge_area"),
        ("pvc_top_row", "bridge_top_row"),
        ("pvc_start_row", "bridge_start_row"),
        ("pvc_bottom_row", "bridge_bottom_row"),
        ("pvc_max_width", "bridge_max_width"),
        ("pvc_bottom_width", "bridge_bottom_width"),
    ]
    float_fields = [
        ("pvc_area_ratio", "bridge_area_ratio"),
        ("pvc_center_x", "bridge_center_x"),
        ("edge_contrast", "edge_contrast"),
        ("left_clip_ratio", "left_clip_ratio"),
        ("right_clip_ratio", "right_clip_ratio"),
        ("dual_clip_ratio", "dual_clip_ratio"),
        ("border_monotonic", "border_monotonic"),
        ("candidate_score", "candidate_score"),
    ]
    mismatches: dict[str, list[int]] = {
        "bridge_found": [],
        "bridge_state": [],
        "left_line_visible": [],
        "right_line_visible": [],
        "top_line_visible": [],
        "entry_line_visible": [],
    }
    for _, c_name in exact_fields:
        mismatches[c_name] = []
    float_diff: dict[str, list[float]] = {c_name: [] for _, c_name in float_fields}

    for frame in common:
        p, q = py[frame], c[frame]
        if as_bool(p["pvc_found"]) != as_bool(q["bridge_found"]):
            mismatches["bridge_found"].append(frame)
        if STATE_CODE.get(p["pvc_state"], -1) != int(q["bridge_state_code"]):
            mismatches["bridge_state"].append(frame)
        visibility = [
            ("left_line_visible", "left_line_visible"),
            ("right_line_visible", "right_line_visible"),
            ("pink_line_visible", "top_line_visible"),
            ("yellow_line_visible", "entry_line_visible"),
        ]
        for py_name, c_name in visibility:
            key = c_name
            if as_bool(p[py_name]) != as_bool(q[c_name]):
                mismatches[key].append(frame)
        for py_name, c_name in exact_fields:
            if int(p[py_name]) != int(q[c_name]):
                mismatches[c_name].append(frame)
        for py_name, c_name in float_fields:
            left = float(p[py_name] or 0.0)
            right = float(q[c_name] or 0.0)
            float_diff[c_name].append(abs(left - right))

    timing = json.loads(timing_json.read_text(encoding="utf-8"))
    segment_diagnostics: dict[str, dict[str, float | int]] = {}
    for field in ("left_line_segment", "right_line_segment", "center_line_segment"):
        pairs = [(python_segment(py[frame][field]), c_segment(c[frame][field])) for frame in common]
        both = [(left, right) for left, right in pairs if left is not None and right is not None]
        errors = [abs(a - b) for left, right in both for a, b in zip(left, right)]
        segment_diagnostics[field] = {
            "presence_mismatch": sum((left is None) != (right is None) for left, right in pairs),
            "both_present": len(both),
            "exact_endpoint_match": sum(left == right for left, right in both),
            "coordinate_mae_px": sum(errors) / len(errors) if errors else 0.0,
            "coordinate_max_error_px": max(errors, default=0),
        }
    report = {
        "python_csv": str(python_csv),
        "c_csv": str(c_csv),
        "common_frames": len(common),
        "python_frames": len(py),
        "c_frames": len(c),
        "python_detected": sum(as_bool(row["pvc_found"]) for row in py.values()),
        "c_detected": sum(as_bool(row["bridge_found"]) for row in c.values()),
        "mismatch_counts": {key: len(value) for key, value in mismatches.items()},
        "first_mismatches": {key: value[:20] for key, value in mismatches.items() if value},
        "float_error": {
            key: {
                "mae": sum(values) / len(values) if values else 0.0,
                "max": max(values, default=0.0),
            }
            for key, values in float_diff.items()
        },
        "render_derived_segment_diagnostics": segment_diagnostics,
        "timing": timing,
    }
    lines = [
        "# Single Bridge Detection C vs Python",
        "",
        f"- Common frames: `{len(common)}`",
        f"- Detected count: Python `{report['python_detected']}`, C `{report['c_detected']}`",
        f"- `bridge_found` mismatches: `{len(mismatches['bridge_found'])}`",
        f"- state mismatches: `{len(mismatches['bridge_state'])}`",
        "",
        "## Exact-field mismatches",
        "",
    ]
    for key, count in report["mismatch_counts"].items():
        lines.append(f"- `{key}`: `{count}`")
    lines += ["", "## Float error", ""]
    for key, values in report["float_error"].items():
        lines.append(f"- `{key}`: MAE `{values['mae']:.6f}`, max `{values['max']:.6f}`")
    lines += [
        "",
        "## Render-derived segment diagnostics",
        "",
        "The C core intentionally omits the Python annotation-only endpoint correction rules. Segment presence is compared exactly; endpoint error is diagnostic rather than a parity gate.",
        "",
    ]
    for key, values in segment_diagnostics.items():
        lines.append(
            f"- `{key}`: presence mismatch `{values['presence_mismatch']}`, coordinate MAE `{values['coordinate_mae_px']:.3f}px`, max `{values['coordinate_max_error_px']}px`"
        )
    lines += ["", "## Detector-only timing", ""]
    for key, value in timing.items():
        lines.append(f"- `{key}`: `{value}`")
    if report["first_mismatches"]:
        lines += ["", "## First mismatch frames", ""]
        for key, values in report["first_mismatches"].items():
            lines.append(f"- `{key}`: `{values}`")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    output.with_suffix(".json").write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    return report


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--python-summary", type=Path, required=True)
    parser.add_argument("--c-summary", type=Path, required=True)
    parser.add_argument("--timing", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    report = compare(args.python_summary, args.c_summary, args.timing, args.output)
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
