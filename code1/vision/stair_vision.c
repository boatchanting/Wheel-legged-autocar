/**
 * @file    stair_vision.c
 * @brief   台阶检测视觉模块 — Core 1 算法封装层实现
 * @details 完整台阶检测流水线, 内部状态自包含。
 *          仅对外暴露 init / reset / process / get_output 四个 API。
 *
 *          内存策略 (DTCM, ~4KB):
 *            - 4 行图像展开环形缓冲 (Gx 用 2 行, Gy 用 4 行)
 *            - Gx/Gy 单行输出缓冲 (卷积结果暂存, 立即累加后丢弃)
 *            - |Gx| 列和累加器 P[185] + Gy 行均值剖面 gy_prof[117]
 *            - fit_gy_edges 阶段按需从原图重算 3 行 Gy (~1.1KB 栈)
 *
 *          流水线:
 *            逐行 uint8→int16 展开 (4 行环形缓冲)
 *            → v9_conv_gx_row (2行→1行) → 增量累加 P[c]
 *            → v9_conv_gy_row (4行→1行) → 增量累加 gy_prof[r]
 *            → stair_discriminate (用累积的 P[] 和 gy_prof[])
 *            → detect_crease (用 gy_prof[])
 *            → 按需重算 3 行 Gy → fit_gy_edges
 *            → 多帧滤波 → 发布
 *
 * @date    2026-07-24
 */

#include "stair_vision.h"
#include "tcm.h"
#include "v9_stair_conv_asm.h"
#include <string.h>


/* ==========================================================================
 * DTCM 环形缓冲 + 累加器
 * ========================================================================== */

/* 4 行图像展开环形缓冲 (Gx 需连续 2 行, Gy 需连续 4 行) */
#define STAIR_ROW_RING_DEPTH    (4U)
DTCM_BSS static int16_t  stair_row_ring[STAIR_ROW_RING_DEPTH][STAIR_IMAGE_W];

/* Gx/Gy 单行输出缓冲 (卷积结果立即累加, 无需保留) */
DTCM_BSS static int16_t  stair_gx_row[STAIR_GX_OUT_W];
DTCM_BSS static int16_t  stair_gy_row[STAIR_GY_OUT_COLS];

/* |Gx| 列和累加器 P[c] */
DTCM_BSS static float    stair_P_accum[STAIR_GX_OUT_W];

/* Gy 行均值剖面 gy_prof[r] (117 个 float) */
DTCM_BSS static float    stair_gy_prof[STAIR_GY_OUT_ROWS];


/* ==========================================================================
 * 多帧滤波参数
 * ========================================================================== */
#define STAIR_CONFIRM_FRAMES        (2U)
#define STAIR_LOST_HOLD_FRAMES      (3U)


/* ==========================================================================
 * 全局状态
 * ========================================================================== */
volatile runtime_profiler_t g_stair_vision_cost_profiler = {0};
volatile runtime_profiler_t g_stair_vision_frame_profiler = {0};
volatile stair_vision_output_t g_stair_vision_output = {0};
volatile uint8 g_stair_vision_output_write_busy = 0U;

static stair_vision_output_t g_stair_output_shadow;
#if STAIR_VISION_PROFILE_ENABLE
static uint32 g_stair_last_frame_time_us = 0U;
#endif


/* ==========================================================================
 * 内部: 按需重算峰值行附近的 3 行 Gy (用于 fit_gy_edges)
 *
 * fit_gy_edges 需要访问 upper_peak_y 行及其 ±1 行的 Gy 数据做垂直扩展.
 * 由于我们不存储全图 Gy (省 43KB SRAM), 此处从原图重算所需 3 行.
 *
 * 输入: gray = 原图, upper_peak_y = 上峰在 Gy 输出中的行号 (0~116)
 * 输出: gy_3rows[3][185], 其中 gy_3rows[1] 对应 upper_peak_y 行
 * ========================================================================== */
static void stair_recompute_gy_3rows(const uint8 *gray, int16_t upper_peak_y,
                                     int16_t gy_3rows[3][STAIR_GY_OUT_COLS])
{
    /* Gy 卷积需要 4 行输入产生 1 行输出.
       要得到 Gy 输出行 [peak_y-1, peak_y, peak_y+1],
       需要输入行 [peak_y-1 .. peak_y+3], 共 5 行. */
    int16_t in_buf[4][STAIR_IMAGE_W];
    int     out_idx, i, x;
    int16_t base_row;

    if (upper_peak_y < 1) upper_peak_y = 1;
    if (upper_peak_y > (int16_t)(STAIR_IMAGE_H - 5)) upper_peak_y = (int16_t)(STAIR_IMAGE_H - 5);

    base_row = (int16_t)(upper_peak_y - 1);

    for (out_idx = 0; out_idx < 3; out_idx++) {
        /* 展开 4 行输入 */
        for (i = 0; i < 4; i++) {
            const uint8 *src = &gray[((int32_t)base_row + out_idx + i) * STAIR_IMAGE_W];
            for (x = 0; x < STAIR_IMAGE_W; x++) {
                in_buf[i][x] = (int16_t)src[x];
            }
        }
        v9_conv_gy_row(in_buf[0], in_buf[1], in_buf[2], in_buf[3],
                       gy_3rows[out_idx], STAIR_GY_OUT_COLS);
    }
}


/* ==========================================================================
 * 内部: 单帧检测流水线 (无滤波)
 * ========================================================================== */
static void stair_detect_process(const uint8 *gray, v9_stair_result_t *out_result)
{
    int      row;
    uint32_t ring_idx = 0;
    int      gy_row_count = 0;
    int      x, c;

    /* 清零累加器 */
    memset(stair_P_accum, 0, sizeof(stair_P_accum));
    memset(stair_gy_prof, 0, sizeof(stair_gy_prof));

    /* ---- 逐行流水线: 展开 + Gx/Gy 卷积 + 增量累加 ---- */
    for (row = 0; row < STAIR_IMAGE_H; row++)
    {
        const uint8 *src_row = &gray[row * STAIR_IMAGE_W];

        /* 1) uint8 → int16 展开到当前 ring slot */
        for (x = 0; x < STAIR_IMAGE_W; x++) {
            stair_row_ring[ring_idx][x] = (int16_t)src_row[x];
        }

        /* 2) Gx 卷积: 需要连续 2 行 → 累加 |Gx| 列和 */
        if (row >= 1) {
            uint32_t prev = (ring_idx + STAIR_ROW_RING_DEPTH - 1) & 3;
            v9_conv_gx_row(stair_row_ring[prev], stair_row_ring[ring_idx],
                           stair_gx_row, STAIR_GX_OUT_W);
            for (x = 0; x < STAIR_GX_OUT_W; x++) {
                int16_t v = stair_gx_row[x];
                if (v < 0) v = (int16_t)(-v);
                stair_P_accum[x] += (float)v;
            }
        }

        /* 3) Gy 卷积: 需要连续 4 行 → 累加行均值剖面 */
        if (row >= 3) {
            uint32_t r0 = (ring_idx + STAIR_ROW_RING_DEPTH - 3) & 3;
            uint32_t r1 = (ring_idx + STAIR_ROW_RING_DEPTH - 2) & 3;
            uint32_t r2 = (ring_idx + STAIR_ROW_RING_DEPTH - 1) & 3;

            v9_conv_gy_row(stair_row_ring[r0], stair_row_ring[r1],
                           stair_row_ring[r2], stair_row_ring[ring_idx],
                           stair_gy_row, STAIR_GY_OUT_COLS);

            float sum_gy = 0.0f;
            for (x = 0; x < STAIR_GY_OUT_COLS; x++) {
                sum_gy += (float)stair_gy_row[x];
            }
            stair_gy_prof[gy_row_count] = sum_gy / (float)STAIR_GY_OUT_COLS;
            gy_row_count++;
        }

        ring_idx = (ring_idx + 1) & 3;
    }

    /* ---- 后处理 ---- */
    memset(out_result, 0, sizeof(*out_result));
    out_result->crease_y    = -1;
    out_result->upper_peak_y = -1;
    out_result->lower_peak_y = -1;

    /* stair_discriminate: 用累加的 P[] 和 gy_prof[] */
    {
        float sum_P = 0.0f;
        for (c = 0; c < STAIR_GX_OUT_W; c++) sum_P += stair_P_accum[c];
        if (sum_P >= 1.0f) {
            float mean_P = sum_P / (float)STAIR_GX_OUT_W;
            float var_P = 0.0f;
            for (c = 0; c < STAIR_GX_OUT_W; c++) {
                float d = stair_P_accum[c] - mean_P;
                var_P += d * d;
            }
            var_P /= (float)STAIR_GX_OUT_W;
            float gx_score = var_P * var_P / sum_P;

            uint32_t start = V9_GY_VAR_START_ROW;
            if (start >= (uint32_t)gy_row_count) start = (uint32_t)(gy_row_count - 1);
            uint32_t n_bottom = (uint32_t)gy_row_count - start;
            if (n_bottom >= 2) {
                float sum_gy2 = 0.0f;
                uint32_t r;
                for (r = start; r < (uint32_t)gy_row_count; r++)
                    sum_gy2 += stair_gy_prof[r];
                float mean_gy2 = sum_gy2 / (float)n_bottom;
                float var_gy2 = 0.0f;
                for (r = start; r < (uint32_t)gy_row_count; r++) {
                    float d = stair_gy_prof[r] - mean_gy2;
                    var_gy2 += d * d;
                }
                var_gy2 /= (float)n_bottom;
                out_result->joint_score = gx_score * var_gy2;
            }
        }
    }

    /* detect_crease: gy_prof[] 已经是行均值, 直接传入 */
    detect_crease(NULL, stair_gy_prof, (uint32_t)gy_row_count, STAIR_GY_OUT_COLS, out_result);

    /* fit_gy_edges: 按需重算上峰附近 3 行 Gy, 传入 3×185 视图 */
    if (out_result->upper_peak_y >= 0) {
        int16_t gy_3rows[3][STAIR_GY_OUT_COLS];
        stair_recompute_gy_3rows(gray, out_result->upper_peak_y, gy_3rows);

        /* 在 3 行视图中, 上峰行位于索引 1 */
        fit_gy_edges((const int16_t *)gy_3rows, 3, STAIR_GY_OUT_COLS,
                     (int16_t)1, out_result);

        if (out_result->num_edge_points >= 2) {
            out_result->has_stairs = 1;
        }
    }
}


/* ==========================================================================
 * 对外 API
 * ========================================================================== */

void stair_vision_init(void)
{
    memset((void *)stair_row_ring, 0, sizeof(stair_row_ring));
    memset((void *)stair_gx_row,   0, sizeof(stair_gx_row));
    memset((void *)stair_gy_row,   0, sizeof(stair_gy_row));
    stair_vision_reset_filter();

#if STAIR_VISION_PROFILE_ENABLE
    timer_init(STAIR_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(STAIR_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_stair_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_stair_vision_frame_profiler);
    g_stair_last_frame_time_us = timer_get(STAIR_VISION_PROFILE_TIMER);
#endif
}

void stair_vision_reset_filter(void)
{
    stair_vision_output_t empty;
    memset(&empty, 0, sizeof(empty));
    g_stair_output_shadow = empty;
    g_stair_vision_output_write_busy = 1U;
    g_stair_vision_output = empty;
    g_stair_vision_output_write_busy = 0U;
}

const volatile stair_vision_output_t *stair_vision_get_output(void)
{
    return &g_stair_vision_output;
}

void stair_vision_process_camera_frame(const uint8 *gray)
{
    v9_stair_result_t     detect;
    stair_vision_output_t next;

    if (gray == NULL) return;

#if STAIR_VISION_PROFILE_ENABLE
    {
        const uint32 now_us = timer_get(STAIR_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_stair_vision_frame_profiler,
                                (uint32)(now_us - g_stair_last_frame_time_us));
        g_stair_last_frame_time_us = now_us;
    }
    RUNTIME_PROFILE_BEGIN(g_stair_vision_cost_profiler, STAIR_VISION_PROFILE_TIMER);
#endif

    stair_detect_process(gray, &detect);

    /* 多帧滤波 */
    next = g_stair_output_shadow;
    next.frame_id++;
    next.raw_detected = detect.has_stairs;

    if (detect.has_stairs) {
        next.detected_streak++;
        next.lost_streak = 0;
        if (next.detected_streak >= STAIR_CONFIRM_FRAMES) {
            next.detected = 1;
            next.result = detect;
        }
    } else {
        next.lost_streak++;
        next.detected_streak = 0;
        if (next.lost_streak >= STAIR_LOST_HOLD_FRAMES) {
            next.detected = 0;
            memset(&next.result, 0, sizeof(next.result));
            next.result.crease_y    = -1;
            next.result.upper_peak_y = -1;
            next.result.lower_peak_y = -1;
        }
    }

    /* 发布 */
    g_stair_vision_output_write_busy = 1U;
    g_stair_vision_output = next;
    g_stair_output_shadow = next;
    g_stair_vision_output_write_busy = 0U;

#if STAIR_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_stair_vision_cost_profiler, STAIR_VISION_PROFILE_TIMER);
#endif
}

