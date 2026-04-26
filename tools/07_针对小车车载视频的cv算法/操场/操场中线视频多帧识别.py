"""蓝色操场白色单线多帧跟踪。

这个脚本在 ``操场中线识别.py`` 的单帧检测基础上加入时序约束：
- 当前帧优先选择接近上一帧目标线的候选，避免多条跑道线之间跳变。
- 短时丢线时输出上一帧预测，不立即清空控制量。
- 汇总每帧的横向误差、角度和跟踪状态，便于后续转成车端控制表。

默认处理 data/frames/蓝色操场 下的全部 frame_*.png。
默认输出到 data/蓝色操场/操场中线视频识别。
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
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "蓝色操场" / "操场中线视频识别"


@dataclass
class TrackState:
    active: bool = False
    lost_count: int = 0
    bottom_x: float = 0.0
    lookahead_x: float = 0.0
    yaw_deg: float = 0.0
    score: float = 0.0


@dataclass
class FrameTrackResult:
    frame_index: int
    frame_name: str
    mode: str
    detected: bool
    raw_candidates: int
    raw_best_score: float | None
    temporal_score: float | None
    bottom_x: float | None
    lookahead_x: float | None
    yaw_deg: float | None
    lateral_error_px: float | None
    lost_count: int
    overlay_path: str | None


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


def temporal_candidate_score(candidate: Any, state: TrackState, image_width: int) -> float:
    if not state.active:
        return float(candidate.score)

    dx_bottom = abs(float(candidate.bottom_x) - state.bottom_x)
    dx_lookahead = abs(float(candidate.lookahead_x) - state.lookahead_x)
    dyaw = abs(float(single.compute_line_yaw_deg(candidate, 480)) - state.yaw_deg)

    pos_penalty = min(1.0, (0.65 * dx_lookahead + 0.35 * dx_bottom) / max(1.0, image_width * 0.22))
    yaw_penalty = min(1.0, dyaw / 28.0)
    return float(candidate.score) - 0.42 * pos_penalty - 0.18 * yaw_penalty


def choose_temporal_candidate(candidates: list[Any], state: TrackState, image_shape: tuple[int, int, int], min_temporal_score: float) -> tuple[Any | None, float | None]:
    if not candidates:
        return None, None

    image_width = image_shape[1]
    image_height = image_shape[0]
    scored: list[tuple[float, Any]] = []
    for candidate in candidates:
        # 车端控制最关心车前方近处的线。远处短线、起跑线碎片和里程标记
        # 可以用于提示，但不能拿来刷新主跟踪状态。
        min_y_max_ratio = 0.45 if state.active else 0.58
        min_height_ratio = 0.18 if state.active else 0.28
        if candidate.y_max < image_height * min_y_max_ratio:
            continue
        if candidate.height < image_height * min_height_ratio:
            continue

        if state.active:
            yaw = single.compute_line_yaw_deg(candidate, image_height)
            dx_bottom = abs(float(candidate.bottom_x) - state.bottom_x)
            dx_lookahead = abs(float(candidate.lookahead_x) - state.lookahead_x)
            dyaw = abs(float(yaw) - state.yaw_deg)
            pos_penalty = min(1.0, (0.65 * dx_lookahead + 0.35 * dx_bottom) / max(1.0, image_width * 0.22))
            yaw_penalty = min(1.0, dyaw / 28.0)
            score = float(candidate.score) - 0.42 * pos_penalty - 0.18 * yaw_penalty
        else:
            score = float(candidate.score)
        scored.append((score, candidate))

    if not scored:
        return None, None

    scored.sort(key=lambda item: item[0], reverse=True)
    best_score, best_candidate = scored[0]
    if best_score < min_temporal_score:
        return None, best_score
    return best_candidate, float(best_score)


def update_track_state(state: TrackState, candidate: Any | None, image_height: int, alpha: float, max_lost: int) -> tuple[TrackState, bool]:
    if candidate is None:
        if state.active:
            state.lost_count += 1
            if state.lost_count > max_lost:
                state.active = False
        return state, False

    yaw = float(single.compute_line_yaw_deg(candidate, image_height))
    if not state.active:
        state.bottom_x = float(candidate.bottom_x)
        state.lookahead_x = float(candidate.lookahead_x)
        state.yaw_deg = yaw
        state.score = float(candidate.score)
        state.active = True
        state.lost_count = 0
        return state, True

    state.bottom_x = alpha * float(candidate.bottom_x) + (1.0 - alpha) * state.bottom_x
    state.lookahead_x = alpha * float(candidate.lookahead_x) + (1.0 - alpha) * state.lookahead_x
    state.yaw_deg = alpha * yaw + (1.0 - alpha) * state.yaw_deg
    state.score = alpha * float(candidate.score) + (1.0 - alpha) * state.score
    state.lost_count = 0
    return state, True


def draw_tracking_overlay(image_bgr: np.ndarray, mask: np.ndarray, candidates: list[Any], selected: Any | None, state: TrackState, frame_name: str, temporal_score: float | None) -> np.ndarray:
    overlay = single.draw_overlay(image_bgr, mask, candidates, selected, frame_name)
    h, w = image_bgr.shape[:2]
    center_x = w * 0.5

    if state.active:
        lookahead_y = int(h * 0.72)
        bottom_y = int(h * 0.93)
        cv2.line(
            overlay,
            (int(round(state.bottom_x)), bottom_y),
            (int(round(state.lookahead_x)), lookahead_y),
            (255, 0, 0),
            2,
            cv2.LINE_AA,
        )
        cv2.circle(overlay, (int(round(state.lookahead_x)), lookahead_y), 8, (255, 0, 0), 2)
        text = (
            f"track err={state.lookahead_x - center_x:.1f}px yaw={state.yaw_deg:.1f}deg "
            f"lost={state.lost_count}"
        )
        if temporal_score is not None:
            text += f" ts={temporal_score:.2f}"
        cv2.rectangle(overlay, (0, h - 28), (w - 1, h - 1), (0, 0, 0), -1)
        cv2.putText(overlay, text, (8, h - 9), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 1, cv2.LINE_AA)

    return overlay


def write_summary(output_dir: Path, results: list[FrameTrackResult], config: dict[str, Any]) -> None:
    (output_dir / "summary.json").write_text(
        json.dumps({"config": config, "results": [asdict(r) for r in results]}, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    with (output_dir / "summary.csv").open("w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "frame_index",
                "frame_name",
                "mode",
                "detected",
                "raw_candidates",
                "raw_best_score",
                "temporal_score",
                "bottom_x",
                "lookahead_x",
                "yaw_deg",
                "lateral_error_px",
                "lost_count",
                "overlay_path",
            ],
        )
        writer.writeheader()
        for result in results:
            writer.writerow(asdict(result))


def write_contact_sheet(output_dir: Path, overlay_paths: list[Path], max_images: int = 24) -> Path | None:
    if not overlay_paths:
        return None

    from PIL import Image, ImageDraw

    if len(overlay_paths) > max_images:
        idxs = np.linspace(0, len(overlay_paths) - 1, max_images).round().astype(int)
        selected_paths = [overlay_paths[int(i)] for i in idxs]
    else:
        selected_paths = overlay_paths

    thumbs = []
    for path in selected_paths:
        image = Image.open(path).convert("RGB")
        image.thumbnail((426, 240))
        canvas = Image.new("RGB", (426, 265), "white")
        canvas.paste(image, (0, 0))
        draw = ImageDraw.Draw(canvas)
        draw.text((4, 244), path.name, fill=(0, 0, 0))
        thumbs.append(canvas)

    cols = 3
    rows = int(math.ceil(len(thumbs) / cols))
    sheet = Image.new("RGB", (cols * 426, rows * 265), "white")
    for idx, thumb in enumerate(thumbs):
        sheet.paste(thumb, ((idx % cols) * 426, (idx // cols) * 265))

    out = output_dir / "overlay_contact_sheet.png"
    sheet.save(out)
    return out


def create_video_writer(
    output_dir: Path,
    video_name: str,
    fps: float,
    frame_size: tuple[int, int],
) -> tuple[cv2.VideoWriter, Path, Path]:
    """Create a VideoWriter through an ASCII temp path, then move it later."""

    final_path = output_dir / video_name
    temp_path = PROJECT_ROOT / "data" / "_temp_playground_line_tracking.mp4"
    if temp_path.exists():
        temp_path.unlink()

    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    writer = cv2.VideoWriter(str(temp_path), fourcc, fps, frame_size)
    if not writer.isOpened():
        raise RuntimeError(f"failed to open video writer: {temp_path}")
    return writer, temp_path, final_path


def process_video_frames(args: argparse.Namespace) -> dict[str, Any]:
    output_dir: Path = args.output
    overlay_dir = output_dir / "overlay"
    mask_dir = output_dir / "mask"
    ensure_dir(output_dir)
    ensure_dir(overlay_dir)
    if args.save_masks:
        ensure_dir(mask_dir)

    frames = list_frames(args.frame_dir, args.max_frames)
    state = TrackState()
    results: list[FrameTrackResult] = []
    saved_overlays: list[Path] = []
    video_writer: cv2.VideoWriter | None = None
    temp_video_path: Path | None = None
    final_video_path: Path | None = None

    for frame_index, frame_path in enumerate(frames, start=1):
        image = single.imread_unicode(frame_path)
        mask, _mask_stats = single.build_white_line_mask(image)
        candidates = single.find_line_candidates(mask)
        raw_best_score = float(candidates[0].score) if candidates else None

        selected, temporal_score = choose_temporal_candidate(candidates, state, image.shape, args.min_temporal_score)
        state, accepted = update_track_state(state, selected, image.shape[0], args.smooth_alpha, args.max_lost)

        mode = "detected" if accepted else ("predicted" if state.active else "lost")
        overlay_path: Path | None = None
        need_overlay = args.write_video or (
            args.save_every > 0 and ((frame_index - 1) % args.save_every == 0 or frame_index == len(frames))
        )
        overlay = None
        if need_overlay:
            overlay = draw_tracking_overlay(image, mask, candidates, selected, state, frame_path.name, temporal_score)

        if args.write_video:
            if video_writer is None:
                video_writer, temp_video_path, final_video_path = create_video_writer(
                    output_dir,
                    args.video_name,
                    args.video_fps,
                    (overlay.shape[1], overlay.shape[0]),
                )
            video_writer.write(overlay)

        if args.save_every > 0 and ((frame_index - 1) % args.save_every == 0 or frame_index == len(frames)):
            if overlay is None:
                overlay = draw_tracking_overlay(image, mask, candidates, selected, state, frame_path.name, temporal_score)
            overlay_path = overlay_dir / f"{frame_path.stem}_track.png"
            single.imwrite_unicode(overlay_path, overlay)
            saved_overlays.append(overlay_path)

        if args.save_masks and args.save_every > 0 and ((frame_index - 1) % args.save_every == 0 or frame_index == len(frames)):
            single.imwrite_unicode(mask_dir / f"{frame_path.stem}_mask.png", mask)

        center_x = image.shape[1] * 0.5
        if state.active:
            bottom_x = state.bottom_x
            lookahead_x = state.lookahead_x
            yaw_deg = state.yaw_deg
            lateral_error = state.lookahead_x - center_x
        else:
            bottom_x = None
            lookahead_x = None
            yaw_deg = None
            lateral_error = None

        results.append(
            FrameTrackResult(
                frame_index=frame_index,
                frame_name=frame_path.name,
                mode=mode,
                detected=accepted,
                raw_candidates=len(candidates),
                raw_best_score=raw_best_score,
                temporal_score=temporal_score,
                bottom_x=bottom_x,
                lookahead_x=lookahead_x,
                yaw_deg=yaw_deg,
                lateral_error_px=lateral_error,
                lost_count=state.lost_count,
                overlay_path=str(overlay_path) if overlay_path is not None else None,
            )
        )

        if args.progress_every > 0 and frame_index % args.progress_every == 0:
            print(f"processed {frame_index}/{len(frames)} frames, mode={mode}, lost={state.lost_count}")

    if video_writer is not None:
        video_writer.release()
        assert temp_video_path is not None
        assert final_video_path is not None
        if final_video_path.exists():
            final_video_path.unlink()
        shutil.move(str(temp_video_path), str(final_video_path))

    config = {
        "frame_dir": str(args.frame_dir),
        "output": str(output_dir),
        "frames": len(frames),
        "save_every": args.save_every,
        "write_video": args.write_video,
        "video_name": args.video_name,
        "video_fps": args.video_fps,
        "min_temporal_score": args.min_temporal_score,
        "smooth_alpha": args.smooth_alpha,
        "max_lost": args.max_lost,
    }
    write_summary(output_dir, results, config)
    sheet = write_contact_sheet(output_dir, saved_overlays)

    detected = sum(1 for r in results if r.mode == "detected")
    predicted = sum(1 for r in results if r.mode == "predicted")
    lost = sum(1 for r in results if r.mode == "lost")
    return {
        "output_dir": str(output_dir),
        "frames": len(results),
        "detected": detected,
        "predicted": predicted,
        "lost": lost,
        "saved_overlays": len(saved_overlays),
        "video": str(final_video_path) if final_video_path is not None else None,
        "contact_sheet": str(sheet) if sheet else None,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="蓝色操场白色单线视频多帧跟踪")
    parser.add_argument("--frame-dir", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--max-frames", type=int, default=None)
    parser.add_argument("--save-every", type=int, default=0, help="每 N 帧保存一张 overlay，0 表示不保存")
    parser.add_argument("--save-masks", action="store_true")
    parser.add_argument("--write-video", action=argparse.BooleanOptionalAction, default=True)
    parser.add_argument("--video-name", default="操场中线视频识别.mp4")
    parser.add_argument("--video-fps", type=float, default=30.0)
    parser.add_argument("--min-temporal-score", type=float, default=0.36)
    parser.add_argument("--smooth-alpha", type=float, default=0.45)
    parser.add_argument("--max-lost", type=int, default=20)
    parser.add_argument("--progress-every", type=int, default=300)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    result = process_video_frames(args)
    print(json.dumps(result, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
