"""Create visual contact sheets and threshold records for C detector masks."""

from __future__ import annotations

import argparse
import csv
import math
import re
from pathlib import Path

from PIL import Image, ImageDraw


FRAME_NUMBER = re.compile(r"frame_?(\d+)")
STATE_NAME = {0: "none", 1: "prepare_enter", 2: "on_bridge", 3: "prepare_exit"}


def frame_number(name: str) -> int:
    match = FRAME_NUMBER.search(name)
    return int(match.group(1)) if match else 0


def load_rows(path: Path) -> dict[str, dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        return {Path(row["frame"]).name: row for row in csv.DictReader(stream)}


def mask_path(mask_dir: Path, frame: str, suffix: str) -> Path:
    return mask_dir / f"{Path(frame).stem}_{suffix}.pgm"


def render_tile(original: Image.Image, visible: Image.Image, outer: Image.Image,
                row: dict[str, str], scale: int) -> Image.Image:
    panels = [original.convert("RGB"), visible.convert("RGB"), outer.convert("RGB")]
    panel_w, panel_h = original.width * scale, original.height * scale
    header_h, footer_h = 16, 16
    canvas = Image.new("RGB", (panel_w * 3, header_h + panel_h + footer_h), (18, 18, 18))
    draw = ImageDraw.Draw(canvas)
    for index, (panel, label) in enumerate(zip(panels, ("Original", "Visible mask", "Outer hull mask"))):
        x = index * panel_w
        canvas.paste(panel.resize((panel_w, panel_h), Image.Resampling.NEAREST), (x, header_h))
        draw.text((x + 2, 2), label, fill=(240, 240, 240))
    found = row["bridge_found"]
    state = STATE_NAME.get(int(row["bridge_state_code"]), "bad")
    draw.text((2, header_h + panel_h + 2),
              f"thr={row['threshold']} found={found} state={state} area={row['bridge_area']} score={float(row['candidate_score']):.1f}",
              fill=(255, 230, 120))
    return canvas


def contact_sheet(tiles: list[Image.Image], columns: int = 4) -> Image.Image:
    tile_w = max(tile.width for tile in tiles)
    tile_h = max(tile.height for tile in tiles)
    canvas = Image.new("RGB", (tile_w * columns, tile_h * math.ceil(len(tiles) / columns)), (10, 10, 10))
    for index, tile in enumerate(tiles):
        canvas.paste(tile, ((index % columns) * tile_w, (index // columns) * tile_h))
    return canvas


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--frames", type=Path, required=True)
    parser.add_argument("--summary", type=Path, required=True)
    parser.add_argument("--mask-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    rows = load_rows(args.summary)
    ordered = sorted(rows, key=frame_number)
    args.output.mkdir(parents=True, exist_ok=True)

    threshold_fields = ["frame", "threshold", "bridge_found", "bridge_state", "bridge_area", "bridge_center_x", "candidate_score"]
    with (args.output / "selected_thresholds.csv").open("w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=threshold_fields)
        writer.writeheader()
        writer.writerows({field: rows[frame][field] for field in threshold_fields} for frame in ordered)

    selected: list[str] = []
    for index in range(24):
        selected.append(ordered[round(index * (len(ordered) - 1) / 23)])
    for frame in ordered[1:]:
        if rows[frame]["threshold"] != rows[ordered[ordered.index(frame) - 1]]["threshold"]:
            selected.append(frame)
    unique: list[str] = []
    for frame in selected:
        if frame not in unique:
            unique.append(frame)
    unique = sorted(unique[:48], key=frame_number)
    tiles: list[Image.Image] = []
    for frame in unique:
        with Image.open(args.frames / frame) as original, \
             Image.open(mask_path(args.mask_dir, frame, "visible")) as visible, \
             Image.open(mask_path(args.mask_dir, frame, "outer")) as outer:
            tiles.append(render_tile(original, visible, outer, rows[frame], 2))
    contact_sheet(tiles).save(args.output / "selected_mask_contact_sheet.png", format="PNG")
    (args.output / "README.md").write_text(
        "# C detector selected masks\n\n"
        "- `visible`: selected component after local close/open.\n"
        "- `outer`: convex-hull mask used for candidate geometry and scoring.\n"
        "- `selected_thresholds.csv`: final winning threshold for every frame in adaptive mode.\n",
        encoding="utf-8",
    )
    print(f"wrote {len(ordered)} threshold rows and {len(tiles)} mask tiles to {args.output}")


if __name__ == "__main__":
    main()
