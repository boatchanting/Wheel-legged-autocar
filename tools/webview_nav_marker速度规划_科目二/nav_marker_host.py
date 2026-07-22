import csv
import math
import os
import socket
import struct
import threading
import time

import webview

import atexit

HOST_IP = "192.168.137.1"
HOST_PORT = 8086

# 打滑检测标记绘制开关：1=启用，0=禁用
ENABLE_SLIP_MARKERS = 1

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

PAYLOAD_SIZE_V1 = 84
PAYLOAD_SIZE_V2 = 86
PAYLOAD_CTRL_BYTES = 3
PAYLOAD_DEBUG_BYTES = 20
PAYLOAD_NAV_DIAG_BYTES = 100
PAYLOAD_SIZE_CTRL = PAYLOAD_SIZE_V2 + PAYLOAD_CTRL_BYTES
PAYLOAD_SIZE_CTRL_DEBUG = PAYLOAD_SIZE_CTRL + PAYLOAD_DEBUG_BYTES
PAYLOAD_SIZE_CTRL_NAV_DIAG = PAYLOAD_SIZE_CTRL_DEBUG + PAYLOAD_NAV_DIAG_BYTES

STRUCT_FMT_V1 = "<IffffHBBBBBBHHHHHHddbbffBfBfff"

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

WIFI_TELEMETRY_LOG_FIELDS = [
    "recorded_at",
    "frame_index",
    "frame_cmd",
    "raw_frame_hex",
] + FIELD_NAMES_V2 + [
    "pid_mode",
    "slip_flag",
    "minefield_is_active",
    "target_speed",
    "speed_L",
    "speed_R",
    "theoretical_yaw_rate",
    "actual_yaw_rate",
    "nav_replay_state",
    "nav_special_action_trigger",
    "nav_current_point_type",
    "nav_special_target_idx",
    "nav_special_target_x",
    "nav_special_target_y",
    "nav_special_dist_mm",
    "nav_special_brake_radius_mm",
    "nav_special_speed_ref_mm_s",
    "nav_special_zero_brake_issued",
    "nav_special_zero_brake_active",
    "nav_special_crawl_active",
    "nav_special_prep_zero_latched",
    "brake_ff_pwm",
    "accel_ff_pwm",
    "motor_enable",
    "fallen",
    "remote_brake_active",
    "remote_reverse_brake_active",
    "minefield_accumulated_angle",
    "minefield_angle_cmd",
    "minefield_feedforward_speed",
    "minefield_current_speed_cmd",
    "minefield_stall_elapsed_s",
    "minefield_spin_abort_reason",
    "has_debug",
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


def _normalize_wifi_log_value(value):
    if value is None:
        return ""
    if isinstance(value, bool):
        return int(value)
    return value


def _build_wifi_log_filename():
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    return f"wifi_telemetry_{timestamp}_{time.time_ns()}.csv"


class WifiTelemetryCsvRecorder:
    def __init__(self, output_dir=None):
        self.output_dir = output_dir or os.path.join(os.path.dirname(os.path.abspath(__file__)), "logs")
        self._lock = threading.Lock()
        self._file = None
        self._writer = None
        self._path = ""
        self._row_count = 0
        self._started_at = None

    def _is_recording_locked(self):
        return self._file is not None and self._writer is not None

    def start(self):
        with self._lock:
            if self._is_recording_locked():
                return {
                    "success": True,
                    "recording": True,
                    "path": self._path,
                    "rows": self._row_count,
                    "msg": f"日志已在记录中: {self._path}",
                }

            os.makedirs(self.output_dir, exist_ok=True)
            filename = _build_wifi_log_filename()
            path = os.path.join(self.output_dir, filename)

            try:
                file_obj = open(path, "w", newline="", encoding="utf-8-sig")
            except Exception as exc:
                return {"success": False, "recording": False, "path": "", "rows": 0, "msg": f"打开日志失败: {exc}"}

            self._file = file_obj
            self._writer = csv.DictWriter(file_obj, fieldnames=WIFI_TELEMETRY_LOG_FIELDS)
            self._writer.writeheader()
            self._path = path
            self._row_count = 0
            self._started_at = time.time()
            return {
                "success": True,
                "recording": True,
                "path": path,
                "rows": 0,
                "msg": f"已开始记录日志: {path}",
            }

    def record_telemetry_frame(self, data, raw_frame_bytes=None, frame_cmd=CMD_TELEMETRY):
        with self._lock:
            if not self._is_recording_locked():
                return False

            row = {field: "" for field in WIFI_TELEMETRY_LOG_FIELDS}
            row["recorded_at"] = time.strftime("%Y-%m-%d %H:%M:%S")
            row["frame_index"] = self._row_count + 1
            row["frame_cmd"] = f"0x{int(frame_cmd) & 0xFF:02X}"
            row["raw_frame_hex"] = raw_frame_bytes.hex() if raw_frame_bytes is not None else ""
            for field, value in data.items():
                if field in row:
                    row[field] = _normalize_wifi_log_value(value)

            self._writer.writerow(row)
            self._file.flush()
            self._row_count += 1
            return True

    def stop(self):
        with self._lock:
            if not self._is_recording_locked():
                return {"success": False, "recording": False, "path": "", "rows": 0, "msg": "当前没有正在记录的日志"}

            path = self._path
            rows = self._row_count
            try:
                self._file.flush()
            except Exception:
                pass
            try:
                self._file.close()
            except Exception:
                pass

            self._file = None
            self._writer = None
            self._path = ""
            self._row_count = 0
            self._started_at = None
            return {
                "success": True,
                "recording": False,
                "path": path,
                "rows": rows,
                "msg": f"已结束记录日志: {path}",
            }

    def status(self):
        with self._lock:
            return {
                "recording": self._is_recording_locked(),
                "path": self._path,
                "rows": self._row_count,
                "started_at": self._started_at,
            }

    def close(self):
        with self._lock:
            if self._file is not None:
                try:
                    self._file.flush()
                except Exception:
                    pass
                try:
                    self._file.close()
                except Exception:
                    pass
            self._file = None
            self._writer = None
            self._path = ""
            self._row_count = 0
            self._started_at = None


_wifi_telemetry_recorder = WifiTelemetryCsvRecorder()
atexit.register(_wifi_telemetry_recorder.close)


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


def _resolve_trace_layout(size):
    return {
        "has_ctrl": size >= PAYLOAD_SIZE_CTRL,
        "has_debug": size >= PAYLOAD_SIZE_CTRL_DEBUG,
        "has_nav_diag": size >= PAYLOAD_SIZE_CTRL_NAV_DIAG,
        "ctrl_base": PAYLOAD_SIZE_V2,
        "debug_base": PAYLOAD_SIZE_CTRL,
        "nav_diag_base": PAYLOAD_SIZE_CTRL_DEBUG,
    }


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


def _decode_payload(payload_bytes):
    size = len(payload_bytes)

    if size < PAYLOAD_SIZE_V1:
        return None

    # 兼容扩展 payload：基础字段始终按 V1 解析，后续字段按可用字节补齐。
    unpacked = struct.unpack(STRUCT_FMT_V1, payload_bytes[:PAYLOAD_SIZE_V1])
    data = dict(zip(FIELD_NAMES_V1, unpacked))
    layout = _resolve_trace_layout(size)

    if size >= PAYLOAD_SIZE_V2:
        data["mark_trigger"] = payload_bytes[PAYLOAD_SIZE_V1]
        data["point_type"] = payload_bytes[PAYLOAD_SIZE_V1 + 1]
    else:
        data["mark_trigger"] = 0
        data["point_type"] = 0

    if layout["has_ctrl"]:
        ctrl_base = layout["ctrl_base"]
        data["pid_mode"] = payload_bytes[ctrl_base]
        data["slip_flag"] = payload_bytes[ctrl_base + 1]
        data["minefield_is_active"] = payload_bytes[ctrl_base + 2]
    else:
        data["pid_mode"] = None
        data["slip_flag"] = None
        data["minefield_is_active"] = None

    debug_fields = [
        "target_speed",
        "speed_L",
        "speed_R",
        "theoretical_yaw_rate",
        "actual_yaw_rate",
    ]
    if layout["has_debug"]:
        debug_values = struct.unpack("<5f", payload_bytes[layout["debug_base"] : layout["debug_base"] + PAYLOAD_DEBUG_BYTES])
        data.update(zip(debug_fields, debug_values))
    else:
        data.update({field: None for field in debug_fields})

    nav_diag_fields = [
        "nav_replay_state",
        "nav_special_action_trigger",
        "nav_current_point_type",
        "nav_special_target_idx",
        "nav_special_target_x",
        "nav_special_target_y",
        "nav_special_dist_mm",
        "nav_special_brake_radius_mm",
        "nav_special_speed_ref_mm_s",
        "nav_special_zero_brake_issued",
        "nav_special_zero_brake_active",
        "nav_special_crawl_active",
        "nav_special_prep_zero_latched",
        "brake_ff_pwm",
        "accel_ff_pwm",
        "motor_enable",
        "fallen",
        "remote_brake_active",
        "remote_reverse_brake_active",
        "minefield_accumulated_angle",
        "minefield_angle_cmd",
        "minefield_feedforward_speed",
        "minefield_current_speed_cmd",
        "minefield_stall_elapsed_s",
        "minefield_spin_abort_reason",
    ]
    if layout["has_nav_diag"]:
        nav_diag_values = struct.unpack(
            "<25f",
            payload_bytes[layout["nav_diag_base"] : layout["nav_diag_base"] + PAYLOAD_NAV_DIAG_BYTES],
        )
        data.update(zip(nav_diag_fields, nav_diag_values))
    else:
        data.update({field: None for field in nav_diag_fields})

    telemetry_diag_fields = [
        "euler_roll",
        "euler_pitch",
        "euler_yaw",
        "servo_angle_0",
        "servo_angle_1",
        "servo_angle_2",
        "servo_angle_3",
        "err_degree",
        "imu_gyro_x_rad_s",
        "imu_gyro_y_rad_s",
        "imu_gyro_z_rad_s",
    ]
    if layout["has_telemetry_diag"]:
        telemetry_diag_base = layout["telemetry_diag_base"]
        telemetry_diag_values = struct.unpack(
            "<11f",
            payload_bytes[telemetry_diag_base : telemetry_diag_base + PAYLOAD_TELEMETRY_DIAG_BYTES],
        )
        data.update(zip(telemetry_diag_fields, telemetry_diag_values))
    else:
        data.update({field: None for field in telemetry_diag_fields})

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
            _wifi_telemetry_recorder.record_telemetry_frame(data, raw_frame_bytes=bytes(raw_buffer[:frame_len]), frame_cmd=cmd)
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
            print(f"[TCP] Disconnected: {peer}")
        except socket.timeout:
            continue
        except Exception as exc:
            with state_lock:
                peer_addr = ""
                active_conn = None
                server_error = f"Server error: {exc}"
            time.sleep(0.5)


class Api:
    def __init__(self, telemetry_recorder=None):
        self.telemetry_recorder = telemetry_recorder or _wifi_telemetry_recorder

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
            log_status = self.telemetry_recorder.status()
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
                "wifi_log_recording": log_status["recording"],
                "wifi_log_path": log_status["path"],
                "wifi_log_rows": log_status["rows"],
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

    def start_wifi_csv_recording(self):
        return self.telemetry_recorder.start()

    def stop_wifi_csv_recording(self):
        return self.telemetry_recorder.stop()

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
        title="科目二速度规划打点上位机 (WebView)",
        url=html_path,
        js_api=api,
        width=1480,
        height=920,
        min_size=(1120, 700),
    )
    webview.start(debug=False)

