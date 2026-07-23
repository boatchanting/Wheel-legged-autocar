/**
 * ============================================================================
 * v9_stair_postprocess.h  ——  V9 台阶检测后处理接口
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
 * ============================================================================
 * 实现于 v9_stair_postprocess.c
 *
 * 三阶段后处理:
 *   1. stair_discriminate  — 台阶/背景判别 (joint score)
 *   2. hough_boundary      — Hough 变换 → 左右边界线 + 中线
 *   3. detect_crease       — Gy 剖面双峰配对 → crease 位置
 * ============================================================================
 */

#ifndef _V9_STAIR_POSTPROCESS_H_
#define _V9_STAIR_POSTPROCESS_H_

#include <stdint.h>
#include "v9_stair_conv_asm.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ==========================================================================
 * 参数常量 (来自 V9_PSEUDOCODE.md §四)
 * ========================================================================== */
#define V9_SPAN_MAX         20          /* 双峰最大间距 (px)            */
#define V9_WEAK_RATIO       0.25f       /* 弱极值过滤比例              */
#define V9_VALLEY_MARGIN    10          /* 谷点验证阈值                */
#define V9_TOP_N            6           /* 最多考虑前 N 个极值配对     */
#define V9_MAX_CANDIDATES   2           /* 最多保留候选组数            */
#define V9_CANDIDATE_GAP    10          /* 候选组间最小间隔 (px)       */
#define V9_JOINT_THRESHOLD  50000000.0f /* 台阶/背景判别阈值           */
#define V9_HOUGH_PERCENTILE 80          /* Hough 边缘点分位数          */
#define V9_HOUGH_THETA_MIN  (-35.0f)    /* θ 范围下限 (度)             */
#define V9_HOUGH_THETA_MAX  (+35.0f)    /* θ 范围上限 (度)             */

/* Hough 参数空间尺寸 */
#define V9_HOUGH_NUM_THETA  31          /* -35°~+35°, 步长 ~2.33° (从61缩减以省SRAM) */
#define V9_HOUGH_RHO_MAX    110         /* diag ≈ sqrt(91²+57²) ≈ 107 */

/* gy_var 只取底部行 (台阶下半部才有横纹结构) */
#define V9_GY_VAR_START_ROW 30          /* gy_var 起始行 (0=顶部)      */


/* ==========================================================================
 * stair_discriminate  ——  台阶/背景判别
 * ==========================================================================
 * 输入:
 *   p_gx:       Gx[57][91] int16, 行优先
 *   p_gy:       Gy[57][91] int16, 行优先
 *   rows:       57
 *   cols:       91
 *
 * 输出:
 *   返回 joint_score (gx_score × gy_var), > V9_JOINT_THRESHOLD 判为台阶
 *
 * 算法:
 *   P[x] = Σ_y |Gx[y][x]|
 *   gx_score = var(P)² / ΣP
 *   gy_prof[y] = mean(Gy[y][:])
 *   gy_var = var(gy_prof[gy_start_row .. rows-1])   ← 仅底部行
 *   joint = gx_score × gy_var
 * ========================================================================== */
float stair_discriminate(
    const int16_t *p_gx,
    const int16_t *p_gy,
    uint32_t       rows,
    uint32_t       cols,
    uint32_t       gy_var_start_row);


/* ==========================================================================
 * hough_boundary  ——  Hough 变换边界检测
 * ==========================================================================
 * 输入:
 *   p_gx:       Gx[57][91] int16 (signed), 行优先
 *   rows:       57
 *   cols:       91
 *
 * 输出 (写入 p_result):
 *   left_rho, left_theta, right_rho, right_theta
 *   center_a, center_b, center_c
 *
 * 算法见 V9_PSEUDOCODE.md §三
 * ========================================================================== */
void hough_boundary(
    const int16_t    *p_gx,
    uint32_t          rows,
    uint32_t          cols,
    v9_stair_result_t *p_result);


/* ==========================================================================
 * detect_crease  ——  Crease 检测
 * ==========================================================================
 * 输入:
 *   p_gy:       Gy[57][91] int16, 行优先
 *   rows:       57
 *   cols:       91
 *
 * 输出 (写入 p_result):
 *   crease_y, crease_span
 *
 * 算法见 V9_PSEUDOCODE.md §二
 * ========================================================================== */
void detect_crease(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v9_stair_result_t *p_result);


/* ==========================================================================
 * v9_stair_process_full  ——  完整 V9 后处理管线
 * ==========================================================================
 * 一站式调用: stair_discriminate → (if stairs) hough_boundary + detect_crease
 *
 * p_gx, p_gy: 已裁剪到共同尺寸 57×91 的 Gx, Gy (int16, 行优先)
 * ========================================================================== */
void v9_stair_process_full(
    const int16_t    *p_gx,
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v9_stair_result_t *p_result);


#ifdef __cplusplus
}
#endif

#endif /* _V9_STAIR_POSTPROCESS_H_ */
