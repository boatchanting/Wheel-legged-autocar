from __future__ import annotations

import json
import math
import mimetypes
import os
import shutil
import struct
import threading
import time
import traceback
import webbrowser
import zlib
from dataclasses import dataclass
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, quote, unquote, urlparse

import cv2
import numpy as np
from PIL import Image


APP_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = APP_DIR.parents[2]
OUTPUT_ROOT = APP_DIR / "output"
UPLOAD_DIR = OUTPUT_ROOT / "uploads"
STREAM_DIR = OUTPUT_ROOT / "streams"
DECODED_DIR = OUTPUT_ROOT / "decoded"
SHOWCASE_DIR = OUTPUT_ROOT / "showcase"
PREVIEW_DIR = OUTPUT_ROOT / "preview"
SUMMARY_DIR = OUTPUT_ROOT / "summary"
INDEX_PATH = APP_DIR / "index.html"
DATA_DIR = PROJECT_ROOT / "data"

MAGIC = b"LBVC1"
HEADER_LEN_STRUCT = struct.Struct("<I")
PACKET_HEADER_STRUCT = struct.Struct("<BII")
PACKET_KEYFRAME = 1
PACKET_DELTA = 2
PACKET_SKIP = 3
RAW_UPLOAD_CHUNK = 64 * 1024
SHOWCASE_SIZE = (1280, 720)
WEB_PREVIEW_WIDTH = 720
WEB_PREVIEW_MAX_FRAMES = 72
SHOWCASE_BG = (15, 23, 34)
SHOWCASE_PANEL = (25, 35, 48)
SHOWCASE_ACCENT = (77, 185, 255)
SUPPORTED_SOURCE_SUFFIXES = {".avi", ".mp4", ".mov", ".mkv", ".lbvc"}
HISTORY_LIMIT = 12


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


for _path in (OUTPUT_ROOT, UPLOAD_DIR, STREAM_DIR, DECODED_DIR, SHOWCASE_DIR, PREVIEW_DIR, SUMMARY_DIR):
    ensure_dir(_path)


def now_tag() -> str:
    return time.strftime("%Y%m%d_%H%M%S")


def sanitize_name(name: str) -> str:
    cleaned = []
    for ch in name:
        if ch in '\\/:*?"<>|':
            cleaned.append("_")
        else:
            cleaned.append(ch)
    result = "".join(cleaned).strip().strip(".")
    return result or f"file_{now_tag()}"


def json_bytes(payload: dict) -> bytes:
    return json.dumps(payload, ensure_ascii=False).encode("utf-8")


def rel_to_output(path: Path) -> str:
    return path.resolve().relative_to(OUTPUT_ROOT.resolve()).as_posix()


def file_info(path: Path) -> dict:
    resolved = path.resolve()
    url = None
    try:
        url = f"/files/{quote(rel_to_output(resolved))}"
    except ValueError:
        url = None
    return {
        "name": resolved.name,
        "path": str(resolved),
        "url": url,
        "size": resolved.stat().st_size if resolved.exists() else 0,
    }


def write_unicode_image(path: Path, image_bgr: np.ndarray) -> None:
    ensure_dir(path.parent)
    suffix = path.suffix.lower() or ".png"
    ok, encoded = cv2.imencode(suffix, image_bgr)
    if not ok:
        raise RuntimeError(f"failed to encode image: {path}")
    path.write_bytes(encoded.tobytes())


@dataclass(frozen=True)
class VideoInfo:
    path: Path
    width: int
    height: int
    fps: float
    frame_count_hint: int


def probe_video(path: Path) -> VideoInfo:
    cap = cv2.VideoCapture(str(path))
    if not cap.isOpened():
        raise RuntimeError(f"无法打开视频: {path}")
    try:
        width = int(round(cap.get(cv2.CAP_PROP_FRAME_WIDTH)))
        height = int(round(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)))
        fps = float(cap.get(cv2.CAP_PROP_FPS) or 0.0)
        frame_count_hint = int(round(cap.get(cv2.CAP_PROP_FRAME_COUNT) or 0.0))
    finally:
        cap.release()

    if width <= 0 or height <= 0:
        raise RuntimeError(f"无法识别视频分辨率: {path}")
    if fps <= 0:
        fps = 25.0

    return VideoInfo(
        path=path,
        width=width,
        height=height,
        fps=fps,
        frame_count_hint=frame_count_hint,
    )


def iter_capture_frames(cap: cv2.VideoCapture, frame_count_hint: int = 0):
    # Some AVI files emit FFmpeg warnings when we probe one read past EOF.
    # If the container exposes a frame count, stop exactly on that boundary.
    if frame_count_hint > 0:
        for _ in range(frame_count_hint):
            ok, frame = cap.read()
            if not ok:
                break
            yield frame
        return

    while True:
        ok, frame = cap.read()
        if not ok:
            break
        yield frame


def compute_target_size(src_width: int, src_height: int, scale: float) -> tuple[int, int]:
    ratio = min(scale, 1.0)
    dst_width = max(1, int(round(src_width * ratio)))
    dst_height = max(1, int(round(src_height * ratio)))
    return dst_width, dst_height


def prepare_frame(frame_bgr: np.ndarray, size: tuple[int, int], color_mode: str) -> np.ndarray:
    src_h, src_w = frame_bgr.shape[:2]
    if (src_w, src_h) != size:
        frame_bgr = cv2.resize(frame_bgr, size, interpolation=cv2.INTER_AREA)
    if color_mode == "gray":
        return cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2GRAY)
    return frame_bgr


def encode_intra(frame: np.ndarray, intra_codec: str, jpeg_quality: int) -> bytes:
    if intra_codec == "png":
        ok, encoded = cv2.imencode(".png", frame, [cv2.IMWRITE_PNG_COMPRESSION, 3])
        if not ok:
            raise RuntimeError("PNG ????")
        return encoded.tobytes()
    ok, encoded = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, jpeg_quality])
    if not ok:
        raise RuntimeError("JPEG ????")
    return encoded.tobytes()


def decode_intra(data: bytes, color_mode: str, intra_codec: str) -> np.ndarray:
    encoded = np.frombuffer(data, dtype=np.uint8)
    flag = cv2.IMREAD_GRAYSCALE if color_mode == "gray" else cv2.IMREAD_COLOR
    frame = cv2.imdecode(encoded, flag)
    if frame is None:
        raise RuntimeError(f"{intra_codec.upper()} ????")
    return frame


def frame_mean_absdiff(current: np.ndarray, reference: np.ndarray) -> float:
    return float(cv2.absdiff(current, reference).mean())


def block_grid(width: int, height: int, block_size: int) -> tuple[int, int, int]:
    blocks_x = math.ceil(width / block_size)
    blocks_y = math.ceil(height / block_size)
    return blocks_x, blocks_y, blocks_x * blocks_y


def encode_delta(
    current: np.ndarray,
    reference: np.ndarray,
    block_size: int,
    change_threshold: float,
    zlib_level: int,
) -> tuple[bytes, np.ndarray]:
    height, width = current.shape[:2]
    blocks_x, blocks_y, total_blocks = block_grid(width, height, block_size)
    flags = np.zeros(total_blocks, dtype=np.uint8)
    changed_chunks: list[bytes] = []
    reconstructed = reference.copy()

    index = 0
    for by in range(blocks_y):
        y0 = by * block_size
        y1 = min(y0 + block_size, height)
        for bx in range(blocks_x):
            x0 = bx * block_size
            x1 = min(x0 + block_size, width)
            current_block = current[y0:y1, x0:x1]
            reference_block = reference[y0:y1, x0:x1]
            if float(cv2.absdiff(current_block, reference_block).mean()) >= change_threshold:
                flags[index] = 1
                changed_chunks.append(current_block.tobytes())
                reconstructed[y0:y1, x0:x1] = current_block
            index += 1

    packed_flags = np.packbits(flags, bitorder="little").tobytes()
    raw_payload = packed_flags + b"".join(changed_chunks)
    return zlib.compress(raw_payload, level=zlib_level), reconstructed


def decode_delta(payload: bytes, reference: np.ndarray, width: int, height: int, block_size: int) -> np.ndarray:
    raw_payload = zlib.decompress(payload)
    blocks_x, blocks_y, total_blocks = block_grid(width, height, block_size)
    flag_len = (total_blocks + 7) // 8
    packed_flags = raw_payload[:flag_len]
    block_bytes = raw_payload[flag_len:]
    flags = np.unpackbits(np.frombuffer(packed_flags, dtype=np.uint8), bitorder="little")[:total_blocks]

    reconstructed = reference.copy()
    cursor = 0
    index = 0
    channels = 1 if reconstructed.ndim == 2 else reconstructed.shape[2]

    for by in range(blocks_y):
        y0 = by * block_size
        y1 = min(y0 + block_size, height)
        block_h = y1 - y0
        for bx in range(blocks_x):
            x0 = bx * block_size
            x1 = min(x0 + block_size, width)
            if flags[index]:
                block_w = x1 - x0
                block_len = block_h * block_w * channels
                chunk = block_bytes[cursor : cursor + block_len]
                if len(chunk) != block_len:
                    raise RuntimeError("差分帧数据损坏")
                block_array = np.frombuffer(chunk, dtype=np.uint8)
                if channels == 1:
                    block_array = block_array.reshape(block_h, block_w)
                else:
                    block_array = block_array.reshape(block_h, block_w, channels)
                reconstructed[y0:y1, x0:x1] = block_array
                cursor += block_len
            index += 1

    return reconstructed


def write_header(stream, header: dict) -> None:
    payload = json.dumps(header, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
    stream.write(MAGIC)
    stream.write(HEADER_LEN_STRUCT.pack(len(payload)))
    stream.write(payload)


def read_header(stream) -> dict:
    magic = stream.read(len(MAGIC))
    if magic != MAGIC:
        raise RuntimeError("不是受支持的 LBVC 文件")
    header_len_bytes = stream.read(HEADER_LEN_STRUCT.size)
    if len(header_len_bytes) != HEADER_LEN_STRUCT.size:
        raise RuntimeError("LBVC 头损坏")
    header_len = HEADER_LEN_STRUCT.unpack(header_len_bytes)[0]
    payload = stream.read(header_len)
    if len(payload) != header_len:
        raise RuntimeError("LBVC 头不完整")
    return json.loads(payload.decode("utf-8"))


def write_packet(stream, packet_type: int, frame_index: int, payload: bytes) -> int:
    stream.write(PACKET_HEADER_STRUCT.pack(packet_type, frame_index, len(payload)))
    stream.write(payload)
    return PACKET_HEADER_STRUCT.size + len(payload)


def read_packet_header(stream) -> tuple[int, int, int] | None:
    header = stream.read(PACKET_HEADER_STRUCT.size)
    if not header:
        return None
    if len(header) != PACKET_HEADER_STRUCT.size:
        raise RuntimeError("LBVC 包头损坏")
    return PACKET_HEADER_STRUCT.unpack(header)


def open_temp_video_writer(output_path: Path, fps: float, frame_size: tuple[int, int], codec: str = "mp4v"):
    ensure_dir(output_path.parent)
    temp_path = output_path.parent / f"__temp_{output_path.stem}_{int(time.time() * 1000)}{output_path.suffix}"
    if temp_path.exists():
        temp_path.unlink()
    fourcc = cv2.VideoWriter_fourcc(*codec)
    writer = cv2.VideoWriter(str(temp_path), fourcc, fps, frame_size)
    if not writer.isOpened():
        raise RuntimeError(f"无法创建视频: {output_path}")
    return writer, temp_path


def as_bgr(frame: np.ndarray) -> np.ndarray:
    if frame.ndim == 2:
        return cv2.cvtColor(frame, cv2.COLOR_GRAY2BGR)
    return frame


def encode_stream_file(
    input_path: Path,
    output_path: Path,
    target_bitrate_kbps: float,
    scale: float,
    color_mode: str,
    jpeg_quality: int,
    block_size: int = 8,
    change_threshold: float = 0.0,
    keyframe_interval: int = 15,
    delta_ratio_threshold: float = 0.92,
    zlib_level: int = 6,
    frame_step: int = 1,
    skip_threshold: float = 0.0,
    intra_codec: str = "png",
) -> dict:
    info = probe_video(input_path)
    target_size = compute_target_size(info.width, info.height, scale)
    stream_fps = info.fps / frame_step
    header = {
        "version": 1,
        "source_video": str(input_path),
        "source_width": info.width,
        "source_height": info.height,
        "source_fps": info.fps,
        "source_frame_count_hint": info.frame_count_hint,
        "stream_width": target_size[0],
        "stream_height": target_size[1],
        "stream_fps": stream_fps,
        "frame_step": frame_step,
        "color_mode": color_mode,
        "target_bitrate_kbps": target_bitrate_kbps,
        "scale": scale,
        "jpeg_quality": jpeg_quality,
        "block_size": block_size,
        "change_threshold": change_threshold,
        "keyframe_interval": keyframe_interval,
        "delta_ratio_threshold": delta_ratio_threshold,
        "zlib_level": zlib_level,
        "skip_threshold": skip_threshold,
        "intra_codec": intra_codec,
        "created_at": time.strftime("%Y-%m-%d %H:%M:%S"),
    }

    cap = cv2.VideoCapture(str(input_path))
    if not cap.isOpened():
        raise RuntimeError(f"无法打开视频: {input_path}")

    processed = 0
    encoded_frames = 0
    keyframes = 0
    delta_frames = 0
    skip_frames = 0
    stream_bytes = len(MAGIC) + HEADER_LEN_STRUCT.size
    prev_reconstructed = None
    start_time = time.perf_counter()
    try:
        with output_path.open("wb") as stream:
            write_header(stream, header)
            stream_bytes += len(json.dumps(header, ensure_ascii=False, separators=(",", ":")).encode("utf-8"))

            for frame_bgr in iter_capture_frames(cap, info.frame_count_hint):
                source_index = processed
                processed += 1
                if source_index % frame_step != 0:
                    continue

                current = prepare_frame(frame_bgr, target_size, color_mode)
                force_keyframe = prev_reconstructed is None or encoded_frames % keyframe_interval == 0
                if not force_keyframe and prev_reconstructed is not None:
                    if frame_mean_absdiff(current, prev_reconstructed) <= skip_threshold:
                        stream_bytes += write_packet(stream, PACKET_SKIP, encoded_frames, b"")
                        skip_frames += 1
                        encoded_frames += 1
                        continue
                intra_payload = encode_intra(current, intra_codec, jpeg_quality)
                if force_keyframe:
                    stream_bytes += write_packet(stream, PACKET_KEYFRAME, encoded_frames, intra_payload)
                    prev_reconstructed = decode_intra(intra_payload, color_mode, intra_codec)
                    keyframes += 1
                else:
                    delta_payload, reconstructed = encode_delta(
                        current,
                        prev_reconstructed,
                        block_size,
                        change_threshold,
                        zlib_level,
                    )
                    if len(delta_payload) < len(intra_payload) * delta_ratio_threshold:
                        stream_bytes += write_packet(stream, PACKET_DELTA, encoded_frames, delta_payload)
                        prev_reconstructed = reconstructed
                        delta_frames += 1
                    else:
                        stream_bytes += write_packet(stream, PACKET_KEYFRAME, encoded_frames, intra_payload)
                        prev_reconstructed = decode_intra(intra_payload, color_mode, intra_codec)
                        keyframes += 1
                encoded_frames += 1
    finally:
        cap.release()

    elapsed = max(time.perf_counter() - start_time, 1e-6)
    avg_frame_bytes = stream_bytes / max(encoded_frames, 1)
    estimated_network_fps = min(stream_fps, (target_bitrate_kbps * 1000.0 / 8.0) / avg_frame_bytes)
    estimated_bandwidth_kbps = avg_frame_bytes * stream_fps * 8.0 / 1000.0

    return {
        "input": str(input_path),
        "output": str(output_path),
        "source_width": info.width,
        "source_height": info.height,
        "source_fps": round(info.fps, 3),
        "stream_width": target_size[0],
        "stream_height": target_size[1],
        "stream_fps": round(stream_fps, 3),
        "frames_encoded": encoded_frames,
        "keyframes": keyframes,
        "delta_frames": delta_frames,
        "skip_frames": skip_frames,
        "stream_bytes": stream_bytes,
        "avg_frame_bytes": round(avg_frame_bytes, 2),
        "estimated_network_fps_at_target": round(estimated_network_fps, 2),
        "estimated_realtime_bandwidth_kbps": round(estimated_bandwidth_kbps, 2),
        "encode_elapsed_sec": round(elapsed, 3),
        "color_mode": color_mode,
    }


def inspect_stream_file(input_path: Path) -> dict:
    with input_path.open("rb") as stream:
        header = read_header(stream)
        packets = 0
        keyframes = 0
        delta_frames = 0
        skip_frames = 0
        payload_bytes = 0
        while True:
            packet_header = read_packet_header(stream)
            if packet_header is None:
                break
            packet_type, _frame_index, payload_len = packet_header
            payload = stream.read(payload_len)
            if len(payload) != payload_len:
                raise RuntimeError("LBVC 包体损坏")
            packets += 1
            payload_bytes += PACKET_HEADER_STRUCT.size + payload_len
            if packet_type == PACKET_KEYFRAME:
                keyframes += 1
            elif packet_type == PACKET_DELTA:
                delta_frames += 1
            elif packet_type == PACKET_SKIP:
                skip_frames += 1
    avg_frame_bytes = payload_bytes / max(packets, 1)
    stream_fps = float(header["stream_fps"])
    target_bitrate = float(header["target_bitrate_kbps"])
    estimated_network_fps = min(stream_fps, (target_bitrate * 1000.0 / 8.0) / avg_frame_bytes)
    return {
        "stream_width": int(header["stream_width"]),
        "stream_height": int(header["stream_height"]),
        "stream_fps": round(stream_fps, 3),
        "packets": packets,
        "keyframes": keyframes,
        "delta_frames": delta_frames,
        "skip_frames": skip_frames,
        "avg_frame_bytes": round(avg_frame_bytes, 2),
        "estimated_network_fps_at_target": round(estimated_network_fps, 2),
        "target_bitrate_kbps": target_bitrate,
        "color_mode": header["color_mode"],
        "header": header,
    }


def decode_stream_file(input_path: Path, output_path: Path) -> dict:
    with input_path.open("rb") as stream:
        header = read_header(stream)
        width = int(header["stream_width"])
        height = int(header["stream_height"])
        color_mode = str(header["color_mode"])
        intra_codec = str(header.get("intra_codec", "jpg"))
        fps = float(header["stream_fps"])

        writer, temp_path = open_temp_video_writer(output_path, fps, (width, height), "mp4v")
        decoded_frames = 0
        keyframes = 0
        delta_frames = 0
        skip_frames = 0
        prev_frame = None
        start_time = time.perf_counter()
        try:
            while True:
                packet_header = read_packet_header(stream)
                if packet_header is None:
                    break
                packet_type, _frame_index, payload_len = packet_header
                payload = stream.read(payload_len)
                if len(payload) != payload_len:
                    raise RuntimeError("LBVC 包体损坏")

                if packet_type == PACKET_KEYFRAME:
                    frame = decode_intra(payload, color_mode, intra_codec)
                    keyframes += 1
                elif packet_type == PACKET_DELTA:
                    if prev_frame is None:
                        raise RuntimeError("首帧不能是差分帧")
                    frame = decode_delta(payload, prev_frame, width, height, int(header["block_size"]))
                    delta_frames += 1
                elif packet_type == PACKET_SKIP:
                    if prev_frame is None:
                        raise RuntimeError("首帧不能是跳帧包")
                    frame = prev_frame.copy()
                    skip_frames += 1
                else:
                    raise RuntimeError(f"未知包类型: {packet_type}")

                writer.write(as_bgr(frame))
                prev_frame = frame
                decoded_frames += 1
        finally:
            writer.release()

    if output_path.exists():
        output_path.unlink()
    shutil.move(str(temp_path), str(output_path))
    elapsed = max(time.perf_counter() - start_time, 1e-6)
    return {
        "output": str(output_path),
        "stream_width": width,
        "stream_height": height,
        "stream_fps": round(fps, 3),
        "frames_decoded": decoded_frames,
        "keyframes": keyframes,
        "delta_frames": delta_frames,
        "skip_frames": skip_frames,
        "decode_elapsed_sec": round(elapsed, 3),
        "color_mode": color_mode,
    }


def letterbox_frame(frame_bgr: np.ndarray, box_size: tuple[int, int], background: tuple[int, int, int]) -> np.ndarray:
    box_w, box_h = box_size
    canvas = np.full((box_h, box_w, 3), background, dtype=np.uint8)
    src_h, src_w = frame_bgr.shape[:2]
    ratio = min(box_w / src_w, box_h / src_h)
    draw_w = max(1, int(round(src_w * ratio)))
    draw_h = max(1, int(round(src_h * ratio)))
    resized = cv2.resize(frame_bgr, (draw_w, draw_h), interpolation=cv2.INTER_NEAREST)
    x0 = (box_w - draw_w) // 2
    y0 = (box_h - draw_h) // 2
    canvas[y0 : y0 + draw_h, x0 : x0 + draw_w] = resized
    return canvas


def draw_text_lines(
    image: np.ndarray,
    origin: tuple[int, int],
    lines: list[str],
    color: tuple[int, int, int],
    font_scale: float = 0.65,
    thickness: int = 1,
    line_gap: int = 28,
) -> None:
    x, y = origin
    for index, line in enumerate(lines):
        cv2.putText(
            image,
            line,
            (x, y + index * line_gap),
            cv2.FONT_HERSHEY_SIMPLEX,
            font_scale,
            color,
            thickness,
            cv2.LINE_AA,
        )


def draw_panel(canvas: np.ndarray, rect: tuple[int, int, int, int], frame_bgr: np.ndarray | None, title: str, subtitle: str) -> None:
    x0, y0, x1, y1 = rect
    panel_w = x1 - x0
    panel_h = y1 - y0
    cv2.rectangle(canvas, (x0, y0), (x1, y1), SHOWCASE_PANEL, -1)
    cv2.rectangle(canvas, (x0, y0), (x1, y1), SHOWCASE_ACCENT, 2)
    if frame_bgr is not None:
        preview = letterbox_frame(frame_bgr, (panel_w - 24, panel_h - 74), SHOWCASE_PANEL)
        canvas[y0 + 56 : y0 + 56 + preview.shape[0], x0 + 12 : x0 + 12 + preview.shape[1]] = preview
    draw_text_lines(canvas, (x0 + 16, y0 + 30), [title, subtitle], (235, 241, 245), font_scale=0.56, line_gap=22)


def create_showcase_video(original_video: Path | None, decoded_video: Path, output_path: Path, summary: dict, preview_path: Path, frame_step: int) -> dict:
    decoded_cap = cv2.VideoCapture(str(decoded_video))
    if not decoded_cap.isOpened():
        raise RuntimeError(f"无法打开解码视频: {decoded_video}")

    original_cap = None
    if original_video is not None:
        cap = cv2.VideoCapture(str(original_video))
        if cap.isOpened():
            original_cap = cap
        else:
            cap.release()

    fps = decoded_cap.get(cv2.CAP_PROP_FPS) or 25.0
    frame_count = int(round(decoded_cap.get(cv2.CAP_PROP_FRAME_COUNT) or 0.0))
    writer, temp_path = open_temp_video_writer(output_path, fps, SHOWCASE_SIZE, "mp4v")
    preview_written = False
    written_frames = 0
    left_rect = (34, 86, 622, 506)
    right_rect = (658, 86, 1246, 506)
    stats_rect = (34, 540, 1246, 684)

    try:
        for decoded_frame in iter_capture_frames(decoded_cap, frame_count):
            source_frame = None
            if original_cap is not None:
                ok_source, source_frame = original_cap.read()
                if ok_source:
                    for _ in range(max(0, frame_step - 1)):
                        original_cap.grab()
                else:
                    source_frame = None

            written_frames += 1
            canvas = np.full((SHOWCASE_SIZE[1], SHOWCASE_SIZE[0], 3), SHOWCASE_BG, dtype=np.uint8)
            cv2.rectangle(canvas, (0, 0), (SHOWCASE_SIZE[0] - 1, 58), (22, 30, 42), -1)
            draw_text_lines(
                canvas,
                (28, 36),
                ["Low-Bandwidth Video Showcase"],
                (245, 248, 251),
                font_scale=0.9,
                thickness=2,
            )

            draw_panel(canvas, left_rect, source_frame, "Original", summary.get("source_spec", "not available"))
            draw_panel(canvas, right_rect, decoded_frame, "Decoded", summary.get("stream_spec", ""))

            x0, y0, x1, y1 = stats_rect
            cv2.rectangle(canvas, (x0, y0), (x1, y1), (22, 30, 42), -1)
            cv2.rectangle(canvas, (x0, y0), (x1, y1), (78, 94, 118), 1)
            lines = [
                f"source: {summary.get('source_name', '-')}",
                f"source spec: {summary.get('source_spec', 'not available')}",
                f"stream spec: {summary.get('stream_spec', '-')}",
                f"avg bytes/frame: {summary.get('avg_frame_bytes', '-')}",
                f"estimated bandwidth: {summary.get('bandwidth_kbps', '-')} kbps",
                f"estimated fps @ target: {summary.get('target_fps', '-')}",
                f"frame: {written_frames}/{frame_count if frame_count else '?'}",
            ]
            draw_text_lines(canvas, (x0 + 18, y0 + 34), lines, (230, 235, 240), font_scale=0.7, thickness=2, line_gap=30)
            writer.write(canvas)

            if not preview_written:
                write_unicode_image(preview_path, canvas)
                preview_written = True
    finally:
        writer.release()
        decoded_cap.release()
        if original_cap is not None:
            original_cap.release()

    if output_path.exists():
        output_path.unlink()
    shutil.move(str(temp_path), str(output_path))
    return {
        "showcase_video": str(output_path),
        "preview_image": str(preview_path),
        "frames_written": written_frames,
        "fps": round(fps, 3),
    }


def extract_source_preview(source_path: Path, preview_path: Path) -> Path | None:
    if source_path.suffix.lower() == ".lbvc":
        return None
    cap = cv2.VideoCapture(str(source_path))
    if not cap.isOpened():
        return None
    ok, frame = cap.read()
    cap.release()
    if not ok or frame is None:
        return None
    canvas = letterbox_frame(frame, (960, 540), SHOWCASE_PANEL)
    write_unicode_image(preview_path, canvas)
    return preview_path


def create_web_preview_animation(video_path: Path, output_path: Path) -> Path:
    cap = cv2.VideoCapture(str(video_path))
    if not cap.isOpened():
        raise RuntimeError(f"无法打开展示视频: {video_path}")

    fps = cap.get(cv2.CAP_PROP_FPS) or 25.0
    frame_count = int(round(cap.get(cv2.CAP_PROP_FRAME_COUNT) or 0.0))
    stride = max(1, math.ceil(frame_count / WEB_PREVIEW_MAX_FRAMES)) if frame_count > 0 else 1
    frames: list[Image.Image] = []
    index = 0

    try:
        for frame in iter_capture_frames(cap, frame_count):
            if index % stride == 0:
                rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                image = Image.fromarray(rgb)
                if image.width > WEB_PREVIEW_WIDTH:
                    ratio = WEB_PREVIEW_WIDTH / image.width
                    image = image.resize(
                        (WEB_PREVIEW_WIDTH, max(1, int(round(image.height * ratio)))),
                        Image.Resampling.BILINEAR,
                    )
                frames.append(image)
                if len(frames) >= WEB_PREVIEW_MAX_FRAMES:
                    break
            index += 1
    finally:
        cap.release()

    if not frames:
        raise RuntimeError("展示视频没有可用帧")

    ensure_dir(output_path.parent)
    duration_ms = max(20, int(round(1000 * stride / fps)))
    frames[0].save(
        output_path,
        format="WEBP",
        save_all=True,
        append_images=frames[1:],
        duration=duration_ms,
        loop=0,
        quality=78,
        method=4,
    )
    return output_path


def create_browser_video_webm(input_video: Path, output_path: Path) -> Path | None:
    cap = cv2.VideoCapture(str(input_video))
    if not cap.isOpened():
        return None
    fps = cap.get(cv2.CAP_PROP_FPS) or 25.0
    width = int(round(cap.get(cv2.CAP_PROP_FRAME_WIDTH) or 0))
    height = int(round(cap.get(cv2.CAP_PROP_FRAME_HEIGHT) or 0))
    frame_count = int(round(cap.get(cv2.CAP_PROP_FRAME_COUNT) or 0))
    if width <= 0 or height <= 0:
        cap.release()
        return None

    try:
        writer, temp_path = open_temp_video_writer(output_path, fps, (width, height), "VP80")
    except Exception:
        cap.release()
        return None

    written = 0
    try:
        for frame in iter_capture_frames(cap, frame_count):
            writer.write(frame)
            written += 1
    finally:
        writer.release()
        cap.release()

    if written <= 0:
        try:
            if temp_path.exists():
                temp_path.unlink()
        except Exception:
            pass
        return None

    if output_path.exists():
        output_path.unlink()
    shutil.move(str(temp_path), str(output_path))
    return output_path


def create_output_paths(stem: str) -> dict[str, Path]:
    base = f"{sanitize_name(stem)}_{now_tag()}"
    return {
        "stream": STREAM_DIR / f"{base}.lbvc",
        "decoded": DECODED_DIR / f"{base}_decoded.mp4",
        "showcase": SHOWCASE_DIR / f"{base}_showcase.mp4",
        "preview": PREVIEW_DIR / f"{base}_showcase.png",
        "web_preview": PREVIEW_DIR / f"{base}_showcase.webp",
        "web_video": SHOWCASE_DIR / f"{base}_showcase.webm",
        "source_preview": PREVIEW_DIR / f"{base}_source.png",
        "summary": SUMMARY_DIR / f"{base}_summary.json",
    }


def process_video_pipeline(input_path: Path, params: dict[str, object]) -> dict:
    input_path = input_path.resolve()
    suffix = input_path.suffix.lower()
    if suffix not in SUPPORTED_SOURCE_SUFFIXES:
        raise RuntimeError(f"暂不支持该文件类型: {suffix}")

    target_bitrate_kbps = float(params.get("target_bitrate_kbps", 200.0))
    scale = float(params.get("scale", 1.0))
    jpeg_quality = int(params.get("jpeg_quality", 95))
    color_mode = str(params.get("color_mode", "color"))
    skip_threshold = float(params.get("skip_threshold", 0.0))
    if scale <= 0 or scale > 1:
        raise RuntimeError("scale 必须在 0~1 之间")
    if jpeg_quality < 10 or jpeg_quality > 95:
        raise RuntimeError("jpeg_quality 必须在 10~95 之间")
    if color_mode not in ("gray", "color"):
        raise RuntimeError("color_mode 仅支持 gray 或 color")
    if skip_threshold < 0:
        raise RuntimeError("skip_threshold 必须大于等于 0")

    paths = create_output_paths(input_path.stem)
    summary = {
        "source_name": input_path.name,
        "source_path": str(input_path),
    }

    encode_result = None
    if suffix == ".lbvc":
        stream_path = input_path
        inspect_result = inspect_stream_file(stream_path)
        decode_result = decode_stream_file(stream_path, paths["decoded"])
        header = inspect_result["header"]
    else:
        probe = probe_video(input_path)
        summary["source_spec"] = f"{probe.width}x{probe.height} @ {probe.fps:.2f}fps"
        encode_result = encode_stream_file(
            input_path=input_path,
            output_path=paths["stream"],
            target_bitrate_kbps=target_bitrate_kbps,
            scale=scale,
            color_mode=color_mode,
            jpeg_quality=jpeg_quality,
            skip_threshold=skip_threshold,
            intra_codec="png",
        )
        stream_path = Path(encode_result["output"])
        inspect_result = inspect_stream_file(stream_path)
        decode_result = decode_stream_file(stream_path, paths["decoded"])
        header = inspect_result["header"]

    summary["stream_spec"] = f"{header['stream_width']}x{header['stream_height']} @ {float(header['stream_fps']):.2f}fps"
    summary["avg_frame_bytes"] = inspect_result["avg_frame_bytes"]
    summary["target_fps"] = inspect_result["estimated_network_fps_at_target"]
    summary["bandwidth_kbps"] = encode_result["estimated_realtime_bandwidth_kbps"] if encode_result else "-"

    showcase_meta = create_showcase_video(
        original_video=None if suffix == ".lbvc" else input_path,
        decoded_video=Path(decode_result["output"]),
        output_path=paths["showcase"],
        preview_path=paths["preview"],
        summary=summary,
        frame_step=int(header.get("frame_step", 1)),
    )
    web_video = create_browser_video_webm(Path(showcase_meta["showcase_video"]), paths["web_video"])
    web_preview = create_web_preview_animation(Path(showcase_meta["showcase_video"]), paths["web_preview"])
    source_preview = extract_source_preview(input_path, paths["source_preview"])

    result = {
        "source": file_info(input_path),
        "stream": file_info(stream_path),
        "decoded": file_info(Path(decode_result["output"])),
        "showcase": file_info(Path(showcase_meta["showcase_video"])),
        "web_video": file_info(web_video) if web_video is not None else None,
        "preview": file_info(Path(showcase_meta["preview_image"])),
        "web_preview": file_info(web_preview),
        "source_preview": file_info(source_preview) if source_preview is not None else None,
        "summary_data": {
            "source_name": summary["source_name"],
            "source_spec": summary.get("source_spec", "not available"),
            "stream_spec": summary["stream_spec"],
            "avg_frame_bytes": summary["avg_frame_bytes"],
            "target_fps": summary["target_fps"],
            "bandwidth_kbps": summary["bandwidth_kbps"],
            "frames_decoded": decode_result["frames_decoded"],
            "keyframes": inspect_result["keyframes"],
            "delta_frames": inspect_result["delta_frames"],
            "skip_frames": inspect_result["skip_frames"],
            "color_mode": inspect_result["color_mode"],
            "transport_mode": "keyframe + delta + skip",
        },
        "inspect": {k: v for k, v in inspect_result.items() if k != "header"},
    }

    summary_payload = {
        "created_at": time.strftime("%Y-%m-%d %H:%M:%S"),
        "params": params,
        "result": result,
    }
    paths["summary"].write_text(json.dumps(summary_payload, indent=2, ensure_ascii=False), encoding="utf-8")
    result["summary"] = file_info(paths["summary"])
    return result


def list_sample_videos() -> list[dict]:
    samples = []
    for path in sorted(DATA_DIR.glob("*")):
        if path.suffix.lower() in {".avi", ".mp4", ".mov", ".mkv"} and path.is_file():
            samples.append(
                {
                    "name": path.name,
                    "path": str(path.relative_to(PROJECT_ROOT)),
                    "size_mb": round(path.stat().st_size / (1024 * 1024), 2),
                }
            )
    return samples


def load_history() -> list[dict]:
    items = []
    for path in sorted(SUMMARY_DIR.glob("*_summary.json"), key=lambda p: p.stat().st_mtime, reverse=True)[:HISTORY_LIMIT]:
        try:
            payload = json.loads(path.read_text(encoding="utf-8"))
            result = payload["result"]
            items.append(
                {
                    "created_at": payload.get("created_at"),
                    "summary_data": result.get("summary_data", {}),
                    "showcase": result.get("showcase"),
                    "web_video": result.get("web_video"),
                    "web_preview": result.get("web_preview"),
                    "preview": result.get("preview"),
                    "summary": file_info(path),
                }
            )
        except Exception:
            continue
    return items


class AppHandler(BaseHTTPRequestHandler):
    server_version = "VideoWebHost/1.0"

    def log_message(self, fmt: str, *args) -> None:
        print(f"[{time.strftime('%H:%M:%S')}] {self.address_string()} - {fmt % args}")

    def send_json(self, payload: dict, status: int = 200) -> None:
        data = json_bytes(payload)
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def send_file(self, path: Path) -> None:
        if not path.exists() or not path.is_file():
            self.send_error(404, "File not found")
            return
        mime, _ = mimetypes.guess_type(path.name)
        self.send_response(200)
        self.send_header("Content-Type", mime or "application/octet-stream")
        self.send_header("Content-Length", str(path.stat().st_size))
        self.end_headers()
        with path.open("rb") as f:
            shutil.copyfileobj(f, self.wfile)

    def parse_params(self) -> dict[str, object]:
        query = parse_qs(urlparse(self.path).query)
        params: dict[str, object] = {}
        for key in ("target_bitrate_kbps", "scale", "jpeg_quality"):
            if key in query:
                params[key] = query[key][0]
        if "skip_threshold" in query:
            params["skip_threshold"] = query["skip_threshold"][0]
        if "color_mode" in query:
            params["color_mode"] = query["color_mode"][0]
        return params

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/":
            self.send_file(INDEX_PATH)
            return
        if parsed.path == "/api/samples":
            self.send_json({"ok": True, "samples": list_sample_videos(), "history": load_history()})
            return
        if parsed.path == "/api/history":
            self.send_json({"ok": True, "history": load_history()})
            return
        if parsed.path.startswith("/files/"):
            rel = unquote(parsed.path[len("/files/") :])
            target = (OUTPUT_ROOT / rel).resolve()
            try:
                target.relative_to(OUTPUT_ROOT.resolve())
            except ValueError:
                self.send_error(403, "Forbidden")
                return
            self.send_file(target)
            return
        self.send_error(404, "Not found")

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        try:
            if parsed.path == "/api/process-upload":
                self.handle_process_upload()
                return
            if parsed.path == "/api/process-sample":
                self.handle_process_sample()
                return
        except Exception as exc:
            traceback.print_exc()
            self.send_json({"ok": False, "error": str(exc)}, status=500)
            return
        self.send_error(404, "Not found")

    def handle_process_upload(self) -> None:
        content_length = int(self.headers.get("Content-Length", "0"))
        if content_length <= 0:
            raise RuntimeError("上传内容为空")

        params = self.parse_params()
        query = parse_qs(urlparse(self.path).query)
        filename = sanitize_name(query.get("filename", ["upload.bin"])[0])
        suffix = Path(filename).suffix.lower()
        if suffix not in SUPPORTED_SOURCE_SUFFIXES:
            raise RuntimeError(f"不支持的文件类型: {suffix}")

        target_path = UPLOAD_DIR / f"{now_tag()}_{filename}"
        received = 0
        with target_path.open("wb") as f:
            while received < content_length:
                need = min(RAW_UPLOAD_CHUNK, content_length - received)
                chunk = self.rfile.read(need)
                if not chunk:
                    raise RuntimeError("上传中断")
                f.write(chunk)
                received += len(chunk)

        result = process_video_pipeline(target_path, params)
        self.send_json({"ok": True, "result": result})

    def handle_process_sample(self) -> None:
        content_length = int(self.headers.get("Content-Length", "0"))
        payload = self.rfile.read(content_length) if content_length > 0 else b"{}"
        body = json.loads(payload.decode("utf-8"))
        params = body.get("params", {})
        sample_rel = str(body.get("sample_path", "")).strip()
        if not sample_rel:
            raise RuntimeError("sample_path 不能为空")

        sample_path = (PROJECT_ROOT / sample_rel).resolve()
        try:
            sample_path.relative_to(DATA_DIR.resolve())
        except ValueError as exc:
            raise RuntimeError("样例路径不合法") from exc
        if not sample_path.exists():
            raise RuntimeError(f"样例文件不存在: {sample_path}")

        result = process_video_pipeline(sample_path, params)
        self.send_json({"ok": True, "result": result})


def open_browser_later(url: str) -> None:
    timer = threading.Timer(1.0, lambda: webbrowser.open(url))
    timer.daemon = True
    timer.start()


def main() -> None:
    host = "127.0.0.1"
    port = 8097
    server = ThreadingHTTPServer((host, port), AppHandler)
    url = f"http://{host}:{port}"
    print(f"Video web host running at {url}")
    print(f"Output directory: {OUTPUT_ROOT}")
    if os.environ.get("VIDEO_WEB_HOST_NO_BROWSER") != "1":
        open_browser_later(url)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
