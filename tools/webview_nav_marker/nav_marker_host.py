import csv
import os
import socket
import struct
import threading
import time

import webview

HOST_IP = "192.168.137.1"
HOST_PORT = 8086

FRAME_HEAD1 = 0x5A
FRAME_HEAD2 = 0xA5
FRAME_TAIL = 0xED
FRAME_MIN_SIZE = 6

PAYLOAD_SIZE_V1 = 84
PAYLOAD_SIZE_V2 = 86

STRUCT_FMT_V1 = "<IffffHBBBBBBHHHHHHddbbffBfBfff"
STRUCT_FMT_V2 = "<IffffHBBBBBBHHHHHHddbbffBfBfffBB"

FIELD_NAMES_V1 = [
    "loop",
    "nav_x",
    "nav_y",
    "vx_body",
    "vy_body",
    "year",
    "month",
    "day",
    "hour",
    "minute",
    "second",
    "state",
    "lat_deg",
    "lat_cent",
    "lat_sec",
    "lon_deg",
    "lon_cent",
    "lon_sec",
    "latitude",
    "longitude",
    "ns",
    "ew",
    "speed",
    "direction",
    "ant_state",
    "ant_direction",
    "sat_used",
    "height",
    "heading",
    "relative_yaw",
]

FIELD_NAMES_V2 = FIELD_NAMES_V1 + ["mark_trigger", "point_type"]

MAX_HISTORY = 20000
MAX_NEW_BUFFER = 4000

state_lock = threading.Lock()
all_history_data = []
new_data_buffer = []

last_rx_time = 0.0
last_payload_size = 0
server_error = ""
peer_addr = ""


def _decode_payload(payload_bytes):
    size = len(payload_bytes)

    if size == PAYLOAD_SIZE_V2:
        unpacked = struct.unpack(STRUCT_FMT_V2, payload_bytes)
        data = dict(zip(FIELD_NAMES_V2, unpacked))
    elif size == PAYLOAD_SIZE_V1:
        unpacked = struct.unpack(STRUCT_FMT_V1, payload_bytes)
        data = dict(zip(FIELD_NAMES_V1, unpacked))
        data["mark_trigger"] = 0
        data["point_type"] = 0
    else:
        return None

    data["payload_size"] = size
    data["time_str"] = f"{data['hour']:02d}:{data['minute']:02d}:{data['second']:02d}"
    return data


def _push_data(data):
    global last_rx_time, last_payload_size

    with state_lock:
        all_history_data.append(data)
        if len(all_history_data) > MAX_HISTORY:
            del all_history_data[: len(all_history_data) - MAX_HISTORY]

        new_data_buffer.append(data)
        if len(new_data_buffer) > MAX_NEW_BUFFER:
            del new_data_buffer[: len(new_data_buffer) - MAX_NEW_BUFFER]

        last_rx_time = time.time()
        last_payload_size = data.get("payload_size", 0)


def _parse_frame_stream(raw_buffer):
    while len(raw_buffer) >= FRAME_MIN_SIZE:
        if raw_buffer[0] != FRAME_HEAD1 or raw_buffer[1] != FRAME_HEAD2:
            del raw_buffer[0]
            continue

        payload_len = raw_buffer[3]
        frame_len = payload_len + FRAME_MIN_SIZE

        if payload_len <= 0 or frame_len > 1024:
            del raw_buffer[0]
            continue

        if len(raw_buffer) < frame_len:
            break

        if raw_buffer[frame_len - 1] != FRAME_TAIL:
            del raw_buffer[0]
            continue

        calc_sum = sum(raw_buffer[: frame_len - 2]) & 0xFF
        recv_sum = raw_buffer[frame_len - 2]
        if calc_sum != recv_sum:
            del raw_buffer[0]
            continue

        payload = bytes(raw_buffer[4 : 4 + payload_len])
        data = _decode_payload(payload)
        if data is not None:
            _push_data(data)

        del raw_buffer[:frame_len]


def tcp_server_thread():
    global server_error, peer_addr

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server_socket.bind((HOST_IP, HOST_PORT))
        server_socket.listen(1)
        server_socket.settimeout(1.0)
        print(f"[TCP] Listening on {HOST_IP}:{HOST_PORT}")
    except Exception as exc:
        with state_lock:
            server_error = f"Bind failed: {exc}"
        print(f"[TCP] {server_error}")
        return

    while True:
        try:
            conn, addr = server_socket.accept()
            peer = f"{addr[0]}:{addr[1]}"
            with state_lock:
                peer_addr = peer
            print(f"[TCP] Connected: {peer}")

            raw_buffer = bytearray()
            with conn:
                conn.settimeout(1.0)
                while True:
                    try:
                        chunk = conn.recv(2048)
                        if not chunk:
                            break
                        raw_buffer.extend(chunk)
                        _parse_frame_stream(raw_buffer)
                    except socket.timeout:
                        continue

            with state_lock:
                peer_addr = ""
            print(f"[TCP] Disconnected: {peer}")
        except socket.timeout:
            continue
        except Exception as exc:
            with state_lock:
                server_error = f"Server error: {exc}"
            time.sleep(0.5)


class Api:
    def get_new_data(self):
        with state_lock:
            if not new_data_buffer:
                return []
            data = list(new_data_buffer)
            new_data_buffer.clear()
            return data

    def get_status(self):
        with state_lock:
            now = time.time()
            connected = (now - last_rx_time) < 1.2
            return {
                "connected": connected,
                "peer": peer_addr,
                "last_payload_size": last_payload_size,
                "history_count": len(all_history_data),
                "server_error": server_error,
                "host_ip": HOST_IP,
                "host_port": HOST_PORT,
            }

    def clear_history(self):
        with state_lock:
            all_history_data.clear()
            new_data_buffer.clear()
        return {"success": True, "msg": "历史数据已清空"}

    def export_mark_points_csv(self, points):
        try:
            if not isinstance(points, list) or not points:
                return {"success": False, "msg": "没有可导出的标记点"}

            count = len(points)
            filename = f"nav_mark_points_{time.strftime('%Y%m%d_%H%M%S')}.csv"
            filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)

            with open(filepath, "w", newline="", encoding="utf-8-sig") as f:
                writer = csv.writer(f)
                writer.writerow(["total_count", "index", "x", "y", "point_type"])
                for i, item in enumerate(points):
                    idx = int(item.get("index", i))
                    x = float(item.get("x", 0.0))
                    y = float(item.get("y", 0.0))
                    point_type = int(item.get("point_type", 0))
                    writer.writerow([count, idx, f"{x:.3f}", f"{y:.3f}", point_type])

            return {"success": True, "msg": f"导出成功: {filepath}", "path": filepath}
        except Exception as exc:
            return {"success": False, "msg": f"导出失败: {exc}"}


if __name__ == "__main__":
    thread = threading.Thread(target=tcp_server_thread, daemon=True)
    thread.start()

    current_dir = os.path.dirname(os.path.abspath(__file__))
    html_path = os.path.join(current_dir, "nav_marker.html")

    if not os.path.exists(html_path):
        raise FileNotFoundError(f"HTML not found: {html_path}")

    api = Api()
    window = webview.create_window(
        title="惯导打点上位机 (WebView)",
        url=html_path,
        js_api=api,
        width=1480,
        height=920,
        min_size=(1120, 700),
    )
    webview.start(debug=False)
