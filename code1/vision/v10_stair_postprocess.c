/**
 * ============================================================================
 * v10_stair_postprocess.c  ——  V10 台阶检测后处理实现
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
 *   2. fit_gy_edges        — Gy 峰值行种子 + RLE 扩展 + 最小二乘拟合
 *   3. detect_crease       — Crease 检测
 * ============================================================================
 */

#include "v10_stair_postprocess.h"
#include <string.h>
#include <math.h>


/* ==========================================================================
 * Gy 边缘 RLE 扩展静态缓冲 (仅峰值行上下扩展, 非全图扫描)
 * ========================================================================== */
typedef struct {
    uint8_t  start_x;
    uint8_t  end_x;
} gy_run_t;

static gy_run_t gy_seed_runs[V10_GY_MAX_RUNS];   /* 峰值行 runs */
static gy_run_t gy_prev_row_runs[V10_GY_MAX_RUNS]; /* 上一行 runs (扩展用) */
static gy_run_t gy_curr_row_runs[V10_GY_MAX_RUNS]; /* 当前行 runs (扩展用) */

/* |Gy| 直方图缓冲 */
static uint32_t gy_hist[1021];

/* 最小二乘累加器 */
typedef struct {
    float    sum_x, sum_y, sum_xy, sum_x2;
    uint16_t num_points;
} gy_lsq_acc_t;


/* ==========================================================================
 * 内部辅助: 直方图法计算 |Gy| 分位数阈值
 * ========================================================================== */
static float gy_percentile_threshold(const int16_t *data, uint32_t n, uint32_t pct)
{
    uint32_t i, cum;
    int16_t  v;

    memset(gy_hist, 0, sizeof(gy_hist));
    for (i = 0; i < n; i++) {
        v = data[i];
        if (v < 0) v = (int16_t)(-v);
        if (v > 1020) v = 1020;
        gy_hist[(uint32_t)v]++;
    }

    cum = 0;
    for (i = 0; i <= 1020; i++) {
        cum += gy_hist[i];
        if (cum >= (uint32_t)((uint64_t)n * pct / 100)) {
            return (float)i;
        }
    }
    return 1020.0f;
}


/* ==========================================================================
 * 内部辅助: 在指定行提取 |Gy| > threshold 的 runs
 * 返回 run 数量, -1 表示溢出
 * ========================================================================== */
static int16_t extract_row_runs(
    const int16_t *row_ptr,
    uint32_t       cols,
    float          threshold,
    gy_run_t      *runs_out,
    int16_t        max_runs)
{
    int16_t count = 0;
    uint32_t c;
    int32_t  thr_i = (int32_t)threshold;

    for (c = 0; c < cols; c++) {
        int16_t v = row_ptr[c];
        int16_t mag = (v < 0) ? (int16_t)(-v) : v;
        if ((int32_t)mag <= thr_i) continue;

        uint8_t start_x = (uint8_t)c;
        while (c + 1 < cols) {
            int16_t v2 = row_ptr[c + 1];
            int16_t m2 = (v2 < 0) ? (int16_t)(-v2) : v2;
            if ((int32_t)m2 <= thr_i) break;
            c++;
        }
        uint8_t end_x = (uint8_t)c;

        /* 与前一个 run 间隙 ≤ 1px → 合并 */
        if (count > 0 && start_x == (uint8_t)(runs_out[count - 1].end_x + 2u)) {
            runs_out[count - 1].end_x = end_x;
        } else if (count < max_runs) {
            runs_out[count].start_x = start_x;
            runs_out[count].end_x   = end_x;
            count++;
        } else {
            return -1;  /* 溢出 */
        }
    }
    return count;
}


/* ==========================================================================
 * 内部辅助: 两个 run 列表匹配——有 x 范围重叠即匹配, 合并到 dst
 * ========================================================================== */
static uint8_t merge_overlapping_runs(
    gy_run_t *dst, int16_t *dst_count, int16_t max_dst,
    const gy_run_t *src, int16_t src_count)
{
    uint8_t any_match = 0;
    int16_t i, j;

    for (i = 0; i < src_count; i++) {
        uint8_t s_start = src[i].start_x;
        uint8_t s_end   = src[i].end_x;
        uint8_t matched  = 0;

        for (j = 0; j < *dst_count; j++) {
            if (s_end < dst[j].start_x || dst[j].end_x < s_start) continue;

            if (s_start < dst[j].start_x) dst[j].start_x = s_start;
            if (s_end   > dst[j].end_x)   dst[j].end_x   = s_end;
            matched = 1;
            any_match = 1;
        }

        if (!matched && *dst_count < max_dst) {
            dst[*dst_count] = src[i];
            (*dst_count)++;
            any_match = 1;
        }
    }
    return any_match;
}


/* ==========================================================================
 * 内部辅助: 计算单行 |Gy| 最大值, 用于行自适应阈值
 * ========================================================================== */
static int16_t row_max_abs(const int16_t *row_ptr, uint32_t cols)
{
    int16_t max_val = 0;
    uint32_t c;
    for (c = 0; c < cols; c++) {
        int16_t v = row_ptr[c];
        if (v < 0) v = (int16_t)(-v);
        if (v > max_val) max_val = v;
    }
    return max_val;
}


static uint8_t compute_peak_midpoint(
    const int16_t *p_gy,
    uint32_t       rows,
    uint32_t       cols,
    int16_t        peak_row,
    float         *p_mid_x,
    float         *p_span)
{
    float    threshold;
    int16_t  count, k;
    uint8_t  x_min, x_max;

    if (peak_row < 0 || (uint32_t)peak_row >= rows) return 0;

    /* 阈值由峰值行决定, 上下行共用 */
    {
        int16_t rmax = row_max_abs(&p_gy[(uint32_t)peak_row * cols], cols);
        threshold = (float)rmax * V10_GY_EDGE_RATIO;
        if (threshold < (float)V10_GY_MIN_ABS_THRESH)
            threshold = (float)V10_GY_MIN_ABS_THRESH;
    }

    /* 提取峰值行 runs */
    count = extract_row_runs(&p_gy[(uint32_t)peak_row * cols], cols,
                             threshold, gy_seed_runs, V10_GY_MAX_RUNS);
    if (count <= 0) return 0;

    /* 向上一行: 提取 + 垂直连通合并 */
    if (peak_row > 0) {
        int16_t above = extract_row_runs(
            &p_gy[(uint32_t)(peak_row - 1) * cols], cols,
            threshold, gy_curr_row_runs, V10_GY_MAX_RUNS);
        if (above > 0)
            merge_overlapping_runs(gy_seed_runs, &count, V10_GY_MAX_RUNS,
                                   gy_curr_row_runs, above);
    }

    /* 向下一行: 提取 + 垂直连通合并 */
    if ((uint32_t)(peak_row + 1) < rows) {
        int16_t below = extract_row_runs(
            &p_gy[(uint32_t)(peak_row + 1) * cols], cols,
            threshold, gy_curr_row_runs, V10_GY_MAX_RUNS);
        if (below > 0)
            merge_overlapping_runs(gy_seed_runs, &count, V10_GY_MAX_RUNS,
                                   gy_curr_row_runs, below);
    }

    /* 过滤宽度<3, 计算包围盒 */
    x_min = 255; x_max = 0;
    {
        int16_t orig = count;
        count = 0;
        for (k = 0; k < orig; k++) {
            uint8_t sx = gy_seed_runs[k].start_x;
            uint8_t ex = gy_seed_runs[k].end_x;
            if ((uint8_t)(ex - sx + 1u) < 3u) continue;
            if (sx < x_min) x_min = sx;
            if (ex > x_max) x_max = ex;
            count++;
        }
    }
    if (count == 0) return 0;

    *p_mid_x = (float)(x_min + x_max) * 0.5f;
    *p_span  = (float)(x_max - x_min + 1);
    return 1;
}


/* ==========================================================================
 * fit_gy_edges  ——  双峰中点提取
 * ========================================================================== */
void fit_gy_edges(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    int16_t           peak_y,
    v10_stair_result_t *p_result)
{
    float span1, span2;
    (void)rows;

    p_result->mid_x           = 0.0f;
    p_result->mid_y           = 0.0f;
    p_result->mid2_x          = 0.0f;
    p_result->mid2_y          = 0.0f;
    p_result->edge_span       = 0.0f;
    p_result->num_edge_points = 0;

    if (peak_y < 0 || (uint32_t)peak_y >= rows) return;

    if (!compute_peak_midpoint(p_gy, rows, cols, peak_y,
                               &p_result->mid_x, &span1))
        return;
    p_result->mid_y     = (float)peak_y;
    p_result->edge_span = span1;
    p_result->num_edge_points = 1;

    if (compute_peak_midpoint(p_gy, rows, cols, p_result->peak2_y,
                              &p_result->mid2_x, &span2)) {
        p_result->mid2_y = (float)p_result->peak2_y;
        if (span2 > p_result->edge_span) p_result->edge_span = span2;
        p_result->num_edge_points = 2;
    }
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
    float    P[V10_GY_COLS];         /* |Gx| 列总和 */
    float    gy_prof[V10_GY_ROWS];   /* Gy 行均值剖面 */
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
 * detect_crease  ——  Crease 检测
 * ========================================================================== */
void detect_crease(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v10_stair_result_t *p_result)
{
    float    gy_prof[V10_GY_ROWS];
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
    extremum_t pos_peaks[V10_GY_ROWS], neg_peaks[V10_GY_ROWS];
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
        for (i = 1; i < num_pos; i++) {
            extremum_t key = pos_peaks[i];
            j = i;
            while (j > 0 && pos_peaks[j-1].val < key.val) {
                pos_peaks[j] = pos_peaks[j-1];
                j--;
            }
            pos_peaks[j] = key;
        }
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
            if (fabsf(chosen[i].val) >= max_abs * V10_WEAK_RATIO) {
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
        uint32_t peak1_idx;
        uint32_t peak2_idx;
    } candidate_t;
    candidate_t candidates[30];
    uint32_t    num_cand = 0;

    {
        uint32_t top_n = (num_chosen < (uint32_t)V10_TOP_N) ? num_chosen : (uint32_t)V10_TOP_N;
        uint32_t k, m;

        for (k = 0; k < top_n; k++) {
            for (m = k + 1; m < top_n; m++) {
                uint32_t i1 = chosen[k].idx;
                uint32_t i2 = chosen[m].idx;
                if (i1 > i2) { uint32_t tmp = i1; i1 = i2; i2 = tmp; }
                uint32_t span = i2 - i1;

                if (span <= (uint32_t)V10_SPAN_MAX) {
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

                    float weaker_peak = fabsf(chosen[k].val);
                    if (fabsf(chosen[m].val) < weaker_peak)
                        weaker_peak = fabsf(chosen[m].val);

                    if (min_abs < weaker_peak - (float)V10_VALLEY_MARGIN) {
                        if (num_cand < 30) {
                            candidates[num_cand].y         = crease_idx;
                            candidates[num_cand].span      = span;
                            candidates[num_cand].bottom_y  = (i1 > i2) ? i1 : i2;
                            candidates[num_cand].peak1_idx = k;
                            candidates[num_cand].peak2_idx = m;
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
        p_result->peak_y      = -1;
        p_result->peak2_y     = -1;
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
            if ((uint32_t)diff >= (uint32_t)V10_CANDIDATE_GAP) {
                valid[num_valid++] = candidates[i];
            }
        }

        p_result->crease_y    = (int16_t)valid[0].y;
        p_result->crease_span = (int16_t)valid[0].span;

        /* 输出双峰行号: 行坐标小的为上峰 (peak_y), 大的为下峰 (peak2_y) */
        {
            uint32_t r1 = chosen[valid[0].peak1_idx].idx;
            uint32_t r2 = chosen[valid[0].peak2_idx].idx;
            if (r1 <= r2) {
                p_result->peak_y  = (int16_t)r1;
                p_result->peak2_y = (int16_t)r2;
            } else {
                p_result->peak_y  = (int16_t)r2;
                p_result->peak2_y = (int16_t)r1;
            }
        }
    }
}


/* ==========================================================================
 * find_gy_peak  ——  查找 |Gy| 全局最大峰值
 * ========================================================================== */
void find_gy_peak(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v10_stair_result_t *p_result)
{
    uint32_t r, c;
    int32_t  max_val = 0;
    int16_t  max_x = 0, max_y = 0;

    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            int16_t v = p_gy[r * cols + c];
            int32_t av = (v < 0) ? -(int32_t)v : (int32_t)v;
            if (av > max_val) {
                max_val = av;
                max_x   = (int16_t)c;
                max_y   = (int16_t)r;
            }
        }
    }

    p_result->gy_max_x   = max_x;
    p_result->gy_max_y   = max_y;
    p_result->gy_max_val = max_val;
}


/* ==========================================================================
 * v10_stair_process_full  ——  完整 V10 后处理管线
 *
 * 无门控: 始终检测 crease + 中点; has_stairs = fit_gy_edges 成功
 * ========================================================================== */
void v10_stair_process_full(
    const int16_t    *p_gx,
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v10_stair_result_t *p_result)
{
    /* 清零结果 */
    memset(p_result, 0, sizeof(*p_result));
    p_result->crease_y = -1;
    p_result->peak_y   = -1;
    p_result->peak2_y  = -1;

    /* joint_score 保留供参考 */
    p_result->joint_score = stair_discriminate(p_gx, p_gy, rows, cols, V10_GY_VAR_START_ROW);

    /* Step 1: Crease 检测 */
    detect_crease(p_gy, rows, cols, p_result);

    /* Step 2: 中点提取 — 成功则 has_stairs=1 */
    if (p_result->peak_y >= 0) {
        fit_gy_edges(p_gy, rows, cols, p_result->peak_y, p_result);
        if (p_result->num_edge_points > 0) {
            p_result->has_stairs = 1;
        }
    }
}
