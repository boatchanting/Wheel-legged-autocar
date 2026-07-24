import csv
import json
import math
import os
import re
import struct
import time
from pathlib import Path


FRAME_HEAD1 = 0x5A
FRAME_HEAD2 = 0xA5
FRAME_TAIL = 0xED
FRAME_MIN_SIZE = 6

CMD_TELEMETRY = 0x01
CMD_HOST_CONTROL = 0x10
CMD_HOST_ACK = 0x11

HOST_CTRL_CLEAR_TRAJECTORY = 0x01
HOST_CTRL_START_CAR = 0x02
HOST_CTRL_START_LOG = 0x05
HOST_CTRL_STOP_LOG = 0x06
HOST_CTRL_SET_TARGET_SPEED = 0x20
HOST_CTRL_ARM_DIRECT_SPEED = 0x21
HOST_CTRL_STOP_DIRECT_SPEED = 0x22

HOST_ACK_ACCEPTED = 0x00
HOST_ACK_REJECTED = 0x01
HOST_ACK_UNKNOWN_CMD = 0x02
HOST_ACK_INVALID_PAYLOAD = 0x03

CONTROL_MODE_NORMAL = 0
CONTROL_MODE_ACCEL = 1
CONTROL_MODE_BRAKE = 2
DEFAULT_SPEED_TO_MM_S = 4.79
HOST_SAMPLE_PERIOD_MS = 10

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

DEBUG_FIELD_NAMES = [
    "target_speed",
    "speed_L",
    "speed_R",
    "theoretical_yaw_rate",
    "actual_yaw_rate",
]

PLAN_DEBUG_FIELD_NAMES = [
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
]

MOTION_IMU_FIELD_NAMES = [
    "att_roll",
    "att_pitch",
    "att_yaw",
    "imu_gyro_x",
    "imu_gyro_y",
    "imu_gyro_z",
    "imu_acc_x",
    "imu_acc_y",
    "imu_acc_z",
    "imu_grav_x",
    "imu_grav_y",
    "imu_grav_z",
]

PAYLOAD_SIZE_V1 = struct.calcsize(STRUCT_FMT_V1)
PAYLOAD_SIZE_V2 = PAYLOAD_SIZE_V1 + 2
PAYLOAD_GPS_TRACE_FLOAT_BYTES = 8
PAYLOAD_GPS_TRACE_FLAG_BYTES = 2
PAYLOAD_GPS_TRACE_CTRL_BYTES = 2
PAYLOAD_DEBUG_BYTES = 20
PAYLOAD_FUSION_TRACE_FLOAT_BYTES = 8
PAYLOAD_FUSION_TRACE_FLAG_BYTES = 1
PAYLOAD_PLAN_DEBUG_BYTES = 19 * 4
PAYLOAD_MOTION_IMU_BYTES = 12 * 4

PAYLOAD_SIZE_GPS_TRACE = PAYLOAD_SIZE_V2 + PAYLOAD_GPS_TRACE_FLOAT_BYTES + PAYLOAD_GPS_TRACE_FLAG_BYTES
PAYLOAD_SIZE_GPS_TRACE_CTRL = PAYLOAD_SIZE_GPS_TRACE + PAYLOAD_GPS_TRACE_CTRL_BYTES
PAYLOAD_SIZE_GPS_TRACE_DEBUG = PAYLOAD_SIZE_GPS_TRACE_CTRL + PAYLOAD_DEBUG_BYTES
PAYLOAD_SIZE_TRACE = PAYLOAD_SIZE_GPS_TRACE + PAYLOAD_FUSION_TRACE_FLOAT_BYTES + PAYLOAD_FUSION_TRACE_FLAG_BYTES
PAYLOAD_SIZE_TRACE_CTRL = PAYLOAD_SIZE_TRACE + PAYLOAD_GPS_TRACE_CTRL_BYTES
PAYLOAD_SIZE_TRACE_DEBUG = PAYLOAD_SIZE_TRACE_CTRL + PAYLOAD_DEBUG_BYTES
PAYLOAD_SIZE_TRACE_PLAN_DEBUG = PAYLOAD_SIZE_TRACE_DEBUG + PAYLOAD_PLAN_DEBUG_BYTES
PAYLOAD_SIZE_TRACE_PLAN_DEBUG_MOTION = PAYLOAD_SIZE_TRACE_PLAN_DEBUG + PAYLOAD_MOTION_IMU_BYTES

BASE_LOG_FIELDS = [
    "recorded_at",
    "elapsed_s",
    "frame_elapsed_ms",
    "phase",
    "phase_elapsed_s",
    "host_sample_period_ms",
    "target_motion_speed_mm_s",
    "target_forward_speed_mm_s",
    "vehicle_speed_cmd",
    "brake_target_speed_mm_s",
    "brake_vehicle_speed_cmd",
    "commanded_vehicle_speed_cmd",
    "speed_to_mm_s",
    "pid_mode",
    "speed_abs",
    "accel_from_vx",
    "distance_from_start",
    "phase_distance",
]

ACCEL_BRAKE_LOG_FIELDS = (
    BASE_LOG_FIELDS
    + FIELD_NAMES_V2
    + [
        "gps_x",
        "gps_y",
        "gps_valid",
        "gps_origin_set",
        "fusion_x",
        "fusion_y",
        "fusion_valid",
        "slip_flag",
    ]
    + DEBUG_FIELD_NAMES
    + PLAN_DEBUG_FIELD_NAMES
    + MOTION_IMU_FIELD_NAMES
    + [
        "has_debug",
        "has_plan_debug",
        "has_motion_imu",
        "payload_size",
        "time_str",
    ]
)

SUMMARY_FIELDS = [
    "run_id",
    "status",
    "target_motion_speed_mm_s",
    "target_forward_speed_mm_s",
    "vehicle_speed_cmd",
    "brake_target_speed_mm_s",
    "brake_vehicle_speed_cmd",
    "speed_to_mm_s",
    "host_sample_period_ms",
    "multi_pid",
    "started_at",
    "finished_at",
    "sample_count",
    "accel_time_s",
    "hold_time_s",
    "brake_time_s",
    "accel_distance_mm",
    "hold_distance_mm",
    "brake_distance_mm",
    "total_distance_mm",
    "csv_path",
    "json_path",
]


def safe_float(value, default=None):
    try:
        num = float(value)
    except (TypeError, ValueError):
        return default
    if not math.isfinite(num):
        return default
    return num


def forward_mm_s_to_vehicle_speed_cmd(forward_mm_s, speed_to_mm_s=DEFAULT_SPEED_TO_MM_S):
    speed_to_mm_s = float(speed_to_mm_s)
    if speed_to_mm_s <= 0.0:
        raise ValueError("speed_to_mm_s must be positive")
    return -abs(float(forward_mm_s)) / speed_to_mm_s


def vehicle_speed_cmd_to_forward_mm_s(vehicle_speed_cmd, speed_to_mm_s=DEFAULT_SPEED_TO_MM_S):
    speed_to_mm_s = float(speed_to_mm_s)
    if speed_to_mm_s <= 0.0:
        raise ValueError("speed_to_mm_s must be positive")
    return abs(float(vehicle_speed_cmd)) * speed_to_mm_s


def signed_mm_s_to_vehicle_speed_cmd(speed_mm_s, speed_to_mm_s=DEFAULT_SPEED_TO_MM_S):
    speed_to_mm_s = float(speed_to_mm_s)
    if speed_to_mm_s <= 0.0:
        raise ValueError("speed_to_mm_s must be positive")
    return -float(speed_mm_s) / speed_to_mm_s


def vehicle_speed_cmd_to_signed_mm_s(vehicle_speed_cmd, speed_to_mm_s=DEFAULT_SPEED_TO_MM_S):
    speed_to_mm_s = float(speed_to_mm_s)
    if speed_to_mm_s <= 0.0:
        raise ValueError("speed_to_mm_s must be positive")
    return -float(vehicle_speed_cmd) * speed_to_mm_s


def _safe_eval_speed_factor(expr):
    expr = expr.replace("f", "").replace("F", "")
    if not re.fullmatch(r"[0-9+\-*/().\s]+", expr):
        raise ValueError(f"unsafe speed factor expression: {expr}")
    return float(eval(expr, {"__builtins__": {}}, {}))


def load_speed_to_mm_s(project_root=None):
    root = Path(project_root) if project_root else Path(__file__).resolve().parents[2]
    car_select_path = root / "code" / "config" / "car_select.h"
    inertial_nav_path = root / "code" / "navigation" / "inertial_nav.h"

    try:
        car_text = car_select_path.read_text(encoding="utf-8", errors="ignore")
        nav_text = inertial_nav_path.read_text(encoding="utf-8", errors="ignore")
        car_match = re.search(r"#define\s+CAR_SELECT\s+(\d+)", car_text)
        if not car_match:
            return DEFAULT_SPEED_TO_MM_S
        car_select = car_match.group(1)

        block_match = re.search(
            rf"#if\s+CAR_SELECT\s*==\s*{re.escape(car_select)}\b(?P<body>.*?)(?=#endif)",
            nav_text,
            re.S,
        )
        if not block_match:
            return DEFAULT_SPEED_TO_MM_S

        speed_match = re.search(r"#define\s+SPEED_TO_MM_S\s+([^\r\n/]+)", block_match.group("body"))
        if not speed_match:
            return DEFAULT_SPEED_TO_MM_S
        return _safe_eval_speed_factor(speed_match.group(1).strip())
    except Exception:
        return DEFAULT_SPEED_TO_MM_S


def build_frame(cmd, payload_bytes=b""):
    payload_len = len(payload_bytes)
    if payload_len > 255:
        raise ValueError("payload too long")
    frame = bytearray([FRAME_HEAD1, FRAME_HEAD2, cmd & 0xFF, payload_len & 0xFF])
    frame.extend(payload_bytes)
    frame.append(sum(frame) & 0xFF)
    frame.append(FRAME_TAIL)
    return bytes(frame)


def build_set_target_speed_payload(target_speed, pid_mode=CONTROL_MODE_NORMAL, flags=0):
    mode = int(pid_mode) & 0xFF
    return struct.pack("<BfBB", HOST_CTRL_SET_TARGET_SPEED, float(target_speed), mode, int(flags) & 0xFF)


def build_set_target_speed_frame(target_speed, pid_mode=CONTROL_MODE_NORMAL, flags=0):
    return build_frame(CMD_HOST_CONTROL, build_set_target_speed_payload(target_speed, pid_mode, flags))


def _resolve_trace_layout(size):
    if size >= PAYLOAD_SIZE_TRACE_DEBUG:
        return {
            "has_gps": True,
            "has_fusion": True,
            "has_pid_mode": True,
            "has_slip_flag": True,
            "has_debug": True,
            "gps_base": PAYLOAD_SIZE_V2,
            "fusion_base": PAYLOAD_SIZE_GPS_TRACE,
            "pid_base": PAYLOAD_SIZE_TRACE,
            "debug_base": PAYLOAD_SIZE_TRACE_CTRL,
        }

    if size >= PAYLOAD_SIZE_GPS_TRACE_DEBUG:
        return {
            "has_gps": True,
            "has_fusion": False,
            "has_pid_mode": True,
            "has_slip_flag": True,
            "has_debug": True,
            "gps_base": PAYLOAD_SIZE_V2,
            "fusion_base": PAYLOAD_SIZE_GPS_TRACE,
            "pid_base": PAYLOAD_SIZE_GPS_TRACE,
            "debug_base": PAYLOAD_SIZE_GPS_TRACE_CTRL,
        }

    if size >= PAYLOAD_SIZE_TRACE_CTRL:
        return {
            "has_gps": True,
            "has_fusion": True,
            "has_pid_mode": True,
            "has_slip_flag": True,
            "has_debug": False,
            "gps_base": PAYLOAD_SIZE_V2,
            "fusion_base": PAYLOAD_SIZE_GPS_TRACE,
            "pid_base": PAYLOAD_SIZE_TRACE,
            "debug_base": None,
        }

    if size >= PAYLOAD_SIZE_GPS_TRACE_CTRL:
        return {
            "has_gps": True,
            "has_fusion": False,
            "has_pid_mode": True,
            "has_slip_flag": True,
            "has_debug": False,
            "gps_base": PAYLOAD_SIZE_V2,
            "fusion_base": PAYLOAD_SIZE_GPS_TRACE,
            "pid_base": PAYLOAD_SIZE_GPS_TRACE,
            "debug_base": None,
        }

    if size >= PAYLOAD_SIZE_TRACE:
        return {
            "has_gps": True,
            "has_fusion": True,
            "has_pid_mode": False,
            "has_slip_flag": False,
            "has_debug": False,
            "gps_base": PAYLOAD_SIZE_V2,
            "fusion_base": PAYLOAD_SIZE_GPS_TRACE,
            "pid_base": None,
            "debug_base": None,
        }

    if size >= PAYLOAD_SIZE_GPS_TRACE:
        return {
            "has_gps": True,
            "has_fusion": False,
            "has_pid_mode": False,
            "has_slip_flag": False,
            "has_debug": False,
            "gps_base": PAYLOAD_SIZE_V2,
            "fusion_base": PAYLOAD_SIZE_GPS_TRACE,
            "pid_base": None,
            "debug_base": None,
        }

    return {
        "has_gps": False,
        "has_fusion": False,
        "has_pid_mode": False,
        "has_slip_flag": False,
        "has_debug": False,
        "gps_base": PAYLOAD_SIZE_V2,
        "fusion_base": PAYLOAD_SIZE_GPS_TRACE,
        "pid_base": None,
        "debug_base": None,
    }


def decode_payload(payload_bytes):
    size = len(payload_bytes)
    if size < PAYLOAD_SIZE_V1:
        return None

    unpacked = struct.unpack(STRUCT_FMT_V1, payload_bytes[:PAYLOAD_SIZE_V1])
    data = dict(zip(FIELD_NAMES_V1, unpacked))
    layout = _resolve_trace_layout(size)

    if size >= PAYLOAD_SIZE_V2:
        data["mark_trigger"] = payload_bytes[PAYLOAD_SIZE_V1]
        data["point_type"] = payload_bytes[PAYLOAD_SIZE_V1 + 1]
    else:
        data["mark_trigger"] = 0
        data["point_type"] = 0

    if layout["has_gps"]:
        gps_base = layout["gps_base"]
        gps_x, gps_y = struct.unpack("<ff", payload_bytes[gps_base : gps_base + 8])
        data["gps_x"] = gps_x
        data["gps_y"] = gps_y
        data["gps_valid"] = payload_bytes[gps_base + 8]
        data["gps_origin_set"] = payload_bytes[gps_base + 9]
    else:
        data["gps_x"] = None
        data["gps_y"] = None
        data["gps_valid"] = 0
        data["gps_origin_set"] = 0

    if layout["has_fusion"]:
        fusion_base = layout["fusion_base"]
        fusion_x, fusion_y = struct.unpack("<ff", payload_bytes[fusion_base : fusion_base + 8])
        data["fusion_x"] = fusion_x
        data["fusion_y"] = fusion_y
        data["fusion_valid"] = payload_bytes[fusion_base + 8]
    else:
        data["fusion_x"] = None
        data["fusion_y"] = None
        data["fusion_valid"] = 0

    data["pid_mode"] = payload_bytes[layout["pid_base"]] if layout["has_pid_mode"] else 0
    data["slip_flag"] = payload_bytes[layout["pid_base"] + 1] if layout["has_slip_flag"] else 0

    if layout["has_debug"]:
        debug_floats = struct.unpack("<fffff", payload_bytes[layout["debug_base"] : layout["debug_base"] + PAYLOAD_DEBUG_BYTES])
        data.update(dict(zip(DEBUG_FIELD_NAMES, debug_floats)))
        data["has_debug"] = True
    else:
        for field in DEBUG_FIELD_NAMES:
            data[field] = None
        data["has_debug"] = False

    if size >= PAYLOAD_SIZE_TRACE_PLAN_DEBUG:
        plan_floats = struct.unpack(
            "<" + "f" * len(PLAN_DEBUG_FIELD_NAMES),
            payload_bytes[PAYLOAD_SIZE_TRACE_DEBUG : PAYLOAD_SIZE_TRACE_PLAN_DEBUG],
        )
        data.update(dict(zip(PLAN_DEBUG_FIELD_NAMES, plan_floats)))
        data["has_plan_debug"] = True
    else:
        for field in PLAN_DEBUG_FIELD_NAMES:
            data[field] = None
        data["has_plan_debug"] = False

    if size >= PAYLOAD_SIZE_TRACE_PLAN_DEBUG_MOTION:
        motion_floats = struct.unpack(
            "<" + "f" * len(MOTION_IMU_FIELD_NAMES),
            payload_bytes[PAYLOAD_SIZE_TRACE_PLAN_DEBUG : PAYLOAD_SIZE_TRACE_PLAN_DEBUG_MOTION],
        )
        data.update(dict(zip(MOTION_IMU_FIELD_NAMES, motion_floats)))
        data["has_motion_imu"] = True
    else:
        for field in MOTION_IMU_FIELD_NAMES:
            data[field] = None
        data["has_motion_imu"] = False

    data["payload_size"] = size
    data["time_str"] = f"{data['hour']:02d}:{data['minute']:02d}:{data['second']:02d}"
    return data


def _fmt(value):
    if value is None:
        return ""
    if isinstance(value, bool):
        return int(value)
    if isinstance(value, float):
        if not math.isfinite(value):
            return ""
        return f"{value:.6f}"
    return value


def _distance_from(frame, start_x, start_y):
    x = safe_float(frame.get("nav_x"))
    y = safe_float(frame.get("nav_y"))
    if x is None or y is None or start_x is None or start_y is None:
        return 0.0
    return math.hypot(x - start_x, y - start_y)


class AccelBrakeRunLogger:
    def __init__(self, output_dir=None):
        base = Path(output_dir) if output_dir else Path(__file__).resolve().parent / "brake_logs"
        self.output_dir = base
        self.summary_path = self.output_dir / "brake_summary.csv"
        self._file = None
        self._writer = None
        self._csv_path = None
        self._json_path = None
        self._target_motion_speed_mm_s = 0.0
        self._run_id = ""
        self._target_forward_speed_mm_s = 0.0
        self._vehicle_speed_cmd = 0.0
        self._brake_target_speed_mm_s = 0.0
        self._brake_vehicle_speed_cmd = 0.0
        self._speed_to_mm_s = DEFAULT_SPEED_TO_MM_S
        self._multi_pid = False
        self._started_at = None
        self._phase = "idle"
        self._phase_started_at = None
        self._phase_started_distance = 0.0
        self._phase_stats = {}
        self._sample_count = 0
        self._start_x = None
        self._start_y = None
        self._prev_time = None
        self._prev_vx = None
        self._last_distance = 0.0
        self._last_logged_sample_period_ms = None

    @property
    def recording(self):
        return self._file is not None and self._writer is not None

    @property
    def csv_path(self):
        return "" if self._csv_path is None else str(self._csv_path)

    def start(
        self,
        target_forward_speed_mm_s,
        vehicle_speed_cmd=None,
        brake_target_speed_mm_s=0.0,
        brake_vehicle_speed_cmd=None,
        multi_pid=False,
        now=None,
        speed_to_mm_s=DEFAULT_SPEED_TO_MM_S,
    ):
        if self.recording:
            return {"success": False, "msg": "run already recording", "csv_path": self.csv_path}

        now = time.time() if now is None else float(now)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self._target_motion_speed_mm_s = float(target_forward_speed_mm_s)
        self._target_forward_speed_mm_s = abs(self._target_motion_speed_mm_s)
        self._speed_to_mm_s = float(speed_to_mm_s)
        self._vehicle_speed_cmd = (
            float(vehicle_speed_cmd)
            if vehicle_speed_cmd is not None
            else signed_mm_s_to_vehicle_speed_cmd(self._target_motion_speed_mm_s, self._speed_to_mm_s)
        )
        self._brake_target_speed_mm_s = float(brake_target_speed_mm_s)
        self._brake_vehicle_speed_cmd = (
            float(brake_vehicle_speed_cmd)
            if brake_vehicle_speed_cmd is not None
            else signed_mm_s_to_vehicle_speed_cmd(self._brake_target_speed_mm_s, self._speed_to_mm_s)
        )
        self._multi_pid = bool(multi_pid)
        self._started_at = now
        self._phase = "idle"
        self._phase_started_at = now
        self._phase_started_distance = 0.0
        self._phase_stats = {}
        self._sample_count = 0
        self._start_x = None
        self._start_y = None
        self._prev_time = None
        self._prev_vx = None
        self._last_distance = 0.0
        self._last_logged_sample_period_ms = None

        speed_prefix = "fwd" if self._target_motion_speed_mm_s >= 0.0 else "rev"
        speed_label = f"{speed_prefix}_{abs(self._target_motion_speed_mm_s):.0f}mmps".replace(".", "p")
        mode_label = "multi_pid" if self._multi_pid else "normal_pid"
        base_run_id = time.strftime("%Y%m%d_%H%M%S", time.localtime(now)) + f"_speed_{speed_label}_{mode_label}"
        self._run_id, self._csv_path, self._json_path = self._allocate_run_paths(base_run_id)
        self._file = self._csv_path.open("w", newline="", encoding="utf-8-sig")
        self._writer = csv.DictWriter(self._file, fieldnames=ACCEL_BRAKE_LOG_FIELDS)
        self._writer.writeheader()
        self._file.flush()
        return {
            "success": True,
            "run_id": self._run_id,
            "csv_path": str(self._csv_path),
            "json_path": str(self._json_path),
            "target_motion_speed_mm_s": self._target_motion_speed_mm_s,
            "target_forward_speed_mm_s": self._target_forward_speed_mm_s,
            "vehicle_speed_cmd": self._vehicle_speed_cmd,
            "brake_target_speed_mm_s": self._brake_target_speed_mm_s,
            "brake_vehicle_speed_cmd": self._brake_vehicle_speed_cmd,
            "speed_to_mm_s": self._speed_to_mm_s,
            "host_sample_period_ms": HOST_SAMPLE_PERIOD_MS,
        }

    def _allocate_run_paths(self, base_run_id):
        suffix = 0
        while True:
            run_id = base_run_id if suffix == 0 else f"{base_run_id}_{suffix:03d}"
            csv_path = self.output_dir / f"accel_brake_{run_id}.csv"
            json_path = self.output_dir / f"accel_brake_{run_id}.json"
            if not csv_path.exists() and not json_path.exists():
                return run_id, csv_path, json_path
            suffix += 1

    def set_phase(self, phase, now=None):
        if not self.recording:
            return
        now = time.time() if now is None else float(now)
        self._close_phase(now)
        self._phase = str(phase)
        self._phase_started_at = now
        self._phase_started_distance = self._last_distance

    def _close_phase(self, now):
        if self._phase in ("idle", ""):
            return
        stat = self._phase_stats.setdefault(self._phase, {"time_s": 0.0, "distance_mm": 0.0})
        if self._phase_started_at is not None:
            stat["time_s"] += max(0.0, float(now) - self._phase_started_at)
        stat["distance_mm"] += max(0.0, self._last_distance - self._phase_started_distance)

    def record_frame(
        self,
        frame,
        now=None,
        commanded_vehicle_speed_cmd=None,
        pid_mode=None,
        commanded_target_speed=None,
    ):
        if not self.recording:
            return False

        now = time.time() if now is None else float(now)
        x = safe_float(frame.get("nav_x"))
        y = safe_float(frame.get("nav_y"))
        if self._start_x is None and x is not None and y is not None:
            self._start_x = x
            self._start_y = y

        distance = _distance_from(frame, self._start_x, self._start_y)
        vx = safe_float(frame.get("vx_body"), 0.0)
        if self._prev_time is None or self._prev_vx is None:
            accel = 0.0
        else:
            dt = max(1e-6, now - self._prev_time)
            accel = (vx - self._prev_vx) / dt

        row = {field: "" for field in ACCEL_BRAKE_LOG_FIELDS}
        row.update({field: _fmt(frame.get(field)) for field in FIELD_NAMES_V2})
        for field in [
            "gps_x",
            "gps_y",
            "gps_valid",
            "gps_origin_set",
            "fusion_x",
            "fusion_y",
            "fusion_valid",
            "slip_flag",
            "has_debug",
            "has_plan_debug",
            "has_motion_imu",
            "payload_size",
            "time_str",
        ] + DEBUG_FIELD_NAMES + PLAN_DEBUG_FIELD_NAMES + MOTION_IMU_FIELD_NAMES:
            row[field] = _fmt(frame.get(field))

        phase_elapsed = 0.0 if self._phase_started_at is None else max(0.0, now - self._phase_started_at)
        elapsed_s = max(0.0, now - self._started_at)
        sample_period_cell = ""
        if self._last_logged_sample_period_ms != HOST_SAMPLE_PERIOD_MS:
            sample_period_cell = _fmt(HOST_SAMPLE_PERIOD_MS)
            self._last_logged_sample_period_ms = HOST_SAMPLE_PERIOD_MS
        row.update(
            {
                "recorded_at": time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(now)),
                "elapsed_s": _fmt(elapsed_s),
                "frame_elapsed_ms": _fmt(elapsed_s * 1000.0),
                "phase": self._phase,
                "phase_elapsed_s": _fmt(phase_elapsed),
                "host_sample_period_ms": sample_period_cell,
                "target_motion_speed_mm_s": _fmt(self._target_motion_speed_mm_s),
                "target_forward_speed_mm_s": _fmt(self._target_forward_speed_mm_s),
                "vehicle_speed_cmd": _fmt(self._vehicle_speed_cmd),
                "brake_target_speed_mm_s": _fmt(self._brake_target_speed_mm_s),
                "brake_vehicle_speed_cmd": _fmt(self._brake_vehicle_speed_cmd),
                "commanded_vehicle_speed_cmd": _fmt(
                    commanded_vehicle_speed_cmd
                    if commanded_vehicle_speed_cmd is not None
                    else commanded_target_speed
                ),
                "speed_to_mm_s": _fmt(self._speed_to_mm_s),
                "pid_mode": "" if pid_mode is None else int(pid_mode),
                "speed_abs": _fmt(abs(vx)),
                "accel_from_vx": _fmt(accel),
                "distance_from_start": _fmt(distance),
                "phase_distance": _fmt(max(0.0, distance - self._phase_started_distance)),
            }
        )

        self._writer.writerow(row)
        self._file.flush()
        self._sample_count += 1
        self._prev_time = now
        self._prev_vx = vx
        self._last_distance = distance
        return True

    def finish(self, status="completed", now=None):
        if not self.recording:
            return {"success": False, "msg": "no active run"}

        now = time.time() if now is None else float(now)
        self._close_phase(now)
        try:
            self._file.flush()
        finally:
            self._file.close()

        self._file = None
        self._writer = None

        summary = self._build_summary(status, now)
        self._json_path.write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
        self._append_summary(summary)
        return {
            "success": True,
            "summary": summary,
            "csv_path": str(self._csv_path),
            "json_path": str(self._json_path),
            "summary_path": str(self.summary_path),
        }

    def _build_summary(self, status, finished_at):
        return {
            "run_id": self._run_id,
            "status": status,
            "target_motion_speed_mm_s": self._target_motion_speed_mm_s,
            "target_forward_speed_mm_s": self._target_forward_speed_mm_s,
            "vehicle_speed_cmd": self._vehicle_speed_cmd,
            "brake_target_speed_mm_s": self._brake_target_speed_mm_s,
            "brake_vehicle_speed_cmd": self._brake_vehicle_speed_cmd,
            "speed_to_mm_s": self._speed_to_mm_s,
            "host_sample_period_ms": HOST_SAMPLE_PERIOD_MS,
            "multi_pid": self._multi_pid,
            "started_at": self._started_at,
            "finished_at": finished_at,
            "sample_count": self._sample_count,
            "accel_time_s": self._phase_stats.get("accel", {}).get("time_s", 0.0),
            "hold_time_s": self._phase_stats.get("hold", {}).get("time_s", 0.0),
            "brake_time_s": self._phase_stats.get("brake", {}).get("time_s", 0.0),
            "accel_distance_mm": self._phase_stats.get("accel", {}).get("distance_mm", 0.0),
            "hold_distance_mm": self._phase_stats.get("hold", {}).get("distance_mm", 0.0),
            "brake_distance_mm": self._phase_stats.get("brake", {}).get("distance_mm", 0.0),
            "total_distance_mm": self._last_distance,
            "csv_path": str(self._csv_path),
            "json_path": str(self._json_path),
        }

    def _append_summary(self, summary):
        self.output_dir.mkdir(parents=True, exist_ok=True)
        write_header = not self.summary_path.exists()
        with self.summary_path.open("a", newline="", encoding="utf-8-sig") as f:
            writer = csv.DictWriter(f, fieldnames=SUMMARY_FIELDS)
            if write_header:
                writer.writeheader()
            row = {field: _fmt(summary.get(field)) for field in SUMMARY_FIELDS}
            writer.writerow(row)

    def status(self):
        return {
            "recording": self.recording,
            "run_id": self._run_id,
            "phase": self._phase,
            "csv_path": self.csv_path,
            "rows": self._sample_count,
            "target_motion_speed_mm_s": self._target_motion_speed_mm_s,
            "target_forward_speed_mm_s": self._target_forward_speed_mm_s,
            "vehicle_speed_cmd": self._vehicle_speed_cmd,
            "brake_target_speed_mm_s": self._brake_target_speed_mm_s,
            "brake_vehicle_speed_cmd": self._brake_vehicle_speed_cmd,
            "speed_to_mm_s": self._speed_to_mm_s,
            "host_sample_period_ms": HOST_SAMPLE_PERIOD_MS,
            "multi_pid": self._multi_pid,
        }

    def close(self):
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


class AccelBrakeExperimentController:
    def __init__(
        self,
        logger,
        send_target_speed,
        send_stop_direct_speed=None,
        reach_ratio=1.0,
        reach_stable_window_s=0.15,
        reach_stable_range_mm_s=150.0,
        plateau_window_s=0.35,
        plateau_min_ratio=0.60,
        plateau_min_speed_mm_s=300.0,
        plateau_stable_range_mm_s=180.0,
        plateau_stable_range_ratio=0.18,
        plateau_min_time_s=0.80,
        hold_after_reach_s=0.5,
        stop_speed_threshold=2.0,
        stop_hold_s=0.3,
        accel_timeout_s=8.0,
        brake_timeout_s=5.0,
        zero_lock_interval_s=0.1,
        telemetry_timeout_s=1.5,
        brake_target_tolerance_mm_s=10.0,
    ):
        self.logger = logger
        self.send_target_speed = send_target_speed
        self.send_stop_direct_speed = send_stop_direct_speed
        self.reach_ratio = float(reach_ratio)
        self.reach_stable_window_s = max(0.0, float(reach_stable_window_s))
        self.reach_stable_range_mm_s = max(0.0, float(reach_stable_range_mm_s))
        self.plateau_window_s = max(0.0, float(plateau_window_s))
        self.plateau_min_ratio = max(0.0, float(plateau_min_ratio))
        self.plateau_min_speed_mm_s = max(0.0, float(plateau_min_speed_mm_s))
        self.plateau_stable_range_mm_s = max(0.0, float(plateau_stable_range_mm_s))
        self.plateau_stable_range_ratio = max(0.0, float(plateau_stable_range_ratio))
        self.plateau_min_time_s = max(0.0, float(plateau_min_time_s))
        self.hold_after_reach_s = float(hold_after_reach_s)
        self.stop_speed_threshold = float(stop_speed_threshold)
        self.stop_hold_s = float(stop_hold_s)
        self.accel_timeout_s = float(accel_timeout_s)
        self.brake_timeout_s = float(brake_timeout_s)
        self.zero_lock_interval_s = max(0.0, float(zero_lock_interval_s))
        self.telemetry_timeout_s = max(0.0, float(telemetry_timeout_s))
        self.brake_target_tolerance_mm_s = max(0.0, float(brake_target_tolerance_mm_s))
        self._active = False
        self._phase = "idle"
        self._target_motion_speed_mm_s = 0.0
        self._target_forward_speed_mm_s = 0.0
        self._vehicle_speed_cmd = 0.0
        self._brake_target_speed_mm_s = 0.0
        self._brake_vehicle_speed_cmd = 0.0
        self._speed_to_mm_s = DEFAULT_SPEED_TO_MM_S
        self._multi_pid = False
        self._started_at = None
        self._last_frame_at = None
        self._reach_window = []
        self._hold_started_at = None
        self._brake_started_at = None
        self._last_zero_lock_at = None
        self._stopped_since = None
        self._last_error = ""
        self._last_finish = None
        self._last_reach_reason = ""

    def start(
        self,
        target_forward_speed_mm_s,
        multi_pid=False,
        now=None,
        speed_to_mm_s=DEFAULT_SPEED_TO_MM_S,
        brake_target_speed_mm_s=0.0,
        target_speed=None,
    ):
        if self._active:
            return {"success": False, "msg": "experiment already active"}

        now = time.time() if now is None else float(now)
        if target_speed is not None:
            target_forward_speed_mm_s = target_speed
        self._target_motion_speed_mm_s = float(target_forward_speed_mm_s)
        self._target_forward_speed_mm_s = abs(self._target_motion_speed_mm_s)
        self._speed_to_mm_s = float(speed_to_mm_s)
        self._vehicle_speed_cmd = signed_mm_s_to_vehicle_speed_cmd(
            self._target_motion_speed_mm_s,
            self._speed_to_mm_s,
        )
        self._brake_target_speed_mm_s = float(brake_target_speed_mm_s)
        self._brake_vehicle_speed_cmd = signed_mm_s_to_vehicle_speed_cmd(
            self._brake_target_speed_mm_s,
            self._speed_to_mm_s,
        )
        self._multi_pid = bool(multi_pid)
        self._started_at = now
        self._last_frame_at = now
        self._reach_window = []
        self._hold_started_at = None
        self._brake_started_at = None
        self._last_zero_lock_at = None
        self._stopped_since = None
        self._last_error = ""
        self._last_finish = None
        self._last_reach_reason = ""

        start_result = self.logger.start(
            self._target_motion_speed_mm_s,
            vehicle_speed_cmd=self._vehicle_speed_cmd,
            brake_target_speed_mm_s=self._brake_target_speed_mm_s,
            brake_vehicle_speed_cmd=self._brake_vehicle_speed_cmd,
            multi_pid=self._multi_pid,
            now=now,
            speed_to_mm_s=self._speed_to_mm_s,
        )
        if not start_result.get("success"):
            self._phase = "failed"
            self._last_error = start_result.get("msg", "logger start failed")
            return {"success": False, "msg": self._last_error}

        self._active = True
        self._phase = "accel"
        self.logger.set_phase("accel", now=now)
        send_result = self._send_target(self._vehicle_speed_cmd, self._pid_for_phase("accel"))
        if not send_result.get("success"):
            return self._finish("failed", now, send_result.get("msg", "target speed command failed"))

        return {"success": True, "phase": self._phase, "run": start_result}

    def feed_frame(self, frame, now=None):
        if not self._active:
            return {"finished": False, "active": False}

        now = time.time() if now is None else float(now)
        self._last_frame_at = now
        self.logger.record_frame(
            frame,
            now=now,
            commanded_vehicle_speed_cmd=self._commanded_speed_for_phase(),
            pid_mode=self._pid_for_phase(self._phase),
        )

        speed_abs = abs(safe_float(frame.get("vx_body"), 0.0))
        signed_motion_speed = -safe_float(frame.get("vx_body"), 0.0)
        target_direction = 1.0 if self._target_motion_speed_mm_s >= 0.0 else -1.0
        speed_along_target = signed_motion_speed * target_direction
        target_abs = self._target_forward_speed_mm_s

        safety_reason = self._frame_safety_stop_reason(frame)
        if safety_reason:
            return self._finish("safety_stop", now, safety_reason)

        if self._phase == "accel":
            if self._started_at is not None and now - self._started_at > self.accel_timeout_s:
                return self._finish("failed", now, "accel timeout")
            self._update_reach_window(now, speed_along_target)
            reach_reason = self._reach_ready_reason(now)
            if reach_reason:
                self._phase = "hold"
                self._last_reach_reason = reach_reason
                self._hold_started_at = now
                self.logger.set_phase("hold", now=now)
                send_result = self._send_target(self._vehicle_speed_cmd, self._pid_for_phase("hold"))
                if not send_result.get("success"):
                    return self._finish("failed", now, send_result.get("msg", "hold command failed"))

        elif self._phase == "hold":
            if (
                self._last_reach_reason != "plateau"
                and target_abs > 1e-6
                and speed_along_target < target_abs * self.reach_ratio
            ):
                self._phase = "accel"
                self._hold_started_at = None
                self._reach_window = []
                self.logger.set_phase("accel", now=now)
                send_result = self._send_target(self._vehicle_speed_cmd, self._pid_for_phase("accel"))
                if not send_result.get("success"):
                    return self._finish("failed", now, send_result.get("msg", "accel command failed"))
            elif self._hold_started_at is not None and now - self._hold_started_at >= self.hold_after_reach_s:
                return self._enter_brake_phase(now, reason="hold elapsed")

        elif self._phase == "brake":
            send_result = self._maybe_send_brake_target_lock(now)
            if send_result is not None and not send_result.get("success"):
                return self._finish("failed", now, send_result.get("msg", "brake target command failed"))
            if self._brake_started_at is not None and now - self._brake_started_at > self.brake_timeout_s:
                return self._finish("failed", now, "brake timeout")
            if self._brake_target_reached(speed_abs, signed_motion_speed):
                if self._stopped_since is None:
                    self._stopped_since = now
                elif now - self._stopped_since >= self.stop_hold_s:
                    return self._finish("completed", now)
            else:
                self._stopped_since = None

        return {"finished": False, "active": self._active, "phase": self._phase}

    def tick(self, now=None):
        if not self._active:
            return {"finished": False, "active": False, "phase": self._phase}

        now = time.time() if now is None else float(now)
        if (
            self.telemetry_timeout_s > 0.0
            and self._last_frame_at is not None
            and now - self._last_frame_at > self.telemetry_timeout_s
        ):
            return self._finish("failed", now, "telemetry timeout")

        if self._phase == "accel":
            if self._started_at is not None and now - self._started_at > self.accel_timeout_s:
                return self._finish("failed", now, "accel timeout")
        elif self._phase == "brake":
            send_result = self._maybe_send_brake_target_lock(now)
            if send_result is not None and not send_result.get("success"):
                return self._finish("failed", now, send_result.get("msg", "brake target command failed"))
            if self._brake_started_at is not None and now - self._brake_started_at > self.brake_timeout_s:
                return self._finish("failed", now, "brake timeout")

        return {"finished": False, "active": self._active, "phase": self._phase}

    def cancel(self, now=None):
        now = time.time() if now is None else float(now)
        if self._active:
            self._send_target(0.0, self._pid_for_phase("brake"))
            return self._finish("cancelled", now)
        return {"success": True, "finished": False, "active": False, "phase": self._phase}

    def force_brake(self, now=None):
        now = time.time() if now is None else float(now)
        if not self._active:
            return {"success": False, "finished": False, "active": False, "phase": self._phase, "msg": "no active run"}
        if self._phase == "brake":
            send_result = self._send_brake_target_lock(now)
            if not send_result.get("success"):
                return self._finish("failed", now, send_result.get("msg", "manual brake command failed"))
            return {"success": True, "finished": False, "active": True, "phase": self._phase, "msg": "brake command resent"}
        return self._enter_brake_phase(now, reason="manual")

    def shutdown(self, now=None):
        now = time.time() if now is None else float(now)
        if self._active:
            self._send_target(0.0, self._pid_for_phase("brake"))
            return self._finish("aborted", now, "host shutdown")
        if self.logger.recording:
            finish_result = self.logger.finish(status="aborted", now=now)
            self._phase = "aborted"
            self._last_finish = finish_result
            return {
                "success": finish_result.get("success", True),
                "finished": True,
                "active": False,
                "phase": self._phase,
                "status": "aborted",
                "error": "host shutdown",
                "result": finish_result,
            }
        return {"success": True, "finished": False, "active": False, "phase": self._phase}

    def _finish(self, status, now, error=""):
        stop_direct_result = None
        if status in ("failed", "cancelled", "safety_stop", "aborted") and self._active:
            self._send_target(0.0, self._pid_for_phase("brake"))
        elif status == "completed" and self._active and abs(self._brake_vehicle_speed_cmd) > 1e-6:
            self._send_target(0.0, self._pid_for_phase("brake"))
        if self._active and self.send_stop_direct_speed is not None:
            stop_direct_result = self.send_stop_direct_speed()
        finish_result = self.logger.finish(status=status, now=now) if self.logger.recording else {"success": True}
        self._active = False
        self._phase = "completed" if status == "completed" else status
        self._last_error = error
        self._last_finish = finish_result
        return {
            "success": finish_result.get("success", True),
            "finished": True,
            "active": False,
            "phase": self._phase,
            "status": status,
            "error": error,
            "result": finish_result,
            "stop_direct_result": stop_direct_result,
        }

    def _send_target(self, speed, pid_mode):
        return self.send_target_speed(float(speed), int(pid_mode)) or {"success": False, "msg": "empty send result"}

    def _update_reach_window(self, now, speed_abs):
        now = float(now)
        self._reach_window.append((now, float(speed_abs)))
        cutoff = now - max(self.reach_stable_window_s, self.plateau_window_s)
        self._reach_window = [(ts, speed) for ts, speed in self._reach_window if ts >= cutoff]

    def _enter_brake_phase(self, now, reason=""):
        self._phase = "brake"
        self._brake_started_at = float(now)
        self._stopped_since = None
        send_result = self._send_brake_target_lock(now)
        self.logger.set_phase("brake", now=now)
        if not send_result.get("success"):
            return self._finish("failed", now, send_result.get("msg", "brake command failed"))
        return {"success": True, "finished": False, "active": True, "phase": self._phase, "reason": reason}

    def _reach_ready_reason(self, now):
        if self._reach_speed_is_stable(now):
            return "target"
        if self._plateau_speed_is_stable(now):
            return "plateau"
        return ""

    def _window_speeds(self, now, window_s):
        now = float(now)
        cutoff = now - max(0.0, float(window_s))
        return [(ts, speed) for ts, speed in self._reach_window if cutoff <= ts <= now]

    def _reach_speed_is_stable(self, now):
        target_abs = self._target_forward_speed_mm_s
        if target_abs <= 1e-6:
            return True
        window = self._window_speeds(now, self.reach_stable_window_s)
        threshold = target_abs * self.reach_ratio
        if not window:
            return False
        if window[-1][1] < threshold:
            return False
        if self.reach_stable_window_s > 0.0 and float(now) - window[0][0] + 1e-9 < self.reach_stable_window_s:
            return False
        speeds = [speed for _ts, speed in window]
        if any(speed < threshold for speed in speeds):
            return False
        return max(speeds) - min(speeds) <= self.reach_stable_range_mm_s

    def _plateau_speed_is_stable(self, now):
        target_abs = self._target_forward_speed_mm_s
        if target_abs <= 1e-6 or self.plateau_window_s <= 0.0:
            return False
        if self._started_at is not None and float(now) - self._started_at < self.plateau_min_time_s:
            return False

        window = self._window_speeds(now, self.plateau_window_s)
        if not window or float(now) - window[0][0] + 1e-9 < self.plateau_window_s:
            return False

        speeds = [speed for _ts, speed in window]
        avg_speed = sum(speeds) / len(speeds)
        min_plateau_speed = max(self.plateau_min_speed_mm_s, target_abs * self.plateau_min_ratio)
        if avg_speed < min_plateau_speed:
            return False

        allowed_range = max(self.plateau_stable_range_mm_s, target_abs * self.plateau_stable_range_ratio)
        return max(speeds) - min(speeds) <= allowed_range

    def _send_brake_target_lock(self, now):
        self._last_zero_lock_at = float(now)
        return self._send_target(self._brake_vehicle_speed_cmd, self._pid_for_phase("brake"))

    def _maybe_send_brake_target_lock(self, now):
        if self.zero_lock_interval_s <= 0.0:
            return None
        if self._last_zero_lock_at is None or float(now) - self._last_zero_lock_at >= self.zero_lock_interval_s:
            return self._send_brake_target_lock(now)
        return None

    def _brake_target_reached(self, speed_abs, signed_forward_speed):
        if abs(self._brake_target_speed_mm_s) <= 1e-6:
            return speed_abs <= self.stop_speed_threshold
        if self._target_motion_speed_mm_s >= self._brake_target_speed_mm_s:
            return signed_forward_speed <= self._brake_target_speed_mm_s + self.brake_target_tolerance_mm_s
        return signed_forward_speed >= self._brake_target_speed_mm_s - self.brake_target_tolerance_mm_s

    def _frame_safety_stop_reason(self, frame):
        motor_enable = safe_float(frame.get("motor_enable"))
        fallen = safe_float(frame.get("fallen"))
        remote_brake = safe_float(frame.get("remote_brake_active"))
        remote_reverse_brake = safe_float(frame.get("remote_reverse_brake_active"))

        if motor_enable is not None and motor_enable <= 0.5:
            return "motor disabled"
        if fallen is not None and fallen > 0.5:
            return "fallen"
        if remote_brake is not None and remote_brake > 0.5:
            return "remote CH5 brake"
        if remote_reverse_brake is not None and remote_reverse_brake > 0.5:
            return "remote reverse brake"
        return ""

    def _pid_for_phase(self, phase):
        if not self._multi_pid:
            return CONTROL_MODE_NORMAL
        if phase == "accel":
            return CONTROL_MODE_ACCEL
        if phase == "brake":
            return CONTROL_MODE_BRAKE
        return CONTROL_MODE_NORMAL

    def _commanded_speed_for_phase(self):
        if self._phase == "brake":
            return self._brake_vehicle_speed_cmd
        return self._vehicle_speed_cmd

    def status(self):
        status = self.logger.status()
        status.update(
            {
                "active": self._active,
                "phase": self._phase,
                "target_motion_speed_mm_s": self._target_motion_speed_mm_s,
                "target_forward_speed_mm_s": self._target_forward_speed_mm_s,
                "vehicle_speed_cmd": self._vehicle_speed_cmd,
                "brake_target_speed_mm_s": self._brake_target_speed_mm_s,
                "brake_vehicle_speed_cmd": self._brake_vehicle_speed_cmd,
                "speed_to_mm_s": self._speed_to_mm_s,
                "multi_pid": self._multi_pid,
                "reach_ratio": self.reach_ratio,
                "reach_stable_window_s": self.reach_stable_window_s,
                "reach_stable_range_mm_s": self.reach_stable_range_mm_s,
                "plateau_window_s": self.plateau_window_s,
                "plateau_min_ratio": self.plateau_min_ratio,
                "plateau_stable_range_mm_s": self.plateau_stable_range_mm_s,
                "reach_reason": self._last_reach_reason,
                "last_error": self._last_error,
                "last_finish": self._last_finish,
            }
        )
        return status
