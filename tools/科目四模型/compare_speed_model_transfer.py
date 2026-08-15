#!/usr/bin/env python3
"""Transfer old-route speed models to a new route without using its telemetry."""

from __future__ import annotations

import json
import sys
import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

sys.path.insert(0, str(Path(__file__).parent))
import analyze_speed_models as speed_model


DT_S = 0.01
WHEEL_MM_PER_RPM = 4.79


def route_distance(route: pd.DataFrame) -> np.ndarray:
    xy = route[["x", "y"]].to_numpy(float)
    return np.r_[0.0, np.cumsum(np.sqrt(np.sum(np.diff(xy, axis=0) ** 2, axis=1)))]


def load_evaluation_logs(paths: list[Path], route: pd.DataFrame) -> pd.DataFrame:
    """Load only the explicitly selected new-route logs for evaluation."""
    route_xy = route[["x", "y"]].to_numpy(float)
    frames = []
    for path in paths:
        raw = pd.read_csv(path)
        frame = raw.loc[raw["g_replay_state"] == 1].copy()
        if frame.empty:
            continue
        pts = frame[["nav_x", "nav_y"]].to_numpy(float)
        d2 = ((pts[:, None, :] - route_xy[None, :, :]) ** 2).sum(axis=2)
        idx = d2.argmin(axis=1)
        frame["file"] = path.name
        frame["route_index"] = idx
        frame["v_actual_mm_s"] = -frame["vx_body"].astype(float)
        frame["route_match_error_mm"] = np.sqrt(d2[np.arange(len(frame)), idx])
        frames.append(frame)
    if not frames:
        raise ValueError("No new-route automatic replay samples found")
    return pd.concat(frames, ignore_index=True)


def wheel_commands(u_rpm: float, curvature: float, track_width_mm: float) -> tuple[float, float]:
    """Convert planned centre speed/curvature to differential-wheel commands."""
    centre = WHEEL_MM_PER_RPM * u_rpm
    left = centre * (1.0 - track_width_mm * curvature / 2.0)
    right = centre * (1.0 + track_width_mm * curvature / 2.0)
    return left, right


def fit_wheel_models(samples: pd.DataFrame, track_width_mm: float) -> tuple[np.ndarray, np.ndarray]:
    """Fit vL/vR first-order plants from the old-route logs only."""
    train = samples.copy()
    train["cmd_left_mm_s"], train["cmd_right_mm_s"] = zip(
        *[
            wheel_commands(u, k, track_width_mm)
            for u, k in zip(train["u_rpm"].to_numpy(float), train["route_curvature"].to_numpy(float))
        ]
    )
    train["wheel_left_mm_s"] = WHEEL_MM_PER_RPM * train["speed_L"].astype(float)
    train["wheel_right_mm_s"] = -WHEEL_MM_PER_RPM * train["speed_R"].astype(float)
    result = []
    for side in ("left", "right"):
        v = f"wheel_{side}_mm_s"
        command = f"cmd_{side}_mm_s"
        blocks = []
        targets = []
        for _, frame in train.groupby("file", sort=False):
            y = frame[v].to_numpy(float)
            cmd = frame[command].to_numpy(float)
            blocks.append(np.column_stack([np.ones(len(frame) - 1), y[:-1], cmd[1:]]))
            targets.append(y[1:])
        result.append(np.linalg.lstsq(np.vstack(blocks), np.concatenate(targets), rcond=None)[0])
    return result[0], result[1]


def simulate_differential_drive(
    route: pd.DataFrame, track_width_mm: float, left_coef: np.ndarray, right_coef: np.ndarray
) -> pd.DataFrame:
    """Pure red simulation: wheel plants + differential-drive kinematics."""
    distance = route_distance(route)
    u = -route["target_speed_rpm"].to_numpy(float)
    curvature = route["curvature"].to_numpy(float)
    s_mm = 0.0
    psi_rad = 0.0
    v_left = 0.0
    v_right = 0.0
    rows = []
    for step in range(100000):
        idx = int(np.searchsorted(distance, s_mm, side="right") - 1)
        idx = min(max(idx, 0), len(route) - 1)
        cmd_left, cmd_right = wheel_commands(u[idx], curvature[idx], track_width_mm)
        next_left = max(float(left_coef[0] + left_coef[1] * v_left + left_coef[2] * cmd_left), 0.0)
        next_right = max(float(right_coef[0] + right_coef[1] * v_right + right_coef[2] * cmd_right), 0.0)
        v = (next_left + next_right) / 2.0
        omega = (next_right - next_left) / track_width_mm
        rows.append(
            {
                "time_s": step * DT_S,
                "route_index": idx,
                "distance_mm": s_mm,
                "target_speed_rpm": -u[idx],
                "curvature_plan": curvature[idx],
                "v_left_mm_s": next_left,
                "v_right_mm_s": next_right,
                "predicted_speed_mm_s": v,
                "predicted_yaw_rate_rad_s": omega,
                "predicted_relative_yaw_deg": np.rad2deg(psi_rad),
            }
        )
        s_mm += v * DT_S
        psi_rad += omega * DT_S
        v_left, v_right = next_left, next_right
        if idx >= len(route) - 1:
            break
    return pd.DataFrame(rows)


def speed_by_route(simulation: pd.DataFrame, route_count: int) -> np.ndarray:
    grouped = simulation.groupby("route_index")["predicted_speed_mm_s"].median()
    return np.interp(np.arange(route_count), grouped.index.to_numpy(), grouped.to_numpy())


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--train-dir", type=Path, default=Path("data/nav_mark_points_20260813_204901.csv"))
    ap.add_argument("--predict-dir", type=Path, required=True)
    ap.add_argument("--log-glob", default="wifi_telemetry*.csv")
    ap.add_argument("--out-dir", type=Path, required=True)
    args = ap.parse_args()
    old_dir = args.train_dir
    new_dir = args.predict_dir
    old_route = speed_model.parse_route_table(next(old_dir.glob("nav_replay_route_table*.h")))
    new_route = speed_model.parse_route_table(next(new_dir.glob("nav_replay_route_table*.h")))
    old_samples, _ = speed_model.load_samples(old_dir, old_route)

    # Both model parameter sets are learned only from the old 204901 route.
    track_width_mm = speed_model.estimate_track_width(old_samples)
    left_coef, right_coef = fit_wheel_models(old_samples, track_width_mm)
    red = simulate_differential_drive(new_route, track_width_mm, left_coef, right_coef)
    purple_coef = speed_model.fit_model(old_samples, "first_order_curvature")
    purple = speed_model.simulate_from_route_table(new_route, purple_coef, dt_s=DT_S, initial_speed_mm_s=0.0)

    # These three logs are evaluation-only. No fields from them enter either simulation.
    test_paths = sorted(new_dir.glob(args.log_glob))
    actual = load_evaluation_logs(test_paths, new_route)
    dist = route_distance(new_route)
    red_by_route = speed_by_route(red, len(new_route))
    purple_by_route = speed_by_route(purple, len(new_route))

    metrics = []
    for run_no, (file_name, frame) in enumerate(actual.groupby("file", sort=False), start=1):
        idx = frame["route_index"].to_numpy(int)
        y = frame["v_actual_mm_s"].to_numpy(float)
        for name, prediction in (("red_pure_differential", red_by_route[idx]), ("purple_old_arx_transfer", purple_by_route[idx])):
            error = prediction - y
            metrics.append(
                {
                    "run": run_no,
                    "file": file_name,
                    "model": name,
                    "route_aligned_rmse_mm_s": float(np.sqrt(np.mean(error**2))),
                    "route_aligned_mae_mm_s": float(np.mean(np.abs(error))),
                }
            )
    metrics_df = pd.DataFrame(metrics)

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)
    red.to_csv(out_dir / "red_pure_differential_drive_simulation.csv", index=False)
    purple.to_csv(out_dir / "purple_old_route_arx_transfer_simulation.csv", index=False)
    actual.to_csv(out_dir / "new_route_actual_auto_samples.csv", index=False)
    metrics_df.to_csv(out_dir / "transfer_metrics.csv", index=False)
    (out_dir / "transfer_model_parameters.json").write_text(
        json.dumps(
            {
                "trained_on": str(old_dir),
                "predicted_route": str(new_dir),
                "red_pure_differential_drive": {
                    "track_width_mm": track_width_mm,
                    "left_wheel_coefficients": left_coef.tolist(),
                    "right_wheel_coefficients": right_coef.tolist(),
                    "initial_state": {"v_left_mm_s": 0.0, "v_right_mm_s": 0.0, "relative_yaw_deg": 0.0},
                },
                "purple_old_route_arx_transfer": {
                    "coefficients": purple_coef.tolist(),
                    "uses_old_route_residual_table": False,
                    "initial_speed_mm_s": 0.0,
                },
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    fig, ax = plt.subplots(figsize=(12.8, 5.6))
    colors = ["#4E79A7", "#59A14F", "#76B7B2"]
    for run_no, ((_, frame), color) in enumerate(zip(actual.groupby("file", sort=False), colors), start=1):
        ax.plot(
            dist[frame["route_index"].to_numpy(int)],
            frame["v_actual_mm_s"],
            color=color,
            lw=0.85,
            alpha=0.62,
            label=f"original -vx_body: run {run_no}",
        )
    ax.plot(
        red["distance_mm"],
        red["predicted_speed_mm_s"],
        color="#D62728",
        lw=2.6,
        label="red: pure differential-drive simulation",
        zorder=5,
    )
    ax.plot(
        purple["distance_mm"],
        purple["predicted_speed_mm_s"],
        color="#7E57C2",
        lw=2.4,
        label="purple: old-route ARX transfer (no residual table)",
        zorder=4,
    )
    ax.set(
        xlabel="route distance from new point-table start (mm)",
        ylabel="forward speed (mm/s)",
        title=f"Transfer Prediction on {new_dir.name}",
    )
    ax.grid(color="#D9DDE3", alpha=0.7, linewidth=0.7)
    ax.spines[["top", "right"]].set_visible(False)
    ax.legend(fontsize=8.5, ncol=2, frameon=False, loc="lower right")
    fig.tight_layout()
    fig.savefig(out_dir / "red_pure_vs_purple_transfer_vs_original.png", dpi=180)
    plt.close(fig)

    summary = metrics_df.groupby("model")[["route_aligned_rmse_mm_s", "route_aligned_mae_mm_s"]].mean()
    report = [
        "# Cross-route speed-model transfer",
        "",
        "Both predictions use only the new route table at simulation time. The new-route telemetry is used only for this evaluation plot and metrics.",
        "",
        f"- Red model: pure differential-drive simulation, B={track_width_mm:.1f} mm, with independently identified left/right first-order wheel plants.",
        "- Purple model: old-route first-order-curvature ARX transfer. It does not use the old route residual table.",
        "",
        "| model | mean route-aligned RMSE (mm/s) | mean MAE (mm/s) |",
        "|---|---:|---:|",
    ]
    for name, row in summary.iterrows():
        report.append(f"| {name} | {row['route_aligned_rmse_mm_s']:.1f} | {row['route_aligned_mae_mm_s']:.1f} |")
    (out_dir / "report.md").write_text("\n".join(report) + "\n", encoding="utf-8")
    print(json.dumps({"out_dir": str(out_dir), "metrics": summary.to_dict(orient="index")}, indent=2))


if __name__ == "__main__":
    main()
