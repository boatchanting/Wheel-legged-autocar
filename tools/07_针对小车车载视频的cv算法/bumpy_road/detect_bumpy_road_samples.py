"""Detect bumpy-road PVC entrance and transverse ribs in sample frames.

This is a first-pass detector for 96x60 on-car frames. It follows the
visual-fusion plan used by subject three:

- inertial navigation opens this detector only near the expected bumpy point
- vision confirms white PVC entrance / visible bumpy ribs
- the bumpy-road controller still owns speed, heading lock, and stall handling

Outputs are written under data/bumpy_road_detection_samples by default.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import matplotlib.pyplot as plt
import numpy as np
from PIL import Image, ImageDraw


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data/bumpy_road_detection_samples"
DEFAULT_FRAME_GLOB = "2026_04_17_21_44_42*"


@dataclass(frozen=True)
class Sample:
    frame: str
    label: str
    expected: str


@dataclass
class Component:
    area: int
    xmin: int
    ymin: int
    xmax: int
    ymax: int
    centroid_x: float
    centroid_y: float
    fill_ratio: float
    touches_border: bool
    mean_gray: float
    score: float = 0.0

    @property
    def width(self) -> int:
        return self.xmax - self.xmin + 1

    @property
    def height(self) -> int:
        return self.ymax - self.ymin + 1


@dataclass
class RibBand:
    ymin: int
    ymax: int
    xmin: int
    xmax: int
    area: int
    max_row_pixels: int
    mean_gray: float

    @property
    def width(self) -> int:
        return self.xmax - self.xmin + 1

    @property
    def height(self) -> int:
        return self.ymax - self.ymin + 1

    @property
    def center_y(self) -> float:
        return 0.5 * (self.ymin + self.ymax)


DEFAULT_SAMPLES: tuple[Sample, ...] = (
    Sample("frame_000206.png", "entry white PVC, before ribs", "entry"),
    Sample("frame_000401.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_000465.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_000501.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_000873.png", "far/partial rib near top", "bumpy"),
    Sample("frame_000591.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_000681.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_000723.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_000795.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_000812.png", "far/partial rib near top", "bumpy"),
    Sample("frame_002012.png", "white PVC / no rib visible", "entry"),
    Sample("frame_001197.png", "far/partial rib near top", "bumpy"),
    Sample("frame_001251.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_001340.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_001382.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_001449.png", "representative bumpy ribs", "bumpy"),
    Sample("frame_001545.png", "close multiple ribs", "bumpy"),
    Sample("frame_001617.png", "close multiple ribs", "bumpy"),
    Sample("frame_001839.png", "close multiple ribs", "bumpy"),
    Sample("frame_001917.png", "user note: no bumpy road", "no_bumpy"),
)


WHITE_THRESHOLD = 235
MIN_WHITE_AREA = 600
MIN_PVC_SCORE = 0.48

ROI_X0 = 2
ROI_X1 = 94
ROI_Y0 = 6
ROI_Y1 = 58

MIN_RIB_ROW_PIXELS = 20
MIN_RIB_WIDTH = 24
MIN_RIB_HEIGHT = 2
MIN_BUMPY_SCORE = 0.43


def find_default_frame_dir() -> Path:
    frames_root = PROJECT_ROOT / "data/frames"
    matches = sorted(frames_root.glob(DEFAULT_FRAME_GLOB))
    if not matches:
        raise FileNotFoundError(f"no frame directory matches {frames_root / DEFAULT_FRAME_GLOB}")
    return matches[0]


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def save_mask(mask: np.ndarray, path: Path) -> None:
    Image.fromarray(mask.astype(np.uint8) * 255, mode="L").save(path)


def find_components(mask: np.ndarray, gray: np.ndarray) -> list[Component]:
    h, w = mask.shape
    visited = np.zeros_like(mask, dtype=bool)
    components: list[Component] = []

    for y0 in range(h):
        for x0 in range(w):
            if visited[y0, x0] or not mask[y0, x0]:
                continue

            stack = [(y0, x0)]
            visited[y0, x0] = True
            xs: list[int] = []
            ys: list[int] = []

            while stack:
                y, x = stack.pop()
                xs.append(x)
                ys.append(y)
                for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    ny = y + dy
                    nx = x + dx
                    if (
                        0 <= ny < h
                        and 0 <= nx < w
                        and mask[ny, nx]
                        and not visited[ny, nx]
                    ):
                        visited[ny, nx] = True
                        stack.append((ny, nx))

            xs_arr = np.asarray(xs)
            ys_arr = np.asarray(ys)
            xmin = int(xs_arr.min())
            xmax = int(xs_arr.max())
            ymin = int(ys_arr.min())
            ymax = int(ys_arr.max())
            area = int(xs_arr.size)
            bbox_area = max(1, (xmax - xmin + 1) * (ymax - ymin + 1))
            touches_border = xmin == 0 or ymin == 0 or xmax == (w - 1) or ymax == (h - 1)
            mean_gray = float(gray[ys_arr, xs_arr].mean())

            components.append(
                Component(
                    area=area,
                    xmin=xmin,
                    ymin=ymin,
                    xmax=xmax,
                    ymax=ymax,
                    centroid_x=float(xs_arr.mean()),
                    centroid_y=float(ys_arr.mean()),
                    fill_ratio=float(area / bbox_area),
                    touches_border=touches_border,
                    mean_gray=mean_gray,
                )
            )

    return sorted(components, key=lambda c: c.area, reverse=True)


def score_pvc_component(component: Component) -> float:
    area_score = min(component.area / 2200.0, 1.0)
    width_score = min(component.width / 70.0, 1.0)
    height_score = min(component.height / 35.0, 1.0)
    fill_score = min(component.fill_ratio / 0.65, 1.0)
    brightness_score = min(max((component.mean_gray - 220.0) / 35.0, 0.0), 1.0)
    border_score = 1.0 if component.touches_border else 0.0

    return (
        0.34 * area_score
        + 0.18 * width_score
        + 0.12 * height_score
        + 0.12 * fill_score
        + 0.14 * brightness_score
        + 0.10 * border_score
    )


def detect_pvc(gray: np.ndarray) -> tuple[np.ndarray, list[Component], Component | None, bool]:
    white_mask = gray >= WHITE_THRESHOLD
    components = find_components(white_mask, gray)
    candidates: list[Component] = []

    for component in components:
        component.score = score_pvc_component(component)
        if component.area < MIN_WHITE_AREA:
            continue
        if component.width < 25 or component.height < 6:
            continue
        candidates.append(component)

    candidates = sorted(candidates, key=lambda c: c.score, reverse=True)
    best = candidates[0] if candidates else None
    detected = best is not None and best.score >= MIN_PVC_SCORE
    return white_mask, components, best, detected


def adaptive_dark_threshold(gray: np.ndarray) -> float:
    roi = gray[ROI_Y0:ROI_Y1, ROI_X0:ROI_X1]
    p50 = float(np.percentile(roi, 50))
    return min(170.0, max(120.0, p50 - 25.0))


def white_supported_dark_mask(gray: np.ndarray, white_mask: np.ndarray) -> tuple[np.ndarray, float]:
    dark_threshold = adaptive_dark_threshold(gray)
    dark_mask = gray <= dark_threshold

    # A bumpy rib is a dark horizontal strip embedded in white PVC, so pixels
    # should have white support both above and below in the same column.
    support = np.zeros_like(dark_mask, dtype=bool)
    for offset in range(2, 8):
        above = np.zeros_like(white_mask, dtype=bool)
        below = np.zeros_like(white_mask, dtype=bool)
        above[offset:, :] = white_mask[:-offset, :]
        below[:-offset, :] = white_mask[offset:, :]
        support |= above & below

    return dark_mask & support, dark_threshold


def group_rows(rows: np.ndarray) -> list[tuple[int, int]]:
    if rows.size == 0:
        return []

    groups: list[tuple[int, int]] = []
    start = prev = int(rows[0])
    for raw_y in rows[1:]:
        y = int(raw_y)
        if y <= prev + 1:
            prev = y
        else:
            groups.append((start, prev))
            start = prev = y
    groups.append((start, prev))
    return groups


def find_rib_bands(gray: np.ndarray, rib_mask: np.ndarray) -> list[RibBand]:
    roi = rib_mask[ROI_Y0:ROI_Y1, ROI_X0:ROI_X1]
    row_pixels = roi.sum(axis=1)
    rows = np.where(row_pixels >= MIN_RIB_ROW_PIXELS)[0] + ROI_Y0
    bands: list[RibBand] = []

    for ymin, ymax in group_rows(rows):
        if ymax - ymin + 1 < MIN_RIB_HEIGHT:
            continue

        band_mask = rib_mask[ymin : ymax + 1, ROI_X0:ROI_X1]
        col_pixels = band_mask.sum(axis=0)
        cols = np.where(col_pixels > 0)[0]
        if cols.size == 0:
            continue

        xmin = int(cols.min()) + ROI_X0
        xmax = int(cols.max()) + ROI_X0
        area = int(band_mask.sum())
        width = xmax - xmin + 1
        if width < MIN_RIB_WIDTH:
            continue

        ys, xs = np.where(rib_mask[ymin : ymax + 1, xmin : xmax + 1])
        if ys.size == 0:
            mean_gray = 0.0
        else:
            mean_gray = float(gray[ymin : ymax + 1, xmin : xmax + 1][ys, xs].mean())

        bands.append(
            RibBand(
                ymin=ymin,
                ymax=ymax,
                xmin=xmin,
                xmax=xmax,
                area=area,
                max_row_pixels=int(row_pixels[(ymin - ROI_Y0) : (ymax - ROI_Y0 + 1)].max()),
                mean_gray=mean_gray,
            )
        )

    return sorted(bands, key=lambda b: b.center_y)


def estimate_forward_mm_from_row(y: float) -> float:
    # Placeholder until a measured row->distance table is calibrated.
    # The value is only for sorting/debug, not for closed-loop control yet.
    return max(0.0, (59.0 - y) * 20.0)


def estimate_lateral_mm_from_x(x: float) -> float:
    # Placeholder scale for 96 px image. Replace with the calibrated table.
    return (x - 47.5) * 8.0


def score_bumpy(pvc_detected: bool, bands: list[RibBand]) -> float:
    if not pvc_detected:
        return 0.0

    band_count_score = min(len(bands) / 3.0, 1.0)
    width_score = 0.0
    if bands:
        width_score = min(max(b.width for b in bands) / 70.0, 1.0)
    lower_band_score = 0.0
    if any(b.center_y >= 18.0 for b in bands):
        lower_band_score = 1.0

    return 0.50 * band_count_score + 0.30 * width_score + 0.20 * lower_band_score


def classify_frame(pvc_detected: bool, bands: list[RibBand], bumpy_score: float) -> str:
    if pvc_detected and bumpy_score >= MIN_BUMPY_SCORE:
        return "bumpy_visible"
    if pvc_detected:
        return "pvc_entry_or_clear"
    return "unknown"


def draw_overlay(
    rgb: np.ndarray,
    sample: Sample,
    best_pvc: Component | None,
    pvc_detected: bool,
    bands: list[RibBand],
    classification: str,
    bumpy_score: float,
) -> Image.Image:
    scale = 6
    image = Image.fromarray(rgb, mode="RGB").resize(
        (rgb.shape[1] * scale, rgb.shape[0] * scale),
        Image.Resampling.NEAREST,
    )
    draw = ImageDraw.Draw(image)

    draw.rectangle(
        [ROI_X0 * scale, ROI_Y0 * scale, ROI_X1 * scale, ROI_Y1 * scale],
        outline="cyan",
        width=1,
    )

    if best_pvc is not None:
        color = "lime" if pvc_detected else "yellow"
        draw.rectangle(
            [
                best_pvc.xmin * scale,
                best_pvc.ymin * scale,
                (best_pvc.xmax + 1) * scale - 1,
                (best_pvc.ymax + 1) * scale - 1,
            ],
            outline=color,
            width=2,
        )
        draw.ellipse(
            [
                best_pvc.centroid_x * scale - 4,
                best_pvc.centroid_y * scale - 4,
                best_pvc.centroid_x * scale + 4,
                best_pvc.centroid_y * scale + 4,
            ],
            fill="lime",
        )

    for idx, band in enumerate(bands, start=1):
        draw.rectangle(
            [
                band.xmin * scale,
                band.ymin * scale,
                (band.xmax + 1) * scale - 1,
                (band.ymax + 1) * scale - 1,
            ],
            outline="red",
            width=2,
        )
        draw.text((band.xmin * scale + 2, band.ymin * scale + 1), str(idx), fill="red")

    title = (
        f"{sample.frame} | {classification} | pvc={pvc_detected} "
        f"ribs={len(bands)} score={bumpy_score:.2f}"
    )
    draw.rectangle([0, 0, image.width - 1, 22], fill=(0, 0, 0))
    draw.text((4, 4), title, fill="white")
    return image


def save_profiles(
    gray: np.ndarray,
    white_mask: np.ndarray,
    rib_mask: np.ndarray,
    bands: list[RibBand],
    path: Path,
    title: str,
) -> None:
    row_white = white_mask.sum(axis=1)
    row_rib = rib_mask.sum(axis=1)

    fig, axes = plt.subplots(1, 3, figsize=(11, 3))
    axes[0].hist(gray.ravel(), bins=32, range=(0, 255), color="gray")
    axes[0].axvline(WHITE_THRESHOLD, color="green", linestyle="--", linewidth=1)
    axes[0].set_title("gray histogram")

    axes[1].plot(row_white, np.arange(gray.shape[0]), color="green", label="white")
    axes[1].plot(row_rib, np.arange(gray.shape[0]), color="red", label="rib")
    axes[1].invert_yaxis()
    axes[1].set_title("row pixels")
    axes[1].legend()

    axes[2].imshow(gray, cmap="gray", vmin=0, vmax=255)
    for band in bands:
        axes[2].axhspan(band.ymin, band.ymax, color="red", alpha=0.25)
    axes[2].set_title("rib bands")
    axes[2].axis("off")

    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)


def component_to_dict(component: Component | None) -> dict | None:
    if component is None:
        return None
    return {
        "score": round(component.score, 4),
        "area": component.area,
        "bbox": [component.xmin, component.ymin, component.xmax, component.ymax],
        "width": component.width,
        "height": component.height,
        "centroid": [round(component.centroid_x, 2), round(component.centroid_y, 2)],
        "fill_ratio": round(component.fill_ratio, 4),
        "touches_border": component.touches_border,
        "mean_gray": round(component.mean_gray, 2),
        "approx_lateral_mm": round(estimate_lateral_mm_from_x(component.centroid_x), 1),
    }


def band_to_dict(band: RibBand) -> dict:
    return {
        "bbox": [band.xmin, band.ymin, band.xmax, band.ymax],
        "width": band.width,
        "height": band.height,
        "area": band.area,
        "max_row_pixels": band.max_row_pixels,
        "mean_gray": round(band.mean_gray, 2),
        "center_y": round(band.center_y, 2),
        "approx_forward_mm": round(estimate_forward_mm_from_row(band.center_y), 1),
    }


def detect_sample(sample: Sample, frame_dir: Path, output_dir: Path) -> dict:
    image_path = frame_dir / sample.frame
    if not image_path.exists():
        raise FileNotFoundError(image_path)

    rgb = np.asarray(Image.open(image_path).convert("RGB"))
    gray = np.asarray(Image.fromarray(rgb).convert("L"))

    white_mask, white_components, best_pvc, pvc_detected = detect_pvc(gray)
    rib_mask, dark_threshold = white_supported_dark_mask(gray, white_mask)
    bands = find_rib_bands(gray, rib_mask)
    bumpy_score = score_bumpy(pvc_detected, bands)
    classification = classify_frame(pvc_detected, bands, bumpy_score)

    sample_dir = output_dir / sample.frame.removesuffix(".png")
    ensure_dir(sample_dir)

    Image.fromarray(rgb, mode="RGB").save(sample_dir / "01_original.png")
    Image.fromarray(gray, mode="L").save(sample_dir / "02_gray.png")
    save_mask(white_mask, sample_dir / "03_white_pvc_mask.png")
    save_mask(rib_mask, sample_dir / "04_rib_mask.png")
    draw_overlay(
        rgb,
        sample,
        best_pvc,
        pvc_detected,
        bands,
        classification,
        bumpy_score,
    ).save(sample_dir / "05_overlay.png")
    save_profiles(
        gray,
        white_mask,
        rib_mask,
        bands,
        sample_dir / "06_profiles.png",
        sample.frame,
    )

    first_band = bands[0] if bands else None
    return {
        "frame": sample.frame,
        "label": sample.label,
        "expected": sample.expected,
        "classification": classification,
        "pvc_detected": pvc_detected,
        "bumpy_visible": classification == "bumpy_visible",
        "bumpy_score": round(bumpy_score, 4),
        "white_threshold": WHITE_THRESHOLD,
        "dark_threshold": round(dark_threshold, 2),
        "white_component_count": len(white_components),
        "best_pvc": component_to_dict(best_pvc),
        "rib_count": len(bands),
        "first_rib_forward_mm": (
            round(estimate_forward_mm_from_row(first_band.center_y), 1)
            if first_band is not None
            else None
        ),
        "rib_bands": [band_to_dict(band) for band in bands],
    }


def make_contact_sheet(output_dir: Path, results: list[dict]) -> None:
    overlays: list[Image.Image] = []
    for result in results:
        overlay_path = output_dir / result["frame"].removesuffix(".png") / "05_overlay.png"
        overlays.append(Image.open(overlay_path).convert("RGB"))

    if not overlays:
        return

    cols = 4
    rows = int(np.ceil(len(overlays) / cols))
    w, h = overlays[0].size
    sheet = Image.new("RGB", (cols * w, rows * h), "white")
    for i, image in enumerate(overlays):
        sheet.paste(image, ((i % cols) * w, (i // cols) * h))
    sheet.save(output_dir / "contact_sheet.png")


def write_summary(output_dir: Path, frame_dir: Path, results: list[dict]) -> None:
    summary = {
        "frame_dir": str(frame_dir),
        "output_dir": str(output_dir),
        "sample_count": len(results),
        "thresholds": {
            "white_threshold": WHITE_THRESHOLD,
            "min_pvc_score": MIN_PVC_SCORE,
            "min_bumpy_score": MIN_BUMPY_SCORE,
            "roi": [ROI_X0, ROI_Y0, ROI_X1, ROI_Y1],
            "min_rib_row_pixels": MIN_RIB_ROW_PIXELS,
            "min_rib_width": MIN_RIB_WIDTH,
            "min_rib_height": MIN_RIB_HEIGHT,
        },
        "results": results,
    }

    (output_dir / "summary.json").write_text(
        json.dumps(summary, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    lines: list[str] = []
    lines.append("# Bumpy Road Detection Samples")
    lines.append("")
    lines.append(f"frame_dir: `{frame_dir}`")
    lines.append(f"sample_count: `{len(results)}`")
    lines.append("")
    lines.append("| frame | expected | classification | pvc | ribs | score | first_rib_forward_mm |")
    lines.append("| --- | --- | --- | --- | ---: | ---: | ---: |")
    for result in results:
        lines.append(
            "| "
            f"{result['frame']} | {result['expected']} | {result['classification']} | "
            f"{result['pvc_detected']} | {result['rib_count']} | "
            f"{result['bumpy_score']:.2f} | {result['first_rib_forward_mm']} |"
        )
    lines.append("")
    lines.append("## Control use")
    lines.append("")
    lines.append("1. NavReplay opens this detector when the bumpy-road point is within 800 mm.")
    lines.append("2. `pvc_entry_or_clear` confirms white PVC entrance or exit, but does not mean ribs are visible.")
    lines.append("3. `bumpy_visible` means transverse dark ribs are visible inside white PVC.")
    lines.append("4. Use continuous 3 to 5 frame confirmation before triggering `BumpyRoad_Trigger()`.")
    lines.append("5. Replace `approx_forward_mm` with a calibrated row-to-distance table before using it on car.")
    lines.append("")
    lines.append("## Notes")
    lines.append("")
    lines.append(
        "- `frame_001917.png` is marked by the user as no bumpy road, but this detector "
        "still sees transverse rib-like bands. Keep it as a review sample while tuning."
    )
    (output_dir / "summary.md").write_text("\n".join(lines), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Detect bumpy-road sample frames.")
    parser.add_argument("--frame-dir", type=Path, default=None)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--frames", nargs="*", default=None, help="optional frame file names")
    return parser.parse_args()


def samples_from_args(frame_names: Iterable[str] | None) -> tuple[Sample, ...]:
    if not frame_names:
        return DEFAULT_SAMPLES
    return tuple(Sample(frame=name, label="cli sample", expected="unknown") for name in frame_names)


def main() -> None:
    args = parse_args()
    frame_dir = args.frame_dir or find_default_frame_dir()
    output_dir = args.output
    ensure_dir(output_dir)

    samples = samples_from_args(args.frames)
    results = [detect_sample(sample, frame_dir, output_dir) for sample in samples]
    make_contact_sheet(output_dir, results)
    write_summary(output_dir, frame_dir, results)

    bumpy_count = sum(1 for result in results if result["classification"] == "bumpy_visible")
    pvc_count = sum(1 for result in results if result["pvc_detected"])
    print(f"frame_dir: {frame_dir}")
    print(f"output_dir: {output_dir}")
    print(f"samples: {len(results)}")
    print(f"pvc_detected: {pvc_count}/{len(results)}")
    print(f"bumpy_visible: {bumpy_count}/{len(results)}")


if __name__ == "__main__":
    main()
