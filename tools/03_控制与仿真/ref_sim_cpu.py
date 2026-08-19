# -*- coding: utf-8 -*-
"""
基准 CPU 仿真（精确复刻 lqr_vs_pd_enter_sim.py 逻辑），用于与 GPU 仿真对照。
输出指定 (K_E,P,D,x0,v) 的上桥点状态与完整轨迹。
"""
import numpy as np

DEG2RAD = np.pi / 180.0
DT = 0.002
T_VIS = 0.030
DT_VIS_STEPS = int(round(T_VIS / DT))
W_MAX = 2.2
W_SLEW = 9.0
DETECT_RANGE_M = 1.5
PVC_CONFIRM_D = 0.40
TURN_ANG_KP = -8.0
SIG_FX_MM, SIG_FY_MM, SIG_YAW_DEG = 10.0, 15.0, 0.4


def norm_deg(a):
    a = np.asarray(a, dtype=float)
    while np.any(a > 180.0):
        a = np.where(a > 180.0, a - 360.0, a)
    while np.any(a < -180.0):
        a = np.where(a < -180.0, a + 360.0, a)
    return a


def clamp(x, lo, hi):
    return max(lo, min(hi, x))


class PDController:
    def __init__(self, P_PSI, D_PSI, K_E, psi_d_tau=0.05):
        self.P_PSI, self.D_PSI, self.K_E = P_PSI, D_PSI, K_E
        self.psi_d_tau = psi_d_tau
        self.entry_yaw = 0.0
        self.psi_err_prev = 0.0
        self.e_prev = 0.0
        self.psi_valid = False
        self.vis_valid = False
        self.psi_err_dot_f = 0.0

    def reset(self, entry_yaw_deg):
        self.entry_yaw = entry_yaw_deg
        self.psi_err_prev = 0.0
        self.e_prev = 0.0
        self.psi_valid = False
        self.vis_valid = False
        self.psi_err_dot_f = 0.0

    def update(self, fx_mm, fy_mm, yaw_deg, v_mps, dt, new_vision):
        psi_err = norm_deg(self.entry_yaw - yaw_deg) * DEG2RAD
        if self.psi_valid:
            psi_err_dot = (psi_err - self.psi_err_prev) / dt
        else:
            psi_err_dot = 0.0
        if self.psi_d_tau > 0.0:
            alpha = dt / (self.psi_d_tau + dt)
            self.psi_err_dot_f += alpha * (psi_err_dot - self.psi_err_dot_f)
            psi_err_dot = self.psi_err_dot_f
        self.psi_err_prev = psi_err
        self.psi_valid = True

        if new_vision and not (fx_mm == 32767 or fy_mm == 32767):
            D_m = np.hypot(fx_mm, fy_mm) / 1000.0
            beta = np.arctan2(fx_mm, fy_mm)
            e_m = D_m * np.sin(beta - psi_err)
            self.e_prev = e_m
            self.vis_valid = True
        else:
            e_m = self.e_prev if self.vis_valid else 0.0

        omega = self.P_PSI * psi_err + self.D_PSI * psi_err_dot + self.K_E * e_m
        return clamp(omega, -W_MAX, W_MAX)


class VehicleSim:
    def __init__(self, x0, y0, psi0_deg, v_mps, tx=0.0, ty=0.0, seed=1,
                 vision_latency_frames=1, tau_act=0.03):
        self.x, self.y = x0, y0
        self.psi = psi0_deg * DEG2RAD
        self.v = v_mps
        self.tx, self.ty = tx, ty
        self.omega = 0.0
        self.rng = np.random.default_rng(seed)
        self.lat = vision_latency_frames
        self.buf = [(32767, 32767)] * (self.lat + 1)
        self.tick = 0
        self.tau_act = tau_act

    def yaw_meas(self):
        return self.psi + self.rng.normal(0.0, SIG_YAW_DEG * DEG2RAD)

    def step(self, omega_cmd):
        acc = (omega_cmd - self.omega) / self.tau_act
        acc = clamp(acc, -W_SLEW, W_SLEW)
        self.omega = clamp(self.omega + acc * DT, -W_MAX, W_MAX)
        self.x += self.v * np.cos(self.psi) * DT
        self.y += self.v * np.sin(self.psi) * DT
        self.psi += self.omega * DT

    def measure(self):
        self.tick += 1
        if self.tick % DT_VIS_STEPS != 1:
            return None, None
        dx, dy = self.tx - self.x, self.ty - self.y
        fx = -dx * np.sin(self.psi) + dy * np.cos(self.psi)
        fy = dx * np.cos(self.psi) + dy * np.sin(self.psi)
        D = np.hypot(fx, fy)
        if D > DETECT_RANGE_M:
            fx, fy = 32767, 32767
        else:
            fx = fx * 1000.0 + self.rng.normal(0, SIG_FX_MM)
            fy = fy * 1000.0 + self.rng.normal(0, SIG_FY_MM)
        self.buf.append((fx, fy))
        self.buf.pop(0)
        return self.buf[0]


def run(x0, v, P_PSI, D_PSI, K_E, entry_yaw_deg=90.0, max_t=4.0, seed=1):
    ctrl = PDController(P_PSI=P_PSI, D_PSI=D_PSI, K_E=K_E)
    ctrl.reset(entry_yaw_deg)
    veh = VehicleSim(x0, -1.5, 90.0, v, seed=seed)
    last_fx, last_fy = 32767, 32767
    n = int(max_t / DT)
    traj = []
    for i in range(n):
        fx, fy = veh.measure()
        nv = fx is not None
        if nv:
            last_fx, last_fy = fx, fy
        om = ctrl.update(last_fx, last_fy, np.degrees(veh.yaw_meas()), v, DT, nv)
        veh.step(om)
        traj.append((i * DT, veh.x, veh.y, np.degrees(veh.psi), veh.omega, last_fx, last_fy))
        if np.hypot(veh.x, veh.y) < PVC_CONFIRM_D:
            psi_err = norm_deg(entry_yaw_deg - np.degrees(veh.yaw_meas())) * DEG2RAD
            return veh.x, psi_err, traj
    return None, None, traj


if __name__ == "__main__":
    CASES = [
        (15.0, 9.5, 0.25, 0.70, 1.0),
        (15.0, 9.5, 0.25, 0.40, 1.5),
        (11.0, 7.0, 0.00, 0.50, 1.0),
        (50.0, 50.0, 3.5, 0.20, 1.0),
        (50.0, 50.0, 3.5, 0.10, 1.0),
        (50.0, 50.0, 3.5, 0.30, 1.0),
        (12.0, 8.0, 0.00, 0.40, 1.0),
        (14.0, 9.0, 0.20, 0.70, 1.0),
    ]
    for ke, p, d, x0, v in CASES:
        xe, pse, traj = run(x0, v, p, d, ke)
        if xe is None:
            print(f"K_E={ke:5.1f} P={p:5.1f} D={d:4.2f} x0={x0:.2f} v={v}: 未到上桥点")
        else:
            ok = abs(xe) <= 0.115 and abs(np.degrees(pse)) <= 20.0
            print(f"K_E={ke:5.1f} P={p:5.1f} D={d:4.2f} x0={x0:.2f} v={v}: "
                  f"x={xe*100:6.1f}cm ψ={np.degrees(pse):6.1f}° ok={ok}")

    # PD50@0.2 轨迹前 30 拍（对照 GPU 行为）
    print("\n--- PD50@x0=0.2 v=1.0 轨迹（每 20 步采样）---")
    xe, pse, traj = run(0.20, 1.0, 50.0, 3.5, 50.0)
    for t, x, y, ps, om, lfx, lfy in traj[::20]:
        print(f"  t={t:5.2f} x={x*100:6.1f}cm y={y*100:6.1f}cm psi={ps:7.1f}° omega={om:5.2f}")
