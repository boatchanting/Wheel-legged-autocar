"""蓝色操场白色单线寻迹样张检测。

当前脚本面向已经抽帧得到的蓝色操场视频帧，目标是从多条白色跑道线、
起跑线交叉和零散里程标记中选出一条最适合小车跟随的纵向白线。

默认输入为本次挑选的 10 张样张，默认输出到 data/蓝色操场/操场中线识别。
"""

from __future__ import annotations

import argparse
import csv
import json
import math
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable

import cv2
import numpy as np


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_FRAME_DIR = PROJECT_ROOT / "data" / "frames" / "蓝色操场"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "蓝色操场" / "操场中线识别"

DEFAULT_SAMPLE_NAMES = (
    "frame_002793.png",
    "frame_002811.png",
    "frame_002217.png",
    "frame_002325.png",
    "frame_002433.png",
    "frame_002541.png",
    "frame_002721.png",
    "frame_002739.png",
    "frame_002763.png",
    "frame_002781.png",
)


@dataclass
class LineCandidate:
    label_id: int
    area: int
    bbox: tuple[int, int, int, int]
    centroid: tuple[float, float]
    bottom_x: float
    top_x: float
    lookahead_x: float
    y_min: int
    y_max: int
    height: int
    width: int
    row_coverage: float
    verticality: float
    lower_presence: float
    center_score: float
    height_score: float
    thin_score: float
    score: float
    line_points: list[tuple[int, int]]


@dataclass
class DetectionResult:
    frame: str
    detected: bool
    selected_score: float | None
    line_x_bottom: float | None
    line_x_lookahead: float | None
    line_yaw_deg: float | None
    lateral_error_px: float | None
    candidates: int
    image_width: int
    image_height: int
    output_overlay: str
    output_mask: str


def imread_unicode(path: Path) -> np.ndarray:
    data = np.fromfile(str(path), dtype=np.uint8)
    image = cv2.imdecode(data, cv2.IMREAD_COLOR)
    if image is None:
        raise RuntimeError(f"failed to read image: {path}")
    return image


def imwrite_unicode(path: Path, image: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    ext = path.suffix or ".png"
    ok, encoded = cv2.imencode(ext, image)
    if not ok:
        raise RuntimeError(f"failed to encode image: {path}")
    encoded.tofile(str(path))


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def build_white_line_mask(image_bgr: np.ndarray) -> tuple[np.ndarray, dict[str, float]]:
    """分割蓝色操场上的白线。

    白线特点是亮度高、饱和度低；蓝色地面亮度可能也高，但饱和度明显更高。
    为了保留阴影中的白线，阈值做成两路合并，而不是只用固定灰度阈值。
    """

    hsv = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2HSV)
    gray = cv2.cvtColor(image_bgr, cv2.COLOR_BGR2GRAY)

    h_ch, s_ch, v_ch = cv2.split(hsv)
    _ = h_ch

    strict_white = (s_ch < 70) & (v_ch > 145)
    bright_low_sat = (s_ch < 105) & (gray > 155) & (v_ch > 150)

    # 动态光照下，远处/右侧白线可能不是“绝对亮”，但仍然比周围蓝色地面更亮、
    # 饱和度更低。用局部背景差分和 top-hat 补这一类低对比白线。
    local_bg = cv2.GaussianBlur(gray, (0, 0), sigmaX=13.0, sigmaY=13.0)
    local_delta = cv2.subtract(gray, local_bg)
    top_hat_kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (17, 17))
    top_hat = cv2.morphologyEx(gray, cv2.MORPH_TOPHAT, top_hat_kernel)
    local_low_sat = (
        ((local_delta > 7) | (top_hat > 16))
        & (s_ch < 138)
        & (v_ch > 92)
        & (gray > 82)
    )

    white_mask = (strict_white | bright_low_sat | local_low_sat).astype(np.uint8) * 255

    # 去除小亮斑，连接纵向白线中的轻微断裂。
    open_kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
    close_kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 13))
    white_mask = cv2.morphologyEx(white_mask, cv2.MORPH_OPEN, open_kernel, iterations=1)
    white_mask = cv2.morphologyEx(white_mask, cv2.MORPH_CLOSE, close_kernel, iterations=1)

    # 上方远处交叉线容易粘连，先保留但降低候选评分；底部车前线更重要。
    stats = {
        "white_ratio": float((white_mask > 0).mean()),
        "local_low_sat_ratio": float(local_low_sat.mean()),
        "gray_mean": float(gray.mean()),
        "sat_mean": float(s_ch.mean()),
    }
    return white_mask, stats


def _row_centers_for_label(labels: np.ndarray, label_id: int, y_min: int, y_max: int) -> list[tuple[int, int]]:
    """Extract one vertical branch from a connected component.

    起跑线会和纵向跑道线粘成同一个连通域。如果每行直接取所有白点的
    中位数，横线会把中心点拉到画面中间。这里改成从车前方近处往远处
    追踪同一条分支，遇到明显横向长条时跳过该行。
    """

    image_width = labels.shape[1]
    max_run_width = max(45, int(image_width * 0.08))
    centers: list[tuple[int, int]] = []
    prev_x: float | None = None

    for y in range(y_max, y_min - 1, -1):
        xs = np.where(labels[y] == label_id)[0]
        if xs.size == 0:
            continue

        total_span = int(xs[-1] - xs[0] + 1)
        if total_span > int(image_width * 0.22) or xs.size > int(max_run_width * 1.8):
            # Fragmented horizontal paint can be split into several short runs,
            # so single-run width is not enough to reject it.
            continue

        # Split row pixels into contiguous runs.
        breaks = np.where(np.diff(xs) > 1)[0] + 1
        runs = np.split(xs, breaks)
        run_info: list[tuple[float, int, int, int]] = []
        for run in runs:
            if run.size == 0:
                continue
            center = float((int(run[0]) + int(run[-1])) * 0.5)
            width = int(run[-1] - run[0] + 1)
            run_info.append((center, width, int(run[0]), int(run[-1])))

        if not run_info:
            continue

        if prev_x is None:
            # At the bottom of the image the line closest to image center is the
            # most useful initial branch for steering.
            target_x = image_width * 0.5
        else:
            target_x = prev_x

        center, width, x0, x1 = min(run_info, key=lambda item: abs(item[0] - target_x))

        if width > max_run_width:
            # This is usually a start line / mileage mark crossing the lane.
            # Keep tracking continuity but do not feed the wide row to fitting.
            if prev_x is None:
                prev_x = center
            continue

        prev_x = center
        centers.append((int(round(center)), y))

    centers.reverse()
    return centers


def _line_x_at(points: list[tuple[int, int]], y: float) -> float | None:
    if not points:
        return None

    pts = np.asarray(points, dtype=np.float32)
    xs = pts[:, 0]
    ys = pts[:, 1]
    if len(points) >= 8 and float(ys.max() - ys.min()) > 10.0:
        degree = 2 if len(points) >= 18 else 1
        coeff = np.polyfit(ys, xs, degree)
        return float(np.polyval(coeff, y))

    near = np.argsort(np.abs(ys - y))[: min(5, len(points))]
    return float(xs[near].mean())


def _component_verticality(xs: np.ndarray, ys: np.ndarray) -> float:
    if xs.size < 8:
        return 0.0

    centered = np.column_stack((xs.astype(np.float32), ys.astype(np.float32)))
    centered -= centered.mean(axis=0, keepdims=True)
    cov = centered.T @ centered / max(1, centered.shape[0] - 1)
    eigvals, eigvecs = np.linalg.eigh(cov)
    main_vec = eigvecs[:, int(np.argmax(eigvals))]
    vx, vy = float(main_vec[0]), float(main_vec[1])
    return abs(vy) / (abs(vx) + abs(vy) + 1e-6)


def find_line_candidates(mask: np.ndarray) -> list[LineCandidate]:
    h, w = mask.shape
    num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats((mask > 0).astype(np.uint8), 8)
    candidates: list[LineCandidate] = []
    image_center_x = w * 0.5

    for label_id in range(1, num_labels):
        x, y, comp_w, comp_h, area = [int(v) for v in stats[label_id]]
        if area < 180:
            continue
        if comp_h < max(45, int(h * 0.12)):
            # 起跑线、里程短标记通常高度很小，直接排除。
            continue
        if comp_w > int(w * 0.55) and comp_h < int(h * 0.45):
            continue

        ys, xs = np.where(labels == label_id)
        if xs.size == 0:
            continue

        y_min = int(ys.min())
        y_max = int(ys.max())
        x_min = int(xs.min())
        x_max = int(xs.max())
        height = y_max - y_min + 1
        width = x_max - x_min + 1
        if height <= 0 or width <= 0:
            continue

        line_points = _row_centers_for_label(labels, label_id, y_min, y_max)
        if len(line_points) < 20:
            continue

        bottom_query_y = min(h - 1, max(y_min, int(h * 0.93)))
        lookahead_y = min(h - 1, max(y_min, int(h * 0.72)))
        top_query_y = max(y_min, int(h * 0.22))

        bottom_x = _line_x_at(line_points, bottom_query_y)
        lookahead_x = _line_x_at(line_points, lookahead_y)
        top_x = _line_x_at(line_points, top_query_y)
        if bottom_x is None or lookahead_x is None or top_x is None:
            continue

        row_count = len({p[1] for p in line_points})
        row_coverage = row_count / max(1, height)
        verticality = _component_verticality(xs, ys)
        lower_presence = y_max / max(1.0, float(h - 1))
        center_distance = abs(bottom_x - image_center_x)

        center_score = max(0.0, 1.0 - center_distance / (w * 0.42))
        height_score = min(1.0, height / (h * 0.72))
        lower_score = min(1.0, lower_presence)
        coverage_score = min(1.0, row_coverage / 0.82)
        vertical_score = min(1.0, verticality / 0.72)
        thin_score = max(0.0, 1.0 - width / (w * 0.42))

        # 选择策略：车前方能看到、接近画面中部、纵向连续的白线优先。
        score = (
            0.34 * center_score
            + 0.20 * lower_score
            + 0.18 * height_score
            + 0.14 * vertical_score
            + 0.09 * coverage_score
            + 0.05 * thin_score
        )

        # 横向起跑线或白色字块偶尔会形成大连通块；纵向主线分数应明显更高。
        if verticality < 0.42 and height < h * 0.65:
            score *= 0.45

        candidates.append(
            LineCandidate(
                label_id=label_id,
                area=area,
                bbox=(x_min, y_min, x_max, y_max),
                centroid=(float(centroids[label_id][0]), float(centroids[label_id][1])),
                bottom_x=float(bottom_x),
                top_x=float(top_x),
                lookahead_x=float(lookahead_x),
                y_min=y_min,
                y_max=y_max,
                height=height,
                width=width,
                row_coverage=float(row_coverage),
                verticality=float(verticality),
                lower_presence=float(lower_presence),
                center_score=float(center_score),
                height_score=float(height_score),
                thin_score=float(thin_score),
                score=float(score),
                line_points=line_points,
            )
        )

    candidates.sort(key=lambda c: c.score, reverse=True)
    return candidates


def compute_line_yaw_deg(candidate: LineCandidate, image_height: int) -> float:
    y1 = int(image_height * 0.93)
    y2 = int(image_height * 0.62)
    x1 = _line_x_at(candidate.line_points, y1)
    x2 = _line_x_at(candidate.line_points, y2)
    if x1 is None or x2 is None:
        return 0.0
    # 相对图像竖直方向的角度。正值表示线向右倾。
    return float(math.degrees(math.atan2(x2 - x1, y1 - y2)))


def draw_overlay(
    image_bgr: np.ndarray,
    mask: np.ndarray,
    candidates: list[LineCandidate],
    selected: LineCandidate | None,
    frame_name: str,
) -> np.ndarray:
    h, w = mask.shape
    overlay = image_bgr.copy()
    mask_color = np.zeros_like(overlay)
    mask_color[:, :, 1] = mask
    overlay = cv2.addWeighted(overlay, 0.82, mask_color, 0.18, 0)

    center_x = int(w * 0.5)
    cv2.line(overlay, (center_x, 0), (center_x, h - 1), (0, 255, 255), 1)
    cv2.line(overlay, (0, int(h * 0.72)), (w - 1, int(h * 0.72)), (255, 255, 0), 1)

    for idx, cand in enumerate(candidates[:6]):
        x1, y1, x2, y2 = cand.bbox
        color = (0, 180, 255)
        thickness = 1
        if cand is selected:
            color = (0, 0, 255)
            thickness = 2

        cv2.rectangle(overlay, (x1, y1), (x2, y2), color, thickness)
        cv2.putText(
            overlay,
            f"{idx + 1}:{cand.score:.2f}",
            (x1, max(14, y1 - 4)),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.46,
            color,
            1,
            cv2.LINE_AA,
        )

    if selected is not None:
        pts = np.asarray(selected.line_points, dtype=np.int32)
        if pts.shape[0] >= 2:
            # points are (x, y)
            cv2.polylines(overlay, [pts.reshape((-1, 1, 2))], False, (0, 0, 255), 3, cv2.LINE_AA)

        lookahead_y = int(h * 0.72)
        bottom_y = int(h * 0.93)
        cv2.circle(overlay, (int(round(selected.lookahead_x)), lookahead_y), 7, (255, 0, 255), -1)
        cv2.circle(overlay, (int(round(selected.bottom_x)), bottom_y), 6, (0, 255, 255), -1)

        yaw = compute_line_yaw_deg(selected, h)
        lateral_error = selected.lookahead_x - center_x
        title = (
            f"{frame_name} detected score={selected.score:.2f} "
            f"err={lateral_error:.1f}px yaw={yaw:.1f}deg"
        )
    else:
        title = f"{frame_name} no line"

    cv2.rectangle(overlay, (0, 0), (w - 1, 28), (0, 0, 0), -1)
    cv2.putText(overlay, title, (8, 19), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 1, cv2.LINE_AA)
    return overlay


def draw_candidate_debug(mask: np.ndarray, candidates: list[LineCandidate], selected: LineCandidate | None) -> np.ndarray:
    debug = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
    h, w = mask.shape
    cv2.line(debug, (int(w * 0.5), 0), (int(w * 0.5), h - 1), (0, 255, 255), 1)

    for cand in candidates[:10]:
        x1, y1, x2, y2 = cand.bbox
        color = (0, 128, 255)
        if cand is selected:
            color = (0, 0, 255)
        cv2.rectangle(debug, (x1, y1), (x2, y2), color, 1)
        cv2.putText(debug, f"{cand.score:.2f}", (x1, min(h - 4, y2 + 14)), cv2.FONT_HERSHEY_SIMPLEX, 0.42, color, 1)
    return debug


def resolve_frames(frame_args: Iterable[str], frame_dir: Path) -> list[Path]:
    if frame_args:
        frames = [Path(p) for p in frame_args]
        return [p if p.is_absolute() else frame_dir / p for p in frames]
    return [frame_dir / name for name in DEFAULT_SAMPLE_NAMES]


def detect_one(frame_path: Path, output_dir: Path, min_score: float) -> tuple[DetectionResult, dict]:
    image = imread_unicode(frame_path)
    mask, mask_stats = build_white_line_mask(image)
    candidates = find_line_candidates(mask)
    selected = candidates[0] if candidates and candidates[0].score >= min_score else None

    overlay = draw_overlay(image, mask, candidates, selected, frame_path.name)
    candidate_debug = draw_candidate_debug(mask, candidates, selected)

    overlay_path = output_dir / "overlay" / f"{frame_path.stem}_overlay.png"
    mask_path = output_dir / "mask" / f"{frame_path.stem}_mask.png"
    candidate_path = output_dir / "candidate_debug" / f"{frame_path.stem}_candidates.png"
    imwrite_unicode(overlay_path, overlay)
    imwrite_unicode(mask_path, mask)
    imwrite_unicode(candidate_path, candidate_debug)

    if selected is not None:
        yaw = compute_line_yaw_deg(selected, image.shape[0])
        lateral_error = selected.lookahead_x - image.shape[1] * 0.5
        result = DetectionResult(
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
        result = DetectionResult(
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


def save_summary(output_dir: Path, results: list[DetectionResult], debug_by_frame: dict[str, dict]) -> None:
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
    parser = argparse.ArgumentParser(description="蓝色操场白色单线寻迹样张检测")
    parser.add_argument("--frame-dir", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--frames", nargs="*", default=[])
    parser.add_argument("--min-score", type=float, default=0.48)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    output_dir: Path = args.output
    ensure_dir(output_dir)
    ensure_dir(output_dir / "overlay")
    ensure_dir(output_dir / "mask")
    ensure_dir(output_dir / "candidate_debug")

    frames = resolve_frames(args.frames, args.frame_dir)
    results: list[DetectionResult] = []
    debug_by_frame: dict[str, dict] = {}

    for frame_path in frames:
        if not frame_path.exists():
            raise FileNotFoundError(frame_path)
        result, debug = detect_one(frame_path, output_dir, args.min_score)
        results.append(result)
        debug_by_frame[frame_path.name] = debug

    save_summary(output_dir, results, debug_by_frame)

    print(json.dumps({"output_dir": str(output_dir), "frames": len(results), "detected": sum(r.detected for r in results)}, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
