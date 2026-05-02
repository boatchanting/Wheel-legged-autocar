import base64
import threading
import time
from collections import deque
from pathlib import Path
from typing import Optional

import cv2
import numpy as np
import webview


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


class DiffHostApi:
    def __init__(self) -> None:
        self.lock = threading.RLock()

        self.running = False
        self.connected = False
        self.host = "0.0.0.0"
        self.port = 8086
        self.fps_hint = 120.0

        self.output_root = Path(__file__).resolve().parent / "output_realtime"
        self.frames_dir = self.output_root / "frames"
        self.videos_dir = self.output_root / "videos"
        self.output_root.mkdir(parents=True, exist_ok=True)
        self.frames_dir.mkdir(parents=True, exist_ok=True)
        self.videos_dir.mkdir(parents=True, exist_ok=True)

        self.server_sock = None
        self.conn_sock = None
        self.server_thread = None
        self.stop_event = threading.Event()
        self.client_addr = ""

        self.frame_gray: Optional[np.ndarray] = None
        self.frame_id = 0
        self.frame_serial = 0
        self.frame_w = 0
        self.frame_h = 0
        self.keyframes = 0
        self.diffframes = 0
        self.skipframes = 0
        self.output_frames = 0
        self.total_bytes = 0
        self.total_packets = 0
        self.raw_bytes = 0
        self.raw_chunks = 0
        self.bad_checksums = 0
        self.bad_versions = 0
        self.frames_before_key = 0
        self.start_ts = 0.0
        self.last_error = ""
        self.last_frame_ts = 0.0
        self.frame_times = deque()
        self.net_times = deque()
        self.net_bytes_window = 0

        self.preview_serial = -1
        self.preview_data_uri = ""

        self.recording = False
        self.recording_pending = False
        self.record_writer = None
        self.record_path = ""
        self.recorded_frames = 0
        self.record_codec = ""

        self.logs: list[str] = []

    def _log(self, msg: str) -> None:
        ts = time.strftime("%H:%M:%S")
        line = f"[{ts}] {msg}"
        with self.lock:
            self.logs.append(line)
            if len(self.logs) > 200:
                self.logs = self.logs[-200:]
        print(line)

    def _reset_stream_stats(self) -> None:
        self.frame_gray = None
        self.frame_id = 0
        self.frame_serial = 0
        self.frame_w = 0
        self.frame_h = 0
        self.keyframes = 0
        self.diffframes = 0
        self.skipframes = 0
        self.output_frames = 0
        self.total_bytes = 0
        self.total_packets = 0
        self.raw_bytes = 0
        self.raw_chunks = 0
        self.bad_checksums = 0
        self.bad_versions = 0
        self.frames_before_key = 0
        self.start_ts = time.time()
        self.preview_serial = -1
        self.preview_data_uri = ""
        self.last_error = ""
        self.last_frame_ts = 0.0
        self.frame_times.clear()
        self.net_times.clear()
        self.net_bytes_window = 0

    def _stop_record_writer(self) -> None:
        if self.record_writer is not None:
            try:
                self.record_writer.release()
            except Exception:
                pass
            self.record_writer = None

    def _open_video_writer_auto(self):
        if self.frame_w <= 0 or self.frame_h <= 0:
            return None, "", ""

        ts = time.strftime("%Y%m%d_%H%M%S")
        base_name = f"realtime_record_{ts}"
        fallback_root = Path.cwd() / "output_realtime_ascii" / "videos"
        fallback_root.mkdir(parents=True, exist_ok=True)
        base_candidates = [
            self.videos_dir / base_name,
            fallback_root / base_name,
        ]
        # Prioritize broadly playable codecs first, then quality-oriented fallback.
        codec_candidates = [
            ("MJPG", ".avi"),
            ("mp4v", ".mp4"),
            ("XVID", ".avi"),
        ]
        for base in base_candidates:
            for codec, ext in codec_candidates:
                out_path = str(base.with_suffix(ext))
                fourcc = cv2.VideoWriter_fourcc(*codec)
                writer = cv2.VideoWriter(out_path, fourcc, self.fps_hint, (self.frame_w, self.frame_h), True)
                if writer.isOpened():
                    try:
                        # Best-effort quality hint for backends that support it.
                        writer.set(cv2.VIDEOWRITER_PROP_QUALITY, 95)
                    except Exception:
                        pass
                    return writer, out_path, codec
                writer.release()
        return None, "", ""

    def _open_record_writer_if_needed(self) -> None:
        if not self.recording:
            return
        if self.record_writer is not None:
            return
        if self.frame_w <= 0 or self.frame_h <= 0:
            return

        writer, out_path, codec = self._open_video_writer_auto()
        if writer is None:
            self.last_error = "cannot open any video writer codec"
            self._log(self.last_error)
            self.recording = False
            self.recording_pending = False
            return

        self.record_writer = writer
        self.record_path = out_path
        self.record_codec = codec
        self.recording_pending = False
        self._log(f"start recording: {self.record_path} (codec={self.record_codec})")

    def _mark_frame_tick(self) -> None:
        now = time.time()
        self.last_frame_ts = now
        self.frame_times.append(now)
        while self.frame_times and (now - self.frame_times[0] > 1.0):
            self.frame_times.popleft()

    def _mark_net_tick(self, chunk_size: int) -> None:
        now = time.time()
        self.net_times.append((now, chunk_size))
        self.net_bytes_window += chunk_size
        while self.net_times and (now - self.net_times[0][0] > 1.0):
            _, old_size = self.net_times.popleft()
            self.net_bytes_window -= old_size

    def _write_record_frame(self, frame_gray: np.ndarray) -> None:
        with self.lock:
            if not self.recording:
                return
            self._open_record_writer_if_needed()
            writer = self.record_writer
        if writer is None:
            return

        try:
            bgr = cv2.cvtColor(frame_gray, cv2.COLOR_GRAY2BGR)
            writer.write(bgr)
            with self.lock:
                self.recorded_frames += 1
        except Exception as exc:
            with self.lock:
                self.last_error = f"record write failed: {exc}"
                self.recording = False
                self.recording_pending = False
                self._stop_record_writer()
            self._log(self.last_error)

    def _handle_packet(self, pkt: bytes) -> None:
        h = pkt[:HEADER_SIZE]
        frame_type = h[3]
        frame_id = le_u32(h, 4)
        stream_w = le_u16(h, 8)
        stream_h = le_u16(h, 10)
        x = le_u16(h, 12)
        y = le_u16(h, 14)
        roi_w = le_u16(h, 16)
        roi_h = le_u16(h, 18)
        payload_len = le_u32(h, 20)
        payload = pkt[HEADER_SIZE: HEADER_SIZE + payload_len]

        frame_for_record = None
        with self.lock:
            self.total_packets += 1
            self.total_bytes += len(pkt)
            self.frame_id = frame_id
            if self.frame_w == 0:
                self.frame_w = stream_w
                self.frame_h = stream_h

            if frame_type == FRAME_KEY:
                expect = self.frame_w * self.frame_h
                if len(payload) != expect:
                    return
                arr = np.frombuffer(payload, dtype=np.uint8)
                self.frame_gray = arr.reshape((self.frame_h, self.frame_w)).copy()
                self.keyframes += 1
                self.output_frames += 1
                self.frame_serial += 1
                self._mark_frame_tick()
            elif frame_type == FRAME_DIFF:
                if self.frame_gray is None:
                    self.frames_before_key += 1
                    return
                expect = roi_w * roi_h
                if len(payload) != expect:
                    return
                if x + roi_w > self.frame_w or y + roi_h > self.frame_h:
                    return
                roi = np.frombuffer(payload, dtype=np.uint8).reshape((roi_h, roi_w))
                self.frame_gray[y:y + roi_h, x:x + roi_w] = roi
                self.diffframes += 1
                self.output_frames += 1
                self.frame_serial += 1
                self._mark_frame_tick()
            elif frame_type == FRAME_SKIP:
                if self.frame_gray is None:
                    self.frames_before_key += 1
                    return
                self.skipframes += 1
                self.output_frames += 1
                self.frame_serial += 1
                self._mark_frame_tick()
            else:
                return

            if self.recording and self.frame_gray is not None:
                frame_for_record = self.frame_gray.copy()

        if frame_for_record is not None:
            self._write_record_frame(frame_for_record)

    def _parse_packets(self, buf: bytearray) -> list[bytes]:
        packets = []
        i = 0
        n = len(buf)
        while i + HEADER_SIZE + 1 <= n:
            if buf[i] != SYNC0 or buf[i + 1] != SYNC1:
                i += 1
                continue
            if buf[i + 2] not in (VERSION, 0):
                with self.lock:
                    self.bad_versions += 1
                i += 1
                continue

            payload_len = le_u32(buf, i + 20)
            if payload_len > 300000:
                i += 1
                continue

            total_len = HEADER_SIZE + payload_len + 1
            if i + total_len > n:
                break

            pkt = bytes(buf[i:i + total_len])
            checksum = sum(pkt[:-1]) & 0xFF
            if checksum == pkt[-1]:
                packets.append(pkt)
                i += total_len
            else:
                with self.lock:
                    self.bad_checksums += 1
                i += 1

        if i > 0:
            del buf[:i]
        return packets

    def _close_sockets(self) -> None:
        if self.conn_sock is not None:
            try:
                self.conn_sock.close()
            except Exception:
                pass
            self.conn_sock = None
        if self.server_sock is not None:
            try:
                self.server_sock.close()
            except Exception:
                pass
            self.server_sock = None

    def _server_loop(self) -> None:
        import socket

        self._log(f"listening on {self.host}:{self.port}")
        buf = bytearray()
        try:
            self.server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_sock.bind((self.host, self.port))
            self.server_sock.listen(1)
            self.server_sock.settimeout(0.5)
        except Exception as exc:
            with self.lock:
                self.last_error = str(exc)
                self.running = False
            self._log(f"server start failed: {exc}")
            self._close_sockets()
            return

        while not self.stop_event.is_set():
            try:
                conn, addr = self.server_sock.accept()
            except socket.timeout:
                continue
            except Exception:
                break

            with self.lock:
                self.connected = True
                self.client_addr = f"{addr[0]}:{addr[1]}"
            self._log(f"client connected: {self.client_addr}")

            self.conn_sock = conn
            self.conn_sock.settimeout(0.5)
            try:
                while not self.stop_event.is_set():
                    try:
                        chunk = self.conn_sock.recv(8192)
                    except socket.timeout:
                        continue
                    if not chunk:
                        break
                    with self.lock:
                        self.raw_bytes += len(chunk)
                        self.raw_chunks += 1
                        self._mark_net_tick(len(chunk))
                    buf.extend(chunk)
                    for pkt in self._parse_packets(buf):
                        self._handle_packet(pkt)
            except Exception as exc:
                with self.lock:
                    self.last_error = str(exc)
                self._log(f"recv error: {exc}")
            finally:
                try:
                    self.conn_sock.close()
                except Exception:
                    pass
                self.conn_sock = None
                with self.lock:
                    self.connected = False
                    self.client_addr = ""
                self._log("client disconnected")

        with self.lock:
            self.running = False
            self.connected = False
            self.client_addr = ""
        self._close_sockets()
        self._log("receiver stopped")

    def start_receive(self, host: str, port: int, fps: float, output_dir: str):
        with self.lock:
            if self.running:
                return {"ok": False, "message": "already running"}
            self.host = (host or "0.0.0.0").strip()
            self.port = int(port)
            self.fps_hint = float(fps) if float(fps) > 0 else 120.0
            if output_dir and output_dir.strip():
                self.output_root = Path(output_dir).expanduser().resolve()
                self.frames_dir = self.output_root / "frames"
                self.videos_dir = self.output_root / "videos"
            self.output_root.mkdir(parents=True, exist_ok=True)
            self.frames_dir.mkdir(parents=True, exist_ok=True)
            self.videos_dir.mkdir(parents=True, exist_ok=True)
            self._reset_stream_stats()
            self.recording = False
            self.recording_pending = False
            self.record_path = ""
            self.record_codec = ""
            self.recorded_frames = 0
            self._stop_record_writer()
            self.stop_event.clear()
            self.running = True

        self.server_thread = threading.Thread(target=self._server_loop, daemon=True)
        self.server_thread.start()
        return {"ok": True, "message": f"started at {self.host}:{self.port}"}

    def stop_receive(self):
        with self.lock:
            if not self.running:
                return {"ok": False, "message": "not running"}
            self.stop_event.set()
            if self.recording:
                self.recording = False
                self.recording_pending = False
                self._stop_record_writer()
        self._close_sockets()
        if self.server_thread is not None:
            self.server_thread.join(timeout=2.0)
        return {"ok": True, "message": "stopped"}

    def start_record_video(self):
        with self.lock:
            if self.recording:
                return {"ok": False, "message": "recording already started"}
            self.recording = True
            self.recording_pending = True
            self.recorded_frames = 0
            self.record_path = ""
            self.record_codec = ""
            self._open_record_writer_if_needed()
            if not self.recording:
                return {"ok": False, "message": self.last_error or "recording start failed"}
            if self.record_writer is None:
                return {"ok": True, "message": "record armed, waiting first frame", "path": ""}
        return {"ok": True, "message": "recording started", "path": self.record_path}

    def stop_record_video(self):
        with self.lock:
            if not self.recording:
                return {"ok": False, "message": "recording is not active"}
            self.recording = False
            self.recording_pending = False
            path = self.record_path
            frames = self.recorded_frames
            codec = self.record_codec
            self._stop_record_writer()
        if not path or frames <= 0:
            self._log("record stopped: no frames written")
            return {"ok": False, "message": "no frames written, please receive frames first", "path": path, "frames": frames}
        self._log(f"record saved: {path} ({frames} frames, codec={codec})")
        return {"ok": True, "message": "recording stopped", "path": path, "frames": frames}

    def save_current_frame(self):
        with self.lock:
            if self.frame_gray is None:
                return {"ok": False, "message": "no frame available"}
            ts = time.strftime("%Y%m%d_%H%M%S")
            ms = int((time.time() % 1.0) * 1000.0)
            out = self.frames_dir / f"frame_{ts}_{ms:03d}_{self.frame_id}.png"
            out.parent.mkdir(parents=True, exist_ok=True)
            frame_copy = self.frame_gray.copy()
        ok, enc = cv2.imencode(".png", frame_copy)
        if not ok:
            return {"ok": False, "message": "save frame failed: encode error"}
        try:
            enc.tofile(str(out))
        except Exception as exc:
            return {"ok": False, "message": f"save frame failed: {exc}"}
        if (not out.exists()) or out.stat().st_size <= 0:
            return {"ok": False, "message": "save frame failed: file not written"}
        self._log(f"frame saved: {out}")
        return {"ok": True, "message": "frame saved", "path": str(out)}

    def clear_logs(self):
        with self.lock:
            self.logs.clear()
        return {"ok": True}

    def get_state(self):
        preview_frame = None
        preview_target_serial = -1

        with self.lock:
            now = time.time()
            _elapsed = max(now - self.start_ts, 1e-6) if self.start_ts > 0 else 0.0
            while self.frame_times and (now - self.frame_times[0] > 1.0):
                self.frame_times.popleft()
            while self.net_times and (now - self.net_times[0][0] > 1.0):
                _, old_size = self.net_times.popleft()
                self.net_bytes_window -= old_size

            if (self.last_frame_ts <= 0.0) or (now - self.last_frame_ts > 1.2):
                fps = 0.0
            else:
                fps = float(len(self.frame_times))
            kbps = (max(self.net_bytes_window, 0) * 8.0) / 1000.0

            if self.frame_gray is not None and self.preview_serial != self.frame_serial:
                preview_frame = self.frame_gray.copy()
                preview_target_serial = self.frame_serial

            state = {
                "running": self.running,
                "connected": self.connected,
                "client_addr": self.client_addr,
                "host": self.host,
                "port": self.port,
                "fps_hint": self.fps_hint,
                "output_root": str(self.output_root),
                "frame_id": self.frame_id,
                "width": self.frame_w,
                "height": self.frame_h,
                "frames": self.output_frames,
                "keyframes": self.keyframes,
                "diffframes": self.diffframes,
                "skipframes": self.skipframes,
                "packets": self.total_packets,
                "bytes": self.total_bytes,
                "raw_bytes": self.raw_bytes,
                "raw_chunks": self.raw_chunks,
                "bad_checksums": self.bad_checksums,
                "bad_versions": self.bad_versions,
                "frames_before_key": self.frames_before_key,
                "fps": round(fps, 2),
                "kbps": round(kbps, 2),
                "recording": self.recording,
                "recording_pending": self.recording_pending,
                "record_path": self.record_path,
                "recorded_frames": self.recorded_frames,
                "last_error": self.last_error,
                "protocol_hint": (
                    "已收到TCP数据但未解析出差分帧，可能下位机仍在发送其它协议"
                    if (self.connected and self.raw_bytes > 2048 and self.total_packets == 0)
                    else ""
                ),
                "preview": self.preview_data_uri,
                "logs": self.logs[-40:],
            }

        if preview_frame is not None:
            ok, enc = cv2.imencode(".png", preview_frame)
            if ok:
                b64 = base64.b64encode(enc.tobytes()).decode("ascii")
                preview_uri = f"data:image/png;base64,{b64}"
                with self.lock:
                    if self.preview_serial < preview_target_serial:
                        self.preview_data_uri = preview_uri
                        self.preview_serial = preview_target_serial
                    state["preview"] = self.preview_data_uri

        return state


def main() -> None:
    root = Path(__file__).resolve().parent
    html_path = root / "实时图传上位机_webview.html"
    api = DiffHostApi()
    html = html_path.read_text(encoding="utf-8")
    webview.create_window(
        "实时图传上位机",
        html=html,
        js_api=api,
        width=1320,
        height=860,
        min_size=(1000, 700),
    )
    webview.start(debug=False)


if __name__ == "__main__":
    main()
