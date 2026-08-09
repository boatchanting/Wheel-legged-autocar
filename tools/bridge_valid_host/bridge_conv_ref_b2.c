/**
 * bridge_conv_ref_b2.c —— b2_conv1d_horiz_gxgy / b2_conv1d_vert_gxgy_row 的可移植 C 参考实现
 * 语义逐位复刻 code1/vision/bridge_asm_ops.s (SMLAD 32 位精确累加 + STRH int16 截断):
 *   水平: Gx = D*P, D=[-1,-1,1,1];  Gy = S*P, S=[1,3,3,1]
 *   垂直: Gx = S 作用 4 行 gx_h;     Gy = D 作用 4 行 gy_h
 * 仅供 PC 宿主验证使用, 不进 MCU 工程。
 * (由 trials/pc_tools/bridge_conv_ref.c 改名为 b2_ 前缀, 与主工程 C26 符号名一致)
 */
#include "bridge_asm_ops.h"

/* -DCONV_S_3113: 模拟 bridge_asm_ops.s 修复前的半字装反行为 (S=[3,1,1,3]),
 * 用于复现 IAR/上机结果做对照。默认按文档预期语义 S=[1,3,3,1]。 */
#ifdef CONV_S_3113
#define S0 3
#define S1 1
#define S2 1
#define S3 3
#else
#define S0 1
#define S1 3
#define S2 3
#define S3 1
#endif

void b2_conv1d_horiz_gxgy(const int16_t *p_input,
                          int16_t *p_gx_out, int16_t *p_gy_out,
                          uint32_t out_width)
{
    uint32_t i;
    for (i = 0; i < out_width; i++) {
        int p0 = p_input[i], p1 = p_input[i + 1];
        int p2 = p_input[i + 2], p3 = p_input[i + 3];
        p_gx_out[i] = (int16_t)(-p0 - p1 + p2 + p3);
        p_gy_out[i] = (int16_t)(S0 * p0 + S1 * p1 + S2 * p2 + S3 * p3);
    }
}

void b2_conv1d_vert_gxgy_row(const int16_t *x0, const int16_t *x1,
                             const int16_t *x2, const int16_t *x3,
                             const int16_t *y0, const int16_t *y1,
                             const int16_t *y2, const int16_t *y3,
                             int16_t *p_gx_out, int16_t *p_gy_out,
                             uint32_t out_width)
{
    uint32_t j;
    for (j = 0; j < out_width; j++) {
        p_gx_out[j] = (int16_t)(S0 * x0[j] + S1 * x1[j] +
                                S2 * x2[j] + S3 * x3[j]);
        p_gy_out[j] = (int16_t)(-y0[j] - y1[j] + y2[j] + y3[j]);
    }
}
