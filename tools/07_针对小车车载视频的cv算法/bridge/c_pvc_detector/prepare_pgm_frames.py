"""Convert PNG video frames to binary PGM for the C PVC detector.

The C detector intentionally consumes raw 8-bit gray frames so the same core
can later run on the car with MT9V03X image buffers. This helper is only for
desktop testing against the existing PNG frame folders.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


PROJECT_ROOT = Path(__file__).resolve().parents[4]
DEFAULT_FRAME_DIR = PROJECT_ROOT / "data" / "frames" / "2026_04_17_21_18_39_Video"
DEFAULT_OUTPUT_DIR = PROJECT_ROOT / "data" / "bridge_white_pvc_c_pgm_frames"


def write_pgm(path: Path, gray: Image.Image) -> None:
    data = gray.tobytes()
    header = f"P5\n{gray.width} {gray.height}\n255\n".encode("ascii")
    path.write_bytes(header + data)


def convert_frames(frame_dir: Path, output_dir: Path, max_frames: int | None) -> int:
    output_dir.mkdir(parents=True, exist_ok=True)
    for old_frame in output_dir.glob("*.pgm"):
        old_frame.unlink()

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
    parser = argparse.ArgumentParser(description="Convert PNG frames to P5 PGM frames.")
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
