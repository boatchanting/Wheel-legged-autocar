/**
 * ============================================================================
 * bridge_v2_arbiter.c  ——  新单边桥管线仲裁层实现 (设计文档 §3.3/§4.3/§5.2 C21)
 * ============================================================================
 * 仲裁规则:
 *   RB/RMB → 红蓝中点线 x(y) = (x_red(y)+x_blue(y))/2, 支撑=两线支撑交集
 *   RM/MB  → 绿线
 *   R/B/M/NONE/RB_Q → 失能 (b2_valid=0, source=2)
 * 定点: a×1000, b×100; 支撑 u_lo/u_hi 钳位到 [0,59]。
 * 写忙保护: process() 置 busy, 发布端读前检查 (与旧 bridge_vision 同模式)。
 * ============================================================================
 */

#include "bridge_v2_arbiter.h"
#include <string.h>

static bridge_v2_arb_t g_bridge_v2_arb;
static uint32 g_bridge_v2_frame_id = 0U;
static volatile uint8 g_bridge_v2_write_busy = 0U;

static int16 clamp_i16(float v, float lo, float hi)
{
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return (int16)v;
}

static uint8 clamp_u8(float v, float lo, float hi)
{
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return (uint8)v;
}

void bridge_v2_arbitrate(const bridge_result_t *res, bridge_v2_arb_t *out)
{
    memset(out, 0, sizeof(*out));
    if (res == NULL)
    {
        out->source = 2U;
        return;
    }

    out->mode           = (uint8)res->mode;
    out->gate           = res->gate;
    out->has_top        = res->has_top;
    out->spacing_x100   = (uint16)clamp_i16(res->spacing  * 100.0f,  0.0f, 65535.0f);
    out->mid_ratio_x1000 = (uint16)clamp_i16(res->mid_ratio * 1000.0f, 0.0f, 65535.0f);

    if (res->has_top)
    {
        out->top_a_x1000 = (int16)clamp_i16(res->top.a * 1000.0f, -32768.0f, 32767.0f);
        out->top_b_x100  = (int16)clamp_i16(res->top.b * 100.0f,  -32768.0f, 32767.0f);
    }

    switch (res->mode)
    {
    case BRIDGE_MODE_RB:
    case BRIDGE_MODE_RMB:
        /* 以左右边线为准: 红蓝中点线 */
        if (!res->has_red || !res->has_blue)
        {
            out->source = 2U;
            return;
        }
        out->valid  = 1U;
        out->source = 0U;
        out->line_a_x1000 = (int16)clamp_i16((res->red.a + res->blue.a) * 0.5f * 1000.0f,
                                             -32768.0f, 32767.0f);
        out->line_b_x100  = (int16)clamp_i16((res->red.b + res->blue.b) * 0.5f * 100.0f,
                                             -32768.0f, 32767.0f);
        /* 支撑 = 两线支撑交集; 空交集自然使 25∈[lo,hi] 校验失败 (保守) */
        out->u_lo = clamp_u8((res->red.u_lo > res->blue.u_lo) ? res->red.u_lo : res->blue.u_lo,
                             0.0f, 59.0f);
        out->u_hi = clamp_u8((res->red.u_hi < res->blue.u_hi) ? res->red.u_hi : res->blue.u_hi,
                             0.0f, 59.0f);
        break;

    case BRIDGE_MODE_RM:
    case BRIDGE_MODE_MB:
        /* 直接出中线 (绿线) */
        if (!res->has_green)
        {
            out->source = 2U;
            return;
        }
        out->valid  = 1U;
        out->source = 1U;
        out->line_a_x1000 = (int16)clamp_i16(res->green.a * 1000.0f, -32768.0f, 32767.0f);
        out->line_b_x100  = (int16)clamp_i16(res->green.b * 100.0f,  -32768.0f, 32767.0f);
        out->u_lo = clamp_u8(res->green.u_lo, 0.0f, 59.0f);
        out->u_hi = clamp_u8(res->green.u_hi, 0.0f, 59.0f);
        break;

    default:
        /* R(1)/B(2)/M(3)/NONE(0)/RB_Q(8): 视觉失能, 回锁角 */
        out->source = 2U;
        break;
    }
}

void bridge_v2_arbiter_process(const bridge_result_t *res)
{
    g_bridge_v2_write_busy = 1U;
    bridge_v2_arbitrate(res, &g_bridge_v2_arb);
    g_bridge_v2_frame_id++;
    g_bridge_v2_write_busy = 0U;
}

const bridge_v2_arb_t *bridge_v2_arbiter_get(void)
{
    return &g_bridge_v2_arb;
}

uint32 bridge_v2_arbiter_get_frame_id(void)
{
    return g_bridge_v2_frame_id;
}

uint8 bridge_v2_arbiter_is_busy(void)
{
    return g_bridge_v2_write_busy;
}
