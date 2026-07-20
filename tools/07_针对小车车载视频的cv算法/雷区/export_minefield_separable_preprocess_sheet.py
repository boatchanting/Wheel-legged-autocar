"""Export contact sheets for the separable-kernel minefield preprocessing.

This script keeps only the preprocessing learned from
`detect_minefield_people_separable_edge_v1.py`:

    Gx = [-1, 0, 0, 1] * [1, 3, 3, 1]^T
    Gy = [1, 3, 3, 1] * [-1, 0, 0, 1]^T

The visualization image is the normalized L1 response `abs(gx) + abs(gy)`.
"""

from __future__ import annotations

import argparse
import math
from pathlib import Path

import cv2
import numpy as np


SMOOTH = np.array([1.0, 3.0, 3.0, 1.0], dtype=np.float32) / 8.0
DIFF = np.array([-1.0, 0.0, 0.0, 1.0], dtype=np.float32) / 2.0
IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".bmp"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run the minefield separable-kernel preprocessing on all frames and export PNG sheets."
    )
    parser.add_argument(
        "input_dirs",
        nargs="+",
        type=Path,
        help="Frame directories containing PNG/JPG images.",
    )
    parser.add_argument(
        "--output-name",
        default="separable_preprocess_sheet.png",
        help="Per-directory output PNG name.",
    )
    parser.add_argument(
        "--combined-output",
        type=Path,
        default=None,
        help="Optional PNG path for a sheet that combines all directories.",
    )
    return parser.parse_args()


def read_gray(path: Path) -> np.ndarray:
    data = np.fromfile(str(path), dtype=np.uint8)
    image = cv2.imdecode(data, cv2.IMREAD_GRAYSCALE)
    if image is None:
        raise ValueError(f"Failed to read image: {path}")
    return image


def write_png(path: Path, image: np.ndarray) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    ok, encoded = cv2.imencode(".png", image)
    if not ok:
        raise ValueError(f"Failed to encode png: {path}")
    encoded.tofile(str(path))


def separable_preprocess(gray: np.ndarray) -> np.ndarray:
    signal = gray.astype(np.float32)
    gx = cv2.sepFilter2D(signal, cv2.CV_32F, DIFF, SMOOTH, borderType=cv2.BORDER_REPLICATE)
    gy = cv2.sepFilter2D(signal, cv2.CV_32F, SMOOTH, DIFF, borderType=cv2.BORDER_REPLICATE)
    return np.abs(gx) + np.abs(gy)


def normalize_responses(responses: list[np.ndarray]) -> list[np.ndarray]:
    stacked = np.concatenate([response.reshape(-1) for response in responses]).astype(np.float32)
    high = float(np.percentile(stacked, 99.0))
    scale = high if high > 1e-6 else 1.0
    normalized: list[np.ndarray] = []
    for response in responses:
        image = np.clip(response * (255.0 / scale), 0.0, 255.0).astype(np.uint8)
        normalized.append(image)
    return normalized


def build_contact_sheet(images: list[np.ndarray]) -> np.ndarray:
    if not images:
        raise ValueError("No images provided for contact sheet.")
    height, width = images[0].shape[:2]
    columns = max(1, math.ceil(math.sqrt(len(images))))
    rows = math.ceil(len(images) / columns)
    sheet = np.zeros((rows * height, columns * width), dtype=np.uint8)
    for index, image in enumerate(images):
        row = index // columns
        col = index % columns
        y0 = row * height
        x0 = col * width
        sheet[y0 : y0 + height, x0 : x0 + width] = image
    return sheet


def iter_frame_paths(input_dir: Path) -> list[Path]:
    return sorted(
        path
        for path in input_dir.iterdir()
        if path.is_file() and path.suffix.lower() in IMAGE_SUFFIXES
    )


def process_directory(input_dir: Path, output_name: str) -> tuple[Path, list[np.ndarray]]:
    frame_paths = iter_frame_paths(input_dir)
    if not frame_paths:
        raise ValueError(f"No frames found in {input_dir}")
    responses = [separable_preprocess(read_gray(path)) for path in frame_paths]
    normalized = normalize_responses(responses)
    sheet = build_contact_sheet(normalized)
    output_path = input_dir / output_name
    write_png(output_path, sheet)
    return output_path, normalized


def main() -> None:
    args = parse_args()
    combined_images: list[np.ndarray] = []
    for input_dir in args.input_dirs:
        output_path, normalized = process_directory(input_dir, args.output_name)
        combined_images.extend(normalized)
        print(f"{input_dir} -> {output_path}")
    if args.combined_output is not None:
        combined_sheet = build_contact_sheet(combined_images)
        write_png(args.combined_output, combined_sheet)
        print(f"combined -> {args.combined_output}")


if __name__ == "__main__":
    main()
