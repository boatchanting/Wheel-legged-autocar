"""Detect single-side bridge black wedge obstacles on selected on-car frames.

This script copies the idea from ``单边桥识别.py``:
- threshold / morphology preprocessing
- bridge/PVC region as the white background
- abrupt dark obstacle geometry as the bridge feature

For the real 96x60 on-car frames here, the car is already on the white PVC.
So the robust detection target is not "find PVC", but "find dark wedge-like
connected components on the PVC background", then classify left/right by the
top edge center of the dark block.
"""

from __future__ import annotations

import importlib.util
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import cv2
import matplotlib.pyplot as plt
import numpy as np


PROJECT_ROOT = Path(__file__).resolve().parents[3]
FRAME_DIR = PROJECT_ROOT / "data/frames/2026_04_17_21_13_28_Video"
OUTPUT_DIR = PROJECT_ROOT / "data/bridge_single_side_detection_samples"
COPIED_ORIGINAL = Path(__file__).with_name("单边桥识别.py")


@dataclass(frozen=True)
class Sample:
    frame: str
    label: str
    expected: tuple[str, ...]


@dataclass
class DarkBridgeComponent:
    area: int
    bbox: tuple[int, int, int, int]
    centroid: tuple[float, float]
    top_center_x: float
    bottom_center_x: float
    fill_ratio: float
    mean_gray: float
    side: str
    score: float

    @property
    def width(self) -> int:
        return self.bbox[2] - self.bbox[0] + 1

    @property
    def height(self) -> int:
        return self.bbox[3] - self.bbox[1] + 1


SAMPLES: tuple[Sample, ...] = (
    Sample("frame_000202.png", "右单边桥", ("RIGHT",)),
    Sample("frame_000271.png", "右单边桥", ("RIGHT",)),
    Sample("frame_000392.png", "右单边桥", ("RIGHT",)),
    Sample("frame_000403.png", "无单边桥", tuple()),
    Sample("frame_000426.png", "无单边桥", tuple()),
    Sample("frame_000453.png", "左单边桥", ("LEFT",)),
    Sample("frame_000586.png", "先左单边桥 后右单边桥", ("LEFT", "RIGHT")),
    Sample("frame_000597.png", "左单边桥", ("LEFT",)),
    Sample("frame_000941.png", "右单边桥", ("RIGHT",)),
)


DARK_THRESHOLD = 150
MIN_DARK_AREA = 180
MIN_DARK_WIDTH = 12
MIN_DARK_HEIGHT = 7
MIN_FILL_RATIO = 0.20
SIDE_SPLIT_X = 48.0


def load_original_detector_classes() -> tuple[Any, Any]:
    spec = importlib.util.spec_from_file_location("copied_danbianqiao", COPIED_ORIGINAL)
    if spec is None or spec.loader is None:
        raise ImportError(f"cannot load {COPIED_ORIGINAL}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.DanBianQiaoParams, module.DanBianQiaoDetector


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def save_gray(path: Path, image: np.ndarray) -> None:
    cv2.imwrite(str(path), image)


def save_color(path: Path, image_bgr: np.ndarray) -> None:
    cv2.imwrite(str(path), image_bgr)


def build_original_debug(gray: np.ndarray) -> dict[str, Any]:
    Params, Detector = load_original_detector_classes()
    params = Params(
        use_otsu=True,
        close_ksize=3,
        open_ksize=3,
        min_run=20,
        require_jump_type="any",
    )
    detector = Detector(params, debug=True)
    bw = detector.preprocess_to_binary(gray)
    l_border, r_border, left_stop, right_stop, hightest = detector.extract_borders(bw)
    result = detector.detect_from_borders(bw, l_border, r_border, left_stop, right_stop, hightest)
    vis = detector.draw_debug(gray, bw, l_border, r_border, result)
    return {
        "bw": bw,
        "l_border": l_border,
        "r_border": r_border,
        "result": result,
        "vis": vis,
    }


def make_dark_mask(gray: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    raw = (gray < DARK_THRESHOLD).astype(np.uint8) * 255
    kernel_open = cv2.getStructuringElement(cv2.MORPH_RECT, (2, 2))
    kernel_close = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
    clean = cv2.morphologyEx(raw, cv2.MORPH_OPEN, kernel_open, iterations=1)
    clean = cv2.morphologyEx(clean, cv2.MORPH_CLOSE, kernel_close, iterations=1)
    return raw, clean


def component_from_stats(gray: np.ndarray, labels: np.ndarray, label_id: int, stats: np.ndarray) -> DarkBridgeComponent | None:
    x, y, w, h, area = [int(v) for v in stats[label_id]]
    if area < MIN_DARK_AREA or w < MIN_DARK_WIDTH or h < MIN_DARK_HEIGHT:
        return None

    mask = labels[y : y + h, x : x + w] == label_id
    ys, xs = np.where(mask)
    if xs.size == 0:
        return None

    abs_xs = xs + x
    abs_ys = ys + y
    fill_ratio = float(area / max(1, w * h))
    if fill_ratio < MIN_FILL_RATIO:
        return None

    top_band = abs_ys <= abs_ys.min() + 2
    bottom_band = abs_ys >= abs_ys.max() - 2
    top_center_x = float(abs_xs[top_band].mean())
    bottom_center_x = float(abs_xs[bottom_band].mean())
    centroid_x = float(abs_xs.mean())
    centroid_y = float(abs_ys.mean())
    mean_gray = float(gray[abs_ys, abs_xs].mean())

    # In these camera frames, the top/leading edge center is a better side cue
    # than centroid because the wedge can extend diagonally across the image.
    side = "LEFT" if top_center_x < SIDE_SPLIT_X else "RIGHT"
    area_score = min(area / 900.0, 1.0)
    size_score = min(w / 45.0, 1.0) * 0.5 + min(h / 30.0, 1.0) * 0.5
    dark_score = min(max((180.0 - mean_gray) / 80.0, 0.0), 1.0)
    score = 0.45 * area_score + 0.35 * size_score + 0.20 * dark_score

    return DarkBridgeComponent(
        area=area,
        bbox=(x, y, x + w - 1, y + h - 1),
        centroid=(centroid_x, centroid_y),
        top_center_x=top_center_x,
        bottom_center_x=bottom_center_x,
        fill_ratio=fill_ratio,
        mean_gray=mean_gray,
        side=side,
        score=score,
    )


def detect_dark_bridge(gray: np.ndarray) -> tuple[np.ndarray, np.ndarray, list[DarkBridgeComponent]]:
    raw, clean = make_dark_mask(gray)
    num_labels, labels, stats, _centroids = cv2.connectedComponentsWithStats((clean > 0).astype(np.uint8), 8)
    comps: list[DarkBridgeComponent] = []
    for label_id in range(1, num_labels):
        comp = component_from_stats(gray, labels, label_id, stats)
        if comp is not None:
            comps.append(comp)

    comps.sort(key=lambda c: c.area, reverse=True)

    # Keep at most one component per side. This makes frame_000586 report LEFT
    # then RIGHT, and prevents tiny duplicated fragments from over-counting.
    by_side: dict[str, DarkBridgeComponent] = {}
    for comp in comps:
        old = by_side.get(comp.side)
        if old is None or comp.score > old.score:
            by_side[comp.side] = comp

    ordered: list[DarkBridgeComponent] = []
    if "LEFT" in by_side:
        ordered.append(by_side["LEFT"])
    if "RIGHT" in by_side:
        ordered.append(by_side["RIGHT"])
    return raw, clean, ordered


def draw_components_overlay(gray: np.ndarray, components: list[DarkBridgeComponent], title: str) -> np.ndarray:
    vis = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
    vis = cv2.resize(vis, (gray.shape[1] * 6, gray.shape[0] * 6), interpolation=cv2.INTER_NEAREST)
    scale = 6

    cv2.line(vis, (int(SIDE_SPLIT_X * scale), 0), (int(SIDE_SPLIT_X * scale), vis.shape[0] - 1), (255, 0, 255), 1)

    for comp in components:
        x1, y1, x2, y2 = comp.bbox
        color = (0, 255, 0) if comp.side == "LEFT" else (0, 128, 255)
        cv2.rectangle(vis, (x1 * scale, y1 * scale), ((x2 + 1) * scale - 1, (y2 + 1) * scale - 1), color, 2)
        cv2.circle(vis, (int(comp.top_center_x * scale), y1 * scale), 4, (0, 0, 255), -1)
        cv2.circle(vis, (int(comp.centroid[0] * scale), int(comp.centroid[1] * scale)), 3, (255, 0, 0), -1)
        cv2.putText(
            vis,
            f"{comp.side} a={comp.area} s={comp.score:.2f}",
            (x1 * scale, max(14, y1 * scale - 4)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.45,
            color,
            1,
            cv2.LINE_AA,
        )

    cv2.rectangle(vis, (0, 0), (vis.shape[1] - 1, 20), (0, 0, 0), -1)
    cv2.putText(vis, title, (4, 14), cv2.FONT_HERSHEY_SIMPLEX, 0.42, (255, 255, 255), 1, cv2.LINE_AA)
    return vis


def save_profiles(gray: np.ndarray, dark_mask: np.ndarray, components: list[DarkBridgeComponent], output_path: Path, title: str) -> None:
    fig, axes = plt.subplots(1, 3, figsize=(11, 3))
    axes[0].hist(gray.ravel(), bins=32, range=(0, 255), color="gray")
    axes[0].axvline(DARK_THRESHOLD, color="red", linestyle="--")
    axes[0].set_title("gray histogram")

    axes[1].plot(dark_mask.sum(axis=1) / 255, np.arange(gray.shape[0]))
    axes[1].invert_yaxis()
    axes[1].set_title("dark pixels / row")
    axes[1].set_xlabel("count")

    axes[2].imshow(gray, cmap="gray", vmin=0, vmax=255)
    for comp in components:
        x1, y1, x2, y2 = comp.bbox
        color = "lime" if comp.side == "LEFT" else "orange"
        axes[2].add_patch(plt.Rectangle((x1, y1), x2 - x1 + 1, y2 - y1 + 1, fill=False, edgecolor=color, linewidth=1.5))
    axes[2].set_title("component boxes")
    axes[2].axis("off")

    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(output_path, dpi=150)
    plt.close(fig)


def sides_from_components(components: list[DarkBridgeComponent]) -> tuple[str, ...]:
    return tuple(comp.side for comp in components)


def result_matches(expected: tuple[str, ...], detected: tuple[str, ...]) -> bool:
    return expected == detected


def process_sample(sample: Sample, output_dir: Path) -> dict[str, Any]:
    frame_path = FRAME_DIR / sample.frame
    if not frame_path.exists():
        raise FileNotFoundError(frame_path)

    gray = cv2.imread(str(frame_path), cv2.IMREAD_GRAYSCALE)
    if gray is None:
        raise ValueError(f"failed to read {frame_path}")

    original_debug = build_original_debug(gray)
    raw_dark, clean_dark, components = detect_dark_bridge(gray)
    detected = sides_from_components(components)
    ok = result_matches(sample.expected, detected)

    sample_dir = output_dir / sample.frame.removesuffix(".png")
    ensure_dir(sample_dir)
    save_gray(sample_dir / "01_gray.png", gray)
    save_gray(sample_dir / "02_original_detector_white_binary.png", original_debug["bw"])
    save_color(sample_dir / "03_original_detector_borders.png", original_debug["vis"])
    save_gray(sample_dir / "04_dark_mask_raw.png", raw_dark)
    save_gray(sample_dir / "05_dark_mask_clean.png", clean_dark)
    overlay = draw_components_overlay(gray, components, f"{sample.frame} expect={sample.expected} detected={detected}")
    save_color(sample_dir / "06_dark_bridge_overlay.png", overlay)
    save_profiles(gray, clean_dark, components, sample_dir / "07_profiles.png", sample.frame)

    return {
        "frame": sample.frame,
        "label": sample.label,
        "expected": list(sample.expected),
        "detected": list(detected),
        "ok": ok,
        "dark_threshold": DARK_THRESHOLD,
        "original_detector_result": original_debug["result"],
        "components": [
            {
                "side": comp.side,
                "score": round(comp.score, 4),
                "area": comp.area,
                "bbox": list(comp.bbox),
                "centroid": [round(comp.centroid[0], 2), round(comp.centroid[1], 2)],
                "top_center_x": round(comp.top_center_x, 2),
                "bottom_center_x": round(comp.bottom_center_x, 2),
                "fill_ratio": round(comp.fill_ratio, 4),
                "mean_gray": round(comp.mean_gray, 2),
            }
            for comp in components
        ],
    }


def write_summary(results: list[dict[str, Any]], output_dir: Path) -> None:
    pass_count = sum(1 for item in results if item["ok"])
    lines: list[str] = [
        "单边桥黑色楔形识别样例汇总",
        f"样例数量: {len(results)}",
        f"判断正确: {pass_count}/{len(results)}",
        f"暗色阈值: gray < {DARK_THRESHOLD}",
        f"最小面积: {MIN_DARK_AREA}",
        "",
    ]

    for item in results:
        lines.append(f"[{item['frame']}] {item['label']}")
        lines.append(f"  expected={item['expected']} detected={item['detected']} ok={item['ok']}")
        lines.append(f"  copied_original_result={item['original_detector_result']}")
        if not item["components"]:
            lines.append("  components=None")
        else:
            for idx, comp in enumerate(item["components"], start=1):
                lines.append(
                    f"  comp{idx}: side={comp['side']} score={comp['score']} area={comp['area']} "
                    f"bbox={comp['bbox']} top_center_x={comp['top_center_x']} "
                    f"centroid={comp['centroid']} mean_gray={comp['mean_gray']}"
                )
        lines.append("")

    lines += [
        "视觉识别原理:",
        "  1. 车已经进入白色 PVC 后，PVC 大白块检测不再适合作为主算法。",
        "  2. 单边桥楔形在图像中表现为白底上的深灰/黑色连通块。",
        "  3. 先用 gray < 150 提取暗色区域，再做开闭运算去除孤立噪声。",
        "  4. 用连通块面积、宽高、填充率过滤小高光/边缘噪声。",
        "  5. 楔形经常是斜着进入画面，质心会偏移，所以用暗块顶边中心 top_center_x 判断左右。",
        "  6. top_center_x 小于图像中心认为 LEFT，大于图像中心认为 RIGHT。",
        "  7. 如果同一帧有左右两个有效暗块，则输出 LEFT, RIGHT，表示先左后右都在视野中。",
        "",
        "控制思路:",
        "  1. SEARCH_PVC_ENTRY 阶段仍用白色 PVC 大区域确认入口。",
        "  2. 进入 ON_PVC 后切换到本算法，识别黑色楔形的 side 和出现顺序。",
        "  3. 方向控制仍以 PVC 中心线/入口锁定航向为主，黑色楔形不作为路径中心。",
        "  4. 检测到 LEFT 时，按左桥策略给 roll_degree/腿长偏置；检测到 RIGHT 时，按右桥策略给偏置。",
        "  5. 每个楔形要加 local_s_mm 去重，同一侧暗块连续多帧只计一次。",
        "  6. 计数达到 3 个且走过固定桥长，或看到 PVC 出口后，降底盘并修正惯导。",
        "  7. 视觉短时丢失时保持最近一次侧别和 entry_yaw，最多盲跑一个桥间距。",
    ]

    (output_dir / "summary.txt").write_text("\n".join(lines), encoding="utf-8")
    (output_dir / "summary.json").write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8")


def main() -> None:
    ensure_dir(OUTPUT_DIR)
    results = [process_sample(sample, OUTPUT_DIR) for sample in SAMPLES]
    write_summary(results, OUTPUT_DIR)
    print(f"output: {OUTPUT_DIR}")
    print(f"ok: {sum(1 for r in results if r['ok'])}/{len(results)}")


if __name__ == "__main__":
    main()
