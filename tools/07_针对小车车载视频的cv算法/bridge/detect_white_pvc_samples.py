"""Detect white PVC bridge entrance regions in selected on-car frames.

This is a first-pass algorithm intentionally tuned for the current sample set:
96x60 DIB frames from data/frames/2026_04_17_21_18_39_Video.

Outputs:
- intermediate masks and diagnostic plots for every sample
- overlay visualizations
- summary.txt with per-frame decisions and measurements
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import matplotlib.pyplot as plt
import numpy as np
from PIL import Image, ImageDraw, ImageFont


PROJECT_ROOT = Path(__file__).resolve().parents[3]
FRAME_DIR = PROJECT_ROOT / "data/frames/2026_04_17_21_18_39_Video"
OUTPUT_DIR = PROJECT_ROOT / "data/bridge_white_pvc_detection"


@dataclass(frozen=True)
class Sample:
    frame: str
    label: str
    expect_pvc: bool


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


SAMPLES: tuple[Sample, ...] = (
    Sample("frame_000039.png", "无白色pvc", False),
    Sample("frame_000062.png", "白色pvc漏出一角两边", True),
    Sample("frame_000210.png", "白色pvc漏出一角两边", True),
    Sample("frame_000307.png", "白色pvc漏出一角两边+一个半边", True),
    Sample("frame_000561.png", "白色pvc漏出一角两边+一个半边", True),
    Sample("frame_000742.png", "白色pvc漏出一角两边+一个半边", True),
    Sample("frame_000600.png", "白色pvc 0个角 三条边", True),
    Sample("frame_000932.png", "白色pvc 2个角 三条边", True),
    Sample("frame_002182.png", "白色pvc小区域，在左上角", True),
    Sample("frame_002183.png", "无白色pvc", False),
)


WHITE_THRESHOLD = 245
MIN_AREA = 120
MIN_WIDTH = 12
MIN_HEIGHT = 4
MIN_FILL_RATIO = 0.25
MIN_DECISION_SCORE = 0.58


def ensure_clean_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def save_mask(mask: np.ndarray, path: Path) -> None:
    Image.fromarray((mask.astype(np.uint8) * 255), mode="L").save(path)


def find_components(mask: np.ndarray, gray: np.ndarray) -> list[Component]:
    h, w = mask.shape
    visited = np.zeros_like(mask, dtype=bool)
    components: list[Component] = []

    for y0 in range(h):
        for x0 in range(w):
            if not mask[y0, x0] or visited[y0, x0]:
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
            bbox_area = (xmax - xmin + 1) * (ymax - ymin + 1)
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
                    fill_ratio=float(area / max(1, bbox_area)),
                    touches_border=touches_border,
                    mean_gray=mean_gray,
                )
            )

    return sorted(components, key=lambda c: c.area, reverse=True)


def score_component(component: Component, image_area: int) -> float:
    area_score = min(component.area / 600.0, 1.0)
    width_score = min(component.width / 45.0, 1.0)
    height_score = min(component.height / 18.0, 1.0)
    fill_score = min(component.fill_ratio / 0.55, 1.0)
    border_score = 1.0 if component.touches_border else 0.0
    brightness_score = min(max((component.mean_gray - 235.0) / 20.0, 0.0), 1.0)

    # PVC in these samples enters from the image border and forms a large,
    # saturated, compact region. Specular spots are bright but too small.
    return (
        0.35 * area_score
        + 0.18 * width_score
        + 0.12 * height_score
        + 0.12 * fill_score
        + 0.18 * border_score
        + 0.05 * brightness_score
    )


def filter_candidates(components: Iterable[Component], image_area: int) -> list[Component]:
    candidates: list[Component] = []
    for component in components:
        component.score = score_component(component, image_area)
        if component.area < MIN_AREA:
            continue
        if component.width < MIN_WIDTH or component.height < MIN_HEIGHT:
            continue
        if component.fill_ratio < MIN_FILL_RATIO:
            continue
        if not component.touches_border:
            continue
        candidates.append(component)
    return sorted(candidates, key=lambda c: c.score, reverse=True)


def build_candidate_mask(mask: np.ndarray, candidate: Component | None) -> np.ndarray:
    if candidate is None:
        return np.zeros_like(mask, dtype=bool)

    crop = mask[candidate.ymin : candidate.ymax + 1, candidate.xmin : candidate.xmax + 1]
    local_components = find_components(crop, crop.astype(np.uint8) * 255)
    if not local_components:
        return np.zeros_like(mask, dtype=bool)

    best_local = local_components[0]
    out = np.zeros_like(mask, dtype=bool)
    sub = crop.copy()

    # Re-run component extraction in the full mask and keep pixels matching
    # candidate bbox; this is enough for current diagnostics because there is
    # only one large component per PVC sample.
    out[candidate.ymin : candidate.ymax + 1, candidate.xmin : candidate.xmax + 1] = sub
    return out


def draw_overlay(
    rgb: np.ndarray,
    components: list[Component],
    best: Component | None,
    detected: bool,
    sample: Sample,
) -> Image.Image:
    image = Image.fromarray(rgb, mode="RGB").resize((rgb.shape[1] * 6, rgb.shape[0] * 6), Image.Resampling.NEAREST)
    draw = ImageDraw.Draw(image)
    scale = 6

    for component in components[:6]:
        color = "yellow"
        if best is component and detected:
            color = "lime"
        elif component.area < MIN_AREA:
            color = "orange"
        draw.rectangle(
            [
                component.xmin * scale,
                component.ymin * scale,
                (component.xmax + 1) * scale - 1,
                (component.ymax + 1) * scale - 1,
            ],
            outline=color,
            width=2,
        )

    title = f"{sample.frame} | expect={sample.expect_pvc} detected={detected}"
    if best is not None:
        title += f" | score={best.score:.2f} area={best.area} bbox=({best.xmin},{best.ymin})-({best.xmax},{best.ymax})"
        draw.ellipse(
            [
                best.centroid_x * scale - 4,
                best.centroid_y * scale - 4,
                best.centroid_x * scale + 4,
                best.centroid_y * scale + 4,
            ],
            fill="red",
        )

    draw.rectangle([0, 0, image.width - 1, 22], fill=(0, 0, 0))
    draw.text((4, 4), title, fill="white")
    return image


def save_profiles(gray: np.ndarray, mask: np.ndarray, output_path: Path, title: str) -> None:
    row_sum = mask.sum(axis=1)
    col_sum = mask.sum(axis=0)

    fig, axes = plt.subplots(1, 3, figsize=(11, 3))
    axes[0].hist(gray.ravel(), bins=32, range=(0, 255), color="gray")
    axes[0].axvline(WHITE_THRESHOLD, color="red", linestyle="--", linewidth=1)
    axes[0].set_title("gray histogram")

    axes[1].plot(row_sum, np.arange(mask.shape[0]))
    axes[1].invert_yaxis()
    axes[1].set_title("white pixels / row")
    axes[1].set_xlabel("count")
    axes[1].set_ylabel("y")

    axes[2].plot(np.arange(mask.shape[1]), col_sum)
    axes[2].set_title("white pixels / col")
    axes[2].set_xlabel("x")
    axes[2].set_ylabel("count")

    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(output_path, dpi=150)
    plt.close(fig)


def detect_sample(sample: Sample, output_dir: Path) -> dict:
    image_path = FRAME_DIR / sample.frame
    if not image_path.exists():
        raise FileNotFoundError(image_path)

    rgb = np.asarray(Image.open(image_path).convert("RGB"))
    gray = np.asarray(Image.fromarray(rgb).convert("L"))
    raw_mask = gray >= WHITE_THRESHOLD

    components = find_components(raw_mask, gray)
    candidates = filter_candidates(components, gray.size)
    best = candidates[0] if candidates else None
    detected = best is not None and best.score >= MIN_DECISION_SCORE
    candidate_mask = build_candidate_mask(raw_mask, best if detected else None)

    sample_dir = output_dir / sample.frame.removesuffix(".png")
    ensure_clean_dir(sample_dir)

    Image.fromarray(rgb, mode="RGB").save(sample_dir / "01_original.png")
    Image.fromarray(gray, mode="L").save(sample_dir / "02_gray.png")
    save_mask(raw_mask, sample_dir / "03_raw_white_mask.png")
    save_mask(candidate_mask, sample_dir / "04_selected_pvc_mask.png")
    draw_overlay(rgb, components, best, detected, sample).save(sample_dir / "05_overlay.png")
    save_profiles(gray, raw_mask, sample_dir / "06_profiles.png", sample.frame)

    result = {
        "frame": sample.frame,
        "label": sample.label,
        "expect_pvc": sample.expect_pvc,
        "detected_pvc": detected,
        "ok": detected == sample.expect_pvc,
        "threshold": WHITE_THRESHOLD,
        "component_count": len(components),
        "candidate_count": len(candidates),
        "best": None,
    }

    if best is not None:
        result["best"] = {
            "score": round(best.score, 4),
            "area": best.area,
            "bbox": [best.xmin, best.ymin, best.xmax, best.ymax],
            "width": best.width,
            "height": best.height,
            "centroid": [round(best.centroid_x, 2), round(best.centroid_y, 2)],
            "fill_ratio": round(best.fill_ratio, 4),
            "touches_border": best.touches_border,
            "mean_gray": round(best.mean_gray, 2),
            "entry_bottom_y": best.ymax,
            "entry_top_y": best.ymin,
        }

    return result


def write_summary(results: list[dict], output_dir: Path) -> None:
    lines: list[str] = []
    pass_count = sum(1 for r in results if r["ok"])
    lines.append("白色 PVC 区域识别样例汇总")
    lines.append(f"样例数量: {len(results)}")
    lines.append(f"判断正确: {pass_count}/{len(results)}")
    lines.append(f"白色阈值: gray >= {WHITE_THRESHOLD}")
    lines.append(f"决策阈值: score >= {MIN_DECISION_SCORE}")
    lines.append("")

    for result in results:
        best = result["best"]
        lines.append(f"[{result['frame']}] {result['label']}")
        lines.append(f"  expect_pvc={result['expect_pvc']} detected_pvc={result['detected_pvc']} ok={result['ok']}")
        lines.append(f"  component_count={result['component_count']} candidate_count={result['candidate_count']}")
        if best is None:
            lines.append("  best=None")
        else:
            lines.append(
                "  best="
                f"score={best['score']}, area={best['area']}, bbox={best['bbox']}, "
                f"centroid={best['centroid']}, fill={best['fill_ratio']}, "
                f"entry_bottom_y={best['entry_bottom_y']}"
            )
        lines.append("")

    lines.append("控制思路:")
    lines.append("  1. NavReplay 预测单边桥特殊点距离小于 800mm 后，开启白色 PVC ROI 检测。")
    lines.append("  2. 连续 3~5 帧 detected_pvc=true 且 bbox/centroid 稳定，确认进入单边桥入口搜索区。")
    lines.append("  3. 使用 PVC 大区域的下边缘 entry_bottom_y 查表得到入口距离 forward_mm。")
    lines.append("  4. 用 PVC 区域中心与图像中心差值估算 lateral_mm，给 err_degree 一个小幅视觉纠偏。")
    lines.append("  5. forward_mm 小于触发阈值后记录 entry_pose，抬底盘，进入 Bridge 状态机。")
    lines.append("  6. 通过过程中视觉继续以白色 PVC 中心线控方向，黑色单边桥只做计数和左右侧判断。")
    lines.append("  7. 计数达到 3 且看到 PVC 终点，或 local_s_mm 超过固定桥长后，降底盘并按固定长度修正惯导。")

    (output_dir / "summary.txt").write_text("\n".join(lines), encoding="utf-8")
    (output_dir / "summary.json").write_text(
        json.dumps(results, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )


def main() -> None:
    ensure_clean_dir(OUTPUT_DIR)
    results = [detect_sample(sample, OUTPUT_DIR) for sample in SAMPLES]
    write_summary(results, OUTPUT_DIR)
    print(f"output: {OUTPUT_DIR}")
    print(f"ok: {sum(1 for r in results if r['ok'])}/{len(results)}")


if __name__ == "__main__":
    main()
