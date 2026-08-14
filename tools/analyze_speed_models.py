#!/usr/bin/env python3
"""Identify and compare speed models from replay telemetry and a route table."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


WHEEL_MM_PER_RPM = 4.79


def parse_route_table(path: Path) -> pd.DataFrame:
    text = path.read_text(encoding="utf-8", errors="ignore")
    rows = []
    for raw in re.findall(r"\{([^{}]+)\}", text):
        vals = [v.strip().replace("(uint8)", "").rstrip("f") for v in raw.split(",")]
        if len(vals) < 7:
            continue
        try:
            rows.append(
                {
                    "route_index": len(rows),
                    "x": float(vals[0]),
                    "y": float(vals[1]),
                    "target_yaw_deg": float(vals[2]),
                    "heading_deg": float(vals[3]),
                    "point_type": int(float(vals[4])),
                    "target_speed_rpm": float(vals[5]),
                    "curvature": float(vals[6]),
                }
            )
        except ValueError:
            continue
    route = pd.DataFrame(rows)
    if len(route) < 10:
        raise ValueError(f"Could not parse route points from {path}")
    return route


def load_samples(data_dir: Path, route: pd.DataFrame) -> tuple[pd.DataFrame, pd.DataFrame]:
    route_xy = route[["x", "y"]].to_numpy(float)
    all_rows = []
    summaries = []
    for path in sorted(data_dir.glob("wifi_telemetry_*.csv")):
        df = pd.read_csv(path)
        auto = df.loc[df["g_replay_state"] == 1].copy()
        if auto.empty:
            continue
        dt_ms = auto["loop"].diff().fillna(auto["loop"].diff().median()).to_numpy(float)
        dt_s = np.maximum(dt_ms, 1.0) / 1000.0
        pts = auto[["nav_x", "nav_y"]].to_numpy(float)
        # The route is traversed monotonically, and nearest (x, y) is robust to
        # the documented vision-fusion jumps. Keep the raw nearest-point error.
        d2 = ((pts[:, None, :] - route_xy[None, :, :]) ** 2).sum(axis=2)
        route_idx = d2.argmin(axis=1)
        err_mm = np.sqrt(d2[np.arange(len(auto)), route_idx])
        auto["file"] = path.name
        auto["t_s"] = (auto["loop"].to_numpy(float) - float(auto["loop"].iloc[0])) / 1000.0
        auto["dt_s"] = dt_s
        auto["route_index"] = route_idx
        auto["route_match_error_mm"] = err_mm
        auto["route_x"] = route.iloc[route_idx]["x"].to_numpy()
        auto["route_y"] = route.iloc[route_idx]["y"].to_numpy()
        auto["route_curvature"] = route.iloc[route_idx]["curvature"].to_numpy()
        auto["route_target_speed_rpm"] = route.iloc[route_idx]["target_speed_rpm"].to_numpy()
        auto["point_type_mapped"] = route.iloc[route_idx]["point_type"].to_numpy()
        # Body X is negative while moving forward in these logs.
        auto["u_rpm"] = -auto["target_speed_set"].astype(float)
        auto["v_body_mm_s"] = -auto["vx_body"].astype(float)
        auto["wheel_left_mm_s"] = WHEEL_MM_PER_RPM * auto["speed_L"].astype(float)
        auto["wheel_right_mm_s"] = -WHEEL_MM_PER_RPM * auto["speed_R"].astype(float)
        auto["wheel_delta_mm_s"] = auto["wheel_right_mm_s"] - auto["wheel_left_mm_s"]
        auto["v_wheel_mm_s"] = (auto["wheel_left_mm_s"] + auto["wheel_right_mm_s"]) / 2.0
        # Unwrap relative_yaw before differentiating; it is a wrapped degree
        # signal and otherwise produces false +/-360 degree/s spikes.
        yaw_rad = np.unwrap(np.deg2rad(auto["relative_yaw"].to_numpy(float)))
        yaw_rate = np.gradient(yaw_rad, float(np.median(dt_s)))
        auto["yaw_rate_rad_s"] = pd.Series(yaw_rate).rolling(5, center=True, min_periods=1).median().to_numpy()
        auto["wheel_curvature"] = auto["yaw_rate_rad_s"] / np.maximum(auto["v_wheel_mm_s"].abs(), 500.0)
        auto["du_rpm_s"] = auto["u_rpm"].diff().fillna(0.0) / auto["dt_s"]
        auto["abs_curvature"] = auto["route_curvature"].abs()
        auto["u_times_abs_curvature"] = auto["u_rpm"] * auto["abs_curvature"]
        summaries.append(
            {
                "file": path.name,
                "n_auto": len(auto),
                "loop_start": int(auto["loop"].iloc[0]),
                "loop_end": int(auto["loop"].iloc[-1]),
                "duration_s": float(auto["t_s"].iloc[-1]),
                "route_index_start": int(route_idx[0]),
                "route_index_end": int(route_idx[-1]),
                "route_match_error_median_mm": float(np.median(err_mm)),
                "route_match_error_p95_mm": float(np.quantile(err_mm, 0.95)),
                "u_rpm_mean": float(auto["u_rpm"].mean()),
                "v_body_mean_mm_s": float(auto["v_body_mm_s"].mean()),
                "v_wheel_body_bias_mm_s": float((auto["v_wheel_mm_s"] - auto["v_body_mm_s"]).mean()),
            }
        )
        all_rows.append(auto)
    if not all_rows:
        raise ValueError(f"No g_replay_state == 1 rows found under {data_dir}")
    return pd.concat(all_rows, ignore_index=True), pd.DataFrame(summaries)


MODEL_FEATURES = {
    "static_gain": ["one", "u"],
    "static_curvature": ["one", "u", "u_kappa"],
    "first_order": ["one", "y_prev", "u"],
    "first_order_curvature": ["one", "y_prev", "u", "u_kappa", "du"],
    "second_order_arx": ["one", "y_prev", "y_prev2", "u", "u_prev", "du"],
}


def design(frame: pd.DataFrame, model: str, use_true_y: bool = True) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    y = frame["v_body_mm_s"].to_numpy(float)
    u = frame["u_rpm"].to_numpy(float)
    kappa = frame["abs_curvature"].to_numpy(float)
    du = frame["du_rpm_s"].to_numpy(float)
    yp = np.roll(y, 1)
    yp2 = np.roll(y, 2)
    up = np.roll(u, 1)
    valid = np.ones(len(frame), dtype=bool)
    if model in {"first_order", "first_order_curvature"}:
        valid[0] = False
    if model == "second_order_arx":
        valid[:2] = False
    if not use_true_y:
        # The caller supplies recursive predictions separately; this path is
        # retained to make feature naming explicit.
        raise NotImplementedError
    cols = {
        "one": np.ones(len(frame)),
        "u": u,
        "u_kappa": u * kappa,
        "y_prev": yp,
        "y_prev2": yp2,
        "u_prev": up,
        "du": du,
    }
    X = np.column_stack([cols[name] for name in MODEL_FEATURES[model]])
    return X[valid], y[valid], valid


def fit_model(train: pd.DataFrame, model: str) -> np.ndarray:
    blocks = []
    targets = []
    for _, frame in train.groupby("file", sort=False):
        x, y, _ = design(frame, model)
        blocks.append(x)
        targets.append(y)
    return np.linalg.lstsq(np.vstack(blocks), np.concatenate(targets), rcond=None)[0]


def predict_one_step(frame: pd.DataFrame, model: str, coef: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    X, y, valid = design(frame, model)
    pred = np.full(len(frame), np.nan)
    pred[valid] = X @ coef
    return pred, valid


def predict_recursive(frame: pd.DataFrame, model: str, coef: np.ndarray) -> np.ndarray:
    u = frame["u_rpm"].to_numpy(float)
    kappa = frame["abs_curvature"].to_numpy(float)
    du = frame["du_rpm_s"].to_numpy(float)
    y = frame["v_body_mm_s"].to_numpy(float)
    pred = np.full(len(frame), np.nan)
    if model.startswith("static"):
        cols = [np.ones(len(frame)), u]
        if model == "static_curvature":
            cols.append(u * kappa)
        return np.column_stack(cols) @ coef
    pred[0] = y[0]
    if model == "second_order_arx" and len(frame) > 1:
        pred[1] = y[1]
    start = 1 if model != "second_order_arx" else 2
    for i in range(start, len(frame)):
        vals = {
            "one": 1.0,
            "u": u[i],
            "u_kappa": u[i] * kappa[i],
            "y_prev": pred[i - 1],
            "y_prev2": pred[i - 2],
            "u_prev": u[i - 1],
            "du": du[i],
        }
        pred[i] = sum(c * vals[name] for c, name in zip(coef, MODEL_FEATURES[model]))
    return pred


def metrics(y: np.ndarray, pred: np.ndarray) -> dict[str, float]:
    mask = np.isfinite(y) & np.isfinite(pred)
    e = pred[mask] - y[mask]
    rmse = float(np.sqrt(np.mean(e**2)))
    mae = float(np.mean(np.abs(e)))
    ss_tot = float(np.sum((y[mask] - np.mean(y[mask])) ** 2))
    r2 = float(1.0 - np.sum(e**2) / ss_tot) if ss_tot > 0 else float("nan")
    return {"n": int(mask.sum()), "rmse_mm_s": rmse, "mae_mm_s": mae, "r2": r2}


def simulate_from_route_table(
    route: pd.DataFrame,
    coef: np.ndarray,
    dt_s: float = 0.01,
    initial_speed_mm_s: float = 0.0,
    max_steps: int = 100000,
    route_correction_mm_s: np.ndarray | None = None,
) -> pd.DataFrame:
    """Free-run a speed model using only route-table values.

    The route point index advances from the predicted distance, so no telemetry
    target-speed or measured-speed samples are used. The first-order curvature
    model needs a target-speed derivative; it is calculated from consecutive
    point-table values at the simulation time step.
    """
    xy = route[["x", "y"]].to_numpy(float)
    ds = np.sqrt(np.sum(np.diff(xy, axis=0) ** 2, axis=1))
    s_route = np.r_[0.0, np.cumsum(ds)]
    u_route = -route["target_speed_rpm"].to_numpy(float)
    k_route = route["curvature"].abs().to_numpy(float)
    rows = []
    t = 0.0
    s = 0.0
    v = max(float(initial_speed_mm_s), 0.0)
    prev_u = u_route[0]
    for _ in range(max_steps):
        idx = int(np.searchsorted(s_route, s, side="right") - 1)
        idx = max(0, min(idx, len(route) - 1))
        u = float(u_route[idx])
        kappa = float(k_route[idx])
        du = (u - prev_u) / dt_s
        vals = {
            "one": 1.0,
            "y_prev": v,
            "y_prev2": v,
            "u": u,
            "u_prev": prev_u,
            "u_kappa": u * kappa,
            "du": du,
        }
        # The route-only reconstruction uses the calibrated first-order
        # curvature model at the same 10 ms step as the telemetry logs.
        v_next = sum(c * vals[name] for c, name in zip(coef, MODEL_FEATURES["first_order_curvature"]))
        v_next = max(float(v_next), 0.0)
        if route_correction_mm_s is not None:
            v_next = max(v_next + float(route_correction_mm_s[idx]), 0.0)
        rows.append(
            {
                "time_s": t,
                "route_index": idx,
                "distance_mm": s,
                "target_speed_rpm": -u,
                "target_speed_magnitude_rpm": u,
                "curvature": kappa,
                "predicted_speed_mm_s": v_next,
            }
        )
        s += v_next * dt_s
        t += dt_s
        prev_u = u
        v = v_next
        if idx >= len(route) - 1:
            break
    return pd.DataFrame(rows)


def estimate_track_width(samples: pd.DataFrame) -> float:
    mask = (
        samples["yaw_rate_rad_s"].abs().between(0.05, 3.0)
        & samples["wheel_delta_mm_s"].abs().between(1.0, 1600.0)
    )
    om = samples.loc[mask, "yaw_rate_rad_s"].to_numpy(float)
    dv = samples.loc[mask, "wheel_delta_mm_s"].to_numpy(float)
    if len(om) < 20:
        return float("nan")
    ratio = dv / om
    keep = np.isfinite(ratio) & (ratio > 100.0) & (ratio < 350.0)
    if keep.sum() < 20:
        return float(np.dot(om, dv) / np.dot(om, om))
    # Trim slip/fusion outliers before applying v_R-v_L = B*omega.
    return float(np.dot(om[keep], dv[keep]) / np.dot(om[keep], om[keep]))


def build_route_curve_correction(
    samples: pd.DataFrame, route_only: pd.DataFrame, route_count: int, window: int = 5
) -> np.ndarray:
    """Build a route-specific, robust curve correction from wheel speed.

    The correction is deliberately a separate calibration layer: it captures
    repeatable track/steering effects on this route that a global longitudinal
    ARX model cannot infer from target speed alone.
    """
    base_by_route = route_only.groupby("route_index")["predicted_speed_mm_s"].median()
    base = np.interp(
        samples["route_index"].to_numpy(),
        base_by_route.index.to_numpy(),
        base_by_route.to_numpy(),
    )
    residual = samples["v_wheel_mm_s"].to_numpy(float) - base
    raw = np.zeros(route_count, dtype=float)
    for idx, values in pd.Series(residual).groupby(samples["route_index"].to_numpy()):
        raw[int(idx)] = float(np.median(values))
    return pd.Series(raw).rolling(window, center=True, min_periods=1).median().to_numpy()


def fit_first100_constrained(frame: pd.DataFrame, pole: float, ridge: float = 10.0) -> np.ndarray:
    """Fit offset and input gain from a short prefix while fixing the pole."""
    h = frame.iloc[: min(100, len(frame))]
    y = h["v_body_mm_s"].to_numpy(float)[1:]
    yp = h["v_body_mm_s"].to_numpy(float)[:-1]
    u = h["u_rpm"].to_numpy(float)[1:]
    X = np.column_stack([np.ones(len(y)), u])
    z = y - pole * yp
    # Prior is the all-data first-order equilibrium input term. This is a
    # stable short-window calibration rather than an unconstrained 3-parameter fit.
    prior = np.array([-8.5954072, 0.08472334])
    return np.linalg.solve(X.T @ X + ridge * np.eye(2), X.T @ z + ridge * prior)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--data-dir", type=Path, default=Path("data/nav_mark_points_20260813_204901.csv"))
    ap.add_argument("--route", type=Path, default=None)
    ap.add_argument("--out-dir", type=Path, default=Path("results/speed_model_analysis_20260814"))
    args = ap.parse_args()
    if args.route is None:
        args.route = next(args.data_dir.glob("nav_replay_route_table*.h"))
    args.out_dir.mkdir(parents=True, exist_ok=True)

    route = parse_route_table(args.route)
    samples, summary = load_samples(args.data_dir, route)
    route.to_csv(args.out_dir / "route_points_parsed.csv", index=False)
    samples.to_csv(args.out_dir / "auto_samples_enriched.csv", index=False)
    summary.to_csv(args.out_dir / "run_summary.csv", index=False)

    files = summary["file"].tolist()
    rows = []
    coefficients = {}
    prediction_example = None
    for model in MODEL_FEATURES:
        fold_coefs = {}
        for test_file in files:
            train = samples.loc[samples["file"] != test_file]
            test = samples.loc[samples["file"] == test_file]
            coef = fit_model(train, model)
            one, valid = predict_one_step(test, model, coef)
            rec = predict_recursive(test, model, coef)
            m1 = metrics(test["v_body_mm_s"].to_numpy(float), one)
            mr = metrics(test["v_body_mm_s"].to_numpy(float), rec)
            rows.append({"model": model, "test_file": test_file, "mode": "one_step", **m1})
            rows.append({"model": model, "test_file": test_file, "mode": "recursive", **mr})
            fold_coefs[test_file] = coef.tolist()
            if model == "first_order_curvature" and prediction_example is None:
                prediction_example = (test.copy(), one, rec)
        coefficients[model] = fold_coefs
    metric_df = pd.DataFrame(rows)
    metric_df.to_csv(args.out_dir / "model_metrics_leave_one_file_out.csv", index=False)
    (args.out_dir / "model_coefficients.json").write_text(json.dumps(coefficients, indent=2), encoding="utf-8")

    # Fit all logs for a compact model comparison and route/speed overview.
    all_fit = {}
    reconstruction = samples[["file", "loop", "t_s", "route_index", "u_rpm", "v_body_mm_s"]].copy()
    for model in MODEL_FEATURES:
        coef = fit_model(samples, model)
        all_fit[model] = coef.tolist()
        for file_name, frame in samples.groupby("file", sort=False):
            idx = frame.index.to_numpy()
            one, _ = predict_one_step(frame, model, coef)
            rec = predict_recursive(frame, model, coef)
            reconstruction.loc[idx, f"{model}_one_step"] = one
            reconstruction.loc[idx, f"{model}_recursive"] = rec
    (args.out_dir / "model_coefficients_all_data.json").write_text(json.dumps(all_fit, indent=2), encoding="utf-8")
    reconstruction.to_csv(args.out_dir / "speed_curve_reconstruction_all_data_fit.csv", index=False)

    route_only = simulate_from_route_table(route, np.asarray(all_fit["first_order_curvature"], dtype=float))
    route_only.to_csv(args.out_dir / "point_table_only_speed_prediction.csv", index=False)
    track_width_mm = estimate_track_width(samples)
    samples["yaw_rate_prev_rad_s"] = samples.groupby("file")["yaw_rate_rad_s"].shift(1)
    samples["v_body_prev_mm_s"] = samples.groupby("file")["v_body_mm_s"].shift(1)
    yaw_fit = samples.dropna(subset=["yaw_rate_prev_rad_s", "v_body_prev_mm_s"])
    yaw_X = np.column_stack(
        [
            np.ones(len(yaw_fit)),
            yaw_fit["yaw_rate_prev_rad_s"].to_numpy(float),
            yaw_fit["v_body_prev_mm_s"].to_numpy(float) * yaw_fit["route_curvature"].to_numpy(float),
        ]
    )
    yaw_coef = np.linalg.lstsq(yaw_X, yaw_fit["yaw_rate_rad_s"].to_numpy(float), rcond=None)[0]
    (args.out_dir / "yaw_state_model_coefficients.json").write_text(
        json.dumps(
            {
                "equation": "omega[k] = c_omega + a_omega*omega[k-1] + b_omega*v[k-1]*curvature_plan[k]",
                "coefficients": yaw_coef.tolist(),
                "track_width_mm": track_width_mm,
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    route_curve_correction = build_route_curve_correction(samples, route_only, len(route), window=5)
    # Apply the route correction to the base curve after propagation. The
    # correction is a calibrated output residual, not an additional physical
    # acceleration term; feeding it back into distance propagation would make
    # large startup residuals incorrectly stall the simulated vehicle.
    route_corrected = route_only.copy()
    route_corrected["predicted_speed_mm_s"] = np.maximum(
        route_corrected["predicted_speed_mm_s"].to_numpy(float)
        + route_curve_correction[route_corrected["route_index"].to_numpy(int)],
        0.0,
    )
    route_corrected.to_csv(args.out_dir / "point_table_curve_corrected_speed_prediction.csv", index=False)
    pd.DataFrame(
        {
            "route_index": np.arange(len(route)),
            "route_correction_mm_s": route_curve_correction,
            "route_curvature": route["curvature"].to_numpy(),
            "target_speed_rpm": route["target_speed_rpm"].to_numpy(),
        }
    ).to_csv(args.out_dir / "route_curve_correction.csv", index=False)

    # Leave-one-log-out validation for the route correction. This separates the
    # optimistic all-log visual calibration from its cross-log generalization.
    correction_cv_rows = []
    for test_file in files:
        train = samples.loc[samples["file"] != test_file]
        test = samples.loc[samples["file"] == test_file]
        train_coef = fit_model(train, "first_order_curvature")
        train_base = simulate_from_route_table(route, train_coef)
        train_corr = build_route_curve_correction(train, train_base, len(route), window=5)
        base_by_route_cv = train_base.groupby("route_index")["predicted_speed_mm_s"].median()
        base_values_cv = np.interp(
            test["route_index"].to_numpy(),
            base_by_route_cv.index.to_numpy(),
            base_by_route_cv.to_numpy(),
        )
        corrected_values_cv = base_values_cv + train_corr[test["route_index"].to_numpy()]
        y_test = test["v_body_mm_s"].to_numpy(float)
        correction_cv_rows.append(
            {
                "test_file": test_file,
                "base_rmse_mm_s": float(np.sqrt(np.mean((base_values_cv - y_test) ** 2))),
                "corrected_rmse_mm_s": float(np.sqrt(np.mean((corrected_values_cv - y_test) ** 2))),
                "base_mae_mm_s": float(np.mean(np.abs(base_values_cv - y_test))),
                "corrected_mae_mm_s": float(np.mean(np.abs(corrected_values_cv - y_test))),
            }
        )
    correction_cv = pd.DataFrame(correction_cv_rows)
    correction_cv.to_csv(args.out_dir / "curve_correction_leave_one_file_out.csv", index=False)
    route_xy = route[["x", "y"]].to_numpy(float)
    route_distance = np.r_[0.0, np.cumsum(np.sqrt(np.sum(np.diff(route_xy, axis=0) ** 2, axis=1)))]

    # Differential-drive diagnostic: planned curvature versus curvature inferred
    # from relative_yaw and wheel speed. Curves are plotted in route distance.
    measured_kappa = samples.groupby("route_index")["wheel_curvature"].median()
    fig, ax1 = plt.subplots(figsize=(12, 5))
    ax1.plot(route_distance, route["curvature"], color="tab:blue", lw=1.2, label="planned curvature")
    ax1.plot(
        route_distance[measured_kappa.index.to_numpy()],
        measured_kappa.to_numpy(),
        color="tab:red",
        lw=1.0,
        alpha=0.85,
        label="measured curvature from yaw/wheels",
    )
    ax1.set(xlabel="route distance (mm)", ylabel="curvature (1/mm)", title="Differential-drive curvature diagnostic")
    ax1.grid(alpha=0.25)
    ax1.legend()
    fig.tight_layout()
    fig.savefig(args.out_dir / "differential_drive_curvature_diagnostics.png", dpi=160)
    plt.close(fig)

    # Short-prefix adaptation experiment: use the first 100 measured samples
    # only to estimate the input gain/offset, then replace future target speed
    # with the route-table value and recursively forecast the rest.
    global_first = np.asarray(all_fit["first_order"], dtype=float)
    prefix_rows = []
    prefix_metrics = []
    for file_name, frame in samples.groupby("file", sort=False):
        n_prefix = min(100, len(frame))
        adapt = fit_first100_constrained(frame, global_first[1])
        future = frame.iloc[n_prefix - 1 :].copy()
        route_idx = future["route_index"].to_numpy(int)
        future["u_rpm"] = -route.iloc[route_idx]["target_speed_rpm"].to_numpy(float)
        future["du_rpm_s"] = future["u_rpm"].diff().fillna(0.0) / future["dt_s"]
        coef_short = np.array([adapt[0], global_first[1], adapt[1]])
        pred = predict_recursive(future, "first_order", coef_short)
        prefix_rows.append(
            pd.DataFrame(
                {
                    "file": file_name,
                    "sample_from_run": np.arange(n_prefix - 1, len(frame)),
                    "route_index": route_idx,
                    "route_distance_mm": route_distance[route_idx],
                    "v_actual_mm_s": future["v_body_mm_s"].to_numpy(float),
                    "v_predicted_mm_s": pred,
                    "phase": ["prefix_last"] + ["forecast"] * (len(future) - 1),
                }
            )
        )
        m = metrics(future["v_body_mm_s"].to_numpy(float)[1:], pred[1:])
        prefix_metrics.append(
            {
                "file": file_name,
                "prefix_steps": n_prefix,
                "adapt_offset": float(adapt[0]),
                "adapt_gain_mm_s_per_rpm": float(adapt[1]),
                "fixed_pole": float(global_first[1]),
                "forecast_rmse_mm_s": m["rmse_mm_s"],
                "forecast_mae_mm_s": m["mae_mm_s"],
                "forecast_r2": m["r2"],
            }
        )
    prefix_df = pd.concat(prefix_rows, ignore_index=True)
    prefix_df.to_csv(args.out_dir / "first100_prefix_forecast.csv", index=False)
    pd.DataFrame(prefix_metrics).to_csv(args.out_dir / "first100_prefix_forecast_metrics.csv", index=False)

    # Compare the point-table-only curve with each original vx trace in route
    # distance coordinates. This avoids misalignment caused by different run
    # durations and makes the comparison about the same physical route.
    pred_by_route = route_only.groupby("route_index")["predicted_speed_mm_s"].median()
    pred_route_values = np.interp(np.arange(len(route)), pred_by_route.index.to_numpy(), pred_by_route.to_numpy())
    comparison_rows = []
    for file_name, frame in samples.groupby("file", sort=False):
        pred = pred_route_values[frame["route_index"].to_numpy()]
        corrected = route_curve_correction[frame["route_index"].to_numpy()] + pred
        comparison_rows.append(
            {
                "file": file_name,
                "n": len(frame),
                "route_aligned_rmse_mm_s": float(np.sqrt(np.mean((pred - frame["v_body_mm_s"].to_numpy()) ** 2))),
                "route_aligned_mae_mm_s": float(np.mean(np.abs(pred - frame["v_body_mm_s"].to_numpy()))),
                "curve_corrected_rmse_mm_s": float(np.sqrt(np.mean((corrected - frame["v_body_mm_s"].to_numpy()) ** 2))),
                "curve_corrected_mae_mm_s": float(np.mean(np.abs(corrected - frame["v_body_mm_s"].to_numpy()))),
            }
        )
    pd.DataFrame(comparison_rows).to_csv(args.out_dir / "point_table_vs_original_vx_metrics.csv", index=False)

    # Per-run first-order parameters expose the plant variation that makes a
    # single recursive model less accurate than its one-step fit.
    parameter_rows = []
    for file_name, frame in samples.groupby("file", sort=False):
        coef = fit_model(frame, "first_order")
        a = float(coef[1])
        dt = float(frame["dt_s"].median())
        tau = float(-dt / np.log(a)) if 0.0 < a < 1.0 else float("nan")
        gain = float(coef[2] / (1.0 - a))
        eq_offset = float(coef[0] / (1.0 - a))
        parameter_rows.append(
            {
                "file": file_name,
                "intercept": float(coef[0]),
                "a_y_prev": a,
                "b_u": float(coef[2]),
                "tau_s": tau,
                "steady_state_gain_mm_s_per_rpm": gain,
                "steady_state_offset_mm_s": eq_offset,
                "one_step_rmse_mm_s": metrics(frame["v_body_mm_s"].to_numpy(float), predict_one_step(frame, "first_order", coef)[0])["rmse_mm_s"],
            }
        )
    pd.DataFrame(parameter_rows).to_csv(args.out_dir / "first_order_parameters_by_run.csv", index=False)

    # In-sample comparison: fit the primary first-order-curvature model
    # independently on each log, then replay that same log with its own
    # coefficients. This shows calibration fit quality, not cross-log
    # generalization.
    per_file_rows = []
    n_files = len(files)
    ncols = 1
    nrows = n_files
    fig, axes = plt.subplots(nrows=nrows, ncols=ncols, figsize=(13, max(3.0 * nrows, 8.0)), squeeze=False)
    for row_no, file_name in enumerate(files):
        frame = samples.loc[samples["file"] == file_name].copy()
        coef = fit_model(frame, "first_order_curvature")
        one, _ = predict_one_step(frame, "first_order_curvature", coef)
        rec = predict_recursive(frame, "first_order_curvature", coef)
        y = frame["v_body_mm_s"].to_numpy(float)
        m_one = metrics(y, one)
        m_rec = metrics(y, rec)
        per_file_rows.append(
            {
                "file": file_name,
                "mode": "one_step",
                "rmse_mm_s": m_one["rmse_mm_s"],
                "mae_mm_s": m_one["mae_mm_s"],
                "r2": m_one["r2"],
                "c": coef[0],
                "a_y_prev": coef[1],
                "b_u": coef[2],
                "b_u_kappa": coef[3],
                "b_du": coef[4],
            }
        )
        per_file_rows.append(
            {
                "file": file_name,
                "mode": "recursive",
                "rmse_mm_s": m_rec["rmse_mm_s"],
                "mae_mm_s": m_rec["mae_mm_s"],
                "r2": m_rec["r2"],
                "c": coef[0],
                "a_y_prev": coef[1],
                "b_u": coef[2],
                "b_u_kappa": coef[3],
                "b_du": coef[4],
            }
        )
        ax = axes[row_no, 0]
        ax.plot(frame["t_s"], y, lw=0.8, label="measured -vx_body")
        ax.plot(frame["t_s"], one, lw=0.8, label="one-step")
        ax.plot(frame["t_s"], rec, lw=1.0, label="recursive")
        ax.set_ylabel("mm/s")
        ax.set_title(f"run {row_no + 1}: {file_name} | one-step RMSE {m_one['rmse_mm_s']:.1f}, recursive RMSE {m_rec['rmse_mm_s']:.1f} mm/s")
        ax.grid(alpha=0.25)
        if row_no == 0:
            ax.legend(loc="upper left", fontsize=8)
    axes[-1, 0].set_xlabel("time from replay segment (s)")
    fig.suptitle("Per-file calibrated first-order-curvature model", y=0.995)
    fig.tight_layout(rect=(0, 0, 1, 0.985))
    fig.savefig(args.out_dir / "per_file_calibrated_comparison.png", dpi=160)
    plt.close(fig)
    pd.DataFrame(per_file_rows).to_csv(args.out_dir / "per_file_calibrated_metrics.csv", index=False)

    # Plot one representative held-out run and the geometry-dependent target.
    if prediction_example is not None:
        test, one, rec = prediction_example
        fig, ax = plt.subplots(figsize=(12, 5))
        ax.plot(test["t_s"], test["v_body_mm_s"], lw=1.0, label="measured -vx_body")
        ax.plot(test["t_s"], one, lw=1.0, label="first_order_curvature one-step")
        ax.plot(test["t_s"], rec, lw=1.2, label="first_order_curvature recursive")
        ax.set(xlabel="time from replay segment (s)", ylabel="forward speed (mm/s)", title=f"Held-out speed model: {test['file'].iloc[0]}")
        ax.grid(alpha=0.25)
        ax.legend()
        fig.tight_layout()
        fig.savefig(args.out_dir / "heldout_speed_prediction.png", dpi=160)
        plt.close(fig)

    fig, ax1 = plt.subplots(figsize=(12, 5))
    ax1.plot(route["route_index"], -route["target_speed_rpm"], color="tab:blue", label="target speed magnitude (RPM)")
    ax1.set(xlabel="route point index", ylabel="target speed magnitude (RPM)")
    ax2 = ax1.twinx()
    ax2.plot(route["route_index"], route["curvature"].abs(), color="tab:red", alpha=0.75, label="|curvature|")
    ax2.set_ylabel("absolute curvature (1/mm)")
    ax1.grid(alpha=0.25)
    lines = ax1.get_lines() + ax2.get_lines()
    ax1.legend(lines, [line.get_label() for line in lines], loc="best")
    fig.tight_layout()
    fig.savefig(args.out_dir / "route_target_and_curvature.png", dpi=160)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(12, 5))
    ax.plot(route_only["time_s"], route_only["predicted_speed_mm_s"], color="tab:green", lw=1.4)
    ax.set(
        xlabel="simulated time from route start (s)",
        ylabel="predicted forward speed (mm/s)",
        title="Point-table-only speed prediction (initial speed = 0)",
    )
    ax.grid(alpha=0.25)
    fig.tight_layout()
    fig.savefig(args.out_dir / "point_table_only_speed_prediction.png", dpi=160)
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(12, 5))
    for run_no, (file_name, frame) in enumerate(samples.groupby("file", sort=False), start=1):
        ax.plot(
            route_distance[frame["route_index"].to_numpy()],
            frame["v_body_mm_s"],
            lw=0.8,
            alpha=0.42,
            label=f"original -vx_body: run {run_no}",
        )
    ax.plot(
        route_only["distance_mm"],
        route_only["predicted_speed_mm_s"],
        color="black",
        lw=2.0,
        label="point-table-only base model",
    )
    ax.plot(
        route_corrected["distance_mm"],
        route_corrected["predicted_speed_mm_s"],
        color="tab:purple",
        lw=2.0,
        label="point-table-only + route curve correction",
    )
    ax.set(
        xlabel="route distance from point-table start (mm)",
        ylabel="forward speed (mm/s)",
        title="Point-table-only prediction vs original -vx_body",
    )
    ax.grid(alpha=0.25)
    ax.legend(fontsize=8, ncol=2)
    fig.tight_layout()
    fig.savefig(args.out_dir / "point_table_vs_original_vx.png", dpi=160)
    plt.close(fig)

    # Presentation view requested for judging the calibrated prediction alone:
    # original traces are deliberately subordinate to the purple prediction.
    fig, ax = plt.subplots(figsize=(12.5, 5.4))
    trace_colors = ["#4E79A7", "#59A14F", "#F28E2B", "#E15759", "#76B7B2"]
    for run_no, ((_, frame), color) in enumerate(
        zip(samples.groupby("file", sort=False), trace_colors), start=1
    ):
        ax.plot(
            route_distance[frame["route_index"].to_numpy()],
            frame["v_body_mm_s"],
            color=color,
            lw=0.85,
            alpha=0.62,
            label=f"original -vx_body: run {run_no}",
        )
    ax.plot(
        route_corrected["distance_mm"],
        route_corrected["predicted_speed_mm_s"],
        color="#7E57C2",
        lw=2.8,
        label="route-calibrated point-table prediction",
        zorder=5,
    )
    ax.set(
        xlabel="route distance from point-table start (mm)",
        ylabel="forward speed (mm/s)",
        title="Route-Calibrated Point-Table Speed Prediction vs Original Runs",
    )
    ax.grid(color="#D9DDE3", alpha=0.7, linewidth=0.7)
    ax.spines[["top", "right"]].set_visible(False)
    ax.legend(fontsize=8.5, ncol=2, frameon=False, loc="lower right")
    fig.tight_layout()
    fig.savefig(args.out_dir / "purple_prediction_vs_original_vx.png", dpi=180)
    plt.close(fig)

    fig, axes = plt.subplots(3, 2, figsize=(13, 11), sharex=False, sharey=True)
    axes = axes.ravel()
    for ax, (run_no, (file_name, frame)) in zip(axes, enumerate(samples.groupby("file", sort=False), start=1)):
        q = prefix_df[prefix_df["file"] == file_name]
        ax.plot(q["route_distance_mm"], q["v_actual_mm_s"], color="tab:blue", lw=0.8, label="original -vx_body")
        ax.plot(q["route_distance_mm"], q["v_predicted_mm_s"], color="tab:orange", lw=1.2, label="forecast after first 100")
        ax.axvline(q["route_distance_mm"].iloc[0], color="0.5", ls="--", lw=0.8)
        ax.set_title(f"run {run_no}: {file_name}")
        ax.set_xlabel("route distance (mm)")
        ax.set_ylabel("speed (mm/s)")
        ax.grid(alpha=0.2)
        ax.legend(fontsize=8)
    axes[-1].axis("off")
    fig.suptitle("First 100 steps calibrated, then point-table target-speed forecast", y=0.995)
    fig.tight_layout()
    fig.savefig(args.out_dir / "first100_prefix_forecast_vs_original_vx.png", dpi=160)
    plt.close(fig)

    # Human-readable report with aggregate cross-validation values.
    agg = metric_df.groupby(["model", "mode"])[["rmse_mm_s", "mae_mm_s", "r2"]].mean().reset_index()
    best = agg.sort_values(["mode", "rmse_mm_s"]).groupby("mode").first().reset_index()
    fo = np.asarray(all_fit["first_order"], dtype=float)
    fo_a = float(fo[1])
    fo_tau = float(-samples["dt_s"].median() / np.log(fo_a)) if 0.0 < fo_a < 1.0 else float("nan")
    fo_gain = float(fo[2] / (1.0 - fo_a))
    fo_offset = float(fo[0] / (1.0 - fo_a))
    curvature_corr = float(samples[["route_curvature", "wheel_curvature"]].corr().iloc[0, 1])
    correction_cv_base_rmse = float(correction_cv["base_rmse_mm_s"].mean())
    correction_cv_rmse = float(correction_cv["corrected_rmse_mm_s"].mean())
    prefix_mean_rmse = float(np.mean([row["forecast_rmse_mm_s"] for row in prefix_metrics]))
    prefix_mean_mae = float(np.mean([row["forecast_mae_mm_s"] for row in prefix_metrics]))
    lines = [
        "# Replay speed model analysis",
        "",
        f"- Auto rows: {len(samples):,} across {len(files)} logs; sampling period median {samples['dt_s'].median()*1000:.1f} ms.",
        f"- Actual forward speed definition: `v = -vx_body` (mm/s). Wheel cross-check: `4.79*(speed_L-speed_R)/2`.",
        f"- Route matching error: median {samples['route_match_error_mm'].median():.1f} mm, p95 {samples['route_match_error_mm'].quantile(.95):.1f} mm.",
        f"- All-data first-order model: `v[k] = {fo_a:.6f} v[k-1] + {fo[2]:.6f} u[k] {fo[0]:+.3f}`, where `u=-target_speed_set` (RPM); equivalent steady-state map `v_eq={fo_gain:.3f}u{fo_offset:+.1f}` and time constant `tau={fo_tau:.3f} s`.",
        f"- Point-table-only simulation starts from `v0=0` and reaches route point {int(route_only['route_index'].iloc[-1])}/{len(route)-1} in {route_only['time_s'].iloc[-1]:.2f} s; it uses no telemetry target-speed or measured-speed samples.",
        f"- Differential-drive estimate: equivalent track width `B={track_width_mm:.1f} mm`, using `omega=(v_R-v_L)/B`; measured wheel curvature is `kappa_actual=omega/v_wheel`.",
        f"- Planned/measured curvature correlation is {curvature_corr:.3f}; the yaw-state fit is `omega[k]={yaw_coef[0]:+.5f} {yaw_coef[1]:+.5f}*omega[k-1] {yaw_coef[2]:+.5f}*v[k-1]*kappa_plan[k]`.",
        "- The orange one-step model uses the measured previous speed in the ARX equation; the green recursive model uses its own previous prediction. The point-table-only base curve is the recursive version with `v0=0` and route-table target speed/curvature inputs.",
        "- The purple curve adds a route-specific correction learned from the median wheel-speed residual at each point and smoothed over 5 points. It improves repeatability on this calibrated route but is not a universal vehicle model for unseen routes.",
        f"- Leave-one-log-out route-correction RMSE changes from {correction_cv_base_rmse:.1f} to {correction_cv_rmse:.1f} mm/s; see `curve_correction_leave_one_file_out.csv`.",
        "- The first-100-step experiment fixes the global pole, estimates only offset/gain from the first 100 measured samples, then replaces future `target_speed_set` with the point-table target speed and forecasts recursively. The forecast starts from the 100th measured speed for evaluation.",
        f"- First-100 forecast mean across 5 runs: RMSE {prefix_mean_rmse:.1f} mm/s, MAE {prefix_mean_mae:.1f} mm/s. See `first100_prefix_forecast_vs_original_vx.png` and `first100_prefix_forecast_metrics.csv`.",
        "- `per_file_calibrated_comparison.png` fits the first-order-curvature coefficients separately on each file and evaluates on that same file; it is an in-sample calibration view, not a held-out validation result.",
        "",
        "## Leave-one-file-out mean metrics",
        "",
        "| model | mode | RMSE (mm/s) | MAE (mm/s) | R2 |",
        "|---|---|---:|---:|---:|",
    ]
    for _, row in agg.sort_values(["mode", "rmse_mm_s"]).iterrows():
        lines.append(f"| {row['model']} | {row['mode']} | {row['rmse_mm_s']:.1f} | {row['mae_mm_s']:.1f} | {row['r2']:.4f} |")
    lines += [
        "",
        "The static models map target RPM directly to speed. The dynamic models are discrete-time ARX models; recursive mode uses its own previous prediction after initializing from the first one or two measured samples. Coefficients are in `model_coefficients_all_data.json` and fold-specific values are in `model_coefficients.json`.",
        "",
        "Recommended deployment model: `first_order` for free-running reconstruction, with state reset at each `g_replay_state` segment, output clamped to the observed speed range, and an optional per-run gain calibration. `second_order_arx` is best for one-step correction when the measured previous speed is available, but is less suitable for long recursive simulation.",
    ]
    (args.out_dir / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"out_dir": str(args.out_dir), "rows": len(samples), "metrics": best.to_dict(orient="records")}, indent=2))


if __name__ == "__main__":
    main()
