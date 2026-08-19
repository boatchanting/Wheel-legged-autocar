# -*- coding: utf-8 -*-
"""
MuJoCo 物理仿真：2kg 并联轮腿车进入段方向控制
- 车：底盘 1.4kg + 两轮 0.3kg×2 = 2.0kg；差分驱动；橡胶轮胎。
- 路面：塑胶操场跑道平面。摩擦系数：
    干 μ≈0.90 / 湿 μ≈0.55 / 随机 μ~U(0.45,0.95)+时变扰动。
- 控制：进入段 PD（K_E/P/D）→ ω_cmd（slew+W_MAX）→ 差分轮速 → 物理实现。
- 验收：上桥瞬间（D<0.4m）|x|≤11.5cm 且 |ψ|≤20°。
- 并行：multiprocessing 用满 192 核。
用法：
  python mujoco_entry_sim.py calibrate          # 标定轮速方向/符号
  python mujoco_entry_sim.py sweep             # 候选参数×速度×摩擦×x0 扫描
"""
import numpy as np, sys, time, os
from concurrent.futures import ProcessPoolExecutor

DEG2RAD = np.pi / 180.0
DT = 0.002
N_VIS = 15                      # 30ms
W_MAX = 2.2
W_SLEW = 9.0
TAU_D = 0.05
DETECT = 1.5
PVC = 0.4
X_TOL = 0.115
PSI_TOL_DEG = 20.0
SIG_FX_MM, SIG_FY_MM, SIG_YAW_DEG = 10.0, 15.0, 0.4
L_WHEEL = 0.20                  # 轮距
R_WHEEL = 0.06                  # 轮半径
KF_FWD = 6.0                    # 前向速度 P（N per m/s）
KT_YAW = 1.5                    # 航向角速度内环 P（Nm per rad/s）

XML = """<mujoco model="wheelleg_entry">
  <compiler angle="degree" coordinate="local" inertiafromgeom="true"/>
  <option timestep="0.002" gravity="0 0 -9.81"/>
  <size nstack="800000" nconmax="3000"/>
  <default>
    <geom friction="1.0 0.06 0.0006" condim="3"/>
    <joint damping="0.01"/>
  </default>
  <worldbody>
    <light diffuse="0.8 0.8 0.8" pos="0 0 3"/>
    <geom name="ground" type="plane" size="10 10 0.1" friction="0.9 0.06 0.0006"
          rgba="0.85 0.82 0.72 1" contype="2" conaffinity="2"/>
    <body name="chassis" pos="0 0 0.10">
      <joint name="sx" type="slide" axis="1 0 0"/>
      <joint name="sy" type="slide" axis="0 1 0"/>
      <joint name="sz" type="slide" axis="0 0 1"/>
      <joint name="yaw" type="hinge" axis="0 0 1"/>
      <geom name="chassis_g" type="box" size="0.20 0.135 0.035" mass="1.4"
            euler="-3 0 0" rgba="0.2 0.4 0.8 1" contype="1" conaffinity="1"/>
      <!-- 轮腿：重心前倾 3°（并联轮腿靠关节实现：先重心前移→轮胎追上→加速） -->
      <body name="wheel_l" pos="-0.10 0 -0.04">
        <joint name="wl" type="hinge" axis="1 0 0"/>
        <geom name="wl_g" type="capsule" size="0.06 0.018" mass="0.3"
              euler="0 90 0" friction="1.0 0.06 0.0006" rgba="0.1 0.1 0.1 1"
              contype="2" conaffinity="2"/>
      </body>
      <body name="wheel_r" pos="0.10 0 -0.04">
        <joint name="wr" type="hinge" axis="1 0 0"/>
        <geom name="wr_g" type="capsule" size="0.06 0.018" mass="0.3"
              euler="0 90 0" friction="1.0 0.06 0.0006" rgba="0.1 0.1 0.1 1"
              contype="2" conaffinity="2"/>
      </body>
    </body>
    <body name="target" pos="0 0 0.01">
      <geom name="tg" type="box" size="0.15 0.15 0.005" rgba="1 0.2 0.2 0.6"
            contype="0" conaffinity="0"/>
    </body>
  </worldbody>
  <actuator>
    <motor name="ml" joint="wl" ctrlrange="-8 8" gear="1"/>
    <motor name="mr" joint="wr" ctrlrange="-8 8" gear="1"/>
  </actuator>
</mujoco>"""


def _norm_rad(a):
    return np.mod(a + np.pi, 2 * np.pi) - np.pi


def _body_heading(d):
    """车体前进方向（局部 +y 轴）的世界方位角 ψ（rad）。
    约定与运动学仿真一致：前向=(cosψ,sinψ)，朝 +y 时 ψ=90°。"""
    R = d.xmat[d.body("chassis").id].reshape(3, 3)
    fwd = R[:, 1]                       # 局部 +y → 世界
    return np.arctan2(fwd[1], fwd[0])


def _measure_target(d, psi, rng):
    """目标(0,0)在车体系坐标（fx 右正、fy 前正，mm）；D>1.5m 无效。
    公式与运动学仿真完全一致：fx=-dx·sinψ+dy·cosψ, fy=dx·cosψ+dy·sinψ。"""
    dx = -d.qpos[0]                     # 目标(0,0) - 车
    dy = -d.qpos[1]
    fx = -dx * np.sin(psi) + dy * np.cos(psi)
    fy = dx * np.cos(psi) + dy * np.sin(psi)
    D = np.hypot(fx, fy)
    if D > DETECT:
        return 32767.0, 32767.0
    return fx * 1000.0 + rng.normal(0, SIG_FX_MM), fy * 1000.0 + rng.normal(0, SIG_FY_MM)


def _set_mu(m, d, mu):
    """设置有效接触摩擦 μ（本版 MuJoCo frictioncombine=max，需同时设地面与轮子）。"""
    for name in ("ground", "wl_g", "wr_g"):
        g = m.geom(name).id
        m.geom_friction[g, 0] = mu


def run_episode(params, x0, v, mu_mode, seed, max_t=4.0, trace=False):
    """单次物理仿真。返回 dict。"""
    KE, P, D = params
    rng = np.random.default_rng(seed)
    try:
        import mujoco
        m = mujoco.MjModel.from_xml_string(XML)
        d = mujoco.MjData(m)
    except Exception as e:
        return {"err": str(e)}

    # 摩擦设定（橡胶轮胎在塑胶跑道路面：干 μ≈0.90，湿 μ≈0.40，随机扰动 μ~U(0.35,0.95)）
    if mu_mode == "dry":
        mu = 0.90
    elif mu_mode == "wet":
        mu = 0.40
    else:  # rand
        mu = rng.uniform(0.35, 0.95)
    gid = m.geom("ground").id
    _set_mu(m, d, mu)

    # 初始位姿：车在 (x0, -1.5)，车头朝 +y（ψ=90°）
    d.qpos[0] = x0
    d.qpos[1] = -1.5
    d.qpos[3] = 0.0                    # yaw=0（局部 +y 已朝世界 +y）；qpos[2]=z 留待重力压实
    mujoco.mj_forward(m, d)
    for _ in range(30):                # 让车体在 z 向沉降压实轮子
        mujoco.mj_step(m, d)

    entry_yaw = 90.0
    psi_err_prev, psi_err_dot_f = 0.0, 0.0
    psi_valid = False
    e_prev, vis_valid = 0.0, False
    hold_fx, hold_fy = 32767.0, 32767.0
    prev_fx, prev_fy = 32767.0, 32767.0
    omega_cmd = 0.0
    done = False
    x_entry = psi_entry = None
    max_abs_x = 0.0
    max_lat_vel = 0.0
    max_slip = 0.0
    max_slip_ang = 0.0
    yaw0 = entry_yaw * DEG2RAD

    n = int(max_t / DT)
    for step in range(n):
        new_vis = ((step + 1) % N_VIS == 1)
        psi = _body_heading(d)
        # 随机摩擦时变扰动（每 0.5s 小抖动）
        if mu_mode == "rand" and (step % 250 == 0):
            mu = float(np.clip(mu + rng.normal(0, 0.03), 0.30, 1.0))
            _set_mu(m, d, mu)
        if new_vis:
            fx, fy = _measure_target(d, psi, rng)
            hold_fx, hold_fy = prev_fx, prev_fy
            prev_fx, prev_fy = fx, fy

        yaw_deg = (psi + rng.normal(0, SIG_YAW_DEG * DEG2RAD)) / DEG2RAD
        psi_err = _norm_rad((entry_yaw - yaw_deg) * DEG2RAD)
        psi_err_dot = (psi_err - psi_err_prev) / DT if psi_valid else 0.0
        alpha = DT / (TAU_D + DT)
        psi_err_dot_f += alpha * (psi_err_dot - psi_err_dot_f)
        psi_err_prev, psi_valid = psi_err, True

        valid_vis = (hold_fx != 32767.0) and (hold_fy != 32767.0)
        if valid_vis and new_vis:
            Dm = np.hypot(hold_fx, hold_fy) / 1000.0
            beta = np.arctan2(hold_fx, hold_fy)
            e_prev = Dm * np.sin(beta - psi_err)
            vis_valid = True
        e = e_prev if vis_valid else 0.0

        omega_des = P * psi_err + D * psi_err_dot_f + KE * e
        omega_des = float(np.clip(omega_des, -W_MAX, W_MAX))
        # slew 限幅
        acc = float(np.clip((omega_des - omega_cmd) / DT, -W_SLEW, W_SLEW))
        omega_cmd = float(np.clip(omega_cmd + acc * DT, -W_MAX, W_MAX))

        # 低速层：前向速度 P + 航向角速度内环 P，扭矩分配（轮胎抓地力自然限制打滑）
        omega_act = d.qvel[3]          # yaw 角速度（rad/s）
        # 车体前向速度
        R = d.xmat[d.body("chassis").id].reshape(3, 3)
        fwd = R[:, 1]
        v_act = d.qvel[0] * fwd[0] + d.qvel[1] * fwd[1]
        F_fwd = KF_FWD * (v - v_act)                     # 前向 P
        T_yaw = KT_YAW * (omega_cmd - omega_act)         # 航向内环 P
        t_l = float(np.clip(F_fwd / 2.0 - T_yaw / L_WHEEL, -8, 8))
        t_r = float(np.clip(F_fwd / 2.0 + T_yaw / L_WHEEL, -8, 8))
        d.ctrl[0] = -t_l                # 负扭矩=前进（标定确定）
        d.ctrl[1] = -t_r
        mujoco.mj_step(m, d)

        if trace and (step % 40 == 0):
            print(f"  t={step*DT:.3f} x={d.qpos[0]*100:.1f}cm y={d.qpos[1]*100:.1f}cm "
                  f"psi={np.degrees(psi):6.1f}° om_cmd={omega_cmd:5.2f} om_act={d.qvel[3]:5.2f} "
                  f"e={e*100:.1f}cm psi_err={np.degrees(psi_err):6.1f}° "
                  f"v_act={v_act:.2f}", flush=True)

        # 状态记录
        cx, cy = d.qpos[0], d.qpos[1]
        max_abs_x = max(max_abs_x, abs(cx))
        # 侧滑速度（垂直于车头方向）
        vx, vy = d.qvel[0], d.qvel[1]
        lat = -vx * np.sin(psi) + vy * np.cos(psi)   # 右向速度分量（车体系 fx 方向）
        max_lat_vel = max(max_lat_vel, abs(lat))
        # 轮胎滑移率：|ω_wheel·R − v_前向| / max(v_前向, 0.1)
        v_act_s = abs(d.qvel[0] * fwd[0] + d.qvel[1] * fwd[1])
        slip = max(abs(d.qvel[4] * R_WHEEL - v_act_s),
                   abs(d.qvel[5] * R_WHEEL - v_act_s)) / max(v_act_s, 0.1)
        max_slip = max(max_slip, slip)
        # 侧偏角：航向与速度方向夹角（轮胎打滑时增大）
        speed = np.hypot(vx, vy)
        if speed > 0.1:
            slip_ang = abs(_norm_rad(np.arctan2(vy, vx) - psi))
            max_slip_ang = max(max_slip_ang, slip_ang)
        D_now = np.hypot(cx, cy)
        if (not done) and (D_now < PVC):
            done = True
            x_entry = cx
            psi_entry = psi_err / DEG2RAD
        if D_now > 3.0 and not done:
            done = True                 # 发散剪枝
            x_entry = None
            psi_entry = None
            break
        if done:
            break

    reached = x_entry is not None
    ok = reached and (abs(x_entry) <= X_TOL) and (abs(psi_entry) <= PSI_TOL_DEG)
    return {
        "x0": x0, "v": v, "mu_mode": mu_mode, "mu": mu,
        "reached": reached, "ok": ok,
        "x_entry_cm": x_entry * 100.0 if reached else None,
        "psi_entry": psi_entry if reached else None,
        "max_abs_x_cm": max_abs_x * 100.0,
        "max_lat_vel": max_lat_vel,
        "max_slip": max_slip,
        "max_slip_ang": max_slip_ang,
    }


def calibrate():
    """标定：接触诊断 + 直行/转向方向。"""
    import mujoco
    m = mujoco.MjModel.from_xml_string(XML)
    d = mujoco.MjData(m)
    d.qpos[1] = -1.5
    mujoco.mj_forward(m, d)
    for _ in range(50):                # 沉降压实
        mujoco.mj_step(m, d)
    print("沉降后轮子 z:", d.geom_xpos[m.geom('wl_g').id][2], d.geom_xpos[m.geom('wr_g').id][2])
    print("接触数:", d.ncon)
    for i in range(d.ncon):
        c = d.contact[i]
        g1, g2 = m.geom(c.geom1).name, m.geom(c.geom2).name
        f = np.zeros(6)
        mujoco.mj_contactForce(m, d, i, f)
        print(f"  contact {i}: {g1} <-> {g2}  法向={f[0]:.2f}N  切向=({f[1]:.2f},{f[2]:.2f})")
    # 试：两轮同向扭矩 / 差动扭矩
    for trial, (t0, t1) in enumerate([(-2, -2), (2, 2), (-2, 0), (0, -2)]):
        dd = mujoco.MjData(m)
        dd.qpos[1] = -1.5
        mujoco.mj_forward(m, dd)
        for _ in range(50):
            mujoco.mj_step(m, dd)
        dd.ctrl[0] = t0
        dd.ctrl[1] = t1
        for _ in range(250):
            mujoco.mj_step(m, dd)
        psi = _body_heading(dd)
        print(f"trial c=({t0},{t1}): pos=({dd.qpos[0]:.3f},{dd.qpos[1]:.3f}) "
              f"heading={np.degrees(psi):.1f}° yaw_vel={dd.qvel[3]:.3f} 轮速={dd.qvel[4:6]}")


def skid_test():
    """直接侧滑标定：v=2 猛打方向 2.2rad/s，对比干/湿侧偏角。"""
    import mujoco
    for mm, mu in [("dry", 0.90), ("wet", 0.40), ("icy", 0.25)]:
        m = mujoco.MjModel.from_xml_string(XML)
        d = mujoco.MjData(m)
        d.qpos[1] = -1.5
        m.geom_friction[m.geom("ground").id, 0] = mu
        mujoco.mj_forward(m, d)
        max_ang, max_lat = 0.0, 0.0
        v_act = 0.0
        for step in range(1500):   # 3s
            R = d.xmat[d.body("chassis").id].reshape(3, 3)
            fwd = R[:, 1]
            v_act = d.qvel[0] * fwd[0] + d.qvel[1] * fwd[1]
            F_fwd = 6.0 * (2.0 - v_act)              # 提速到 2 m/s
            T_yaw = 1.5 * (2.2 - d.qvel[3]) if step > 400 else 0.0  # 0.8s 后猛打方向
            d.ctrl[0] = -float(np.clip(F_fwd / 2 - T_yaw / L_WHEEL, -8, 8))
            d.ctrl[1] = -float(np.clip(F_fwd / 2 + T_yaw / L_WHEEL, -8, 8))
            mujoco.mj_step(m, d)
            vx, vy = d.qvel[0], d.qvel[1]
            sp = np.hypot(vx, vy)
            psi = _body_heading(d)
            if sp > 0.1:
                ang = abs(_norm_rad(np.arctan2(vy, vx) - psi))
                max_ang = max(max_ang, ang)
            max_lat = max(max_lat, abs(-vx * np.sin(psi) + vy * np.cos(psi)))
        print(f"{mm} μ={mu}: v_final={v_act:.2f}m/s 最大侧偏角={np.degrees(max_ang):5.1f}° "
              f"最大横向速度={max_lat:.2f}m/s", flush=True)



def friction_debug():
    """调试：μ 极端值下猛打方向，检查接触切向力是否被 μ·N 限制。"""
    import mujoco
    for mu in (0.1, 0.5, 2.0):
        m = mujoco.MjModel.from_xml_string(XML)
        d = mujoco.MjData(m)
        _set_mu(m, d, mu)
        d.qpos[1] = -1.5
        mujoco.mj_forward(m, d)
        for _ in range(30):
            mujoco.mj_step(m, d)
        print(f"μ={mu}  ground.friction[0]={m.geom_friction[m.geom('ground').id,0]:.2f} "
              f"wheel.friction[0]={m.geom_friction[m.geom('wl_g').id,0]:.2f}")
        max_tan, max_norm, max_lat = 0, 0, 0
        for step in range(300):
            d.ctrl[0] = 3.0
            d.ctrl[1] = -3.0
            mujoco.mj_step(m, d)
            for i in range(d.ncon):
                f = np.zeros(6)
                mujoco.mj_contactForce(m, d, i, f)
                max_tan = max(max_tan, np.hypot(f[1], f[2]))
                max_norm = max(max_norm, f[0])
            psi = _body_heading(d)
            vx, vy = d.qvel[0], d.qvel[1]
            max_lat = max(max_lat, abs(-vx * np.sin(psi) + vy * np.cos(psi)))
        print(f"  结果: 最大法向={max_norm:.1f}N 最大切向={max_tan:.1f}N "
              f"(μ·N上限={mu*max_norm:.1f}N) 最大横向速度={max_lat:.2f}m/s", flush=True)



def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else "calibrate"
    if mode == "calibrate":
        calibrate()
        return
    if mode == "friction":
        friction_debug()
        return
    if mode == "skid":
        skid_test()
        return
    if mode == "test":
        for (params, x0, v, mm) in [
            ((15.0, 9.5, 0.25), 0.40, 1.0, "dry"),
            ((15.0, 9.5, 0.25), 0.40, 1.0, "wet"),
            ((15.0, 9.5, 0.25), 0.70, 1.0, "dry"),
            ((15.0, 9.5, 0.25), 0.70, 1.0, "wet"),
            ((15.0, 9.5, 0.25), 0.40, 2.0, "dry"),
            ((15.0, 9.5, 0.25), 0.40, 2.0, "wet"),
            ((50.0, 50.0, 3.5), 0.20, 1.0, "dry"),
            ((50.0, 50.0, 3.5), 0.20, 1.0, "wet"),
            ((50.0, 50.0, 3.5), 0.40, 1.0, "dry"),
            ((50.0, 50.0, 3.5), 0.40, 1.0, "wet"),
        ]:
            r = run_episode(params, x0, v, mm, 1)
            print(f"params={params} x0={x0} v={v} {mm}: "
                  f"reached={r['reached']} ok={r['ok']} x_entry={r['x_entry_cm']}cm "
                  f"psi={r['psi_entry']}° max_lat_vel={r['max_lat_vel']:.2f}m/s "
                  f"max|x|={r['max_abs_x_cm']:.1f}cm mu={r['mu']:.2f}", flush=True)
        return
    if mode == "sweep":
        run_sweep()


def run_sweep():
    # 候选参数：GPU 深搜 Top + 对照
    CANDIDATES = [
        ("opt-15/9.5/0.25", (15.0, 9.5, 0.25)),
        ("opt-14/9/0.2",    (14.0, 9.0, 0.20)),
        ("opt-12/8/0",      (12.0, 8.0, 0.00)),
        ("opt-11/7/0",      (11.0, 7.0, 0.00)),
        ("opt-10/7/0.4",    (10.0, 7.0, 0.40)),
        ("lqr-120/32",      None),
        ("cur-50/50/3.5",   (50.0, 50.0, 3.5)),
        ("safe-11/7/1",     (11.0, 7.0, 1.0)),
    ]
    speeds = [1.0, 1.5, 2.0, 2.5]
    mu_modes = ["dry", "wet", "rand"]
    x0s = [0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.60, 0.70, 0.80, 1.00]
    NSEED = 10

    tasks = []
    for name, params in CANDIDATES:
        for v in speeds:
            # LQR：k1=√Qy, k2(v)=√(2·max(v,0.3)·k1+Qψ), D=0
            if params is None:
                k1 = np.sqrt(120.0)
                k2 = np.sqrt(2.0 * max(v, 0.3) * k1 + 32.0)
                pp = (k1, k2, 0.0)
            else:
                pp = params
            for mm in mu_modes:
                for x0 in x0s:
                    for s in range(NSEED):
                        tasks.append((name, pp, x0, v, mm, s))

    t0 = time.time()
    results = {}
    with ProcessPoolExecutor(max_workers=96) as ex:
        futs = [ex.submit(run_episode, params, x0, v, mm, seed, trace=False)
                for (name, params, x0, v, mm, seed) in tasks]
        for (name, params, x0, v, mm, seed), f in zip(tasks, futs):
            r = f.result()
            key = (name, v, mm)
            results.setdefault(key, []).append(r)
    print(f"总耗时 {time.time()-t0:.0f}s，任务数 {len(tasks)}", flush=True)

    # 汇总：成功概率 ≥ 80% 的最大 x0
    def p80(rs):
        from collections import Counter
        cnt = Counter(r["x0"] for r in rs if r["ok"])
        n = NSEED
        rob = [x0 for x0, c in cnt.items() if c >= 0.8 * n]
        return max(rob) if rob else 0.0

    summary = {}
    slip_ang = {}
    for (name, v, mm), rs in results.items():
        summary[(name, v, mm)] = p80(rs)
        slip_ang[(v, mm)] = max(slip_ang.get((v, mm), 0.0),
                                max((r.get("max_slip_ang", 0.0) for r in rs), default=0.0))

    print("\n===== 最大可修 x0（成功概率≥80%，10 种子）=====")
    for name, _ in CANDIDATES:
        for v in speeds:
            line = []
            for mm in mu_modes:
                maxx = summary.get((name, v, mm), 0.0)
                line.append(f"{mm}:{maxx*100:.0f}cm")
            print(f"{name:<14} {v:>4.1f} | " + "  ".join(line))

    print("\n===== 峰值侧偏角（判断抓地是否饱和，°）=====")
    for v in speeds:
        line = [f"v={v:.1f}"]
        for mm in mu_modes:
            line.append(f"{mm}:{np.degrees(slip_ang.get((v,mm),0.0)):.1f}°")
        print("  " + "  ".join(line))

    print("\n===== 干/湿差异（湿−干，负=湿更差）=====")
    for name, _ in CANDIDATES:
        diffs = []
        for v in speeds:
            d = summary.get((name, v, "dry"), 0.0)
            w = summary.get((name, v, "wet"), 0.0)
            diffs.append(f"v={v:.1f}:{(w-d)*100:+.0f}cm")
        print(f"  {name:<14} " + "  ".join(diffs))

    # 存结果
    import json
    out = {f"{k[0]}|{k[1]}|{k[2]}": v for k, v in summary.items()}
    with open("/root/mujoco_entry_result.json", "w") as f:
        json.dump(out, f, indent=1)
    print("\n已保存 /root/mujoco_entry_result.json")


if __name__ == "__main__":
    main()
