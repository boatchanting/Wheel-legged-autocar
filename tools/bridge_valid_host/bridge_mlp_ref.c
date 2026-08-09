/**
 * bridge_mlp_ref.c —— mlp_fc_s8_layer 的可移植 C 参考实现
 * 逐位复刻 code1/vision/bridge_asm_ops.s (SMLAD 语义 + Q30 requant):
 *   acc = bias + Σ_{groups} (x0*w0 + x1*w1 + x2*w2 + x3*w3)   (int32 回绕)
 *   out = clip( (acc64 * mul) >> 30 )                          (int8)
 * 权重为 SMLAD 打包格式: pack[2g] = {w[4g+2] | w[4g+0]},
 *                        pack[2g+1] = {w[4g+3] | w[4g+1]}      (int16 半字, 有符号)
 * 仅供 PC 宿主验证使用, 不进 MCU 工程。
 * (由 trials/pc_tools/bridge_mlp_ref.c 移植; mlp_fc_s8_layer 未加 b2_ 前缀, 与主工程一致)
 */
#include "bridge_asm_ops.h"

void mlp_fc_s8_layer(const int8_t  *p_in,
                     const int32_t *p_w,
                     const int32_t *p_b,
                     const int32_t *p_mul,
                     const int32_t *p_shf,
                     int8_t        *p_out,
                     uint32_t       n_groups,
                     uint32_t       n_out,
                     uint32_t       relu)
{
    uint32_t c, g;
    (void)p_shf;                /* 当前模型恒 30, 与汇编硬编码一致 */

    for (c = 0; c < n_out; c++) {
        const int32_t *pw = p_w + c * (n_groups * 2);
        const int8_t  *px = p_in;
        int32_t acc = p_b[c];

        for (g = 0; g < n_groups; g++) {
            int32_t p0 = pw[2 * g];
            int32_t p1 = pw[2 * g + 1];
            int16_t w0 = (int16_t)(p0 & 0xFFFF);
            int16_t w2 = (int16_t)(p0 >> 16);
            int16_t w1 = (int16_t)(p1 & 0xFFFF);
            int16_t w3 = (int16_t)(p1 >> 16);
            acc += (int32_t)px[4 * g + 0] * w0
                 + (int32_t)px[4 * g + 1] * w1
                 + (int32_t)px[4 * g + 2] * w2
                 + (int32_t)px[4 * g + 3] * w3;
        }
        {
            int64_t prod = (int64_t)acc * (int64_t)p_mul[c];
            int32_t r = (int32_t)(prod >> 30);
            if (relu) {
                if (r < 0) r = 0;
                if (r > 127) r = 127;
            } else {
                if (r > 127) r = 127;
                if (r < -128) r = -128;
            }
            p_out[c] = (int8_t)r;
        }
    }
}
