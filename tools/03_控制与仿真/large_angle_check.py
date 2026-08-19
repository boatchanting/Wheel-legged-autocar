# -*- coding: utf-8 -*-
"""
大角度进入段校验脚本（复用 lqr_vs_pd_enter_sim.py 的模型）
场景：检测瞬间 β0=35°（目标相对车体右偏 35°），D0=1.5m，与用户实测一致。
验证：当前 K_E=50/P=7/D=3.5 在大角度下到底是「过冲」还是「圆弧侧撞」，
      以及 LQR 对照；并给出车辆轨迹（看是否拐成圆弧、横向偏移量）。
"""
import numpy as np
from lqr_vs_pd_enter_sim import (run_sim, PDController, LQRController, metrics,
                                 DEG2RAD, DETECT_RANGE_M, PVC_CONFIRM_D)


def make_large_angle_scenario(beta_deg, D0, v_mps, psi0_deg=90.0):
    """构造大角度工况：目标固定在世界原点(0,0)；车在 (x0,y0)，初始航向 psi0，
       使检测瞬间目标相对车体方位角=beta_deg、距离=D0。"""
    beta = beta_deg * DEG2RAD
    fx, fy = D0 * np.sin(beta), D0 * np.cos(beta)   # 车体系：右正、前正
    # 车体→世界（psi0=90° 车头朝 +y）：x0=fx, y0=-fy
    x0, y0 = fx, -fy
    return x0, y0, psi0_deg, v_mps


def check(beta_deg, D0, v_mps, label, ctrl_factory):
    x0, y0, p0, v = make_large_angle_scenario(beta_deg, D0, v_mps)
    res = run_sim(ctrl_factory(), x0, y0, p0, v, max_t=4.0, seed=1)
    m = metrics(res)
    veh = res["veh"]
    # 轨迹相对初始参考线（x=0 竖线）的最大/最终横向偏移
    xs = [veh.x]
    # 记录过程中的横向偏移需要重跑或从 res 反推；这里直接用终态 + 峰值
    print(f"[{label}] β0={beta_deg}° D0={D0}m v={v}m/s")
    print(f"   done={m['done']}  conv={m['conv_s']}s  peak_e={m['peak_e_mm']:.0f}mm  "
          f"peak_psi={m['peak_psi_deg']:.1f}°  T={m['T']:.2f}s")
    print(f"   终点: pos=({veh.x:.2f},{veh.y:.2f})  heading={np.degrees(veh.psi):.1f}°  "
          f"|D|={np.hypot(veh.tx-veh.x, veh.ty-veh.y):.2f}m")
    return res


if __name__ == "__main__":
    print("=" * 70)
    print("场景：β0=35°, D0=1.5m（目标在世界原点，车初始航向朝目标侧外）")
    print("=" * 70)
    for v in (1.0, 2.0):
        print(f"\n--- v={v} m/s ---")
        check(35.0, 1.5, v, "PD 50/7/3.5",
              lambda: PDController(P_PSI=7.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05))
        check(35.0, 1.5, v, "PD 11/7/1.0 ",
              lambda: PDController(P_PSI=7.0, D_PSI=1.0, K_E=11.0, psi_d_tau=0.05))
        check(35.0, 1.5, v, "LQR 120/32   ",
              lambda: LQRController())

    print("\n" + "=" * 70)
    print("中等角度边界：β0=20° D0=1.5m v=2.0 —— PD 50/7/3.5 对照")
    print("=" * 70)
    check(20.0, 1.5, 2.0, "PD 50/7/3.5",
          lambda: PDController(P_PSI=7.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05))

    print("\n" + "=" * 70)
    print("场景：β0=35°, D0=1.5m, v=1.0 —— 输出轨迹序列（每 50ms 采样）")
    print("=" * 70)
    x0, y0, p0, v = make_large_angle_scenario(35.0, 1.5, 1.0)
    ctrl = PDController(P_PSI=7.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05)
    res = run_sim(ctrl, x0, y0, p0, v, max_t=4.0, seed=1)
    print("   t(s)   x(m)    y(m)   psi(deg)  |e|(m)   omega")
    veh = res["veh"]
    # 用轻量重放打印轨迹
    from lqr_vs_pd_enter_sim import VehicleSim, norm_deg, DT, T_VIS
    veh2 = VehicleSim(x0, y0, p0, v, tx=0.0, ty=0.0, seed=1)
    ctrl2 = PDController(P_PSI=7.0, D_PSI=3.5, K_E=50.0, psi_d_tau=0.05)
    ctrl2.reset(p0)
    last_fx, last_fy = 32767, 32767
    n = int(4.0 / DT)
    for i in range(n):
        fx, fy = veh2.measure()
        nv = fx is not None
        if nv:
            last_fx, last_fy = fx, fy
        om = ctrl2.update(last_fx, last_fy, np.degrees(veh2.yaw_meas()), v, DT, nv)
        veh2.step(om)
        if i % 25 == 0:
            dx, dy = 0.0 - veh2.x, 0.0 - veh2.y
            fxx = -dx * np.sin(veh2.psi) + dy * np.cos(veh2.psi)
            fyy = dx * np.cos(veh2.psi) + dy * np.sin(veh2.psi)
            DD = np.hypot(fxx, fyy)
            beta = np.arctan2(fxx, fyy)
            pe = norm_deg(p0 - np.degrees(veh2.psi)) * DEG2RAD
            e = DD * np.sin(beta - pe)
            print(f"  {i*DT:6.2f}  {veh2.x:6.2f}  {veh2.y:6.2f}  "
                  f"{np.degrees(veh2.psi):7.1f}  {abs(e):6.2f}  {veh2.omega:6.2f}")
        if np.hypot(0 - veh2.x, 0 - veh2.y) < PVC_CONFIRM_D:
            break
