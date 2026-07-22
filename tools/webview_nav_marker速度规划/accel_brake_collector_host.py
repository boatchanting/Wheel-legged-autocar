import atexit
import os
import socket
import threading
import time

import webview

import accel_brake_collector_core as core


HOST_IP = "192.168.137.1"
HOST_PORT = 8086
ACK_EVENT_MAX = 64
MAX_NEW_BUFFER = 2000
MAX_HISTORY = 10000

state_lock = threading.Lock()
tx_lock = threading.Lock()
ack_cond = threading.Condition()

active_conn = None
peer_addr = ""
listen_ip = HOST_IP
server_error = ""
last_rx_time = 0.0
last_frame_time = 0.0
last_payload_size = 0
ack_seq = 0
ack_events = []
new_data_buffer = []
history_data = []


def _push_host_ack(control_code, status_code):
    global ack_seq
    with ack_cond:
        ack_seq += 1
        ack_events.append((ack_seq, int(control_code) & 0xFF, int(status_code) & 0xFF, time.time()))
        if len(ack_events) > ACK_EVENT_MAX:
            del ack_events[: len(ack_events) - ACK_EVENT_MAX]
        ack_cond.notify_all()


def _push_data(data):
    global last_rx_time, last_payload_size
    with state_lock:
        history_data.append(data)
        if len(history_data) > MAX_HISTORY:
            del history_data[: len(history_data) - MAX_HISTORY]
        new_data_buffer.append(data)
        if len(new_data_buffer) > MAX_NEW_BUFFER:
            del new_data_buffer[: len(new_data_buffer) - MAX_NEW_BUFFER]
        last_rx_time = time.time()
        last_payload_size = data.get("payload_size", 0)


def _format_send_result(ok, msg="", control_code=None):
    return {
        "success": bool(ok),
        "msg": msg,
        "control_code": "" if control_code is None else f"0x{int(control_code) & 0xFF:02X}",
    }


def _send_control_payload(payload_bytes):
    global active_conn, peer_addr, server_error
    if not payload_bytes:
        return _format_send_result(False, "empty control payload")

    control_code = payload_bytes[0]
    frame = core.build_frame(core.CMD_HOST_CONTROL, payload_bytes)
    with tx_lock:
        with state_lock:
            conn = active_conn
        if conn is None:
            return _format_send_result(False, "小车未连接", control_code)
        try:
            conn.sendall(frame)
        except Exception as exc:
            with state_lock:
                if active_conn is conn:
                    active_conn = None
                peer_addr = ""
                server_error = f"Send failed: {exc}"
            return _format_send_result(False, f"发送失败: {exc}", control_code)
    return _format_send_result(True, "命令已发送", control_code)


def _send_simple_control(control_code):
    return _send_control_payload(bytes([int(control_code) & 0xFF]))


def _send_target_speed(target_speed, pid_mode):
    return _send_control_payload(core.build_set_target_speed_payload(target_speed, pid_mode=pid_mode, flags=0))


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
    raise RuntimeError("Bind failed")


def _parse_frame_stream(raw_buffer, controller):
    global last_frame_time, last_payload_size
    while len(raw_buffer) >= core.FRAME_MIN_SIZE:
        if raw_buffer[0] != core.FRAME_HEAD1 or raw_buffer[1] != core.FRAME_HEAD2:
            del raw_buffer[0]
            continue

        payload_len = raw_buffer[3]
        frame_len = payload_len + core.FRAME_MIN_SIZE
        if payload_len <= 0 or frame_len > 320:
            del raw_buffer[0]
            continue
        if len(raw_buffer) < frame_len:
            break
        if raw_buffer[frame_len - 1] != core.FRAME_TAIL:
            del raw_buffer[0]
            continue
        calc_sum = sum(raw_buffer[: frame_len - 2]) & 0xFF
        recv_sum = raw_buffer[frame_len - 2]
        if calc_sum != recv_sum:
            del raw_buffer[0]
            continue

        cmd = raw_buffer[2]
        payload = bytes(raw_buffer[4 : 4 + payload_len])
        if cmd == core.CMD_HOST_ACK:
            if payload_len >= 2:
                _push_host_ack(payload[0], payload[1])
            del raw_buffer[:frame_len]
            continue

        if cmd == core.CMD_TELEMETRY:
            now = time.time()
            with state_lock:
                last_frame_time = now
                last_payload_size = payload_len
            data = core.decode_payload(payload)
            if data is not None:
                _push_data(data)
                controller.feed_frame(data, now=now)

        del raw_buffer[:frame_len]


def tcp_server_thread(controller):
    global active_conn, peer_addr, server_error
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
                active_conn = conn
                peer_addr = peer
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
                        _parse_frame_stream(raw_buffer, controller)
                    except socket.timeout:
                        continue
            with state_lock:
                if active_conn is conn:
                    active_conn = None
                peer_addr = ""
            print(f"[TCP] Disconnected: {peer}")
        except socket.timeout:
            continue
        except Exception as exc:
            with state_lock:
                active_conn = None
                peer_addr = ""
                server_error = f"Server error: {exc}"
            time.sleep(0.5)


class Api:
    def __init__(self, controller):
        self.controller = controller

    def get_new_data(self):
        with state_lock:
            data = list(new_data_buffer)
            new_data_buffer.clear()
        return data

    def get_status(self):
        with state_lock:
            now = time.time()
            connected = bool(peer_addr)
            fresh = (now - last_rx_time) < 1.2
            frame_fresh = (now - last_frame_time) < 1.2
            acks = list(ack_events[-5:])
            status = {
                "connected": connected,
                "data_fresh": fresh,
                "frame_fresh": frame_fresh,
                "peer": peer_addr,
                "host_ip": listen_ip,
                "host_port": HOST_PORT,
                "last_payload_size": last_payload_size,
                "history_count": len(history_data),
                "server_error": server_error,
                "acks": [
                    {"seq": seq, "control": ctrl, "status": ack_status, "time": ts}
                    for seq, ctrl, ack_status, ts in acks
                ],
            }
        status["experiment"] = self.controller.status()
        return status

    def start_experiment(self, target_speed, multi_pid):
        try:
            speed = float(target_speed)
        except Exception:
            return {"success": False, "msg": "目标速度非法"}
        _send_simple_control(core.HOST_CTRL_START_CAR)
        return self.controller.start(speed, bool(multi_pid), now=time.time())

    def cancel_experiment(self):
        return self.controller.cancel(now=time.time())

    def send_zero_speed(self):
        return _send_target_speed(0.0, core.CONTROL_MODE_BRAKE)

    def send_target_speed(self, target_speed, pid_mode):
        try:
            speed = float(target_speed)
            mode = int(pid_mode)
        except Exception:
            return {"success": False, "msg": "参数非法"}
        return _send_target_speed(speed, mode)

    def clear_history(self):
        with state_lock:
            history_data.clear()
            new_data_buffer.clear()
        return {"success": True, "msg": "历史数据已清空"}


def main():
    output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "brake_logs")
    logger = core.AccelBrakeRunLogger(output_dir=output_dir)
    controller = core.AccelBrakeExperimentController(logger=logger, send_target_speed=_send_target_speed)
    atexit.register(logger.close)

    thread = threading.Thread(target=tcp_server_thread, args=(controller,), daemon=True)
    thread.start()

    current_dir = os.path.dirname(os.path.abspath(__file__))
    html_path = os.path.join(current_dir, "accel_brake_collector.html")
    if not os.path.exists(html_path):
        raise FileNotFoundError(f"HTML not found: {html_path}")

    api = Api(controller)
    window = webview.create_window(
        title="加速刹车采集器",
        url=html_path,
        js_api=api,
        width=1320,
        height=860,
        min_size=(1040, 680),
    )
    webview.start(debug=False)


if __name__ == "__main__":
    main()
