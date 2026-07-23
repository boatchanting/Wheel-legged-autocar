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
 * 三阶段后处理 (输入: Gx[117][185] + Gy[117][185] int16):
 *   1. stair_discriminate  — 台阶/背景判别 (joint score)
 *   2. detect_crease       — Gy 剖面双峰配对 → 折痕 + 双峰行号
 *   3. fit_gy_edges        — 上峰 RLE 提取 → 两个中间点 x 坐标
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
 * 参数常量
 * ========================================================================== */
#define V9_SPAN_MAX         40          /* 双峰最大间距 (px), 全尺寸放宽       */
#define V9_WEAK_RATIO       0.25f       /* 弱极值过滤比例                       */
#define V9_VALLEY_MARGIN    10          /* 谷点验证阈值                         */
#define V9_TOP_N            6           /* 最多考虑前 N 个极值配对              */
#define V9_MAX_CANDIDATES   2           /* 最多保留候选组数                     */
#define V9_CANDIDATE_GAP    20          /* 候选组间最小间隔 (px), 全尺寸放宽    */
#define V9_JOINT_THRESHOLD  50000000.0f /* 台阶/背景判别阈值                    */

/* Gy 边缘提取 (峰值行种子 + RLE 扩展) */
#define V9_GY_EDGE_RATIO        0.08f   /* 阈值 = 峰值行 max|Gy| × ratio        */
#define V9_GY_MIN_ABS_THRESH    5       /* |Gy| 绝对值下限, 防全黑误检           */
#define V9_GY_MAX_EXPAND_ROWS   3       /* 从峰值行向上/下最大扩展行数           */
#define V9_GY_MIN_SPAN          6       /* 线段最小跨度 (px), 防噪声误检         */

/* RLE 游程缓冲 (仅用于峰值行扩展) */
#define V9_GY_MAX_RUNS          80      /* 单行最大 run 数, 全尺寸放宽           */

/* gy_var 只取底部行 (台阶下半部才有横纹结构) */
#define V9_GY_VAR_START_ROW     60      /* gy_var 起始行 (0=顶部)               */


/* ==========================================================================
 * stair_discriminate  ——  台阶/背景判别
 * ==========================================================================
 * 输入:
 *   p_gx:       Gx[rows][cols] int16, 行优先
 *   p_gy:       Gy[rows][cols] int16, 行优先
 *   rows:       Gy 行数 (117)
 *   cols:       Gy 列数 (185)
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
 * fit_gy_edges  ——  上峰横线双中点提取
 * ==========================================================================
 * 在上峰行提取 Gy 边缘 runs, 取左右两个最大 run 各自的中点。
 *
 * 输入:
 *   p_gy:          Gy[rows][cols] int16, 行优先
 *   rows, cols:    Gy 尺寸 (117×185)
 *   upper_peak_y:  上峰行号
 *
 * 输出 (写入 p_result):
 *   upper_mid1_x, upper_mid2_x, edge_span, num_edge_points
 * ========================================================================== */
void fit_gy_edges(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    int16_t           upper_peak_y,
    v9_stair_result_t *p_result);


/* ==========================================================================
 * detect_crease  ——  Crease + 双峰检测
 * ==========================================================================
 * 输入:
 *   p_gy:       Gy[rows][cols] int16 (可为 NULL, 此时用 gy_prof 参数)
 *   gy_prof:    Gy 行均值剖面 float[rows] (可为 NULL, 此时从 p_gy 计算)
 *               至少提供一个非 NULL 参数
 *   rows:       117
 *   cols:       185
 *
 * 输出 (写入 p_result):
 *   crease_y, crease_span, upper_peak_y, lower_peak_y
 * ========================================================================== */
void detect_crease(
    const int16_t    *p_gy,
    const float      *gy_prof,
    uint32_t          rows,
    uint32_t          cols,
    v9_stair_result_t *p_result);


/* ==========================================================================
 * v9_stair_process_full  ——  完整 V9 后处理管线
 * ==========================================================================
 * 一站式调用: stair_discriminate → detect_crease → fit_gy_edges
 *
 * p_gx, p_gy: 已裁剪到 117×185 的 Gx, Gy (int16, 行优先)
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
