/**
 * ============================================================================
 * v10_stair_postprocess.h  ——  V10 台阶检测后处理接口
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
 * 实现于 v10_stair_postprocess.c
 *
 * 三阶段后处理:
 *   1. stair_discriminate  — 台阶/背景判别 (joint score)
 *   2. fit_gy_edges        — Gy RLE-CCL 聚类 + 最小二乘直线拟合 → 中点/倾角/跨度
 *   3. detect_crease       — Gy 剖面双峰配对 → crease 位置
 * ============================================================================
 */

#ifndef _V10_STAIR_POSTPROCESS_H_
#define _V10_STAIR_POSTPROCESS_H_

#include <stdint.h>
#include "v10_stair_conv_asm.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ==========================================================================
 * 图像尺寸 (120×188 管线原生分辨率, MT9V03X_W=188, MT9V03X_H=120)
 * ========================================================================== */
#define V10_IMG_ROWS         120         /* 输入图像行数 (高度), =MT9V03X_H  */
#define V10_IMG_COLS         188         /* 输入图像列数 (宽度), =MT9V03X_W  */
#define V10_GX_ROWS          (V10_IMG_ROWS - 1)  /* Gx 输出: 119 行       */
#define V10_GX_COLS          (V10_IMG_COLS - 3)  /* Gx 输出: 185 列       */
#define V10_GY_ROWS          (V10_IMG_ROWS - 3)  /* Gy 输出: 117 行       */
#define V10_GY_COLS          (V10_IMG_COLS - 3)  /* Gy 输出: 185 列       */

/* ==========================================================================
 * 参数常量 (来自 V10_PSEUDOCODE.md §四)
 * ========================================================================== */
#define V10_SPAN_MAX         20          /* 双峰最大间距 (px)            */
#define V10_WEAK_RATIO       0.25f       /* 弱极值过滤比例              */
#define V10_VALLEY_MARGIN    10          /* 谷点验证阈值                */
#define V10_TOP_N            6           /* 最多考虑前 N 个极值配对     */
#define V10_MAX_CANDIDATES   2           /* 最多保留候选组数            */
#define V10_CANDIDATE_GAP    10          /* 候选组间最小间隔 (px)       */
#define V10_JOINT_THRESHOLD  50000000.0f /* 台阶/背景判别阈值           */

/* Gy 边缘提取与直线拟合 (峰值行种子 + RLE 扩展) */
#define V10_GY_EDGE_RATIO        0.30f    /* 阈值 = 峰值行 max|Gy| × ratio (可调) */
#define V10_GY_MIN_ABS_THRESH    5        /* |Gy| 绝对值下限, 防全黑误检      */
#define V10_GY_MAX_EXPAND_ROWS   3        /* 从峰值行向上/下最大扩展行数        */
#define V10_GY_MIN_SPAN          6        /* 线段最小跨度 (px), 防噪声误检    */
#define V10_GY_MAX_RESIDUAL      2.0f     /* 最大平均残差 (px), 超此视为噪声   */

/* RLE 游程缓冲 (仅用于峰值行扩展) */
#define V10_GY_MAX_RUNS          60       /* 单行最大 run 数                  */

/* gy_var 只取底部行 (台阶下半部才有横纹结构) */
#define V10_GY_VAR_START_ROW 61           /* gy_var 起始行 (~下半部, 117*0.52≈61) */


/* ==========================================================================
 * stair_discriminate  ——  台阶/背景判别
 * ==========================================================================
 * 输入:
 *   p_gx:       Gx[117][185] int16, 行优先
 *   p_gy:       Gy[117][185] int16, 行优先
 *   rows:       117
 *   cols:       185
 *
 * 输出:
 *   返回 joint_score (gx_score × gy_var), 仅供参考, 不参与 has_stairs 判定
 * ========================================================================== */
float stair_discriminate(
    const int16_t *p_gx,
    const int16_t *p_gy,
    uint32_t       rows,
    uint32_t       cols,
    uint32_t       gy_var_start_row);


/* ==========================================================================
 * fit_gy_edges  ——  Gy 横线直线拟合
 * ==========================================================================
 * 输入:
 *   p_gy:       Gy[rows][cols] int16, 行优先
 *   rows, cols: Gy 尺寸 (117×185)
 *   peak_y:     detect_crease 输出的上峰行号
 *
 * 输出 (写入 p_result):
 *   mid_x, mid_y, mid2_x, mid2_y, edge_span, num_edge_points
 * ========================================================================== */
void fit_gy_edges(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    int16_t           peak_y,
    v10_stair_result_t *p_result);


/* ==========================================================================
 * detect_crease  ——  Crease 检测
 * ==========================================================================
 * 输入:
 *   p_gy:       Gy[117][185] int16, 行优先
 *   rows:       117
 *   cols:       185
 *
 * 输出 (写入 p_result):
 *   crease_y, crease_span, peak_y (上峰), peak2_y (下峰)
 * ========================================================================== */
void detect_crease(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v10_stair_result_t *p_result);


/* ==========================================================================
 * find_gy_peak  ——  查找 |Gy| 全局最大峰值
 * ==========================================================================
 * 输入:
 *   p_gy:       Gy[117][185] int16, 行优先
 *   rows, cols: Gy 尺寸 (117×185)
 *
 * 输出 (写入 p_result):
 *   gy_max_x, gy_max_y, gy_max_val — |Gy| 最大值的坐标和绝对值
 * ========================================================================== */
void find_gy_peak(
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v10_stair_result_t *p_result);


/* ==========================================================================
 * v10_stair_process_full  ——  完整 V10 后处理管线
 * ==========================================================================
 * 一站式调用: stair_discriminate → detect_crease → fit_gy_edges
 *
 * p_gx, p_gy: 已裁剪到共同尺寸 117×185 的 Gx, Gy (int16, 行优先)
 * ========================================================================== */
void v10_stair_process_full(
    const int16_t    *p_gx,
    const int16_t    *p_gy,
    uint32_t          rows,
    uint32_t          cols,
    v10_stair_result_t *p_result);


#ifdef __cplusplus
}
#endif

#endif /* _V10_STAIR_POSTPROCESS_H_ */
