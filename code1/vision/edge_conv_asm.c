/* C implementation of the 4x4 edge convolution operators. */

#include "edge_conv_asm.h"
#include "tcm.h"

ITCM_FUNC void conv1d_horiz_gxgy(const int16_t *p_input,
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

        /* The coefficient order matches the packed SMLAD constants. */
        p_gx_out[i] = (int16_t)(-p0 - p1 + p2 + p3);
        p_gy_out[i] = (int16_t)(3 * p0 + p1 + p2 + 3 * p3);
    }
}

ITCM_FUNC void conv1d_vert_gxgy_row(const int16_t *p_gx_h0,
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

        p_gx_out[i] = (int16_t)(3 * gx0 + gx1 + gx2 + 3 * gx3);
        p_gy_out[i] = (int16_t)(-gy0 - gy1 + gy2 + gy3);
    }
}

ITCM_FUNC void gradient_mag_dir_fixed(const int16_t *p_gx,
                                      const int16_t *p_gy,
                                      edge_dir_result_t *p_out,
                                      int32_t fixed_thr,
                                      uint32_t total_pixels)
{
    uint32_t i;
    int32_t sum_gx = 0;
    int32_t sum_gy = 0;
    uint32_t strong_count = 0U;

    for (i = 0U; i < total_pixels; ++i)
    {
        const int32_t gx = p_gx[i];
        const int32_t gy = p_gy[i];
        const int32_t abs_gx = (gx < 0) ? -gx : gx;
        const int32_t abs_gy = (gy < 0) ? -gy : gy;
        const int32_t magnitude = abs_gx + abs_gy;

        if ((abs_gx >= fixed_thr) || (magnitude >= fixed_thr))
        {
            ++strong_count;
            sum_gx += gx;
            sum_gy += gy;
        }
    }

    p_out->sum_gx = sum_gx;
    p_out->sum_gy = sum_gy;
    p_out->strong_count = strong_count;
    p_out->total_pixels = total_pixels;
}
