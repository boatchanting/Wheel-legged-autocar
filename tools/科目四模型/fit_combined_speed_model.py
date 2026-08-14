#!/usr/bin/env python3
"""Fit one common speed model from both routes and evaluate it on each route.

The generated model and all evaluation artefacts are stored in ``model/`` by
default.  The point-table simulations deliberately use no telemetry inputs;
telemetry is used only to identify the common parameters and calculate the
reported evaluation metrics.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


THIS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(THIS_DIR))
import analyze_speed_models as speed_model


DT_S = 0.01
WHEEL_MM_PER_RPM = 4.79


def route_distance(route: pd.DataFrame) -> np.ndarray:
    xy = route[["x", "y"]].to_numpy(float)
    return np.r_[0.0, np.cumsum(np.sqrt(np.sum(np.diff(xy, axis=0) ** 2, axis=1)))]


def wheel_commands(u_rpm: float, curvature: float, track_width_mm: float) -> tuple[float, float]:
    centre = WHEEL_MM_PER_RPM * u_rpm
    return (
        centre * (1.0 - track_width_mm * curvature / 2.0),
        centre * (1.0 + track_width_mm * curvature / 2.0),
    )


def fit_wheel_models(samples: pd.DataFrame, track_width_mm: float) -> tuple[np.ndarray, np.ndarray]:
    """Identify one first-order plant for each wheel from all replay logs."""
    train = samples.copy()
    commands = [
        wheel_commands(u, k, track_width_mm)
        for u, k in zip(train["u_rpm"].to_numpy(float), train["route_curvature"].to_numpy(float))
    ]
    train["cmd_left_mm_s"], train["cmd_right_mm_s"] = zip(*commands)
    result = []
    for side in ("left", "right"):
        y_name = f"wheel_{side}_mm_s"
        u_name = f"cmd_{side}_mm_s"
        blocks, targets = [], []
        for _, frame in train.groupby("file", sort=False):
            y = frame[y_name].to_numpy(float)
            u = frame[u_name].to_numpy(float)
            blocks.append(np.column_stack([np.ones(len(frame) - 1), y[:-1], u[1:]]))
            targets.append(y[1:])
        result.append(np.linalg.lstsq(np.vstack(blocks), np.concatenate(targets), rcond=None)[0])
    return result[0], result[1]


def simulate_differential_drive(
    route: pd.DataFrame,
    track_width_mm: float,
    left_coef: np.ndarray,
    right_coef: np.ndarray,
    turn_oscillator: dict | None = None,
) -> pd.DataFrame:
    """Free-run wheel plants and differential-drive kinematics from a route table.

    When supplied, ``turn_oscillator`` is a damped state excited by changes in
    lateral acceleration above a calibrated threshold.  Its correction is
    applied to each wheel before advancing the differential-drive state.
    """
    distance = route_distance(route)
    u = -route["target_speed_rpm"].to_numpy(float)
    curvature = route["curvature"].to_numpy(float)
    s_mm = psi_rad = v_left = v_right = 0.0
    # Keep the identified motor plants stable and treat the turn transient as
    # an output disturbance, rather than feeding it back into motor memory.
    plant_left = plant_right = 0.0
    q_prev = q_prev2 = previous_turn_load_g = 0.0
    rows = []
    for step in range(100000):
        idx = int(np.searchsorted(distance, s_mm, side="right") - 1)
        idx = min(max(idx, 0), len(route) - 1)
        cmd_left, cmd_right = wheel_commands(u[idx], curvature[idx], track_width_mm)
        v_left_base = max(float(left_coef[0] + left_coef[1] * plant_left + left_coef[2] * cmd_left), 0.0)
        v_right_base = max(float(right_coef[0] + right_coef[1] * plant_right + right_coef[2] * cmd_right), 0.0)
        plant_left, plant_right = v_left_base, v_right_base
        turn_state = 0.0
        if turn_oscillator is not None:
            base_speed = max((v_left_base + v_right_base) / 2.0, 0.0)
            turn_load_g = max(base_speed**2 * abs(curvature[idx]) / 9810.0 - turn_oscillator["threshold_g"], 0.0)
            excitation_g_s = (turn_load_g - previous_turn_load_g) / DT_S
            turn_state = (
                turn_oscillator["pole_1"] * q_prev
                + turn_oscillator["pole_2"] * q_prev2
                + excitation_g_s
            )
            signed_state = np.sign(curvature[idx]) * turn_state
            left_c = turn_oscillator["left_correction"]
            right_c = turn_oscillator["right_correction"]
            v_left_base += left_c[0] + left_c[1] * turn_state + left_c[2] * signed_state
            v_right_base += right_c[0] + right_c[1] * turn_state + right_c[2] * signed_state
            q_prev2, q_prev = q_prev, turn_state
            previous_turn_load_g = turn_load_g
        v_left = max(v_left_base, 0.0)
        v_right = max(v_right_base, 0.0)
        v = (v_left + v_right) / 2.0
        omega = (v_right - v_left) / track_width_mm
        rows.append(
            {
                "time_s": step * DT_S,
                "route_index": idx,
                "distance_mm": s_mm,
                "curvature_plan": curvature[idx],
                "v_left_mm_s": v_left,
                "v_right_mm_s": v_right,
                "predicted_speed_mm_s": v,
                "predicted_yaw_rate_rad_s": omega,
                "predicted_relative_yaw_deg": np.rad2deg(psi_rad),
                "turn_oscillator_state": turn_state,
            }
        )
        s_mm += v * DT_S
        psi_rad += omega * DT_S
        if idx >= len(route) - 1:
            break
    return pd.DataFrame(rows)


def speed_by_route(simulation: pd.DataFrame, route_count: int) -> np.ndarray:
    grouped = simulation.groupby("route_index")["predicted_speed_mm_s"].median()
    return np.interp(np.arange(route_count), grouped.index.to_numpy(), grouped.to_numpy())


def values_by_route(simulation: pd.DataFrame, column: str, route_count: int) -> np.ndarray:
    grouped = simulation.groupby("route_index")[column].median()
    return np.interp(np.arange(route_count), grouped.index.to_numpy(), grouped.to_numpy())


def oscillator_state_by_route(
    route: pd.DataFrame, base_simulation: pd.DataFrame, threshold_g: float, pole_radius: float, frequency_hz: float
) -> np.ndarray:
    """Generate a route-indexed turn-transient state from the base simulation."""
    idx = base_simulation["route_index"].to_numpy(int)
    speed = base_simulation["predicted_speed_mm_s"].to_numpy(float)
    curvature = route["curvature"].to_numpy(float)[idx]
    load_g = np.maximum(speed**2 * np.abs(curvature) / 9810.0 - threshold_g, 0.0)
    excitation = np.r_[0.0, np.diff(load_g)] / DT_S
    pole_1 = 2.0 * pole_radius * np.cos(2.0 * np.pi * frequency_hz * DT_S)
    pole_2 = -(pole_radius**2)
    q = np.zeros(len(base_simulation))
    for i in range(2, len(q)):
        q[i] = pole_1 * q[i - 1] + pole_2 * q[i - 2] + excitation[i]
    grouped = pd.Series(q).groupby(idx).median()
    return np.interp(np.arange(len(route)), grouped.index.to_numpy(), grouped.to_numpy())


def fit_turn_oscillator(
    routes: dict[str, pd.DataFrame], groups: dict[str, pd.DataFrame], base_simulations: dict[str, pd.DataFrame]
) -> dict:
    """Fit a common damped turning-transient correction to wheel residuals.

    The grid search identifies the pole pair from both routes jointly.  For a
    fixed pole pair, left/right correction gains are simple least-squares fits
    against wheel-speed residuals.  No route residual table is retained.
    """
    best: tuple[float, dict] | None = None
    for threshold_g in (0.30, 0.40, 0.50):
        for pole_radius in (0.90, 0.93, 0.95, 0.97, 0.99):
            for frequency_hz in (0.5, 1.0, 2.0, 3.0, 5.0, 8.0):
                states = {
                    name: oscillator_state_by_route(routes[name], base_simulations[name], threshold_g, pole_radius, frequency_hz)
                    for name in routes
                }
                gains: dict[str, list[float]] = {}
                square_error = 0.0
                count = 0
                for side in ("left", "right"):
                    x_blocks, y_blocks = [], []
                    for name, frame in groups.items():
                        idx = frame["route_index"].to_numpy(int)
                        q = states[name][idx]
                        signed_q = np.sign(routes[name]["curvature"].to_numpy(float)[idx]) * q
                        base = values_by_route(base_simulations[name], f"v_{side}_mm_s", len(routes[name]))[idx]
                        x_blocks.append(np.column_stack([np.ones(len(frame)), q, signed_q]))
                        y_blocks.append(frame[f"wheel_{side}_mm_s"].to_numpy(float) - base)
                    coef = np.linalg.lstsq(np.vstack(x_blocks), np.concatenate(y_blocks), rcond=None)[0]
                    gains[side] = coef.tolist()
                    error = np.vstack(x_blocks) @ coef - np.concatenate(y_blocks)
                    square_error += float(np.dot(error, error))
                    count += len(error)
                candidate = {
                    "threshold_g": threshold_g,
                    "pole_radius": pole_radius,
                    "frequency_hz": frequency_hz,
                    "pole_1": 2.0 * pole_radius * np.cos(2.0 * np.pi * frequency_hz * DT_S),
                    "pole_2": -(pole_radius**2),
                    "left_correction": gains["left"],
                    "right_correction": gains["right"],
                }
                score = float(np.sqrt(square_error / count))
                if best is None or score < best[0]:
                    best = score, candidate
    assert best is not None
    best[1]["wheel_fit_rmse_mm_s"] = best[0]
    return best[1]


def route_metric(actual: np.ndarray, route_idx: np.ndarray, prediction: np.ndarray) -> dict[str, float]:
    error = prediction[route_idx] - actual
    return {
        "n": int(len(actual)),
        "rmse_mm_s": float(np.sqrt(np.mean(error**2))),
        "mae_mm_s": float(np.mean(np.abs(error))),
    }


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--group1-dir", type=Path, default=Path("data/nav_mark_points_20260813_204901.csv"))
    ap.add_argument("--group2-dir", type=Path, default=Path("data/nav_mark_points_20260814_153034.csv"))
    ap.add_argument("--out-dir", type=Path, default=THIS_DIR / "model")
    args = ap.parse_args()
    args.out_dir.mkdir(parents=True, exist_ok=True)

    group_dirs = {"group1_20260813": args.group1_dir, "group2_20260814": args.group2_dir}
    routes: dict[str, pd.DataFrame] = {}
    groups: dict[str, pd.DataFrame] = {}
    summaries = []
    for name, data_dir in group_dirs.items():
        route = speed_model.parse_route_table(next(data_dir.glob("nav_replay_route_table*.h")))
        samples, summary = speed_model.load_samples(data_dir, route)
        # File names are unique in the supplied data, but keep group identity
        # explicit so future data sets cannot accidentally share a sequence.
        samples["file"] = name + "__" + samples["file"].astype(str)
        samples["dataset"] = name
        summary["dataset"] = name
        routes[name], groups[name] = route, samples
        summaries.append(summary)

    all_samples = pd.concat(groups.values(), ignore_index=True)
    first_order = speed_model.fit_model(all_samples, "first_order")
    first_order_curvature = speed_model.fit_model(all_samples, "first_order_curvature")
    track_width_mm = speed_model.estimate_track_width(all_samples)
    left_coef, right_coef = fit_wheel_models(all_samples, track_width_mm)
    base_dd = {
        name: simulate_differential_drive(route, track_width_mm, left_coef, right_coef)
        for name, route in routes.items()
    }
    turn_oscillator = fit_turn_oscillator(routes, groups, base_dd)

    metrics_rows = []
    prediction_frames = []
    for name, frame in groups.items():
        route = routes[name]
        route_arx = speed_model.simulate_from_route_table(
            route, first_order_curvature, dt_s=DT_S, initial_speed_mm_s=0.0
        )
        route_dd_base = base_dd[name]
        route_dd = simulate_differential_drive(route, track_width_mm, left_coef, right_coef, turn_oscillator)
        arx_by_route = speed_by_route(route_arx, len(route))
        dd_base_by_route = speed_by_route(route_dd_base, len(route))
        dd_by_route = speed_by_route(route_dd, len(route))
        route_arx.to_csv(args.out_dir / f"{name}_point_table_arx_simulation.csv", index=False)
        route_dd_base.to_csv(args.out_dir / f"{name}_pure_differential_base_simulation.csv", index=False)
        route_dd.to_csv(args.out_dir / f"{name}_pure_differential_simulation.csv", index=False)

        for file_name, run in frame.groupby("file", sort=False):
            y = run["v_body_mm_s"].to_numpy(float)
            route_idx = run["route_index"].to_numpy(int)
            one_step, _ = speed_model.predict_one_step(run, "first_order", first_order)
            recursive = speed_model.predict_recursive(run, "first_order", first_order)
            for model, mode, pred in (
                ("common_first_order", "one_step_telemetry_input", one_step),
                ("common_first_order", "recursive_telemetry_input", recursive),
            ):
                metrics_rows.append(
                    {"dataset": name, "file": file_name, "model": model, "mode": mode, **speed_model.metrics(y, pred)}
                )
            for model, prediction in (
                ("common_first_order_curvature", arx_by_route),
                ("common_pure_differential_drive_base", dd_base_by_route),
                ("common_pure_differential_drive", dd_by_route),
            ):
                metrics_rows.append(
                    {
                        "dataset": name,
                        "file": file_name,
                        "model": model,
                        "mode": "point_table_only_free_run",
                        **route_metric(y, route_idx, prediction),
                    }
                )
            prediction_frames.append(
                pd.DataFrame(
                    {
                        "dataset": name,
                        "file": file_name,
                        "route_index": route_idx,
                        "route_distance_mm": route_distance(route)[route_idx],
                        "actual_speed_mm_s": y,
                        "first_order_recursive_mm_s": recursive,
                        "point_table_arx_mm_s": arx_by_route[route_idx],
                        "pure_differential_base_mm_s": dd_base_by_route[route_idx],
                        "pure_differential_mm_s": dd_by_route[route_idx],
                    }
                )
            )

        fig, ax = plt.subplots(figsize=(12.8, 5.6))
        colors = ["#4E79A7", "#59A14F", "#F28E2B", "#E15759", "#76B7B2"]
        d = route_distance(route)
        for run_no, ((_, run), color) in enumerate(zip(frame.groupby("file", sort=False), colors), start=1):
            ax.plot(d[run["route_index"].to_numpy(int)], run["v_body_mm_s"], color=color, lw=0.8, alpha=0.6, label=f"actual run {run_no}")
        ax.plot(route_arx["distance_mm"], route_arx["predicted_speed_mm_s"], color="#7E57C2", lw=2.5, label="common ARX, point-table only")
        ax.plot(route_dd_base["distance_mm"], route_dd_base["predicted_speed_mm_s"], color="#8A8A8A", lw=1.2, ls="--", label="base differential drive")
        ax.plot(route_dd["distance_mm"], route_dd["predicted_speed_mm_s"], color="#D62728", lw=2.2, label="red: differential drive + turn oscillator")
        ax.set(xlabel="route distance (mm)", ylabel="forward speed (mm/s)", title=f"Common model evaluation: {name}")
        ax.grid(alpha=0.25)
        ax.legend(fontsize=8, ncol=2)
        fig.tight_layout()
        fig.savefig(args.out_dir / f"{name}_common_model_vs_actual.png", dpi=180)
        plt.close(fig)

    metrics_df = pd.DataFrame(metrics_rows)
    metrics_df.to_csv(args.out_dir / "combined_model_group_metrics.csv", index=False)
    pd.concat(prediction_frames, ignore_index=True).to_csv(args.out_dir / "combined_model_predictions.csv", index=False)
    pd.concat(summaries, ignore_index=True).to_csv(args.out_dir / "combined_model_run_summary.csv", index=False)

    dt_s = float(all_samples["dt_s"].median())
    model = {
        "training": {
            "datasets": {name: str(path) for name, path in group_dirs.items()},
            "auto_replay_rows": int(len(all_samples)),
            "sampling_period_s": dt_s,
            "speed_definition": "v_mm_s = -vx_body",
            "input_definition": "u_rpm = -target_speed_set",
        },
        "common_first_order": {
            "equation": "v[k] = c + a*v[k-1] + b*u[k]",
            "coefficients_c_a_b": first_order.tolist(),
            "time_constant_s": float(-dt_s / np.log(first_order[1])),
        },
        "common_first_order_curvature": {
            "equation": "v[k] = c + a*v[k-1] + b*u[k] + b_kappa*u[k]*abs(kappa[k]) + b_du*du[k]",
            "coefficients_c_a_b_b_kappa_b_du": first_order_curvature.tolist(),
        },
        "common_pure_differential_drive": {
            "track_width_mm": float(track_width_mm),
            "left_wheel_equation": "vL[k] = cL + aL*vL[k-1] + bL*vL_cmd[k]",
            "left_coefficients_c_a_b": left_coef.tolist(),
            "right_wheel_equation": "vR[k] = cR + aR*vR[k-1] + bR*vR_cmd[k]",
            "right_coefficients_c_a_b": right_coef.tolist(),
            "initial_state": {"v_left_mm_s": 0.0, "v_right_mm_s": 0.0, "relative_yaw_deg": 0.0},
            "turn_oscillator": {
                "equation": "q[k] = pole_1*q[k-1] + pole_2*q[k-2] + d(max(v_base^2*abs(curvature)/9810-threshold_g, 0))/dt; wheel correction = c0 + c1*q + c2*sign(curvature)*q",
                **turn_oscillator,
            },
        },
    }
    (args.out_dir / "combined_speed_model.json").write_text(json.dumps(model, indent=2), encoding="utf-8")

    aggregate = metrics_df.groupby(["dataset", "model", "mode"])[["rmse_mm_s", "mae_mm_s"]].mean().reset_index()
    lines = [
        "# 两组共同速度模型",
        "",
        f"- 训练数据：两组自动回放片段合计 {len(all_samples):,} 帧，采样周期中位数 {dt_s * 1000:.1f} ms。",
        "- 评估复用了训练样本，因此这里反映的是共同参数对两条路线的拟合一致性，不是独立泛化验证。",
        "- `point_table_only_free_run` 仅使用对应路线点表；其余两项使用日志的实时目标转速，递推项以每段首帧实测速度初始化。",
        f"- 红色模型在横向加速度超过 {turn_oscillator['threshold_g']:.2f} g 后，以 {turn_oscillator['frequency_hz']:.1f} Hz、极点半径 {turn_oscillator['pole_radius']:.2f} 的阻尼状态响应横向加速度变化；该状态直接修正左右轮速度。",
        "",
        "| 数据组 | 模型 | 模式 | RMSE (mm/s) | MAE (mm/s) |",
        "|---|---|---|---:|---:|",
    ]
    for _, row in aggregate.iterrows():
        lines.append(f"| {row['dataset']} | {row['model']} | {row['mode']} | {row['rmse_mm_s']:.1f} | {row['mae_mm_s']:.1f} |")
    (args.out_dir / "combined_model_report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"model": str(args.out_dir / "combined_speed_model.json"), "metrics": aggregate.to_dict(orient="records")}, indent=2))


if __name__ == "__main__":
    main()
