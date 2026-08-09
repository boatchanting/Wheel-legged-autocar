#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
verify_pc_rule.py —— PC 规则交叉验证 + 拒绝原因分布 (提示词 §4a/§4c)
复刻 trials/pc_tools/review_bridge_gui.py 的 _det_verdict 级联 (权威实现):
  用 serial 里的 R/G/B 线参数 (a,b) + 图像 bin 重算 v=,
  与参考 serial_valid_all.txt 的 v= 对比, 目标 0 差异;
  同时统计拒绝原因分布, 与提示词 §4c 记录核对。
用法: python verify_pc_rule.py
"""
import io
import math
import os
import re
from collections import Counter

TRIALS = r'D:\WORKS\2026LunTui\trials\bridge\pc_tools'
BINS = os.path.join(TRIALS, 'out_v13c_bins')
REF = os.path.join(TRIALS, 'serial_valid_all.txt')
VIDEOS = ['20260807_单边桥_01_室外', '20260807_单边桥_02_室外', '20260807_单边桥_03_室外',
          '20260807_单边桥_04_室外', '20260807_单边桥_05_室外', '20260807_单边桥_06_室外',
          '20260807_单边桥_07_室外']

MXR_THR = 0.35
WMIN_THR = 15.0
STRIP_W_THR = 0.5
ANGLE_MAX = 90.0
YC_MAX = 30.0

LINE_RE = re.compile(
    r'bridge (\S+): \d+ us mode=(\d+) nl=(\d+) nr=(\d+) '
    r'R(\d) G(\d) B(\d) T(\d) '
    r'ra=(-?\d+) rb=(-?\d+) ga=(-?\d+) gb=(-?\d+) '
    r'ba=(-?\d+) bb=(-?\d+) ta=(-?\d+) tb=(-?\d+) tn=(\d+) trms=(-?\d+)(?: v=(\d+))?')
KEYS = ['mode', 'nl', 'nr', 'R', 'G', 'B', 'T',
        'ra', 'rb', 'ga', 'gb', 'ba', 'bb', 'ta', 'tb', 'tn', 'trms', 'v']


def load_serial(path):
    out = {}
    for ln in io.open(path, encoding='utf-8-sig', errors='replace'):
        m = LINE_RE.match(ln.strip())
        if m:
            d = {k: int(m.group(i + 2)) for i, k in enumerate(KEYS[:-1])}
            d['v'] = int(m.group(19)) if m.group(19) is not None else 0
            out[m.group(1)] = d
    return out


def line_maxr(im, a, b, side):
    H, W = len(im), len(im[0])
    seg = H // 4
    maxr = 0.0
    for s in range(4):
        y0, y1 = s * seg, (s + 1) * seg
        ls2 = rs2 = 0.0
        lc = rc = 0
        for y in range(y0, y1):
            x = int(round(a * y + b))
            for k in range(6):
                xl, xr = x - 11 + k, x + 6 + k
                if 0 <= xl < W:
                    v = im[y][xl] / 255.0
                    ls2 += v * v
                    lc += 1
                if 0 <= xr < W:
                    v = im[y][xr] / 255.0
                    rs2 += v * v
                    rc += 1
        if lc >= 6 and rc >= 6:
            i2 = rs2 / rc if side < 0 else ls2 / lc
            o2 = ls2 / lc if side < 0 else rs2 / rc
            r = abs(i2 - o2) / (i2 + o2 + 1e-3)
            maxr = max(maxr, r)
    return maxr


def interline_maxwhite(im, a1, b1, a2, b2, nstrip=12, thr=200):
    H, W = len(im), len(im[0])
    mx = 0.0
    for s in range(nstrip):
        y0 = int(s * H / nstrip)
        y1 = int((s + 1) * H / nstrip)
        br = tot = 0
        for y in range(y0, y1):
            xl = a1 * y + b1
            xr = a2 * y + b2
            x0 = int(min(xl, xr)) + 2
            x1 = int(max(xl, xr)) - 2
            if x0 < 0:
                x0 = 0
            if x1 > W - 1:
                x1 = W - 1
            for x in range(x0, x1 + 1):
                if im[y][x] > thr:
                    br += 1
                tot += 1
        if tot and br / tot > mx:
            mx = br / tot
    return mx


def det_verdict(d, im):
    """复刻 review_bridge_gui._det_verdict -> (effective, reason); None=无检测"""
    if not (d['R'] or d['B'] or d['G'] or d['T']):
        return None, '无检测'
    lines = {}
    for tag, side in (('R', -1), ('B', +1), ('G', 0)):
        if d[tag]:
            a = d[tag.lower() + 'a'] / 100.0
            b = d[tag.lower() + 'b'] / 100.0
            if abs(a) > 5 or abs(b) > 200:
                continue
            lines[tag] = (a, b, side, line_maxr(im, a, b, side))
    if d['mode'] == 3:
        return False, '纯绿线'
    edges = {t: L for t, L in lines.items() if t in 'RB'}
    if not edges:
        return False, '无边线'
    if any(L[3] <= MXR_THR for L in edges.values()):
        return False, '边线无亮度差 maxr<=%.2f' % MXR_THR
    if 'R' in lines and 'B' in lines:
        pair = (lines['R'], lines['B'])
        pname = 'R-B'
    elif 'G' in lines:
        e = list(edges)[0]
        pair = (lines[e], lines['G'])
        pname = '%s-G' % e
    else:
        return False, '无边线对(仅单边线)'
    (a1, b1, _, _), (a2, b2, _, _) = pair
    cc = (a1 * a2 + 1.0) / (math.sqrt(a1 * a1 + 1.0) * math.sqrt(a2 * a2 + 1.0))
    ang = math.degrees(math.acos(max(-1.0, min(1.0, cc))))
    if ang > ANGLE_MAX:
        return False, '线对%s夹角过大 %.0f°' % (pname, ang)
    da = a2 - a1
    db = b2 - b1
    yc = None if abs(da) < 1e-9 else -db / da
    if yc is not None and yc > YC_MAX:
        return False, '线对%s靠近点在下 y=%.0f' % (pname, yc)
    wmin = min(abs((a2 * y + b2) - (a1 * y + b1)) for y in range(60))
    if wmin < WMIN_THR:
        return False, '线对%s过近 w_min=%.0f' % (pname, wmin)
    mxw = interline_maxwhite(im, a1, b1, a2, b2)
    if mxw <= STRIP_W_THR:
        return False, '线对%s内无白带 白=%.2f' % (pname, mxw)
    return True, '全通'


def load_bin(path):
    with open(path, 'rb') as f:
        data = f.read(94 * 60)
    return [list(data[y * 94:(y + 1) * 94]) for y in range(60)]


def main():
    ref = load_serial(REF)
    print('参考帧总数:', len(ref))
    n_ok = n_bad = 0
    reasons = Counter()
    valid_cnt = 0
    bad = []
    for nm, d in ref.items():
        # 帧名: "20260807_单边桥_XX_室外__20260807_单边桥_XX_室外__frame_00000"
        parts = nm.split('__')
        vid = parts[0]          # 20260807_单边桥_01_室外
        fno = parts[-1]         # frame_00000
        # bin 文件名 = {vid}__{fno}.bin
        p = os.path.join(BINS, vid + '_bin', vid + '__' + fno + '.bin')
        if not os.path.isfile(p):
            bad.append((nm, '缺 bin'))
            n_bad += 1
            continue
        im = load_bin(p)
        eff, reason = det_verdict(d, im)
        if eff is None:
            reasons['无检测'] += 1
            # 无检测帧: 参考 v= 应为 0
            if d['v'] != 0:
                n_bad += 1
                bad.append((nm, f'无检测但 v={d["v"]}'))
            else:
                n_ok += 1
            continue
        reasons[reason] += 1
        if eff:
            valid_cnt += 1
        if (1 if eff else 0) == d['v']:
            n_ok += 1
        else:
            n_bad += 1
            bad.append((nm, f'PC={int(eff)} ref v={d["v"]}  {reason}'))

    total = len(ref)
    print(f'\n===== PC 规则 vs 参考 v= (共 {total} 帧) =====')
    print(f'一致: {n_ok}/{total}   不一致: {n_bad}')
    if bad:
        print('差异示例(前20):')
        for b in bad[:20]:
            print(' ', b)
    print(f'\n===== 拒绝原因分布 (PC 规则) =====')
    print(f'有效(全通): {valid_cnt}')
    for k, c in reasons.most_common():
        if k == '无检测':
            continue
        print(f'  {k}: {c}')
    print(f'  无检测(无线): {reasons.get("无检测", 0)}')
    # 与提示词 §4c 核对 (按拒绝原因分组, 提示词为简化分类)
    print('\n===== 与提示词 §4c 记录核对 (按原因分组) =====')
    groups = {
        '纯绿线': lambda r: r.startswith('纯绿线'),
        '仅单边线/无边线对': lambda r: r.startswith('无边线对'),
        '边线无亮度差 maxr<=0.35': lambda r: r.startswith('边线无亮度差'),
        '线对过近 w_min<15': lambda r: '过近' in r,
        '线对内无白带 <=0.5': lambda r: '内无白带' in r,
        '夹角过大 >=90': lambda r: '夹角过大' in r,
        '靠近点在下 y>30': lambda r: '靠近点在下' in r,
        '无边线(有标志无线)': lambda r: r == '无边线',
    }
    for key, fn in groups.items():
        got = sum(c for rk, c in reasons.items() if fn(rk))
        print(f'  {key}: {got}')
    print(f'  无检测(无线): {reasons.get("无检测", 0)}')
    print(f'\n有效帧数: PC={valid_cnt}  参考={sum(1 for d in ref.values() if d["v"]==1)}  (期望 1903)')
    if n_bad == 0:
        print('\n[PASS] PC 规则交叉验证通过: 0 差异')
    else:
        print(f'\n[FAIL] 有 {n_bad} 帧不一致')
    return 0 if n_bad == 0 else 1


if __name__ == '__main__':
    import sys
    sys.exit(main())
