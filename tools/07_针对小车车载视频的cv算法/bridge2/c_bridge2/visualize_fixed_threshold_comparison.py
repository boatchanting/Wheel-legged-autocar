"""Render original-frame comparisons for adaptive versus fixed-threshold C results."""

from __future__ import annotations

import argparse
import csv
import math
import re
from pathlib import Path

from PIL import Image, ImageDraw


STATE_NAME = {0: "none", 1: "prepare_enter", 2: "on_bridge", 3: "prepare_exit"}
FRAME_NUMBER = re.compile(r"frame_?(\d+)")


def read_csv(path: Path) -> dict[str, dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        return {Path(row["frame"]).name: row for row in csv.DictReader(stream)}


def as_bool(value: str) -> bool:
    return value.strip().lower() == "true"


def as_int(value: str) -> int:
    return int(float(value or 0))


def as_float(value: str) -> float:
    return float(value or 0.0)


def parse_segment(value: str) -> tuple[int, int, int, int] | None:
    if not value:
        return None
    parts = value.split(";")
    if len(parts) != 4:
        return None
    return tuple(int(part) for part in parts)  # type: ignore[return-value]


def draw_cross(draw: ImageDraw.ImageDraw, x: int, y: int, color: tuple[int, int, int]) -> None:
    draw.line((x - 2, y, x + 2, y), fill=color, width=1)
    draw.line((x, y - 2, x, y + 2), fill=color, width=1)


def overlay_result(image: Image.Image, row: dict[str, str], fixed: bool) -> Image.Image:
    output = image.convert("RGB").copy()
    draw = ImageDraw.Draw(output)
    colors = ((255, 80, 40), (40, 190, 255), (0, 255, 80)) if not fixed else ((255, 190, 0), (205, 80, 255), (255, 255, 0))
    for field, color in zip(("left_line_segment", "right_line_segment", "center_line_segment"), colors):
        segment = parse_segment(row[field])
        if segment is not None:
            draw.line(segment, fill=color, width=1)
    if as_bool(row["bridge_found"]):
        draw_cross(draw, int(round(as_float(row["control_center_x"]))), as_int(row["bridge_bottom_row"]), colors[2])
    return output


def frame_number(name: str) -> int:
    match = FRAME_NUMBER.search(name)
    return int(match.group(1)) if match else 0


def render_comparison(original: Image.Image, adaptive: dict[str, str], fixed: dict[str, str], scale: int) -> Image.Image:
    original = original.convert("RGB")
    panels = [original, overlay_result(original, adaptive, False), overlay_result(original, fixed, True)]
    image_w, image_h = original.size
    panel_w, panel_h = image_w * scale, image_h * scale
    header_h, footer_h = 16, 27
    canvas = Image.new("RGB", (panel_w * 3, header_h + panel_h + footer_h), (20, 20, 20))
    draw = ImageDraw.Draw(canvas)
    labels = ["Original", "Adaptive (RGB lines)", "Fixed 225 (Y/M lines)"]
    for index, (panel, label) in enumerate(zip(panels, labels)):
        x = index * panel_w
        canvas.paste(panel.resize((panel_w, panel_h), Image.Resampling.NEAREST), (x, header_h))
        draw.text((x + 2, 2), label, fill=(235, 235, 235))
    area_delta = as_int(fixed["bridge_area"]) - as_int(adaptive["bridge_area"])
    center_delta = as_float(fixed["bridge_center_x"]) - as_float(adaptive["bridge_center_x"])
    y = header_h + panel_h + 2
    draw.text((2, y), f"A: found={adaptive['bridge_found']} state={STATE_NAME.get(as_int(adaptive['bridge_state_code']), 'bad')} thr={adaptive['threshold']} area={adaptive['bridge_area']} cx={as_float(adaptive['bridge_center_x']):.1f}", fill=(180, 240, 255))
    draw.text((panel_w + 2, y), f"F: found={fixed['bridge_found']} state={STATE_NAME.get(as_int(fixed['bridge_state_code']), 'bad')} thr={fixed['threshold']} area={fixed['bridge_area']} cx={as_float(fixed['bridge_center_x']):.1f}", fill=(255, 235, 130))
    draw.text((panel_w * 2 + 2, y), f"delta: area={area_delta:+d} cx={center_delta:+.1f}px", fill=(255, 180, 180))
    return canvas


def select_contact_frames(rows: list[dict[str, object]], count: int = 36) -> list[dict[str, object]]:
    selected: list[dict[str, object]] = []
    valid = [row for row in rows if row["adaptive_found"] == "True" and row["fixed_found"] == "True"]
    if not valid:
        return selected
    for index in range(12):
        selected.append(valid[round(index * (len(valid) - 1) / 11)])
    selected.extend(sorted(valid, key=lambda row: float(row["abs_area_delta"]), reverse=True)[:12])
    selected.extend(sorted(valid, key=lambda row: float(row["abs_center_delta"]), reverse=True)[:12])
    unique: dict[str, dict[str, object]] = {str(row["frame"]): row for row in selected}
    return sorted(unique.values(), key=lambda row: frame_number(str(row["frame"])))[:count]


def build_contact_sheet(tiles: list[Image.Image], columns: int = 4) -> Image.Image:
    rows = math.ceil(len(tiles) / columns)
    tile_w = max(tile.width for tile in tiles)
    tile_h = max(tile.height for tile in tiles)
    canvas = Image.new("RGB", (tile_w * columns, tile_h * rows), (12, 12, 12))
    for index, tile in enumerate(tiles):
        canvas.paste(tile, ((index % columns) * tile_w, (index // columns) * tile_h))
    return canvas


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--frames", type=Path, required=True)
    parser.add_argument("--adaptive", type=Path, required=True)
    parser.add_argument("--fixed", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    adaptive_rows = read_csv(args.adaptive)
    fixed_rows = read_csv(args.fixed)
    common = sorted(adaptive_rows.keys() & fixed_rows.keys(), key=frame_number)
    args.output.mkdir(parents=True, exist_ok=True)
    per_frame_dir = args.output / "per_frame"
    per_frame_dir.mkdir(exist_ok=True)
    comparisons: list[dict[str, object]] = []

    for frame in common:
        original_path = args.frames / frame
        if not original_path.is_file():
            continue
        adaptive = adaptive_rows[frame]
        fixed = fixed_rows[frame]
        with Image.open(original_path) as original:
            render_comparison(original, adaptive, fixed, 4).save(per_frame_dir / frame, format="PNG")
        comparisons.append(
            {
                "frame": frame,
                "adaptive_found": adaptive["bridge_found"],
                "fixed_found": fixed["bridge_found"],
                "adaptive_state": adaptive["bridge_state"],
                "fixed_state": fixed["bridge_state"],
                "adaptive_threshold": adaptive["threshold"],
                "fixed_threshold": fixed["threshold"],
                "adaptive_area": adaptive["bridge_area"],
                "fixed_area": fixed["bridge_area"],
                "area_delta": as_int(fixed["bridge_area"]) - as_int(adaptive["bridge_area"]),
                "abs_area_delta": abs(as_int(fixed["bridge_area"]) - as_int(adaptive["bridge_area"])),
                "adaptive_center_x": adaptive["bridge_center_x"],
                "fixed_center_x": fixed["bridge_center_x"],
                "center_delta_px": as_float(fixed["bridge_center_x"]) - as_float(adaptive["bridge_center_x"]),
                "abs_center_delta": abs(as_float(fixed["bridge_center_x"]) - as_float(adaptive["bridge_center_x"])),
            }
        )

    fieldnames = list(comparisons[0]) if comparisons else []
    with (args.output / "comparison.csv").open("w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(comparisons)

    selected = select_contact_frames(comparisons)
    tiles: list[Image.Image] = []
    for row in selected:
        adaptive = adaptive_rows[str(row["frame"])]
        fixed = fixed_rows[str(row["frame"])]
        with Image.open(args.frames / str(row["frame"])) as original:
            tiles.append(render_comparison(original, adaptive, fixed, 2))
    if tiles:
        build_contact_sheet(tiles).save(args.output / "representative_contact_sheet.png", format="PNG")

    found_mismatch = sum(row["adaptive_found"] != row["fixed_found"] for row in comparisons)
    state_mismatch = sum(row["adaptive_state"] != row["fixed_state"] for row in comparisons)
    valid = [row for row in comparisons if row["adaptive_found"] == "True" and row["fixed_found"] == "True"]
    area_mismatch = sum(row["area_delta"] != 0 for row in valid)
    center_values = [float(row["abs_center_delta"]) for row in valid]
    max_area = max(valid, key=lambda row: float(row["abs_area_delta"])) if valid else None
    max_center = max(valid, key=lambda row: float(row["abs_center_delta"])) if valid else None
    summary = [
        "# Adaptive versus Fixed 225 Comparison",
        "",
        f"- Frames: `{len(comparisons)}`",
        f"- bridge_found mismatches: `{found_mismatch}`",
        f"- state mismatches: `{state_mismatch}`",
        f"- valid bridge frames (both bridge_found): `{len(valid)}`",
        f"- area mismatches among valid bridge frames: `{area_mismatch}`",
        f"- centre MAE among valid bridge frames: `{sum(center_values) / len(center_values):.3f} px`",
        f"- centre max error among valid bridge frames: `{max(center_values):.3f} px`",
        f"- maximum area delta frame: `{max_area['frame'] if max_area else ''}`",
        f"- maximum centre delta frame: `{max_center['frame'] if max_center else ''}`",
        "",
        "`per_frame/` uses original | adaptive | fixed-225 panels.",
        "Adaptive: red/blue/green = left/right/centre; fixed: orange/purple/yellow = left/right/centre.",
    ]
    (args.output / "summary.md").write_text("\n".join(summary) + "\n", encoding="utf-8")
    print(f"wrote {len(comparisons)} frame panels to {args.output}")


if __name__ == "__main__":
    main()
