from __future__ import annotations

import argparse
import json
import time
from dataclasses import asdict, dataclass
from pathlib import Path

import cv2
import numpy as np


DATA_DIR_NAME = "\u5355\u8fb9\u6865"
IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".bmp"}
DEFAULT_FPS = 50.0


@dataclass
class ExportStats:
    folder_name: str
    output_dir: str
    frame_count: int
    fps: float
    width: int
    height: int
    avg_encode_ms: float
    max_encode_ms: float
    total_encode_ms: float
    wall_ms: float
    mp4_path: str


def resolve_project_root() -> Path:
    here = Path(__file__).resolve()
    for parent in here.parents:
        if (parent / "data" / DATA_DIR_NAME).exists():
            return parent
    raise FileNotFoundError(f"Could not locate project root containing data/{DATA_DIR_NAME}.")


PROJECT_ROOT = resolve_project_root()
DATA_ROOT = PROJECT_ROOT / "data" / DATA_DIR_NAME
FRAMES_DIR = DATA_ROOT / "frames"


def collect_draw_paths(draw_dir: Path) -> list[Path]:
    return sorted(path for path in draw_dir.iterdir() if path.is_file() and path.suffix.lower() in IMAGE_SUFFIXES)


def load_image_bgr(image_path: Path) -> np.ndarray:
    data = np.fromfile(str(image_path), dtype=np.uint8)
    image = cv2.imdecode(data, cv2.IMREAD_COLOR)
    if image is None:
        raise ValueError(f"Failed to decode image: {image_path}")
    return image


def load_fps(output_dir: Path, folder_name: str) -> float:
    timing_path = output_dir / "timing_summary.json"
    if timing_path.exists():
        data = json.loads(timing_path.read_text(encoding="utf-8"))
        fps = float(data.get("fps", DEFAULT_FPS))
        if fps > 0:
            return fps
    return DEFAULT_FPS


def default_output_dir_for_frame_dir(frame_dir: Path) -> Path:
    return frame_dir.parent / f"{frame_dir.name}_output"


def resolve_output_dir(name_or_path: Path) -> Path:
    if name_or_path.is_absolute():
        return name_or_path
    candidate = name_or_path
    if candidate.exists():
        return candidate.resolve()
    candidate = FRAMES_DIR / name_or_path
    if candidate.exists():
        return candidate.resolve()
    candidate = FRAMES_DIR / f"{name_or_path.name}_output"
    if candidate.exists():
        return candidate.resolve()
    raise FileNotFoundError(f"Output directory not found: {name_or_path}")


def export_output_dir(output_dir: Path) -> ExportStats:
    draw_dir = output_dir / "draw"
    if not draw_dir.exists():
        raise FileNotFoundError(f"Draw directory not found: {draw_dir}")

    frame_paths = collect_draw_paths(draw_dir)
    if not frame_paths:
        raise ValueError(f"No draw images found in {draw_dir}")

    folder_name = output_dir.name.removesuffix("_output")
    fps = load_fps(output_dir, folder_name)
    first_frame = load_image_bgr(frame_paths[0])
    height, width = first_frame.shape[:2]
    mp4_path = output_dir / f"{folder_name}_draw.mp4"

    writer = cv2.VideoWriter(
        str(mp4_path),
        cv2.VideoWriter_fourcc(*"mp4v"),
        fps,
        (width, height),
    )
    if not writer.isOpened():
        raise RuntimeError(f"Failed to open MP4 writer for: {mp4_path}")

    encode_times_ms: list[float] = []
    started = time.perf_counter()
    try:
        for index, frame_path in enumerate(frame_paths):
            frame = first_frame if index == 0 else load_image_bgr(frame_path)
            if frame.shape[0] != height or frame.shape[1] != width:
                frame = cv2.resize(frame, (width, height), interpolation=cv2.INTER_NEAREST)
            t0 = time.perf_counter()
            writer.write(frame)
            encode_times_ms.append((time.perf_counter() - t0) * 1000.0)
    finally:
        writer.release()

    wall_ms = (time.perf_counter() - started) * 1000.0
    avg_encode_ms = float(np.mean(encode_times_ms)) if encode_times_ms else 0.0
    max_encode_ms = float(np.max(encode_times_ms)) if encode_times_ms else 0.0
    total_encode_ms = float(np.sum(encode_times_ms)) if encode_times_ms else 0.0

    stats = ExportStats(
        folder_name=folder_name,
        output_dir=str(output_dir),
        frame_count=len(frame_paths),
        fps=round(float(fps), 6),
        width=width,
        height=height,
        avg_encode_ms=round(avg_encode_ms, 6),
        max_encode_ms=round(max_encode_ms, 6),
        total_encode_ms=round(total_encode_ms, 6),
        wall_ms=round(float(wall_ms), 6),
        mp4_path=str(mp4_path),
    )
    (output_dir / "mp4_export_summary.json").write_text(
        json.dumps(asdict(stats), ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    return stats


def export_all_existing_outputs() -> list[ExportStats]:
    output_dirs = sorted(path for path in FRAMES_DIR.iterdir() if path.is_dir() and path.name.endswith("_output"))
    stats_list = [export_output_dir(output_dir) for output_dir in output_dirs]
    (DATA_ROOT / "frames_mp4_summary.json").write_text(
        json.dumps([asdict(stats) for stats in stats_list], ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    return stats_list


def main() -> None:
    parser = argparse.ArgumentParser(description="Export existing draw PNG sequences to MP4.")
    parser.add_argument("--output-dir", type=Path, default=None, help="One *_output directory to export.")
    parser.add_argument("--all-existing-outputs", action="store_true", help="Export all frames/*_output folders.")
    args = parser.parse_args()

    if args.all_existing_outputs:
        stats_list = export_all_existing_outputs()
        for stats in stats_list:
            print(
                f"[{stats.folder_name}] frames={stats.frame_count} "
                f"fps={stats.fps:.3f} wall_ms={stats.wall_ms:.3f} "
                f"mp4={stats.mp4_path}"
            )
        return

    if args.output_dir is None:
        raise ValueError("Use --output-dir or --all-existing-outputs.")

    stats = export_output_dir(resolve_output_dir(args.output_dir))
    print(
        f"[{stats.folder_name}] frames={stats.frame_count} "
        f"fps={stats.fps:.3f} wall_ms={stats.wall_ms:.3f} "
        f"mp4={stats.mp4_path}"
    )


if __name__ == "__main__":
    main()
