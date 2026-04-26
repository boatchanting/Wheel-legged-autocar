"""操场白线动态预瞄可视化。

这个脚本用于验证“视觉线 + 曲率 + 动态预瞄”的控制输入，不包含项目状态机。
输出视频会画出固定预瞄点、动态预瞄点、拟合线、曲率因子、建议速度和转向量。
"""

from __future__ import annotations

import argparse
import csv
import importlib.util
import json
import math
import shutil
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

import cv2
import numpy as np


PROJECT_ROOT = Path(__file__).resolve().parents[3]
SINGLE_FRAME_SCRIPT = Path(__file__).with_name("操场中线识别.py")
DEFAULT_FRAME_DIR = PROJECT_ROOT / "data" / "frames" / "蓝色操场"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "蓝色操场" / "操场动态预瞄可视化"


@dataclass
class TrackState:
    active: bool = False
    lost_count: int = 0
    bottom_x: float = 0.0
    lookahead_x: float = 0.0
    yaw_deg: float = 0.0
    score: float = 0.0
    curve_factor: float = 0.0
    dynamic_y_ratio: float = 0.72
    dynamic_x: float = 0.0
    speed_cmd: float = 0.0
    steer_norm: float = 0.0


@dataclass
class DynamicResult:
    frame_index: int
    frame_name: str
    mode: str
    raw_candidates: int
    temporal_score: float | None
    score: float | None
    fixed_x: float | None
    dynamic_x: float | None
    dynamic_y_ratio: float | None
    bottom_x: float | None
    yaw_deg: float | None
    curve_factor: float | None
    heading_change_deg: float | None
    curvature_px: float | None
    lateral_error_norm: float | None
    steer_norm: float | None
    speed_cmd: float | None
    lost_count: int


def load_single_frame_module() -> Any:
    spec = importlib.util.spec_from_file_location("playground_single_line", SINGLE_FRAME_SCRIPT)
    if spec is None or spec.loader is None:
        raise ImportError(f"cannot load {SINGLE_FRAME_SCRIPT}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


single = load_single_frame_module()


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def list_frames(frame_dir: Path, max_frames: int | None) -> list[Path]:
    frames = sorted(frame_dir.glob("frame_*.png"))
    if max_frames is not None:
        frames = frames[:max_frames]
    if not frames:
        raise FileNotFoundError(f"no frame_*.png found in {frame_dir}")
    return frames


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def fit_line_poly(candidate: Any) -> np.ndarray | None:
    points = np.asarray(candidate.line_points, dtype=np.float32)
    if points.shape[0] < 12:
        return None
    xs = points[:, 0]
    ys = points[:, 1]
    if float(ys.max() - ys.min()) < 35.0:
        return None
    degree = 2 if points.shape[0] >= 24 else 1
    return np.polyfit(ys, xs, degree)


def line_x_at(candidate: Any, y: float, coeff: np.ndarray | None = None) -> float | None:
    if coeff is not None:
        return float(np.polyval(coeff, y))
    return single._line_x_at(candidate.line_points, y)


def compute_curve_metrics(candidate: Any, image_height: int) -> tuple[float, float, float, np.ndarray | None]:
    coeff = fit_line_poly(candidate)
    y_low = min(float(candidate.y_max), image_height * 0.93)
    y_high = max(float(candidate.y_min), image_height * 0.52)
    if y_low <= y_high + 20.0:
        return 0.0, 0.0, 0.0, coeff

    # 从近处往远处采样。直线斜着走不算弯，只有角度沿路径明显变化才算曲率。
    ys = np.linspace(y_low, y_high, 7)
    xs: list[float] = []
    valid_ys: list[float] = []
    for y in ys:
        x = line_x_at(candidate, float(y), coeff)
        if x is None:
            continue
        xs.append(float(x))
        valid_ys.append(float(y))

    if len(xs) < 4:
        return 0.0, 0.0, 0.0, coeff

    angles: list[float] = []
    for i in range(len(xs) - 1):
        dx = xs[i + 1] - xs[i]
        dy_forward = valid_ys[i] - valid_ys[i + 1]
        if dy_forward <= 1.0:
            continue
        angles.append(math.degrees(math.atan2(dx, dy_forward)))

    if not angles:
        return 0.0, 0.0, 0.0, coeff

    heading_change_deg = float(max(angles) - min(angles))
    heading_factor = clamp(heading_change_deg / 28.0, 0.0, 1.0)

    curvature_px = 0.0
    if coeff is not None and len(coeff) == 3:
        a, b, _c = [float(v) for v in coeff]
        kappas = []
        for y in valid_ys:
            dx_dy = 2.0 * a * y + b
            kappa = abs(2.0 * a) / pow(1.0 + dx_dy * dx_dy, 1.5)
            kappas.append(kappa)
        curvature_px = float(np.mean(kappas)) if kappas else 0.0

    curvature_factor = clamp(curvature_px * image_height * 5.0, 0.0, 1.0)
    curve_factor = max(heading_factor, curvature_factor)
    return curve_factor, heading_change_deg, curvature_px, coeff


def temporal_candidate_score(candidate: Any, state: TrackState, image_width: int, image_height: int) -> float:
    if not state.active:
        return float(candidate.score)

    dx_bottom = abs(float(candidate.bottom_x) - state.bottom_x)
    dx_lookahead = abs(float(candidate.lookahead_x) - state.lookahead_x)
    dyaw = abs(float(single.compute_line_yaw_deg(candidate, image_height)) - state.yaw_deg)
    pos_penalty = min(1.0, (0.65 * dx_lookahead + 0.35 * dx_bottom) / max(1.0, image_width * 0.22))
    yaw_penalty = min(1.0, dyaw / 28.0)
    return float(candidate.score) - 0.42 * pos_penalty - 0.18 * yaw_penalty


def choose_temporal_candidate(
    candidates: list[Any],
    state: TrackState,
    image_shape: tuple[int, int, int],
    min_temporal_score: float,
) -> tuple[Any | None, float | None]:
    if not candidates:
        return None, None

    image_height, image_width = image_shape[:2]
    scored: list[tuple[float, Any]] = []
    for candidate in candidates:
        min_y_max_ratio = 0.45 if state.active else 0.58
        min_height_ratio = 0.18 if state.active else 0.28
        if candidate.y_max < image_height * min_y_max_ratio:
            continue
        if candidate.height < image_height * min_height_ratio:
            continue
        scored.append((temporal_candidate_score(candidate, state, image_width, image_height), candidate))

    if not scored:
        return None, None
    scored.sort(key=lambda item: item[0], reverse=True)
    score, candidate = scored[0]
    if score < min_temporal_score:
        return None, float(score)
    return candidate, float(score)


def compute_dynamic_preview(
    candidate: Any,
    image_shape: tuple[int, int, int],
    state: TrackState,
    args: argparse.Namespace,
) -> dict[str, float | np.ndarray | None]:
    h, w = image_shape[:2]
    center_x = w * 0.5
    curve_factor, heading_change_deg, curvature_px, coeff = compute_curve_metrics(candidate, h)

    fixed_y = h * args.fixed_y_ratio
    fixed_x = line_x_at(candidate, fixed_y, coeff)
    if fixed_x is None:
        fixed_x = float(candidate.lookahead_x)

    fixed_error_norm = clamp((float(fixed_x) - center_x) / (w * 0.5), -1.0, 1.0)
    yaw_deg = float(single.compute_line_yaw_deg(candidate, h))
    yaw_penalty = clamp(abs(yaw_deg) / args.yaw_slow_deg, 0.0, 1.0)
    error_penalty = clamp(abs(fixed_error_norm) / args.error_slow_norm, 0.0, 1.0)

    # 先按曲率降速，再按偏航/横向误差二次降速；这对应惯导里的曲率减速和角度纠偏减速。
    curve_slow = pow(curve_factor, args.curve_exponent)
    speed_cmd = args.speed_max - (args.speed_max - args.speed_min) * curve_slow
    speed_cmd *= 1.0 - args.yaw_speed_penalty * yaw_penalty
    speed_cmd *= 1.0 - args.error_speed_penalty * error_penalty
    speed_cmd = clamp(speed_cmd, args.speed_min, args.speed_max)

    speed_norm = (speed_cmd - args.speed_min) / max(1e-6, args.speed_max - args.speed_min)
    far_pull = speed_norm * (1.0 - args.curve_ld_penalty * curve_slow) * (1.0 - args.error_ld_penalty * error_penalty)
    far_pull = clamp(far_pull, 0.0, 1.0)
    dynamic_y_ratio = args.near_y_ratio - (args.near_y_ratio - args.far_y_ratio) * far_pull
    dynamic_y_ratio = clamp(dynamic_y_ratio, args.far_y_ratio, args.near_y_ratio)
    dynamic_y = h * dynamic_y_ratio

    dynamic_x = line_x_at(candidate, dynamic_y, coeff)
    if dynamic_x is None:
        dynamic_x = fixed_x

    e_norm = clamp((float(dynamic_x) - center_x) / (w * 0.5), -1.0, 1.0)
    bottom_y = h * 0.93
    bottom_x = line_x_at(candidate, bottom_y, coeff)
    if bottom_x is None:
        bottom_x = float(candidate.bottom_x)

    aim_yaw_deg = math.degrees(math.atan2(float(dynamic_x) - float(bottom_x), bottom_y - dynamic_y))
    steer_norm = args.k_error * e_norm + args.k_yaw * clamp(aim_yaw_deg / args.steer_yaw_norm_deg, -1.0, 1.0)
    steer_norm = clamp(steer_norm, -1.0, 1.0)

    # 输出只做轻滤波；真正上车时还应在底层做舵角斜率限制。
    if state.active:
        alpha = args.output_alpha
        dynamic_x = alpha * float(dynamic_x) + (1.0 - alpha) * state.dynamic_x
        dynamic_y_ratio = alpha * dynamic_y_ratio + (1.0 - alpha) * state.dynamic_y_ratio
        speed_cmd = alpha * speed_cmd + (1.0 - alpha) * state.speed_cmd
        steer_norm = alpha * steer_norm + (1.0 - alpha) * state.steer_norm
        curve_factor = alpha * curve_factor + (1.0 - alpha) * state.curve_factor

    return {
        "coeff": coeff,
        "fixed_x": float(fixed_x),
        "fixed_y_ratio": float(args.fixed_y_ratio),
        "dynamic_x": float(dynamic_x),
        "dynamic_y_ratio": float(dynamic_y_ratio),
        "bottom_x": float(bottom_x),
        "yaw_deg": float(yaw_deg),
        "aim_yaw_deg": float(aim_yaw_deg),
        "curve_factor": float(curve_factor),
        "heading_change_deg": float(heading_change_deg),
        "curvature_px": float(curvature_px),
        "lateral_error_norm": float(e_norm),
        "steer_norm": float(steer_norm),
        "speed_cmd": float(speed_cmd),
    }


def update_state(state: TrackState, candidate: Any | None, dynamic: dict[str, float | np.ndarray | None] | None, args: argparse.Namespace) -> tuple[TrackState, bool]:
    if candidate is None or dynamic is None:
        if state.active:
            state.lost_count += 1
            if state.lost_count > args.max_lost:
                state.active = False
        return state, False

    alpha = args.track_alpha
    if not state.active:
        alpha = 1.0
        state.active = True

    state.bottom_x = alpha * float(dynamic["bottom_x"]) + (1.0 - alpha) * state.bottom_x
    state.lookahead_x = alpha * float(dynamic["fixed_x"]) + (1.0 - alpha) * state.lookahead_x
    state.yaw_deg = alpha * float(dynamic["yaw_deg"]) + (1.0 - alpha) * state.yaw_deg
    state.score = alpha * float(candidate.score) + (1.0 - alpha) * state.score
    state.curve_factor = float(dynamic["curve_factor"])
    state.dynamic_y_ratio = float(dynamic["dynamic_y_ratio"])
    state.dynamic_x = float(dynamic["dynamic_x"])
    state.speed_cmd = float(dynamic["speed_cmd"])
    state.steer_norm = float(dynamic["steer_norm"])
    state.lost_count = 0
    return state, True


def draw_polyline(overlay: np.ndarray, candidate: Any, coeff: np.ndarray | None, color: tuple[int, int, int]) -> None:
    h, w = overlay.shape[:2]
    y0 = max(int(candidate.y_min), int(h * 0.45))
    y1 = min(int(candidate.y_max), int(h * 0.95))
    if y1 <= y0:
        return
    pts = []
    for y in np.linspace(y0, y1, 48):
        x = line_x_at(candidate, float(y), coeff)
        if x is None:
            continue
        xi = int(round(clamp(float(x), 0.0, w - 1.0)))
        pts.append((xi, int(round(y))))
    if len(pts) >= 2:
        cv2.polylines(overlay, [np.asarray(pts, dtype=np.int32)], False, color, 3, cv2.LINE_AA)


def draw_text_panel(overlay: np.ndarray, lines: list[str]) -> None:
    x0, y0 = 8, 8
    line_h = 22
    width = 420
    height = 14 + line_h * len(lines)
    cv2.rectangle(overlay, (x0 - 4, y0 - 4), (x0 + width, y0 + height), (0, 0, 0), -1)
    for i, text in enumerate(lines):
        y = y0 + 17 + i * line_h
        cv2.putText(overlay, text, (x0, y), cv2.FONT_HERSHEY_SIMPLEX, 0.56, (255, 255, 255), 1, cv2.LINE_AA)


def draw_overlay(
    image: np.ndarray,
    mask: np.ndarray,
    candidates: list[Any],
    selected: Any | None,
    dynamic: dict[str, float | np.ndarray | None] | None,
    state: TrackState,
    frame_name: str,
    mode: str,
) -> np.ndarray:
    h, w = image.shape[:2]
    overlay = image.copy()
    mask_color = np.zeros_like(overlay)
    mask_color[:, :, 1] = mask
    overlay = cv2.addWeighted(overlay, 0.82, mask_color, 0.18, 0)

    center_x = int(w * 0.5)
    cv2.line(overlay, (center_x, 0), (center_x, h - 1), (0, 255, 255), 1)

    for cand in candidates[:5]:
        x1, y1, x2, y2 = cand.bbox
        color = (0, 180, 255) if cand is not selected else (0, 0, 255)
        cv2.rectangle(overlay, (x1, y1), (x2, y2), color, 1 if cand is not selected else 2)

    if selected is not None and dynamic is not None:
        coeff = dynamic["coeff"]
        draw_polyline(overlay, selected, coeff if isinstance(coeff, np.ndarray) else None, (0, 0, 255))

        fixed_y = int(round(h * float(dynamic["fixed_y_ratio"])))
        dyn_y = int(round(h * float(dynamic["dynamic_y_ratio"])))
        fixed_x = int(round(float(dynamic["fixed_x"])))
        dyn_x = int(round(float(dynamic["dynamic_x"])))
        bottom_x = int(round(float(dynamic["bottom_x"])))
        bottom_y = int(round(h * 0.93))

        cv2.line(overlay, (0, fixed_y), (w - 1, fixed_y), (255, 255, 0), 1)
        cv2.circle(overlay, (fixed_x, fixed_y), 7, (255, 0, 255), -1)
        cv2.putText(overlay, "fixed LD", (fixed_x + 8, fixed_y - 6), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 255), 1)

        cv2.line(overlay, (0, dyn_y), (w - 1, dyn_y), (0, 165, 255), 2)
        cv2.circle(overlay, (dyn_x, dyn_y), 9, (0, 165, 255), -1)
        cv2.circle(overlay, (bottom_x, bottom_y), 6, (0, 255, 255), -1)
        cv2.line(overlay, (bottom_x, bottom_y), (dyn_x, dyn_y), (0, 165, 255), 2, cv2.LINE_AA)
        cv2.putText(overlay, "dynamic LD", (dyn_x + 8, dyn_y - 8), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 165, 255), 1)

        steer = float(dynamic["steer_norm"])
        bar_x0, bar_y0 = w - 210, h - 42
        cv2.rectangle(overlay, (bar_x0, bar_y0), (bar_x0 + 200, bar_y0 + 24), (0, 0, 0), -1)
        cv2.line(overlay, (bar_x0 + 100, bar_y0), (bar_x0 + 100, bar_y0 + 24), (255, 255, 255), 1)
        cv2.rectangle(
            overlay,
            (bar_x0 + 100, bar_y0 + 4),
            (bar_x0 + 100 + int(steer * 95), bar_y0 + 20),
            (0, 165, 255),
            -1,
        )

        text_lines = [
            f"{frame_name}  mode={mode} score={selected.score:.2f}",
            f"curve={float(dynamic['curve_factor']):.2f}  head_d={float(dynamic['heading_change_deg']):.1f}deg",
            f"dyn_y={float(dynamic['dynamic_y_ratio']):.2f}  err={float(dynamic['lateral_error_norm']):+.2f}",
            f"yaw={float(dynamic['yaw_deg']):+.1f}deg  steer={float(dynamic['steer_norm']):+.2f}",
            f"speed={float(dynamic['speed_cmd']):.0f}  lost={state.lost_count}",
        ]
    else:
        text_lines = [
            f"{frame_name}  mode={mode}",
            f"pred steer={state.steer_norm:+.2f} speed={state.speed_cmd:.0f}",
            f"lost={state.lost_count}",
        ]

    draw_text_panel(overlay, text_lines)
    return overlay


def create_video_writer(output_dir: Path, video_name: str, fps: float, frame_size: tuple[int, int]) -> tuple[cv2.VideoWriter, Path, Path]:
    final_path = output_dir / video_name
    temp_path = PROJECT_ROOT / "data" / "_temp_dynamic_preview.mp4"
    if temp_path.exists():
        temp_path.unlink()
    writer = cv2.VideoWriter(str(temp_path), cv2.VideoWriter_fourcc(*"mp4v"), fps, frame_size)
    if not writer.isOpened():
        raise RuntimeError(f"failed to open video writer: {temp_path}")
    return writer, temp_path, final_path


def write_summary(output_dir: Path, results: list[DynamicResult], config: dict[str, Any]) -> None:
    (output_dir / "summary.json").write_text(
        json.dumps({"config": config, "results": [asdict(r) for r in results]}, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    with (output_dir / "summary.csv").open("w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(f, fieldnames=list(asdict(results[0]).keys()) if results else [])
        if results:
            writer.writeheader()
            for result in results:
                writer.writerow(asdict(result))


def process(args: argparse.Namespace) -> dict[str, Any]:
    ensure_dir(args.output)
    frames = list_frames(args.frame_dir, args.max_frames)
    state = TrackState()
    results: list[DynamicResult] = []
    writer: cv2.VideoWriter | None = None
    temp_video: Path | None = None
    final_video: Path | None = None

    for frame_index, frame_path in enumerate(frames, start=1):
        image = single.imread_unicode(frame_path)
        mask, _stats = single.build_white_line_mask(image)
        candidates = single.find_line_candidates(mask)
        selected, temporal_score = choose_temporal_candidate(candidates, state, image.shape, args.min_temporal_score)
        dynamic = compute_dynamic_preview(selected, image.shape, state, args) if selected is not None else None
        state, accepted = update_state(state, selected, dynamic, args)
        mode = "detected" if accepted else ("predicted" if state.active else "lost")

        overlay = draw_overlay(image, mask, candidates, selected, dynamic, state, frame_path.name, mode)
        if writer is None:
            writer, temp_video, final_video = create_video_writer(
                args.output,
                args.video_name,
                args.video_fps,
                (overlay.shape[1], overlay.shape[0]),
            )
        writer.write(overlay)

        if dynamic is None:
            result = DynamicResult(
                frame_index=frame_index,
                frame_name=frame_path.name,
                mode=mode,
                raw_candidates=len(candidates),
                temporal_score=temporal_score,
                score=None,
                fixed_x=None,
                dynamic_x=state.dynamic_x if state.active else None,
                dynamic_y_ratio=state.dynamic_y_ratio if state.active else None,
                bottom_x=state.bottom_x if state.active else None,
                yaw_deg=state.yaw_deg if state.active else None,
                curve_factor=state.curve_factor if state.active else None,
                heading_change_deg=None,
                curvature_px=None,
                lateral_error_norm=None,
                steer_norm=state.steer_norm if state.active else None,
                speed_cmd=state.speed_cmd if state.active else None,
                lost_count=state.lost_count,
            )
        else:
            result = DynamicResult(
                frame_index=frame_index,
                frame_name=frame_path.name,
                mode=mode,
                raw_candidates=len(candidates),
                temporal_score=temporal_score,
                score=float(selected.score) if selected is not None else None,
                fixed_x=float(dynamic["fixed_x"]),
                dynamic_x=float(dynamic["dynamic_x"]),
                dynamic_y_ratio=float(dynamic["dynamic_y_ratio"]),
                bottom_x=float(dynamic["bottom_x"]),
                yaw_deg=float(dynamic["yaw_deg"]),
                curve_factor=float(dynamic["curve_factor"]),
                heading_change_deg=float(dynamic["heading_change_deg"]),
                curvature_px=float(dynamic["curvature_px"]),
                lateral_error_norm=float(dynamic["lateral_error_norm"]),
                steer_norm=float(dynamic["steer_norm"]),
                speed_cmd=float(dynamic["speed_cmd"]),
                lost_count=state.lost_count,
            )
        results.append(result)

        if args.progress_every > 0 and frame_index % args.progress_every == 0:
            print(
                f"processed {frame_index}/{len(frames)} mode={mode} "
                f"curve={state.curve_factor:.2f} y={state.dynamic_y_ratio:.2f} steer={state.steer_norm:+.2f}"
            )

    if writer is not None:
        writer.release()
        assert temp_video is not None
        assert final_video is not None
        if final_video.exists():
            final_video.unlink()
        shutil.move(str(temp_video), str(final_video))

    config = vars(args).copy()
    config["frame_dir"] = str(args.frame_dir)
    config["output"] = str(args.output)
    write_summary(args.output, results, config)

    detected = sum(1 for r in results if r.mode == "detected")
    predicted = sum(1 for r in results if r.mode == "predicted")
    lost = sum(1 for r in results if r.mode == "lost")
    return {
        "output_dir": str(args.output),
        "video": str(final_video) if final_video else None,
        "frames": len(results),
        "detected": detected,
        "predicted": predicted,
        "lost": lost,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="操场白线动态预瞄可视化")
    parser.add_argument("--frame-dir", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--max-frames", type=int, default=None)
    parser.add_argument("--video-name", default="操场中线动态预瞄可视化.mp4")
    parser.add_argument("--video-fps", type=float, default=30.0)
    parser.add_argument("--progress-every", type=int, default=300)
    parser.add_argument("--min-temporal-score", type=float, default=0.36)
    parser.add_argument("--max-lost", type=int, default=20)
    parser.add_argument("--track-alpha", type=float, default=0.45)
    parser.add_argument("--output-alpha", type=float, default=0.42)

    parser.add_argument("--fixed-y-ratio", type=float, default=0.72)
    parser.add_argument("--near-y-ratio", type=float, default=0.84)
    parser.add_argument("--far-y-ratio", type=float, default=0.56)
    parser.add_argument("--curve-exponent", type=float, default=1.25)
    parser.add_argument("--curve-ld-penalty", type=float, default=0.78)
    parser.add_argument("--error-ld-penalty", type=float, default=0.35)

    parser.add_argument("--speed-min", type=float, default=45.0)
    parser.add_argument("--speed-max", type=float, default=100.0)
    parser.add_argument("--yaw-slow-deg", type=float, default=24.0)
    parser.add_argument("--error-slow-norm", type=float, default=0.45)
    parser.add_argument("--yaw-speed-penalty", type=float, default=0.22)
    parser.add_argument("--error-speed-penalty", type=float, default=0.18)

    parser.add_argument("--k-error", type=float, default=0.72)
    parser.add_argument("--k-yaw", type=float, default=0.36)
    parser.add_argument("--steer-yaw-norm-deg", type=float, default=28.0)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    result = process(args)
    print(json.dumps(result, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
