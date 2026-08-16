/**
 * ============================================================================
 * bridge_fusion.c  ——  远近融合桥检测管线 (见 bridge_fusion.h 头注释)
 * ============================================================================
 * Copyright (C) 2026  Ji Zixiang
 * GPL v3 (or later), 与 bridge_detect.h 同许可。
 * ============================================================================
 */
#include "bridge_fusion.h"

#include <string.h>

/* v8 内部常量 (bridge_detect_v8.c 未导出, 此处保持一致) */
#define BF_Y_REF        55.0f       /* 间距参考行 (同 v8 Y_REF)        */
#define BF_MIN_SPACING  14.0f       /* 最小红蓝间距 (同 v8 MIN_SPACING) */

/* ---- 行带 [y0,y1] 内两条竖线 (x=a*y+b) 包络的白像素占比 ----
   与 v8 底部 gate 同构: 每行取 xl/xr 内缩 2px, 越界钳位;
   tot < 8 (包络退化) 时返回 -1 表示本带不可评估。 */
static float bf_white_ratio_band(const uint8_t *img,
                                 float al, float bl, float ar, float br,
                                 int y0, int y1)
{
    int br_cnt = 0, tot = 0, y, x;
    for (y = y0; y <= y1; y++) {
        float xl = al * y + bl;
        float xr = ar * y + br;
        int x0 = (int)(xl < xr ? xl : xr) + 2;
        int x1 = (int)(xl > xr ? xl : xr) - 2;
        const uint8_t *row;
        if (x0 < 0)
            x0 = 0;
        if (x1 > BF_W - 1)
            x1 = BF_W - 1;
        row = img + y * BF_W;
        for (x = x0; x <= x1; x++) {
            br_cnt += row[x] > BF_WHITE_TH;
            tot++;
        }
    }
    if (tot < 8)
        return -1.0f;
    return (float)br_cnt / (float)tot;
}

/* ---- 参考检测器线段 (x0,y0)-(x1,y1) → x = a*y+b, 返回是否可表示 ---- */
static int bf_segment_to_ab(const BridgeDetectionSegment *s, float *a, float *b)
{
    int dy = s->y1 - s->y0;
    if (!s->valid || dy <= 0)
        return 0;
    *a = (float)(s->x1 - s->x0) / (float)dy;
    *b = (float)s->x0 - *a * (float)s->y0;
    return 1;
}

/* ---- 底部白 gate 更新 (ref 阶段): 镜像 v8 gate 逻辑 ----
   v8 原版: 行带 [GATE_ROWS,H) 内最外两条线包络白占比 > 75% 单帧锁存。
   ref 阶段以参考检测器左右线段为包络, 要求 bridge_found 成立。
   [2026-08-15 用户决策 6] 准备进入阶段改用专用 PVC 判定 gate, 本函数注释保留。 */
#if 0
static void bf_update_gate_bottom_ref(bf_state_t *st, const uint8_t *img,
                                      const BridgeDetectionResult *ref)
{
    float al, bl, ar, br;
    if (st->gate_bottom || !ref->bridge_found)
        return;
    if (!bf_segment_to_ab(&ref->left_segment, &al, &bl) ||
        !bf_segment_to_ab(&ref->right_segment, &ar, &br))
        return;
    if (bf_white_ratio_band(img, al, bl, ar, br,
                            BF_GATE_BOT_ROW_LO, BF_H - 1) > BF_GATE_BOT_WHITE_MIN)
        st->gate_bottom = 1;
}
#endif

/* ---- "进入"的底部全亮门控 (v8 阶段逐帧评估, 镜像 v8 内部 gate) ----
   底部行带 [BF_GATE_BOT_ROW_LO, BF_H-1] 左右边界包络内白占比 >
   BF_GATE_BOT_WHITE_MIN。
   左界 = 红(左界)线, 无红则回退画面左缘 x=2;
   右界 = 蓝(右界)线, 无蓝则回退画面右缘 x=W-2 (2026-08-14 用户定案:
   mode=2 等单边线场景也能评估底部全亮)。与 v8 内部 gate 同构 (行带 52..59,
   br*2>tot); 白阈用融合层 BF_WHITE_TH (v8 用其内部双峰 tb_in, 未导出)。 */
static int bf_v8_bottom_bright(const uint8_t *img, const bridge_result_t *v8)
{
    bridge_line_t edge_l, edge_r;
    const bridge_line_t *L, *R;
    float r;
    if (v8->has_red)
        L = &v8->red;
    else {
        edge_l.a = 0.0f;
        edge_l.b = 2.0f;                /* 无左边线 → x=2 */
        L = &edge_l;
    }
    if (v8->has_blue)
        R = &v8->blue;
    else {
        edge_r.a = 0.0f;
        edge_r.b = (float)(BF_W - 2);   /* 无右边线 → x=W-2 */
        R = &edge_r;
    }
    if (L == R)
        return 0;                       /* 包络退化 (理论上不会发生) */
    r = bf_white_ratio_band(img, L->a, L->b, R->a, R->b,
                            BF_GATE_BOT_ROW_LO, BF_H - 1);
    return r > BF_GATE_BOT_WHITE_MIN;
}

/* ---- 脱出双重门控更新 (v8 阶段): 三态衰减累计 (2026-08-15) ----
   与 ref 脱出线确认同一套累计算法:
   正确帧 (+1): 检出脱出线 (v8.has_top) 且底部全亮 (bf_v8_bottom_bright);
   无检测帧 (保持): v8 本帧无有效线 (valid=0, 缺帧/无输出);
   坏帧 (-1): 有有效线但未满足脱出双重门控;
   累计达 BF_TOP_T_FRAMES 即锁存 gate_top (不撤销), 次帧切回参考检测器。 */
static void bf_update_gate_top_v8(bf_state_t *st, const uint8_t *img,
                                  const bridge_result_t *v8, float *ratio_out)
{
    *ratio_out = -1.0f;                 /* 原顶部白占比不再评估 */
    if (st->gate_top)
        return;

    if (v8->has_top && bf_v8_bottom_bright(img, v8)) {
        /* 正确帧: +1 */
        if (st->top_t_streak < 255U)
            st->top_t_streak++;
    } else if (!v8->valid) {
        /* 无检测帧: 保持 (缺帧/无输出不打断连击) */
    } else {
        /* 坏帧: 有有效线但未满足脱出双重门控 → 衰减 */
        if (st->top_t_streak > 0U)
            st->top_t_streak--;
    }

    if (st->top_t_streak >= BF_TOP_T_FRAMES)
        st->gate_top = 1;
}

/* ---- v8 结果 → 统一中线 x = a*y+b ----
   优先级: 红蓝均值 > 绿(中缝) > 单边线按半先验间距内推。 */
static void bf_center_from_v8(const bf_state_t *st, const bridge_result_t *v8,
                              bridge_line_t *c)
{
    memset(c, 0, sizeof(*c));
    if (v8->has_red && v8->has_blue) {
        c->a = 0.5f * (v8->red.a + v8->blue.a);
        c->b = 0.5f * (v8->red.b + v8->blue.b);
        c->rms = v8->red.rms > v8->blue.rms ? v8->red.rms : v8->blue.rms;
        c->n = (int16_t)((v8->red.n + v8->blue.n) / 2);
        c->u_lo = v8->red.u_lo > v8->blue.u_lo ? v8->red.u_lo : v8->blue.u_lo;
        c->u_hi = v8->red.u_hi < v8->blue.u_hi ? v8->red.u_hi : v8->blue.u_hi;
    } else if (v8->has_green) {
        *c = v8->green;
    } else if (v8->has_red || v8->has_blue) {
        float prior = st->v8_st.wp_a * BF_Y_REF + st->v8_st.wp_b;
        if (prior < BF_MIN_SPACING)
            prior = BF_MIN_SPACING;
        *c = v8->has_red ? v8->red : v8->blue;
        c->b += v8->has_red ? 0.5f * prior : -0.5f * prior;
    } else {
        return;                         /* 无线, 无效 */
    }
    if (c->u_hi <= c->u_lo) {           /* 支撑范围退化时给全行带 */
        c->u_lo = 0.0f;
        c->u_hi = (float)(BF_H - 1);
    }
}

/* ---- 参考检测器结果 → 统一中线 x = a*y+b ---- */
static void bf_center_from_ref(const BridgeDetectionResult *ref, bridge_line_t *c)
{
    float a, b;
    memset(c, 0, sizeof(*c));
    if (!bf_segment_to_ab(&ref->center_segment, &a, &b))
        return;
    c->a = a;
    c->b = b;
    c->n = (int16_t)(ref->left_line.inlier_count + ref->right_line.inlier_count);
    c->rms = 0.5f * (ref->left_line.residual + ref->right_line.residual);
    c->u_lo = (float)ref->center_segment.y0;
    c->u_hi = (float)ref->center_segment.y1;
}

/* ---- 专用 PVC 结果 → 统一中线 x = a*y+b (竖直线 x=target_x, 全行带支撑) ----
   valid 与 target_x 严格绑定: 均取 stable (用户决策 3)。
   u_lo/u_hi 必须给全行带 [0,BF_H-1], 保证 0核 y=25 前视点在支撑内。 */
void bf_center_from_pvc(const bridge_pvc_vision_output_t *pvc,
                        bridge_line_t *c, uint8_t *valid)
{
    memset(c, 0, sizeof(*c));
    *valid = pvc->stable_detected;
    if (*valid == 0U)
        return;                         /* 未稳定看到: 无效, 0核锁角兜底 */

    c->a = 0.0f;                        /* 竖直线 x = target_x */
    c->b = (float)pvc->stable.target_x_px_x100 * 0.01f;
    c->rms = 0.0f;
    c->n = 0;
    c->u_lo = 0.0f;
    c->u_hi = (float)(BF_H - 1);
}

/* ---- 准备进入阶段 gate (专用 PVC): 白色连通域底线 entry_bottom_y 严格大于
   阈值即锁存 gate_bottom, 拖动状态机切 v8。与 ref 原 gate 同构 (单帧锁存)。 ---- */
void bf_update_gate_bottom_pvc(bf_state_t *st,
                               const bridge_pvc_vision_output_t *pvc)
{
    if (st->gate_bottom)
        return;
    if (!pvc->stable_detected)
        return;
    if (pvc->stable.entry_bottom_y > BF_PVC_GATE_BOT_Y)
        st->gate_bottom = 1;
}

/* ---- 参考检测器顶边横线 segment → y = a*x+b ---- */
static int bf_top_segment_to_ab(const BridgeDetectionSegment *s, float *a, float *b)
{
    int dx = s->x1 - s->x0;
    if (!s->valid || dx == 0)
        return 0;
    *a = (float)(s->y1 - s->y0) / (float)dx;
    *b = (float)s->y0 - *a * (float)s->x0;
    return 1;
}

/* ---- 脱出线确认 (ref 阶段, gate_top 锁存后) ----
   三态衰减累计: 正确帧 +1, 无检测帧保持, 坏帧 -1; 达阈值锁存 exit_confirmed。
   正确帧始终刷新几何缓存 (确认后也持续更新, 供 0核 exit_y 随车下移)。 */
static void bf_update_exit_line(bf_state_t *st, const BridgeDetectionResult *ref,
                                uint8_t *confirmed, float *top_a, float *top_b)
{
    float a = 0.0f, b = 0.0f;

    if (ref->bridge_found && ref->top_line_visible &&
        ref->top_segment.valid &&
        bf_top_segment_to_ab(&ref->top_segment, &a, &b))
    {
        /* 正确帧: 始终刷新几何缓存 */
        st->exit_top_a = a;
        st->exit_top_b = b;
        if (st->exit_confirmed == 0U)
        {
            if (st->exit_streak < 255U)
                st->exit_streak++;
            if (st->exit_streak >= BF_EXIT_STREAK_THRESHOLD)
                st->exit_confirmed = 1U;
        }
    }
    else if (!ref->bridge_found)
    {
        /* 无检测帧: 保持 (缺帧/无输出不打断连击) */
    }
    else
    {
        /* 坏帧: 找到桥面但无顶边线 → 衰减 (确认后不再衰减) */
        if (st->exit_confirmed == 0U && st->exit_streak > 0U)
            st->exit_streak--;
    }

    *confirmed = st->exit_confirmed;
    *top_a = st->exit_top_a;
    *top_b = st->exit_top_b;
}

void bridge_fusion_init(bf_state_t *st)
{
    memset(st, 0, sizeof(*st));
    bridge_detect_init(&st->v8_st);
    bridge_detection_default_config(&st->ref_cfg);
    st->ref_cfg.fixed_threshold = BF_REF_FIXED_THRESHOLD;
    bridge_pvc_vision_init();   /* 单边桥专用 PVC: 状态机刚开默认跑, 一并初始化/复位 */
}

void bridge_fusion_frame(const uint8_t *img94, bf_state_t *st, bf_result_t *out)
{
    memset(out, 0, sizeof(*out));
    out->top_white_ratio = -1.0f;
    if (img94 == 0)
        return;

    /* ---- 管线选择: 上一帧门控决定本帧引擎 (帧首唯一的 if) ----
       初始 gate_bottom=0 → 专用 PVC (准备进入); gate_bottom 锁存 → v8 (桥上);
       gate_top 再锁存 → 参考检测器 (准备脱出, 脱出线)。 */
    if (st->gate_bottom && !st->gate_top) {
        /* == v8 三线透视 (桥上) == */
        out->source = BF_SRC_V8;
        if (st->on_bridge_frames < 255U)
            st->on_bridge_frames++;
        bridge_detect_frame(img94, &st->v8_st, &out->v8);
        bf_center_from_v8(st, &out->v8, &out->center);
        out->valid = (uint8_t)(out->v8.valid &&
                     (out->v8.has_red || out->v8.has_green || out->v8.has_blue));
        /* 门控更新: 底部 gate 与 v8 内部锁存同源同步; 评估脱出双重门控 */
        if (out->v8.gate)
            st->gate_bottom = 1;
        /* 0-1-2 防瞬间跳边: 进入桥上后最少待 BF_ON_BRIDGE_MIN_FRAMES 帧才评估脱出门控 */
        if (st->on_bridge_frames >= BF_ON_BRIDGE_MIN_FRAMES)
            bf_update_gate_top_v8(st, img94, &out->v8, &out->top_white_ratio);
    } else if (st->gate_top) {
        /* == 准备脱出: 参考检测器 (脱出线) == */
        out->source = BF_SRC_REF;
        (void)bridge_detection_detect_gray(img94, BF_W, BF_H, BF_W,
                                           &st->ref_cfg, &st->ref_scratch,
                                           &out->ref);
        bf_center_from_ref(&out->ref, &out->center);
        out->valid = (uint8_t)(out->ref.bridge_found &&
                               out->ref.center_segment.valid);
        /* 脱出阶段 (gate_top 已锁存): 三态确认脱出线 (ref 顶边横线) */
        bf_update_exit_line(st, &out->ref,
                            &out->exit_confirmed,
                            &out->exit_top_a, &out->exit_top_b);
    } else {
        /* == 准备进入: 单边桥专用 PVC (替代 ref, 检测执行在状态机内) == */
        bridge_pvc_vision_output_t pvc_local;
        out->source = BF_SRC_PVC;
        bridge_pvc_vision_process_camera_frame(img94);
        pvc_local = *bridge_pvc_vision_get_output();  /* 去掉 volatile 拷贝 */
        bf_center_from_pvc(&pvc_local, &out->center, &out->valid);
        /* gate_bottom 判定: PVC「最后结束线」entry_bottom_y > 阈值 → 锁存 → 切 v8 */
        bf_update_gate_bottom_pvc(st, &pvc_local);
    }

    out->gate_bottom = st->gate_bottom;
    out->gate_top = st->gate_top;
}
