# -*- coding: utf-8 -*-
"""
GPU 全向量化进入段 PD 参数寻优（平行偏差专用）
- 场景：车平行于道路（psi0=90°），初始横向偏差 x0，目标在 (0,0) 前方 1.5m。
- 验收：上桥瞬间（D<0.4m）|x|<=11.5cm 且 |psi_err|<=20°。
- 全部 (K_E,P,D) × 速度 × x0 轨迹并行在 CUDA 上推进。
- 用法：python gpu_entry_search.py <K_E_step> <P_step> <D_step> [x0_grid模式]
"""
import torch, numpy as np, time, sys, json

torch.manual_seed(1)
DEV = "cuda" if torch.cuda.is_available() else "cpu"
print(f"device={DEV}  torch={torch.__version__}", flush=True)

# ---------------- 常量（与固件/本地仿真一致） ----------------
DT = 0.002
N_VIS = int(0.030 / DT)          # 15 步 = 30ms
W_MAX = 2.2
W_SLEW = 9.0
TAU_ACT = 0.03
TAU_D = 0.05                     # PD_D_TAU_S
DETECT = 1.5
PVC = 0.4
X_TOL = 0.115
PSI_TOL_DEG = 20.0
DEG2RAD = 0.0174532925
SIG_FX_MM, SIG_FY_MM, SIG_YAW_DEG = 10.0, 15.0, 0.4
INV = 32767.0
MAX_STEPS = int(4.0 / DT)        # 2000 步 = 4s


def norm_deg_rad(a):
    """角度(rad)归一化到 [-pi, pi]"""
    return torch.remainder(a + np.pi, 2 * np.pi) - np.pi


def run_batch(KE, P, D, x0, v, max_steps=MAX_STEPS):
    """批量仿真。KE/P/D/x0/v 形状相同 [B]。返回 (x_entry, psi_err_deg_entry, reached)"""
    B = KE.numel()
    x = x0.clone()                       # 初始横向偏差
    y = -1.5 * torch.ones(B, device=DEV)
    psi = (90.0 * DEG2RAD) * torch.ones(B, device=DEV)
    omega = torch.zeros(B, device=DEV)

    psi_err_prev = torch.zeros(B, device=DEV)
    psi_err_dot_f = torch.zeros(B, device=DEV)
    psi_valid = torch.zeros(B, dtype=torch.bool, device=DEV)
    e_prev = torch.zeros(B, device=DEV)
    vis_valid = torch.zeros(B, dtype=torch.bool, device=DEV)
    hold_fx = torch.full((B,), INV, device=DEV)
    hold_fy = torch.full((B,), INV, device=DEV)
    prev_fx = torch.full((B,), INV, device=DEV)
    prev_fy = torch.full((B,), INV, device=DEV)

    done = torch.zeros(B, dtype=torch.bool, device=DEV)
    x_entry = torch.full((B,), torch.nan, device=DEV)
    psi_entry = torch.full((B,), torch.nan, device=DEV)

    for step in range(max_steps):
        new_vis = ((step + 1) % N_VIS == 1)   # 与本地仿真 tick%15==1 对齐

        # ---------- 视觉测量（30ms 一次，1 帧延迟 + 噪声） ----------
        if new_vis:
            dx, dy = -x, -y
            fx_m = -dx * torch.sin(psi) + dy * torch.cos(psi)   # 米
            fy_m = dx * torch.cos(psi) + dy * torch.sin(psi)
            Dm = torch.sqrt(fx_m * fx_m + fy_m * fy_m)
            in_range = Dm <= DETECT
            fx_mm = fx_m * 1000.0 + torch.randn(B, device=DEV) * SIG_FX_MM
            fy_mm = fy_m * 1000.0 + torch.randn(B, device=DEV) * SIG_FY_MM
            fx_mm = torch.where(in_range, fx_mm, torch.full((B,), INV, device=DEV))
            fy_mm = torch.where(in_range, fy_mm, torch.full((B,), INV, device=DEV))
            # 1 帧延迟：控制器本轮看到上一帧
            hold_fx = prev_fx.clone()
            hold_fy = prev_fy.clone()
            prev_fx = fx_mm
            prev_fy = fy_mm

        # ---------- 控制器 ----------
        yaw_deg = (psi + torch.randn(B, device=DEV) * SIG_YAW_DEG * DEG2RAD) / DEG2RAD
        psi_err = norm_deg_rad((90.0 - yaw_deg) * DEG2RAD)
        psi_err_dot = torch.where(psi_valid, (psi_err - psi_err_prev) / DT, torch.zeros_like(psi_err))
        alpha = DT / (TAU_D + DT)
        psi_err_dot_f = psi_err_dot_f + alpha * (psi_err_dot - psi_err_dot_f)
        psi_err_prev = psi_err
        psi_valid = torch.ones(B, dtype=torch.bool, device=DEV)

        valid_vis = ~((hold_fx == INV) | (hold_fy == INV))
        D_m = torch.sqrt(hold_fx * hold_fx + hold_fy * hold_fy) / 1000.0
        beta = torch.atan2(hold_fx, hold_fy)
        e = D_m * torch.sin(beta - psi_err)
        e = torch.where(valid_vis & new_vis, e,
                        torch.where(vis_valid, e_prev, torch.zeros_like(e)))
        e_prev = e
        vis_valid = vis_valid | valid_vis

        omega_cmd = P * psi_err + D * psi_err_dot_f + KE * e
        omega_cmd = torch.clamp(omega_cmd, -W_MAX, W_MAX)

        # ---------- 执行器（slew + 幅值） + 积分 ----------
        acc = torch.clamp((omega_cmd - omega) / TAU_ACT, -W_SLEW, W_SLEW)
        omega = torch.clamp(omega + acc * DT, -W_MAX, W_MAX)
        x = x + v * torch.cos(psi) * DT
        y = y + v * torch.sin(psi) * DT
        psi = psi + omega * DT

        # ---------- 上桥点记录 ----------
        D_now = torch.sqrt(x * x + y * y)
        reached = (~done) & (D_now < PVC)
        x_entry = torch.where(reached, x, x_entry)
        psi_entry = torch.where(reached, norm_deg_rad((90.0 - yaw_deg) * DEG2RAD), psi_entry)
        done = done | reached
        # 发散剪枝：远离目标 → 判失败结束
        diverged = (~done) & (D_now > 3.0)
        done = done | diverged
        if bool(done.all()):
            break

    reached = ~torch.isnan(x_entry)
    psi_deg = psi_entry / DEG2RAD
    return x_entry, psi_deg, reached


def evaluate_grid(KEs, Ps, Ds, x0s, speeds, max_steps=MAX_STEPS):
    """评估所有组合。返回 cap[v] 形状 [nK,nP,nD] 的最大通过 x0。"""
    nK, nP, nD = len(KEs), len(Ps), len(Ds)
    nX, nS = len(x0s), len(speeds)
    # 参数张量
    KE = torch.tensor(KEs, device=DEV)[:, None, None, None, None].expand(nK, nP, nD, nS, nX).reshape(-1)
    P = torch.tensor(Ps, device=DEV)[None, :, None, None, None].expand(nK, nP, nD, nS, nX).reshape(-1)
    D = torch.tensor(Ds, device=DEV)[None, None, :, None, None].expand(nK, nP, nD, nS, nX).reshape(-1)
    x0 = torch.tensor(x0s, device=DEV)[None, None, None, None, :].expand(nK, nP, nD, nS, nX).reshape(-1)
    v = torch.tensor(speeds, device=DEV)[None, None, None, :, None].expand(nK, nP, nD, nS, nX).reshape(-1)
    B = nK * nP * nD * nS * nX
    print(f"批量 B={B:,}  (nK={nK}, nP={nP}, nD={nD}, nS={nS}, nX={nX})", flush=True)
    t0 = time.time()
    x_entry, psi_deg, reached = run_batch(KE, P, D, x0, v, max_steps)
    dt = time.time() - t0
    print(f"仿真耗时 {dt:.1f}s", flush=True)

    ok = reached & (torch.abs(x_entry) <= X_TOL) & (torch.abs(psi_deg) <= PSI_TOL_DEG)
    ok = ok.reshape(nK, nP, nD, nS, nX)
    idx = torch.full((nK, nP, nD, nS), -1, dtype=torch.long, device=DEV)
    # 每个 (k,p,d,s) 下通过的 x0 索引最大值
    arange = torch.arange(nX, device=DEV)
    valid_idx = torch.where(ok, arange[None, None, None, None, :], torch.full_like(arange, -1))
    idx = valid_idx.max(dim=4).values                     # 最大通过 x0 索引
    has = idx >= 0
    idx_cl = idx.clamp(min=0)
    x0t = torch.tensor(x0s, device=DEV)
    cap = torch.where(has, x0t[idx_cl], torch.zeros_like(x0t[idx_cl].float()))
    return cap, ok


def main():
    ke_step = float(sys.argv[1]) if len(sys.argv) > 1 else 1.0
    p_step = float(sys.argv[2]) if len(sys.argv) > 2 else 0.5
    d_step = float(sys.argv[3]) if len(sys.argv) > 3 else 0.1
    x0_mode = int(sys.argv[4]) if len(sys.argv) > 4 else 1   # 1=细 x0 网格, 0=粗
    if x0_mode:
        x0s = [0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.00, 1.10, 1.20]
    else:
        x0s = [0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 1.00, 1.10, 1.20]
    speeds = [1.0, 1.5, 2.0]

    KEs = np.arange(4.0, 46.01, ke_step)
    Ps = np.arange(3.0, 17.01, p_step)
    Ds = np.arange(0.0, 2.01, d_step)
    print(f"K_E: {len(KEs)}个({KEs[0]}..{KEs[-1]}, step {ke_step})  "
          f"P: {len(Ps)}个  D: {len(Ds)}个", flush=True)

    cap, ok = evaluate_grid(KEs, Ps, Ds, x0s, speeds)

    # 汇总：每参数总能力 = 三速度最大可修 x0 之和
    nK, nP, nD = len(KEs), len(Ps), len(Ds)
    total = cap.sum(dim=3)                    # [nK,nP,nD]
    flat = total.reshape(-1)
    vals, idxs = torch.topk(flat, 20)
    print("\n===== 深层寻优 Top 20（平行偏差专用）=====", flush=True)
    rows = []
    for score, fi in zip(vals.tolist(), idxs.tolist()):
        k = fi // (nP * nD); r = fi % (nP * nD); p = r // nD; d = r % nD
        c = cap[k, p, d]                       # [nS]
        rows.append((score, KEs[k], Ps[p], Ds[d], c.tolist()))
        print(f"  K_E={KEs[k]:5.2f} P={Ps[p]:5.2f} D={Ds[d]:4.2f} => "
              f"@1.0={c[0]*100:4.0f} @1.5={c[1]*100:4.0f} @2.0={c[2]*100:4.0f}cm "
              f"总分={score*100:.0f}cm", flush=True)

    # 对照：LQR 与现役 PD50
    print("\n===== 对照（单独跑）=====", flush=True)
    def ref_cap(cfg):
        caps = []
        for v in speeds:
            xes = []
            for x0 in x0s:
                B = 1
                KE = torch.tensor([cfg[0]], device=DEV); P = torch.tensor([cfg[1]], device=DEV)
                D = torch.tensor([cfg[2]], device=DEV)
                xe, ps, re = run_batch(KE, P, D, torch.tensor([x0], device=DEV),
                                       torch.tensor([v], device=DEV))
                okk = bool(re[0]) and abs(float(xe[0])) <= X_TOL and abs(float(ps[0])) <= PSI_TOL_DEG
                if okk:
                    xes.append(x0)
            caps.append(xes[-1] if xes else 0.0)
        return caps
    for name, cfg in [("现役PD 50/50/3.5", (50.0, 50.0, 3.5)),
                      ("LQR 120/32     ", (None, None, None))]:
        if cfg[0] is None:
            # LQR 单独跑
            caps = []
            for v in speeds:
                xes = []
                for x0 in x0s:
                    B = 1
                    ke = torch.tensor([np.sqrt(120.0)], device=DEV)
                    k2 = torch.sqrt(2.0 * torch.clamp(torch.tensor([v], device=DEV), 0.3, None) * np.sqrt(120.0) + 32.0)
                    # LQR: omega = k1*e + k2*psi_err —— 用 K_E=k1, P=k2, D=0 近似
                    xe, ps, re = run_batch(ke, k2, torch.tensor([0.0], device=DEV),
                                           torch.tensor([x0], device=DEV), torch.tensor([v], device=DEV))
                    okk = bool(re[0]) and abs(float(xe[0])) <= X_TOL and abs(float(ps[0])) <= PSI_TOL_DEG
                    if okk:
                        xes.append(x0)
                caps.append(xes[-1] if xes else 0.0)
        else:
            caps = ref_cap(cfg)
        print(f"  {name}: @1.0={caps[0]*100:.0f} @1.5={caps[1]*100:.0f} @2.0={caps[2]*100:.0f}cm", flush=True)

    # 保存
    out = {"ke_step": ke_step, "p_step": p_step, "d_step": d_step, "x0s": x0s,
           "speeds": speeds, "KEs": KEs.tolist(), "Ps": Ps.tolist(), "Ds": Ds.tolist(),
           "top": [{"KE": r[1], "P": r[2], "D": r[3], "cap": r[4]} for r in rows]}
    with open("/root/entry_search_result.json", "w") as f:
        json.dump(out, f, indent=1)
    print("\n已保存 /root/entry_search_result.json", flush=True)


if __name__ == "__main__":
    main()
