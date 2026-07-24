/**
 * ============================================================================
 * v10_stair_conv_asm.h  ——  V10 台阶检测汇编算子接口
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
 * 平台:  Infineon CYT4BB7 (Cortex-M7, ARMv7E-M)
 * 所有函数由 v10_stair_conv_asm.s 实现, 置于 SELF_ITCM (0x00000000)
 *
 * 图像规格: uint8 灰度 120×188 (行×列)
 * Gx 核: 2×4 Box-Diff  [[-1,-1,1,1],[-1,-1,1,1]]  → 输出 119×185 int16
 * Gy 核: 4×4 二项式差分  1/16 * [[-1,-3,-3,-1],[-1,-3,-3,-1],[1,3,3,1],[1,3,3,1]]
 *   可分离: [-1,-1,1,1]^T ⊗ [1,3,3,1], 响应水平~45°倾斜边缘
 *   输出 117×185 int16
 * ============================================================================
 */

#ifndef _V10_STAIR_CONV_ASM_H_
#define _V10_STAIR_CONV_ASM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ==========================================================================
 * 台阶检测完整结果 (与 V10_PSEUDOCODE.md §五 对齐)
 * ========================================================================== */
typedef struct {
    uint8_t  has_stairs;         /* 是否有台阶                          */
    float    joint_score;        /* gx_score × gy_var 联合判别分数       */
    /* 横线中点 */
    float    mid_x;              /* 上峰中点 x (0~184)                  */
    float    mid_y;              /* 上峰中点 y (0~116)                  */
    float    mid2_x;             /* 下峰中点 x (0~184)                  */
    float    mid2_y;             /* 下峰中点 y (0~116)                  */
    float    edge_span;          /* 线段水平跨度 (px)                    */
    int16_t  num_edge_points;    /* 参与拟合的点数                       */
    /* Crease */
    int16_t  crease_y;           /* crease 行号 (0=图像顶部)             */
    int16_t  crease_span;        /* 双峰间距 (px)                       */
    int16_t  peak_y;             /* 上峰行号 (绝对值更大的)              */
    int16_t  peak2_y;            /* 下峰行号                            */
    /* |Gy| 全局最大峰值 (不考虑台阶结构, 纯最大梯度点)   */
    int16_t  gy_max_x;           /* |Gy| 最大值的列坐标 (Gy 空间)       */
    int16_t  gy_max_y;           /* |Gy| 最大值的行坐标 (Gy 空间)       */
    int32_t  gy_max_val;         /* |Gy| 最大绝对值                      */
} v10_stair_result_t;


/* ==========================================================================
 * v10_conv_gx_row  ——  Gx 2×4 Box-Diff 卷积: 2 行 → 1 行
 * ==========================================================================
 * 对连续两行输入做 2×4 卷积, SMLAD 双发射。
 * 核: [[-1,-1,1,1],[-1,-1,1,1]] 全部常驻寄存器 (MOVW+MOVT)。
 *
 * p_row0:  第 y   行, 188 个 int16 (uint8 零扩展打包)
 * p_row1:  第 y+1 行, 188 个 int16
 * p_out:   Gx 输出, 185 个 int16
 * out_width: 185
 *
 * 值域: 输出 ∈ [-1020, +1020]  ✅ int16 安全
 * ========================================================================== */
void v10_conv_gx_row(
    const int16_t *p_row0,
    const int16_t *p_row1,
    int16_t       *p_out,
    uint32_t       out_width);


/* ==========================================================================
 * v10_conv_gy_row  ——  Gy 4×4 二项式差分: 4 行 → 1 行
 * ==========================================================================
 * 对连续四行输入做 4×4 卷积, SMLAD 双发射。
 * 核: 1/16 * [[-1,-3,-3,-1],[-1,-3,-3,-1],[1,3,3,1],[1,3,3,1]]
 *
 * p_row0..p_row3: 第 y..y+3 行, 各 188 个 int16
 * p_out:   Gy 输出, 185 个 int16
 * out_width: 185
 *
 * 值域: 输出 ∈ [-8160, +8160]  ✅ int16 安全
 * ========================================================================== */
void v10_conv_gy_row(
    const int16_t *p_row0,
    const int16_t *p_row1,
    const int16_t *p_row2,
    const int16_t *p_row3,
    int16_t       *p_out,
    uint32_t       out_width);


#ifdef __cplusplus
}
#endif

#endif /* _V10_STAIR_CONV_ASM_H_ */
