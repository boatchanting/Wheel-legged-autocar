/**
 * ============================================================================
 * bridge_fusion.h  ——  远近融合桥检测管线 (参考检测器 + 现有 v8 三线透视)
 * ============================================================================
 * Copyright (C) 2026  Ji Zixiang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * 动机:
 *   v8 三线透视在远处直线拟合效果差, 无法识别桥面; 参考检测器
 *   (bridge_ref_detection.c, 亮区连通域+凸包, 绝对阈值) 远处稳定。
 *
 * 管线 (每帧开头按上一帧门控二选一, 跑完后用本帧结果更新门控):
 *
 *     gate_bottom=0 --------------> gate_bottom=1 ----------> gate_top=1
 *     [专用 PVC]    入口到达锁存    [v8 三线透视]  双重门控     [参考检测器]
 *     入口竖直线                   桥上中线        锁存         脱出线
 *
 *   - gate_bottom (2026-08-15 起): 准备进入阶段由专用 PVC (bridge_pvc_vision)
 *     检测白色入口, 其"最后结束线"(白色连通域底线 entry_bottom_y) 严格大于
 *     BF_PVC_GATE_BOT_Y 即单帧锁存; v8 阶段直接用 v8 内部 st->gate
 *     (两者同源, 必然已锁存)。原 ref 底部白 gate 逻辑已注释保留 (见 bridge_fusion.c)。
 *   - gate_top (脱出双重门控): v8 阶段每帧评估两个条件 ——
 *       ① 检出脱出线 (v8.has_top / v11_top_gy);
 *       ② 满足"进入"的底部全亮门控 (底部行带 [52,59] 最外侧两线包络内
 *          白占比 >75%, 镜像 v8 内部 gate);
 *     连续 BF_TOP_T_FRAMES 帧两条件同时成立即锁存, 次帧切回参考检测器
 *     提取脱出线。锁存后不撤销 (与底部 gate 同构)。
 *     2026-08-14 用户定案: 原"顶部行带白占比<25%"判据在宽桥场景下永不触发;
 *     先后试过"连续检出结束线 5 帧 / 3 帧 / 加 y>5", 最终定为上述双重门控。
 *
 * 注意:
 *   - bf_state_t 含 BridgeDetectionScratch (~22 KiB), 必须静态/全局分配,
 *     严禁放任务栈。
 *   - 每帧只跑一个引擎, v8 的间距先验自校准从切入门控后的首帧才开始累积。
 *   - v8 内部 gate 在切入后首帧即自行锁存 (桥上底部必白), 粉线/结束线
 *     不受影响。
 *
 * 使用:
 *   static bf_state_t  fst;
 *   static bf_result_t fres;
 *   bridge_fusion_init(&fst);
 *   while (1) {
 *       bridge_fusion_frame(image_94x60, &fst, &fres);
 *       // fres.valid 时 fres.center 为统一中线 x = a*y + b
 *   }
 * ============================================================================
 */
#ifndef _BRIDGE_FUSION_H_
#define _BRIDGE_FUSION_H_

#include <stdint.h>

#include "bridge_detect.h"          /* 现有 v8 管线 (近处/桥上)          */
#include "bridge_ref_detection.h"   /* 参考检测器 (远处接近/脱出)        */
#include "bridge_pvc_vision.h"      /* 单边桥专用 PVC (准备进入)         */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 图像规格: 与 v8 一致, 94x60 uint8 灰度 (行优先, 行宽 94) ---- */
#define BF_W                    BRIDGE_W    /* 94 */
#define BF_H                    BRIDGE_H    /* 60 */

/* ---- 参考检测器配置 ---- */
#define BF_REF_FIXED_THRESHOLD  140         /* 绝对阈值 (同 code1 BRIDGE_VISION_FIXED_THRESHOLD) */

/* ---- 底部白 gate (ref 阶段评估, 镜像 v8 gate: 行带 52..59, 白占比 > 75%) ---- */
#define BF_GATE_BOT_ROW_LO      52          /* 底部行带起始 (同 v8 GATE_ROWS) */
#define BF_GATE_BOT_WHITE_MIN   0.75f       /* 白占比锁存阈值 (2026-08-14: 50%→75%) */

/* ---- 准备进入阶段专用 PVC gate (2026-08-15): 白色连通域底线 entry_bottom_y
   严格大于该行号即锁存 gate_bottom, 拖动状态机切 v8。可调参, 现场标定。 ---- */
#define BF_PVC_GATE_BOT_Y        45

/* ---- v8→ref 切换 (v8 阶段评估, 2026-08-15 门控放开) ----
   v8 结束线检出 (v8.has_top && 底部全亮) 只用于切换到准确脱出管线 (ref);
   视觉侧门控彻底放开: 阈值=1 即单帧检出即锁存 gate_top, 多帧防抖由控制侧 (0核) 处理。
   正确帧 = v8.has_top 且底部全亮门控 (底部行带红/蓝左右边界包络白占比 >50%,
   无左/右边线时分别回退画面边缘 x=2 / x=W-2)。 */
#define BF_TOP_T_FRAMES        1           /* 视觉侧门控已放开 (单帧即切, 防抖靠控制侧) */

/* ---- 0-1-2 防瞬间跳边 (2026-08-15) ----------------
   进入桥上(v8)阶段后, 最少待 BF_ON_BRIDGE_MIN_FRAMES 帧才允许评估
   脱出门控 (切"准备脱出")。可调节宏: 实车按帧率/桥长现场标定。 */
#define BF_ON_BRIDGE_MIN_FRAMES  10

/* ---- 脱出线确认 (ref 阶段, 2026-08-15 门控放开) ----------------
   视觉侧门控彻底放开: 阈值=1 即单帧检出顶边线即锁存 exit_confirmed (不撤销);
   多帧防抖/阈值由控制侧 (0核) 处理 (0核仅在 mode2 且 exit_y 连续达阈值才脱出)。 */
#define BF_EXIT_STREAK_THRESHOLD  1U

/* ---- 白像素判定灰度阈 (与参考检测器绝对阈值同源) ---- */
#define BF_WHITE_TH             BF_REF_FIXED_THRESHOLD

/* ---- 本帧输出来源 ---- */
typedef enum {
    BF_SRC_REF = 0,     /* 参考检测器 (远处接近 / 脱出)  */
    BF_SRC_V8  = 1,     /* 现有 v8 三线透视 (桥上)       */
    BF_SRC_PVC = 2      /* 单边桥专用 PVC (准备进入)     */
} bf_source_t;

/* ---- 跨帧状态 (含 ~22 KiB 参考检测器工作区, 必须静态分配) ---- */
typedef struct {
    bridge_state_t          v8_st;          /* v8 私有状态 (v8 帧才更新)   */
    BridgeDetectionConfig   ref_cfg;        /* 参考检测器配置              */
    BridgeDetectionScratch  ref_scratch;    /* 参考检测器工作区 (~22 KiB)  */
    uint8_t  gate_bottom;                   /* 底部变白锁存 (0→1 切 v8)    */
    uint8_t  gate_top;                      /* 结束线锁存 (0→1 切回 ref)   */
    uint8_t  top_t_streak;                  /* 连续结束线帧计数            */
    uint8_t  on_bridge_frames;              /* v8 阶段累计帧数 (防瞬间 0-1-2) */
    uint8_t  exit_streak;                   /* 脱出线三态累计计数 (+1正确/+0无检测/-1坏帧) */
    uint8_t  exit_confirmed;                /* 脱出线确认锁存 (0→1 不撤销) */
    float    exit_top_a;                    /* 最近正确帧脱出线几何缓存 y=a*x+b */
    float    exit_top_b;
} bf_state_t;

/* ---- 单帧结果 ---- */
typedef struct {
    bf_source_t source;             /* 本帧管线来源                    */
    uint8_t   valid;                /* 统一中线有效                    */
    uint8_t   gate_bottom;          /* 帧末底部白 gate                 */
    uint8_t   gate_top;             /* 帧末顶部白 gate                 */
    bridge_line_t center;           /* 统一中线 x = a*y+b (valid 时)   */
    float     top_white_ratio;      /* 本帧顶部包络白占比; 未评估为 -1 */
    uint8_t   exit_confirmed;       /* 帧末脱出线确认锁存              */
    float     exit_top_a;           /* 脱出线几何 (确认后有效) y=a*x+b */
    float     exit_top_b;
    bridge_result_t v8;             /* v8 原始输出 (source==V8 时有效) */
    BridgeDetectionResult ref;      /* ref 原始输出 (source==REF 时有效) */
} bf_result_t;

void bridge_fusion_init(bf_state_t *st);

/* img94: 94x60 uint8 灰度, 行优先, 行宽 94 */
void bridge_fusion_frame(const uint8_t *img94, bf_state_t *st, bf_result_t *out);

/* ---- 准备进入阶段专用 PVC 辅助 (实现见 bridge_fusion.c) ---- */
void bf_center_from_pvc(const bridge_pvc_vision_output_t *pvc, bridge_line_t *c, uint8_t *valid);
void bf_update_gate_bottom_pvc(bf_state_t *st, const bridge_pvc_vision_output_t *pvc);

#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_FUSION_H_ */
