# -*- coding: utf-8 -*-
"""
简单版回放验证 (2026-08-09 用户定调: 复杂滤波过度设计, 录像不具唯一性):
  主判据: exit_y > 15 (退出线进入近场带)
  叠加: 衰减累计 ≥3 帧确认 (+1/-1, 轻微递增感, 容忍 has_top 抖动)
数据: oscilloscope_data_20260808_230901.csv (1核 arbiter 逐帧)
"""
import csv

PATH = r"d:\WORKS\2026LunTui\project\tools\05_通用数据处理工具\视频网页上位机\output_realtime\oscilloscope_data_20260808_230901.csv"
YKEY = '(arb->top_a_x1000*47)/1000.0f+(arb->top_b_x100)/100.0f'

EXIT_Y_TH = 15.0      # 退出线下移带 (图像行)
HOLD = 3              # 衰减累计 ≥3 帧确认

with open(PATH, newline="", encoding="utf-8-sig") as f:
    rows = list(csv.DictReader(f))
n = len(rows)
t0 = float(rows[0]["time"]); t1 = float(rows[-1]["time"]); dt = (t1 - t0) / n

def fv(r, k):
    try: return float(r[k])
    except Exception: return 0.0
def iv(r, k):
    return 1 if fv(r, k) > 0.5 else 0

high = 0
prev_fire = 0
events = []
old_y_lt10 = 0

for idx, r in enumerate(rows):
    has = iv(r, 'arb->has_top')
    y = fv(r, YKEY)
    valid = (has == 1) and (0.0 <= y <= 59.0)

    # 主判据: y > 15, 衰减累计 (+1/-1)
    if valid and y > EXIT_Y_TH:
        high = min(high + 1, 99)
    else:
        high = max(high - 1, 0)

    fire = high >= HOLD
    if fire and not prev_fire:
        events.append((t0 + idx * dt, y))
    prev_fire = fire

    if valid and y < 10.0:
        old_y_lt10 += 1

print("=" * 60)
print("简单版: exit_y > %.0f, 衰减累计 >=%d 帧" % (EXIT_Y_TH, HOLD))
print("=" * 60)
print("[简单版] 脱出(FIRE) 事件 (t, 触发时y):")
if not events:
    print("  !! 无触发")
for t, y in events:
    print("  t=%7.1fs  y=%.1f" % (t - t0, y))
print()
print("[旧逻辑 y<10] 满足帧总数:", old_y_lt10)
