"""小车近端直线巡线检测。

输入是 extract_video_frames.py 拆出来的 94x60 PNG 帧序列。
算法只使用图像下半部分的近端区域，寻找白色赛道/PVC 区域的中心走向；
远端像素默认不参与拟合，避免远处曝光、杂物和透视压缩影响控制。

输出内容：
- overlays/: 每帧可视化结果。
- masks/: 每帧近端白色区域 mask。
- line_results.csv: 每帧控制量和评分。
- line_summary.json: 汇总统计。

控制层建议只使用 detected=True 的帧：
- lateral_error_px：近端中心线相对图像中心的横向误差，正数表示线在右侧。
- yaw_error_deg：中心线相对图像竖直方向的角度，正数表示线向右偏。
- confidence：0~1，高于 min_confidence 才输出 detected=True。
"""

from __future__ import annotations

import argparse
import csv
import json
import math
from dataclasses import asdict, dataclass
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_FRAME_DIR = PROJECT_ROOT / "data" / "frames" / "寻直线"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "寻直线"


@dataclass
class StraightLineResult:
    frame: str
    detected: bool
    confidence: float
    bridge_detected: bool
    bridge_confidence: float
    bridge_components: int
    lateral_error_px: float | None
    yaw_error_deg: float | None
    target_speed_hint: float
    line_x_bottom: float | None
    line_x_lookahead: float | None
    points_used: int
    y_span: int
    fit_rmse: float | None
    mean_track_width: float | None
    roi_white_ratio: float
    output_overlay: str
    output_mask: str


@dataclass
class BridgeComponent:
    area: int
    bbox: tuple[int, int, int, int]
    mean_gray: float
    fill_ratio: float
    score: float


def imread_rgb(path: Path) -> np.ndarray:
    return np.asarray(Image.open(path).convert("RGB"), dtype=np.uint8)


def imwrite_rgb(path: Path, image: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(image.astype(np.uint8), mode="RGB").save(path)


def imwrite_gray(path: Path, image: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(image.astype(np.uint8), mode="L").save(path)


def ensure_clean_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)
    for child in path.glob("*"):
        if child.is_file():
            child.unlink()


def find_runs(mask_row: np.ndarray) -> list[tuple[int, int]]:
    xs = np.where(mask_row > 0)[0]
    if xs.size == 0:
        return []
    breaks = np.where(np.diff(xs) > 1)[0] + 1
    runs: list[tuple[int, int]] = []
    for run in np.split(xs, breaks):
        if run.size:
            runs.append((int(run[0]), int(run[-1])))
    return runs


def binary_dilate(mask: np.ndarray, kernel_w: int, kernel_h: int) -> np.ndarray:
    pad_x = kernel_w // 2
    pad_y = kernel_h // 2
    padded = np.pad(mask > 0, ((pad_y, pad_y), (pad_x, pad_x)), mode="constant")
    out = np.zeros_like(mask, dtype=bool)
    for dy in range(kernel_h):
        for dx in range(kernel_w):
            out |= padded[dy : dy + mask.shape[0], dx : dx + mask.shape[1]]
    return out.astype(np.uint8) * 255


def binary_erode(mask: np.ndarray, kernel_w: int, kernel_h: int) -> np.ndarray:
    pad_x = kernel_w // 2
    pad_y = kernel_h // 2
    padded = np.pad(mask > 0, ((pad_y, pad_y), (pad_x, pad_x)), mode="constant")
    out = np.ones_like(mask, dtype=bool)
    for dy in range(kernel_h):
        for dx in range(kernel_w):
            out &= padded[dy : dy + mask.shape[0], dx : dx + mask.shape[1]]
    return out.astype(np.uint8) * 255


def binary_open(mask: np.ndarray, kernel_w: int, kernel_h: int) -> np.ndarray:
    return binary_dilate(binary_erode(mask, kernel_w, kernel_h), kernel_w, kernel_h)


def binary_close(mask: np.ndarray, kernel_w: int, kernel_h: int) -> np.ndarray:
    return binary_erode(binary_dilate(mask, kernel_w, kernel_h), kernel_w, kernel_h)


def find_dark_components(gray: np.ndarray, y_min: int, y_max: int, threshold: int = 180) -> list[BridgeComponent]:
    """检测视野上方/中部的黑色单边桥块。

    参考 detect_single_side_bridge_samples.py 的思路：
    - 单边桥是白色 PVC/赛道上的深色连通块。
    - 先用灰度阈值提取暗块，再按面积、宽高、填充率、亮度评分。
    - 这里只判断“是否出现单边桥”，不区分左右，也不做计数。
    """

    height, width = gray.shape
    mask = np.zeros_like(gray, dtype=np.uint8)
    mask[y_min : y_max + 1] = (gray[y_min : y_max + 1] < threshold).astype(np.uint8) * 255
    mask = binary_open(mask, 2, 2)
    mask = binary_close(mask, 3, 3)

    visited = np.zeros_like(mask, dtype=bool)
    components: list[BridgeComponent] = []

    for sy in range(y_min, y_max + 1):
        for sx in range(width):
            if mask[sy, sx] == 0 or visited[sy, sx]:
                continue

            stack = [(sx, sy)]
            visited[sy, sx] = True
            xs: list[int] = []
            ys: list[int] = []

            while stack:
                x, y = stack.pop()
                xs.append(x)
                ys.append(y)
                for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                    if 0 <= nx < width and y_min <= ny <= y_max and mask[ny, nx] and not visited[ny, nx]:
                        visited[ny, nx] = True
                        stack.append((nx, ny))

            area = len(xs)
            if area < 35:
                continue

            xmin, xmax = min(xs), max(xs)
            ymin, ymax = min(ys), max(ys)
            comp_w = xmax - xmin + 1
            comp_h = ymax - ymin + 1
            if comp_w < 18 or comp_h < 4:
                continue

            fill_ratio = area / max(1, comp_w * comp_h)
            if fill_ratio < 0.22:
                continue

            pixels = gray[np.asarray(ys), np.asarray(xs)]
            mean_gray = float(pixels.mean())
            area_score = min(area / 260.0, 1.0)
            size_score = 0.5 * min(comp_w / 28.0, 1.0) + 0.5 * min(comp_h / 12.0, 1.0)
            dark_score = min(max((190.0 - mean_gray) / 70.0, 0.0), 1.0)
            top_score = max(0.0, 1.0 - ymin / max(1.0, height * 0.45))
            score = 0.38 * area_score + 0.28 * size_score + 0.22 * dark_score + 0.12 * top_score

            components.append(
                BridgeComponent(
                    area=area,
                    bbox=(xmin, ymin, xmax, ymax),
                    mean_gray=mean_gray,
                    fill_ratio=fill_ratio,
                    score=score,
                )
            )

    components.sort(key=lambda item: item.score, reverse=True)
    return components


def build_near_white_mask(gray: np.ndarray, y_min: int, y_max: int) -> tuple[np.ndarray, float]:
    """生成近端白色赛道 mask。

    这里不用固定全局阈值，而是每一行动态阈值：
    - 白色 PVC/赛道通常是该行里最亮的一段。
    - 地面、阴影会随距离和曝光变化，用 row_mean + margin 更稳。
    - 阈值封顶到 245，避免全白过曝时把整行都吃进去。
    """

    blur = np.asarray(Image.fromarray(gray).filter(ImageFilter.GaussianBlur(radius=1.1)), dtype=np.uint8)
    mask = np.zeros_like(gray, dtype=np.uint8)

    for y in range(y_min, y_max + 1):
        row = blur[y].astype(np.float32)
        row_mean = float(row.mean())
        row_p70 = float(np.percentile(row, 70))
        row_p92 = float(np.percentile(row, 92))
        threshold = max(145.0, min(245.0, max(row_mean + 8.0, row_p70 + 4.0, row_p92 - 22.0)))
        mask[y] = (row >= threshold).astype(np.uint8) * 255

    mask = binary_open(mask, 3, 2)
    mask = binary_close(mask, 7, 3)

    roi = mask[y_min : y_max + 1]
    white_ratio = float((roi > 0).mean()) if roi.size else 0.0
    return mask, white_ratio


def extract_center_points(
    mask: np.ndarray,
    y_min: int,
    y_max: int,
    min_width: int,
    max_width_ratio: float,
) -> tuple[list[tuple[float, float]], list[float]]:
    """逐行提取近端白色区域中心点。

    从车前最近处向远处跟踪同一块白色区域。每行可能有多个白色 run，
    选择离上一行中心最近的 run，避免被旁边反光块或远处杂物抢走。
    """

    height, width = mask.shape
    max_width = int(width * max_width_ratio)
    prev_center = width * 0.5
    points: list[tuple[float, float]] = []
    widths: list[float] = []
    lost_rows = 0

    for y in range(y_max, y_min - 1, -1):
        runs = []
        for x0, x1 in find_runs(mask[y]):
            run_width = x1 - x0 + 1
            if run_width < min_width:
                continue
            if run_width > max_width:
                # 整行过曝或近端全部白时，中心仍可能有用，但权重较低。
                # 这里先保留，不直接丢掉，否则车正对赛道时底部会没有点。
                pass
            center = (x0 + x1) * 0.5
            runs.append((center, run_width, x0, x1))

        if not runs:
            lost_rows += 1
            if lost_rows >= 4 and points:
                break
            continue

        center, run_width, _, _ = min(runs, key=lambda item: abs(item[0] - prev_center))
        if abs(center - prev_center) > width * 0.30 and points:
            lost_rows += 1
            continue

        lost_rows = 0
        prev_center = 0.70 * prev_center + 0.30 * center
        points.append((center, float(y)))
        widths.append(float(run_width))

    points.reverse()
    widths.reverse()
    return points, widths


def fit_line(points: list[tuple[float, float]]) -> tuple[float, float, float]:
    """拟合 x = k*y + b，并返回 k、b、rmse。"""

    pts = np.asarray(points, dtype=np.float32)
    xs = pts[:, 0]
    ys = pts[:, 1]
    k, b = np.polyfit(ys, xs, 1)
    pred = k * ys + b
    rmse = float(np.sqrt(np.mean((xs - pred) ** 2)))
    return float(k), float(b), rmse


def draw_circle(draw: ImageDraw.ImageDraw, x: float, y: float, radius: int, color: tuple[int, int, int]) -> None:
    draw.ellipse(
        (
            int(round(x)) - radius,
            int(round(y)) - radius,
            int(round(x)) + radius,
            int(round(y)) + radius,
        ),
        fill=color,
    )


def detect_straight_line(
    image_rgb: np.ndarray,
    min_confidence: float,
    roi_top_ratio: float,
    min_rows: int,
    min_width: int,
    min_y_span: int,
    max_abs_yaw_deg: float,
    bridge_min_confidence: float,
    bridge_speed_hint: float,
) -> tuple[StraightLineResult, np.ndarray, np.ndarray]:
    height, width = image_rgb.shape[:2]
    gray = (
        0.299 * image_rgb[:, :, 0]
        + 0.587 * image_rgb[:, :, 1]
        + 0.114 * image_rgb[:, :, 2]
    ).astype(np.uint8)

    y_min = int(height * roi_top_ratio)
    y_max = height - 2
    lookahead_y = int(height * 0.62)
    bottom_y = y_max
    bridge_y_min = int(height * 0.03)
    bridge_y_max = height - 2

    bridge_components = find_dark_components(gray, bridge_y_min, bridge_y_max)
    bridge_confidence = bridge_components[0].score if bridge_components else 0.0
    bridge_detected = bridge_confidence >= bridge_min_confidence

    mask, roi_white_ratio = build_near_white_mask(gray, y_min, y_max)
    points, widths = extract_center_points(
        mask=mask,
        y_min=y_min,
        y_max=y_max,
        min_width=min_width,
        max_width_ratio=0.96,
    )

    detected = False
    confidence = 0.0
    lateral_error_px: float | None = None
    yaw_error_deg: float | None = None
    line_x_bottom: float | None = None
    line_x_lookahead: float | None = None
    fit_rmse: float | None = None
    mean_track_width = float(np.mean(widths)) if widths else None
    y_span = 0
    target_speed_hint = 0.0

    overlay_img = Image.fromarray(image_rgb.copy(), mode="RGB")
    draw = ImageDraw.Draw(overlay_img)
    draw.rectangle((0, y_min, width - 1, y_max), outline=(255, 160, 0))
    draw.rectangle((0, bridge_y_min, width - 1, bridge_y_max), outline=(0, 128, 255))

    for x, y in points:
        draw_circle(draw, x, y, 1, (255, 255, 0))

    for comp in bridge_components[:3]:
        x0, y0, x1, y1 = comp.bbox
        box_color = (0, 255, 255) if comp.score >= bridge_min_confidence else (0, 80, 255)
        draw.rectangle((x0, y0, x1, y1), outline=box_color)
        draw.text((x0, max(0, y0 - 9)), f"B {comp.score:.2f}", fill=box_color)

    if len(points) >= 2:
        y_values = [p[1] for p in points]
        y_span = int(max(y_values) - min(y_values))

    if len(points) >= max(2, min_rows):
        k, b, rmse = fit_line(points)
        fit_rmse = rmse
        line_x_bottom = k * bottom_y + b
        line_x_lookahead = k * lookahead_y + b
        lateral_error_px = line_x_bottom - (width - 1) * 0.5
        yaw_error_deg = math.degrees(math.atan(k))

        row_score = min(len(points) / max(1.0, float(y_max - y_min + 1) * 0.70), 1.0)
        span_score = min(y_span / max(1.0, float(y_max - y_min) * 0.75), 1.0)
        rmse_score = max(0.0, 1.0 - rmse / 5.5)
        width_score = 1.0
        if mean_track_width is not None:
            width_score = min(mean_track_width / max(1.0, width * 0.45), 1.0)
        center_score = max(0.0, 1.0 - abs(float(lateral_error_px)) / (width * 0.55))

        confidence = (
            0.30 * row_score
            + 0.22 * span_score
            + 0.24 * rmse_score
            + 0.14 * width_score
            + 0.10 * center_score
        )
        detected = (
            confidence >= min_confidence
            and y_span >= min_y_span
            and abs(yaw_error_deg) <= max_abs_yaw_deg
        )

        if bridge_detected:
            detected = False
            confidence = 0.0
            lateral_error_px = 0.0
            yaw_error_deg = 0.0
            line_x_bottom = None
            line_x_lookahead = None
            target_speed_hint = bridge_speed_hint

        color = (0, 255, 0) if detected else (0, 0, 255)
        if line_x_bottom is not None and line_x_lookahead is not None:
            x0 = int(round(k * y_min + b))
            x1 = int(round(k * y_max + b))
            draw.line((x0, y_min, x1, y_max), fill=color, width=2)
            draw_circle(draw, line_x_bottom, bottom_y, 3, color)
            draw_circle(draw, line_x_lookahead, lookahead_y, 3, color)

    draw.line((width // 2, y_min, width // 2, y_max), fill=(255, 0, 255), width=1)
    text_lines = [
        f"det={int(detected)} conf={confidence:.2f} bridge={int(bridge_detected)} {bridge_confidence:.2f}",
        f"pts={len(points)} span={y_span} rmse={fit_rmse if fit_rmse is not None else -1:.1f}",
        f"lat={lateral_error_px if lateral_error_px is not None else 0:.1f}px yaw={yaw_error_deg if yaw_error_deg is not None else 0:.1f} spd={target_speed_hint:.0f}",
    ]
    for i, text in enumerate(text_lines):
        color = (255, 0, 0) if i == 0 and not detected else (0, 255, 0)
        draw.text((2, 1 + i * 9), text, fill=color)

    result = StraightLineResult(
        frame="",
        detected=detected,
        confidence=confidence,
        bridge_detected=bridge_detected,
        bridge_confidence=bridge_confidence,
        bridge_components=len(bridge_components),
        lateral_error_px=lateral_error_px if (detected or bridge_detected) else None,
        yaw_error_deg=yaw_error_deg if (detected or bridge_detected) else None,
        target_speed_hint=target_speed_hint,
        line_x_bottom=line_x_bottom if detected else None,
        line_x_lookahead=line_x_lookahead if detected else None,
        points_used=len(points),
        y_span=y_span,
        fit_rmse=fit_rmse,
        mean_track_width=mean_track_width,
        roi_white_ratio=roi_white_ratio,
        output_overlay="",
        output_mask="",
    )
    return result, np.asarray(overlay_img, dtype=np.uint8), mask


def process_frames(
    frame_dir: Path,
    output_dir: Path,
    min_confidence: float,
    roi_top_ratio: float,
    min_rows: int,
    min_width: int,
    min_y_span: int,
    max_abs_yaw_deg: float,
    bridge_min_confidence: float,
    bridge_speed_hint: float,
    max_frames: int | None,
) -> dict:
    overlay_dir = output_dir / "overlays"
    mask_dir = output_dir / "masks"
    ensure_clean_dir(overlay_dir)
    ensure_clean_dir(mask_dir)

    frames = sorted(frame_dir.glob("*.png"))
    if max_frames is not None:
        frames = frames[:max_frames]
    if not frames:
        raise RuntimeError(f"no png frames found in {frame_dir}")

    results: list[StraightLineResult] = []
    for index, frame_path in enumerate(frames, start=1):
        image = imread_rgb(frame_path)
        result, overlay, mask = detect_straight_line(
            image_rgb=image,
            min_confidence=min_confidence,
            roi_top_ratio=roi_top_ratio,
            min_rows=min_rows,
            min_width=min_width,
            min_y_span=min_y_span,
            max_abs_yaw_deg=max_abs_yaw_deg,
            bridge_min_confidence=bridge_min_confidence,
            bridge_speed_hint=bridge_speed_hint,
        )

        overlay_name = f"{frame_path.stem}_line.png"
        mask_name = f"{frame_path.stem}_mask.png"
        overlay_path = overlay_dir / overlay_name
        mask_path = mask_dir / mask_name
        imwrite_rgb(overlay_path, overlay)
        imwrite_gray(mask_path, mask)

        result.frame = frame_path.name
        result.output_overlay = str(overlay_path.relative_to(output_dir))
        result.output_mask = str(mask_path.relative_to(output_dir))
        results.append(result)

        if index % 100 == 0:
            print(f"processed {index}/{len(frames)} frames")

    csv_path = output_dir / "line_results.csv"
    with csv_path.open("w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(f, fieldnames=list(asdict(results[0]).keys()))
        writer.writeheader()
        for result in results:
            writer.writerow(asdict(result))

    detected_count = sum(1 for result in results if result.detected)
    bridge_count = sum(1 for result in results if result.bridge_detected)
    confidences = [result.confidence for result in results]
    detected_confidences = [result.confidence for result in results if result.detected]
    summary = {
        "frame_dir": str(frame_dir),
        "output_dir": str(output_dir),
        "frames_total": len(results),
        "frames_detected": detected_count,
        "frames_bridge_detected": bridge_count,
        "detected_ratio": detected_count / len(results),
        "bridge_detected_ratio": bridge_count / len(results),
        "min_confidence": min_confidence,
        "roi_top_ratio": roi_top_ratio,
        "min_rows": min_rows,
        "min_width": min_width,
        "min_y_span": min_y_span,
        "max_abs_yaw_deg": max_abs_yaw_deg,
        "bridge_min_confidence": bridge_min_confidence,
        "bridge_speed_hint": bridge_speed_hint,
        "confidence_avg": float(np.mean(confidences)) if confidences else 0.0,
        "detected_confidence_avg": float(np.mean(detected_confidences))
        if detected_confidences
        else 0.0,
        "csv": str(csv_path),
    }
    (output_dir / "line_summary.json").write_text(
        json.dumps(summary, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    return summary


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Detect high-confidence near straight line from 94x60 frames.")
    parser.add_argument("--frames", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--min-confidence", type=float, default=0.72)
    parser.add_argument("--roi-top-ratio", type=float, default=0.25, help="ignore pixels above height*ratio")
    parser.add_argument("--min-rows", type=int, default=15)
    parser.add_argument("--min-width", type=int, default=8)
    parser.add_argument("--min-y-span", type=int, default=22, help="minimum vertical span of fitted near line")
    parser.add_argument("--max-abs-yaw-deg", type=float, default=35.0, help="reject overly slanted lines")
    parser.add_argument("--bridge-min-confidence", type=float, default=0.56)
    parser.add_argument("--bridge-speed-hint", type=float, default=-90.0)
    parser.add_argument("--max-frames", type=int, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    summary = process_frames(
        frame_dir=args.frames,
        output_dir=args.output,
        min_confidence=args.min_confidence,
        roi_top_ratio=args.roi_top_ratio,
        min_rows=args.min_rows,
        min_width=args.min_width,
        min_y_span=args.min_y_span,
        max_abs_yaw_deg=args.max_abs_yaw_deg,
        bridge_min_confidence=args.bridge_min_confidence,
        bridge_speed_hint=args.bridge_speed_hint,
        max_frames=args.max_frames,
    )
    print(json.dumps(summary, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
