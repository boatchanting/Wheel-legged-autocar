/* C implementation of the bridge convolution and MLP operators. */

#include "bridge_asm_ops.h"
#include "tcm.h"

ITCM_FUNC void b2_conv1d_horiz_gxgy(const int16_t *p_input,
                                    int16_t *p_gx_out,
                                    int16_t *p_gy_out,
                                    uint32_t out_width)
{
    uint32_t i;

    for (i = 0U; i < out_width; ++i)
    {
        const int32_t p0 = p_input[i + 0U];
        const int32_t p1 = p_input[i + 1U];
        const int32_t p2 = p_input[i + 2U];
        const int32_t p3 = p_input[i + 3U];

        p_gx_out[i] = (int16_t)(-p0 - p1 + p2 + p3);
        p_gy_out[i] = (int16_t)(p0 + 3 * p1 + 3 * p2 + p3);
    }
}

ITCM_FUNC void b2_conv1d_vert_gxgy_row(const int16_t *p_gx_h0,
                                       const int16_t *p_gx_h1,
                                       const int16_t *p_gx_h2,
                                       const int16_t *p_gx_h3,
                                       const int16_t *p_gy_h0,
                                       const int16_t *p_gy_h1,
                                       const int16_t *p_gy_h2,
                                       const int16_t *p_gy_h3,
                                       int16_t *p_gx_out,
                                       int16_t *p_gy_out,
                                       uint32_t out_width)
{
    uint32_t i;

    for (i = 0U; i < out_width; ++i)
    {
        const int32_t gx0 = p_gx_h0[i];
        const int32_t gx1 = p_gx_h1[i];
        const int32_t gx2 = p_gx_h2[i];
        const int32_t gx3 = p_gx_h3[i];
        const int32_t gy0 = p_gy_h0[i];
        const int32_t gy1 = p_gy_h1[i];
        const int32_t gy2 = p_gy_h2[i];
        const int32_t gy3 = p_gy_h3[i];

        p_gx_out[i] = (int16_t)(gx0 + 3 * gx1 + 3 * gx2 + gx3);
        p_gy_out[i] = (int16_t)(-gy0 - gy1 + gy2 + gy3);
    }
}

static int16_t packed_low(int32_t value)
{
    return (int16_t)(uint16_t)(uint32_t)value;
}

static int16_t packed_high(int32_t value)
{
    return (int16_t)(uint16_t)((uint32_t)value >> 16);
}

static int32_t requant_s30(int32_t value, int32_t multiplier)
{
    return (int32_t)(((int64_t)value * (int64_t)multiplier) >> 30);
}

ITCM_FUNC void mlp_fc_s8_layer(const int8_t *p_in,
                               const int32_t *p_w,
                               const int32_t *p_b,
                               const int32_t *p_mul,
                               const int32_t *p_shf,
                               int8_t *p_out,
                               uint32_t n_groups,
                               uint32_t n_out,
                               uint32_t relu)
{
    uint32_t c;

    /* The assembly implementation currently uses a fixed shift of 30. */
    (void)p_shf;

    for (c = 0U; c < n_out; ++c)
    {
        uint32_t g;
        uint32_t acc_bits = (uint32_t)p_b[c];
        const int32_t *w = p_w + (c * n_groups * 2U);

        for (g = 0U; g < n_groups; ++g)
        {
            const uint32_t input_offset = g * 4U;
            const int16_t w02 = packed_low(w[g * 2U]);
            const int16_t w13 = packed_low(w[g * 2U + 1U]);
            const int16_t w20 = packed_high(w[g * 2U]);
            const int16_t w31 = packed_high(w[g * 2U + 1U]);
            int32_t term;

            term = (int32_t)p_in[input_offset + 0U] * w02;
            acc_bits += (uint32_t)term;
            term = (int32_t)p_in[input_offset + 2U] * w20;
            acc_bits += (uint32_t)term;
            term = (int32_t)p_in[input_offset + 1U] * w13;
            acc_bits += (uint32_t)term;
            term = (int32_t)p_in[input_offset + 3U] * w31;
            acc_bits += (uint32_t)term;
        }

        {
            int32_t value = requant_s30((int32_t)acc_bits, p_mul[c]);
            if (relu != 0U)
            {
                if (value < 0) value = 0;
                if (value > 127) value = 127;
            }
            else
            {
                if (value > 127) value = 127;
                if (value < -128) value = -128;
            }
            p_out[c] = (int8_t)value;
        }
    }
}
