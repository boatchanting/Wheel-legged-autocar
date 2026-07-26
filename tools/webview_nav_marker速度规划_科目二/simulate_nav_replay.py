#!/usr/bin/env python3
"""NavReplay_Process 全链路状态机模拟器"""
import math

POINTS = [
    {"x": -6157.630, "y": -2400.899, "type": 1},
    {"x": -9010.166, "y":  1548.725, "type": 1},
    {"x":-13279.622, "y": -1906.268, "type": 1},
    {"x":-13813.029, "y": -4210.723, "type": 1},
    {"x":-15472.892, "y": -4835.228, "type": 0},
    {"x":-16274.644, "y": -3431.491, "type": 0},
    {"x": -8777.436, "y":   158.597, "type": 0},
]
N = len(POINTS)
EXEC_R=200.0; PREP_R=200.0; BRAKE_M=100.0; DECEL=170.0
BR_MIN=2600.0; BR_MAX=2600.0; WEAK_FF=180.0
FAST=800.0; SLOW=120.0; PATH_R=70.0
STEP=240.0; STEP_START=250.0; TRIG=800.0; CRAWL_M=80.0; DT=0.01

def d(a,b): return math.hypot(b["x"]-a["x"],b["y"]-a["y"])
def br(spd):
    sd=(spd**2)/(2*DECEL); r=EXEC_R+BRAKE_M+sd
    r=max(BR_MIN,min(BR_MAX,r))+WEAK_FF; return max(BR_MIN,min(BR_MAX,r))
def cr(abs_s): return (abs_s**2)/(2*DECEL)+CRAWL_M
def pds(dist_mm,sr):
    rem=dist_mm-sr
    if rem<=0: return 0.0
    v=min(math.sqrt(2*110*rem),FAST)
    if v<SLOW and rem>PATH_R: v=SLOW
    return v

def is_sp(t): return t!=0

print("="*72)
print("NavReplay 全链路模拟 (dist<brake 触发补丁)")
print("="*72)
print("\n路表:")
for i,p in enumerate(POINTS):
    t="CIRCLE" if p["type"]==1 else ("PATH(终)" if i==N-1 else "PATH")
    print(f"  P{i}: ({p['x']:+.0f},{p['y']:+.0f}) {t}")

print(f"\n点间距 (brake={br(FAST):.0f} mm):")
for i in range(N-1):
    dd=d(POINTS[i],POINTS[i+1])
    s=""
    if POINTS[i]["type"]==1 and POINTS[i+1]["type"]==1:
        if dd<br(FAST):
            s=f" ⚠ 补丁! ({dd:.0f}<brake={br(FAST):.0f}, patched={max(EXEC_R,min(BR_MAX,dd*0.5)):.0f})"
        else:
            s=f" ✓正常"
    print(f"  P{i}→P{i+1}: {dd:.0f} mm{s}")

cx,cy=0.0,0.0; ti=0; cyc=0
zb_i=zb_a=cr_a=pr_l=ex_e=False; cap_r=0.0

print("\n逐周期 (仅关键事件):")
last_state=""
def log(s):
    global last_state
    # 提取状态关键字 (P? + 状态名)
    parts=s.split("] ",1)[-1].split(" ") if "] " in s else [s]
    key=" ".join(parts[:2]) if len(parts)>=2 else s
    if key!=last_state: print(s); last_state=key

while cyc<50000 and ti<N:
    cyc+=1; p=POINTS[ti]; dtp=d({"x":cx,"y":cy},p); pt=p["type"]; last=(ti>=N-1)
    if not is_sp(pt):
        if last:
            log(f"[{cyc:3d}] P{ti} FINISH d={dtp:.0f}"); break
        if dtp<=PATH_R:
            log(f"[{cyc:3d}] P{ti} ARRIVE d={dtp:.0f} -> P{ti+1}")
            ti+=1; zb_i=zb_a=cr_a=pr_l=ex_e=False; cap_r=0.0; continue
        v=pds(dtp,PATH_R)
        log(f"[{cyc:3d}] P{ti} PATH d={dtp:.0f} spd={v:.0f}")
        mv=min(v*DT,dtp); dx=p["x"]-cx;dy=p["y"]-cy; l=math.hypot(dx,dy)
        if l>0: cx+=dx/l*mv;cy+=dy/l*mv
        continue

    obr=br(FAST); pbr=obr
    ni=ti+1; dtn=1e9
    if ni<N and is_sp(POINTS[ni]["type"]):
        dtn=d(p,POINTS[ni])
        if dtn<obr: pbr=max(EXEC_R,min(BR_MAX,dtn*0.5))
    in_b=(dtp<=pbr)

    if not zb_i and not in_b and dtn<obr:
        ov=-FAST*0.5
        log(f"[{cyc:3d}] P{ti} PATCH spd={ov:.0f} d={dtp:.0f} pbr={pbr:.0f}")
        mv=abs(ov)*DT; dx=p["x"]-cx;dy=p["y"]-cy; l=math.hypot(dx,dy)
        if l>0: cx+=dx/l*mv;cy+=dy/l*mv; continue

    if not zb_i and not in_b:
        v=pds(dtp,EXEC_R)
        log(f"[{cyc:3d}] P{ti} CRUISE d={dtp:.0f} spd={v:.0f}")
        mv=v*DT; dx=p["x"]-cx;dy=p["y"]-cy; l=math.hypot(dx,dy)
        if l>0: cx+=dx/l*mv;cy+=dy/l*mv; continue

    if in_b and not zb_i:
        zb_i=zb_a=True; cap_r=FAST
        log(f"[{cyc:3d}] P{ti} BRAKE_ZONE d={dtp:.0f} pbr={pbr:.0f}")

    ex_e=(dtp<=EXEC_R)
    if ex_e:
        log(f"[{cyc:3d}] P{ti} TRIGGER d={dtp:.0f}")
        zb_i=zb_a=cr_a=pr_l=ex_e=False; cap_r=0.0; ti+=1
        log(f"[{cyc:3d}] P{ti-1} SPIN_DONE -> P{ti}"); continue

    rem=dtp-EXEC_R; crel=cr(STEP_START)
    if cr_a:
        if dtp<=PREP_R and not pr_l:
            pr_l=True; cr_a=False; zb_a=True
            log(f"[{cyc:3d}] P{ti} CRAWL->PREP spd=0"); continue
        if dtp>EXEC_R:
            log(f"[{cyc:3d}] P{ti} CRAWL d={dtp:.0f} spd={STEP:.0f}")
            mv=STEP*DT; dx=p["x"]-cx;dy=p["y"]-cy; l=math.hypot(dx,dy)
            if l>0: cx+=dx/l*mv;cy+=dy/l*mv; continue

    if zb_a:
        if dtp<=PREP_R and not pr_l: pr_l=True
        if rem>crel:
            zb_a=False; cr_a=True
            log(f"[{cyc:3d}] P{ti} ZB->CRAWL rem={rem:.0f}>{crel:.0f}")
            mv=STEP*DT; dx=p["x"]-cx;dy=p["y"]-cy; l=math.hypot(dx,dy)
            if l>0: cx+=dx/l*mv;cy+=dy/l*mv; continue
        else:
            log(f"[{cyc:3d}] P{ti} ZB_WAIT d={dtp:.0f} rem={rem:.0f}"); continue

    log(f"[{cyc:3d}] P{ti} WAIT d={dtp:.0f}")

print(f"\n模拟结束, {cyc} 周期")
