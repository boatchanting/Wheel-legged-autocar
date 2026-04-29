from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path
from typing import BinaryIO

import numpy as np
from PIL import Image, ImageDraw

from bumpy_road_line_common import (
    DEFAULT_FRAME_DIR,
    DEFAULT_VIDEO_PATH,
    IMAGE_CENTER_X,
    ROI_X0,
    ROI_X1,
    ROI_Y0,
    ROI_Y1,
    detect_bumpy_road_frame,
    ensure_dir,
)


DEFAULT_OUTPUT_DIR = Path(r"E:\github_projects\autocar1\environment4BB7\Seekfree_CYT4BB_Opensource_Library\project2\data\bumpy_road_line_detection_video")
FPS = 50.0
SCALE = 4
FOURCC_DIB = b"DIB "


class DibAviWriter:
    def __init__(self, path: Path, width: int, height: int, fps: float) -> None:
        self.path = path
        self.width = width
        self.height = height
        self.fps = fps
        self.frame_count = 0
        self.index: list[tuple[int, int, int]] = []
        self.file: BinaryIO | None = None
        self.riff_size_pos = 0
        self.avih_frames_pos = 0
        self.strh_frames_pos = 0
        self.movi_list_start = 0
        self.movi_data_start = 0

    @property
    def stride(self) -> int:
        return ((self.width * 3 + 3) // 4) * 4

    @property
    def frame_size(self) -> int:
        return self.stride * self.height

    def _write(self, data: bytes) -> None:
        assert self.file is not None
        self.file.write(data)

    @staticmethod
    def _u32(value: int) -> bytes:
        return struct.pack("<I", value)

    @staticmethod
    def _i32(value: int) -> bytes:
        return struct.pack("<i", value)

    def __enter__(self) -> "DibAviWriter":
        ensure_dir(self.path.parent)
        self.file = self.path.open("wb")

        self._write(b"RIFF")
        self.riff_size_pos = self.file.tell()
        self._write(self._u32(0))
        self._write(b"AVI ")

        self._write_header()
        self.movi_list_start = self.file.tell()
        self._write(b"LIST")
        self._write(self._u32(0))
        self._write(b"movi")
        self.movi_data_start = self.file.tell()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        assert self.file is not None
        if exc_type is None:
            self._write_index()
            file_end = self.file.tell()

            movi_size = file_end - self.movi_list_start - 8
            self.file.seek(self.movi_list_start + 4)
            self._write(self._u32(movi_size))

            riff_size = file_end - 8
            self.file.seek(self.riff_size_pos)
            self._write(self._u32(riff_size))

            self.file.seek(self.avih_frames_pos)
            self._write(self._u32(self.frame_count))
            self.file.seek(self.strh_frames_pos)
            self._write(self._u32(self.frame_count))

        self.file.close()
        self.file = None

    def _write_list(self, list_type: bytes, payload: bytes) -> None:
        self._write(b"LIST")
        self._write(self._u32(len(payload) + 4))
        self._write(list_type)
        self._write(payload)
        if (len(payload) + 4) & 1:
            self._write(b"\x00")

    def _write_chunk(self, chunk_id: bytes, payload: bytes) -> None:
        self._write(chunk_id)
        self._write(self._u32(len(payload)))
        self._write(payload)
        if len(payload) & 1:
            self._write(b"\x00")

    def _write_header(self) -> None:
        assert self.file is not None
        hdrl_payload_start = self.file.tell()

        avih = bytearray()
        avih += self._u32(int(1_000_000 / self.fps))
        avih += self._u32(int(self.frame_size * self.fps))
        avih += self._u32(0)
        avih += self._u32(0x10)
        self.avih_frames_pos = 12 + 8 + 4 + 8 + len(avih)
        avih += self._u32(0)
        avih += self._u32(0)
        avih += self._u32(1)
        avih += self._u32(self.frame_size)
        avih += self._u32(self.width)
        avih += self._u32(self.height)
        avih += b"\x00" * 16

        strh = bytearray()
        strh += b"vids"
        strh += FOURCC_DIB
        strh += self._u32(0)
        strh += self._u32(0)
        strh += self._u32(0)
        strh += self._u32(1)
        strh += self._u32(int(self.fps))
        strh += self._u32(0)
        strh_frames_offset = len(strh)
        strh += self._u32(0)
        strh += self._u32(self.frame_size)
        strh += self._u32(0xFFFFFFFF)
        strh += self._u32(0)
        strh += struct.pack("<hhhh", 0, 0, self.width, self.height)

        strf = bytearray()
        strf += self._u32(40)
        strf += self._i32(self.width)
        strf += self._i32(self.height)
        strf += struct.pack("<H", 1)
        strf += struct.pack("<H", 24)
        strf += self._u32(0)
        strf += self._u32(self.frame_size)
        strf += self._i32(0)
        strf += self._i32(0)
        strf += self._u32(0)
        strf += self._u32(0)

        strl = bytearray()
        strl += b"strh" + self._u32(len(strh)) + strh
        self.strh_frames_pos = (
            hdrl_payload_start
            + 12
            + (8 + len(avih))
            + 8
            + 4
            + 8
            + strh_frames_offset
        )
        strl += b"strf" + self._u32(len(strf)) + strf

        hdrl = bytearray()
        hdrl += b"avih" + self._u32(len(avih)) + avih
        hdrl += b"LIST" + self._u32(len(strl) + 4) + b"strl" + strl
        self._write_list(b"hdrl", bytes(hdrl))

    def write_frame(self, image: Image.Image) -> None:
        assert self.file is not None
        if image.size != (self.width, self.height):
            raise ValueError(f"frame size mismatch: {image.size}, expected {(self.width, self.height)}")

        rgb = np.asarray(image.convert("RGB"), dtype=np.uint8)
        bgr = rgb[:, :, ::-1]
        padding = self.stride - self.width * 3
        rows = []
        for row in bgr[::-1]:
            row_bytes = row.tobytes()
            if padding:
                row_bytes += b"\x00" * padding
            rows.append(row_bytes)
        payload = b"".join(rows)

        chunk_start = self.file.tell()
        self._write_chunk(b"00db", payload)
        offset = chunk_start - self.movi_data_start
        self.index.append((0x10, offset, len(payload)))
        self.frame_count += 1

    def _write_index(self) -> None:
        payload = bytearray()
        for flags, offset, size in self.index:
            payload += b"00db"
            payload += self._u32(flags)
            payload += self._u32(offset)
            payload += self._u32(size)
        self._write_chunk(b"idx1", bytes(payload))


def make_overlay(rgb: np.ndarray, frame_index: int, detection: dict) -> Image.Image:
    image = Image.fromarray(rgb, mode="RGB").resize(
        (rgb.shape[1] * SCALE, rgb.shape[0] * SCALE),
        Image.Resampling.NEAREST,
    )
    draw = ImageDraw.Draw(image)

    draw.rectangle(
        [ROI_X0 * SCALE, ROI_Y0 * SCALE, (ROI_X1 + 1) * SCALE - 1, (ROI_Y1 + 1) * SCALE - 1],
        outline="cyan",
        width=1,
    )

    component = detection["best_component"]
    if component is not None:
        draw.rectangle(
            [
                component.xmin * SCALE,
                component.ymin * SCALE,
                (component.xmax + 1) * SCALE - 1,
                (component.ymax + 1) * SCALE - 1,
            ],
            outline="yellow",
            width=2,
        )

    for run in detection["centerline_runs"]:
        draw.line(
            [
                run.xmin * SCALE,
                run.y * SCALE + SCALE // 2,
                run.xmax * SCALE,
                run.y * SCALE + SCALE // 2,
            ],
            fill="deepskyblue",
            width=2,
        )

    for idx, band in enumerate(detection["rib_bands"], start=1):
        draw.rectangle(
            [
                band.xmin * SCALE,
                band.ymin * SCALE,
                (band.xmax + 1) * SCALE - 1,
                (band.ymax + 1) * SCALE - 1,
            ],
            outline="red",
            width=2,
        )
        draw.text((band.xmin * SCALE + 2, band.ymin * SCALE + 1), str(idx), fill="red")

    target_x = detection["centerline"]["target_x"]
    draw.line(
        [target_x * SCALE, ROI_Y0 * SCALE, target_x * SCALE, (ROI_Y1 + 1) * SCALE - 1],
        fill="lime",
        width=2,
    )
    draw.line(
        [IMAGE_CENTER_X * SCALE, ROI_Y0 * SCALE, IMAGE_CENTER_X * SCALE, (ROI_Y1 + 1) * SCALE - 1],
        fill="magenta",
        width=1,
    )

    status_color = {
        "approach_bumpy": (110, 80, 10),
        "inside_bumpy": (20, 110, 20),
        "exit_bumpy": (20, 60, 140),
        "white_surface_only": (70, 70, 70),
        "uncertain": (120, 40, 40),
    }.get(detection["phase"], (70, 70, 70))
    draw.rectangle([0, 0, image.width - 1, 30], fill=(0, 0, 0))
    draw.rectangle([0, 30, image.width - 1, 36], fill=status_color)
    draw.text(
        (4, 4),
        (
            f"frame={frame_index:06d} phase={detection['phase']} mode={detection['controller_mode']} "
            f"ribs={len(detection['rib_bands'])}"
        ),
        fill="white",
    )
    draw.text(
        (4, 18),
        (
            f"white={detection['white_threshold_int']} dark={int(round(detection['dark_threshold']))} "
            f"target_x={target_x:.1f} err={detection['centerline']['steer_error_px']:.1f}"
        ),
        fill="white",
    )
    return image


def timeline_entry(frame_index: int, frame_name: str, detection: dict) -> dict:
    component = detection["best_component"]
    centerline = detection["centerline"]
    return {
        "frame": frame_index,
        "frame_name": frame_name,
        "phase": detection["phase"],
        "mode": detection["controller_mode"],
        "white_threshold": round(float(detection["white_threshold"]), 2),
        "white_threshold_candidate": round(float(detection["white_threshold_candidate"]), 2),
        "dark_threshold": round(float(detection["dark_threshold"]), 2),
        "target_x": round(float(centerline["target_x"]), 2),
        "steer_error_px": round(float(centerline["steer_error_px"]), 2),
        "centerline_row_count": centerline["row_count"],
        "centerline_bottom_row_count": centerline["bottom_row_count"],
        "centerline_top_y": centerline["top_y"],
        "centerline_bottom_y": centerline["bottom_y"],
        "centerline_mean_width": round(float(centerline["mean_width"]), 2),
        "rib_count": len(detection["rib_bands"]),
        "best_component_bbox": (
            None if component is None else [component.xmin, component.ymin, component.xmax, component.ymax]
        ),
    }


def summarize_timeline(timeline: list[dict]) -> dict:
    phase_counts: dict[str, int] = {}
    for row in timeline:
        phase_counts[row["phase"]] = phase_counts.get(row["phase"], 0) + 1

    def first_frame(phase: str) -> int | None:
        for row in timeline:
            if row["phase"] == phase:
                return row["frame"]
        return None

    def last_frame(phase: str) -> int | None:
        for row in reversed(timeline):
            if row["phase"] == phase:
                return row["frame"]
        return None

    return {
        "frame_count": len(timeline),
        "phase_counts": phase_counts,
        "first_approach_frame": first_frame("approach_bumpy"),
        "first_inside_frame": first_frame("inside_bumpy"),
        "first_exit_frame": first_frame("exit_bumpy"),
        "last_inside_frame": last_frame("inside_bumpy"),
        "last_exit_frame": last_frame("exit_bumpy"),
    }


def write_outputs(output_dir: Path, frame_dir: Path, output_video: Path, timeline: list[dict]) -> None:
    summary = summarize_timeline(timeline)
    payload = {
        "frame_dir": str(frame_dir),
        "output_video": str(output_video),
        **summary,
        "timeline": timeline,
    }
    (output_dir / "full_line_detection_timeline.json").write_text(
        json.dumps(payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )

    lines = [
        "# Bumpy Road Full Line Detection",
        "",
        f"frame_dir: `{frame_dir}`",
        f"output_video: `{output_video}`",
        f"frame_count: `{summary['frame_count']}`",
        f"first_approach_frame: `{summary['first_approach_frame']}`",
        f"first_inside_frame: `{summary['first_inside_frame']}`",
        f"first_exit_frame: `{summary['first_exit_frame']}`",
        f"last_inside_frame: `{summary['last_inside_frame']}`",
        f"last_exit_frame: `{summary['last_exit_frame']}`",
        "",
        "phase_counts:",
    ]
    for phase, count in summary["phase_counts"].items():
        lines.append(f"- `{phase}`: `{count}`")
    lines.extend(
        [
            "",
            "control note:",
            "Use `steer_error_px` as the lateral line-following signal only after 3 to 5 consecutive frames keep the same phase.",
        ]
    )
    (output_dir / "full_line_detection_summary.md").write_text("\n".join(lines), encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run dynamic bumpy-road line following on a full frame sequence.")
    parser.add_argument("--video", type=Path, default=DEFAULT_VIDEO_PATH)
    parser.add_argument("--frames", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--output-name", default="full_line_detection_overlay.avi")
    parser.add_argument("--max-frames", type=int, default=None)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    frame_dir = args.frames
    output_dir = args.output_dir
    ensure_dir(output_dir)

    frame_paths = sorted(frame_dir.glob("frame_*.png"))
    if not frame_paths:
        raise FileNotFoundError(f"no frame_*.png files found in {frame_dir}")
    if args.max_frames is not None:
        frame_paths = frame_paths[: args.max_frames]

    first_rgb = np.asarray(Image.open(frame_paths[0]).convert("RGB"))
    output_video = output_dir / args.output_name
    prev_white_threshold = None
    timeline: list[dict] = []
    with DibAviWriter(output_video, first_rgb.shape[1] * SCALE, first_rgb.shape[0] * SCALE, FPS) as writer:
        for frame_index, frame_path in enumerate(frame_paths, start=1):
            rgb = np.asarray(Image.open(frame_path).convert("RGB"))
            detection = detect_bumpy_road_frame(rgb, prev_white_threshold=prev_white_threshold)
            prev_white_threshold = float(detection["white_threshold"])

            overlay = make_overlay(rgb, frame_index, detection)
            writer.write_frame(overlay)
            timeline.append(timeline_entry(frame_index, frame_path.name, detection))

    write_outputs(output_dir, frame_dir, output_video, timeline)
    summary = summarize_timeline(timeline)
    print(f"frame_dir: {frame_dir}")
    print(f"output_video: {output_video}")
    print(f"frame_count: {summary['frame_count']}")
    print(f"phase_counts: {summary['phase_counts']}")
    print(f"first_inside_frame: {summary['first_inside_frame']}")
    print(f"first_exit_frame: {summary['first_exit_frame']}")


if __name__ == "__main__":
    main()
