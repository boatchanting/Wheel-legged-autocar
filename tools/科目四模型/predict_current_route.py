#!/usr/bin/env python3
"""Predict the default firmware route table with the three common models."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


THIS_DIR = Path(__file__).resolve().parent
PROJECT_DIR = THIS_DIR.parent.parent
sys.path.insert(0, str(THIS_DIR))
import analyze_speed_models as speed_model
import fit_combined_speed_model as combined
import compare_three_methods_group2 as compare


def main() -> None:
    ap = argparse.ArgumentParser(description="Predict a route table using the common ARX and differential-drive models.")
    ap.add_argument("--route", type=Path, default=Path("code/navigation/nav_replay_route_table.h"))
    ap.add_argument("--model", type=Path, default=THIS_DIR / "model" / "combined_speed_model.json")
    ap.add_argument("--out-dir", type=Path, default=THIS_DIR / "model" / "nav_replay_route_table_prediction")
    args = ap.parse_args()
    route_path = args.route if args.route.is_absolute() else PROJECT_DIR / args.route
    model_path = args.model if args.model.is_absolute() else PROJECT_DIR / args.model
    out_dir = args.out_dir if args.out_dir.is_absolute() else PROJECT_DIR / args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    route = speed_model.parse_route_table(route_path)
    model = json.loads(model_path.read_text(encoding="utf-8"))
    dd = model["common_pure_differential_drive"]
    left = np.asarray(dd["left_coefficients_c_a_b"], dtype=float)
    right = np.asarray(dd["right_coefficients_c_a_b"], dtype=float)
    track = float(dd["track_width_mm"])
    arx_coef = np.asarray(model["common_first_order_curvature"]["coefficients_c_a_b_b_kappa_b_du"], dtype=float)

    predictions = {
        "purple_arx_planned_kinematics": compare.arx_with_planned_kinematics(route, arx_coef),
        "gray_base_differential_drive": compare.add_xy_from_yaw(
            combined.simulate_differential_drive(route, track, left, right), route
        ),
        "red_differential_drive_turn_oscillator": compare.add_xy_from_yaw(
            combined.simulate_differential_drive(route, track, left, right, dd["turn_oscillator"]), route
        ),
    }
    styles = {
        "purple_arx_planned_kinematics": ("#7E57C2", "ARX + planned curvature"),
        "gray_base_differential_drive": ("#777777", "base differential drive"),
        "red_differential_drive_turn_oscillator": ("#D62728", "differential drive + turn oscillator"),
    }
    rows = []
    for name, simulation in predictions.items():
        if "predicted_yaw_rate_rad_s" not in simulation:
            simulation["predicted_yaw_rate_rad_s"] = np.gradient(
                np.deg2rad(simulation["predicted_relative_yaw_deg"].to_numpy(float)), combined.DT_S
            )
        simulation.to_csv(out_dir / f"{name}.csv", index=False)
        rows.append(
            {
                "method": name,
                "completion_time_s": float(simulation["time_s"].iloc[-1]),
                "route_distance_mm": float(simulation["distance_mm"].iloc[-1]),
                "peak_speed_mm_s": float(simulation["predicted_speed_mm_s"].max()),
                "terminal_speed_mm_s": float(simulation["predicted_speed_mm_s"].iloc[-1]),
                "peak_abs_yaw_rate_rad_s": float(simulation["predicted_yaw_rate_rad_s"].abs().max()),
                "terminal_relative_yaw_deg": float(simulation["predicted_relative_yaw_deg"].iloc[-1]),
            }
        )
    summary = pd.DataFrame(rows)
    summary.to_csv(out_dir / "prediction_summary.csv", index=False)

    fig, (ax_speed, ax_xy) = plt.subplots(1, 2, figsize=(15.5, 5.8))
    for name, simulation in predictions.items():
        color, label = styles[name]
        ax_speed.plot(simulation["distance_mm"], simulation["predicted_speed_mm_s"], color=color, lw=2.0, label=label)
        ax_xy.plot(
            simulation["predicted_x_mm"] - simulation["predicted_x_mm"].iloc[0],
            simulation["predicted_y_mm"] - simulation["predicted_y_mm"].iloc[0],
            color=color,
            lw=2.0,
            label=label,
        )
    ax_speed.set(xlabel="predicted route distance (mm)", ylabel="forward speed (mm/s)", title="Prediction for route table")
    ax_xy.set(xlabel="relative X (mm)", ylabel="relative Y (mm)", title="Predicted trajectory")
    for ax in (ax_speed, ax_xy):
        ax.grid(alpha=0.25)
        ax.legend(fontsize=8)
    ax_xy.set_aspect("equal", adjustable="box")
    fig.tight_layout()
    fig.savefig(out_dir / "prediction_comparison.png", dpi=180)
    plt.close(fig)

    report = [
        "# Route-table prediction",
        "",
        f"- Input: `{route_path.as_posix()}`, {len(route)} points.",
        "- This is a point-table-only forecast. MSE requires a matching telemetry log.",
        "",
        "| method | completion time (s) | predicted distance (mm) | peak speed (mm/s) | terminal speed (mm/s) | terminal relative yaw (deg) |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    for row in rows:
        report.append(
            f"| {row['method']} | {row['completion_time_s']:.3f} | {row['route_distance_mm']:.1f} | {row['peak_speed_mm_s']:.1f} | {row['terminal_speed_mm_s']:.1f} | {row['terminal_relative_yaw_deg']:.1f} |"
        )
    (out_dir / "report.md").write_text("\n".join(report) + "\n", encoding="utf-8")

    print(f"Route: {route_path}")
    print(f"Route points: {len(route)}")
    print(summary.to_string(index=False, float_format=lambda value: f"{value:.3f}"))
    print(f"Outputs: {out_dir}")


if __name__ == "__main__":
    main()
