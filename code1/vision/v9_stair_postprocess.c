/**
 * ============================================================================
 * v9_stair_postprocess.c  ——  V9 台阶检测后处理实现
 * ============================================================================
 * Copyright (C) 2026  Ji Zixiang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * ============================================================================
 * 平台: Infineon CYT4BB7 Cortex-M7
 *
 * 实现三阶段后处理:
 *   1. stair_discriminate  — 台阶/背景判别
 *   2. hough_boundary      — Hough 变换边界检测
 *   3. detect_crease       — Crease 检测
 * ============================================================================
 */

#include "v9_stair_postprocess.h"
#include <string.h>
#include <math.h>


/* ==========================================================================
 * Hough 参数空间 (静态分配, 避免 malloc)
 * 注意: 采用 [theta][rho] 转置布局 —— 投票按 theta 分条进行,
 *       每个 theta 行仅 884B, 投票访问全部落在 L1 D-Cache 内,
 *       消除原 [rho][theta] 布局的跨行随机 cache miss (主要瓶颈)。
 * ========================================================================== */
#define HOUGH_RHO_BINS  (V9_HOUGH_RHO_MAX * 2 + 1)   /* 221 bins */
static int32_t  hough_accum[V9_HOUGH_NUM_THETA][HOUGH_RHO_BINS];

/* 边缘点列表: 投票前一次性提取, 避免阈值判断在 5187×61 内层重复执行
 * rc 打包为 (r<<16)|c, 与打包后的 (sin<<16)|cos 配合,
 * 投票内层可用单条 SMLAD 完成 c*cos + r*sin (见 V9_SMLAD) */
static uint32_t hough_edge_rc[57 * 91];    /* (r<<16)|c, ~20KB SRAM */
static uint16_t hough_edge_mag[57 * 91];   /* |Gx|,      ~10KB SRAM */

/* SMLAD: 双 16×16 有符号乘加 (ARMv7E-M 单指令)。
 * IAR 9.40 内建签名为 __SMLAD(x, y, sum) = x.lo*y.lo + x.hi*y.hi + sum,
 * 利用第三参数直接折叠 +16384 舍入项; PC 验证 (MSVC/gcc) 用等效 C 实现,
 * 数学结果逐位一致 */
#if defined(__ICCARM__)
#include <intrinsics.h>
#define V9_SMLAD_RHO(rc, sc)  (__SMLAD((rc), (sc), 16384) >> 15)
#else
#define V9_SMLAD_RHO(rc, sc)  (((int32_t)(int16_t)((rc) & 0xFFFFu) * (int16_t)((sc) & 0xFFFFu) \
                              + (int32_t)(int16_t)((rc) >> 16)     * (int16_t)((sc) >> 16) \
                              + 16384) >> 15)
#endif


/* ==========================================================================
 * 内部辅助: 直方图法计算分位数阈值 (int16 值域 [-1020,+1020]→abs [0,1020])
 * ========================================================================== */
static float percentile_histogram(const int16_t *data, uint32_t n, uint32_t pct)
{
    uint32_t hist[1021];
    uint32_t i, cum;
    int16_t  v;

    memset(hist, 0, sizeof(hist));
    for (i = 0; i < n; i++) {
        v = data[i];
        if (v < 0) v = (int16_t)(-v);
        if (v > 1020) v = 1020;
        hist[(uint32_t)v]++;
    }

    cum = 0;
    for (i = 0; i <= 1020; i++) {
        cum += hist[i];
        if (cum >= (uint32_t)((uint64_t)n * pct / 100)) {
            return (float)i;
        }
    }
    return 1020.0f;
}


/* ==========================================================================
 * stair_discriminate  ——  台阶/背景判别
 * ========================================================================== */
float stair_discriminate(
    const int16_t *p_gx,
    const int16_t *p_gy,
    uint32_t       rows,
    uint32_t       cols,
    uint32_t       gy_var_start_row)
{
    float    P[91];         /* |Gx| 列总和 */
    float    gy_prof[57];   /* Gy 行均值剖面 */
    uint32_t r, c;
    float    sum_P, mean_P, var_P, gx_score;
    float    sum_gy, mean_gy, var_gy;

    /* ---- 计算 |Gx| 列总和 P[cols] ---- */
    memset(P, 0, sizeof(P));
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            int16_t v = p_gx[r * cols + c];
            if (v < 0) v = -v;
            P[c] += (float)v;
        }
    }

    /* gx_score = var(P)^2 / sum(P) */
    sum_P  = 0.0f;
    for (c = 0; c < cols; c++) sum_P += P[c];
    if (sum_P < 1.0f) return 0.0f;

    mean_P = sum_P / (float)cols;
    var_P  = 0.0f;
    for (c = 0; c < cols; c++) {
        float d = P[c] - mean_P;
        var_P += d * d;
    }
    var_P /= (float)cols;
    gx_score = var_P * var_P / sum_P;

    /* ---- 计算 Gy 行均值剖面 gy_prof[rows] ---- */
    memset(gy_prof, 0, sizeof(gy_prof));
    for (r = 0; r < rows; r++) {
        sum_gy = 0.0f;
        for (c = 0; c < cols; c++) {
            sum_gy += (float)p_gy[r * cols + c];
        }
        gy_prof[r] = sum_gy / (float)cols;
    }

    /* gy_var = var(gy_prof[gy_var_start_row .. rows-1]) — 仅底部行 */
    if (gy_var_start_row >= rows) gy_var_start_row = rows - 1;
    {
        uint32_t n_bottom = rows - gy_var_start_row;
        if (n_bottom < 2) return 0.0f;

        sum_gy  = 0.0f;
        for (r = gy_var_start_row; r < rows; r++) sum_gy += gy_prof[r];
        mean_gy = sum_gy / (float)n_bottom;
        var_gy  = 0.0f;
        for (r = gy_var_start_row; r < rows; r++) {
            float d = gy_prof[r] - mean_gy;
            var_gy += d * d;
        }
        var_gy /= (float)n_bottom;
    }

    return gx_score * var_gy;
}


/* ==========================================================================
 * hough_boundary  ——  Hough 变换边界检测 (Q15 定点 + 缓存优化 v2)
 * v2: 累加器转置 [theta][rho] + 边缘点预提取 + 峰值检测阈值预筛,
 *     输出与原 [rho][theta] 实现逐位一致 (投票值、峰值集合、收集顺序相同)
 * ========================================================================== */
void hough_boundary(
    const int16_t    *p_gx,
    uint32_t          rows,
    uint32_t          cols,
    v9_stair_result_t *p_result)
{
    /* Q15 定点 cos/sin 查找表: cos_q15=cos(θ)*32768, sin_q15=sin(θ)*32768 */
    int16_t  cos_q15[V9_HOUGH_NUM_THETA];
    int16_t  sin_q15[V9_HOUGH_NUM_THETA];
    float    thetas[V9_HOUGH_NUM_THETA];
    int32_t  rho_offset = V9_HOUGH_RHO_MAX;
    uint32_t r, c, t;
    int32_t  diag;
    float    threshold;
    int32_t  max_val;
    uint32_t num_peaks;

    /* ---- 预计算 θ, cos, sin 表 (一次, 61 个值) ---- */
    for (t = 0; t < V9_HOUGH_NUM_THETA; t++) {
        float th = (V9_HOUGH_THETA_MIN
                    + (V9_HOUGH_THETA_MAX - V9_HOUGH_THETA_MIN)
                      * (float)t / (float)(V9_HOUGH_NUM_THETA - 1))
                   * 3.14159265f / 180.0f;
        thetas[t]    = th;
        cos_q15[t]   = (int16_t)(cosf(th) * 32768.0f + 0.5f);
        sin_q15[t]   = (int16_t)(sinf(th) * 32768.0f + 0.5f);
    }

    diag = (int32_t)(sqrtf((float)(rows * rows + cols * cols)) + 0.5f);

    /* ---- 直方图法计算 80 分位数阈值 ---- */
    threshold = percentile_histogram(p_gx, rows * cols,
                                     (uint32_t)V9_HOUGH_PERCENTILE);

    /* ---- 边缘点一次性提取 (阈值判断只执行 5187 次, 而非在投票内层重复) ----
     * threshold 恒为整数值 (percentile 返回 (float)i), 整数比较与浮点比较逐位等价 */
    uint32_t num_edges = 0;
    {
        int32_t thr_i = (int32_t)threshold;
        for (r = 0; r < rows; r++) {
            for (c = 0; c < cols; c++) {
                int16_t v = p_gx[r * cols + c];
                int16_t mag = (v < 0) ? (int16_t)(-v) : v;
                if ((int32_t)mag <= thr_i) continue;
                if (num_edges < (uint32_t)(57 * 91)) {
                    hough_edge_rc[num_edges]  = ((uint32_t)r << 16) | c;
                    hough_edge_mag[num_edges] = (uint16_t)mag;
                    num_edges++;
                }
            }
        }
    }

    /* ---- 参数空间投票 (Q15 定点, 按 theta 分条, 行内 cache 全命中) ----
     * 注: 已穷举证明 |rho| <= 106 < diag (=107) 对所有 (r,c,t) 成立,
     *     原实现的 [-diag, diag] 边界检查永不触发, 故安全移除;
     *     内层单条 SMLAD 完成 c*cos + r*sin, 与 int32 乘法逐位等价 */
    memset(hough_accum, 0, sizeof(hough_accum));
    for (t = 0; t < V9_HOUGH_NUM_THETA; t++) {
        int32_t *accum_row = hough_accum[t];
        uint32_t packed_sc = ((uint32_t)(uint16_t)sin_q15[t] << 16)
                           | (uint16_t)cos_q15[t];
        uint32_t i;

        /* 内循环: Q15 定点 rho = (c*cos + r*sin + 16384) >> 15 */
        for (i = 0; i < num_edges; i++) {
            int32_t rho = V9_SMLAD_RHO(hough_edge_rc[i], packed_sc);
            accum_row[rho + rho_offset] += (int32_t)hough_edge_mag[i];
        }
    }

    /* ---- 7×7 最大值滤波 + 峰值检测 (两遍: 先找全局最大, 再 0.35 阈值) ---- */
    max_val    = 0;
    num_peaks  = 0;

    typedef struct { int32_t rho; int32_t theta_idx; int32_t votes; } peak_t;
    peak_t peaks[15];

    /* 第一遍: 纯全局最大值扫描
     * (全局最大点必然是其 7×7 邻域最大, 原实现的邻域检查对 max_val 无影响,
     *  逐位等价但省去 13.5K × 49 次比较) */
    for (t = 3; t < V9_HOUGH_NUM_THETA - 3; t++) {
        const int32_t *accum_row = hough_accum[t];
        for (r = 3; r < (uint32_t)(diag * 2 - 3); r++) {
            if (accum_row[r] > max_val) max_val = accum_row[r];
        }
    }

    /* 第二遍: 峰值检测, 阈值 = 0.35 × 全局最大 (与 Python 一致)
     * 实现: 先按 theta 主序收集超过阈值的候选单元 (行内顺序访问, cache 友好),
     *       再按 (rho, theta) 升序排序恢复原版 rho 主序扫描顺序,
     *       峰值集合与收集顺序与原版逐位一致 (候选集合相同, 处理顺序相同) */
    {
        int32_t peak_thr = (int32_t)((float)max_val * 0.35f);
        uint32_t num_cand = 0;

        typedef struct { uint16_t ri; uint16_t ti; } cand_t;
        static cand_t cands[4096];   /* 12KB SRAM; 实际候选通常 < 200 */

        for (t = 3; t < V9_HOUGH_NUM_THETA - 3; t++) {
            const int32_t *accum_row = hough_accum[t];
            for (r = 3; r < (uint32_t)(diag * 2 - 3); r++) {
                if (accum_row[r] > peak_thr) {
                    if (num_cand < 4096) {
                        cands[num_cand].ri = (uint16_t)r;
                        cands[num_cand].ti = (uint16_t)t;
                        num_cand++;
                    }
                }
            }
        }

        /* 按 (rho_bin, theta) 升序排序 (原版扫描顺序), 插入排序足够 */
        {
            uint32_t i, j;
            for (i = 1; i < num_cand; i++) {
                cand_t key = cands[i];
                uint32_t key_k = ((uint32_t)key.ri << 16) | key.ti;
                j = i;
                while (j > 0) {
                    uint32_t prev_k = ((uint32_t)cands[j-1].ri << 16) | cands[j-1].ti;
                    if (prev_k <= key_k) break;
                    cands[j] = cands[j-1];
                    j--;
                }
                cands[j] = key;
            }
        }

        {
            uint32_t k;
            for (k = 0; k < num_cand && num_peaks < 15; k++) {
                int32_t  val;
                uint32_t rr, tt;
                int32_t  is_max = 1;

                r = cands[k].ri;
                t = cands[k].ti;
                val = hough_accum[t][r];

                for (tt = t - 3; tt <= t + 3 && is_max; tt++) {
                    for (rr = r - 3; rr <= r + 3 && is_max; rr++) {
                        if (rr == r && tt == t) continue;
                        if (hough_accum[tt][rr] > val) is_max = 0;
                    }
                }
                if (!is_max) continue;

                peaks[num_peaks].rho       = (int32_t)r - rho_offset;
                peaks[num_peaks].theta_idx = (int32_t)t;
                peaks[num_peaks].votes     = val;
                num_peaks++;
            }
        }
    }

    if (num_peaks < 2) return;

    /* 按票数排序 */
    {
        uint32_t i, j;
        for (i = 0; i < num_peaks - 1; i++) {
            for (j = i + 1; j < num_peaks; j++) {
                if (peaks[j].votes > peaks[i].votes) {
                    peak_t tmp = peaks[i];
                    peaks[i] = peaks[j];
                    peaks[j] = tmp;
                }
            }
        }
    }

    /* ---- 左右分组 ---- */
    {
        int32_t mid_x  = (int32_t)cols / 2;
        int32_t left_best  = -1, right_best = -1;
        int32_t left_votes = 0, right_votes = 0;
        uint32_t i;

        for (i = 0; i < num_peaks && i < 15; i++) {
            float   th = thetas[peaks[i].theta_idx];
            float   x_intercept = (cosf(th) > 0.05f)
                                ? (float)peaks[i].rho / cosf(th)
                                : (float)peaks[i].rho;

            if (x_intercept > 0.0f && x_intercept < (float)mid_x
                && peaks[i].votes > left_votes) {
                left_best  = (int32_t)i;
                left_votes = peaks[i].votes;
            }
            if (x_intercept > (float)mid_x && peaks[i].votes > right_votes) {
                right_best  = (int32_t)i;
                right_votes = peaks[i].votes;
            }
        }

        if (left_best >= 0 && right_best >= 0) {
            float thL = thetas[peaks[left_best].theta_idx];
            float thR = thetas[peaks[right_best].theta_idx];

            p_result->left_rho    = (float)peaks[left_best].rho;
            p_result->left_theta  = thL;
            p_result->right_rho   = (float)peaks[right_best].rho;
            p_result->right_theta = thR;

            /* ---- 角平分线中线 ---- */
            {
                float aL = cosf(thL), bL = sinf(thL);
                float aR = cosf(thR), bR = sinf(thR);
                float cL = -(float)peaks[left_best].rho;
                float cR = -(float)peaks[right_best].rho;

                float a1 = aL + aR, b1 = bL + bR, c1 = cL + cR;
                float a2 = aL - aR, b2 = bL - bR, c2 = cL - cR;

                float n1 = sqrtf(a1*a1 + b1*b1);
                float n2 = sqrtf(a2*a2 + b2*b2);

                if (n1 > 1e-6f && n2 > 1e-6f) {
                    float x1 = (a1 != 0) ? -c1 / a1 : 999;
                    float xL  = (cosf(thL) > 0.05f)
                              ? (float)peaks[left_best].rho / cosf(thL) : 999;
                    float xR  = (cosf(thR) > 0.05f)
                              ? (float)peaks[right_best].rho / cosf(thR) : 999;
                    float x_mid = (xL + xR) / 2.0f;

                    if (fabsf(x1 - x_mid) < fabsf((-c2/a2) - x_mid)) {
                        p_result->center_a = a1 / n1;
                        p_result->center_b = b1 / n1;
                        p_result->center_c = c1 / n1;
                    } else {
                        p_result->center_a = a2 / n2;
                        p_result->center_b = b2 / n2;
                        p_result->center_c = c2 / n2;
                    }
                }
            }
        }
    }
}


/* ==========================================================================
 * detect_crease  ——  Crease 检测
 * ========================================================================== */
void detect_crease(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v9_stair_result_t *p_result)
{
    float    gy_prof[57];
    uint32_t r, c;
    float    sum_gy;

    /* ---- 计算 Gy 行均值剖面 ---- */
    for (r = 0; r < rows; r++) {
        sum_gy = 0.0f;
        for (c = 0; c < cols; c++) {
            sum_gy += (float)p_gy[r * cols + c];
        }
        gy_prof[r] = sum_gy / (float)cols;
    }

    /* ---- Step 2.1: 提取正峰和负谷 ---- */
    typedef struct { uint32_t idx; float val; } extremum_t;
    extremum_t pos_peaks[57], neg_peaks[57];
    uint32_t   num_pos = 0, num_neg = 0;

    for (r = 1; r < rows - 1; r++) {
        if (gy_prof[r] > gy_prof[r-1] && gy_prof[r] > gy_prof[r+1]) {
            pos_peaks[num_pos].idx = r;
            pos_peaks[num_pos].val = gy_prof[r];
            num_pos++;
        } else if (gy_prof[r] < gy_prof[r-1] && gy_prof[r] < gy_prof[r+1]) {
            neg_peaks[num_neg].idx = r;
            neg_peaks[num_neg].val = gy_prof[r];
            num_neg++;
        }
    }

    /* ---- Step 2.2: 排序 ---- */
    {
        uint32_t i, j;
        /* 正峰: 降序 */
        for (i = 1; i < num_pos; i++) {
            extremum_t key = pos_peaks[i];
            j = i;
            while (j > 0 && pos_peaks[j-1].val < key.val) {
                pos_peaks[j] = pos_peaks[j-1];
                j--;
            }
            pos_peaks[j] = key;
        }
        /* 负谷: 绝对值降序 */
        for (i = 1; i < num_neg; i++) {
            extremum_t key = neg_peaks[i];
            j = i;
            while (j > 0 && fabsf(neg_peaks[j-1].val) < fabsf(key.val)) {
                neg_peaks[j] = neg_peaks[j-1];
                j--;
            }
            neg_peaks[j] = key;
        }
    }

    /* ---- Step 2.3: 极性选择 ---- */
    extremum_t *chosen = NULL;
    uint32_t    num_chosen = 0;
    float       pos_prod = 0.0f, neg_prod = 0.0f;

    if (num_pos >= 2) pos_prod = pos_peaks[0].val * pos_peaks[1].val;
    if (num_neg >= 2) neg_prod = fabsf(neg_peaks[0].val * neg_peaks[1].val);

    if (pos_prod >= neg_prod && num_pos >= 2) {
        chosen     = pos_peaks;
        num_chosen = num_pos;
    } else if (num_neg >= 2) {
        chosen     = neg_peaks;
        num_chosen = num_neg;
    } else {
        p_result->crease_y    = -1;
        p_result->crease_span = 0;
        return;
    }

    /* ---- Step 2.4: 弱极值过滤 ---- */
    {
        float    max_abs = fabsf(chosen[0].val);
        uint32_t i, new_count = 0;

        for (i = 0; i < num_chosen; i++) {
            if (fabsf(chosen[i].val) >= max_abs * V9_WEAK_RATIO) {
                chosen[new_count++] = chosen[i];
            }
        }
        num_chosen = new_count;
        if (num_chosen < 2) {
            p_result->crease_y    = -1;
            p_result->crease_span = 0;
            return;
        }
    }

    /* ---- Step 2.5: 配对尝试 ---- */
    typedef struct {
        uint32_t y;
        uint32_t span;
        uint32_t bottom_y;
    } candidate_t;
    candidate_t candidates[30];
    uint32_t    num_cand = 0;

    {
        uint32_t top_n = (num_chosen < (uint32_t)V9_TOP_N) ? num_chosen : (uint32_t)V9_TOP_N;
        uint32_t k, m;

        for (k = 0; k < top_n; k++) {
            for (m = k + 1; m < top_n; m++) {
                uint32_t i1 = chosen[k].idx;
                uint32_t i2 = chosen[m].idx;
                if (i1 > i2) { uint32_t tmp = i1; i1 = i2; i2 = tmp; }
                uint32_t span = i2 - i1;

                if (span <= (uint32_t)V9_SPAN_MAX) {
                    /* 在两峰之间找 |Gy| 最小的点 */
                    uint32_t crease_idx = i1;
                    float    min_abs    = fabsf(gy_prof[i1]);
                    uint32_t ii;

                    for (ii = i1 + 1; ii <= i2; ii++) {
                        float a = fabsf(gy_prof[ii]);
                        if (a < min_abs) {
                            min_abs    = a;
                            crease_idx = ii;
                        }
                    }

                    /* 谷点验证 */
                    float weaker_peak = fabsf(chosen[k].val);
                    if (fabsf(chosen[m].val) < weaker_peak)
                        weaker_peak = fabsf(chosen[m].val);

                    if (min_abs < weaker_peak - (float)V9_VALLEY_MARGIN) {
                        if (num_cand < 30) {
                            candidates[num_cand].y        = crease_idx;
                            candidates[num_cand].span     = span;
                            candidates[num_cand].bottom_y = (i1 > i2) ? i1 : i2;
                            num_cand++;
                        }
                    }
                }
            }
        }
    }

    if (num_cand == 0) {
        p_result->crease_y    = -1;
        p_result->crease_span = 0;
        return;
    }

    /* ---- Step 2.6: 按 bottom_y 降序排序 ---- */
    {
        uint32_t i, j;
        for (i = 1; i < num_cand; i++) {
            candidate_t key = candidates[i];
            j = i;
            while (j > 0 && candidates[j-1].bottom_y < key.bottom_y) {
                candidates[j] = candidates[j-1];
                j--;
            }
            candidates[j] = key;
        }
    }

    /* 选最多 2 组, 组间间隔 ≥ candidate_gap */
    {
        candidate_t valid[2];
        uint32_t    num_valid = 1;
        uint32_t    i;

        valid[0] = candidates[0];

        for (i = 1; i < num_cand && num_valid < 2; i++) {
            int32_t diff = (int32_t)candidates[i].bottom_y
                         - (int32_t)valid[num_valid-1].bottom_y;
            if (diff < 0) diff = -diff;
            if ((uint32_t)diff >= (uint32_t)V9_CANDIDATE_GAP) {
                valid[num_valid++] = candidates[i];
            }
        }

        p_result->crease_y    = (int16_t)(valid[0].bottom_y - valid[0].span);
        p_result->crease_span = (int16_t)valid[0].span;
    }
}


/* ==========================================================================
 * v9_stair_process_full  ——  完整 V9 后处理管线
 * ========================================================================== */
void v9_stair_process_full(
    const int16_t    *p_gx,
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v9_stair_result_t *p_result)
{
    float joint;

    /* 清零结果 */
    memset(p_result, 0, sizeof(*p_result));
    p_result->crease_y = -1;

    /* Step 1: 台阶判别 */
    joint = stair_discriminate(p_gx, p_gy, rows, cols, V9_GY_VAR_START_ROW);
    p_result->joint_score = joint;

    if (joint > V9_JOINT_THRESHOLD) {
        p_result->has_stairs = 1;

        /* Step 2: Hough 边界检测 (hough_boundary 内部处理 |Gx|) */
        hough_boundary(p_gx, rows, cols, p_result);

        /* Step 3: Crease 检测 */
        detect_crease(p_gy, rows, cols, p_result);
    }
}
