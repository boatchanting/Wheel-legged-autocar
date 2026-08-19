# -*- coding: utf-8 -*-
"""
仅平行偏差的进入段 PD 参数精修寻优
场景：车平行于道路（psi0=90°），初始横向偏差 x0，目标在前方 1.5m 路面中心 (0,0)。
验收：上桥瞬间（D<0.4m）|x| <= 11.5cm（轮子对得上 50cm 桥面）且 |psi_err| <= 20°。
目标：最大化各速度下能通过验收的最大 x0（= 平行偏差可修能力）。
"""
import numpy as np
from lqr_vs_pd_enter_sim import (PDController, LQRController, PVC_CONFIRM_D,
                                 VehicleSim, DT, norm_deg)

X_TOL, PSI_TOL = 0.115, 20.0
X0_GRID = [0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.00, 1.10, 1.20]
SPEEDS = [1.0, 1.5, 2.0]


def run_entry(x0, v, ctrl, max_t=5.0):
    ctrl.reset(90.0)
    veh = VehicleSim(x0, -1.5, 90.0, v, tx=0.0, ty=0.0, seed=1)
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
            return veh.x, norm_deg(90.0 - np.degrees(veh.psi))
    return None


def pass_ok(x0, v, ctrl_factory):
    r = run_entry(x0, v, ctrl_factory())
    if r is None:
        return False, None
    x, psi = r
    return (abs(x) <= X_TOL and abs(psi) <= PSI_TOL), (x, psi)


def max_x0_per_speed(ctrl_factory, x0s=X0_GRID, speeds=SPEEDS):
    """每个速度下能通过的最大 x0（网格扫描，取最高通过值）。"""
    out = {}
    for v in speeds:
        best = 0.0
        for x0 in x0s:
            ok, _ = pass_ok(x0, v, ctrl_factory)
            if ok:
                best = x0
        out[v] = best
    return out


def eval_params(K_E, P, D, verbose=False):
    ctrl = lambda: PDController(P_PSI=P, D_PSI=D, K_E=K_E, psi_d_tau=0.05)
    cap = max_x0_per_speed(ctrl)
    total = sum(cap.values())
    if verbose:
        print(f"  K_E={K_E:5.1f} P={P:5.1f} D={D:4.2f} => 最大可修 x0: "
              f"@1.0={cap[1.0]*100:4.0f}cm @1.5={cap[1.5]*100:4.0f}cm "
              f"@2.0={cap[2.0]*100:4.0f}cm  总分={total*100:.0f}")
    return {"cap": cap, "total": total}


def grid_search():
    # 粗网格
    coarse = []
    for K_E in (5, 8, 11, 15, 20, 25, 30, 40):
        for P in (4, 6, 8, 10, 12, 15):
            for D in (0.0, 0.5, 1.0):
                coarse.append((K_E, P, D))
    results = []
    for i, (K_E, P, D) in enumerate(coarse):
        r = eval_params(K_E, P, D)
        results.append((r["total"], K_E, P, D, r["cap"]))
    results.sort(reverse=True, key=lambda a: a[0])
    print("\n===== 粗网格 Top 8（仅平行偏差）=====")
    for total, K_E, P, D, cap in results[:8]:
        print(f"  K_E={K_E:5.1f} P={P:5.1f} D={D:4.2f} => "
              f"@1.0={cap[1.0]*100:3.0f} @1.5={cap[1.5]*100:3.0f} "
              f"@2.0={cap[2.0]*100:3.0f}cm 总分={total*100:.0f}cm")
    # 精网格：围绕粗网格 top3 的 (K_E,P) 区域
    print("\n===== 精网格 =====")
    fine = []
    for (total, K_E, P, D, cap) in results[:3]:
        for KE in np.arange(max(2, K_E - 4), K_E + 5, 1.0):
            for PP in np.arange(max(2, P - 3), P + 3.5, 0.5):
                for DD in (0.0, 0.15, 0.25, 0.5, 0.75):
                    fine.append((round(float(KE), 2), round(float(PP), 2), float(DD)))
    fres = []
    for K_E, P, D in fine:
        r = eval_params(K_E, P, D)
        fres.append((r["total"], K_E, P, D, r["cap"]))
    fres.sort(reverse=True, key=lambda a: a[0])
    for total, K_E, P, D, cap in fres[:8]:
        print(f"  K_E={K_E:5.2f} P={P:5.2f} D={D:4.2f} => "
              f"@1.0={cap[1.0]*100:3.0f} @1.5={cap[1.5]*100:3.0f} "
              f"@2.0={cap[2.0]*100:3.0f}cm 总分={total*100:.0f}cm")
    return fres[0]


if __name__ == "__main__":
    print(f"仅平行偏差寻优；验收: 上桥瞬间 |x|≤{X_TOL*100:.1f}cm 且 |ψ|≤{PSI_TOL:.0f}°")
    print(f"x0 网格 {int(min(X0_GRID)*100)}~{int(max(X0_GRID)*100)}cm，速度 {SPEEDS}")
    best = grid_search()
    print("\n===== 最优参数（平行偏差专用）详细能力 =====")
    cap = eval_params(best[1], best[2], best[3])
    print(f"  最优: K_E={best[1]} P={best[2]} D={best[3]}")
    for v in SPEEDS:
        ctrl = lambda: PDController(P_PSI=best[2], D_PSI=best[3], K_E=best[1], psi_d_tau=0.05)
        # 精确到边界：在最大通过点附近细化
        lo = cap["cap"][v]
        for x0 in np.arange(lo, lo + 0.15, 0.01):
            ok, r = pass_ok(round(float(x0), 3), v, ctrl())
            if ok:
                print(f"  v={v}: 通过 x0={x0*100:.1f}cm (x={r[0]*100:.1f}cm, ψ={r[1]:.1f}°)")
            else:
                print(f"  v={v}: 边界 x0={x0*100:.1f}cm 失败 (x={r[0]*100:.1f}cm, ψ={r[1]:.1f}°)")
                break
    print("\n===== 对照组 =====")
    for name, fac in [("现役50/50/3.5", lambda: PDController(P_PSI=50.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05)),
                      ("LQR120/32  ", lambda: LQRController())]:
        c = max_x0_per_speed(fac)
        print(f"  {name}: @1.0={c[1.0]*100:.0f}cm @1.5={c[1.5]*100:.0f}cm @2.0={c[2.0]*100:.0f}cm")
