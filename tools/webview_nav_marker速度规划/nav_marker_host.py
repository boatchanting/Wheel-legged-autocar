import csv
import math
import os
import socket
import struct
import threading
import time

import webview

HOST_IP = "192.168.137.1"
HOST_PORT = 8086

# 打滑检测标记绘制开关：1=启用，0=禁用
ENABLE_SLIP_MARKERS = 0

FRAME_HEAD1 = 0x5A
FRAME_HEAD2 = 0xA5
FRAME_TAIL = 0xED
FRAME_MIN_SIZE = 6

CMD_TELEMETRY = 0x01
CMD_HOST_CONTROL = 0x10
CMD_HOST_ACK = 0x11
HOST_CTRL_CLEAR_TRAJECTORY = 0x01
HOST_CTRL_START_CAR = 0x02
HOST_ACK_ACCEPTED = 0x00
HOST_ACK_REJECTED = 0x01
HOST_ACK_UNKNOWN_CMD = 0x02
HOST_ACK_INVALID_PAYLOAD = 0x03
HOST_ACK_TIMEOUT_SEC = 1.5

PAYLOAD_SIZE_V1 = 100  # 84(original) + 16(fused nav: 4 floats)
PAYLOAD_SIZE_V2 = 102  # V1 + mark_trigger(1) + point_type(1)

STRUCT_FMT_V1 = "<IffffffffHBBBBBBHHHHHHddbbffBfBfff"

FIELD_NAMES_V1 = [
    "loop",
    "nav_x",
    "nav_y",
    "vx_body",
    "vy_body",
    "fused_x",
    "fused_y",
    "fused_vx",
    "gps_weight",
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

# 新增：全量日志 CSV 的列名，包含基础字段、扩展字段及解析辅助字段
FULL_LOG_FIELDS = FIELD_NAMES_V2 + [
    "pid_mode",
    "slip_flag",
    "target_speed",
    "speed_L",
    "speed_R",
    "theoretical_yaw_rate",
    "actual_yaw_rate",
    "payload_size",
    "time_str",
]

MAX_HISTORY = 20000
MAX_NEW_BUFFER = 4000
START_POINT_CAPTURE_RADIUS_MM = 120.0

state_lock = threading.Lock()
tx_lock = threading.Lock()
all_history_data = []
new_data_buffer = []

last_rx_time = 0.0
last_frame_time = 0.0
last_payload_size = 0
server_error = ""
peer_addr = ""
listen_ip = HOST_IP
active_conn = None
ack_cond = threading.Condition()
ack_seq = 0
ack_events = []
ACK_EVENT_MAX = 64


def _build_frame(cmd, payload_bytes=b""):
    payload_len = len(payload_bytes)
    if payload_len > 255:
        raise ValueError("payload too long")

    frame = bytearray([FRAME_HEAD1, FRAME_HEAD2, cmd & 0xFF, payload_len & 0xFF])
    frame.extend(payload_bytes)
    check_sum = sum(frame) & 0xFF
    frame.append(check_sum)
    frame.append(FRAME_TAIL)
    return bytes(frame)


def _push_host_ack(control_code, status_code):
    global ack_seq
    with ack_cond:
        ack_seq += 1
        ack_events.append((ack_seq, int(control_code) & 0xFF, int(status_code) & 0xFF, time.time()))
        if len(ack_events) > ACK_EVENT_MAX:
            del ack_events[: len(ack_events) - ACK_EVENT_MAX]
        ack_cond.notify_all()


def _wait_host_ack(control_code, start_seq, timeout_sec):
    deadline = time.time() + max(0.1, float(timeout_sec))
    with ack_cond:
        while True:
            for seq_no, ack_ctrl, ack_status, _ in ack_events:
                if seq_no > start_seq and ack_ctrl == control_code:
                    return ack_status

            remain = deadline - time.time()
            if remain <= 0:
                return None
            ack_cond.wait(remain)


def _format_host_ack_result(control_code, ack_status):
    if ack_status == HOST_ACK_ACCEPTED:
        return {"success": True, "msg": "小车回传成功：命令已执行"}

    if ack_status == HOST_ACK_REJECTED:
        if control_code == HOST_CTRL_CLEAR_TRAJECTORY:
            return {
                "success": True,
                "executed": False,
                "msg": "小车已回传：清除轨迹被拒绝（需电机使能且航向已初始化）",
            }
        if control_code == HOST_CTRL_START_CAR:
            return {
                "success": True,
                "executed": False,
                "msg": "小车已回传：开始发车被拒绝（需电机使能）",
            }
        return {"success": True, "executed": False, "msg": "小车已回传：命令被拒绝（条件不满足）"}

    if ack_status == HOST_ACK_UNKNOWN_CMD:
        return {"success": False, "msg": "小车回传失败：未知命令"}

    if ack_status == HOST_ACK_INVALID_PAYLOAD:
        return {"success": False, "msg": "小车回传失败：命令载荷格式错误"}

    return {"success": False, "msg": f"小车回传失败：未知ACK状态 0x{ack_status:02X}"}


def _send_control_to_vehicle(ctrl_code):
    global active_conn, peer_addr, server_error

    code = int(ctrl_code) & 0xFF
    frame = _build_frame(CMD_HOST_CONTROL, bytes([code]))

    with ack_cond:
        start_seq = ack_seq

    with tx_lock:
        with state_lock:
            conn = active_conn
            peer = peer_addr

        if conn is None:
            return {"success": False, "msg": "小车未连接，命令未发送"}

        try:
            conn.sendall(frame)
        except Exception as exc:
            with state_lock:
                if active_conn is conn:
                    active_conn = None
                peer_addr = ""
                server_error = f"Send control failed: {exc}"
            return {"success": False, "msg": f"命令发送失败: {exc}"}

    ack_status = _wait_host_ack(code, start_seq, HOST_ACK_TIMEOUT_SEC)
    if ack_status is None:
        return {
            "success": False,
            "msg": f"命令已发送到小车({peer})，但未收到回传（请确认小车已烧录最新协议）",
        }

    return _format_host_ack_result(code, ack_status)

def _safe_float(value):
    try:
        num = float(value)
    except (TypeError, ValueError):
        return None
    if not math.isfinite(num):
        return None
    return num


def _normalize_heading_deg(value):
    num = _safe_float(value)
    if num is None:
        return None
    num = math.fmod(num, 360.0)
    if num < 0.0:
        num += 360.0
    return num


def _normalize_relative_yaw_deg(value):
    num = _safe_float(value)
    if num is None:
        return None
    while num > 180.0:
        num -= 360.0
    while num <= -180.0:
        num += 360.0
    return num


def _estimate_start_heading():
    with state_lock:
        history = list(all_history_data)

    if not history:
        return None

    # IMU963RA下 heading 是绝对航向；若数据流里始终为0，回退到 relative_yaw。
    has_heading_signal = False
    for item in history:
        h = _normalize_heading_deg(item.get("heading"))
        if h is not None and abs(h) > 1e-3:
            has_heading_signal = True
            break

    radius2 = START_POINT_CAPTURE_RADIUS_MM * START_POINT_CAPTURE_RADIUS_MM
    best_dist2 = float("inf")
    best_heading = None

    for item in history:
        x = _safe_float(item.get("nav_x"))
        y = _safe_float(item.get("nav_y"))
        if x is None or y is None:
            continue

        if has_heading_signal:
            heading_deg = _normalize_heading_deg(item.get("heading"))
        else:
            heading_deg = _normalize_heading_deg(item.get("relative_yaw"))

        if heading_deg is None:
            continue

        dist2 = x * x + y * y
        if dist2 <= radius2:
            return heading_deg

        if dist2 < best_dist2:
            best_dist2 = dist2
            best_heading = heading_deg

    return best_heading


def _decode_payload(payload_bytes):
    size = len(payload_bytes)

    if size < PAYLOAD_SIZE_V1:
        return None

    # 兼容扩展 payload：基础字段始终按 V1 解析，后续字段按可用字节补齐。
    unpacked = struct.unpack(STRUCT_FMT_V1, payload_bytes[:PAYLOAD_SIZE_V1])
    data = dict(zip(FIELD_NAMES_V1, unpacked))

    if size >= PAYLOAD_SIZE_V2:
        data["mark_trigger"] = payload_bytes[PAYLOAD_SIZE_V1]
        data["point_type"] = payload_bytes[PAYLOAD_SIZE_V1 + 1]
    else:
        data["mark_trigger"] = 0
        data["point_type"] = 0

    baseline = 112 if size >= 112 else 102

    if size >= baseline + 1:
        data["pid_mode"] = payload_bytes[baseline]
    else:
        data["pid_mode"] = 0

    if size >= baseline + 2:
        data["slip_flag"] = payload_bytes[baseline + 1]
    else:
        data["slip_flag"] = 0

    if size >= baseline + 22:
        debug_floats = struct.unpack('<fffff', payload_bytes[baseline + 2 : baseline + 22])
        data["target_speed"] = debug_floats[0]
        data["speed_L"] = debug_floats[1]
        data["speed_R"] = debug_floats[2]
        data["theoretical_yaw_rate"] = debug_floats[3]
        data["actual_yaw_rate"] = debug_floats[4]
        data["has_debug"] = True
    else:
        data["has_debug"] = False

    data["payload_size"] = size
    data["time_str"] = f"{data['hour']:02d}:{data['minute']:02d}:{data['second']:02d}"
    return data


debug_log_file = None

def _handle_debug_logging(data):
    global debug_log_file
    if data.get("has_debug"):
        if debug_log_file is None:
            filename = time.strftime("slip_debug_log_%Y%m%d_%H%M%S.txt")
            try:
                debug_log_file = open(filename, "w", encoding="utf-8")
                debug_log_file.write("Time,TargetSpeed,SpeedL,SpeedR,TheoYawRate,ActualYawRate,SlipFlag\n")
                print(f"[DEBUG LOG] 开始写入调试日志: {filename}")
            except Exception as e:
                print(f"[DEBUG LOG] 打开日志失败: {e}")
                return
        
        try:
            line = f"{data['time_str']},{data['target_speed']:.2f},{data['speed_L']:.2f},{data['speed_R']:.2f},{data['theoretical_yaw_rate']:.4f},{data['actual_yaw_rate']:.4f},{data['slip_flag']}\n"
            debug_log_file.write(line)
            debug_log_file.flush()
        except Exception as e:
            print(f"[DEBUG LOG] 写入日志失败: {e}")
    else:
        if debug_log_file is not None:
            try:
                debug_log_file.close()
            except:
                pass
            debug_log_file = None
            print("[DEBUG LOG] 退出复刻，结束日志写入。")


# ============= 新增：全量数据日志相关 =============
full_log_file = None
full_log_writer = None

def _open_full_log():
    """打开全量日志 CSV 文件并写入表头"""
    global full_log_file, full_log_writer
    if full_log_file is not None:
        return
    
    filename = time.strftime("full_data_log_%Y%m%d_%H%M%S.csv")
    try:
        # 使用 utf-8-sig 编码，防止 Excel 打开中文乱码
        full_log_file = open(filename, "w", newline="", encoding="utf-8-sig")
        full_log_writer = csv.writer(full_log_file)
        full_log_writer.writerow(FULL_LOG_FIELDS)
        full_log_file.flush()
        print(f"[FULL LOG] 开始写入全量数据日志: {filename}")
    except Exception as e:
        print(f"[FULL LOG] 创建全量日志失败: {e}")
        full_log_file = None
        full_log_writer = None

def _close_full_log():
    """关闭全量日志 CSV 文件"""
    global full_log_file, full_log_writer
    if full_log_file is not None:
        try:
            full_log_file.close()
            print("[FULL LOG] 全量数据日志已关闭")
        except:
            pass
        finally:
            full_log_file = None
            full_log_writer = None

def _write_full_log(data):
    """将一帧全量数据写入 CSV"""
    global full_log_writer
    if full_log_writer is None:
        return

    try:
        # 按 FULL_LOG_FIELDS 定义的顺序提取数据，如果字段不存在则留空
        row = [data.get(field, "") for field in FULL_LOG_FIELDS]
        full_log_writer.writerow(row)
        full_log_file.flush()
    except Exception as e:
        print(f"[FULL LOG] 写入全量日志失败: {e}")
# ==================================================


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

    # 在数据入缓冲后，写入全量日志
    _write_full_log(data)


def _parse_frame_stream(raw_buffer):
    global last_frame_time, last_payload_size

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

        cmd = raw_buffer[2]
        payload = bytes(raw_buffer[4 : 4 + payload_len])

        if cmd == CMD_HOST_ACK:
            if payload_len >= 2:
                _push_host_ack(payload[0], payload[1])
            del raw_buffer[:frame_len]
            continue

        if cmd != CMD_TELEMETRY:
            del raw_buffer[:frame_len]
            continue

        with state_lock:
            last_frame_time = time.time()
            last_payload_size = payload_len

        data = _decode_payload(payload)
        if data is not None:
            _handle_debug_logging(data)
            _push_data(data)

        del raw_buffer[:frame_len]


def _bind_server_socket(server_socket):
    global listen_ip

    bind_targets = [HOST_IP]
    if HOST_IP != "0.0.0.0":
        bind_targets.append("0.0.0.0")

    last_exc = None
    for ip in bind_targets:
        try:
            server_socket.bind((ip, HOST_PORT))
            listen_ip = ip
            return
        except Exception as exc:
            last_exc = exc

    if last_exc is not None:
        raise last_exc
    raise RuntimeError("Bind failed: unknown error")


def tcp_server_thread():
    global server_error, peer_addr, active_conn

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        _bind_server_socket(server_socket)
        server_socket.listen(1)
        server_socket.settimeout(1.0)
        with state_lock:
            server_error = ""
        print(f"[TCP] Listening on {listen_ip}:{HOST_PORT}")
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
                active_conn = conn
                server_error = ""
            print(f"[TCP] Connected: {peer}")

            # 新增：设备连接时打开全量日志
            _open_full_log()

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
                if active_conn is conn:
                    active_conn = None
            
            # 新增：设备断开时关闭全量日志
            _close_full_log()
            
            print(f"[TCP] Disconnected: {peer}")
        except socket.timeout:
            continue
        except Exception as exc:
            with state_lock:
                peer_addr = ""
                active_conn = None
                server_error = f"Server error: {exc}"
            
            # 异常时也要尝试关闭日志
            _close_full_log()
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
            connected = bool(peer_addr)
            data_fresh = (now - last_rx_time) < 1.2
            frame_fresh = (now - last_frame_time) < 1.2
            return {
                "connected": connected,
                "data_fresh": data_fresh,
                "frame_fresh": frame_fresh,
                "peer": peer_addr,
                "last_payload_size": last_payload_size,
                "history_count": len(all_history_data),
                "server_error": server_error,
                "host_ip": listen_ip,
                "host_port": HOST_PORT,
                "enable_slip_markers": ENABLE_SLIP_MARKERS,
            }

    def clear_history(self):
        with state_lock:
            all_history_data.clear()
            new_data_buffer.clear()
        return {"success": True, "msg": "历史数据已清空"}

    def send_host_control(self, control_code):
        try:
            code = int(control_code)
        except Exception:
            return {"success": False, "msg": "control_code 非法"}

        if not (0 <= code <= 255):
            return {"success": False, "msg": "control_code 超出范围"}

        return _send_control_to_vehicle(code)

    def export_mark_points_csv(self, points):
        try:
            if not isinstance(points, list) or not points:
                return {"success": False, "msg": "没有可导出的标记点"}

            count = len(points)
            start_heading = _estimate_start_heading()
            start_heading_str = "" if start_heading is None else f"{start_heading:.3f}"
            filename = f"nav_mark_points_{time.strftime('%Y%m%d_%H%M%S')}.csv"
            filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)

            with open(filepath, "w", newline="", encoding="utf-8-sig") as f:
                writer = csv.writer(f)
                writer.writerow([
                    "total_count",
                    "start_heading",
                    "index",
                    "x",
                    "y",
                    "relative_yaw",
                    "heading",
                    "point_type",
                ])
                for i, item in enumerate(points):
                    idx = int(item.get("index", i))
                    x = float(item.get("x", 0.0))
                    y = float(item.get("y", 0.0))
                    relative_yaw = _normalize_relative_yaw_deg(item.get("relative_yaw", 0.0))
                    heading_deg = _normalize_heading_deg(item.get("heading", 0.0))
                    point_type = int(item.get("point_type", 0))
                    if relative_yaw is None:
                        relative_yaw = 0.0
                    if heading_deg is None:
                        heading_deg = 0.0
                    writer.writerow([
                        count,
                        start_heading_str,
                        idx,
                        f"{x:.3f}",
                        f"{y:.3f}",
                        f"{relative_yaw:.3f}",
                        f"{heading_deg:.3f}",
                        point_type,
                    ])

            heading_msg = "NA" if start_heading is None else f"{start_heading:.3f}"
            return {
                "success": True,
                "msg": f"导出成功: {filepath} (start_heading={heading_msg})",
                "path": filepath,
                "start_heading": start_heading,
            }
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
