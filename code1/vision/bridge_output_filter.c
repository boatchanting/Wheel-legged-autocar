/**
 * ============================================================================
 * bridge_output_filter.c  ——  单边桥仲裁输出中值滤波层实现 (2026-08-14)
 * ============================================================================
 * 策略见 bridge_output_filter.h 头注释。全部静态分配, 无堆;
 * 中值为 W(=5) 元素插入排序取中位, 每帧两次, 耗时可忽略 (<5us)。
 * ============================================================================
 */

#include "bridge_output_filter.h"
#include <string.h>

/* ---------------- 滑动窗 ---------------- */
typedef struct
{
    int16 a[BRIDGE_FILTER_WINDOW];
    int16 b[BRIDGE_FILTER_WINDOW];
    uint8 valid[BRIDGE_FILTER_WINDOW];
    uint8 head;     /* 下一个写入位置 */
    uint8 count;    /* 已写入帧数 (≤ WINDOW) */
} bridge_med_win_t;

typedef struct
{
    bridge_med_win_t line;          /* 控制线窗 */
    bridge_med_win_t top;           /* 结束线窗 (仅 has_top=1 帧) */
    uint8  last_source;             /* 上帧 source, 切换时清 line 窗 */
    bridge_v2_arb_t filtered;       /* 滤波后输出 */
    uint32 frame_id;
    volatile uint8 write_busy;
} bridge_output_filter_t;

static bridge_output_filter_t s_f;

/* ---------------- 窗操作 ---------------- */
static void win_clear(bridge_med_win_t *w)
{
    memset(w, 0, sizeof(*w));
}

static void win_push(bridge_med_win_t *w, int16 a, int16 b, uint8 valid)
{
    w->a[w->head]     = a;
    w->b[w->head]     = b;
    w->valid[w->head] = valid;
    w->head = (uint8)((w->head + 1U) % BRIDGE_FILTER_WINDOW);
    if (w->count < BRIDGE_FILTER_WINDOW) w->count++;
}

/* 窗内有效帧数 */
static uint8 win_valid_count(const bridge_med_win_t *w)
{
    uint8 i, n = 0U;
    for (i = 0U; i < w->count; i++)
        if (w->valid[i]) n++;
    return n;
}

/* 窗内有效帧的中值 (调用前保证有效帧数 >= BRIDGE_FILTER_MIN_VALID >= 1)。
 * 偶数个有效帧时取上中位 (下标 n/2)。 */
static int16 win_median(const bridge_med_win_t *w, int is_b)
{
    int16 buf[BRIDGE_FILTER_WINDOW];
    uint8 i, j, n = 0U;
    int16 t;
    for (i = 0U; i < w->count; i++)
    {
        if (!w->valid[i]) continue;
        buf[n++] = is_b ? w->b[i] : w->a[i];
    }
    /* 插入排序 (n ≤ 5) */
    for (i = 1U; i < n; i++)
    {
        t = buf[i];
        for (j = i; j > 0U && buf[j - 1U] > t; j--)
            buf[j] = buf[j - 1U];
        buf[j] = t;
    }
    return buf[n / 2U];
}

/* ---------------- 对外接口 ---------------- */
void bridge_output_filter_reset(void)
{
    s_f.write_busy = 1U;
    memset(&s_f, 0, sizeof(s_f));   /* write_busy 一并清 0 */
}

void bridge_output_filter_update(const bridge_v2_arb_t *raw)
{
    uint8 nv;

    if (raw == 0) return;

    s_f.write_busy = 1U;

    /* 0. 直通: 先整帧拷贝, 之后只覆写需要滤波的字段 */
    s_f.filtered = *raw;

    /* 1. 控制线: source 切换清窗 (红蓝中点线与绿线不跨源求中值) */
    if (raw->source != s_f.last_source)
    {
        win_clear(&s_f.line);
        s_f.last_source = raw->source;
    }
    win_push(&s_f.line, raw->line_a_x1000, raw->line_b_x100, raw->valid);
    nv = win_valid_count(&s_f.line);
    if (raw->valid && nv >= BRIDGE_FILTER_MIN_VALID)
    {
        s_f.filtered.line_a_x1000 = win_median(&s_f.line, 0);
        s_f.filtered.line_b_x100  = win_median(&s_f.line, 1);
    }

    /* 2. 结束线: 直通 (确认/锁存已上移到融合层, 2026-08-15); 仅保留几何中值平滑 */
    if (raw->has_top)
    {
        win_push(&s_f.top, raw->top_a_x1000, raw->top_b_x100, 1U);
    }
    else
    {
        win_clear(&s_f.top);
    }
    s_f.filtered.has_top = raw->has_top;
    if (raw->has_top && win_valid_count(&s_f.top) >= BRIDGE_FILTER_MIN_VALID)
    {
        s_f.filtered.top_a_x1000 = win_median(&s_f.top, 0);
        s_f.filtered.top_b_x100  = win_median(&s_f.top, 1);
    }

    s_f.frame_id++;
    s_f.write_busy = 0U;
}

const bridge_v2_arb_t *bridge_output_filter_get(void)
{
    return &s_f.filtered;
}

uint32 bridge_output_filter_get_frame_id(void)
{
    return s_f.frame_id;
}

uint8 bridge_output_filter_is_busy(void)
{
    return s_f.write_busy;
}
