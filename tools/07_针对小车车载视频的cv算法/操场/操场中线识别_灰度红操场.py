"""灰度/红操场单帧白线识别。

面向低分辨率(如 96x60)与高噪声采集图像：
- 使用灰度自适应白线分割
- 对候选线增加亮度一致性与越界过滤
- 复用原有候选评分与可视化框架，便于和旧脚本对照
"""

from __future__ import annotations

import argparse
import csv
import importlib.util
import json
import sys
from dataclasses import asdict
from pathlib import Path
from typing import Any, Iterable

import cv2
import numpy as np


PROJECT_ROOT = Path(__file__).resolve().parents[3]
SINGLE_FRAME_SCRIPT = Path(__file__).with_name("操场中线识别.py")
DEFAULT_FRAME_DIR = PROJECT_ROOT / "data" / "frames" / "video_2026_05_01_10_01_22_from11s"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "single_gray_red_line_detect"


def load_single_frame_module() -> Any:
    spec = importlib.util.spec_from_file_location("playground_single_line", SINGLE_FRAME_SCRIPT)
    if spec is None or spec.loader is None:
        raise ImportError(f"cannot load {SINGLE_FRAME_SCRIPT}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


single = load_single_frame_module()


def _line_x_at_robust(points: list[tuple[int, int]], y: float) -> float | None:
    if not points:
        return None

    pts = np.asarray(points, dtype=np.float32)
    xs = pts[:, 0]
    ys = pts[:, 1]
    if pts.shape[0] >= 6 and float(ys.max() - ys.min()) > 8.0:
        coeff = np.polyfit(ys, xs, 1)
        x = float(np.polyval(coeff, y))
        x_min = float(xs.min()) - 8.0
        x_max = float(xs.max()) + 8.0
        return float(np.clip(x, x_min, x_max))

    near = np.argsort(np.abs(ys - y))[: min(5, len(points))]
    return float(xs[near].mean())


def _row_centers_for_label_stable(labels: np.ndarray, label_id: int, y_min: int, y_max: int) -> list[tuple[int, int]]:
    """Trace one stable trunk through noisy connected component rows."""
    image_width = labels.shape[1]
    image_center = image_width * 0.5
    min_run_width = 2
    max_jump = max(10, int(image_width * 0.10))
    centers: list[tuple[int, int]] = []
    prev_x: float | None = None

    for y in range(y_max, y_min - 1, -1):
        xs = np.where(labels[y] == label_id)[0]
        if xs.size < min_run_width:
            continue

        breaks = np.where(np.diff(xs) > 1)[0] + 1
        runs = np.split(xs, breaks)
        run_info: list[tuple[float, int]] = []
        for run in runs:
            if run.size == 0:
                continue
            width = int(run[-1] - run[0] + 1)
            if width < min_run_width:
                continue
            center = float((int(run[0]) + int(run[-1])) * 0.5)
            run_info.append((center, width))

        if not run_info:
            continue

        if prev_x is None:
            center, _width = max(
                run_info,
                key=lambda item: item[1] - 0.35 * abs(item[0] - image_center),
            )
        else:
            nearby = [item for item in run_info if abs(item[0] - prev_x) <= max_jump]
            pool = nearby if nearby else run_info
            center, _width = max(
                pool,
                key=lambda item: item[1] - 0.90 * abs(item[0] - prev_x),
            )

        prev_x = center
        centers.append((int(round(center)), y))

    centers.reverse()
    if len(centers) >= 7:
        xs = np.asarray([p[0] for p in centers], dtype=np.float32)
        ys = [p[1] for p in centers]
        kernel = np.ones(5, dtype=np.float32) / 5.0
        xs_pad = np.pad(xs, (2, 2), mode="edge")
        xs_smooth = np.convolve(xs_pad, kernel, mode="valid")
        centers = [(int(round(x)), y) for x, y in zip(xs_smooth, ys)]
    return centers


# Patch line sampling to avoid quadratic overshoot on noisy low-res images.
single._line_x_at = _line_x_at_robust
single._row_centers_for_label = _row_centers_for_label_stable


def _normalize_to_u8(image: np.ndarray) -> np.ndarray:
    img = image.astype(np.float32)
    min_v = float(np.min(img))
    max_v = float(np.max(img))
    if max_v <= min_v + 1e-6:
        return np.zeros_like(image, dtype=np.uint8)
    norm = (img - min_v) * (255.0 / (max_v - min_v))
    return np.clip(norm, 0, 255).astype(np.uint8)


def _keep_main_component(mask: np.ndarray) -> tuple[np.ndarray, dict[str, float]]:
    h, w = mask.shape
    n_labels, labels, stats, centroids = cv2.connectedComponentsWithStats((mask > 0).astype(np.uint8), 8)
    out = np.zeros_like(mask)
    best_label = -1
    best_score = -1e9

    for i in range(1, n_labels):
        x, y, comp_w, comp_h, area = [int(v) for v in stats[i]]
        if area < max(90, int(h * w * 0.012)):
            continue
        if comp_h < max(14, int(h * 0.22)):
            continue

        cx = float(centroids[i][0])
        center_score = 1.0 - abs(cx - w * 0.5) / max(1.0, w * 0.5)
        top_touch = 1.0 if y <= int(h * 0.14) else 0.0
        tall_score = comp_h / max(1.0, float(h))
        area_score = area / max(1.0, float(h * w))
        width_penalty = comp_w / max(1.0, float(w))
        score = 1.35 * tall_score + 0.9 * center_score + 1.0 * top_touch + 0.25 * area_score - 0.5 * width_penalty
        if score > best_score:
            best_score = score
            best_label = i

    if best_label > 0:
        out[labels == best_label] = 255

    return out, {"main_label_score": float(best_score), "components": float(n_labels - 1)}


def build_white_line_mask_gray_red(image_bgr: np.ndarray) -> tuple[np.ndarray, dict[str, float], dict[str, np.ndarray]]:
    gray_raw = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2GRAY)
    gray_blur = cv2.GaussianBlur(gray_raw, (3, 3), 0)
    gray_denoised = cv2.fastNlMeansDenoising(gray_blur, None, 9, 7, 21)

    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    gray_eq = clahe.apply(gray_denoised)

    local_bg = cv2.GaussianBlur(gray_eq, (0, 0), sigmaX=7.0, sigmaY=7.0)
    local_delta = cv2.subtract(gray_eq, local_bg)
    top_hat = cv2.morphologyEx(
        gray_eq,
        cv2.MORPH_TOPHAT,
        cv2.getStructuringElement(cv2.MORPH_RECT, (9, 9)),
    )

    q_hi = float(np.percentile(gray_eq, 88.0))
    q_delta = float(np.percentile(local_delta, 80.0))
    q_top = float(np.percentile(top_hat, 85.0))

    bright_core = (gray_eq >= max(148.0, q_hi)) & (local_delta >= 2)
    local_white = local_delta >= max(7.0, q_delta * 0.86)
    tophat_white = top_hat >= max(10.0, q_top * 0.72)
    # User decision: use bright_core directly as final mask.
    mask_raw = (bright_core.astype(np.uint8)) * 255
    mask_morph = mask_raw.copy()
    mask_main = mask_raw.copy()
    mask_vert = mask_raw.copy()
    mask_final = mask_raw.copy()
    comp_stats = {"main_label_score": 0.0, "components": 0.0}

    stats = {
        "white_ratio": float((mask_final > 0).mean()),
        "gray_mean": float(gray_raw.mean()),
        "gray_eq_mean": float(gray_eq.mean()),
        "local_delta_mean": float(local_delta.mean()),
        "top_hat_mean": float(top_hat.mean()),
        "q_hi": q_hi,
        "q_delta": q_delta,
        "q_top": q_top,
        "main_label_score": comp_stats["main_label_score"],
        "components": comp_stats["components"],
    }

    process_images = {
        "01_gray_raw": gray_raw,
        "02_gray_blur": gray_blur,
        "03_gray_denoised": gray_denoised,
        "04_gray_eq": gray_eq,
        "05_local_delta": _normalize_to_u8(local_delta),
        "06_top_hat": _normalize_to_u8(top_hat),
        "07_bright_core": bright_core.astype(np.uint8) * 255,
        "08_local_white": local_white.astype(np.uint8) * 255,
        "09_tophat_white": tophat_white.astype(np.uint8) * 255,
        "10_mask_raw": mask_raw,
        "11_mask_morph": mask_morph,
        "12_mask_main": mask_main,
        "13_mask_vertical": mask_vert,
        "14_mask_final": mask_final,
    }
    return mask_final, stats, process_images


def filter_candidates_gray_red(candidates: list[Any], image_bgr: np.ndarray) -> list[Any]:
    if not candidates:
        return candidates

    gray = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2GRAY)
    h, w = gray.shape
    global_mean = float(gray.mean())

    filtered: list[Any] = []
    for cand in candidates:
        bx = float(cand.bottom_x)
        lx = float(cand.lookahead_x)
        tx = float(cand.top_x)

        if bx < -0.1 * w or bx > 1.1 * w:
            continue
        if lx < -0.1 * w or lx > 1.1 * w:
            continue
        if tx < -0.2 * w or tx > 1.2 * w:
            continue
        if abs(bx - lx) > w * 0.7:
            continue

        # Cross line / paint block suppression.
        if cand.width > w * 0.56 and cand.height < h * 0.88:
            continue

        sample_vals: list[float] = []
        step = max(1, len(cand.line_points) // 24)
        for x_i, y_i in cand.line_points[::step]:
            if y_i < int(h * 0.52):
                continue
            xi = int(np.clip(x_i, 0, w - 1))
            yi = int(np.clip(y_i, 0, h - 1))
            sample_vals.append(float(gray[yi, xi]))

        if sample_vals:
            line_mean = float(np.mean(sample_vals))
            if line_mean < global_mean + 8.0:
                continue

        cand.bottom_x = float(np.clip(bx, 0.0, w - 1.0))
        cand.lookahead_x = float(np.clip(lx, 0.0, w - 1.0))
        cand.top_x = float(np.clip(tx, 0.0, w - 1.0))
        filtered.append(cand)

    filtered.sort(key=lambda c: c.score, reverse=True)
    return filtered


def resolve_frames(frame_args: Iterable[str], frame_dir: Path) -> list[Path]:
    if frame_args:
        frames = [Path(p) for p in frame_args]
        return [p if p.is_absolute() else frame_dir / p for p in frames]
    return sorted(frame_dir.glob("frame_*.png"))


def detect_one(frame_path: Path, output_dir: Path, min_score: float) -> tuple[Any, dict]:
    image = single.imread_unicode(frame_path)
    mask, mask_stats, process_images = build_white_line_mask_gray_red(image)
    candidates = single.find_line_candidates(mask)
    candidates = filter_candidates_gray_red(candidates, image)
    selected = candidates[0] if candidates and candidates[0].score >= min_score else None

    overlay = single.draw_overlay(image, mask, candidates, selected, frame_path.name)
    candidate_debug = single.draw_candidate_debug(mask, candidates, selected)

    overlay_path = output_dir / "overlay" / f"{frame_path.stem}_overlay.png"
    mask_path = output_dir / "mask" / f"{frame_path.stem}_mask.png"
    candidate_path = output_dir / "candidate_debug" / f"{frame_path.stem}_candidates.png"
    single.imwrite_unicode(overlay_path, overlay)
    single.imwrite_unicode(mask_path, mask)
    single.imwrite_unicode(candidate_path, candidate_debug)

    process_dir = output_dir / "process" / frame_path.stem
    single.ensure_dir(process_dir)
    for name, proc_img in process_images.items():
        proc_path = process_dir / f"{name}.png"
        single.imwrite_unicode(proc_path, proc_img)

    if selected is not None:
        yaw = single.compute_line_yaw_deg(selected, image.shape[0])
        lateral_error = selected.lookahead_x - image.shape[1] * 0.5
        result = single.DetectionResult(
            frame=str(frame_path),
            detected=True,
            selected_score=selected.score,
            line_x_bottom=selected.bottom_x,
            line_x_lookahead=selected.lookahead_x,
            line_yaw_deg=yaw,
            lateral_error_px=lateral_error,
            candidates=len(candidates),
            image_width=image.shape[1],
            image_height=image.shape[0],
            output_overlay=str(overlay_path),
            output_mask=str(mask_path),
        )
    else:
        result = single.DetectionResult(
            frame=str(frame_path),
            detected=False,
            selected_score=None,
            line_x_bottom=None,
            line_x_lookahead=None,
            line_yaw_deg=None,
            lateral_error_px=None,
            candidates=len(candidates),
            image_width=image.shape[1],
            image_height=image.shape[0],
            output_overlay=str(overlay_path),
            output_mask=str(mask_path),
        )

    debug = {
        "mask_stats": mask_stats,
        "candidates": [
            {
                **asdict(cand),
                "line_points": cand.line_points[:: max(1, len(cand.line_points) // 30)],
            }
            for cand in candidates[:12]
        ],
    }
    return result, debug


def save_summary(output_dir: Path, results: list[Any], debug_by_frame: dict[str, dict]) -> None:
    summary_json = {
        "results": [asdict(r) for r in results],
        "debug": debug_by_frame,
    }
    (output_dir / "summary.json").write_text(json.dumps(summary_json, indent=2, ensure_ascii=False), encoding="utf-8")

    with (output_dir / "summary.csv").open("w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "frame",
                "detected",
                "selected_score",
                "line_x_bottom",
                "line_x_lookahead",
                "line_yaw_deg",
                "lateral_error_px",
                "candidates",
                "image_width",
                "image_height",
                "output_overlay",
                "output_mask",
            ],
        )
        writer.writeheader()
        for result in results:
            writer.writerow(asdict(result))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="灰度/红操场白线单帧识别")
    parser.add_argument("--frame-dir", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--frames", nargs="*", default=[])
    parser.add_argument("--min-score", type=float, default=0.40)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    output_dir: Path = args.output
    single.ensure_dir(output_dir)
    single.ensure_dir(output_dir / "overlay")
    single.ensure_dir(output_dir / "mask")
    single.ensure_dir(output_dir / "candidate_debug")
    single.ensure_dir(output_dir / "process")

    frames = resolve_frames(args.frames, args.frame_dir)
    results: list[Any] = []
    debug_by_frame: dict[str, dict] = {}

    for frame_path in frames:
        if not frame_path.exists():
            raise FileNotFoundError(frame_path)
        result, debug = detect_one(frame_path, output_dir, args.min_score)
        results.append(result)
        debug_by_frame[frame_path.name] = debug

    save_summary(output_dir, results, debug_by_frame)
    print(
        json.dumps(
            {"output_dir": str(output_dir), "frames": len(results), "detected": int(sum(r.detected for r in results))},
            indent=2,
            ensure_ascii=False,
        )
    )


if __name__ == "__main__":
    main()
