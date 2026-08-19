# -*- coding: utf-8 -*-
"""
横向距离偏差收敛能力扫描（车与道路平行，只有横程偏差 x0）
场景：路面中心线 x=0；车在 (x0, -1.5) 起步，航向 psi0=90°（与道路平行），
      目标（桥入口）在路面中心 (0,0) 前方 D0=1.5m；entry_yaw=90°（检测瞬间航向）。
      => 初始横向偏差 e0 = x0（精确等于横程误差）。
约束：路半宽 25cm，车半宽 13.5cm => 车体中心线 |x| ≤ 11.5cm。
问：最大能收敛多少横向偏差 x0？（e<50mm 收敛，且全程车体不出路面）
"""
import numpy as np
from lqr_vs_pd_enter_sim import (PDController, LQRController, DEG2RAD,
                                 PVC_CONFIRM_D, VehicleSim, DT)

ROAD_HALF_W = 0.25
CAR_HALF_W  = 0.135
MAX_X       = ROAD_HALF_W - CAR_HALF_W   # 11.5cm


def run_lateral(x0, v, ctrl_factory, max_t=4.0):
    p0 = 90.0
    ctrl = ctrl_factory()
    ctrl.reset(p0)
    veh = VehicleSim(x0, -1.5, p0, v, tx=0.0, ty=0.0, seed=1)
    last_fx, last_fy = 32767, 32767
    max_abs_x, conv_t = 0.0, None
    n = int(max_t / DT)
    D = 1e9
    for i in range(n):
        fx, fy = veh.measure()
        nv = fx is not None
        if nv:
            last_fx, last_fy = fx, fy
        om = ctrl.update(last_fx, last_fy, np.degrees(veh.yaw_meas()), v, DT, nv)
        veh.step(om)
        max_abs_x = max(max_abs_x, abs(veh.x))
        dx, dy = -veh.x, -veh.y
        fxx = -dx * np.sin(veh.psi) + dy * np.cos(veh.psi)
        fyy = dx * np.cos(veh.psi) + dy * np.sin(veh.psi)
        D = np.hypot(fxx, fyy)
        if D < PVC_CONFIRM_D:
            break
        if conv_t is None and abs(D * np.sin(np.arctan2(fxx, fyy))) < 0.05:
            conv_t = i * DT
    return {"max_x": max_abs_x, "conv": conv_t, "done": D < PVC_CONFIRM_D}


def main():
    print(f"约束: 路半宽 25cm - 车半宽 13.5cm => 车体中心线 |x|≤{MAX_X*100:.1f}cm")
    print("场景: 车平行于道路, 横向偏差 x0, 目标在前方 1.5m 路面中心")
    print("      e0=x0; 收敛判据 |e|<50mm 且 D<0.4m 前完成")
    ctrls = [
        ("PD50", lambda: PDController(P_PSI=7.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05)),
        ("PD11", lambda: PDController(P_PSI=7.0, D_PSI=1.0, K_E=11.0, psi_d_tau=0.05)),
        ("LQR ", lambda: LQRController()),
    ]
    x0s = [0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.40, 0.50]
    for v in (1.0, 1.5, 2.0):
        print(f"\n===== v={v} m/s =====")
        for x0 in x0s:
            row = [f"x0={x0*100:4.0f}cm"]
            for name, fac in ctrls:
                r = run_lateral(x0, v, fac)
                ok = "✓" if r["max_x"] <= MAX_X + 1e-6 else "✗出路面"
                conv = f"{r['conv']:.2f}s" if r["conv"] is not None else "  —  "
                row.append(f"{name}: max|x|={r['max_x']*100:4.1f}cm[{ok}] conv={conv}")
            print("   " + "  ".join(row))


if __name__ == "__main__":
    main()
