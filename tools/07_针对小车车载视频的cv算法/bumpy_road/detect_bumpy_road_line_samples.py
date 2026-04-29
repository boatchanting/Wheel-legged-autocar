from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from PIL import Image, ImageDraw

from bumpy_road_line_common import (
    DEFAULT_FRAME_DIR,
    IMAGE_CENTER_X,
    ROI_X0,
    ROI_X1,
    ROI_Y0,
    ROI_Y1,
    controller_mode_from_phase,
    detect_bumpy_road_frame,
    ensure_dir,
)


DEFAULT_OUTPUT_DIR = Path(r"E:\github_projects\autocar1\environment4BB7\Seekfree_CYT4BB_Opensource_Library\project2\data\bumpy_road_line_detection_samples")


@dataclass(frozen=True)
class Sample:
    frame: str
    stage: str


DEFAULT_SAMPLES: tuple[Sample, ...] = (
    Sample("frame_000021.png", "approach"),
    Sample("frame_000076.png", "approach"),
    Sample("frame_000301.png", "inside"),
    Sample("frame_000251.png", "inside"),
    Sample("frame_000254.png", "inside"),
    Sample("frame_000256.png", "inside"),
    Sample("frame_000257.png", "inside"),
    Sample("frame_000258.png", "inside"),
    Sample("frame_000266.png", "inside"),
    Sample("frame_000273.png", "inside"),
    Sample("frame_000280.png", "inside"),
    Sample("frame_000287.png", "inside"),
    Sample("frame_000290.png", "inside"),
    Sample("frame_000294.png", "inside"),
    Sample("frame_000295.png", "inside"),
    Sample("frame_000353.png", "exit"),
    Sample("frame_000304.png", "exit"),
    Sample("frame_000305.png", "exit"),
    Sample("frame_000308.png", "exit"),
    Sample("frame_000311.png", "exit"),
    Sample("frame_000315.png", "exit"),
    Sample("frame_000322.png", "exit"),
    Sample("frame_000326.png", "exit"),
    Sample("frame_000327.png", "exit"),
    Sample("frame_000328.png", "exit"),
    Sample("frame_000340.png", "exit"),
    Sample("frame_000346.png", "exit"),
    Sample("frame_000352.png", "exit"),
)


def stage_matches_phase(expected: str, phase: str) -> bool:
    if expected == "approach":
        return phase in {"approach_bumpy", "white_surface_only"}
    if expected == "inside":
        return phase in {"inside_bumpy", "white_surface_only", "approach_bumpy"}
    if expected == "exit":
        return phase in {"exit_bumpy", "white_surface_only", "uncertain"}
    return False


def save_mask(mask: np.ndarray, path: Path) -> None:
    Image.fromarray(mask.astype(np.uint8) * 255, mode="L").save(path)


def draw_overlay(rgb: np.ndarray, frame_name: str, detection: dict) -> Image.Image:
    scale = 6
    image = Image.fromarray(rgb, mode="RGB").resize(
        (rgb.shape[1] * scale, rgb.shape[0] * scale),
        Image.Resampling.NEAREST,
    )
    draw = ImageDraw.Draw(image)

    draw.rectangle(
        [ROI_X0 * scale, ROI_Y0 * scale, (ROI_X1 + 1) * scale - 1, (ROI_Y1 + 1) * scale - 1],
        outline="cyan",
        width=1,
    )

    component = detection["best_component"]
    if component is not None:
        draw.rectangle(
            [
                component.xmin * scale,
                component.ymin * scale,
                (component.xmax + 1) * scale - 1,
                (component.ymax + 1) * scale - 1,
            ],
            outline="yellow",
            width=2,
        )

    for run in detection["centerline_runs"]:
        draw.line(
            [
                run.xmin * scale,
                run.y * scale + scale // 2,
                run.xmax * scale,
                run.y * scale + scale // 2,
            ],
            fill="deepskyblue",
            width=2,
        )
        draw.ellipse(
            [
                run.center_x * scale - 2,
                run.y * scale + scale // 2 - 2,
                run.center_x * scale + 2,
                run.y * scale + scale // 2 + 2,
            ],
            fill="deepskyblue",
        )

    for idx, band in enumerate(detection["rib_bands"], start=1):
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

    target_x = detection["centerline"]["target_x"]
    draw.line(
        [target_x * scale, ROI_Y0 * scale, target_x * scale, (ROI_Y1 + 1) * scale - 1],
        fill="lime",
        width=2,
    )
    draw.line(
        [IMAGE_CENTER_X * scale, ROI_Y0 * scale, IMAGE_CENTER_X * scale, (ROI_Y1 + 1) * scale - 1],
        fill="magenta",
        width=1,
    )

    draw.rectangle([0, 0, image.width - 1, 34], fill=(0, 0, 0))
    draw.text(
        (4, 4),
        (
            f"{frame_name} phase={detection['phase']} mode={controller_mode_from_phase(detection['phase'])} "
            f"ribs={len(detection['rib_bands'])}"
        ),
        fill="white",
    )
    draw.text(
        (4, 18),
        (
            f"white={detection['white_threshold_int']} dark={int(round(detection['dark_threshold']))} "
            f"target_x={target_x:.1f} err={detection['centerline']['steer_error_px']:.1f}"
        ),
        fill="white",
    )
    return image


def save_profiles(gray: np.ndarray, detection: dict, path: Path, title: str) -> None:
    row_white = detection["white_mask"].sum(axis=1)
    row_rib = detection["rib_mask"].sum(axis=1)
    run_centers = [run.center_x for run in detection["centerline_runs"]]
    run_rows = [run.y for run in detection["centerline_runs"]]

    fig, axes = plt.subplots(1, 3, figsize=(12, 3))
    axes[0].hist(gray.ravel(), bins=32, range=(0, 255), color="gray")
    axes[0].axvline(detection["white_threshold_int"], color="lime", linestyle="--", linewidth=1)
    axes[0].axvline(detection["dark_threshold"], color="red", linestyle="--", linewidth=1)
    axes[0].set_title("gray histogram")

    axes[1].plot(row_white, np.arange(gray.shape[0]), color="lime", label="white")
    axes[1].plot(row_rib, np.arange(gray.shape[0]), color="red", label="rib")
    axes[1].invert_yaxis()
    axes[1].set_title("row pixels")
    axes[1].legend()

    axes[2].imshow(gray, cmap="gray", vmin=0, vmax=255)
    if run_centers:
        axes[2].plot(run_centers, run_rows, color="deepskyblue", linewidth=1.5)
    axes[2].axvline(detection["centerline"]["target_x"], color="lime", linewidth=1.0)
    axes[2].set_title("centerline")
    axes[2].axis("off")

    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)


def sample_to_dict(sample: Sample, detection: dict) -> dict:
    component = detection["best_component"]
    centerline = detection["centerline"]
    return {
        "frame": sample.frame,
        "stage": sample.stage,
        "phase": detection["phase"],
        "mode": detection["controller_mode"],
        "stage_ok": stage_matches_phase(sample.stage, detection["phase"]),
        "white_threshold": round(float(detection["white_threshold"]), 2),
        "white_threshold_candidate": round(float(detection["white_threshold_candidate"]), 2),
        "dark_threshold": round(float(detection["dark_threshold"]), 2),
        "target_x": round(float(centerline["target_x"]), 2),
        "steer_error_px": round(float(centerline["steer_error_px"]), 2),
        "centerline_row_count": centerline["row_count"],
        "centerline_bottom_row_count": centerline["bottom_row_count"],
        "centerline_top_y": centerline["top_y"],
        "centerline_bottom_y": centerline["bottom_y"],
        "centerline_mean_width": round(float(centerline["mean_width"]), 2),
        "rib_count": len(detection["rib_bands"]),
        "best_component": (
            None
            if component is None
            else {
                "score": round(float(component.score), 4),
                "bbox": [component.xmin, component.ymin, component.xmax, component.ymax],
                "centroid": [round(float(component.centroid_x), 2), round(float(component.centroid_y), 2)],
                "area": component.area,
            }
        ),
    }


def make_contact_sheet(output_dir: Path, samples: list[Sample]) -> None:
    overlays: list[Image.Image] = []
    for sample in samples:
        overlay_path = output_dir / f"{sample.frame.removesuffix('.png')}_05_overlay.png"
        overlays.append(Image.open(overlay_path).convert("RGB"))

    if not overlays:
        return

    cols = 4
    rows = int(np.ceil(len(overlays) / cols))
    width, height = overlays[0].size
    sheet = Image.new("RGB", (cols * width, rows * height), "white")
    for idx, image in enumerate(overlays):
        sheet.paste(image, ((idx % cols) * width, (idx // cols) * height))
    sheet.save(output_dir / "sample_line_detection_contact_sheet.png")


def detect_sample(sample: Sample, frame_dir: Path, output_dir: Path, prev_threshold: float | None) -> tuple[dict, float]:
    image_path = frame_dir / sample.frame
    if not image_path.exists():
        raise FileNotFoundError(image_path)

    rgb = np.asarray(Image.open(image_path).convert("RGB"))
    detection = detect_bumpy_road_frame(rgb, prev_white_threshold=prev_threshold)

    prefix = output_dir / sample.frame.removesuffix(".png")
    Image.fromarray(rgb, mode="RGB").save(Path(f"{prefix}_01_original.png"))
    Image.fromarray(detection["gray"], mode="L").save(Path(f"{prefix}_02_gray.png"))
    save_mask(detection["global_white_mask"], Path(f"{prefix}_03_global_white_mask.png"))
    save_mask(detection["scan_white_mask"], Path(f"{prefix}_04_scan_white_mask.png"))
    draw_overlay(rgb, sample.frame, detection).save(Path(f"{prefix}_05_overlay.png"))
    save_mask(detection["rib_mask"], Path(f"{prefix}_06_rib_mask.png"))
    save_profiles(detection["gray"], detection, Path(f"{prefix}_07_profiles.png"), sample.frame)

    return sample_to_dict(sample, detection), float(detection["white_threshold"])


def write_summary(output_dir: Path, frame_dir: Path, results: list[dict]) -> None:
    payload = {
        "frame_dir": str(frame_dir),
        "output_dir": str(output_dir),
        "sample_count": len(results),
        "results": results,
    }
    (output_dir / "sample_line_detection_summary.json").write_text(
        json.dumps(payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    ok_count = sum(1 for result in results if result["stage_ok"])
    lines = [
        "# Bumpy Road Line Samples",
        "",
        f"frame_dir: `{frame_dir}`",
        f"sample_count: `{len(results)}`",
        f"stage_ok: `{ok_count}/{len(results)}`",
        "",
        "| frame | stage | phase | mode | target_x | err_px | ribs | rows | ok |",
        "| --- | --- | --- | --- | ---: | ---: | ---: | ---: | --- |",
    ]
    for result in results:
        lines.append(
            "| "
            f"{result['frame']} | {result['stage']} | {result['phase']} | {result['mode']} | "
            f"{result['target_x']:.1f} | {result['steer_error_px']:.1f} | "
            f"{result['rib_count']} | {result['centerline_row_count']} | {result['stage_ok']} |"
        )
    lines.extend(
        [
            "",
            "## Control Use",
            "",
            "1. `approach_bumpy` 时优先朝 `target_x` 搜索并对准白色 PVC 入口。",
            "2. `inside_bumpy` 时直接使用 `steer_error_px` 做横向巡线，暗条纹只负责确认阶段，不参与横向控制。",
            "3. `exit_bumpy` 时继续保持 `target_x`，直到普通赛道寻线恢复稳定。",
            "4. 建议对 `phase` 做 3 到 5 帧连续确认，再切换状态机。",
        ]
    )
    (output_dir / "sample_line_detection_summary.md").write_text("\n".join(lines), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Test dynamic bumpy-road line following on sample frames.")
    parser.add_argument("--frame-dir", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--frames", nargs="*", default=None, help="optional frame names")
    return parser.parse_args()


def samples_from_args(frame_names: list[str] | None) -> list[Sample]:
    if not frame_names:
        return list(DEFAULT_SAMPLES)
    return [Sample(frame=name, stage="custom") for name in frame_names]


def main() -> None:
    args = parse_args()
    frame_dir = args.frame_dir
    output_dir = args.output
    ensure_dir(output_dir)

    samples = samples_from_args(args.frames)
    samples.sort(key=lambda item: item.frame)
    prev_threshold = None
    results: list[dict] = []
    for sample in samples:
        result, prev_threshold = detect_sample(sample, frame_dir, output_dir, prev_threshold)
        results.append(result)

    make_contact_sheet(output_dir, samples)
    write_summary(output_dir, frame_dir, results)

    print(f"frame_dir: {frame_dir}")
    print(f"output_dir: {output_dir}")
    print(f"samples: {len(results)}")
    print(f"stage_ok: {sum(1 for result in results if result['stage_ok'])}/{len(results)}")


if __name__ == "__main__":
    main()
