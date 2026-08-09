# -*- coding: utf-8 -*-
"""查看 t=24~34s 与 t=30~33s 的 has_top/exit_y 原始数据。"""
import csv

PATH = r"d:\WORKS\2026LunTui\project\tools\05_通用数据处理工具\视频网页上位机\output_realtime\oscilloscope_data_20260808_230901.csv"
YKEY = '(arb->top_a_x1000*47)/1000.0f+(arb->top_b_x100)/100.0f'

with open(PATH, newline="", encoding="utf-8-sig") as f:
    rows = list(csv.DictReader(f))
n = len(rows)
t0 = float(rows[0]['time']); t1 = float(rows[-1]['time']); dt = (t1 - t0) / n

def fv(r, k):
    try:
        return float(r[k])
    except Exception:
        return 0.0

def iv(r, k):
    return 1 if fv(r, k) > 0.5 else 0

# 汇总 25~33s 的 (y 轨迹分段)
for lo, hi in [(25.0, 28.5), (30.0, 32.5)]:
    segs = []
    cur = None
    for idx, r in enumerate(rows):
        t = t0 + idx * dt
        if not (lo <= (t - t0) <= hi):
            continue
        has = iv(r, 'arb->has_top')
        y = fv(r, YKEY)
        if has and 0 <= y <= 59:
            if cur is None:
                cur = [t - t0, t - t0, y, y]
            else:
                cur[1] = t - t0
                cur[2] = min(cur[2], y)
                cur[3] = max(cur[3], y)
        else:
            if cur is not None:
                segs.append(cur)
                cur = None
    if cur is not None:
        segs.append(cur)
    print('t=[%.1f,%.1f] has_top 有效段:' % (lo, hi))
    for s in segs[:12]:
        print('  t=%.2f..%.2fs  y: %.1f..%.1f' % (s[0], s[1], s[2], s[3]))
    if not segs:
        print('  无')
