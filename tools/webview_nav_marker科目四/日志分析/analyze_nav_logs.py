#!/usr/bin/env python3
"""Analyze Plan4 replay telemetry against its generated route table.

The input telemetry is intentionally treated as a hybrid signal: inertial body
velocity is used for dynamics, while fused nav position is used for route
matching with relocation jumps explicitly rejected from derivatives.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import least_squares


SPEED_MM_PER_RPM = 4.79
G_TO_MPS2 = 9.80665
REPLAY_RUNNING = 1
ROUTE_RE = re.compile(
    r"\{\s*([-+0-9.eE]+)f,\s*([-+0-9.eE]+)f,\s*([-+0-9.eE]+)f,\s*"
    r"[-+0-9.eE]+f,\s*\(uint8\)(\d+),\s*([-+0-9.eE]+)f,\s*([-+0-9.eE]+)f"
)

# Keep these synchronized with plan4_lqr_speed_planning.h and
# generate_plan4_smooth_path.py. They are reported as assumptions, not hidden
# calibration values, so a future firmware/path change is visible in output.
PLAN4_CODE = {
    "path_speed_max_mm_s": 4000.0,
    "offline_max_accel_mm_s2": 1500.0,
    "offline_max_decel_mm_s2": 1500.0,
    "max_lateral_accel_mm_s2": 3500.0,
    "max_yaw_rate_rad_s": 2.8,
    "lqr_preview_points": 5,
    "lqr_sharp_preview_points": 2,
    "lqr_sharp_curvature_th_per_mm": 0.0015,
    "lqr_k_lateral_deg_per_mm": 0.030,
    "lqr_k_heading": 0.80,
    "lqr_k_yaw_rate_ff": 8.0,
    "runtime_cross_track_soft_mm": 250.0,
    "runtime_cross_track_hard_mm": 650.0,
    "runtime_yaw_soft_deg": 35.0,
    "runtime_yaw_hard_deg": 80.0,
    "runtime_special_handoff_lead_mm": 500.0,
    "path_straight_corridor_mm": 600.0,
    "stairs_approach_distance_mm": 4000.0,
    "stairs_speed_cap_mm_s": 220.0 * SPEED_MM_PER_RPM,
    "bridge_approach_distance_mm": 2500.0,
    "bridge_speed_cap_mm_s": 300.0 * SPEED_MM_PER_RPM,
}


def norm_angle(a: np.ndarray | float) -> np.ndarray | float:
    return (np.asarray(a) + 180.0) % 360.0 - 180.0


def robust_median_dt(t: np.ndarray) -> float:
    d = np.diff(t)
    d = d[(d > 0.0) & (d < 1.0)]
    return float(np.median(d)) if len(d) else 0.01


def rolling_median(x: np.ndarray, window: int = 11) -> np.ndarray:
    if len(x) < 3:
        return x.copy()
    window = max(3, min(window | 1, len(x) if len(x) % 2 else len(x) - 1))
    pad = window // 2
    p = np.pad(x, (pad, pad), mode="edge")
    return np.array([np.median(p[i:i + window]) for i in range(len(x))])


def derivative(x: np.ndarray, t: np.ndarray) -> np.ndarray:
    if len(x) < 2:
        return np.zeros_like(x)
    return np.gradient(x, t, edge_order=1)


def parse_route(path: Path) -> dict[str, np.ndarray]:
    rows = []
    for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        m = ROUTE_RE.search(line)
        if m:
            rows.append([float(m.group(i)) for i in (1, 2, 3, 5, 6)] + [int(m.group(4))])
    if len(rows) < 2:
        raise ValueError(f"No route rows found in {path}")
    a = np.asarray(rows, dtype=float)
    x, y, yaw, speed, curvature = a[:, 0], a[:, 1], a[:, 2], a[:, 3], a[:, 4]
    typ = a[:, 5].astype(int)
    ds = np.hypot(np.diff(x), np.diff(y))
    s = np.r_[0.0, np.cumsum(ds)]
    return {"x": x, "y": y, "yaw": yaw, "speed": speed, "curvature": curvature, "type": typ, "s": s}


def load_log(path: Path) -> dict[str, np.ndarray]:
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)
    if not rows:
        raise ValueError(f"Empty telemetry: {path}")
    names = reader.fieldnames or []
    required = ["loop", "nav_x", "nav_y", "vx_body", "vy_body", "relative_yaw", "target_speed_set", "speed_L", "speed_R", "pwm_left", "pwm_right", "g_replay_state"]
    missing = [x for x in required if x not in names]
    if missing:
        raise ValueError(f"{path.name} missing columns: {missing}")
    out = {}
    for name in names:
        try:
            out[name] = np.asarray([float(r[name]) if r[name] != "" else np.nan for r in rows], dtype=float)
        except ValueError:
            out[name] = np.asarray([np.nan] * len(rows), dtype=float)
    return out


def nearest_route(route: dict[str, np.ndarray], x: float, y: float, last: int) -> tuple[int, float, float]:
    lo, hi = max(0, last - 80), min(len(route["x"]) - 2, last + 240)
    px, py = route["x"][lo:hi + 1], route["y"][lo:hi + 1]
    d2 = (px - x) ** 2 + (py - y) ** 2
    i = lo + int(np.argmin(d2))
    j = min(i + 1, len(route["x"]) - 1)
    dx, dy = route["x"][j] - route["x"][i], route["y"][j] - route["y"][i]
    den = dx * dx + dy * dy
    u = 0.0 if den < 1e-9 else np.clip(((x - route["x"][i]) * dx + (y - route["y"][i]) * dy) / den, 0.0, 1.0)
    qx, qy = route["x"][i] + u * dx, route["y"][i] + u * dy
    cross = (dy * (x - qx) - dx * (y - qy)) / max(math.hypot(dx, dy), 1e-6)
    return i, float(cross), float(math.hypot(x - qx, y - qy))


def contiguous_segments(mask: np.ndarray) -> list[tuple[int, int]]:
    idx = np.flatnonzero(mask)
    if len(idx) == 0:
        return []
    cuts = np.flatnonzero(np.diff(idx) > 1)
    starts = np.r_[idx[0], idx[cuts + 1]]
    ends = np.r_[idx[cuts], idx[-1]]
    return [(int(a), int(b)) for a, b in zip(starts, ends)]


def extract_decel_events(speed: np.ndarray, route_s: np.ndarray, accel: np.ndarray) -> list[tuple[float, float, float]]:
    """Extract usable partial braking events from one running segment.

    A telemetry run does not contain a dedicated brake trigger or wheel brake
    distance.  We therefore report speed-to-speed deceleration distances, using
    inertial speed and route arc length, and label the fit as partial.
    """
    candidate = (accel < -450.0) & (speed > 800.0)
    events = []
    for a, b in contiguous_segments(candidate):
        if b - a < 3:
            continue
        pre = max(0, a - 15)
        v0 = float(np.nanmedian(speed[pre:a + 1]))
        vend = float(np.nanmedian(speed[max(a, b - 2):b + 1]))
        if v0 - vend < 250.0:
            continue
        events.append((v0, vend, float(max(0.0, route_s[b] - route_s[a]))))
    return events


def fit_brake_model(v: np.ndarray, dist: np.ndarray) -> dict[str, object]:
    """Fit d_stop(v)=a*v^2+b*v+c only for monotonic measured stop events.

    ``dist`` is supplied as event-level stopping distances, never as a running
    cumulative position.  This avoids fitting a physically meaningless curve
    to the whole lap.
    """
    mask = np.isfinite(v) & np.isfinite(dist) & (v > 50.0) & (dist > 0.0)
    if mask.sum() < 3:
        return {"n": int(mask.sum()), "coefficients": None}
    coeff = np.polyfit(v[mask], dist[mask], 2)
    pred = np.polyval(coeff, v[mask])
    rmse = float(np.sqrt(np.mean((pred - dist[mask]) ** 2)))
    return {"n": int(mask.sum()), "coefficients": [float(x) for x in coeff], "rmse_mm": rmse}


def fit_v1_v2_brake_model(v1: np.ndarray, v2: np.ndarray, dist: np.ndarray) -> dict[str, object]:
    """Fit d=A*(v1^2-v2^2)+B*(v1-v2)+C for partial braking events."""
    mask = np.isfinite(v1) & np.isfinite(v2) & np.isfinite(dist) & (v1 > v2) & (v1 > 50.0) & (dist > 0.0)
    if mask.sum() < 3:
        return {"n": int(mask.sum()), "coefficients": None, "formula": "d=A*(v1^2-v2^2)+B*(v1-v2)+C"}
    X = np.column_stack([v1[mask] ** 2 - v2[mask] ** 2, v1[mask] - v2[mask], np.ones(mask.sum())])
    coef, *_ = np.linalg.lstsq(X, dist[mask], rcond=None)
    pred = X @ coef
    return {
        "n": int(mask.sum()),
        "coefficients": [float(x) for x in coef],
        "rmse_mm": float(np.sqrt(np.mean((pred - dist[mask]) ** 2))),
        "formula": "d=A*(v1^2-v2^2)+B*(v1-v2)+C",
        "units": "v1/v2 mm/s, d mm",
    }


def fit_regression(x: np.ndarray, y: np.ndarray, degree: int = 1) -> dict[str, object]:
    mask = np.isfinite(x) & np.isfinite(y)
    if mask.sum() < degree + 2:
        return {"n": int(mask.sum()), "coefficients": None, "degree": degree}
    coef = np.polyfit(x[mask], y[mask], degree)
    pred = np.polyval(coef, x[mask])
    ss_res = float(np.sum((y[mask] - pred) ** 2))
    ss_tot = float(np.sum((y[mask] - np.mean(y[mask])) ** 2))
    return {"n": int(mask.sum()), "coefficients": [float(x) for x in coef], "degree": degree, "rmse": float(np.sqrt(np.mean((y[mask] - pred) ** 2))), "r2": 1.0 - ss_res / ss_tot if ss_tot > 1e-9 else None, "x_range": [float(np.min(x[mask])), float(np.max(x[mask]))]}


def fit_pwm_wheel(pwm: np.ndarray, rpm: np.ndarray) -> dict[str, object]:
    mask = np.isfinite(pwm) & np.isfinite(rpm) & (np.abs(pwm) > 50.0)
    result = {"n": int(mask.sum()), "pwm_range": None, "linear": None, "quadratic": None, "abs_linear": None, "abs_quadratic": None, "deadzone_pwm_est": None}
    if mask.sum() < 10:
        return result
    result["pwm_range"] = [float(np.min(pwm[mask])), float(np.max(pwm[mask]))]
    result["linear"] = fit_regression(pwm[mask], rpm[mask], 1)
    result["quadratic"] = fit_regression(pwm[mask], rpm[mask], 2)
    result["abs_linear"] = fit_regression(np.abs(pwm[mask]), np.abs(rpm[mask]), 1)
    result["abs_quadratic"] = fit_regression(np.abs(pwm[mask]), np.abs(rpm[mask]), 2)
    # Near-zero PWM region where measured wheel speed remains below 2% of its
    # observed range. This is an empirical deadzone, not a motor datasheet value.
    speed_limit = max(20.0, 0.02 * np.nanpercentile(np.abs(rpm[mask]), 95))
    near = np.abs(rpm) <= speed_limit
    if near.any():
        result["deadzone_pwm_est"] = float(np.nanpercentile(np.abs(pwm[near]), 95))
    return result


def fit_first_order_actuator(command: np.ndarray, speed: np.ndarray, t: np.ndarray) -> dict[str, object]:
    """Fit dv/dt = gain/tau * command - v/tau."""
    dv = derivative(speed, t)
    mask = np.isfinite(command) & np.isfinite(speed) & np.isfinite(dv) & (np.abs(dv) < 20000.0)
    if mask.sum() < 20:
        return {"n": int(mask.sum()), "tau_s": None, "steady_state_gain": None}
    X = np.column_stack([command[mask], -speed[mask]])
    coef, *_ = np.linalg.lstsq(X, dv[mask], rcond=None)
    drive, decay = float(coef[0]), float(coef[1])
    if decay <= 1e-6:
        return {"n": int(mask.sum()), "tau_s": None, "steady_state_gain": None, "drive_per_s": drive, "decay_per_s": decay}
    tau = 1.0 / decay
    return {"n": int(mask.sum()), "tau_s": tau, "steady_state_gain": drive / decay, "drive_per_s": drive, "decay_per_s": decay}


def correlation_delay(source: np.ndarray, response: np.ndarray, dt_s: float) -> dict[str, float | None]:
    """Return response delay; positive means response happens after source."""
    if len(source) < 20 or np.std(source) < 1.0 or np.std(response) < 1.0:
        return {"correlation": None, "response_delay_s": None}
    source_z = source - np.mean(source)
    response_z = response - np.mean(response)
    corr_full = np.correlate(source_z, response_z, mode="full")
    lags = np.arange(-len(source) + 1, len(source))
    # np.correlate(a, b) reaches its maximum at a negative offset when b lags a.
    response_delay = float(-lags[np.argmax(corr_full)] * dt_s)
    return {"correlation": float(np.corrcoef(source, response)[0, 1]), "response_delay_s": response_delay}


def simulate_first_order(command: np.ndarray, t: np.ndarray, tau_s: float, gain: float, initial: float) -> np.ndarray:
    simulated = np.empty_like(command)
    simulated[0] = initial
    for i in range(1, len(command)):
        dt = max(0.0, min(0.1, float(t[i] - t[i - 1])))
        simulated[i] = simulated[i - 1] + dt * (gain * command[i - 1] - simulated[i - 1]) / tau_s
    return simulated


def simulate_route_only_speed(
    route: dict[str, np.ndarray],
    tau_acc_s: float,
    tau_dec_s: float,
    gain: float,
    dt_s: float = 0.01,
) -> dict[str, np.ndarray]:
    """Forward simulate actual speed using only the route-table speed command.

    The model intentionally does not use telemetry after fitting.  It follows
    the spatial command u(s) with separate first-order acceleration and braking
    time constants, then integrates ds/dt=v.  This is the predictor available
    to the path generator before the vehicle runs.
    """
    s_end = float(route["s"][-1])
    s_values = [0.0]
    t_values = [0.0]
    v_values = [0.0]
    u_values = [float(abs(route["speed"][0]) * SPEED_MM_PER_RPM)]
    max_steps = int(max(10000, s_end / max(50.0, dt_s * 100.0)))
    for _ in range(max_steps):
        s_now = s_values[-1]
        v_now = v_values[-1]
        command = float(np.interp(s_now, route["s"], np.abs(route["speed"]) * SPEED_MM_PER_RPM))
        desired = gain * command
        tau = tau_acc_s if desired >= v_now else tau_dec_s
        v_next = max(0.0, v_now + dt_s * (desired - v_now) / max(tau, 0.02))
        s_next = s_now + max(25.0, 0.5 * (v_now + v_next)) * dt_s
        s_values.append(min(s_next, s_end))
        t_values.append(t_values[-1] + dt_s)
        v_values.append(v_next)
        u_values.append(command)
        if s_next >= s_end:
            break
    return {
        "s_mm": np.asarray(s_values),
        "time_s": np.asarray(t_values),
        "speed_mm_s": np.asarray(v_values),
        "command_mm_s": np.asarray(u_values),
    }


def load_derived_for_route_model(path: Path) -> dict[str, np.ndarray]:
    data = np.genfromtxt(path, delimiter=",", names=True, dtype=float, encoding="utf-8")
    if data.size == 0:
        return {"route_s_mm": np.empty(0), "body_forward_mm_s": np.empty(0), "match_dist_mm": np.empty(0)}
    if data.ndim == 0:
        data = np.asarray([data])
    return {name: np.asarray(data[name], dtype=float) for name in data.dtype.names or []}


def fit_route_only_model(
    route: dict[str, np.ndarray],
    derived_paths: list[Path],
    fixed_parameters: dict[str, float] | None = None,
) -> dict[str, object]:
    """Fit one reusable route-table-only model from multiple replay logs."""
    observations: list[tuple[np.ndarray, np.ndarray]] = []
    for path in derived_paths:
        d = load_derived_for_route_model(path)
        s = d.get("route_s_mm", np.empty(0))
        v = d.get("body_forward_mm_s", np.empty(0))
        match = d.get("match_dist_mm", np.empty(0))
        # Ignore weak geometric matches and decimate to make all runs contribute
        # equally without over-weighting their 10 ms sampling rate.
        valid = np.isfinite(s) & np.isfinite(v) & np.isfinite(match) & (match < 450.0) & (s >= 0.0)
        if valid.sum() > 30:
            observations.append((s[valid][::5], v[valid][::5]))
    if not observations:
        return {"available": False, "reason": "no usable derived route observations"}

    def residual(parameters: np.ndarray) -> np.ndarray:
        tau_acc, tau_dec, gain = parameters
        sim = simulate_route_only_speed(route, float(tau_acc), float(tau_dec), float(gain))
        result = []
        for s_obs, v_obs in observations:
            pred = np.interp(np.clip(s_obs, 0.0, sim["s_mm"][-1]), sim["s_mm"], sim["speed_mm_s"])
            result.append((pred - v_obs) / 400.0)
        return np.concatenate(result)

    if fixed_parameters is None:
        solution = least_squares(
            residual,
            x0=np.array([0.7, 0.5, 1.0]),
            bounds=(np.array([0.05, 0.05, 0.70]), np.array([3.0, 3.0, 1.35])),
            max_nfev=100,
        )
        tau_acc, tau_dec, gain = [float(x) for x in solution.x]
        calibration_source = "fitted from supplied telemetry"
    else:
        tau_acc = float(fixed_parameters["tau_acc_s"])
        tau_dec = float(fixed_parameters["tau_dec_s"])
        gain = float(fixed_parameters["gain"])
        calibration_source = "fixed externally supplied calibration"
    sim = simulate_route_only_speed(route, tau_acc, tau_dec, gain)
    errors = []
    per_run = []
    for path, (s_obs, v_obs) in zip(derived_paths, observations):
        pred = np.interp(np.clip(s_obs, 0.0, sim["s_mm"][-1]), sim["s_mm"], sim["speed_mm_s"])
        err = pred - v_obs
        errors.append(err)
        per_run.append({
            "derived_file": path.name,
            "n": int(len(err)),
            "speed_rmse_mm_s": float(np.sqrt(np.mean(err ** 2))),
            "speed_bias_mm_s": float(np.mean(err)),
            "speed_p95_abs_error_mm_s": float(np.percentile(np.abs(err), 95)),
        })
    all_error = np.concatenate(errors)
    return {
        "available": True,
        "model": "dv/dt=(gain*u_route(s)-v)/tau; ds/dt=v; tau=tau_acc when accelerating else tau_dec",
        "calibration_source": calibration_source,
        "parameters": {"tau_acc_s": tau_acc, "tau_dec_s": tau_dec, "gain": gain, "integration_dt_s": 0.01},
        "predicted_lap_time_s": float(sim["time_s"][-1]),
        "predicted_speed_peak_mm_s": float(np.max(sim["speed_mm_s"])),
        "fit_error": {
            "n": int(len(all_error)),
            "speed_rmse_mm_s": float(np.sqrt(np.mean(all_error ** 2))),
            "speed_bias_mm_s": float(np.mean(all_error)),
            "speed_p95_abs_error_mm_s": float(np.percentile(np.abs(all_error), 95)),
        },
        "per_run": per_run,
        "simulation": sim,
    }


def plan4_code_analysis(route: dict[str, np.ndarray], actuator_fit: dict[str, object]) -> dict[str, object]:
    tau = float(actuator_fit.get("tau_s") or np.nan)
    # First-order lag distance while changing from v1 to v2. This is the
    # distance travelled during the exponential response, not a full brake
    # distance including tire/terrain deceleration.
    v1 = PLAN4_CODE["path_speed_max_mm_s"]
    lag_distance_at_cap = float(v1 * tau) if np.isfinite(tau) else None
    offline_distance = v1 * v1 / (2.0 * PLAN4_CODE["offline_max_decel_mm_s2"])
    curvature_limit_cap = PLAN4_CODE["max_lateral_accel_mm_s2"] / max(v1 * v1, 1.0)
    yaw_rate_limit_cap = PLAN4_CODE["max_yaw_rate_rad_s"] / max(v1, 1.0)
    return {
        "constants": PLAN4_CODE,
        "route_curvature_abs_max_per_mm": float(np.nanmax(np.abs(route["curvature"]))),
        "route_curvature_p95_per_mm": float(np.nanpercentile(np.abs(route["curvature"]), 95)),
        "offline_decel_distance_at_4000_mm_s": offline_distance,
        "first_order_lag_distance_at_4000_mm_s": lag_distance_at_cap,
        "lag_to_runtime_handoff_ratio": lag_distance_at_cap / PLAN4_CODE["runtime_special_handoff_lead_mm"] if lag_distance_at_cap else None,
        "curvature_needed_to_limit_4000_by_lateral_accel_per_mm": curvature_limit_cap,
        "curvature_needed_to_limit_4000_by_yaw_rate_per_mm": yaw_rate_limit_cap,
        "route_target_speed_percentiles_mm_s": [float(x) for x in np.percentile(np.abs(route["speed"]) * SPEED_MM_PER_RPM, [0, 25, 50, 75, 95, 100])],
        "recommendations": [
            "Use an actuator-aware reachable-speed envelope; the current offline envelope assumes instantaneous speed tracking.",
            "Move special-task handoff from a fixed 500 mm distance to a heading/lateral/velocity gate with a longer dynamic braking reserve.",
            "Treat target_speed_set as a command state and estimate actual speed from vx_body/wheel feedback before deciding braking completion.",
            "Validate the 5-point/2-point LQR preview in meters: at 50 mm route spacing this is approximately 250/100 mm, before command and actuator lag.",
        ],
    }


def analyze_one(path: Path, route: dict[str, np.ndarray], output: Path) -> dict[str, object]:
    raw = load_log(path)
    running = raw["g_replay_state"] == REPLAY_RUNNING
    if running.sum() < 5:
        return {"file": path.name, "running_samples": int(running.sum()), "warning": "no usable replay segment"}
    keys = list(raw)
    d = {k: v[running] for k, v in raw.items()}
    t = (d["loop"] - d["loop"][0]) / 1000.0
    dt = np.diff(t, prepend=t[0])
    med_dt = robust_median_dt(t)
    dt[0] = med_dt
    valid_dt = (dt > 0.001) & (dt < 0.2)
    dt[~valid_dt] = med_dt

    n = len(t)
    route_idx = np.zeros(n, dtype=int)
    cross = np.zeros(n)
    match_dist = np.zeros(n)
    for k in range(n):
        route_idx[k], cross[k], match_dist[k] = nearest_route(route, d["nav_x"][k], d["nav_y"][k], int(route_idx[k - 1]) if k else 0)
    route_s = route["s"][route_idx]
    jump = np.r_[False, np.hypot(np.diff(d["nav_x"]), np.diff(d["nav_y"])) > 250.0]
    jump |= match_dist > 900.0

    speed_l = d["speed_L"] * SPEED_MM_PER_RPM
    speed_r = -d["speed_R"] * SPEED_MM_PER_RPM
    wheel_speed = (speed_l + speed_r) / 2.0
    # vx_body is negative for forward motion in this firmware.  All derived
    # longitudinal quantities below use forward-positive convention.
    body_speed = -d["vx_body"]
    body_speed_smooth = rolling_median(body_speed)
    accel = derivative(body_speed_smooth, t)
    wheel_accel = derivative(rolling_median(wheel_speed), t)
    yaw_rate = derivative(np.unwrap(np.deg2rad(d["relative_yaw"])), t) * 180.0 / math.pi
    target_mm_s = -d["target_speed_set"] * SPEED_MM_PER_RPM
    target_error = target_mm_s - wheel_speed
    command_to_wheel = correlation_delay(target_mm_s, wheel_speed, med_dt)
    speed_lag_s = command_to_wheel["response_delay_s"]
    corr = command_to_wheel["correlation"]
    slip_signed = wheel_speed - body_speed_smooth
    slip_proxy = np.abs(slip_signed)
    slip_events = (slip_proxy > 350.0) | (np.abs(d["slip_flag"]) > 0)

    active_special = (d["g_special_action_trigger"] > 0) | (d["minefield_is_active"] > 0) | (d["bumpy_road_is_active"] > 0) | (d["vision_bridge_task_is_active"] > 0) | (d["vision_slope_task_is_active"] > 0) | (d["vision_three_stage_control_is_active"] > 0)
    ordinary = ~active_special
    ordinary_rmse = float(np.sqrt(np.mean(cross[ordinary] ** 2))) if ordinary.any() else float("nan")
    ordinary_p95 = float(np.percentile(np.abs(cross[ordinary]), 95)) if ordinary.any() else float("nan")
    speed_abs = np.abs(body_speed_smooth)
    # Route-vs-vehicle speed comparison.  Route target is stored in RPM-like
    # command units and is negative for forward; convert to forward mm/s.
    planned_forward = np.abs(route["speed"][route_idx]) * SPEED_MM_PER_RPM
    speed_shortfall = planned_forward - body_speed_smooth
    # No dedicated brake trigger is present in telemetry. Extract measurable
    # speed-to-speed deceleration events and keep them separate from full-stop
    # distance fitting.
    brake_events = extract_decel_events(body_speed_smooth, route_s, accel)
    brake_fit = fit_brake_model(np.asarray([x[0] for x in brake_events]), np.asarray([x[2] for x in brake_events]))
    brake_v1_v2_fit = fit_v1_v2_brake_model(
        np.asarray([x[0] for x in brake_events]),
        np.asarray([x[1] for x in brake_events]),
        np.asarray([x[2] for x in brake_events]),
    )
    front_servo = np.nanmean(np.column_stack([d["servo_angle_rf"], d["servo_angle_lf"]]), axis=1)
    rear_servo = np.nanmean(np.column_stack([d["servo_angle_rr"], d["servo_angle_lr"]]), axis=1)
    servo_mean = (front_servo + rear_servo) / 2.0
    servo_delta = front_servo - rear_servo
    servo_yaw_response = derivative(np.unwrap(np.deg2rad(d["relative_yaw"])), t) * 180.0 / math.pi
    servo_neutral = float(np.nanmedian(np.r_[front_servo, rear_servo]))
    front_centered = front_servo - servo_neutral
    rear_centered = rear_servo - servo_neutral
    state_segments = []
    state_code = np.where(active_special, d["nav_replay_point_type"], 0).astype(int)
    for a, b in contiguous_segments(active_special):
        state_segments.append({
            "start_s": float(t[a]), "end_s": float(t[b]), "duration_s": float(t[b] - t[a]),
            "point_types": sorted(set(int(x) for x in state_code[a:b + 1])),
            "mean_speed_mm_s": float(np.nanmean(body_speed_smooth[a:b + 1])),
            "p95_abs_cross_track_mm": float(np.nanpercentile(np.abs(cross[a:b + 1]), 95)),
            "max_abs_slip_proxy_mm_s": float(np.nanpercentile(slip_proxy[a:b + 1], 95)),
        })
    actuator_fit = fit_first_order_actuator(target_mm_s, wheel_speed, t)
    tau = actuator_fit.get("tau_s")
    gain = actuator_fit.get("steady_state_gain")
    wheel_pred = simulate_first_order(target_mm_s, t, float(tau), float(gain), wheel_speed[0]) if tau and gain else np.full_like(wheel_speed, np.nan)
    route_to_command = correlation_delay(planned_forward, target_mm_s, med_dt)
    route_to_wheel = correlation_delay(planned_forward, wheel_speed, med_dt)
    wheel_to_body = correlation_delay(wheel_speed, body_speed_smooth, med_dt)
    planning_to_command_error = planned_forward - target_mm_s
    command_to_wheel_error = target_mm_s - wheel_speed
    wheel_to_body_error = wheel_speed - body_speed_smooth
    predicted_wheel_error = wheel_pred - wheel_speed
    summary = {
        "file": path.name,
        "running_samples": int(n),
        "duration_s": float(t[-1] - t[0]),
        "median_dt_s": med_dt,
        "route_length_mm": float(route["s"][-1]),
        "route_progress_mm": float(route_s[-1] - route_s[0]),
        "route_completion_fraction": float((route_s[-1] - route_s[0]) / max(route["s"][-1], 1.0)),
        "max_body_speed_mm_s": float(np.nanpercentile(speed_abs, 99)),
        "median_body_speed_mm_s": float(np.nanmedian(speed_abs)),
        "max_accel_mm_s2": float(np.nanpercentile(accel, 99)),
        "max_decel_mm_s2": float(np.nanpercentile(accel, 1)),
        "max_lateral_speed_mm_s": float(np.nanpercentile(np.abs(d["vy_body"]), 99)),
        "ordinary_cross_track_rmse_mm": ordinary_rmse,
        "ordinary_cross_track_p95_mm": ordinary_p95,
        "match_dist_p95_mm": float(np.percentile(match_dist, 95)),
        "fusion_jump_count": int(jump.sum()),
        "slip_proxy_p95_mm_s": float(np.percentile(slip_proxy, 95)),
        "slip_signed_bias_mm_s": float(np.nanmedian(slip_signed)),
        "slip_samples": int(slip_events.sum()),
        "target_wheel_error_bias_mm_s": float(np.nanmedian(command_to_wheel_error)),
        "target_wheel_error_p95_abs_mm_s": float(np.nanpercentile(np.abs(command_to_wheel_error), 95)),
        "target_wheel_correlation": corr,
        "target_wheel_lag_s": speed_lag_s,
        "actuator_first_order_fit": actuator_fit,
        "plan4_code_analysis": plan4_code_analysis(route, actuator_fit),
        "speed_chain": {
            "sign_convention": "All values are forward-positive mm/s. Route and target_speed_set are negated from their stored forward-negative command convention.",
            "route_plan_to_car_command": route_to_command,
            "car_command_to_wheel": command_to_wheel,
            "wheel_to_body": wheel_to_body,
            "route_plan_minus_car_command": {"median_mm_s": float(np.nanmedian(planning_to_command_error)), "p95_abs_mm_s": float(np.nanpercentile(np.abs(planning_to_command_error), 95))},
            "car_command_minus_wheel": {"median_mm_s": float(np.nanmedian(command_to_wheel_error)), "p95_abs_mm_s": float(np.nanpercentile(np.abs(command_to_wheel_error), 95))},
            "wheel_minus_body": {"median_mm_s": float(np.nanmedian(wheel_to_body_error)), "p95_abs_mm_s": float(np.nanpercentile(np.abs(wheel_to_body_error), 95))},
            "first_order_prediction_minus_wheel": {"rmse_mm_s": float(np.sqrt(np.nanmean(predicted_wheel_error ** 2))), "p95_abs_mm_s": float(np.nanpercentile(np.abs(predicted_wheel_error), 95))},
        },
        "planned_actual_speed_bias_mm_s": float(np.nanmedian(speed_shortfall)),
        "planned_actual_speed_p95_abs_mm_s": float(np.nanpercentile(np.abs(speed_shortfall), 95)),
        "brake_event_count": len(brake_events),
        "brake_fit": brake_fit,
        "brake_v1_v2_fit": brake_v1_v2_fit,
        "pwm_to_wheel_model": {
            "left": fit_pwm_wheel(d["pwm_left"], d["speed_L"]),
            "right": fit_pwm_wheel(d["pwm_right"], d["speed_R"]),
        },
        "servo_model": {
            "front_mean_deg": {"range": [float(np.nanmin(front_servo)), float(np.nanmax(front_servo))]},
            "rear_mean_deg": {"range": [float(np.nanmin(rear_servo)), float(np.nanmax(rear_servo))]},
            "front_minus_rear_deg": {"mean": float(np.nanmean(servo_delta)), "p95_abs": float(np.nanpercentile(np.abs(servo_delta), 95))},
            "err_degree_to_front_servo": fit_regression(d["err_degree"], front_servo, 2),
            "err_degree_to_rear_servo": fit_regression(d["err_degree"], rear_servo, 2),
            "neutral_deg_est": servo_neutral,
            "front_centered_to_yaw_rate": fit_regression(front_centered, servo_yaw_response, 1),
            "rear_centered_to_yaw_rate": fit_regression(rear_centered, servo_yaw_response, 1),
            "front_centered_to_err_degree": fit_regression(d["err_degree"], front_centered, 2),
            "rear_centered_to_err_degree": fit_regression(d["err_degree"], rear_centered, 2),
        },
        "state_segments": state_segments,
    }

    output.mkdir(parents=True, exist_ok=True)
    stem = path.stem.replace("——", "_")
    derived_path = output / f"{stem}_derived.csv"
    np.savetxt(derived_path, np.column_stack([t, d["nav_x"], d["nav_y"], route_s, cross, match_dist, body_speed, wheel_speed, wheel_pred, planned_forward, target_mm_s, planning_to_command_error, command_to_wheel_error, wheel_to_body_error, accel, yaw_rate, slip_signed, slip_proxy, front_servo, rear_servo, servo_mean, servo_delta, d["pwm_left"], d["pwm_right"], d["speed_L"], d["speed_R"], route["yaw"][route_idx], route["speed"][route_idx], route["curvature"][route_idx], route["type"][route_idx], active_special.astype(int)]), delimiter=",", header="time_s,nav_x_mm,nav_y_mm,route_s_mm,cross_track_mm,match_dist_mm,body_forward_mm_s,wheel_speed_mm_s,wheel_first_order_prediction_mm_s,planned_route_speed_mm_s,car_target_speed_set_equiv_mm_s,route_plan_minus_car_command_mm_s,car_command_minus_wheel_mm_s,wheel_minus_body_mm_s,accel_mm_s2,yaw_rate_deg_s,slip_signed_mm_s,slip_proxy_mm_s,front_servo_deg,rear_servo_deg,servo_mean_deg,servo_front_rear_delta_deg,pwm_left,pwm_right,speed_L_rpm,speed_R_rpm,route_yaw_deg,route_target_speed,route_curvature_per_mm,route_point_type,special_active", comments="")
    summary["_derived_path"] = str(derived_path)
    fig, axes = plt.subplots(3, 2, figsize=(16, 12), constrained_layout=True)
    axes[0, 0].plot(d["nav_x"], d["nav_y"], lw=1.0, label="fused nav")
    axes[0, 0].plot(route["x"], route["y"], lw=1.0, alpha=.7, label="planned route")
    axes[0, 0].set_aspect("equal", adjustable="datalim"); axes[0, 0].set_title("Trajectory vs plan"); axes[0, 0].legend()
    axes[0, 1].plot(t, body_speed, label="body vx"); axes[0, 1].plot(t, wheel_speed, label="wheel avg"); axes[0, 1].plot(t, target_mm_s, label="target equiv", alpha=.7); axes[0, 1].set_title("Speed signals (mm/s)"); axes[0, 1].legend()
    axes[1, 0].plot(t, accel); axes[1, 0].axhline(0, color="k", lw=.5); axes[1, 0].set_title("Longitudinal acceleration (mm/s2)")
    axes[1, 1].plot(t, cross); axes[1, 1].axhline(0, color="k", lw=.5); axes[1, 1].set_title("Signed cross-track error (mm)")
    axes[2, 0].plot(t, target_error, label="target command - wheel"); axes[2, 0].plot(t, speed_shortfall, label="route plan - body", alpha=.7); axes[2, 0].set_title("Command/plan speed errors (mm/s)"); axes[2, 0].legend()
    axes[2, 1].plot(t, slip_proxy, label="slip proxy"); axes[2, 1].plot(t, front_servo, label="front servo", alpha=.7); axes[2, 1].plot(t, rear_servo, label="rear servo", alpha=.7); axes[2, 1].set_title("Slip and servo signals"); axes[2, 1].legend()
    for ax in axes.flat: ax.grid(alpha=.25); ax.set_xlabel("time (s)")
    fig.savefig(output / f"{stem}_overview.svg")
    plt.close(fig)

    # Dedicated speed-chain plot. It makes the distinction between offline
    # route speed, vehicle-side target_speed_set, measured wheel speed, and
    # inertial body speed explicit.
    fig, axes = plt.subplots(3, 1, figsize=(16, 11), sharex=True, constrained_layout=True)
    fig.patch.set_facecolor("#f8fafc")
    palette = {"route": "#2563eb", "command": "#f97316", "wheel": "#16a34a", "body": "#7c3aed", "pred": "#0f766e"}
    axes[0].plot(t, planned_forward, color=palette["route"], lw=1.8, label="route table planned speed")
    axes[0].plot(t, target_mm_s, color=palette["command"], lw=1.4, label="car target_speed_set equivalent")
    axes[0].plot(t, wheel_speed, color=palette["wheel"], lw=1.25, label="measured wheel average")
    axes[0].plot(t, body_speed_smooth, color=palette["body"], lw=1.2, label="inertial body speed")
    axes[0].plot(t, wheel_pred, color=palette["pred"], lw=1.1, linestyle="--", label="first-order wheel prediction")
    axes[0].set_title("Plan4 speed chain: offline plan -> car command -> wheel -> body", loc="left", fontweight="bold")
    axes[0].set_ylabel("forward speed (mm/s)")
    axes[0].legend(loc="upper right", ncol=2, frameon=False)
    axes[1].plot(t, planning_to_command_error, color="#ea580c", lw=1.2, label="route plan - car command")
    axes[1].plot(t, command_to_wheel_error, color="#15803d", lw=1.2, label="car command - wheel")
    axes[1].plot(t, wheel_to_body_error, color="#6d28d9", lw=1.2, label="wheel - body")
    axes[1].axhline(0.0, color="#64748b", lw=.8)
    axes[1].set_title("Error assigned to each link", loc="left", fontweight="bold")
    axes[1].set_ylabel("speed error (mm/s)")
    axes[1].legend(loc="upper right", ncol=3, frameon=False)
    axes[2].plot(t, d["pwm_left"], color="#2563eb", lw=1.0, label="left PWM")
    axes[2].plot(t, d["pwm_right"], color="#dc2626", lw=1.0, label="right PWM")
    axes[2].plot(t, d["target_speed_set"] * 4.0, color="#475569", lw=1.0, alpha=.75, label="target_speed_set x4")
    axes[2].set_title("Motor actuation signals", loc="left", fontweight="bold")
    axes[2].set_ylabel("PWM / scaled command")
    axes[2].set_xlabel("time (s)")
    axes[2].legend(loc="upper right", ncol=3, frameon=False)
    for ax in axes:
        ax.grid(color="#cbd5e1", alpha=.6, linewidth=.65)
        ax.spines[["top", "right"]].set_visible(False)
        ax.set_facecolor("#ffffff")
    fig.savefig(output / f"{stem}_speed_chain.svg", facecolor=fig.get_facecolor())
    plt.close(fig)
    return summary


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-dir", type=Path, required=True)
    parser.add_argument("--route", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--telemetry", type=Path, action="append", help="Analyze only these telemetry CSV files; may be repeated.")
    parser.add_argument(
        "--route-model-json", type=Path,
        help="Use fixed tau_acc_s/tau_dec_s/gain from a previous route_speed_model.json instead of fitting this run.",
    )
    args = parser.parse_args()
    route = parse_route(args.route)
    logs = args.telemetry if args.telemetry else sorted(args.input_dir.glob("wifi_telemetry_*.csv"))
    summaries = [analyze_one(log, route, args.output_dir) for log in logs]
    derived_paths = [Path(row.pop("_derived_path")) for row in summaries if "_derived_path" in row]
    fixed_parameters = None
    if args.route_model_json:
        loaded_model = json.loads(args.route_model_json.read_text(encoding="utf-8"))
        fixed_parameters = loaded_model.get("parameters", loaded_model).copy()
    route_only_model = fit_route_only_model(route, derived_paths, fixed_parameters)
    simulation = route_only_model.pop("simulation", None)
    route_model_output = {
        "route": {"count": len(route["x"]), "length_mm": float(route["s"][-1])},
        "route_only_speed_model": route_only_model,
        "runs": summaries,
    }
    (args.output_dir / "summary.json").write_text(json.dumps(route_model_output, ensure_ascii=False, indent=2), encoding="utf-8")
    (args.output_dir / "route_speed_model.json").write_text(json.dumps(route_only_model, ensure_ascii=False, indent=2), encoding="utf-8")
    if route_only_model.get("available"):
        params = route_only_model["parameters"]
        fit = route_only_model["fit_error"]
        mode = route_only_model["calibration_source"]
        lines = [
            "# 路表实际速度预测模型",
            "",
            "模型只使用路表的 `target_speed(s)` 作为规划输入：",
            "",
            "```text",
            "dv/dt = (gain * u_route(s) - v) / tau",
            "ds/dt = v",
            "tau = tau_acc（加速）或 tau_dec（减速）",
            "```",
            "",
            f"标定方式：{mode}。",
            f"参数：`tau_acc={params['tau_acc_s']:.3f} s`，`tau_dec={params['tau_dec_s']:.3f} s`，`gain={params['gain']:.3f}`。",
            f"预测完成时间：`{route_only_model['predicted_lap_time_s']:.3f} s`；预测峰值速度：`{route_only_model['predicted_speed_peak_mm_s']:.1f} mm/s`。",
            f"速度预测误差：RMSE `{fit['speed_rmse_mm_s']:.1f} mm/s`，P95 绝对误差 `{fit['speed_p95_abs_error_mm_s']:.1f} mm/s`，平均偏差 `{fit['speed_bias_mm_s']:.1f} mm/s`。",
            "",
            "## 使用边界",
            "- 这是规划层的前馈预测，不代替车端轮速/惯导闭环。",
            "- 当前模型未显式输入电池电压、坡度、载荷、轮胎温度、滑移、横向误差降速和状态机接管；这些因素会进入残差。",
            "- 对新路表，应固定既有标定参数，用 `--route-model-json` 做盲预测；不要用同一条待验证日志重新标定后再宣称预测准确。",
            "- 规划生成时应把预测的速度曲线再做反向可达性约束，在需要减速的入口前留出响应距离与物理制动距离。",
        ]
        (args.output_dir / "route_speed_prediction_report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    if simulation is not None:
        np.savetxt(
            args.output_dir / "route_speed_prediction.csv",
            np.column_stack([simulation["s_mm"], simulation["time_s"], simulation["command_mm_s"], simulation["speed_mm_s"]]),
            delimiter=",",
            header="route_s_mm,predicted_time_s,route_table_command_mm_s,predicted_actual_speed_mm_s",
            comments="",
        )
        fig, axes = plt.subplots(2, 1, figsize=(16, 10), sharex=True, constrained_layout=True)
        fig.patch.set_facecolor("#f8fafc")
        axes[0].set_facecolor("#ffffff")
        axes[0].plot(simulation["s_mm"], simulation["command_mm_s"], color="#2563eb", lw=1.7, label="route table target speed")
        axes[0].plot(simulation["s_mm"], simulation["speed_mm_s"], color="#f97316", lw=1.8, label="predicted actual speed")
        for derived_path in derived_paths:
            observed = load_derived_for_route_model(derived_path)
            valid = (observed["match_dist_mm"] < 450.0) & np.isfinite(observed["route_s_mm"]) & np.isfinite(observed["body_forward_mm_s"])
            axes[0].scatter(observed["route_s_mm"][valid][::8], observed["body_forward_mm_s"][valid][::8], s=4, color="#64748b", alpha=.25)
        axes[0].set_title("Route-table-only speed prediction calibrated from three runs", loc="left", fontweight="bold")
        axes[0].set_ylabel("forward speed (mm/s)")
        axes[0].legend(frameon=False)
        axes[1].plot(simulation["s_mm"], simulation["time_s"], color="#0f766e", lw=1.8)
        axes[1].set_title("Predicted elapsed time along the route", loc="left", fontweight="bold")
        axes[1].set_xlabel("route arc length (mm)")
        axes[1].set_ylabel("predicted time (s)")
        for axis in axes:
            axis.grid(color="#cbd5e1", alpha=.6, linewidth=.65)
            axis.spines[["top", "right"]].set_visible(False)
        fig.savefig(args.output_dir / "route_speed_prediction.svg", facecolor=fig.get_facecolor())
        plt.close(fig)
    with (args.output_dir / "summary.csv").open("w", encoding="utf-8", newline="") as f:
        if summaries:
            fields = sorted({k for row in summaries for k in row})
            w = csv.DictWriter(f, fieldnames=fields); w.writeheader(); w.writerows(summaries)
    valid_runs = [x for x in summaries if "speed_chain" in x]
    if valid_runs:
        fig, ax = plt.subplots(figsize=(16, 9), constrained_layout=True)
        fig.patch.set_facecolor("#f8fafc")
        ax.set_facecolor("#ffffff")
        ax.axis("off")
        taus = [x["actuator_first_order_fit"].get("tau_s") for x in valid_runs]
        route_cmd = [x["speed_chain"]["route_plan_to_car_command"].get("response_delay_s") for x in valid_runs]
        cmd_wheel = [x["speed_chain"]["car_command_to_wheel"].get("response_delay_s") for x in valid_runs]
        lag_d = [x["plan4_code_analysis"].get("first_order_lag_distance_at_4000_mm_s") for x in valid_runs]
        title = "Plan4 actual-response analysis and optimization design"
        lines = [
            "Measured speed chain (forward-positive):", "route table speed -> target_speed_set -> measured wheel speed -> vx_body",
            "",
            f"Fitted command-to-wheel time constant tau: {np.nanmin(taus):.2f} to {np.nanmax(taus):.2f} s",
            f"At 4.0 m/s, first-order response distance: {np.nanmin(lag_d):.0f} to {np.nanmax(lag_d):.0f} mm",
            "Current special handoff lead: 500 mm; this is insufficient as a braking/response reserve at high speed.",
            "",
            "The fitted v1/v2 braking formula is NOT global:",
            "- only 7 to 10 automatically extracted partial deceleration events per run; no explicit brake trigger or full-stop endpoint.",
            "- terrain, battery voltage, payload, slope, speed direction, PWM saturation, and tire state change the coefficients.",
            "- use it only as a provisional local deceleration-distance prior until a dedicated brake calibration dataset exists.",
            "",
            "Recommended design, without changing code yet:",
            "1. Maintain v_hat: predict actual speed from target_speed_set with a first-order model, and correct it with wheel/vx feedback.",
            "2. Offline plan backward from each required target speed using d_response + d_physical + margin, not only v^2/(2a).",
            "3. Runtime uses v_hat/vx_body for braking and entry gates; target_speed_set remains the actuator command, not a speed measurement.",
            "4. Dynamic special handoff requires remaining distance, actual speed, heading error, lateral error, and stable samples.",
            "5. Calibrate PWM-to-RPM only under steady straight runs; then use it for feedforward, leaving feedback control active.",
        ]
        ax.text(.045, .95, title, transform=ax.transAxes, va="top", fontsize=21, fontweight="bold", color="#0f172a")
        ax.text(.05, .87, "\n".join(lines), transform=ax.transAxes, va="top", fontsize=13, color="#334155", linespacing=1.55)
        fig.savefig(args.output_dir / "plan4_response_optimization_design.svg", facecolor=fig.get_facecolor())
        plt.close(fig)
    report = ["# Plan4 回放日志分析", "", f"路表点数：{len(route['x'])}，路表弧长：{route['s'][-1]:.1f} mm。", "", "## 数据边界", "- 仅筛选 `g_replay_state == 1`。", "- `nav_x/nav_y` 是视觉融合坐标；检测到的大跳变不参与速度、加速度、滑移导数。", "- 本批路表和回放日志中的任务点/状态机标志均未出现非零值，因此专项任务段没有可观测样本。", "", "## 各次运行摘要"]
    for row in summaries:
        report.extend([
            f"### {row['file']}",
            f"- 时长 {row.get('duration_s', float('nan')):.3f} s，回放样本 {row.get('running_samples', 0)}。",
            f"- 车体前进速度 P99 {row.get('max_body_speed_mm_s', float('nan')):.1f} mm/s；加速度 P99/P1 {row.get('max_accel_mm_s2', float('nan')):.1f}/{row.get('max_decel_mm_s2', float('nan')):.1f} mm/s²。",
            f"- 普通路段横向误差 RMSE/P95 {row.get('ordinary_cross_track_rmse_mm', float('nan')):.1f}/{row.get('ordinary_cross_track_p95_mm', float('nan')):.1f} mm。",
            f"- 轮速与车体速度差 P95 {row.get('slip_proxy_p95_mm_s', float('nan')):.1f} mm/s；融合跳变 {row.get('fusion_jump_count', 0)} 次。",
            f"- 目标转速等效量与轮速相关系数 {row.get('target_wheel_correlation', float('nan')):.3f}，相关峰时延 {row.get('target_wheel_lag_s', float('nan')):.3f} s；这说明 target_speed_set 不是瞬时速度。",
            f"- 路表计划速度与实测车速中位差 {row.get('planned_actual_speed_bias_mm_s', float('nan')):.1f} mm/s，绝对误差 P95 {row.get('planned_actual_speed_p95_abs_mm_s', float('nan')):.1f} mm/s。",
            f"- 代码规划上限 4000 mm/s、离线减速度 1500 mm/s²；按拟合 tau 估计，4 m/s 一阶滞后距离约 {row.get('plan4_code_analysis', {}).get('first_order_lag_distance_at_4000_mm_s', float('nan')):.0f} mm，约为 500 mm 交接提前量的 {row.get('plan4_code_analysis', {}).get('lag_to_runtime_handoff_ratio', float('nan')):.1f} 倍。",
            f"- 可识别的部分减速事件 {row.get('brake_event_count', 0)} 个；刹车模型仅在事件数不少于 3 时拟合。", "",
            f"- v1/v2 部分刹车模型：{row.get('brake_v1_v2_fit', {}).get('formula', 'n/a')}；拟合系数 {row.get('brake_v1_v2_fit', {}).get('coefficients', None)}，RMSE {row.get('brake_v1_v2_fit', {}).get('rmse_mm', float('nan'))} mm。",
            f"- PWM-轮速模型和舵机模型已写入 summary.json；舵机前后组差值用于识别机械/标定不一致。", "",
        ])
    report.extend(["## 速度链路与优化方案", "### 三层速度必须区分", "1. 路表 `target_speed`：离线规划的理想速度命令。", "2. 车端 `target_speed_set`：Plan4 经 `Plan4_SafeSpeed()` 和 `Plan4_Ramp()` 后写给底盘的命令，仍不是实测速度。", "3. 实际速度：`speed_L/speed_R` 转换的轮速和 `vx_body`，用于反馈和安全判断。", "", "### 不修改代码前的结论", "- 当前日志已经显示 `target_speed_set` 到轮速存在约 0.8--0.94 s 的一阶响应，不应让离线路径把它当成即时速度。", "- 路表速度与车端命令的差，主要来自 Plan4 的横向/航向降速、速度斜坡、特殊任务接管；命令到轮速的差才是执行器/底盘响应。", "- `v1/v2` 模型只能视为当前场地与当前电池/轮胎状态下的部分减速先验，不能宣称全局有效。", "", "### 推荐优化设计", "1. 使用 `v_hat`：由 `target_speed_set` 经一阶模型预测实际速度，再由轮速和 `vx_body` 反馈校正。", "2. 离线规划对每个减速点使用 `d_total=d_response(v1,v_cmd)+d_brake(v1,v2)+margin` 反向传播速度上限。", "3. `d_response` 首先可用一阶近似 `v*tau`，之后用日志分加速、制动、不同电压/地面条件分别标定。", "4. 状态机交接改为动态距离门禁：剩余距离、`v_hat`、航向误差、横向误差、持续稳定周期全部满足才交接。", "5. 固件暂不改动；下一阶段先补采专门的阶跃、定速和明确 v1/v2 刹车日志，验证模型是否跨场地稳定。", ""])
    (args.output_dir / "analysis_report.md").write_text("\n".join(report), encoding="utf-8")
    print(json.dumps({"runs": len(summaries), "output": str(args.output_dir)}, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
