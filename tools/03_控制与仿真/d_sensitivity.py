# -*- coding: utf-8 -*-
"""
=============================================================================
d_sensitivity.py —— 大 D 项的两个关键敏感性测试
=============================================================================
问题：D=5~10×P 是否可行？（用户提出）
已有结论：默认执行器惯性 tau=0.03s 下，大 D 因代数反馈 ψ_err'=−ω 压缩增益
          (1+D) 倍，收敛变慢、拉不回来。
本脚本补充验证：
  1. 执行器惯性敏感性：tau=0.03/0.10/0.20s（慢执行器下大 D 是否有价值）；
  2. 大 D 配大增益：K_E/P 同步放大，看能否抵消增益压缩；
  3. 大 D 下 peak_psi（航向摆动）与抖动——确认 D 的方向性收益。
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

BASE = [
    ("A_right0.3m_align",       0.30, -1.50,  90.0, 0.5),
    ("C_right0.3m_yaw+5deg",    0.30, -1.50,  95.0, 0.5),
    ("E_right0.3m_high2mps",    0.30, -1.50,  90.0, 2.0),
    ("G_right0.5m_align",       0.50, -1.50,  90.0, 0.5),
    ("H_yaw+10deg",             0.00, -1.50, 100.0, 0.5),
]
SEEDS = (1, 2, 3, 4, 5)


def run_set(ctrl_factory, scenarios, seeds, tau_override=0.03):
    outs = []
    for sd in seeds:
        res = {}
        for nm, x0, y0, p0, v in scenarios:
            ctrl = ctrl_factory()
            res[nm] = sim.run_sim(ctrl, x0, y0, p0, v, seed=sd,
                                  tau_act=tau_override)
        outs.append(res)
    return outs


def summarize(outs, scenarios):
    names = [s[0] for s in scenarios]
    convs, dones, peaks_e, peaks_psi = [], [], [], []
    for nm in names:
        for o in outs:
            m = sim.metrics(o[nm])
            convs.append(m["conv_s"])
            dones.append(m["done"])
            peaks_e.append(m["peak_e_mm"])
            peaks_psi.append(m["peak_psi_deg"])
    return (np.nanmean(convs), all(dones), np.nanmax(peaks_e),
            np.nanmean(peaks_psi))


def omega_jitter_avg(outs, scenarios):
    names = [s[0] for s in scenarios]
    jits = []
    for nm in names:
        for o in outs:
            r = o[nm]
            if len(r["omega"]) >= 3:
                jits.append(np.std(np.diff(r["omega"])) / sim.DT)
    return np.mean(jits)


print("=" * 78)
print("测试1：执行器惯性 tau 敏感性 —— 慢执行器下大 D 是否有价值")
print("=" * 78)
print(f"{'配置':44s} {'conv':>6s} {'全完成':>6s} {'peak_e':>7s} {'peak_psi':>8s} {'ω抖动':>8s}")
for tau in (0.03, 0.10, 0.20):
    for D_mult in (0.0, 5.0, 10.0):
        D = D_mult * 7.0
        outs = run_set(
            lambda D=D: sim.PDController(P_PSI=7.0, D_PSI=D, K_E=11.0),
            BASE, SEEDS, tau_override=tau)
        conv, done, pe, pp = summarize(outs, BASE)
        jit = omega_jitter_avg(outs, BASE)
        print(f"tau={tau:.2f}s  D={D_mult:5.1f}P={D:5.1f}   "
              f"{conv:6.2f} {str(done):>6s} {pe:7.0f} {pp:8.1f} {jit:8.1f}")

print()
print("=" * 78)
print("测试2：大 D 配大增益（K_E、P 同步放大）能否抵消增益压缩")
print("=" * 78)
print(f"{'配置':44s} {'conv':>6s} {'全完成':>6s} {'peak_e':>7s} {'peak_psi':>8s} {'ω抖动':>8s}")
configs = [
    ("D=0 基准 (K_E=11,P=7)",          7.0, 11.0, 0.0),
    ("D=5P (K_E=11,P=7)",              7.0, 11.0, 35.0),
    ("D=5P 增益x2 (K_E=22,P=14)",     14.0, 22.0, 70.0),
    ("D=5P 增益x3 (K_E=33,P=21)",     21.0, 33.0, 105.0),
    ("D=10P (K_E=11,P=7)",             7.0, 11.0, 70.0),
    ("D=10P 增益x3 (K_E=33,P=21)",    21.0, 33.0, 210.0),
    ("D=2P (K_E=11,P=7)",              7.0, 11.0, 14.0),
    ("D=2P 增益x1.5 (K_E=16.5,P=10.5)", 10.5, 16.5, 21.0),
]
for label, P, KE, D in configs:
    outs = run_set(lambda P=P, KE=KE, D=D: sim.PDController(P_PSI=P, D_PSI=D, K_E=KE),
                   BASE, SEEDS)
    conv, done, pe, pp = summarize(outs, BASE)
    jit = omega_jitter_avg(outs, BASE)
    print(f"{label:44s} {conv:6.2f} {str(done):>6s} {pe:7.0f} {pp:8.1f} {jit:8.1f}")

print()
print("=" * 78)
print("测试3：D 的方向性收益 —— 固定 D=1.75(0.25P)，看 peak_psi 摆动抑制")
print("=" * 78)
for D in (0.0, 0.75, 1.75, 3.5):
    outs = run_set(lambda D=D: sim.PDController(P_PSI=7.0, D_PSI=D, K_E=11.0),
                   BASE, SEEDS)
    conv, done, pe, pp = summarize(outs, BASE)
    jit = omega_jitter_avg(outs, BASE)
    print(f"D={D:5.2f} ({D/7.0*100:4.0f}%P)   conv={conv:6.2f}s done={str(done):>5s} "
          f"peak_e={pe:5.0f}mm peak_psi={pp:6.1f}° ω抖动={jit:6.1f}")
