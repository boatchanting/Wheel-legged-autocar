#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
verify_valid.py —— 有效检测门控移植回归验证
对 7 个视频共 6081 帧, 用移植后的 bridge_detect.c (PC host) 与 trials 参考
serial_valid_all.txt 逐字段对比:
  * v=         必须逐行一致 (valid 级联与源工程完全一致)
  * ra/rb/ga/gb/ba/bb/ta/tb/tn/trms  必须逐行一致 (未破坏既有检测)
  * mode:      参考 v=1 帧 → 必须与参考一致
               参考 v=0 帧 → 必须为 8 (本工程后处理门控覆写, 预期行为)
用法: python verify_valid.py
"""
import io
import os
import re
import subprocess
import sys
from collections import Counter

ROOT = r'D:\WORKS\2026LunTui\project'
TRIALS = r'D:\WORKS\2026LunTui\trials\bridge\pc_tools'
HOST = os.path.join(ROOT, 'tools', 'bridge_valid_host', 'bridge_host_file.exe')
BINS = os.path.join(TRIALS, 'out_v13c_bins')
REF = os.path.join(TRIALS, 'serial_valid_all.txt')
VIDEOS = ['20260807_单边桥_01_室外', '20260807_单边桥_02_室外', '20260807_单边桥_03_室外',
          '20260807_单边桥_04_室外', '20260807_单边桥_05_室外', '20260807_单边桥_06_室外',
          '20260807_单边桥_07_室外']

# 帧名(ASCII 部分) + 数字字段; 键编码与中文无关 (不依赖 tag 编码)
LINE_RE = re.compile(
    r': \d+ us mode=(\d+) nl=(\d+) nr=(\d+) R(\d+) G(\d+) B(\d+) T(\d+) '
    r'ra=(-?\d+) rb=(-?\d+) ga=(-?\d+) gb=(-?\d+) ba=(-?\d+) bb=(-?\d+) '
    r'ta=(-?\d+) tb=(-?\d+) tn=(\d+) trms=(-?\d+) v=(\d+)')
FRAME_RE = re.compile(r'frame_(\d+)')
VID_RE = re.compile(r'单边桥_(\d+)_')


def parse_text(text, vid):
    """解析一段输出文本 -> {frame_num: fields}; vid 为 1..7 用于参考/输出校验"""
    out = {}
    for line in text.splitlines():
        line = line.strip()
        m = LINE_RE.search(line)
        if not m:
            continue
        fm = FRAME_RE.search(line)
        if not fm:
            continue
        fields = [int(x) for x in m.groups()]
        out[int(fm.group(1))] = fields
    return out


def main():
    # 1) 解析参考文件
    ref_all = {}          # (vid, frame) -> fields
    with io.open(REF, encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            m = LINE_RE.search(line)
            if not m:
                continue
            fm = FRAME_RE.search(line)
            vm = VID_RE.search(line)
            if not fm or not vm:
                continue
            ref_all[(int(vm.group(1)), int(fm.group(1)))] = [int(x) for x in m.groups()]
    print(f'参考帧总数: {len(ref_all)}')

    # 2) 逐视频跑 host
    mine_all = {}
    for v in VIDEOS:
        d = os.path.join(BINS, v + '_bin')
        if not os.path.isdir(d):
            print(f'!! 缺失帧目录: {d}')
            sys.exit(2)
        r = subprocess.run([HOST, d, v, '--init-each'],
                           capture_output=True)
        text = r.stdout.decode('utf-8', errors='replace')
        vid = int(VID_RE.search(v).group(1))
        mine_all.update({(vid, k): val for k, val in parse_text(text, vid).items()})

    print(f'host 输出帧总数: {len(mine_all)}')

    # 3) 对比
    n_v_ok = n_param_ok = n_mode_ok = 0
    n_v_bad = n_param_bad = n_mode_bad = 0
    mode8_count = 0
    reason = Counter()
    bad_examples = []
    keys = sorted(set(ref_all) | set(mine_all))
    for k in keys:
        if k not in ref_all:
            reason['仅host有帧'] += 1
            bad_examples.append((k, 'missing in ref'))
            continue
        if k not in mine_all:
            reason['仅参考有帧'] += 1
            bad_examples.append((k, 'missing in host'))
            continue
        rf = ref_all[k]
        mf = mine_all[k]
        # 字段布局: mode,nl,nr,R,G,B,T, ra,rb,ga,gb,ba,bb, ta,tb,tn,trms, v
        if rf[17] == mf[17]:
            n_v_ok += 1
        else:
            n_v_bad += 1
            reason['v不一致'] += 1
            bad_examples.append((k, f'v ref={rf[17]} mine={mf[17]}'))
        if rf[7:17] == mf[7:17]:
            n_param_ok += 1
        else:
            n_param_bad += 1
            reason['线参数不一致'] += 1
            bad_examples.append((k, f'params ref={rf[7:17]} mine={mf[7:17]}'))
        if rf[17] == 1:
            # 参考有效帧: mode 必须一致
            if rf[0] == mf[0]:
                n_mode_ok += 1
            else:
                n_mode_bad += 1
                reason['有效帧mode不一致'] += 1
                bad_examples.append((k, f'mode ref={rf[0]} mine={mf[0]} v=1'))
        else:
            # 参考无效帧: 本工程 mode 必须为 8 (后处理门控覆写)
            if mf[0] == 8:
                n_mode_ok += 1
                mode8_count += 1
            else:
                n_mode_bad += 1
                reason['无效帧mode非8'] += 1
                bad_examples.append((k, f'mode ref={rf[0]} mine={mf[0]} v=0'))

    total = len(ref_all)
    print(f'\n===== 对比结果 (共 {total} 帧) =====')
    print(f'v=       一致: {n_v_ok}/{total}   不一致: {n_v_bad}')
    print(f'线参数   一致: {n_param_ok}/{total}   不一致: {n_param_bad}')
    print(f'mode     合规: {n_mode_ok}/{total}   违规: {n_mode_bad}')
    print(f'  (其中无效帧被覆写为 mode=8: {mode8_count})')
    if reason:
        print('差异分类:', dict(reason))
    if bad_examples:
        print('\n前 20 条差异示例:')
        for k, msg in bad_examples[:20]:
            print(' ', k, msg)
        sys.exit(1)
    print('\n[PASS] 全部通过: 0 差异')


if __name__ == '__main__':
    main()
