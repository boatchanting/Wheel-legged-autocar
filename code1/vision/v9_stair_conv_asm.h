/**
 * ============================================================================
 * v9_stair_conv_asm.h  ——  V9 台阶检测汇编算子接口
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
 * 所有函数由 v9_stair_conv_asm.s 实现, 置于 SELF_ITCM (0x00000000)
 *
 * 图像规格: uint8 灰度 60×94 (行×列)
 * Gx 核: 2×4 Box-Diff  [[-1,-1,1,1],[-1,-1,1,1]]  → 输出 59×91 int16
 * Gy 核: 4×2 Box-Diff  [[-1,-1],[-1,-1],[1,1],[1,1]]  → 输出 57×91 int16 (裁剪到91列)
 * ============================================================================
 */

#ifndef _V9_STAIR_CONV_ASM_H_
#define _V9_STAIR_CONV_ASM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ==========================================================================
 * 台阶检测完整结果 (与 V9_PSEUDOCODE.md §五 对齐)
 * ========================================================================== */
typedef struct {
    uint8_t  has_stairs;         /* 是否有台阶                          */
    float    joint_score;        /* gx_score × gy_var 联合判别分数       */
    /* 边界线 (Hough 参数, ρ 单位像素, θ 单位弧度) */
    float    left_rho;
    float    left_theta;
    float    right_rho;
    float    right_theta;
    /* 中线 (角平分线, ax+by+c=0 归一化) */
    float    center_a;
    float    center_b;
    float    center_c;
    /* Crease */
    int16_t  crease_y;           /* crease 行号 (0=图像顶部)             */
    int16_t  crease_span;        /* 双峰间距 (px)                       */
} v9_stair_result_t;


/* ==========================================================================
 * v9_conv_gx_row  ——  Gx 2×4 Box-Diff 卷积: 2 行 → 1 行
 * ==========================================================================
 * 对连续两行输入做 2×4 卷积, SMLAD 双发射。
 * 核: [[-1,-1,1,1],[-1,-1,1,1]] 全部常驻寄存器 (MOVW+MOVT)。
 *
 * p_row0:  第 y   行, 94 个 int16 (uint8 零扩展打包)
 * p_row1:  第 y+1 行, 94 个 int16
 * p_out:   Gx 输出, 91 个 int16
 * out_width: 91
 *
 * 值域: 输出 ∈ [-1020, +1020]  ✅ int16 安全
 * ========================================================================== */
void v9_conv_gx_row(
    const int16_t *p_row0,
    const int16_t *p_row1,
    int16_t       *p_out,
    uint32_t       out_width);


/* ==========================================================================
 * v9_conv_gy_row  ——  Gy 4×2 Box-Diff 卷积: 4 行 → 1 行
 * ==========================================================================
 * 对连续四行输入做 4×2 卷积, SMLAD 双发射。
 * 核: [[-1,-1],[-1,-1],[1,1],[1,1]] 全部常驻寄存器。
 *
 * p_row0..p_row3: 第 y..y+3 行, 各 94 个 int16
 * p_out:   Gy 输出, 91 个 int16 (裁剪到 V9 后处理所需宽度)
 * out_width: 91
 *
 * 值域: 输出 ∈ [-1020, +1020]  ✅ int16 安全
 * ========================================================================== */
void v9_conv_gy_row(
    const int16_t *p_row0,
    const int16_t *p_row1,
    const int16_t *p_row2,
    const int16_t *p_row3,
    int16_t       *p_out,
    uint32_t       out_width);


#ifdef __cplusplus
}
#endif

#endif /* _V9_STAIR_CONV_ASM_H_ */
