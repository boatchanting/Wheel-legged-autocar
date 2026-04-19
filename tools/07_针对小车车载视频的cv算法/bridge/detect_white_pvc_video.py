"""Run white PVC detection on a full frame sequence and export an AVI overlay.

The environment intentionally does not require OpenCV or ffmpeg. The output
video is an uncompressed DIB AVI, which is large but easy to generate and
portable for debugging.
"""

from __future__ import annotations

import json
import argparse
import struct
from pathlib import Path
from typing import BinaryIO

import numpy as np
from PIL import Image, ImageDraw

from detect_white_pvc_samples import (
    FRAME_DIR as DEFAULT_FRAME_DIR,
    MIN_DECISION_SCORE,
    WHITE_THRESHOLD,
    filter_candidates,
    find_components,
)


PROJECT_ROOT = Path(__file__).resolve().parents[3]
DEFAULT_VIDEO_OUTPUT_DIR = PROJECT_ROOT / "data/bridge_white_pvc_detection_video"

FPS = 50
SCALE = 3
FOURCC_DIB = b"DIB "


class DibAviWriter:
    def __init__(self, path: Path, width: int, height: int, fps: int) -> None:
        self.path = path
        self.width = width
        self.height = height
        self.fps = fps
        self.frame_count = 0
        self.index: list[tuple[int, int, int, int]] = []
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

    def _u32(self, value: int) -> bytes:
        return struct.pack("<I", value)

    def _i32(self, value: int) -> bytes:
        return struct.pack("<i", value)

    def __enter__(self) -> "DibAviWriter":
        self.path.parent.mkdir(parents=True, exist_ok=True)
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

        # Main AVI header.
        avih = bytearray()
        avih += self._u32(int(1_000_000 / self.fps))
        avih += self._u32(self.frame_size * self.fps)
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

        # Stream header.
        strh = bytearray()
        strh += b"vids"
        strh += FOURCC_DIB
        strh += self._u32(0)
        strh += self._u32(0)
        strh += self._u32(0)
        strh += self._u32(1)
        strh += self._u32(self.fps)
        strh += self._u32(0)
        strh_frames_offset_in_strh = len(strh)
        strh += self._u32(0)
        strh += self._u32(self.frame_size)
        strh += self._u32(0xFFFFFFFF)
        strh += self._u32(0)
        strh += struct.pack("<hhhh", 0, 0, self.width, self.height)

        # Bitmap info header.
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
            + strh_frames_offset_in_strh
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
        self.index.append((0x10, offset, len(payload), 0))
        self.frame_count += 1

    def _write_index(self) -> None:
        idx_payload = bytearray()
        for flags, offset, size, _reserved in self.index:
            idx_payload += b"00db"
            idx_payload += self._u32(flags)
            idx_payload += self._u32(offset)
            idx_payload += self._u32(size)
        self._write_chunk(b"idx1", bytes(idx_payload))


def detect_white_pvc(rgb: np.ndarray) -> dict:
    gray = np.asarray(Image.fromarray(rgb).convert("L"))
    raw_mask = gray >= WHITE_THRESHOLD
    components = find_components(raw_mask, gray)
    candidates = filter_candidates(components, gray.size)
    best = candidates[0] if candidates else None
    detected = best is not None and best.score >= MIN_DECISION_SCORE
    return {
        "gray": gray,
        "raw_mask": raw_mask,
        "components": components,
        "best": best,
        "detected": detected,
    }


def make_overlay(rgb: np.ndarray, frame_index: int, detection: dict) -> Image.Image:
    base = Image.fromarray(rgb, mode="RGB").resize(
        (rgb.shape[1] * SCALE, rgb.shape[0] * SCALE),
        Image.Resampling.NEAREST,
    )
    draw = ImageDraw.Draw(base)

    best = detection["best"]
    detected = detection["detected"]
    mask = detection["raw_mask"]

    # Red translucent mask for raw white pixels.
    alpha = Image.fromarray((mask.astype(np.uint8) * 90), mode="L").resize(base.size, Image.Resampling.NEAREST)
    red = Image.new("RGB", base.size, (255, 0, 0))
    base = Image.composite(red, base, alpha)
    draw = ImageDraw.Draw(base)

    for component in detection["components"][:4]:
        color = "yellow"
        width = 1
        if best is component and detected:
            color = "lime"
            width = 2
        draw.rectangle(
            [
                component.xmin * SCALE,
                component.ymin * SCALE,
                (component.xmax + 1) * SCALE - 1,
                (component.ymax + 1) * SCALE - 1,
            ],
            outline=color,
            width=width,
        )

    status = "PVC" if detected else "NO_PVC"
    score = best.score if best is not None else 0.0
    area = best.area if best is not None else 0
    bottom_y = best.ymax if best is not None else -1

    draw.rectangle([0, 0, base.width - 1, 18], fill=(0, 0, 0))
    draw.text(
        (3, 3),
        f"frame={frame_index:06d} {status} score={score:.2f} area={area} bottom_y={bottom_y}",
        fill="white",
    )
    return base


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Detect white PVC on a full frame sequence.")
    parser.add_argument("--frames", type=Path, default=DEFAULT_FRAME_DIR)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_VIDEO_OUTPUT_DIR)
    parser.add_argument("--output-name", default="white_pvc_overlay.avi")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    frame_dir = args.frames
    output_dir = args.output_dir
    output_video = output_dir / args.output_name
    output_summary = output_dir / "video_summary.txt"
    output_json = output_dir / "video_summary.json"

    output_dir.mkdir(parents=True, exist_ok=True)
    frame_paths = sorted(frame_dir.glob("frame_*.png"))
    if not frame_paths:
        raise FileNotFoundError(f"no frames found in {frame_dir}")

    first = Image.open(frame_paths[0]).convert("RGB")
    out_width = first.width * SCALE
    out_height = first.height * SCALE

    detected_count = 0
    first_detected = None
    last_detected = None
    timeline: list[dict] = []

    with DibAviWriter(output_video, out_width, out_height, FPS) as writer:
        for idx, frame_path in enumerate(frame_paths, start=1):
            rgb = np.asarray(Image.open(frame_path).convert("RGB"))
            detection = detect_white_pvc(rgb)
            overlay = make_overlay(rgb, idx, detection)
            writer.write_frame(overlay)

            best = detection["best"]
            detected = detection["detected"]
            if detected:
                detected_count += 1
                if first_detected is None:
                    first_detected = idx
                last_detected = idx

            timeline.append(
                {
                    "frame": idx,
                    "detected": detected,
                    "score": round(best.score, 4) if best is not None else 0.0,
                    "area": best.area if best is not None else 0,
                    "bbox": [best.xmin, best.ymin, best.xmax, best.ymax] if best is not None else None,
                    "entry_bottom_y": best.ymax if best is not None else None,
                }
            )

    summary = {
        "frame_dir": str(frame_dir),
        "output_video": str(output_video),
        "fps": FPS,
        "scale": SCALE,
        "frame_count": len(frame_paths),
        "detected_count": detected_count,
        "first_detected_frame": first_detected,
        "last_detected_frame": last_detected,
        "threshold": WHITE_THRESHOLD,
        "decision_score": MIN_DECISION_SCORE,
    }
    output_json.write_text(
        json.dumps({"summary": summary, "timeline": timeline}, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    lines = [
        "白色 PVC 全视频检测结果",
        f"输入帧目录: {frame_dir}",
        f"输出视频: {output_video}",
        f"输出尺寸: {out_width}x{out_height}",
        f"FPS: {FPS}",
        f"总帧数: {len(frame_paths)}",
        f"检测到 PVC 帧数: {detected_count}",
        f"首次检测帧: {first_detected}",
        f"最后检测帧: {last_detected}",
        f"阈值: gray >= {WHITE_THRESHOLD}, score >= {MIN_DECISION_SCORE}",
        "",
        "控制使用建议:",
        "1. 对单帧检测结果做连续帧滤波，建议连续 3~5 帧 detected=true 后才认为看到 PVC。",
        "2. 使用 entry_bottom_y 查表得到入口距离；bottom_y 越大，PVC 越近。",
        "3. 使用 bbox 中心点与图像中心差值作为轻微横向纠偏，不要直接大幅覆盖 err_degree。",
        "4. 单边桥入口确认后记录 entry_pose，抬底盘，进入桥状态机。",
        "5. 通过中继续检测白色 PVC 中心线；黑色单边桥后续只做计数和左右侧别。",
    ]
    output_summary.write_text("\n".join(lines), encoding="utf-8")

    print(json.dumps(summary, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
