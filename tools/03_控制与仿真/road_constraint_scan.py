# -*- coding: utf-8 -*-
"""
带道路边界约束的进入段收敛能力扫描
约束：路宽 50cm（半宽 25cm），车宽 27cm（半宽 13.5cm）
      => 车体中心线必须满足 |x| <= 11.5cm，否则车身出路面。
场景：车在路面中心线 (x=0) 起步，目标（桥入口）在路面中心前方 D0=1.5m，
      车初始航向相对路面轴线偏 β0（=> 检测瞬间目标方位角 = β0）。
      entry_yaw 锁存为检测瞬间航向（与固件一致）。
问：最佳 PID 参数下，最多能收敛多大 β0（对应投影偏差 e0=D0·sinβ0）而不出路面？
"""
import numpy as np
from lqr_vs_pd_enter_sim import (run_sim, PDController, LQRController, metrics,
                                 DEG2RAD, PVC_CONFIRM_D)

# --- 道路/车体约束 ---
ROAD_HALF_W = 0.25     # 路半宽 25cm
CAR_HALF_W  = 0.135    # 车半宽 13.5cm
MAX_X       = ROAD_HALF_W - CAR_HALF_W   # 车体中心线允许的最大 |x| = 11.5cm

def make_bearing_scenario(beta_deg, D0, v_mps, side=+1.0):
    """车在路面中心线 (x=0) 起步；目标在路面中心前方。
       side=+1: 目标在车体左前方（车头右偏 beta）→ 需左转修正；
       side=-1: 目标在车体右前方（车头左偏 beta）→ 需右转修正。
       车体系方位角 |beta|=beta_deg，检测距离 D0。"""
    beta = beta_deg * DEG2RAD
    y0 = -D0 * np.cos(beta)                    # 车在目标后方
    psi0 = 90.0 - side * beta_deg              # 车头相对路面轴线偏 side*beta
    return 0.0, y0, psi0, v_mps, side


def run_bounded(ctrl_factory, beta_deg, D0, v_mps, side=+1.0, max_t=4.0):
    """跑一次并统计是否出路面。返回 (res, max_|x|, exit_time_of_maxx)"""
    x0, y0, p0, v, s = make_bearing_scenario(beta_deg, D0, v_mps, side)
    # 需要手动重放以跟踪 x(t)（run_sim 不返回轨迹）
    from lqr_vs_pd_enter_sim import VehicleSim, norm_deg, DT, T_VIS
    ctrl = ctrl_factory()
    ctrl.reset(p0)
    veh = VehicleSim(x0, y0, p0, v, tx=0.0, ty=0.0, seed=1)
    last_fx, last_fy = 32767, 32767
    max_abs_x, t_maxx = 0.0, 0.0
    conv_t = None
    n = int(max_t / DT)
    D = 1e9
    for i in range(n):
        fx, fy = veh.measure()
        nv = fx is not None
        if nv:
            last_fx, last_fy = fx, fy
        om = ctrl.update(last_fx, last_fy, np.degrees(veh.yaw_meas()), v, DT, nv)
        veh.step(om)
        ax = abs(veh.x)
        if ax > max_abs_x:
            max_abs_x, t_maxx = ax, i * DT
        # e 收敛判据（与 metrics 一致 |e|<50mm）
        dx, dy = 0.0 - veh.x, 0.0 - veh.y
        fxx = -dx * np.sin(veh.psi) + dy * np.cos(veh.psi)
        fyy = dx * np.cos(veh.psi) + dy * np.sin(veh.psi)
        DD = np.hypot(fxx, fyy)
        if DD < PVC_CONFIRM_D:
            break
        if DD <= D0:
            beta_cur = np.arctan2(fxx, fyy)
            pe = norm_deg(p0 - np.degrees(veh.psi)) * DEG2RAD
            e = DD * np.sin(beta_cur - pe)
            if conv_t is None and abs(e) < 0.05:
                conv_t = i * DT
    return {"max_x": max_abs_x, "t_maxx": t_maxx, "conv": conv_t,
            "done": D < PVC_CONFIRM_D, "veh": veh}


def scan(beta_list, v_list, controllers):
    print(f"约束: 路面半宽 25cm - 车半宽 13.5cm => 车体中心线 |x|≤{MAX_X*100:.1f}cm")
    print(f"检测 D0=1.5m, entry_yaw=检测瞬间航向, e 收敛判据 |e|<50mm")
    for v in v_list:
        print(f"\n===== v={v} m/s =====")
        for beta in beta_list:
            e0 = 1.5 * np.sin(beta * DEG2RAD)
            row = [f"β0={beta:3d}°(e0={e0*100:.0f}cm)"]
            for name, fac in controllers:
                r = run_bounded(fac, beta, 1.5, v)
                ok = "✓" if r["max_x"] <= MAX_X + 1e-6 else "✗出路面"
                conv = f"{r['conv']:.2f}s" if r["conv"] is not None else "  —  "
                row.append(f"{name}: max|x|={r['max_x']*100:4.1f}cm[{ok}] conv={conv}")
            print("   " + "  ".join(row))


if __name__ == "__main__":
    ctrls = [
        ("PD50", lambda: PDController(P_PSI=7.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05)),
        ("PD11", lambda: PDController(P_PSI=7.0, D_PSI=1.0, K_E=11.0, psi_d_tau=0.05)),
        ("LQR ", lambda: LQRController()),
    ]
    scan(list(range(5, 46, 5)), [1.0, 1.5, 2.0], ctrls)
