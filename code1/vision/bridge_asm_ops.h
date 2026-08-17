/**
 * ============================================================================
 * bridge_asm_ops.h  ——  单边桥检测汇编算子接口
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
 * 所有函数由 bridge_asm_ops.c 实现，通过 ITCM_FUNC 置于 SELF_ITCM
 *
 * 由原汇编算子合并精简为 C 实现:
 * 仅保留单边桥检测需要的 4x4 可分离边缘卷积 (lock-x / lock-y 梯度)。
 * ============================================================================
 */

#ifndef _BRIDGE_ASM_OPS_H_
#define _BRIDGE_ASM_OPS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ⚠️ C26 (2026-08-08): 两个卷积函数已加 b2_ 前缀重命名, 避免与主工程
 *    edge_conv_asm.h 的同名导出符号冲突 (二者核方向相反, 不可混用). */

/* ==========================================================================
 * b2_conv1d_horiz_gxgy  ——  水平 Pass: 一行 → Gx_horiz + Gy_horiz
 * ==========================================================================
 * 对输入行做 4 元素 1D 卷积, Gx(deriv) 和 Gy(smooth) 同时计算, SMLAD 双发射.
 * 卷积核 D=[-1,-1,1,1], S=[1,3,3,1] 全部常驻寄存器 (MOVW+MOVT).
 *
 * p_input:  一行 out_width+3 个 int16 (uint8→int16 零扩展), 必须 4 字节对齐
 * p_gx_out: Gx_horiz 输出, out_width 个 int16
 * p_gy_out: Gy_horiz 输出, out_width 个 int16
 * out_width: 188 宽 -> 185 / 94 宽 -> 91
 * ========================================================================== */
void b2_conv1d_horiz_gxgy(
    const int16_t *p_input,
    int16_t       *p_gx_out,
    int16_t       *p_gy_out,
    uint32_t       out_width);


/* ==========================================================================
 * b2_conv1d_vert_gxgy_row  ——  垂直 Pass: 4 行 → 1 行 Gx + 1 行 Gy
 * ==========================================================================
 * 从 4 行水平中间结果 (int16 Gx_horiz, Gy_horiz) 做垂直方向 4 元素 1D 卷积.
 * LDRSH 逐列加载 + PKHBT 打包, Gx(smooth) 和 Gy(deriv) 同时计算, SMLAD 双发射.
 *
 * p_gx_h0..h3: Gx_horiz 的 4 个行指针 (行 r, r+1, r+2, r+3), 各 out_width 个
 * p_gy_h0..h3: Gy_horiz 的 4 个行指针
 * p_gx_out:    Gx 输出, out_width 个 int16
 * p_gy_out:    Gy 输出, out_width 个 int16
 * out_width:   与水平 Pass 一致
 *
 * 注意: 调用者保证 4 个行指针指向有效数据 (由环形缓冲管理).
 * ========================================================================== */
void b2_conv1d_vert_gxgy_row(
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
 * mlp_fc_s8_layer  ——  单层 int8 全连接, SMLAD 双发射 (CMSIS-NN 风格)
 * ==========================================================================
 * 权重须由导出脚本预打包 (mlp_w0p/w1p/w2p): 每 int32 含 2 个 int16 半字,
 * 每组 4 输入 -> 2 个 int32: {w[4k+2]|w[4k+0]}, {w[4k+3]|w[4k+1]}.
 * 输入必须 pad 到 4 倍数且 4 字节对齐.
 *
 * p_in:   输入 int8, n_in_pad 字节 (4 对齐)
 * p_w:    SMLAD 打包权重, n_out * (n_in_pad/2) 个 int32
 * p_b:    n_out 偏置 int32
 * p_mul:  n_out requant 乘数 int32
 * p_shf:  n_out 右移量 int32 (当前模型恒 30)
 * p_out:  n_out 输出 int8
 * n_groups: n_in_pad/4
 * n_out:  输出通道数
 * relu:   0 对称 clip[-128,127]; 1 ReLU clip[0,127]
 * ========================================================================== */
void mlp_fc_s8_layer(
    const int8_t  *p_in,
    const int32_t *p_w,
    const int32_t *p_b,
    const int32_t *p_mul,
    const int32_t *p_shf,
    int8_t        *p_out,
    uint32_t       n_groups,
    uint32_t       n_out,
    uint32_t       relu);


#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_ASM_OPS_H_ */
