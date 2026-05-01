"""Convert PNG frames to PGM for C gray-red line detector."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_FRAME_DIR = PROJECT_ROOT / "data" / "frames" / "video_2026_05_01_10_01_22_from11s"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "line_gray_red_c_pgm_frames"


def write_pgm(path: Path, gray: Image.Image) -> None:
    header = f"P5\n{gray.width} {gray.height}\n255\n".encode("ascii")
    path.write_bytes(header + gray.tobytes())


def convert_frames(frame_dir: Path, output_dir: Path, max_frames: int | None) -> int:
    output_dir.mkdir(parents=True, exist_ok=True)
    for old in output_dir.glob("*.pgm"):
        old.unlink()
    frames = sorted(frame_dir.glob("frame_*.png"))
    if max_frames is not None:
        frames = frames[:max_frames]
    if not frames:
        raise FileNotFoundError(f"no frame_*.png found in {frame_dir}")
    for frame in frames:
        gray = Image.open(frame).convert("L")
        write_pgm(output_dir / f"{frame.stem}.pgm", gray)
    return len(frames)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert frames to P5 PGM.")
    parser.add_argument("--frames", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--max-frames", type=int, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    count = convert_frames(args.frames, args.output, args.max_frames)
    print(f"frames: {count}")
    print(f"output: {args.output}")


if __name__ == "__main__":
    main()
