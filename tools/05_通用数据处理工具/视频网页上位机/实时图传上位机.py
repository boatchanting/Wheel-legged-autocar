import argparse
import socket
import time
from pathlib import Path

import cv2
import numpy as np


SYNC0 = 0xA5
SYNC1 = 0x5A
VERSION = 1
HEADER_SIZE = 24
FRAME_KEY = 1
FRAME_DIFF = 0
FRAME_SKIP = 2


def le_u16(buf: bytes, off: int) -> int:
    return buf[off] | (buf[off + 1] << 8)


def le_u32(buf: bytes, off: int) -> int:
    return buf[off] | (buf[off + 1] << 8) | (buf[off + 2] << 16) | (buf[off + 3] << 24)


class DiffReceiver:
    def __init__(self, out_dir: Path, fps_hint: float) -> None:
        self.out_dir = out_dir
        self.out_dir.mkdir(parents=True, exist_ok=True)
        ts = time.strftime("%Y%m%d_%H%M%S")
        self.out_mp4 = self.out_dir / f"realtime_diff_{ts}.mp4"
        self.writer = None
        self.frame = None
        self.width = 0
        self.height = 0
        self.frames = 0
        self.keyframes = 0
        self.diff_frames = 0
        self.skip_frames = 0
        self.bytes_total = 0
        self.t0 = time.time()
        self.last_print = self.t0
        self.fps_hint = fps_hint

    def _open_writer(self) -> None:
        if self.writer is not None:
            return
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        self.writer = cv2.VideoWriter(str(self.out_mp4), fourcc, self.fps_hint, (self.width, self.height), True)
        if not self.writer.isOpened():
            raise RuntimeError(f"failed to open writer: {self.out_mp4}")

    def _write_frame(self) -> None:
        self._open_writer()
        bgr = cv2.cvtColor(self.frame, cv2.COLOR_GRAY2BGR)
        self.writer.write(bgr)
        self.frames += 1

    def handle_packet(self, packet: bytes) -> None:
        self.bytes_total += len(packet)
        h = packet[:HEADER_SIZE]
        frame_type = h[3]
        w = le_u16(h, 8)
        hgt = le_u16(h, 10)
        x = le_u16(h, 12)
        y = le_u16(h, 14)
        roi_w = le_u16(h, 16)
        roi_h = le_u16(h, 18)
        payload_len = le_u32(h, 20)
        payload = packet[HEADER_SIZE: HEADER_SIZE + payload_len]

        if self.width == 0:
            self.width = w
            self.height = hgt

        if frame_type == FRAME_KEY:
            arr = np.frombuffer(payload, dtype=np.uint8)
            self.frame = arr.reshape((self.height, self.width)).copy()
            self.keyframes += 1
            self._write_frame()
        elif frame_type == FRAME_DIFF:
            if self.frame is None:
                return
            arr = np.frombuffer(payload, dtype=np.uint8)
            roi = arr.reshape((roi_h, roi_w))
            self.frame[y:y + roi_h, x:x + roi_w] = roi
            self.diff_frames += 1
            self._write_frame()
        elif frame_type == FRAME_SKIP:
            if self.frame is None:
                return
            self.skip_frames += 1
            self._write_frame()

        now = time.time()
        if now - self.last_print >= 1.0:
            dt = max(now - self.t0, 1e-6)
            fps = self.frames / dt
            kbps = (self.bytes_total * 8.0) / dt / 1000.0
            print(
                f"frames={self.frames} key={self.keyframes} diff={self.diff_frames} skip={self.skip_frames} "
                f"throughput={kbps:.1f}kbps fps={fps:.2f}"
            )
            self.last_print = now

    def close(self) -> None:
        if self.writer is not None:
            self.writer.release()
            self.writer = None
        dt = max(time.time() - self.t0, 1e-6)
        fps = self.frames / dt
        kbps = (self.bytes_total * 8.0) / dt / 1000.0
        print(f"saved: {self.out_mp4}")
        print(f"summary: frames={self.frames} key={self.keyframes} diff={self.diff_frames} skip={self.skip_frames}")
        print(f"avg: {kbps:.2f} kbps, {fps:.2f} fps")


def parse_packets(stream_buf: bytearray):
    packets = []
    i = 0
    n = len(stream_buf)
    while i + HEADER_SIZE + 1 <= n:
        if stream_buf[i] != SYNC0 or stream_buf[i + 1] != SYNC1:
            i += 1
            continue
        if stream_buf[i + 2] != VERSION:
            i += 1
            continue
        payload_len = le_u32(stream_buf, i + 20)
        total_len = HEADER_SIZE + payload_len + 1
        if i + total_len > n:
            break
        pkt = bytes(stream_buf[i:i + total_len])
        checksum = sum(pkt[:-1]) & 0xFF
        if checksum == pkt[-1]:
            packets.append(pkt)
            i += total_len
        else:
            i += 1
    if i > 0:
        del stream_buf[:i]
    return packets


def run_server(host: str, port: int, out_dir: Path, fps_hint: float) -> None:
    receiver = DiffReceiver(out_dir, fps_hint)
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((host, port))
    server.listen(1)
    print(f"listening on {host}:{port}")
    print(f"output: {out_dir}")
    conn, addr = server.accept()
    print(f"connected: {addr[0]}:{addr[1]}")
    buf = bytearray()
    try:
        while True:
            chunk = conn.recv(8192)
            if not chunk:
                break
            buf.extend(chunk)
            for pkt in parse_packets(buf):
                receiver.handle_packet(pkt)
    finally:
        conn.close()
        server.close()
        receiver.close()


def main() -> None:
    parser = argparse.ArgumentParser(description="Diff frame realtime host receiver")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8086)
    parser.add_argument("--fps", type=float, default=50.0)
    parser.add_argument(
        "--output",
        default=str(Path(__file__).resolve().parent / "output_realtime"),
    )
    args = parser.parse_args()
    run_server(args.host, args.port, Path(args.output), args.fps)


if __name__ == "__main__":
    main()

