# -*- coding: utf-8 -*-
"""
进入段 PD 参数精修寻优
验收（用户定义）：
  上桥瞬间（D<0.4m）|x| <= 11.5cm（轮子对得上 50cm 桥面）且 |psi_err| <= 20°
  （30cm 内可拉回）。不要求过程中在路面内 / e 完全收敛。
工况：
  A) 平行横向偏差：车在 (x0,-1.5) 航向 90°，目标 (0,0)。e0=x0。
  B) 方位角偏差：车在 (0,-D0*cosβ) 航向 90°-β，目标 (0,0)（车在路面中心、车头偏 β）。
寻优目标：最大化所有工况的“通过裕度”之和（裕度 = min(横向裕度, 航向裕度)）。
"""
import numpy as np
from lqr_vs_pd_enter_sim import (PDController, LQRController, DEG2RAD,
                                 PVC_CONFIRM_D, VehicleSim, DT, norm_deg)

X_TOL, PSI_TOL = 0.115, 20.0
D0 = 1.5


def run_entry(x0, y0, psi0, v, ctrl, max_t=4.0):
    ctrl.reset(psi0)
    veh = VehicleSim(x0, y0, psi0, v, tx=0.0, ty=0.0, seed=1)
    last_fx, last_fy = 32767, 32767
    n = int(max_t / DT)
    for i in range(n):
        fx, fy = veh.measure()
        nv = fx is not None
        if nv:
            last_fx, last_fy = fx, fy
        om = ctrl.update(last_fx, last_fy, np.degrees(veh.yaw_meas()), v, DT, nv)
        veh.step(om)
        if np.hypot(veh.x, veh.y) < PVC_CONFIRM_D:
            return veh.x, norm_deg(psi0 - np.degrees(veh.psi))
    return None  # 未到上桥点


def margin(x, psi):
    """通过裕度：两者都通过返回正裕度，否则为 0（未通过）。"""
    if abs(x) > X_TOL or abs(psi) > PSI_TOL:
        return 0.0
    return min((X_TOL - abs(x)) / X_TOL, (PSI_TOL - abs(psi)) / PSI_TOL)


# 工况集：并行偏差 + 方位角，多速度
PARALLEL = [(0.15, 1.0), (0.25, 1.0), (0.40, 1.0), (0.55, 1.0), (0.70, 1.0),
            (0.15, 1.5), (0.25, 1.5), (0.40, 1.5),
            (0.15, 2.0), (0.25, 2.0), (0.40, 2.0)]
BEARING = [(15.0, 1.0), (25.0, 1.0), (35.0, 1.0),
           (15.0, 1.5), (25.0, 1.5),
           (15.0, 2.0), (25.0, 2.0)]


def eval_generic(get_ctrl, verbose=False):
    """对同一套工况评估任意控制器工厂，返回 (total, npass, details)。"""
    total = 0.0
    npass = 0
    details = []
    for x0, v in PARALLEL:
        r = run_entry(x0, -1.5, 90.0, v, get_ctrl())
        if r is None:
            details.append((f"P{x0*100:.0f}@{v}", None, None, 0.0))
            continue
        x, psi = r
        m = margin(x, psi)
        total += m
        if m > 0:
            npass += 1
        details.append((f"P{x0*100:.0f}@{v}", x, psi, m))
    for beta, v in BEARING:
        y0 = -D0 * np.cos(beta * DEG2RAD)
        psi0 = 90.0 - beta
        r = run_entry(0.0, y0, psi0, v, get_ctrl())
        if r is None:
            details.append((f"B{beta:.0f}@{v}", None, None, 0.0))
            continue
        x, psi = r
        m = margin(x, psi)
        total += m
        if m > 0:
            npass += 1
        details.append((f"B{beta:.0f}@{v}", x, psi, m))
    return total, npass, details


def eval_params(K_E, P, D, verbose=False):
    ctrl = lambda: PDController(P_PSI=P, D_PSI=D, K_E=K_E, psi_d_tau=0.05)
    total, npass, details = eval_generic(ctrl)
    if verbose:
        print(f"  K_E={K_E:5.1f} P={P:4.1f} D={D:4.1f} => 通过 {npass}/{len(PARALLEL)+len(BEARING)} 裕度总分 {total:.2f}")
        for name, x, psi, m in details:
            xs = f"{x*100:5.1f}cm" if x is not None else "  —  "
            ps = f"{psi:6.1f}°" if psi is not None else "  —  "
            print(f"     {name:9s} x={xs} ψ={ps} m={m:.2f}")
    return {"total": total, "npass": npass, "ntotal": len(PARALLEL)+len(BEARING)}
    if verbose:
        print(f"  K_E={K_E:5.1f} P={P:4.1f} D={D:4.1f} => 通过 {npass}/{ntotal} 裕度总分 {total:.2f}")
        for name, x, psi, m in details:
            xs = f"{x*100:5.1f}cm" if x is not None else "  —  "
            ps = f"{psi:6.1f}°" if psi is not None else "  —  "
            print(f"     {name:9s} x={xs} ψ={ps} m={m:.2f}")
    return {"total": total, "npass": npass, "ntotal": ntotal}


def grid_search():
    # 第一轮粗网格
    coarse = []
    for K_E in (8, 11, 15, 20, 30, 50):
        for P in (5, 7, 9, 12):
            for D in (0.0, 0.5, 1.0, 2.0, 3.5):
                coarse.append((K_E, P, D))
    results = []
    for i, (K_E, P, D) in enumerate(coarse):
        r = eval_params(K_E, P, D)
        results.append((r["total"], r["npass"], K_E, P, D))
    results.sort(reverse=True)
    print("\n===== 粗网格 Top 10 =====")
    for total, npass, K_E, P, D in results[:10]:
        print(f"  K_E={K_E:5.1f} P={P:4.1f} D={D:4.1f} => 通过 {npass}/{len(PARALLEL)+len(BEARING)} 总分 {total:.2f}")
    # 第二轮精网格：围绕粗网格最优附近
    best = results[0]
    print("\n===== 精网格（围绕最优区域）=====")
    fine = []
    k0, p0, d0 = best[2], best[3], best[4]
    for K_E in np.arange(max(6, k0 - 4), k0 + 5, 1.0):
        for P in np.arange(max(3, p0 - 2.5), p0 + 3, 0.5):
            for D in np.arange(max(0, d0 - 0.75), d0 + 0.8, 0.25):
                fine.append((round(float(K_E), 2), round(float(P), 2), round(float(D), 2)))
    fres = []
    for i, (K_E, P, D) in enumerate(fine):
        r = eval_params(K_E, P, D)
        fres.append((r["total"], r["npass"], K_E, P, D))
    fres.sort(reverse=True)
    for total, npass, K_E, P, D in fres[:10]:
        print(f"  K_E={K_E:5.2f} P={P:4.2f} D={D:4.2f} => 通过 {npass}/{len(PARALLEL)+len(BEARING)} 总分 {total:.2f}")
    return fres[0]


if __name__ == "__main__":
    print(f"验收: 上桥瞬间 |x|≤{X_TOL*100:.1f}cm 且 |psi|≤{PSI_TOL:.0f}°")
    print(f"工况: 平行偏差 {len(PARALLEL)} 个 + 方位角 {len(BEARING)} 个，共 {len(PARALLEL)+len(BEARING)} 个")
    best = grid_search()
    print("\n===== 最优参数详细表现 =====")
    eval_params(best[2], best[3], best[4], verbose=True)
    print("\n===== 对照组：现役 PD50 详细表现 =====")
    eval_params(50.0, 7.0, 3.5, verbose=True)
    print("\n===== 对照组：LQR(120/32) 详细表现 =====")
    total, npass, details = eval_generic(lambda: LQRController())
    print(f"  LQR => 通过 {npass}/{len(PARALLEL)+len(BEARING)} 裕度总分 {total:.2f}")
    for name, x, psi, m in details:
        xs = f"{x*100:5.1f}cm" if x is not None else "  —  "
        ps = f"{psi:6.1f}°" if psi is not None else "  —  "
        print(f"     {name:9s} x={xs} ψ={ps} m={m:.2f}")
