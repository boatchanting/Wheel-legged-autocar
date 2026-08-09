# -*- coding: utf-8 -*-
"""分析两份单边桥示波器日志：has_top 区间 + exit_y 时序轨迹（修正版）。"""
import csv
from collections import Counter

FILES = [
    r"d:\WORKS\2026LunTui\project\tools\05_通用数据处理工具\视频网页上位机\output_realtime\oscilloscope_data_20260808_230613.csv",
    r"d:\WORKS\2026LunTui\project\tools\05_通用数据处理工具\视频网页上位机\output_realtime\oscilloscope_data_20260808_230901.csv",
]
YKEY = "(arb->top_a_x1000*47)/1000.0f+(arb->top_b_x100)/100.0f"

for path in FILES:
    print("=" * 78)
    print("FILE:", path.split("\\")[-1])
    with open(path, newline="", encoding="utf-8-sig") as f:
        rows = list(csv.DictReader(f))
    n = len(rows)
    t0 = float(rows[0]["time"]); t1 = float(rows[-1]["time"])
    print(f"rows={n}  duration={t1-t0:.1f}s")

    def fv(r, k):
        try: return float(r[k])
        except Exception: return 0.0
    def iv(r, k):
        return 1 if fv(r, k) > 0.5 else 0

    gate = Counter(iv(r, "arb->gate") for r in rows)
    has = Counter(iv(r, "arb->has_top") for r in rows)
    valid = Counter(iv(r, "arb->valid") for r in rows)
    print(f"gate:{dict(gate)}  has_top:{dict(has)}  valid:{dict(valid)}")

    # has_top 连续区间
    segs, cur = [], None
    for idx, r in enumerate(rows):
        h = iv(r, "arb->has_top")
        if h and cur is None: cur = [idx, idx]
        elif h and cur is not None: cur[1] = idx
        elif not h and cur is not None:
            segs.append(cur); cur = None
    if cur is not None: segs.append(cur)
    print(f"has_top=1 区间数: {len(segs)}")

    dt = (t1 - t0) / n
    for s in segs[:30]:
        ys = [fv(rows[i], YKEY) for i in range(s[0], s[1] + 1)]
        valid_ys = [y for y in ys if y > 0]
        if valid_ys:
            trend = "升" if valid_ys[-1] > valid_ys[0] + 0.5 else ("降" if valid_ys[-1] < valid_ys[0] - 0.5 else "平")
            print(f"  [{s[0]:>5}..{s[1]:>5}] len={s[1]-s[0]+1:>4} "
                  f"t={t0+s[0]*dt:7.1f}..{t0+s[1]*dt:7.1f}s "
                  f"y首={valid_ys[0]:6.2f} 尾={valid_ys[-1]:6.2f} "
                  f"min={min(valid_ys):6.2f} max={max(valid_ys):6.2f} 趋势{trend}")
        else:
            print(f"  [{s[0]:>5}..{s[1]:>5}] len={s[1]-s[0]+1:>4} (无有效 y)")
    print()
