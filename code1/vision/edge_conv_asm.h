/**
 * ============================================================================
 * edge_conv_asm.h  ——  4x4 边缘卷积汇编算子接口
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
 * 所有函数由 edge_conv_asm.c 实现，通过 ITCM_FUNC 置于 SELF_ITCM
 *
 * 图像规格: uint8 灰度 188x120
 * 输出: 185x117 Gx, Gy (int16)
 * ============================================================================
 */

#ifndef _EDGE_CONV_ASM_H_
#define _EDGE_CONV_ASM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ==========================================================================
 * 边缘方向分析结果 (极简版)
 *   sum_gx, sum_gy: 强边缘 signed Gx/Gy 累加 (int32, 不溢出: max 21645*4080=88M<2^31)
 *   strong_count: 强边缘像素数
 *   total_pixels: 总有效像素数
 *
 * C 端计算:
 *   angle_dev = atan2f(sum_gy, sum_gx) → 偏离角
 *   r_squared = (sum_gx²+sum_gy²) / (strong_count·E[|G|²])  近似一致性
 * ========================================================================== */
typedef struct {
    int32_t  sum_gx;         /* ΣGx (signed), strong edge pixels only       */
    int32_t  sum_gy;         /* ΣGy (signed)                               */
    uint32_t strong_count;   /* 强边缘像素数                                */
    uint32_t total_pixels;   /* 总有效像素数                                */
} edge_dir_result_t;


/* ==========================================================================
 * conv1d_horiz_gxgy  ——  水平 Pass: 一行 → Gx_horiz + Gy_horiz
 * ==========================================================================
 * 对输入行做 4 元素 1D 卷积, Gx(deriv) 和 Gy(smooth) 同时计算, SMLAD 双发射.
 * 卷积核 D=[-1,-1,1,1], S=[1,3,3,1] 全部常驻寄存器 (MOVW+MOVT).
 *
 * p_input:  一行 188 个 int16 (uint8 零扩展, 连续两像素打包为 {p[x+1],p[x]})
 * p_gx_out: Gx_horiz 输出, 185 个 int16
 * p_gy_out: Gy_horiz 输出, 185 个 int16
 * out_width: 185
 * ========================================================================== */
void conv1d_horiz_gxgy(
    const int16_t *p_input,
    int16_t       *p_gx_out,
    int16_t       *p_gy_out,
    uint32_t       out_width);


/* ==========================================================================
 * conv1d_vert_gxgy_row  ——  垂直 Pass: 4 行 → 1 行 Gx + 1 行 Gy
 * ==========================================================================
 * 从 4 行水平中间结果 (int16 Gx_horiz, Gy_horiz) 做垂直方向 4 元素 1D 卷积.
 * LDRSH 逐列加载 + PKHBT 打包, Gx(smooth) 和 Gy(deriv) 同时计算, SMLAD 双发射.
 *
 * p_gx_h0..h3: Gx_horiz 的 4 个行指针 (行 r, r+1, r+2, r+3), 各 185 个 int16
 * p_gy_h0..h3: Gy_horiz 的 4 个行指针
 * p_gx_out:    Gx 输出, 185 个 int16
 * p_gy_out:    Gy 输出, 185 个 int16
 * out_width:   185
 *
 * 注意: 调用者保证 4 个行指针指向有效数据 (由环形缓冲管理).
 * ========================================================================== */
void conv1d_vert_gxgy_row(
    const int16_t *p_gx_h0,
    const int16_t *p_gx_h1,
    const int16_t *p_gx_h2,
    const int16_t *p_gx_h3,
    const int16_t *p_gy_h0,
    const int16_t *p_gy_h1,
    const int16_t *p_gy_h2,
    const int16_t *p_gy_h3,
    int16_t       *p_gx_out,
    int16_t       *p_gy_out,
    uint32_t       out_width);


/* ==========================================================================
 * gradient_mag_dir_fixed  ——  幅值 + 固定阈值 + signed Gx/Gy 累加
 * ==========================================================================
 * 从 Gx, Gy (int16 打包对 {Gy, Gx}) 计算幅值, 与固定阈值比较,
 * 强边缘点累加 signed Gx, Gy.
 *
 * p_gx:  Gx[117][185] int16, 行优先
 * p_gy:  Gy[117][185] int16, 行优先
 * p_out: 结果 (累加值在结构体内)
 * fixed_thr:   固定幅值阈值 (L1: |Gx|+|Gy| >= thr)
 * total_pixels: 185 * 117
 * ========================================================================== */
void gradient_mag_dir_fixed(
    const int16_t    *p_gx,
    const int16_t    *p_gy,
    edge_dir_result_t *p_out,
    int32_t           fixed_thr,
    uint32_t          total_pixels);


#ifdef __cplusplus
}
#endif

#endif /* _EDGE_CONV_ASM_H_ */
