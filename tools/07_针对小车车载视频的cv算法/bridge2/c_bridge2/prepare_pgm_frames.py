"""Desktop-only PNG to P5-PGM adapter for the pure-C detector."""

from __future__ import annotations

import argparse
import re
from pathlib import Path

from PIL import Image


FRAME_RE = re.compile(r"(frame_\d+)$")


def frame_stem(path: Path) -> str:
    match = FRAME_RE.search(path.stem)
    return match.group(1) if match else path.stem


def convert_frames(source: Path, output: Path, max_frames: int | None) -> int:
    frames = sorted(source.glob("*.png"))
    if max_frames is not None and max_frames > 0:
        frames = frames[:max_frames]
    if not frames:
        raise FileNotFoundError(f"no PNG frames found in {source}")
    output.mkdir(parents=True, exist_ok=True)
    for old in output.glob("*.pgm"):
        old.unlink()
    for frame in frames:
        with Image.open(frame) as image:
            gray = image.convert("L")
            header = f"P5\n{gray.width} {gray.height}\n255\n".encode("ascii")
            (output / f"{frame_stem(frame)}.pgm").write_bytes(header + gray.tobytes())
    return len(frames)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--frames", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--max-frames", type=int)
    args = parser.parse_args()
    count = convert_frames(args.frames, args.output, args.max_frames)
    print(f"prepared {count} PGM frames in {args.output}")


if __name__ == "__main__":
    main()
