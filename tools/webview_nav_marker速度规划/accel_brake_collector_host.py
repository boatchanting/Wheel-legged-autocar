import atexit
import os
import signal
import socket
import threading
import time

import webview

import accel_brake_collector_core as core


HOST_IP = "192.168.137.1"
HOST_PORT = 8086
ACK_EVENT_MAX = 64
HOST_ACK_TIMEOUT_SEC = 1.5
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
latest_control_debug = {}
receiver_thread_id = None


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


def _format_send_result(ok, msg="", control_code=None, executed=None):
    return {
        "success": bool(ok),
        "msg": msg,
        "control_code": "" if control_code is None else f"0x{int(control_code) & 0xFF:02X}",
        "executed": executed,
    }


def _format_host_ack_result(control_code, ack_status):
    if ack_status == core.HOST_ACK_ACCEPTED:
        return _format_send_result(True, "小车已执行命令", control_code, executed=True)

    if ack_status == core.HOST_ACK_REJECTED:
        if control_code == core.HOST_CTRL_ARM_DIRECT_SPEED:
            msg = "小车拒绝进入速度采集状态机：请检查遥控器连接、CH6总开关、电机使能"
        elif control_code == core.HOST_CTRL_SET_TARGET_SPEED:
            msg = "小车拒绝目标速度：请检查CH5刹车、CH6总开关、电机使能"
        else:
            msg = "小车拒绝命令：条件不满足"
        return _format_send_result(False, msg, control_code, executed=False)

    if ack_status == core.HOST_ACK_UNKNOWN_CMD:
        return _format_send_result(False, "小车不认识该命令，请确认车端固件已更新", control_code, executed=False)

    if ack_status == core.HOST_ACK_INVALID_PAYLOAD:
        return _format_send_result(False, "小车认为命令载荷格式错误", control_code, executed=False)

    return _format_send_result(False, f"小车返回未知ACK状态 0x{ack_status:02X}", control_code, executed=False)


def _send_control_payload(payload_bytes, wait_ack=True):
    global active_conn, peer_addr, server_error
    if not payload_bytes:
        return _format_send_result(False, "empty control payload")

    control_code = payload_bytes[0]
    frame = core.build_frame(core.CMD_HOST_CONTROL, payload_bytes)
    with ack_cond:
        start_seq = ack_seq

    with tx_lock:
        with state_lock:
            conn = active_conn
            peer = peer_addr
        if conn is None:
            return _format_send_result(False, "小车未连接，命令未发送", control_code, executed=False)
        try:
            conn.sendall(frame)
        except Exception as exc:
            with state_lock:
                if active_conn is conn:
                    active_conn = None
                peer_addr = ""
                server_error = f"Send failed: {exc}"
            return _format_send_result(False, f"发送失败: {exc}", control_code, executed=False)

    if (not wait_ack) or (threading.get_ident() == receiver_thread_id):
        return _format_send_result(True, "命令已发送", control_code, executed=None)

    ack_status = _wait_host_ack(control_code, start_seq, HOST_ACK_TIMEOUT_SEC)
    if ack_status is None:
        return _format_send_result(
            False,
            f"命令已发到小车({peer})，但没有收到ACK：请确认车端已烧录最新协议",
            control_code,
            executed=False,
        )
    return _format_host_ack_result(control_code, ack_status)


def _send_simple_control(control_code, wait_ack=True):
    return _send_control_payload(bytes([int(control_code) & 0xFF]), wait_ack=wait_ack)


def _send_target_speed(target_speed, pid_mode, wait_ack=True):
    return _send_control_payload(
        core.build_set_target_speed_payload(target_speed, pid_mode=pid_mode, flags=0),
        wait_ack=wait_ack,
    )


def _send_stop_direct_speed(wait_ack=True):
    return _send_simple_control(core.HOST_CTRL_STOP_DIRECT_SPEED, wait_ack=wait_ack)


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
    global last_frame_time, last_payload_size, last_rx_time, latest_control_debug
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

        if cmd == core.CMD_CONTROL_DEBUG:
            now = time.time()
            debug = core.decode_control_debug_payload(payload)
            if debug is not None:
                debug["_received_at"] = now
                with state_lock:
                    latest_control_debug = debug
                    last_rx_time = now
            del raw_buffer[:frame_len]
            continue

        if cmd == core.CMD_TELEMETRY:
            now = time.time()
            with state_lock:
                last_frame_time = now
                last_payload_size = payload_len
                control_debug = dict(latest_control_debug)
            data = core.decode_payload(payload)
            if data is not None:
                core.attach_control_debug(data, control_debug, now=now)
                _push_data(data)
                try:
                    controller.feed_frame(data, now=now)
                except Exception as exc:
                    print(f"[TCP] feed_frame error: {exc}")
            else:
                print(f"[TCP] decode_payload returned None, payload_len={payload_len}")

        del raw_buffer[:frame_len]


def tcp_server_thread(controller):
    global active_conn, peer_addr, server_error, receiver_thread_id
    receiver_thread_id = threading.get_ident()
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
        if data:
            print(f"[API] get_new_data returning {len(data)} frames")
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
                "host_sample_period_ms": core.HOST_SAMPLE_PERIOD_MS,
                "last_payload_size": last_payload_size,
                "history_count": len(history_data),
                "server_error": server_error,
                "acks": [
                    {"seq": seq, "control": ctrl, "status": ack_status, "time": ts}
                    for seq, ctrl, ack_status, ts in acks
                ],
            }
        self.controller.tick(now=now)
        status["experiment"] = self.controller.status()
        return status

    def start_experiment(self, target_forward_speed_mm_s, multi_pid, brake_target_speed_mm_s=0.0):
        try:
            speed = float(target_forward_speed_mm_s)
            brake_target_speed = float(brake_target_speed_mm_s)
        except Exception:
            return {"success": False, "msg": "速度参数非法"}
        if abs(speed) <= 1e-6:
            return {"success": False, "msg": "目标速度不能为 0 mm/s"}

        speed_to_mm_s = core.load_speed_to_mm_s()
        vehicle_speed_cmd = core.signed_mm_s_to_vehicle_speed_cmd(speed, speed_to_mm_s)
        brake_vehicle_speed_cmd = core.signed_mm_s_to_vehicle_speed_cmd(brake_target_speed, speed_to_mm_s)

        arm_result = _send_simple_control(core.HOST_CTRL_ARM_DIRECT_SPEED)
        if (not arm_result.get("success")) or (arm_result.get("executed") is False):
            return {
                "success": False,
                "msg": arm_result.get("msg", "速度采集状态机启动失败"),
                "arm_result": arm_result,
            }

        result = self.controller.start(
            target_forward_speed_mm_s=speed,
            brake_target_speed_mm_s=brake_target_speed,
            multi_pid=bool(multi_pid),
            now=time.time(),
            speed_to_mm_s=speed_to_mm_s,
        )
        if not result.get("success"):
            _send_stop_direct_speed()
            return result

        run = result.get("run", {})
        result["msg"] = (
            f"已开始: 目标 {speed:.0f} mm/s -> 车端 {vehicle_speed_cmd:.2f}; "
            f"刹车目标 {brake_target_speed:.0f} mm/s -> 车端 {brake_vehicle_speed_cmd:.2f}"
        )
        result["run_id"] = run.get("run_id", "")
        result["csv_path"] = run.get("csv_path", "")
        result["json_path"] = run.get("json_path", "")
        result["target_motion_speed_mm_s"] = speed
        result["target_forward_speed_mm_s"] = abs(speed)
        result["vehicle_speed_cmd"] = vehicle_speed_cmd
        result["brake_target_speed_mm_s"] = brake_target_speed
        result["brake_vehicle_speed_cmd"] = brake_vehicle_speed_cmd
        result["speed_to_mm_s"] = speed_to_mm_s
        return result

    def cancel_experiment(self):
        return self.controller.cancel(now=time.time())

    def force_brake_experiment(self):
        result = self.controller.force_brake(now=time.time())
        if result.get("success"):
            result["msg"] = "已手动进入刹车阶段"
        return result

    def send_zero_speed(self):
        return _send_target_speed(0.0, core.CONTROL_MODE_BRAKE)

    def send_target_speed(self, target_motion_speed_mm_s, pid_mode):
        try:
            speed = float(target_motion_speed_mm_s)
            mode = int(pid_mode)
        except Exception:
            return {"success": False, "msg": "参数非法"}
        speed_to_mm_s = core.load_speed_to_mm_s()
        vehicle_speed_cmd = core.signed_mm_s_to_vehicle_speed_cmd(speed, speed_to_mm_s)
        result = _send_target_speed(vehicle_speed_cmd, mode)
        result["target_motion_speed_mm_s"] = speed
        result["vehicle_speed_cmd"] = vehicle_speed_cmd
        result["speed_to_mm_s"] = speed_to_mm_s
        if result.get("success"):
            result["msg"] = f"命令已发送: {speed:.0f} mm/s -> 车端 {vehicle_speed_cmd:.2f}"
        return result

    def clear_history(self):
        with state_lock:
            history_data.clear()
            new_data_buffer.clear()
        return {"success": True, "msg": "历史数据已清空"}


def main():
    output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "brake_logs")
    logger = core.AccelBrakeRunLogger(output_dir=output_dir)
    controller = core.AccelBrakeExperimentController(
        logger=logger,
        send_target_speed=_send_target_speed,
        send_stop_direct_speed=_send_stop_direct_speed,
    )

    def shutdown_controller(*_args):
        controller.shutdown(now=time.time())

    def handle_exit_signal(_signum, _frame):
        shutdown_controller()
        raise SystemExit(0)

    atexit.register(shutdown_controller)
    signal.signal(signal.SIGINT, handle_exit_signal)
    if hasattr(signal, "SIGTERM"):
        signal.signal(signal.SIGTERM, handle_exit_signal)

    thread = threading.Thread(target=tcp_server_thread, args=(controller,), daemon=True)
    thread.start()

    current_dir = os.path.dirname(os.path.abspath(__file__))
    html_path = os.path.join(current_dir, "accel_brake_collector.html")
    if not os.path.exists(html_path):
        raise FileNotFoundError(f"HTML not found: {html_path}")

    api = Api(controller)
    webview.create_window(
        title="加速刹车采集器",
        url=html_path,
        js_api=api,
        width=1320,
        height=860,
        min_size=(1040, 680),
    )
    try:
        webview.start(debug=False)
    finally:
        shutdown_controller()


if __name__ == "__main__":
    main()
