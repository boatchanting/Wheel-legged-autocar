# -*- coding: utf-8 -*-
"""
=============================================================================
lqr_vs_pd_enter_sim.py —— 单边桥/坡道「进入段」方向控制：LQR（现役） vs 新 PD 结构
=============================================================================
目标：
  1. 数值复刻当前固件 LQR（vision_entry_lqr.c）在进入段的行为，作为基准；
  2. 实现用户提出的新结构：
        ω = P_PSI·ψ_err + D_PSI·ψ_err' + K_E·e_visual          （航向 PD + 视觉扰动）
    其中 e_visual 用修复后的横向偏差重建公式 e = D·sin(β − ψ_err)；
  3. 参数扫描，找出一组能「基本复刻」LQR 行为的 PD 参数；
  4. 多工况（横向偏差 / 航向偏差 / 不同速度）验证鲁棒性。

物理/工程约束（与固件一致）：
  - 控制周期 2 ms；视觉 IPC 更新 30 ms（含 1 帧延迟 + 高斯噪声）；
  - 转向执行器：|ω| ≤ W_MAX=2.2 rad/s，slew ≤ 9 rad/s²（转向内环 PD 的物理限制）；
  - 视觉检测距离 1.5 m，进入段结束判据 D < 0.4 m（模拟 PVC 确认压上入口）。

坐标系（与固件一致）：
  - 世界系：x 右、y 前（北向上），车头方向单位向量 = (cosψ, sinψ)，ψ 为 IMU 航向；
  - 车身系：fx 右正、fy 前正；β = atan2(fx, fy)；D = hypot(fx, fy)；
  - ψ_err = normalize(entry_yaw − ψ)；e = D·sin(β − ψ_err)。
=============================================================================
"""
import numpy as np

# ---------------------------------------------------------------- 基础常量
DEG2RAD = np.pi / 180.0
DT      = 0.002          # 控制周期 2 ms
T_VIS   = 0.030          # 视觉 IPC 更新周期 30 ms
DT_VIS_STEPS = int(round(T_VIS / DT))

# 固件参数（vision_entry_lqr.h 现役值）
LQR_QY      = 120.0
LQR_QPSI    = 32.0
LQR_W_MAX   = 2.2         # rad/s
LQR_V_FLOOR = 0.3         # m/s
W_SLEW      = 9.0         # rad/s^2（转向内环物理限制，勿动）
DETECT_RANGE_M = 1.5
PVC_CONFIRM_D = 0.40      # 模拟 PVC 确认压上入口的距离
TURN_ANG_KP = -8.0        # err_degree 换算（pid-new.h）

# 视觉噪声（mm，经验值）
SIG_FX_MM, SIG_FY_MM = 10.0, 15.0
# IMU 相对航向噪声（deg，1σ）—— 大 D 项放大的正是这个噪声，必须建模
SIG_YAW_DEG = 0.4


# ---------------------------------------------------------------- 工具函数
def norm_deg(a):
    a = np.asarray(a, dtype=float)
    while np.any(a > 180.0):
        a = np.where(a > 180.0, a - 360.0, a)
    while np.any(a < -180.0):
        a = np.where(a < -180.0, a + 360.0, a)
    return a


def clamp(x, lo, hi):
    return max(lo, min(hi, x))


# ---------------------------------------------------------------- 控制器
class LQRController:
    """现役 LQR：ω = k1·e + k2(v)·ψ_err（严格复刻固件公式）"""
    name = "LQR(现役)"

    def __init__(self):
        self.entry_yaw = 0.0

    def reset(self, entry_yaw_deg):
        self.entry_yaw = entry_yaw_deg

    def update(self, fx_mm, fy_mm, yaw_deg, v_mps, dt, new_vision=False):
        if fx_mm == 32767 or fy_mm == 32767:
            return 0.0
        D_m = np.hypot(fx_mm, fy_mm) / 1000.0
        beta = np.arctan2(fx_mm, fy_mm)              # rad
        psi_err = norm_deg(self.entry_yaw - yaw_deg) * DEG2RAD
        e_m = D_m * np.sin(beta - psi_err)           # 修复后的横向偏差
        k1 = np.sqrt(LQR_QY)
        k2 = np.sqrt(2.0 * max(v_mps, LQR_V_FLOOR) * k1 + LQR_QPSI)
        omega = k1 * e_m + k2 * psi_err
        return clamp(omega, -LQR_W_MAX, LQR_W_MAX)


class PDController:
    """新结构：ω = P·ψ_err + D·ψ_err' + K_E·e_visual
       航向 PD 维持回归正确角度，视觉横向偏差作为扰动注入。
       psi_d_tau>0 时对 ψ_err' 做一阶低通滤波（大 D 必须配滤波，防噪声放大）。"""
    name = "PD+视觉扰动(新)"

    def __init__(self, P_PSI, D_PSI, K_E, K_ED=0.0, psi_d_tau=0.0):
        self.P_PSI, self.D_PSI, self.K_E, self.K_ED = P_PSI, D_PSI, K_E, K_ED
        self.psi_d_tau = psi_d_tau
        self.entry_yaw = 0.0
        self.psi_err_prev = 0.0
        self.e_prev = 0.0
        self.psi_valid = False
        self.vis_valid = False
        self.psi_err_dot_f = 0.0
        self._last_dt = DT

    def reset(self, entry_yaw_deg):
        self.entry_yaw = entry_yaw_deg
        self.psi_err_prev = 0.0
        self.e_prev = 0.0
        self.psi_valid = False
        self.vis_valid = False
        self.psi_err_dot_f = 0.0

    def update(self, fx_mm, fy_mm, yaw_deg, v_mps, dt, new_vision):
        # 航向误差（每控制周期可更新，来自 IMU）
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

        # 视觉横向偏差（扰动源），仅新视觉帧更新
        if new_vision and not (fx_mm == 32767 or fy_mm == 32767):
            D_m = np.hypot(fx_mm, fy_mm) / 1000.0
            beta = np.arctan2(fx_mm, fy_mm)
            e_m = D_m * np.sin(beta - psi_err)       # 修复后的横向偏差
            if self.vis_valid:
                e_dot = (e_m - self.e_prev) / T_VIS
            else:
                e_dot = 0.0
            self.e_prev = e_m
            self.vis_valid = True
        else:
            e_m = self.e_prev if self.vis_valid else 0.0
            e_dot = 0.0

        omega = (self.P_PSI * psi_err + self.D_PSI * psi_err_dot
                 + self.K_E * e_m + self.K_ED * e_dot)
        return clamp(omega, -LQR_W_MAX, LQR_W_MAX)


# ---------------------------------------------------------------- 车辆+环境
class VehicleSim:
    """平面运动学 + 转向执行器（slew+幅值限制）+ 视觉测量（周期/延迟/噪声）"""

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
        self.tau_act = tau_act          # 转向内环等效时间常数（s）

    def yaw_meas(self):
        """IMU 相对航向测量值 = 真值 + 白噪声（控制周期 2ms 采样）。"""
        return self.psi + self.rng.normal(0.0, SIG_YAW_DEG * DEG2RAD)

    def step(self, omega_cmd):
        """执行器一阶惯性 + slew 限制 + 幅值限制"""
        acc = (omega_cmd - self.omega) / self.tau_act
        acc = clamp(acc, -W_SLEW, W_SLEW)
        self.omega = clamp(self.omega + acc * DT, -LQR_W_MAX, LQR_W_MAX)
        self.x += self.v * np.cos(self.psi) * DT
        self.y += self.v * np.sin(self.psi) * DT
        self.psi += self.omega * DT

    def measure(self):
        """返回新视觉帧 (fx_mm, fy_mm)；非新帧返回 (None, None)"""
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

    def state(self):
        return self.x, self.y, self.psi


# ---------------------------------------------------------------- 仿真主函数
def run_sim(controller, x0, y0, psi0_deg, v_mps, max_t=4.0, seed=1,
            entry_yaw_deg=None, tau_act=0.03):
    """跑一次进入段仿真，返回时间序列字典。"""
    if entry_yaw_deg is None:
        entry_yaw_deg = psi0_deg
    controller.reset(entry_yaw_deg)
    veh = VehicleSim(x0, y0, psi0_deg, v_mps, seed=seed, tau_act=tau_act)

    ts, e_hist, psi_hist, omega_hist, yaw_hist = [], [], [], [], []
    n = int(max_t / DT)
    D = 1e9
    last_fx, last_fy = 32767, 32767   # 采样保持视觉值（与固件 IPC 共享内存一致）
    for i in range(n):
        fx, fy = veh.measure()
        new_vis = fx is not None
        if new_vis:
            last_fx, last_fy = fx, fy
        yaw_m = np.degrees(veh.yaw_meas())   # 控制器看到的是带噪 IMU 航向
        omega_cmd = controller.update(last_fx, last_fy, yaw_m, v_mps, DT, new_vis)
        veh.step(omega_cmd)

        # 记录（psi 误差用测量航向算，贴近固件观测）
        if new_vis:
            dx, dy = veh.tx - veh.x, veh.ty - veh.y
            fx_g, fy_g = (-dx*np.sin(veh.psi)+dy*np.cos(veh.psi),
                          dx*np.cos(veh.psi)+dy*np.sin(veh.psi))
            D = np.hypot(fx_g, fy_g)
            beta = np.arctan2(fx_g, fy_g)
            psi_err = norm_deg(entry_yaw_deg - yaw_m) * DEG2RAD
            e = D * np.sin(beta - psi_err)
            ts.append(i * DT)
            e_hist.append(e)
            psi_hist.append(np.degrees(psi_err))
            omega_hist.append(veh.omega)
            yaw_hist.append(yaw_m)
        if D < PVC_CONFIRM_D:
            break

    return {"t": np.array(ts), "e": np.array(e_hist),
            "psi": np.array(psi_hist), "omega": np.array(omega_hist),
            "yaw": np.array(yaw_hist), "veh": veh, "done": D < PVC_CONFIRM_D}


# ---------------------------------------------------------------- 指标
def itae(res, q_e=1.0, q_psi=1.0):
    """时间加权绝对误差积分（越小越好），多信号加权。"""
    if len(res["t"]) == 0:
        return 1e9
    t = res["t"]
    ie = np.trapezoid(t * np.abs(res["e"]), t) * q_e
    ip = np.trapezoid(t * np.abs(res["psi"]) * DEG2RAD, t) * q_psi
    return ie + ip


def metrics(res):
    """返回收敛时间(s,|e|<50mm)、峰值横向偏差、结束状态等"""
    t, e, psi = res["t"], res["e"], res["psi"]
    conv = np.nan
    idx = np.where(np.abs(e) < 0.05)[0]
    if len(idx):
        conv = t[idx[0]]
    peak_e = np.max(np.abs(e)) if len(e) else np.nan
    peak_psi = np.max(np.abs(psi)) if len(psi) else np.nan
    return {"conv_s": conv, "peak_e_mm": peak_e * 1000 if not np.isnan(peak_e) else np.nan,
            "peak_psi_deg": peak_psi if not np.isnan(peak_psi) else np.nan,
            "done": res["done"], "T": t[-1] if len(t) else np.nan}


# ---------------------------------------------------------------- 测试工况
# 目标点固定在世界系原点 (0,0)；车从 (x0, y0=-1.5) 出发，车头朝 y 正方向（=90°）
# 即目标在车正前方 1.5m；x0 为初始横向偏差（>0 偏右）。
# entry_yaw 自动取 psi0（与固件 enter_task 锁存当前航向一致）。
SCENARIOS = [
    # (名称, x0, y0, psi0_deg, v_mps)
    ("A_right0.3m_align",      0.30, -1.50,  90.0, 0.5),
    ("B_align_yaw+5deg",       0.00, -1.50,  95.0, 0.5),
    ("C_right0.3m_yaw+5deg",   0.30, -1.50,  95.0, 0.5),
    ("D_left0.3m_yaw-5deg",   -0.30, -1.50,  85.0, 0.5),
    ("E_right0.3m_high2mps",   0.30, -1.50,  90.0, 2.0),
    ("F_align_yaw+5_high2mps", 0.00, -1.50,  95.0, 2.0),
]


def run_all(controller_factory, verbose=False):
    results = {}
    for name, x0, y0, p0, v in SCENARIOS:
        ctrl = controller_factory()
        res = run_sim(ctrl, x0, y0, p0, v, seed=1)
        results[name] = res
        if verbose:
            m = metrics(res)
            print(f"  {name:26s} conv={m['conv_s']:.2f}s "
                  f"peak_e={m['peak_e_mm']:.0f}mm peak_psi={m['peak_psi_deg']:.1f}° "
                  f"done={m['done']}")
    return results


def total_cost(results, weights=None):
    """多工况加权 ITAE 总代价（评估 PD 与 LQR 的接近程度 + 自身收敛质量）"""
    w = weights or {k: 1.0 for k in results}
    cost = 0.0
    for k, r in results.items():
        cost += w[k] * itae(r)
    return cost


# ---------------------------------------------------------------- 参数扫描
def diff_cost(pd_res, lqr_res, tmax=3.0, w_e=1.0, w_psi=1.0):
    """衡量 PD 与 LQR 行为接近程度：公共时间轴上 e/ψ 轨迹的加权 RMS 差。
    这是『复刻 LQR』的主目标（区别于自身 ITAE）。"""
    t = np.arange(0.0, tmax, 0.02)
    de = np.interp(t, pd_res["t"], pd_res["e"]) - np.interp(t, lqr_res["t"], lqr_res["e"])
    dp = (np.interp(t, pd_res["t"], pd_res["psi"])
          - np.interp(t, lqr_res["t"], lqr_res["psi"]))
    de = np.nan_to_num(de)
    dp = np.nan_to_num(dp)
    return w_e * np.sqrt(np.mean(de ** 2)) + w_psi * np.sqrt(np.mean((dp * DEG2RAD) ** 2))


def scan_pd():
    """网格搜索 PD 参数：以『与 LQR 轨迹最接近』为目标（diff_cost）。"""
    # 先跑 LQR 基线
    print("== LQR 基线（现役参数）==")
    lqr_res = run_all(lambda: LQRController(), verbose=True)

    print("\n== PD 参数扫描（目标：轨迹接近 LQR）==")
    best = None
    best_cost = 1e18
    grid = []
    for K_E in np.arange(8.0, 14.5, 0.5):
        for P in np.arange(4.0, 9.0, 0.5):
            for D in np.arange(0.0, 1.51, 0.25):
                grid.append((round(float(K_E), 3), round(float(P), 3),
                             round(float(D), 3)))
    for i, (K_E, P, D) in enumerate(grid):
        res = run_all(lambda: PDController(P_PSI=P, D_PSI=D, K_E=K_E))
        c = sum(diff_cost(res[k], lqr_res[k]) for k in res)
        if c < best_cost:
            best_cost = c
            best = (K_E, P, D, c)
    print(f"扫描 {len(grid)} 组，最优: K_E={best[0]} P={best[1]} D={best[2]} "
          f"diff_cost={best[3]:.4f}（越小越接近 LQR）")

    # 最优参数详细对比
    print("\n== 最优 PD 参数逐工况对比 ==")
    best_res = run_all(lambda: PDController(P_PSI=best[1], D_PSI=best[2],
                                            K_E=best[0]), verbose=True)
    print("\n== LQR 逐工况（对照）==")
    run_all(lambda: LQRController(), verbose=True)
    return best, lqr_res, best_res


# ---------------------------------------------------------------- 绘图
def plot_compare(lqr_res, pd_res, out_png):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    fig, axes = plt.subplots(3, len(SCENARIOS), figsize=(4.2 * len(SCENARIOS), 9))
    for j, (name, *_ ) in enumerate(SCENARIOS):
        rl, rp = lqr_res[name], pd_res[name]
        for i, (key, ylab) in enumerate([("e", "e (m)"), ("psi", "psi_err (deg)"),
                                         ("omega", "omega (rad/s)")]):
            ax = axes[i][j]
            ax.plot(rl["t"], rl[key], label="LQR", lw=1.6)
            ax.plot(rp["t"], rp[key], label="PD", lw=1.6, ls="--")
            ax.set_title(name, fontsize=9)
            ax.set_ylabel(ylab, fontsize=8)
            ax.grid(alpha=0.3)
            if i == 0:
                ax.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(out_png, dpi=110)
    print(f"\n对比图已保存: {out_png}")


if __name__ == "__main__":
    best, lqr_res, best_res = scan_pd()
    out = r"d:\WORKS\2026LunTui\project2\tools\03_控制与仿真\fig_lqr_vs_pd_enter.png"
    plot_compare(lqr_res, best_res, out)
