# -*- coding: utf-8 -*-
"""查看 t=48~62s 的 has_top/exit_y 原始数据。"""
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

for idx, r in enumerate(rows):
    t = t0 + idx * dt
    if 48 <= (t - t0) <= 62:
        has = iv(r, 'arb->has_top')
        y = fv(r, YKEY)
        if has:
            print('t=%6.2fs has=1 y=%6.2f gate=%.0f valid=%.0f' % (t - t0, y, fv(r, 'arb->gate'), fv(r, 'arb->valid')))
