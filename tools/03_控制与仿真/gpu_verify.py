# -*- coding: utf-8 -*-
"""GPU 仿真定点校验：与本地 CPU 仿真已知结果逐点对比。"""
import torch, numpy as np
from gpu_entry_search import run_batch, DEV, X_TOL, PSI_TOL_DEG, PVC, DEG2RAD

# (K_E, P, D, x0, v, 本地期望 x_cm, 本地期望 psi_deg, 本地是否通过)
CASES = [
    (15.0, 9.5, 0.25, 0.70, 1.0, 6.2, -18.7, True),
    (15.0, 9.5, 0.25, 0.40, 1.5, -4.1, -33.1, False),
    (11.0, 7.0, 0.00, 0.50, 1.0, 9.1, -18.3, True),
    (50.0, 50.0, 3.5, 0.20, 1.0, -19.6, 17.2, False),
    (50.0, 50.0, 3.5, 0.10, 1.0, -8.0, 13.3, True),
    (50.0, 50.0, 3.5, 0.30, 1.0, -25.9, 12.4, False),
    (12.0, 8.0, 0.00, 0.40, 1.0, None, None, True),
    (14.0, 9.0, 0.20, 0.70, 1.0, None, None, True),
]

print(f"{'K_E':>5} {'P':>5} {'D':>5} {'x0':>5} {'v':>4} |  {'x_cm':>7} {'psi':>7} {'ok':>4} | 本地期望")
for ke, p, d, x0, v, ex_x, ex_ps, ex_ok in CASES:
    KE = torch.tensor([ke], device=DEV); P = torch.tensor([p], device=DEV)
    D = torch.tensor([d], device=DEV)
    xe, pse, re = run_batch(KE, P, D, torch.tensor([x0], device=DEV),
                            torch.tensor([v], device=DEV))
    xcm = float(xe[0]) * 100 if bool(re[0]) else float('nan')
    pdeg = float(pse[0]) if bool(re[0]) else float('nan')
    ok = bool(re[0]) and abs(float(xe[0])) <= X_TOL and abs(float(pse[0])) <= PSI_TOL_DEG
    exp = f"x={ex_x} ψ={ex_ps} ok={ex_ok}" if ex_x is not None else "?"
    print(f"{ke:5.1f} {p:5.1f} {d:5.2f} {x0:5.2f} {v:4.1f} |  {xcm:7.1f} {pdeg:7.1f} {str(ok):>4} | {exp}")
