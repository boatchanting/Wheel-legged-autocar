#!/usr/bin/env python3
"""Compare the three common models on the 20260814_153034 route.

Methods: route-table ARX plus planned-curvature kinematics, base differential
drive, and differential drive with the lateral-acceleration turn oscillator.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


THIS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(THIS_DIR))
import analyze_speed_models as speed_model
import fit_combined_speed_model as combined


GROUP2_DIR = Path("data/nav_mark_points_20260814_153034.csv")
OUT_DIR = THIS_DIR / "model"


def initial_heading(route: pd.DataFrame) -> float:
    xy = route[["x", "y"]].to_numpy(float)
    delta = xy[1] - xy[0]
    return float(np.arctan2(delta[1], delta[0]))


def add_xy_from_yaw(simulation: pd.DataFrame, route: pd.DataFrame) -> pd.DataFrame:
    """Integrate the model's predicted yaw and distance into route-frame x/y."""
    out = simulation.copy()
    heading0 = initial_heading(route)
    distance = out["distance_mm"].to_numpy(float)
    yaw = heading0 + np.deg2rad(out["predicted_relative_yaw_deg"].to_numpy(float))
    xy = np.zeros((len(out), 2), dtype=float)
    xy[0] = route[["x", "y"]].iloc[0].to_numpy(float)
    for i in range(1, len(out)):
        ds = max(distance[i] - distance[i - 1], 0.0)
        xy[i] = xy[i - 1] + ds * np.array([np.cos(yaw[i - 1]), np.sin(yaw[i - 1])])
    out["predicted_x_mm"] = xy[:, 0]
    out["predicted_y_mm"] = xy[:, 1]
    return out


def arx_with_planned_kinematics(route: pd.DataFrame, coefficients: np.ndarray) -> pd.DataFrame:
    """Use the ARX speed prediction and integrate planned curvature for yaw."""
    sim = speed_model.simulate_from_route_table(route, coefficients, dt_s=combined.DT_S, initial_speed_mm_s=0.0)
    psi = 0.0
    yaw = []
    curvature = route["curvature"].to_numpy(float)
    for idx, speed in zip(sim["route_index"].to_numpy(int), sim["predicted_speed_mm_s"].to_numpy(float)):
        yaw.append(np.rad2deg(psi))
        psi += speed * curvature[idx] * combined.DT_S
    sim["predicted_relative_yaw_deg"] = yaw
    return add_xy_from_yaw(sim, route)


def simulation_metrics(actual: pd.DataFrame, simulation: pd.DataFrame) -> dict[str, float]:
    """Compare model and telemetry at common elapsed times from replay start."""
    actual_t = actual["t_s"].to_numpy(float)
    duration = float(simulation["time_s"].iloc[-1])
    keep = actual_t <= duration
    t = actual_t[keep]
    if len(t) < 2:
        raise ValueError("Prediction duration has no usable overlap with telemetry")
    pred_t = simulation["time_s"].to_numpy(float)
    speed = np.interp(t, pred_t, simulation["predicted_speed_mm_s"].to_numpy(float))
    x = np.interp(t, pred_t, simulation["predicted_x_mm"].to_numpy(float))
    y = np.interp(t, pred_t, simulation["predicted_y_mm"].to_numpy(float))
    # Work in relative coordinates to remove the small initial map/fusion offset
    # while retaining all subsequent visual-fusion deviations.
    actual_x = actual["nav_x"].to_numpy(float)[keep] - float(actual["nav_x"].iloc[0])
    actual_y = actual["nav_y"].to_numpy(float)[keep] - float(actual["nav_y"].iloc[0])
    pred_x = x - simulation["predicted_x_mm"].iloc[0]
    pred_y = y - simulation["predicted_y_mm"].iloc[0]
    speed_error = speed - actual["v_body_mm_s"].to_numpy(float)[keep]
    position_squared_error = (pred_x - actual_x) ** 2 + (pred_y - actual_y) ** 2
    return {
        "comparison_samples": int(len(t)),
        "predicted_completion_time_s": duration,
        "actual_completion_time_s": float(actual["t_s"].iloc[-1]),
        "completion_time_error_s": duration - float(actual["t_s"].iloc[-1]),
        "speed_mse_mm2_s2": float(np.mean(speed_error**2)),
        "speed_rmse_mm_s": float(np.sqrt(np.mean(speed_error**2))),
        "trajectory_mse_mm2": float(np.mean(position_squared_error)),
        "trajectory_rmse_mm": float(np.sqrt(np.mean(position_squared_error))),
        "endpoint_error_at_overlap_mm": float(np.sqrt(position_squared_error[-1])),
    }


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    model = json.loads((OUT_DIR / "combined_speed_model.json").read_text(encoding="utf-8"))
    route = speed_model.parse_route_table(next(GROUP2_DIR.glob("nav_replay_route_table*.h")))
    samples, _ = speed_model.load_samples(GROUP2_DIR, route)
    first_order_curvature = np.asarray(model["common_first_order_curvature"]["coefficients_c_a_b_b_kappa_b_du"], dtype=float)
    dd_model = model["common_pure_differential_drive"]
    left = np.asarray(dd_model["left_coefficients_c_a_b"], dtype=float)
    right = np.asarray(dd_model["right_coefficients_c_a_b"], dtype=float)
    track = float(dd_model["track_width_mm"])

    predictions = {
        "purple_arx_planned_kinematics": arx_with_planned_kinematics(route, first_order_curvature),
        "gray_base_differential_drive": add_xy_from_yaw(combined.simulate_differential_drive(route, track, left, right), route),
        "red_differential_drive_turn_oscillator": add_xy_from_yaw(
            combined.simulate_differential_drive(route, track, left, right, dd_model["turn_oscillator"]), route
        ),
    }

    rows = []
    for method, sim in predictions.items():
        sim.to_csv(OUT_DIR / f"group2_{method}_full_course.csv", index=False)
        for run_no, (file_name, actual) in enumerate(samples.groupby("file", sort=False), start=1):
            rows.append({"method": method, "run": run_no, "file": file_name, **simulation_metrics(actual, sim)})
    metrics = pd.DataFrame(rows)
    metrics.to_csv(OUT_DIR / "group2_three_method_full_course_metrics.csv", index=False)
    summary = metrics.groupby("method")[
        ["predicted_completion_time_s", "actual_completion_time_s", "completion_time_error_s", "speed_mse_mm2_s2", "speed_rmse_mm_s", "trajectory_mse_mm2", "trajectory_rmse_mm", "endpoint_error_at_overlap_mm"]
    ].mean().reset_index()
    summary.to_csv(OUT_DIR / "group2_three_method_full_course_summary.csv", index=False)

    styles = {
        "purple_arx_planned_kinematics": ("#7E57C2", "ARX + planned curvature"),
        "gray_base_differential_drive": ("#777777", "base differential drive"),
        "red_differential_drive_turn_oscillator": ("#D62728", "differential drive + turn oscillator"),
    }
    fig, (ax_speed, ax_xy) = plt.subplots(1, 2, figsize=(15.5, 6.0))
    actual_colors = ["#4E79A7", "#59A14F"]
    for run_no, ((_, actual), color) in enumerate(zip(samples.groupby("file", sort=False), actual_colors), start=1):
        ax_speed.plot(actual["t_s"], actual["v_body_mm_s"], color=color, lw=0.9, alpha=0.75, label=f"actual speed run {run_no}")
        ax_xy.plot(
            actual["nav_x"] - actual["nav_x"].iloc[0],
            actual["nav_y"] - actual["nav_y"].iloc[0],
            color=color,
            lw=0.9,
            alpha=0.75,
            label=f"actual trajectory run {run_no}",
        )
    for method, sim in predictions.items():
        color, label = styles[method]
        ax_speed.plot(sim["time_s"], sim["predicted_speed_mm_s"], color=color, lw=2.0, label=label)
        ax_xy.plot(
            sim["predicted_x_mm"] - sim["predicted_x_mm"].iloc[0],
            sim["predicted_y_mm"] - sim["predicted_y_mm"].iloc[0],
            color=color,
            lw=2.0,
            label=label,
        )
    ax_speed.set(xlabel="time from replay start (s)", ylabel="forward speed (mm/s)", title="Full-course speed prediction")
    ax_xy.set(xlabel="relative X (mm)", ylabel="relative Y (mm)", title="Full-course trajectory prediction")
    for ax in (ax_speed, ax_xy):
        ax.grid(alpha=0.25)
        ax.legend(fontsize=7.8)
    ax_xy.set_aspect("equal", adjustable="box")
    fig.tight_layout()
    fig.savefig(OUT_DIR / "group2_three_method_full_course_comparison.png", dpi=180)
    plt.close(fig)

    report = [
        "# 第二组：三种方法全程复现对比",
        "",
        "速度和轨迹 MSE 在各回放开始后的共同时间范围内计算。轨迹比较的是 `nav_x/nav_y` 相对首帧的位移，因此排除了起点常量偏移，但保留视觉融合过程中的所有误差。",
        "",
        "| 方法 | 预测全程时间 (s) | 实际时间均值 (s) | 时间误差 (s) | 速度 MSE (mm²/s²) | 轨迹 MSE (mm²) | 轨迹 RMSE (mm) |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ]
    for _, row in summary.iterrows():
        report.append(
            f"| {row['method']} | {row['predicted_completion_time_s']:.3f} | {row['actual_completion_time_s']:.3f} | {row['completion_time_error_s']:.3f} | {row['speed_mse_mm2_s2']:.1f} | {row['trajectory_mse_mm2']:.1f} | {row['trajectory_rmse_mm']:.1f} |"
        )
    (OUT_DIR / "group2_three_method_full_course_report.md").write_text("\n".join(report) + "\n", encoding="utf-8")
    print(summary.to_json(orient="records", indent=2))


if __name__ == "__main__":
    main()
