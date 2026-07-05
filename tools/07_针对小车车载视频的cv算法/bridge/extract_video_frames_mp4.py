"""Extract MP4 video frames to PNG files.

This helper targets on-car MP4 recordings and stores each video's frames in:
data/雷区视觉/frames/{video_name}/frame_000001.png
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import cv2


DEFAULT_OUTPUT_ROOT = Path("data/雷区视觉/frames")


def save_png_unicode_safe(output_path: Path, frame) -> None:
    ok, encoded = cv2.imencode(".png", frame)
    if not ok:
        raise RuntimeError(f"failed to encode frame as PNG: {output_path}")
    output_path.write_bytes(encoded.tobytes())


def extract_frames(
    video_path: Path,
    output_root: Path,
    stride: int = 1,
    max_frames: int | None = None,
) -> dict:
    if stride < 1:
        raise ValueError("--stride must be >= 1")
    if not video_path.is_file():
        raise FileNotFoundError(f"video not found: {video_path}")

    output_dir = output_root / video_path.stem
    output_dir.mkdir(parents=True, exist_ok=True)

    capture = cv2.VideoCapture(str(video_path))
    if not capture.isOpened():
        raise RuntimeError(f"failed to open video: {video_path}")

    fps = capture.get(cv2.CAP_PROP_FPS) or None
    width = int(capture.get(cv2.CAP_PROP_FRAME_WIDTH) or 0)
    height = int(capture.get(cv2.CAP_PROP_FRAME_HEIGHT) or 0)
    frame_count_hint = int(capture.get(cv2.CAP_PROP_FRAME_COUNT) or 0)

    seen = 0
    saved = 0

    try:
        while True:
            ok, frame = capture.read()
            if not ok:
                break

            seen += 1
            if (seen - 1) % stride != 0:
                continue

            saved += 1
            output_path = output_dir / f"frame_{seen:06d}.png"
            save_png_unicode_safe(output_path, frame)

            if max_frames is not None and saved >= max_frames:
                break
    finally:
        capture.release()

    metadata = {
        "video": str(video_path),
        "output_dir": str(output_dir),
        "width": width,
        "height": height,
        "fps": fps,
        "frame_count_hint": frame_count_hint,
        "frames_seen": seen,
        "frames_saved": saved,
        "stride": stride,
    }
    (output_dir / "metadata.json").write_text(
        json.dumps(metadata, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    return metadata


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Extract MP4 video frames to PNG.")
    parser.add_argument("videos", nargs="+", type=Path, help="one or more MP4 files")
    parser.add_argument("--output-root", type=Path, default=DEFAULT_OUTPUT_ROOT)
    parser.add_argument("--stride", type=int, default=1, help="save every Nth frame")
    parser.add_argument("--max-frames", type=int, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    results = [
        extract_frames(video_path, args.output_root, args.stride, args.max_frames)
        for video_path in args.videos
    ]
    print(json.dumps(results, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
