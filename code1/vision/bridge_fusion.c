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
   ref 阶段以参考检测器左右线段为包络, 要求 bridge_found 成立。 */
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

/* ---- 脱出双重门控更新 (v8 阶段): 连续检出脱出线 且 底部全亮 ----
   2026-08-14 用户定案: 双重门控 = ① 连续 BF_TOP_T_FRAMES 帧检出脱出线
   (v8.has_top / v11_top_gy); ② 且每帧满足"进入"的底部全亮门控
   (bf_v8_bottom_bright)。两条件同时满足才计帧, 连续帧数达到即锁存,
   次帧切回参考检测器提取脱出线。 */
static void bf_update_gate_top_v8(bf_state_t *st, const uint8_t *img,
                                  const bridge_result_t *v8, float *ratio_out)
{
    *ratio_out = -1.0f;                 /* 原顶部白占比不再评估 */
    if (st->gate_top)
        return;
    if (v8->has_top && bf_v8_bottom_bright(img, v8)) {
        if (st->top_t_streak < 255)
            st->top_t_streak++;
        if (st->top_t_streak >= BF_TOP_T_FRAMES)
            st->gate_top = 1;
    } else {
        st->top_t_streak = 0;
    }
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

void bridge_fusion_init(bf_state_t *st)
{
    memset(st, 0, sizeof(*st));
    bridge_detect_init(&st->v8_st);
    bridge_detection_default_config(&st->ref_cfg);
    st->ref_cfg.fixed_threshold = BF_REF_FIXED_THRESHOLD;
}

void bridge_fusion_frame(const uint8_t *img94, bf_state_t *st, bf_result_t *out)
{
    memset(out, 0, sizeof(*out));
    out->top_white_ratio = -1.0f;
    if (img94 == 0)
        return;

    /* ---- 管线选择: 上一帧门控决定本帧引擎 (帧首唯一的 if) ----
       初始 gate_bottom=0 → 参考检测器 (远处); gate_bottom 锁存 → v8;
       gate_top 再锁存 → 回到参考检测器 (脱出线)。 */
    if (st->gate_bottom && !st->gate_top) {
        /* == v8 三线透视 (桥上) == */
        out->source = BF_SRC_V8;
        bridge_detect_frame(img94, &st->v8_st, &out->v8);
        bf_center_from_v8(st, &out->v8, &out->center);
        out->valid = (uint8_t)(out->v8.valid &&
                     (out->v8.has_red || out->v8.has_green || out->v8.has_blue));
        /* 门控更新: 底部 gate 与 v8 内部锁存同源同步; 评估脱出双重门控 */
        if (out->v8.gate)
            st->gate_bottom = 1;
        bf_update_gate_top_v8(st, img94, &out->v8, &out->top_white_ratio);
    } else {
        /* == 参考检测器 (远处接近 / 脱出) == */
        out->source = BF_SRC_REF;
        (void)bridge_detection_detect_gray(img94, BF_W, BF_H, BF_W,
                                           &st->ref_cfg, &st->ref_scratch,
                                           &out->ref);
        bf_center_from_ref(&out->ref, &out->center);
        out->valid = (uint8_t)(out->ref.bridge_found &&
                               out->ref.center_segment.valid);
        /* 门控更新: 仅接近阶段 (gate_top 未锁存) 评估底部白 gate;
           脱出阶段两门控均已锁存, 为终态 */
        if (!st->gate_top)
            bf_update_gate_bottom_ref(st, img94, &out->ref);
    }

    out->gate_bottom = st->gate_bottom;
    out->gate_top = st->gate_top;
}
