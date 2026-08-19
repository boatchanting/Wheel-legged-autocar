# -*- coding: utf-8 -*-
"""
放宽验收标准后的横向偏差收敛能力扫描
验收（用户定义）：
  1) 上桥瞬间（D<0.4m）轮子对得上桥面：车中心距目标线 |x| <= 11.5cm
     （桥面 50cm - 车宽 27cm，两侧余量 (50-27)/2=11.5cm）；
  2) 上桥瞬间航向偏差 |psi_err| <= 20°（可在上桥后 30cm 前进内拉回，
     30cm*sin(20°)≈10.3cm < 11.5cm 边缘余量，恰好够）。
  不要求过程中在路面内、不要求 e 完全收敛。
场景：车平行于道路，初始横向偏差 x0，目标在前方 1.5m 路面中心。
"""
import numpy as np
from lqr_vs_pd_enter_sim import (PDController, LQRController, DEG2RAD,
                                 PVC_CONFIRM_D, VehicleSim, DT, norm_deg)

X_TOL   = 0.115    # 上桥瞬间轮子对得上：车中心 |x| <= 11.5cm
PSI_TOL = 20.0     # 上桥瞬间航向偏差 <= 20°（30cm 内可拉回）


def run_lateral(x0, v, ctrl_factory, max_t=4.0):
    p0 = 90.0
    ctrl = ctrl_factory()
    ctrl.reset(p0)
    veh = VehicleSim(x0, -1.5, p0, v, tx=0.0, ty=0.0, seed=1)
    last_fx, last_fy = 32767, 32767
    n = int(max_t / DT)
    D = 1e9
    x_entry = psi_entry = None
    for i in range(n):
        fx, fy = veh.measure()
        nv = fx is not None
        if nv:
            last_fx, last_fy = fx, fy
        om = ctrl.update(last_fx, last_fy, np.degrees(veh.yaw_meas()), v, DT, nv)
        veh.step(om)
        dx, dy = -veh.x, -veh.y
        D = np.hypot(dx, dy)
        if D < PVC_CONFIRM_D:
            x_entry = veh.x
            psi_entry = norm_deg(p0 - np.degrees(veh.psi))
            break
    if x_entry is None:
        return None
    ok = (abs(x_entry) <= X_TOL) and (abs(psi_entry) <= PSI_TOL)
    return {"x": x_entry * 100, "psi": psi_entry, "ok": ok}


def main():
    print(f"验收: 上桥瞬间 |x|≤{X_TOL*100:.1f}cm 且 |psi_err|≤{PSI_TOL:.0f}°")
    print("      （不要求过程在路面内 / e 完全收敛 / 修到路面中间）")
    ctrls = [
        ("PD50", lambda: PDController(P_PSI=7.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05)),
        ("PD11", lambda: PDController(P_PSI=7.0, D_PSI=1.0, K_E=11.0, psi_d_tau=0.05)),
        ("LQR ", lambda: LQRController()),
    ]
    x0s = [0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.80, 1.00]
    for v in (1.0, 1.5, 2.0):
        print(f"\n===== v={v} m/s =====")
        for x0 in x0s:
            row = [f"x0={x0*100:3.0f}cm"]
            for name, fac in ctrls:
                r = run_lateral(x0, v, fac)
                if r is None:
                    row.append(f"{name}: 未达上桥点")
                else:
                    mark = "✓" if r["ok"] else "✗"
                    row.append(f"{name}: x={r['x']:5.1f}cm ψ={r['psi']:5.1f}°[{mark}]")
            print("   " + "  ".join(row))


if __name__ == "__main__":
    main()
