/**
 * ============================================================================
 * bridge_detect.c  ——  单边桥三线透视结构提取 (C 端, 与 pc_tools/bridge_v4.py 一致)
 * ============================================================================
 * Copyright (C) 2026  Ji Zixiang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ============================================================================
 * 流水线:
 *   94x60 → 汇编 4x4 可分离卷积 (Gx/Gy 57x91)
 *   → lock 抑制 + p99 动态阈值 + 每行/列 top-2 候选
 *   → 序贯 RANSAC 提全部竖线 → 间距先验分类 (红/绿/蓝)
 *   → VP 共点精化 → 门控粉色退出线 (五重校验)
 * ============================================================================
 */

#include "bridge_detect.h"
#include "bridge_asm_ops.h"
#include "tcm.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ================================ 常量 (与 PC 版一致) ================================ */
#define W           BRIDGE_W            /* 94  */
#define H           BRIDGE_H            /* 60  */
#define GW          (W - 3)             /* 91  */
#define GH          (H - 3)             /* 57  */

#define LOCK_K      2.0f                /* lock 抑制比 |g| > K·|var|      */
#define T_FLOOR     300.0f              /* 动态阈值下限                   */
#define Q_P99       0.3f                /* 阈值 = Q_P99 · p99             */
#define TOPK        2                   /* 每行/列候选数                  */

#define MIN_INLIERS 4                   /* RANSAC 最少内点                */
#define INLIER_TOL  1.5f                /* 内点容差 (px)                  */
#define RANSAC_ITER 150                 /* RANSAC 迭代数                  */
#define SLOPE_MAX_V 2.5f                /* 竖线斜率上限                   */
#define SLOPE_MAX_H 0.9f                /* 顶线斜率上限                   */

#define VLINE_MAX   4                   /* 每种符号最多提取线数           */
#define MIN_LINE_INL 10                 /* 成线最少内点                   */
#define DEDUP_DX    2.5f                /* 双线去重距离 @Y_REF            */

#define Y_REF       55.0f               /* 间距参考行                     */
#define PRIOR_LO    0.70f               /* 间距先验下容忍                 */
#define MIN_SPACING 14.0f               /* 无先验引导的最小红蓝间距       */

#define GATE_ROWS   52                  /* 底部变白门控起始行             */

/* ---- 行背景判断 (与 pc_tools/bridge_v5.py row_bg_mask 一致) ---- */
#define ROW_BG_FILTER 1                 /* 0=关闭行过滤 (A/B 回退)        */
#define CLU_MAX     4                   /* 行内边缘簇数上限 (红绿蓝+1)    */
#define MID_LO      0.3f                /* 中间带下界 (x 动态阈值 t)      */
#define MID_DIST    2                   /* 强簇拖尾半径                   */
#define MID_OUT_MAX 8                   /* 簇外中间带像素数上限           */
#define ROW_OK_MIN  12                  /* 降级回退的最少有效行数         */

/* ---- 粉色脱出线: 连通亮区顶边界 (与 pc_tools/bridge_v6.py 一致) ---- */
#define TOP_GRAD    0                   /* 1=旧梯度法 (A/B), 0=亮区法     */
#define TOP_SLOPE   0.6f                /* 脱出线斜率上限                 */
#define TOP_MIN_PTS 6                   /* 平台列数/内点下限              */
#define TOP_MIN_SPAN 4.0f               /* 平台 x 跨度下限                */
#define TOP_ABOVE_MAX 0.35f             /* 线上方允许最大亮比例           */
#define TOP_BELOW_MIN 0.5f              /* 线下方最小亮比例               */

#define MID_R_LO    0.35f               /* 中线间距比带                   */
#define MID_R_HI    0.65f
#define MID_A_MRG   0.3f                /* 中线斜率夹逼余量               */
#define MID_SUP_MRG 8.0f                /* 支撑范围余量                   */

#define MAX_CAND    (GH * TOPK)         /* 竖线候选上限 (单符号)          */
#define MAX_TOPC    (GW * TOPK)         /* 顶线候选上限                   */
#define MAX_LINES   (2 * VLINE_MAX)     /* 全部竖线上限                   */

/* ================================ 数据类型 ================================ */
typedef struct { float u, v, w; } bpt_t;    /* 候选点 (自变量, 因变量, 权) */

typedef struct {
    bridge_line_t f;                    /* 拟合直线                       */
    float   inl_u[MAX_CAND];            /* 内点自变量 (竖线: y)           */
    int16_t inl_n;                      /* 内点数                         */
} iline_t;

/* ================================ 静态缓冲 ================================ */
/* SRAM: 降采样图 + 梯度全帧 */
static uint8_t  s_img[H][W];
static int16_t  s_gx[GH][GW];
static int16_t  s_gy[GH][GW];

/* DTCM: 卷积行缓冲 (s_raw 必须 4 字节对齐: 汇编 LDR 字加载) */
#if defined(__ICCARM__)
#define DATA_ALIGN4 _Pragma("data_alignment=4")
#else
#define DATA_ALIGN4
#endif
DATA_ALIGN4 DTCM_BSS int16_t s_raw[W + 2];
DTCM_BSS int16_t s_ringx[4][GW + 1];    /* GW+1=92: 行起始保持 4 字节对齐 */
DTCM_BSS int16_t s_ringy[4][GW + 1];

/* 候选点 / RANSAC 工作区 */
static bpt_t    s_pos[MAX_CAND], s_neg[MAX_CAND];
#if TOP_GRAD
static bpt_t    s_topc[MAX_TOPC];
#endif
static bpt_t    s_rem[MAX_TOPC];        /* 序贯 RANSAC 剩余点 (取大者)    */
static uint8_t  s_mask[MAX_TOPC];
static iline_t  s_lines[MAX_LINES];
static float    s_sort[MAX_CAND];       /* 分位数排序工作区               */

/* 行背景判断: 行有效性掩码 + 行内 strong 标志 + 每行 top-2 暂存 */
static uint8_t  s_row_ok[GH];
static uint8_t  s_strong[GW];
static int16_t  s_bpx[GH][2], s_bpm[GH][2];
static int16_t  s_bnx[GH][2], s_bnm[GH][2];

/* 脱出线 (亮区法): 区域位图 + BFS 队列 + 逐行包络 */
#define REG_WORDS   ((W + 31) / 32)
static uint32_t s_region[H][REG_WORDS];
static uint16_t s_bfs_q[W * H];
static int16_t  s_env_lo[H], s_env_hi[H];
static int8_t   s_col_top[W];

/* ================================ 小工具 ================================ */
static int cmp_f32(const void *a, const void *b)
{
    float d = *(const float *)a - *(const float *)b;
    return (d > 0) - (d < 0);
}

#if TOP_GRAD
static int cmp_u8(const void *a, const void *b)
{
    return (int)*(const uint8_t *)a - (int)*(const uint8_t *)b;
}
#endif

/* gvar: 中间两列垂直 box-diff (抑制 gx 的水平边缘响应), 输出点 (r,j) */
static int gvar_at(int r, int j)
{
    const uint8_t *r0 = s_img[r], *r1 = s_img[r + 1];
    const uint8_t *r2 = s_img[r + 2], *r3 = s_img[r + 3];
    int c1 = j + 1, c2 = j + 2;
    return (r2[c1] + r3[c1] - r0[c1] - r1[c1])
         + (r2[c2] + r3[c2] - r0[c2] - r1[c2]);
}

/* hvar: 中间两行水平 box-diff (抑制 gy), 输出点 (r,j) */
static int hvar_at(int r, int j)
{
    const uint8_t *r1 = s_img[r + 1], *r2 = s_img[r + 2];
    return (r1[j + 2] + r1[j + 3] - r1[j] - r1[j + 1])
         + (r2[j + 2] + r2[j + 3] - r2[j] - r2[j + 1]);
}

/* 256 bin 直方图 (|g|>>4) 的 p99 估计 → 动态阈值 */
static float thr_from_hist(const uint16_t *hist)
{
    uint32_t total = 0, cum = 0;
    int i;
    for (i = 0; i < 256; i++)
        total += hist[i];
    if (!total)
        return T_FLOOR;
    for (i = 0; i < 256; i++) {
        cum += hist[i];
        if (cum * 100 >= total * 99)
            break;
    }
    {
        float p99 = (float)(i << 4);
        float t = Q_P99 * p99;
        return t > T_FLOOR ? t : T_FLOOR;
    }
}

/* ================================ RANSAC ================================ */
static uint32_t s_rng;
static uint32_t xr32(void)
{
    s_rng ^= s_rng << 13;
    s_rng ^= s_rng >> 17;
    s_rng ^= s_rng << 5;
    return s_rng;
}

/* 最大内点数直线 v = a·u + b; 定数种子, 返回内点数 (0=失败) */
static int ransac_best(const bpt_t *p, int n, float smax,
                       float *oa, float *ob, uint8_t *mask)
{
    int best_n = 0, it, i;
    float best_a = 0, best_b = 0;
    if (n < MIN_INLIERS)
        return 0;
    s_rng = 12345;
    for (it = 0; it < RANSAC_ITER; it++) {
        int i1 = (int)(xr32() % (uint32_t)n);
        int i2 = (int)(xr32() % (uint32_t)n);
        float a, b, du;
        int cnt = 0;
        if (i1 == i2)
            continue;
        du = p[i2].u - p[i1].u;
        if (du > -1e-6f && du < 1e-6f)
            continue;
        a = (p[i2].v - p[i1].v) / du;
        if (a > smax || a < -smax)
            continue;
        b = p[i1].v - a * p[i1].u;
        for (i = 0; i < n; i++) {
            float r = p[i].v - (a * p[i].u + b);
            if (r < 0)
                r = -r;
            if (r <= INLIER_TOL)
                cnt++;
        }
        if (cnt > best_n) {
            best_n = cnt;
            best_a = a;
            best_b = b;
        }
    }
    if (best_n < MIN_INLIERS)
        return 0;
    for (i = 0; i < n; i++) {
        float r = p[i].v - (best_a * p[i].u + best_b);
        if (r < 0)
            r = -r;
        mask[i] = (r <= INLIER_TOL);
    }
    *oa = best_a;
    *ob = best_b;
    return best_n;
}

/* 内点加权最小二乘重拟合, 返回 rms */
static float refit(const bpt_t *p, const uint8_t *mask, int n,
                   float *oa, float *ob, int *on)
{
    float sw = 0, su = 0, sv = 0, suu = 0, suv = 0, a, b, se = 0;
    int i, m = 0;
    for (i = 0; i < n; i++) {
        if (mask[i]) {
            float w = p[i].w;
            sw += w;
            su += w * p[i].u;
            sv += w * p[i].v;
            suu += w * p[i].u * p[i].u;
            suv += w * p[i].u * p[i].v;
            m++;
        }
    }
    {
        float den = sw * suu - su * su;
        if (den < 1e-9f && den > -1e-9f)
            den = 1e-9f;
        a = (sw * suv - su * sv) / den;
        b = (sv - a * su) / sw;
    }
    for (i = 0; i < n; i++) {
        if (mask[i]) {
            float r = p[i].v - (a * p[i].u + b);
            se += r * r;
        }
    }
    *oa = a;
    *ob = b;
    *on = m;
    return sqrtf(se / m);
}

/* ================================ 竖线提取 ================================ */
/* 单符号序贯 RANSAC, 结果从 s_lines[base] 起追加, 返回条数 */
static int extract_sign_lines(int base, const bpt_t *pts, int n, int max_out)
{
    int cnt = 0;
    iline_t *L;
    memcpy(s_rem, pts, (size_t)n * sizeof(bpt_t));
    while (cnt < max_out) {
        float a, b, rms;
        int nin, nn, i, m;
        nin = ransac_best(s_rem, n, SLOPE_MAX_V, &a, &b, s_mask);
        if (!nin)
            break;
        rms = refit(s_rem, s_mask, n, &a, &b, &nn);
        if (nn < MIN_LINE_INL)
            break;
        L = &s_lines[base + cnt];
        L->f.a = a;
        L->f.b = b;
        L->f.rms = rms;
        L->f.n = (int16_t)nn;
        /* 记录内点自变量 + 支撑范围 (p10/p90 ± MID_SUP_MRG) */
        m = 0;
        for (i = 0; i < n; i++) {
            if (s_mask[i]) {
                L->inl_u[m] = s_rem[i].u;
                s_sort[m] = s_rem[i].u;
                m++;
            }
        }
        L->inl_n = (int16_t)m;
        qsort(s_sort, (size_t)m, sizeof(float), cmp_f32);
        L->f.u_lo = s_sort[m / 10] - MID_SUP_MRG;
        L->f.u_hi = s_sort[m - 1 - m / 10] + MID_SUP_MRG;
        cnt++;
        /* 移除内点 */
        m = 0;
        for (i = 0; i < n; i++) {
            if (!s_mask[i])
                s_rem[m++] = s_rem[i];
        }
        n = m;
    }
    return cnt;
}

/* 合并排序 (x@Y_REF 升序) + 双线去重, 返回最终线数 */
static int merge_lines(int n)
{
    int i, j, m = 0;
    /* 插入排序 */
    for (i = 1; i < n; i++) {
        iline_t t = s_lines[i];
        float x = t.f.a * Y_REF + t.f.b;
        for (j = i - 1; j >= 0; j--) {
            float xj = s_lines[j].f.a * Y_REF + s_lines[j].f.b;
            if (xj <= x)
                break;
            s_lines[j + 1] = s_lines[j];
        }
        s_lines[j + 1] = t;
    }
    /* 去重: 相邻 < DEDUP_DX 保留内点更多者 */
    for (i = 0; i < n; i++) {
        if (m > 0) {
            float x0 = s_lines[m - 1].f.a * Y_REF + s_lines[m - 1].f.b;
            float x1 = s_lines[i].f.a * Y_REF + s_lines[i].f.b;
            if (x1 - x0 < DEDUP_DX) {
                if (s_lines[i].f.n > s_lines[m - 1].f.n)
                    s_lines[m - 1] = s_lines[i];
                continue;
            }
        }
        s_lines[m++] = s_lines[i];
    }
    return m;
}

/* ================================ 线身份分类 ================================ */
/* 线外侧 4~9px 带亮比例 > 0.5 ?  (side=-1 左 / +1 右) */
static int outer_bright(const iline_t *L, int side, int tb)
{
    int step = L->inl_n / 20, i, br = 0, tot = 0;
    if (step < 1)
        step = 1;
    for (i = 0; i < L->inl_n; i += step) {
        int y = (int)L->inl_u[i];
        int x = (int)(L->f.a * L->inl_u[i] + L->f.b);
        int k;
        if (y < 0 || y >= H)
            continue;
        for (k = 0; k < 6; k++) {
            int xx = (side < 0) ? (x - 9 + k) : (x + 4 + k);
            if (xx >= 0 && xx < W) {
                br += s_img[y][xx] > tb;
                tot++;
            }
        }
    }
    return tot > 0 && br * 2 > tot;
}

/* 中线几何硬条件: 斜率夹逼 + 支撑范围内参考行间距比 ∈ [0.35,0.65] */
static int mid_geo_ok(const iline_t *red, const iline_t *mid,
                      const iline_t *blue)
{
    static const float rows[3] = { 15.0f, 38.0f, Y_REF };
    static const float minw[3] = { 12.0f, 6.0f, 3.0f };
    float alo = (red->f.a < blue->f.a ? red->f.a : blue->f.a) - MID_A_MRG;
    float ahi = (red->f.a > blue->f.a ? red->f.a : blue->f.a) + MID_A_MRG;
    int checked = 0, t;
    if (mid->f.a < alo || mid->f.a > ahi)
        return 0;
    for (t = 0; t < 3; t++) {
        float y = rows[t];
        float xl, xr, w, r;
        if (y < mid->f.u_lo || y > mid->f.u_hi)
            continue;                       /* 支撑外纯外推, 豁免 */
        xl = red->f.a * y + red->f.b;
        xr = blue->f.a * y + blue->f.b;
        w = xr - xl;
        if (w < minw[t])
            continue;
        r = (mid->f.a * y + mid->f.b - xl) / w;
        if (r < MID_R_LO || r > MID_R_HI)
            return 0;
        checked++;
    }
    return checked > 0;
}

/* 分类: 填 ir/ig/ib (索引, -1=无), 返回 mode, *sp_out=红蓝间距(无则0) */
static bridge_mode_t classify(int n, float prior, int tb,
                              int *ir, int *ig, int *ib, float *sp_out)
{
    float xs[MAX_LINES];
    int i, j;
    *ir = *ig = *ib = -1;
    *sp_out = 0;
    for (i = 0; i < n; i++)
        xs[i] = s_lines[i].f.a * Y_REF + s_lines[i].f.b;

    if (n == 0)
        return BRIDGE_MODE_NONE;
    if (n == 1) {
        int lb = outer_bright(&s_lines[0], -1, tb);
        int rb = outer_bright(&s_lines[0], +1, tb);
        if (!lb && rb) { *ir = 0; return BRIDGE_MODE_R; }
        if (lb && !rb) { *ib = 0; return BRIDGE_MODE_B; }
        *ig = 0;
        return BRIDGE_MODE_M;
    }
    /* 选红蓝候选对: 有先验取间距最接近先验的; 无先验取最宽对 */
    {
        int bi = -1, bj = -1;
        float best_sc = 0, bs = 0;
        for (i = 0; i < n; i++) {
            for (j = i + 1; j < n; j++) {
                float s = xs[j] - xs[i], sc;
                if (s < 4)
                    continue;
                sc = (prior > 0) ? fabsf(s - prior) : -s;
                if (bi < 0 || sc < best_sc) {
                    best_sc = sc;
                    bi = i;
                    bj = j;
                    bs = s;
                }
            }
        }
        if (bi < 0)
            return BRIDGE_MODE_NONE;
        if (prior > 0 && bs < PRIOR_LO * prior) {
            /* 间距过近 → 侧线+中线: 外侧亮的是中线 (两侧都在桥面) */
            if (outer_bright(&s_lines[bi], -1, tb)) {
                *ig = bi;
                *ib = bj;
                return BRIDGE_MODE_MB;
            }
            if (outer_bright(&s_lines[bj], +1, tb)) {
                *ir = bi;
                *ig = bj;
                return BRIDGE_MODE_RM;
            }
            *ir = bi;
            *ib = bj;
            return BRIDGE_MODE_RB_Q;    /* 判不出, 保守当红蓝 */
        }
        if (prior <= 0 && bs < MIN_SPACING)
            return BRIDGE_MODE_NONE;
        *ir = bi;
        *ib = bj;
        *sp_out = bs;
        for (j = bi + 1; j < bj; j++) {
            if (mid_geo_ok(&s_lines[bi], &s_lines[j], &s_lines[bj])) {
                *ig = j;
                break;
            }
        }
        return (*ig >= 0) ? BRIDGE_MODE_RMB : BRIDGE_MODE_RB;
    }
}

/* ================================ 亮度阈值 ================================ */
/* 全图 Otsu */
static int otsu_hist(const uint16_t *hist)
{
    uint32_t total = 0, sum = 0, wb = 0, sb = 0;
    float best = -1;
    int i, thr = 0;
    for (i = 0; i < 256; i++) {
        total += hist[i];
        sum += (uint32_t)i * hist[i];
    }
    for (i = 0; i < 256; i++) {
        float num, den, s2;
        wb += hist[i];
        if (!wb || wb == total)
            continue;
        sb += (uint32_t)i * hist[i];
        num = (float)total * sb - (float)sum * wb;
        den = (float)wb * (float)(total - wb);
        s2 = num * num / (den + 1e-12f);
        if (s2 > best) {
            best = s2;
            thr = i;
        }
    }
    return thr;
}

static int otsu_img(void)
{
    uint16_t hist[256];
    int y, x;
    memset(hist, 0, sizeof(hist));
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
            hist[s_img[y][x]]++;
    return otsu_hist(hist);
}

/* 红蓝包裹区内像素的 Otsu (暗块 vs 亮桥面 区内双峰); 样本不足退回全局 */
static int inner_threshold(const iline_t *lf, const iline_t *rf, int tb)
{
    uint16_t hist[256];
    uint32_t n = 0;
    int y, i;
    memset(hist, 0, sizeof(hist));
    for (y = 2; y < H; y += 2) {
        float xl = lf->f.a * y + lf->f.b;
        float xr = rf->f.a * y + rf->f.b;
        int x0 = (int)(xl < xr ? xl : xr) + 2;
        int x1 = (int)(xl > xr ? xl : xr) - 2;
        if (x0 < 0)
            x0 = 0;
        if (x1 > W - 1)
            x1 = W - 1;
        for (i = x0; i <= x1; i++) {
            hist[s_img[y][i]]++;
            n++;
        }
    }
    if (n < 200)
        return tb;
    return otsu_hist(hist);
}

#if TOP_GRAD
/* ================================ 粉线校验 (旧梯度法, TOP_GRAD=1 时编译) ================================ */
/* 区域亮度一致性 (top 带: 亮 1..3, 暗 -4..-1): 亮带p25 - 暗带p75 > delta/2 */
static int region_ok_top(float a, float b, float delta)
{
    static uint8_t bv[256], dv[256];
    int nb = 0, nd = 0, oob = 0, tot = 0, x, off;
    for (x = 1; x < W - 1; x += 2) {
        float yf = a * x + b;
        int yr = (int)(yf + 0.5f);
        for (off = 1; off <= 3; off++) {
            int yi = yr + off;
            tot++;
            if (yi >= 0 && yi < H)
                bv[nb++] = s_img[yi][x];
            else
                oob++;
        }
        for (off = -4; off <= -1; off++) {
            int yi = yr + off;
            if (yi >= 0 && yi < H)
                dv[nd++] = s_img[yi][x];
        }
    }
    if (tot && oob * 10 > tot * 3)
        return 1;
    if (!nb || !nd)
        return 1;
    qsort(bv, (size_t)nb, 1, cmp_u8);
    qsort(dv, (size_t)nd, 1, cmp_u8);
    return (float)bv[nb / 4] - (float)dv[(nd * 3) / 4] > delta * 0.5f;
}

/* 线上方带 (左右线之间) 亮比例 < 0.5 (上方不应是桥面) */
static int bright_ok_top(float a, float b,
                         const iline_t *lf, const iline_t *rf, int tb)
{
    int br = 0, tot = 0, x, off;
    for (x = 2; x < W - 2; x += 2) {
        float yf = a * x + b;
        int yr = (int)(yf + 0.5f);
        for (off = -10; off <= -3; off++) {
            int yi = yr + off;
            float xl, xr;
            if (yi < 0 || yi >= H)
                continue;
            xl = lf->f.a * yi + lf->f.b;
            xr = rf->f.a * yi + rf->f.b;
            if (x >= xl - 2 && x <= xr + 2) {
                br += s_img[yi][x] > tb;
                tot++;
            }
        }
    }
    return tot == 0 || br * 2 < tot;
}

/* 线间灰度带: 均值 + 亮比例 */
static void band_gray(int y0, int y1,
                      const iline_t *lf, const iline_t *rf, int tb,
                      float *mean, float *frac, int *cnt)
{
    int y, x, n = 0, br = 0;
    float sum = 0;
    if (y0 < 0)
        y0 = 0;
    if (y1 > H)
        y1 = H;
    for (y = y0; y < y1; y++) {
        float xl = lf->f.a * y + lf->f.b;
        float xr = rf->f.a * y + rf->f.b;
        int x0 = (int)ceilf(xl < xr ? xl : xr) + 2;
        int x1 = (int)floorf(xl > xr ? xl : xr) - 2;
        if (x0 < 0)
            x0 = 0;
        if (x1 > W - 1)
            x1 = W - 1;
        for (x = x0; x <= x1; x++) {
            sum += s_img[y][x];
            br += s_img[y][x] > tb;
            n++;
        }
    }
    *mean = n ? sum / n : 0;
    *frac = n ? (float)br / n : 0;
    *cnt = n;
}

/* 剖面否决: 上下带同亮且灰度接近 → 同一桥面, 伪顶 */
static int profile_ok_top(float a, float b,
                          const iline_t *lf, const iline_t *rf, int tb)
{
    int y_l = (int)(a * (W * 0.5f) + b + 0.5f);
    float ma, fa, mb, fb;
    int ca, cb;
    band_gray(y_l - 6, y_l - 1, lf, rf, tb, &ma, &fa, &ca);
    band_gray(y_l + 1, y_l + 6, lf, rf, tb, &mb, &fb, &cb);
    if (!ca || !cb)
        return 1;
    return !(fa > 0.6f && fb > 0.6f && fabsf(ma - mb) < 25.0f);
}

/* 禁止横穿亮区: 红蓝跨度内沿线采样, 上下 4px 皆亮比例 > 0.4 → 否决 */
static int crosses_bright(float a, float b,
                          const iline_t *lf, const iline_t *rf, int tb_in)
{
    int both = 0, tot = 0, x;
    for (x = 0; x < W; x += 2) {
        float yf = a * x + b;
        int yi = (int)yf;
        float xl, xr;
        if (yi < 4 || yi >= H - 4)
            continue;
        xl = lf->f.a * yf + lf->f.b;
        xr = rf->f.a * yf + rf->f.b;
        if (x <= xl || x >= xr)
            continue;
        both += (s_img[yi - 4][x] > tb_in) && (s_img[yi + 4][x] > tb_in);
        tot++;
    }
    return tot >= 6 && both * 10 > tot * 4;
}

/* 角点结构: 与两侧线交点须在画面内; 尖端在画面附近时角点须低于尖端 */
static int top_corners_ok(float a, float b,
                          const iline_t *lf, const iline_t *rf)
{
    const iline_t *side[2] = { lf, rf };
    float ymin = 1e9f, dl, vy;
    int t;
    for (t = 0; t < 2; t++) {
        float den = 1.0f - side[t]->f.a * a;
        float x, y;
        if (den < 1e-3f && den > -1e-3f)
            return 0;                       /* 与侧线平行/共线 */
        x = (side[t]->f.a * b + side[t]->f.b) / den;
        y = a * x + b;
        if (x < -3 || x > W + 3 || y < -3 || y > H + 3)
            return 0;
        if (y < ymin)
            ymin = y;
    }
    dl = lf->f.a - rf->f.a;
    if (dl > 1e-6f || dl < -1e-6f) {
        vy = (rf->f.b - lf->f.b) / dl;      /* 红蓝尖端 y */
        if (vy > -20.0f && ymin < vy + 3.0f)
            return 0;                       /* 线穿了尖端 */
    }
    return 1;
}

/* 逐行亮比例 (低通) 最长亮段顶行, 无可靠亮段返回 -1 */
static int bright_run_top(const iline_t *lf, const iline_t *rf, int tb_in)
{
    float prof[H], sm[H];
    int y, x, best_len = 0, best_top = -1, run = -1;
    for (y = 0; y < H; y++) {
        float xl = lf->f.a * y + lf->f.b;
        float xr = rf->f.a * y + rf->f.b;
        int x0, x1, br = 0, n = 0;
        prof[y] = 0;
        if (xr - xl < 6)
            continue;                       /* 消失点上方: 不在桥面 */
        x0 = (int)xl + 2;
        x1 = (int)xr - 2;
        if (x0 < 0)
            x0 = 0;
        if (x1 > W - 1)
            x1 = W - 1;
        for (x = x0; x <= x1; x++) {
            br += s_img[y][x] > tb_in;
            n++;
        }
        if (n >= 2)
            prof[y] = (float)br / n;
    }
    for (y = 0; y < H; y++) {               /* 5 点滑动平均低通 */
        float s = 0;
        int k;
        for (k = -2; k <= 2; k++) {
            if (y + k >= 0 && y + k < H)
                s += prof[y + k];
        }
        sm[y] = s * 0.2f;
    }
    for (y = 0; y <= H; y++) {
        int on = (y < H) && sm[y] > 0.5f;
        if (on && run < 0)
            run = y;
        else if (!on && run >= 0) {
            if (y - run > best_len) {
                best_len = y - run;
                best_top = run;
            }
            run = -1;
        }
    }
    return best_len >= 8 ? best_top : -1;
}

/* ================================ 顶线提取 ================================ */
/* 支撑端点: 内点自变量 pct 分位 → (x_end, y_end) */
static void support_end(const iline_t *L, int pct, float *xe, float *ye)
{
    int m = L->inl_n, i = (m * pct) / 100;
    memcpy(s_sort, L->inl_u, (size_t)m * sizeof(float));
    qsort(s_sort, (size_t)m, sizeof(float), cmp_f32);
    *ye = s_sort[i];
    *xe = L->f.a * s_sort[i] + L->f.b;
}

/* 期望 y(x): 由左右线支撑端点 (10 分位) 插值; 返回锚点数 (0=不可用) */
static int make_yexp(const iline_t *lf, const iline_t *rf,
                     float *x1, float *y1, float *x2, float *y2)
{
    int n = 0;
    if (lf->f.n >= 8) {
        support_end(lf, 10, x1, y1);
        n++;
    }
    if (rf->f.n >= 8) {
        if (n == 0)
            support_end(rf, 10, x1, y1);
        else
            support_end(rf, 10, x2, y2);
        n++;
    }
    return n;
}

static float yexp_at(float x, int nanc,
                     float x1, float y1, float x2, float y2)
{
    if (nanc == 1)
        return y1;
    if (x2 - x1 < 2 && x1 - x2 < 2)
        return (y1 + y2) * 0.5f;
    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}
#endif /* TOP_GRAD */

#if !TOP_GRAD
/* ================================ 脱出线 (亮区顶边界法) ================================ */
/* 与 pc_tools/bridge_v6.py top_from_bright 一致:
   门控行亮像素为种子做 4-连通 BFS (限红蓝包络内) 得桥面亮区,
   逐列顶边 -> 平台 (top<=min_top+tol 最长连续列段) -> 中点精化 -> RANSAC。
   先验: 线上方包络内应全暗; 亮区贴画面顶或上方亮比例高则否决。   */

#define REG_SET(y, x)   (s_region[y][(x) >> 5] |=  (1u << ((x) & 31)))
#define REG_GET(y, x)   (s_region[y][(x) >> 5] &   (1u << ((x) & 31)))

static int extract_top_region(const iline_t *lf, const iline_t *rf,
                              int tb_in, bridge_line_t *tf)
{
    int y, x, i, k, head, tail, ncol, min_top, tol;
    int m = 0, nin, nn;
    float a, b, rms, span;

    /* 逐行包络 (扩张限界 ±3) */
    for (y = 0; y < H; y++) {
        float xl = lf->f.a * y + lf->f.b - 3.0f;
        float xr = rf->f.a * y + rf->f.b + 3.0f;
        s_env_lo[y] = (int16_t)(xl > 0 ? (int)xl : 0);
        s_env_hi[y] = (int16_t)(xr < W - 1 ? (int)xr : W - 1);
    }

    /* 1) BFS: 门控行包络内亮像素为种子 */
    memset(s_region, 0, sizeof(s_region));
    head = tail = 0;
    for (y = GATE_ROWS; y < H; y++) {
        for (x = s_env_lo[y]; x <= s_env_hi[y]; x++) {
            if (s_img[y][x] > tb_in && !REG_GET(y, x)) {
                REG_SET(y, x);
                s_bfs_q[tail++] = (uint16_t)(y * W + x);
            }
        }
    }
    while (head < tail) {
        int p = s_bfs_q[head++];
        static const int8_t d4[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
        int cy = p / W, cx = p % W;
        for (k = 0; k < 4; k++) {
            int yy = cy + d4[k][0], xx = cx + d4[k][1];
            if (yy < 0 || yy >= H || xx < s_env_lo[yy] || xx > s_env_hi[yy])
                continue;
            if (s_img[yy][xx] > tb_in && !REG_GET(yy, xx)) {
                REG_SET(yy, xx);
                s_bfs_q[tail++] = (uint16_t)(yy * W + xx);
            }
        }
    }

    /* 2) 逐列顶边 + 平台 (top <= min_top+tol 的最长连续列段, tol 0..3) */
    ncol = 0;
    min_top = H;
    for (x = 0; x < W; x++) {
        s_col_top[x] = -1;
        for (y = 0; y < H; y++) {
            if (REG_GET(y, x)) {
                s_col_top[x] = (int8_t)y;
                if (y < min_top)
                    min_top = y;
                break;
            }
        }
        ncol += (s_col_top[x] >= 0);
    }
    if (ncol < TOP_MIN_PTS || min_top <= 1)
        return 0;                           /* 列不足 / 亮区贴画面顶 */

    for (tol = 0; tol <= 3 && m == 0; tol++) {
        int run_start = -1, best_len = 0, best_s = -1, best_e = -1;
        for (i = 0; i <= W; i++) {
            int ok = (i < W) && s_col_top[i] >= 0 &&
                     s_col_top[i] <= min_top + tol;
            if (ok && run_start < 0)
                run_start = i;
            if (!ok && run_start >= 0) {
                if (i - run_start > best_len) {
                    best_len = i - run_start;
                    best_s = run_start;
                    best_e = i - 1;
                }
                run_start = -1;
            }
        }
        if (best_len >= TOP_MIN_PTS) {
            /* 2.5) 中点精化: 平台列顶边按局部暗/亮均值中点回扫 */
            for (x = best_s; x <= best_e; x++) {
                int t = s_col_top[x], sd = 0, sb = 0, nd = 0, nb = 0, mid, yy;
                for (yy = t - 6; yy <= t - 2; yy++) {
                    if (yy >= 0) {
                        sd += s_img[yy][x];
                        nd++;
                    }
                }
                for (yy = t + 1; yy <= t + 5; yy++) {
                    if (yy < H) {
                        sb += s_img[yy][x];
                        nb++;
                    }
                }
                if (nd < 2 || nb < 3) {
                    yy = t;
                } else {
                    mid = (sd / nd + sb / nb) / 2;
                    yy = t;
                    while (yy - 1 >= 0 && s_img[yy - 1][x] > mid)
                        yy--;
                }
                s_rem[m].u = x + 0.5f;
                s_rem[m].v = (float)yy;
                s_rem[m].w = 1.0f;
                m++;
            }
        }
    }
    if (m == 0)
        return 0;

    /* 3) 抗噪拟合 */
    nin = ransac_best(s_rem, m, TOP_SLOPE, &a, &b, s_mask);
    if (!nin)
        return 0;
    rms = refit(s_rem, s_mask, m, &a, &b, &nn);
    if (nn < TOP_MIN_PTS)
        return 0;
    {
        float xlo = 1e9f, xhi = -1e9f;
        for (i = 0; i < m; i++) {
            if (s_mask[i]) {
                if (s_rem[i].u < xlo)
                    xlo = s_rem[i].u;
                if (s_rem[i].u > xhi)
                    xhi = s_rem[i].u;
            }
        }
        span = xhi - xlo;
    }
    if (span < TOP_MIN_SPAN)
        return 0;

    /* 4) 先验校验: 线上方 (包络内, 跳过 ±2px 过渡带) 须暗, 线下方须亮 */
    {
        int above = 0, nab = 0, below = 0, nbl = 0;
        for (x = 2; x < W - 2; x += 2) {
            float yf = a * x + b;
            int yi = (int)yf, yy;
            if (!(lf->f.a * yf + lf->f.b < x && x < rf->f.a * yf + rf->f.b))
                continue;
            for (yy = yi - 8; yy <= yi - 3; yy++) {
                if (yy >= 0 && yy < H &&
                    lf->f.a * yy + lf->f.b < x && x < rf->f.a * yy + rf->f.b) {
                    above += s_img[yy][x] > tb_in;
                    nab++;
                }
            }
            for (yy = yi + 2; yy <= yi + 6; yy++) {
                if (yy >= 0 && yy < H &&
                    lf->f.a * yy + lf->f.b < x && x < rf->f.a * yy + rf->f.b) {
                    below += s_img[yy][x] > tb_in;
                    nbl++;
                }
            }
        }
        if (nab < 10) {                 /* 上方包络过窄: 贴顶伪线否决 */
            if (a * (W * 0.5f) + b < 5.0f)
                return 0;
        } else if ((float)above / nab > TOP_ABOVE_MAX) {
            return 0;                   /* 线上方还有大量白色 -> 提取必有错 */
        }
        if (nbl >= 10 && (float)below / nbl < TOP_BELOW_MIN)
            return 0;
    }

    tf->a = a;
    tf->b = b;
    tf->n = (int16_t)nn;
    tf->rms = rms;
    tf->u_lo = tf->u_hi = 0;
    return 1;
}
#endif /* !TOP_GRAD */

/* ================================ 帧处理 ================================ */
void bridge_detect_init(bridge_state_t *st)
{
    memset(st, 0, sizeof(*st));
}

void bridge_detect_frame(const uint8_t *img94,
                         bridge_state_t *st,
                         bridge_result_t *out)
{
    int r, j, i, ring = 0;
    int tb, tb_in = 0;
    int npos = 0, nneg = 0, ntop = 0;
    int nlines, ir, ig, ib;
    float prior = 0, spacing = 0;
    bridge_mode_t mode;

    memset(out, 0, sizeof(*out));

    /* ---- 1) 输入快照 (防处理期间 DMA 改写源缓冲造成撕裂) ---- */
    memcpy(s_img, img94, H * W);

    /* ---- 2) 汇编卷积: 水平环形 + 垂直 ---- */
    for (r = 0; r < H; r++) {
        for (j = 0; j < W; j++)
            s_raw[j] = s_img[r][j];
        bridge_conv1d_horiz_gxgy(s_raw, s_ringx[ring], s_ringy[ring], GW);
        if (r >= 3) {
            int i0 = (ring + 1) & 3;    /* 最老行 r-3 */
            int i1 = (ring + 2) & 3;
            int i2 = (ring + 3) & 3;
            int i3 = ring;              /* 最新行 r   */
            bridge_conv1d_vert_gxgy_row(s_ringx[i0], s_ringx[i1],
                                 s_ringx[i2], s_ringx[i3],
                                 s_ringy[i0], s_ringy[i1],
                                 s_ringy[i2], s_ringy[i3],
                                 s_gx[r - 3], s_gy[r - 3], GW);
        }
        ring = (ring + 1) & 3;
    }

    /* ---- 3) 动态阈值: lock 像素的 |g| p99 ---- */
    {
        static uint16_t hist_p[256], hist_n[256], hist_t[256];
        float tp, tn, tt;
        memset(hist_p, 0, sizeof(hist_p));
        memset(hist_n, 0, sizeof(hist_n));
        memset(hist_t, 0, sizeof(hist_t));
        for (r = 0; r < GH; r++) {
            for (j = 0; j < GW; j++) {
                int gx = s_gx[r][j], ax = gx < 0 ? -gx : gx;
                int gv = gvar_at(r, j);
                int gy = s_gy[r][j], ay = gy < 0 ? -gy : gy;
                int hv = hvar_at(r, j), bin;
                if (gv < 0)
                    gv = -gv;
                if (hv < 0)
                    hv = -hv;
                bin = ax >> 4;
                if (bin > 255)
                    bin = 255;
                if (ax > (int)(LOCK_K * gv)) {
                    if (gx > 0)
                        hist_p[bin]++;
                    else
                        hist_n[bin]++;
                }
                bin = ay >> 4;
                if (bin > 255)
                    bin = 255;
                if (gy > 0 && ay > (int)(LOCK_K * hv))
                    hist_t[bin]++;
            }
        }
        tp = thr_from_hist(hist_p);
        tn = thr_from_hist(hist_n);
        tt = thr_from_hist(hist_t);

        /* ---- 4) 行背景判断 + 候选点: 每行 top-2 ---- */
        /* 第一遍: 各行 strong 标志 + top-2 暂存 + 行有效性掩码
           (bridge_v5.py row_bg_mask: 过阈簇 1..CLU_MAX 且
            簇外中间带 MID_LO·t<|gx|<=t 像素 <= MID_OUT_MAX) */
        for (r = 0; r < GH; r++) {
            int bp_x[2], bp_m[2] = { 0, 0 };
            int bn_x[2], bn_m[2] = { 0, 0 };
            int n_clu = 0, mid_out = 0, last_s = -100;
            for (j = 0; j < GW; j++) {
                int gx = s_gx[r][j], ax = gx < 0 ? -gx : gx;
                int gv = gvar_at(r, j), strong = 0;
                if (gv < 0)
                    gv = -gv;
                if (ax > (int)(LOCK_K * gv)) {
                    if (gx > 0) {
                        if (ax > tp) {
                            strong = 1;
                            if (ax > bp_m[0]) { bp_m[1] = bp_m[0]; bp_x[1] = bp_x[0]; bp_m[0] = ax; bp_x[0] = j; }
                            else if (ax > bp_m[1]) { bp_m[1] = ax; bp_x[1] = j; }
                        } else if (ax > (int)(MID_LO * tp)) {
                            mid_out++;          /* 暂记, 后面剔除强簇邻域 */
                        }
                    } else if (gx < 0) {
                        if (ax > tn) {
                            strong = 1;
                            if (ax > bn_m[0]) { bn_m[1] = bn_m[0]; bn_x[1] = bn_x[0]; bn_m[0] = ax; bn_x[0] = j; }
                            else if (ax > bn_m[1]) { bn_m[1] = ax; bn_x[1] = j; }
                        } else if (ax > (int)(MID_LO * tn)) {
                            mid_out++;
                        }
                    }
                }
                s_strong[j] = (uint8_t)strong;
                if (strong) {
                    if (j - last_s > 2)
                        n_clu++;                /* 间隔 >=3 开新簇 (同 PC) */
                    last_s = j;
                }
            }
            /* 剔除强簇 ±MID_DIST 邻域内的中间带像素 (边缘拖尾) */
            if (mid_out) {
                int in_tail = 0;
                for (j = 0; j < GW; j++) {
                    int k;
                    if (s_strong[j])
                        continue;
                    for (k = j - MID_DIST; k <= j + MID_DIST; k++) {
                        if (k >= 0 && k < GW && s_strong[k])
                            break;
                    }
                    if (k <= j + MID_DIST) {    /* 在强簇拖尾内 */
                        int gx = s_gx[r][j], ax = gx < 0 ? -gx : gx;
                        int gv = gvar_at(r, j);
                        int tmid;
                        if (gv < 0)
                            gv = -gv;
                        tmid = (int)(MID_LO * (gx > 0 ? tp : tn));
                        if (ax > (int)(LOCK_K * gv) && ax > tmid &&
                            ax <= (gx > 0 ? tp : tn))
                            in_tail++;
                    }
                }
                mid_out -= in_tail;
            }
            s_row_ok[r] = (uint8_t)(n_clu >= 1 && n_clu <= CLU_MAX &&
                                    mid_out <= MID_OUT_MAX);
            s_bpx[r][0] = (int16_t)bp_x[0]; s_bpx[r][1] = (int16_t)bp_x[1];
            s_bpm[r][0] = (int16_t)bp_m[0]; s_bpm[r][1] = (int16_t)bp_m[1];
            s_bnx[r][0] = (int16_t)bn_x[0]; s_bnx[r][1] = (int16_t)bn_x[1];
            s_bnm[r][0] = (int16_t)bn_m[0]; s_bnm[r][1] = (int16_t)bn_m[1];
        }
        /* 降级: 有效行过少 -> 全行有效 (整帧杂乱不至于完全无线) */
        {
            int nok = 0;
            for (r = 0; r < GH; r++)
                nok += s_row_ok[r];
            out->n_rows_ok = (uint8_t)nok;
            if (nok < ROW_OK_MIN) {
                for (r = 0; r < GH; r++)
                    s_row_ok[r] = 1;
            }
        }
        /* 第二遍: 有效行候选入队 */
        for (r = 0; r < GH; r++) {
            if (ROW_BG_FILTER && !s_row_ok[r])
                continue;
            for (i = 0; i < 2; i++) {
                if (s_bpm[r][i] && npos < MAX_CAND) {
                    s_pos[npos].u = r + 1.5f;
                    s_pos[npos].v = s_bpx[r][i] + 1.5f;
                    s_pos[npos].w = (float)s_bpm[r][i];
                    npos++;
                }
                if (s_bnm[r][i] && nneg < MAX_CAND) {
                    s_neg[nneg].u = r + 1.5f;
                    s_neg[nneg].v = s_bnx[r][i] + 1.5f;
                    s_neg[nneg].w = (float)s_bnm[r][i];
                    nneg++;
                }
            }
        }
#if TOP_GRAD
        for (j = 0; j < GW; j++) {          /* 顶线: 逐列 top-2 */
            int bt_y[2], bt_m[2] = { 0, 0 };
            for (r = 0; r < GH; r++) {
                int gy = s_gy[r][j], ay;
                int hv = hvar_at(r, j);
                if (gy <= 0)
                    continue;
                ay = gy;
                if (hv < 0)
                    hv = -hv;
                if (ay <= (int)(LOCK_K * hv) || ay <= tt)
                    continue;
                if (ay > bt_m[0]) { bt_m[1] = bt_m[0]; bt_y[1] = bt_y[0]; bt_m[0] = ay; bt_y[0] = r; }
                else if (ay > bt_m[1]) { bt_m[1] = ay; bt_y[1] = r; }
            }
            for (i = 0; i < 2; i++) {
                if (bt_m[i] && ntop < MAX_TOPC) {
                    s_topc[ntop].u = j + 1.5f;   /* 顶线: u=x */
                    s_topc[ntop].v = bt_y[i] + 1.5f;
                    s_topc[ntop].w = (float)bt_m[i];
                    ntop++;
                }
            }
        }
#else
        (void)tt;
        (void)ntop;
#endif
    }

    /* ---- 5) 全图 Otsu ---- */
    tb = otsu_img();

    /* ---- 6) 竖线提取 (正/负分开序贯 RANSAC) ---- */
    nlines = extract_sign_lines(0, s_pos, npos, VLINE_MAX);
    nlines += extract_sign_lines(nlines, s_neg, nneg, VLINE_MAX);
    nlines = merge_lines(nlines);
    out->n_lines = (uint8_t)nlines;

    /* ---- 7) 分类 (红蓝间距先验: 滑动窗中位) ---- */
    if (st->sp_n > 0) {
        float tmp[10];
        memcpy(tmp, st->sp_buf, st->sp_n * sizeof(float));
        qsort(tmp, st->sp_n, sizeof(float), cmp_f32);
        prior = tmp[st->sp_n / 2];
    }
    mode = classify(nlines, prior, tb, &ir, &ig, &ib, &spacing);
    out->mode = mode;
    out->spacing = spacing;
    if ((mode == BRIDGE_MODE_RB || mode == BRIDGE_MODE_RMB) && spacing > 0) {
        st->sp_buf[st->sp_head] = spacing;
        st->sp_head = (uint8_t)((st->sp_head + 1) % 10);
        if (st->sp_n < 10)
            st->sp_n++;
    }

    /* ---- 8) 区内亮阈值 + 门控 (锁存) ---- */
    if (ir >= 0 && ib >= 0)
        tb_in = inner_threshold(&s_lines[ir], &s_lines[ib], tb);
    else
        tb_in = tb;
    if (!st->gate) {
        int first = -1, last = -1;
        if (ir >= 0) { first = last = ir; }
        if (ig >= 0) { if (first < 0) first = ig; last = ig; }
        if (ib >= 0) { if (first < 0) first = ib; last = ib; }
        if (first >= 0 && last != first) {
            const iline_t *fl = &s_lines[first], *fr = &s_lines[last];
            int br = 0, tot = 0, y, x;
            for (y = GATE_ROWS; y < H; y++) {
                float xl = fl->f.a * y + fl->f.b;
                float xr = fr->f.a * y + fr->f.b;
                int x0 = (int)(xl < xr ? xl : xr) + 2;
                int x1 = (int)(xl > xr ? xl : xr) - 2;
                if (x0 < 0)
                    x0 = 0;
                if (x1 > W - 1)
                    x1 = W - 1;
                for (x = x0; x <= x1; x++) {
                    br += s_img[y][x] > tb_in;
                    tot++;
                }
            }
            if (tot > 0 && br * 2 > tot)
                st->gate = 1;
        }
    }
    out->gate = st->gate;

    /* ---- 9) 三线透视共点精化 (失败回退) ---- */
    if (mode == BRIDGE_MODE_RMB) {
        iline_t *lf = &s_lines[ir], *mf = &s_lines[ig], *rf = &s_lines[ib];
        float dl = lf->f.a - rf->f.a, vy, vx;
        float xlv = lf->f.a * Y_REF + lf->f.b;
        float xrv = rf->f.a * Y_REF + rf->f.b;
        out->mid_ratio = (mf->f.a * Y_REF + mf->f.b - xlv)
                       / (xrv - xlv > 1e-6f ? xrv - xlv : 1e-6f);
        if (dl > 1e-6f || dl < -1e-6f) {
            iline_t *trio[3] = { lf, mf, rf };
            float na[3], nb[3];
            int ok = 1, t;
            vy = (rf->f.b - lf->f.b) / dl;
            vx = lf->f.a * vy + lf->f.b;
            if (vy < 80.0f && vx > -400.0f && vx < 400.0f) {
                for (t = 0; t < 3; t++) {
                    float s1 = 0, s2 = 0;
                    for (i = 0; i < trio[t]->inl_n; i++) {
                        float u = trio[t]->inl_u[i];
                        float v = trio[t]->f.a * u + trio[t]->f.b;
                        float dy = u - vy;
                        s1 += dy * dy;
                        s2 += dy * (v - vx);
                    }
                    if (s1 < 1e-6f) {
                        na[t] = trio[t]->f.a;
                        nb[t] = trio[t]->f.b;
                    } else {
                        na[t] = s2 / s1;
                        nb[t] = vx - na[t] * vy;
                    }
                }
                /* 精化后保序: y=15/58 中线须在间距内缩 15% 带内 */
                for (t = 0; t < 2 && ok; t++) {
                    float y = t ? 58.0f : 15.0f;
                    float xl = na[0] * y + nb[0];
                    float xr = na[2] * y + nb[2];
                    float xm = na[1] * y + nb[1];
                    float w = xr - xl;
                    if (w < 3)
                        continue;
                    if (xm < xl + 0.15f * w || xm > xr - 0.15f * w)
                        ok = 0;
                }
                if (ok) {
                    for (t = 0; t < 3; t++) {
                        trio[t]->f.a = na[t];
                        trio[t]->f.b = nb[t];
                    }
                }
            }
        }
    }

    /* ---- 10) 粉色退出线 (门控 + 红蓝都在) ---- */
    if (ir >= 0)
        { out->has_red = 1; out->red = s_lines[ir].f; }
    if (ig >= 0)
        { out->has_green = 1; out->green = s_lines[ig].f; }
    if (ib >= 0)
        { out->has_blue = 1; out->blue = s_lines[ib].f; }

#if TOP_GRAD
    if (st->gate && ir >= 0 && ib >= 0) {
        const iline_t *lf = &s_lines[ir], *rf = &s_lines[ib];
        int nanc, m = 0, round, found = 0;
        float x1 = 0, y1 = 0, x2 = 0, y2 = 0, tlo = 20.0f, thi = 10.0f;
        bridge_line_t tf;
        /* 候选二次过滤: 左右夹逼 ±6 + 期望 y 带 */
        nanc = make_yexp(lf, rf, &x1, &y1, &x2, &y2);
        if (nanc == 1) {
            tlo = 22.0f;
            thi = 12.0f;
        }
        for (i = 0; i < ntop; i++) {
            float x = s_topc[i].u, y = s_topc[i].v;
            float xl = lf->f.a * y + lf->f.b;
            float xr = rf->f.a * y + rf->f.b;
            if (x < xl - 6 || x > xr + 6)
                continue;
            if (nanc > 0) {
                float ye = yexp_at(x, nanc, x1, y1, x2, y2);
                if (y < ye - tlo || y > ye + thi)
                    continue;
            }
            s_rem[m++] = s_topc[i];
        }
        /* 序贯 3 轮: RANSAC + 区域/亮度校验 */
        for (round = 0; round < 3 && !found; round++) {
            float a, b, rms;
            int nin, nn;
            nin = ransac_best(s_rem, m, SLOPE_MAX_H, &a, &b, s_mask);
            if (!nin)
                break;
            rms = refit(s_rem, s_mask, m, &a, &b, &nn);
            if ((a > SLOPE_MAX_H || a < -SLOPE_MAX_H) ||
                !region_ok_top(a, b, 25.0f) ||
                !bright_ok_top(a, b, lf, rf, tb)) {
                /* 剔除该线内点, 继续找下一条 */
                int k = 0;
                for (i = 0; i < m; i++) {
                    if (!s_mask[i])
                        s_rem[k++] = s_rem[i];
                }
                m = k;
                continue;
            }
            tf.a = a;
            tf.b = b;
            tf.n = (int16_t)nn;
            tf.rms = rms;
            tf.u_lo = tf.u_hi = 0;
            found = 1;
        }
        /* 四重否决 */
        if (found && tf.n < 10)
            found = 0;                          /* 内点过少: 背景/阴影伪边 */
        if (found && !profile_ok_top(tf.a, tf.b, lf, rf, tb))
            found = 0;
        if (found && crosses_bright(tf.a, tf.b, lf, rf, tb_in))
            found = 0;                          /* 禁止横穿亮区 */
        if (found && !top_corners_ok(tf.a, tf.b, lf, rf))
            found = 0;                          /* 角点结构 */
        if (found) {
            int ytv = bright_run_top(lf, rf, tb_in);
            if (ytv >= 0 && tf.a * (W * 0.5f) + tf.b > ytv + 5)
                found = 0;                      /* 线落在亮段顶行之下 */
        }
        if (found) {
            out->has_top = 1;
            out->top = tf;
        }
    }
#else
    /* 粉线: 连通亮区顶边界法 (v6), 参数固化为 PC 回归最优值 */
    if (st->gate && ir >= 0 && ib >= 0) {
        bridge_line_t tf;
        if (extract_top_region(&s_lines[ir], &s_lines[ib], tb_in, &tf)) {
            out->has_top = 1;
            out->top = tf;
        }
    }
#endif
}
