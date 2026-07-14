#!/usr/bin/env python3
"""分析固定灰度阈值在多组车载相机帧上的鲁棒性。

脚本兼容 Windows 中文路径，默认使用 THRESH_BINARY：灰度值 >= threshold
的像素为白色。结果包含逐帧 CSV、汇总 JSON、阈值扫描曲线和代表帧对比图。
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

import cv2
import matplotlib
import numpy as np

matplotlib.use("Agg")
import matplotlib.pyplot as plt


IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff"}


def read_gray(path: Path) -> np.ndarray:
    """使用 imdecode 读取图片，以支持 OpenCV 在 Windows 下的中文路径。"""
    data = np.fromfile(path, dtype=np.uint8)
    image = cv2.imdecode(data, cv2.IMREAD_GRAYSCALE)
    if image is None:
        raise ValueError(f"无法读取图片: {path}")
    return image


def write_image(path: Path, image: np.ndarray) -> None:
    """使用 imencode 写图，以支持 OpenCV 在 Windows 下的中文路径。"""
    path.parent.mkdir(parents=True, exist_ok=True)
    success, encoded = cv2.imencode(path.suffix, image)
    if not success:
        raise ValueError(f"无法编码图片: {path}")
    encoded.tofile(path)


def mask_at(gray: np.ndarray, threshold: int) -> np.ndarray:
    return gray >= threshold


def mask_iou(a: np.ndarray, b: np.ndarray) -> float:
    union = np.count_nonzero(a | b)
    return float(np.count_nonzero(a & b) / union) if union else 1.0


def component_metrics(mask: np.ndarray) -> tuple[int, float]:
    count, _, stats, _ = cv2.connectedComponentsWithStats(mask.astype(np.uint8), 8)
    if count <= 1:
        return 0, 0.0
    areas = stats[1:, cv2.CC_STAT_AREA]
    return int(np.count_nonzero(areas >= 3)), float(areas.max() / mask.size)


def distribution(values: list[float]) -> dict[str, float]:
    array = np.asarray(values, dtype=np.float64)
    return {
        "mean": float(array.mean()),
        "std": float(array.std()),
        "min": float(array.min()),
        "p05": float(np.percentile(array, 5)),
        "median": float(np.median(array)),
        "p95": float(np.percentile(array, 95)),
        "max": float(array.max()),
    }


def analyze_sequence(
    directory: Path, threshold: int, scan_thresholds: list[int]
) -> tuple[list[dict[str, object]], dict[str, object], dict[int, list[float]], list[Path]]:
    files = sorted(
        path for path in directory.iterdir()
        if path.is_file() and path.suffix.lower() in IMAGE_SUFFIXES
    )
    if not files:
        raise ValueError(f"目录中没有图像: {directory}")

    rows: list[dict[str, object]] = []
    scan_ratios = {value: [] for value in scan_thresholds}
    perturbations = sorted({max(0, threshold - 8), max(0, threshold - 4),
                            min(255, threshold + 4), min(255, threshold + 8)})
    previous_mask: np.ndarray | None = None

    for frame_index, path in enumerate(files):
        gray = read_gray(path)
        base_mask = mask_at(gray, threshold)
        component_count, largest_component_ratio = component_metrics(base_mask)
        changed = {}
        ious = {}
        for value in perturbations:
            candidate = mask_at(gray, value)
            changed[str(value)] = float(np.mean(candidate != base_mask))
            ious[str(value)] = mask_iou(candidate, base_mask)
        otsu_value, _ = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
        row: dict[str, object] = {
            "sequence": directory.name,
            "frame_index": frame_index,
            "file": path.name,
            "gray_mean": float(gray.mean()),
            "gray_std": float(gray.std()),
            "gray_p05": float(np.percentile(gray, 5)),
            "gray_median": float(np.median(gray)),
            "gray_p95": float(np.percentile(gray, 95)),
            "otsu_threshold": float(otsu_value),
            "white_ratio": float(base_mask.mean()),
            "component_count_ge3": component_count,
            "largest_component_ratio": largest_component_ratio,
            "adjacent_mask_iou": "" if previous_mask is None else mask_iou(previous_mask, base_mask),
        }
        for value in perturbations:
            row[f"changed_ratio_t{value}"] = changed[str(value)]
            row[f"iou_vs_t{threshold}_t{value}"] = ious[str(value)]
        rows.append(row)
        previous_mask = base_mask
        for value in scan_thresholds:
            scan_ratios[value].append(float(mask_at(gray, value).mean()))

    white_ratios = [float(row["white_ratio"]) for row in rows]
    adjacent_ious = [float(row["adjacent_mask_iou"]) for row in rows[1:]]
    summary: dict[str, object] = {
        "directory": str(directory.resolve()),
        "frame_count": len(rows),
        "image_shape": list(read_gray(files[0]).shape),
        "gray_mean": distribution([float(row["gray_mean"]) for row in rows]),
        "otsu_threshold": distribution([float(row["otsu_threshold"]) for row in rows]),
        "white_ratio_at_threshold": distribution(white_ratios),
        "adjacent_mask_iou": distribution(adjacent_ious),
        "largest_component_ratio": distribution(
            [float(row["largest_component_ratio"]) for row in rows]
        ),
        "threshold_perturbation": {
            str(value): {
                "changed_pixel_ratio": distribution(
                    [float(row[f"changed_ratio_t{value}"]) for row in rows]
                ),
                "iou_vs_base": distribution(
                    [float(row[f"iou_vs_t{threshold}_t{value}"]) for row in rows]
                ),
            }
            for value in perturbations
        },
    }
    return rows, summary, scan_ratios, files


def write_csv(path: Path, rows: list[dict[str, object]]) -> None:
    with path.open("w", newline="", encoding="utf-8-sig") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def save_scan_plot(
    path: Path, datasets: list[tuple[str, dict[int, list[float]]]], threshold: int
) -> None:
    fig, ax = plt.subplots(figsize=(9, 5.2))
    for sequence_index, (_, scan) in enumerate(datasets, start=1):
        x = np.asarray(sorted(scan))
        means = np.asarray([np.mean(scan[value]) for value in x]) * 100
        low = np.asarray([np.percentile(scan[value], 5) for value in x]) * 100
        high = np.asarray([np.percentile(scan[value], 95) for value in x]) * 100
        line, = ax.plot(x, means, marker="o", label=f"S{sequence_index}")
        ax.fill_between(x, low, high, color=line.get_color(), alpha=0.13)
    ax.axvline(threshold, color="black", linestyle="--", linewidth=1, label=f"base={threshold}")
    ax.set(xlabel="Threshold", ylabel="White pixel ratio (%)",
           title="Fixed-threshold sensitivity (band: frame P05-P95)")
    ax.grid(alpha=0.25)
    ax.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(path, dpi=180)
    plt.close(fig)


def save_contact_sheet(
    path: Path, datasets: list[tuple[str, list[Path]]], threshold: int
) -> None:
    fractions = np.linspace(0.05, 0.95, 5)
    fig, axes = plt.subplots(len(datasets) * 2, len(fractions), figsize=(13, 6.8))
    for dataset_index, (_, files) in enumerate(datasets):
        selected = [files[min(len(files) - 1, round(f * (len(files) - 1)))] for f in fractions]
        for column, file in enumerate(selected):
            gray = read_gray(file)
            binary = (mask_at(gray, threshold).astype(np.uint8) * 255)
            original_axis = axes[dataset_index * 2, column]
            binary_axis = axes[dataset_index * 2 + 1, column]
            original_axis.imshow(gray, cmap="gray", vmin=0, vmax=255, interpolation="nearest")
            binary_axis.imshow(binary, cmap="gray", vmin=0, vmax=255, interpolation="nearest")
            original_axis.set_title(
                f"S{dataset_index + 1}  {file.stem[-11:]}", fontsize=8
            )
            if column == 0:
                original_axis.set_ylabel("gray")
                binary_axis.set_ylabel(f"T={threshold}")
            original_axis.set_xticks([]); original_axis.set_yticks([])
            binary_axis.set_xticks([]); binary_axis.set_yticks([])
    fig.suptitle("Representative frames: grayscale and fixed-threshold masks", fontsize=12)
    fig.tight_layout()
    fig.savefig(path, dpi=180)
    plt.close(fig)


def save_all_result_images(
    root: Path, datasets: list[tuple[str, list[Path]]], threshold: int
) -> int:
    """保存全量二值图，以及便于人工检查的原图/二值图并排对照图。"""
    saved = 0
    for sequence_index, (_, files) in enumerate(datasets, start=1):
        binary_dir = root / f"S{sequence_index}_binary_t{threshold}"
        comparison_dir = root / f"S{sequence_index}_gray_vs_binary_t{threshold}"
        for frame_index, file in enumerate(files):
            gray = read_gray(file)
            binary = mask_at(gray, threshold).astype(np.uint8) * 255
            comparison = np.hstack((gray, binary))
            output_name = f"frame_{frame_index:06d}.png"
            write_image(binary_dir / output_name, binary)
            write_image(comparison_dir / output_name, comparison)
            saved += 1
    return saved


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path, help="一个或多个帧目录")
    parser.add_argument("--threshold", type=int, default=128, help="基准阈值，默认 128")
    parser.add_argument("--scan-min", type=int, default=80)
    parser.add_argument("--scan-max", type=int, default=176)
    parser.add_argument("--scan-step", type=int, default=8)
    parser.add_argument("--output", type=Path, default=Path("threshold_robustness_output"))
    parser.add_argument(
        "--save-all-images", action="store_true",
        help="保存每帧二值图和原图/二值图并排对照图",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if not 0 <= args.threshold <= 255:
        raise SystemExit("--threshold 必须在 0..255 内")
    if args.scan_step <= 0 or args.scan_min > args.scan_max:
        raise SystemExit("阈值扫描参数无效")
    args.output.mkdir(parents=True, exist_ok=True)
    scan_thresholds = sorted(set(range(args.scan_min, args.scan_max + 1, args.scan_step)) | {args.threshold})

    all_rows: list[dict[str, object]] = []
    summaries: dict[str, object] = {}
    scans: list[tuple[str, dict[int, list[float]]]] = []
    contacts: list[tuple[str, list[Path]]] = []
    for directory in args.inputs:
        rows, summary, scan, files = analyze_sequence(directory, args.threshold, scan_thresholds)
        all_rows.extend(rows)
        summaries[directory.name] = summary
        scans.append((directory.name, scan))
        contacts.append((directory.name, files))

    report = {
        "threshold": args.threshold,
        "binary_rule": f"gray >= {args.threshold} -> white",
        "scan_thresholds": scan_thresholds,
        "sequences": summaries,
    }
    write_csv(args.output / "per_frame_metrics.csv", all_rows)
    (args.output / "summary.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    save_scan_plot(args.output / "threshold_sensitivity.png", scans, args.threshold)
    save_contact_sheet(args.output / "representative_binary_masks.png", contacts, args.threshold)
    if args.save_all_images:
        saved = save_all_result_images(args.output / "all_frames", contacts, args.threshold)
        print(f"已保存 {saved} 帧全量二值图及对照图")
    print(f"完成：{len(all_rows)} 帧，结果位于 {args.output.resolve()}")


if __name__ == "__main__":
    main()
