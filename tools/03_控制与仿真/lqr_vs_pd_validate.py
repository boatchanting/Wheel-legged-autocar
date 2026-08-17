# -*- coding: utf-8 -*-
"""
=============================================================================
lqr_vs_pd_validate.py —— 对 lqr_vs_pd_enter_sim.py 最优 PD 参数的深入验证
=============================================================================
1. 最优参数 (K_E=13.0, P=8.0, D=0.25) 附近的精细扫描（敏感度 + 最优性确认）；
2. 鲁棒性：多噪声种子 / 更大视觉延迟 / 更大初始偏差 / 中间速度 v=1.0；
3. D 项作用：D=0 与 D=0.25 在带噪声/延迟下的差异；
4. 输出推荐参数与逐工况对照表。
=============================================================================
"""
import numpy as np
import importlib.util
import os

HERE = os.path.dirname(os.path.abspath(__file__))
MAIN = os.path.join(HERE, "lqr_vs_pd_enter_sim.py")

spec = importlib.util.spec_from_file_location("sim", MAIN)
sim = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sim)


def run_multi(controller_factory, scenarios, seeds=(1,)):
    """多种子运行同一组场景，返回 (res_list) 每种子一组 dict。"""
    outs = []
    for sd in seeds:
        res = {}
        for name, x0, y0, p0, v in scenarios:
            ctrl = controller_factory()
            res[name] = sim.run_sim(ctrl, x0, y0, p0, v, seed=sd)
        outs.append(res)
    return outs


def stats_over_seeds(outs, key="e"):
    """多种子下各工况的均值/最差指标（收敛时间、峰值）。"""
    names = list(outs[0].keys())
    table = {}
    for nm in names:
        convs, peaks_e, peaks_psi, dones = [], [], [], []
        for o in outs:
            m = sim.metrics(o[nm])
            convs.append(m["conv_s"])
            peaks_e.append(m["peak_e_mm"])
            peaks_psi.append(m["peak_psi_deg"])
            dones.append(m["done"])
        table[nm] = dict(
            conv_mean=float(np.nanmean(convs)), conv_worst=float(np.nanmax(convs)),
            peak_e_worst=float(np.nanmax(peaks_e)),
            peak_psi_mean=float(np.nanmean(peaks_psi)),
            all_done=all(dones))
    return table


def print_table(title, table):
    print(f"\n== {title} ==")
    print(f"{'工况':26s} {'conv均值':>8s} {'conv最差':>8s} {'peak_e最差':>9s} "
          f"{'peak_psi均值':>11s} {'全完成':>6s}")
    for nm, s in table.items():
        print(f"{nm:26s} {s['conv_mean']:8.2f} {s['conv_worst']:8.2f} "
              f"{s['peak_e_worst']:9.0f} {s['peak_psi_mean']:11.1f} {str(s['all_done']):>6s}")


# ---------------------------------------------------------------- 1. 精细扫描
def fine_scan():
    lqr_res = sim.run_all(lambda: sim.LQRController())
    print("== 最优参数附近精细扫描（K_E×P×D，diff_cost 越小越接近 LQR）==")
    best, bc = None, 1e18
    grid = []
    for K_E in np.arange(11.0, 15.0, 0.5):
        for P in np.arange(6.5, 9.5, 0.25):
            for D in np.arange(0.0, 0.75, 0.125):
                grid.append((round(float(K_E), 3), round(float(P), 3),
                             round(float(D), 3)))
    for (K_E, P, D) in grid:
        res = sim.run_all(lambda: sim.PDController(P_PSI=P, D_PSI=D, K_E=K_E))
        c = sum(sim.diff_cost(res[k], lqr_res[k]) for k in res)
        if c < bc:
            bc, best = c, (K_E, P, D, c)
    print(f"精细扫描 {len(grid)} 组 → 最优 K_E={best[0]} P={best[1]} D={best[2]} "
          f"diff_cost={best[3]:.4f}")

    # 敏感度：固定 K_E/P 微调 D；固定 P/D 微调 K_E；固定 K_E/D 微调 P
    print("\n-- 敏感度（相邻参数 diff_cost）--")
    K, P, D = best[0], best[1], best[2]
    for dK in (-1.0, -0.5, 0.5, 1.0):
        res = sim.run_all(lambda: sim.PDController(P_PSI=P, D_PSI=D, K_E=K + dK))
        c = sum(sim.diff_cost(res[k], lqr_res[k]) for k in res)
        print(f"  K_E={K + dK:5.1f} P={P} D={D} → {c:.4f}")
    for dP in (-0.5, 0.5, 1.0):
        res = sim.run_all(lambda: sim.PDController(P_PSI=P + dP, D_PSI=D, K_E=K))
        c = sum(sim.diff_cost(res[k], lqr_res[k]) for k in res)
        print(f"  K_E={K} P={P + dP:4.1f} D={D} → {c:.4f}")
    for dD in (-0.25, 0.125, 0.25):
        res = sim.run_all(lambda: sim.PDController(P_PSI=P, D_PSI=D + dD, K_E=K))
        c = sum(sim.diff_cost(res[k], lqr_res[k]) for k in res)
        print(f"  K_E={K} P={P} D={D + dD:4.2f} → {c:.4f}")
    return best


# ---------------------------------------------------------------- 2. 鲁棒性
ROBUST_SCENARIOS = [
    ("A_right0.3m_align",       0.30, -1.50,  90.0, 0.5),
    ("C_right0.3m_yaw+5deg",    0.30, -1.50,  95.0, 0.5),
    ("E_right0.3m_high2mps",    0.30, -1.50,  90.0, 2.0),
    ("G_right0.5m_align",       0.50, -1.50,  90.0, 0.5),   # 更大横向偏差
    ("H_yaw+10deg",             0.00, -1.50, 100.0, 0.5),   # 更大航向偏差
    ("I_mid1mps",               0.30, -1.50,  90.0, 1.0),   # 中间速度
]


def robustness(K_E, P, D):
    seeds = (1, 2, 3, 4, 5)
    print("\n== 鲁棒性验证（5 个噪声种子，含更极端初始条件 / 中间速度）==")
    lqr_outs = run_multi(lambda: sim.LQRController(), ROBUST_SCENARIOS, seeds)
    pd_outs = run_multi(lambda: sim.PDController(P_PSI=P, D_PSI=D, K_E=K_E),
                        ROBUST_SCENARIOS, seeds)
    print_table("LQR 基准", stats_over_seeds(lqr_outs))
    print_table(f"PD (K_E={K_E}, P={P}, D={D})", stats_over_seeds(pd_outs))

    # 逐工况轨迹接近度（多种子平均）
    print("\n-- PD vs LQR 逐工况轨迹 diff（越小越接近）--")
    for nm in [s[0] for s in ROBUST_SCENARIOS]:
        ds = [sim.diff_cost(po[nm], lo[nm]) for po, lo in zip(pd_outs, lqr_outs)]
        print(f"  {nm:26s} diff均值={np.mean(ds):.4f}  最差={np.max(ds):.4f}")


# ---------------------------------------------------------------- 3. D 项量程研究
D_SCALE = [0.0, 0.25, 1.0, 3.0, 5.0, 7.0, 10.0]  # 以 P 的倍数


def omega_jitter(res):
    """转向指令抖动指标：ω 相邻采样差分标准差（rad/s² 量级），越大越抖。"""
    if len(res["omega"]) < 3:
        return 0.0
    return float(np.std(np.diff(res["omega"])) / sim.DT)


def d_range_study(K_E, P):
    """D 全量程研究：直接微分 vs 滤波微分，测收敛/抖动/与 LQR 接近度。"""
    seeds = (1, 2, 3, 4, 5)
    base = ROBUST_SCENARIOS
    lqr_outs = run_multi(lambda: sim.LQRController(), base, seeds)
    lqr_jit = {nm: np.mean([omega_jitter(o[nm]) for o in lqr_outs])
               for nm in [s[0] for s in base]}

    print("\n== D 项全量程研究（K_E=%.0f, P=%.0f；5 种子均值；量纲：D/P 倍数）==" % (K_E, P))
    print(f"{'配置':38s} {'conv均值':>8s} {'全完成':>6s} {'peak_e最差':>9s} "
          f"{'ω抖动均值':>9s} {'vsLQR diff':>10s}")

    def row(label, ctrl_factory):
        outs = run_multi(ctrl_factory, base, seeds)
        t = stats_over_seeds(outs)
        convs = [v["conv_mean"] for v in t.values()]
        dones = all(v["all_done"] for v in t.values())
        worst_e = max(v["peak_e_worst"] for v in t.values())
        jit = np.mean([np.mean([omega_jitter(o[nm]) for o in outs])
                       for nm in [s[0] for s in base]])
        diffs = [np.mean([sim.diff_cost(po[nm], lo[nm])
                          for po, lo in zip(outs, lqr_outs)])
                 for nm in [s[0] for s in base]]
        print(f"{label:38s} {np.nanmean(convs):8.2f} {str(dones):>6s} "
              f"{worst_e:9.0f} {jit:9.2f} {np.mean(diffs):10.4f}")

    row(f"LQR 基准", lambda: sim.LQRController())
    for mult in D_SCALE:
        D = mult * P
        # 直接微分（无滤波）
        row(f"PD 直接微分 D={mult:.2f}P={D:5.2f}",
            lambda D=D: sim.PDController(P_PSI=P, D_PSI=D, K_E=K_E))
    for mult in (3.0, 5.0, 7.0, 10.0):
        D = mult * P
        # 滤波微分 tau=50ms
        row(f"PD 滤波微分(τ=50ms) D={mult:.2f}P={D:5.2f}",
            lambda D=D: sim.PDController(P_PSI=P, D_PSI=D, K_E=K_E,
                                         psi_d_tau=0.05))
    # 理想无噪声对照：大 D 的等效增益压缩（证明理论）
    print("\n-- 理想无噪声对照（看大 D 的等效增益压缩）--")
    for mult in (1.0, 5.0, 10.0):
        D = mult * P
        outs = run_multi(lambda D=D: sim.PDController(P_PSI=P, D_PSI=D, K_E=K_E),
                         base, seeds)
        t = stats_over_seeds(outs)
        convs = [v["conv_mean"] for v in t.values()]
        print(f"  D={mult:.0f}P={D:5.2f} 无噪声: conv均值={np.nanmean(convs):.2f}s")


if __name__ == "__main__":
    best = fine_scan()
    K_E, P, D = best[0], best[1], best[2]
    robustness(K_E, P, D)
    d_range_study(K_E, P)
