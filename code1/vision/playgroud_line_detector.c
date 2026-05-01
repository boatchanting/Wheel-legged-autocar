/*
 * =================================================================================
 * 文件: playgroud_line_detector.c
 * 作用: 1 核 (Core 1) 红色操场直线检测模块实现
 * 说明:
 * 1) 单帧检测：灰度图预处理 -> 亮核心掩码 -> 连通域 -> 候选评分 -> 直线拟合
 * 2) 时序处理：detected / predicted / lost 三态，最多容忍 max_lost 帧
 * 3) 输出封装：参考 pvc_vision 模块，提供稳定输出与耗时统计
 * =================================================================================
 */
#include "playgroud_line_detector.h"

#if PLAYGROUD_LINE_DETECTOR_ENABLE

#include <math.h>
#include <string.h>

typedef struct
{
    int area;
    int xmin;
    int ymin;
    int xmax;
    int ymax;
    int label_id;
    float centroid_x;
    float centroid_y;
    float score;
} playgroud_component_t;

typedef struct
{
    uint8 active;
    uint8 max_lost;
    uint8 lost_count;
    float smooth_alpha;
    float min_temporal_score;
    float bottom_x;
    float lookahead_x;
    float yaw_deg;
    float confidence;
} playgroud_temporal_state_t;

typedef struct
{
    uint8 mode;
    uint8 accepted;
    float temporal_score;
} playgroud_temporal_decision_t;

typedef struct
{
    uint8 max_lost;
    float smooth_alpha;
    float min_temporal_score;
} playgroud_temporal_config_t;

typedef struct
{
    uint8 gray_blur[PLAYGROUD_IMAGE_SIZE];
    uint8 mask[PLAYGROUD_IMAGE_SIZE];
    uint8 visited[PLAYGROUD_IMAGE_SIZE];
    uint16 labels[PLAYGROUD_IMAGE_SIZE];
    uint16 stack[PLAYGROUD_IMAGE_SIZE];
    int integral[(PLAYGROUD_IMAGE_H + 1U) * (PLAYGROUD_IMAGE_W + 1U)];
    playgroud_component_t components[PLAYGROUD_LINE_MAX_COMPONENTS];
    playgroud_component_t candidates[PLAYGROUD_LINE_MAX_COMPONENTS];
} playgroud_scratch_t;

volatile runtime_profiler_t g_playgroud_line_detector_cost_profiler = {0};
volatile runtime_profiler_t g_playgroud_line_detector_frame_profiler = {0};
volatile playgroud_line_detector_output_t g_playgroud_line_detector_output = {0};
volatile uint8 g_playgroud_line_detector_output_write_busy = 0U;

static playgroud_scratch_t g_playgroud_scratch;
static playgroud_line_detector_output_t g_playgroud_output_shadow;
static playgroud_temporal_state_t g_playgroud_temporal_state;
static playgroud_temporal_config_t g_playgroud_temporal_config =
{
    PLAYGROUD_LINE_DEFAULT_MAX_LOST,
    PLAYGROUD_LINE_DEFAULT_SMOOTH_ALPHA,
    PLAYGROUD_LINE_DEFAULT_MIN_TEMPORAL_SCORE
};
static uint32 g_playgroud_last_frame_time_us = 0U;

static float playgroud_min_f(float a, float b) { return (a < b) ? a : b; }
static float playgroud_max_f(float a, float b) { return (a > b) ? a : b; }
static float playgroud_abs_f(float a) { return (a < 0.0f) ? -a : a; }
static float playgroud_clamp_f(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static void playgroud_clear_frame_result(playgroud_line_detector_frame_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->bbox_xmin = 0xFFU;
    result->bbox_ymin = 0xFFU;
    result->bbox_xmax = 0xFFU;
    result->bbox_ymax = 0xFFU;
    result->line_x_bottom = -1.0f;
    result->line_x_lookahead = -1.0f;
    result->temporal_score = -1.0f;
}

/* 3x3 均值滤波：压制传感器噪点，稳定后续阈值分割 */
static void playgroud_blur3x3_u8(const uint8 *src, uint8 *dst)
{
    for (uint8 y = 0U; y < PLAYGROUD_IMAGE_H; y++)
    {
        const uint8 y0 = (y > 0U) ? (y - 1U) : 0U;
        const uint8 y1 = y;
        const uint8 y2 = (y < (PLAYGROUD_IMAGE_H - 1U)) ? (y + 1U) : (PLAYGROUD_IMAGE_H - 1U);

        for (uint8 x = 0U; x < PLAYGROUD_IMAGE_W; x++)
        {
            const uint8 x0 = (x > 0U) ? (x - 1U) : 0U;
            const uint8 x1 = x;
            const uint8 x2 = (x < (PLAYGROUD_IMAGE_W - 1U)) ? (x + 1U) : (PLAYGROUD_IMAGE_W - 1U);
            uint16 sum = 0U;

            sum += src[(uint16)y0 * PLAYGROUD_IMAGE_W + x0];
            sum += src[(uint16)y0 * PLAYGROUD_IMAGE_W + x1];
            sum += src[(uint16)y0 * PLAYGROUD_IMAGE_W + x2];
            sum += src[(uint16)y1 * PLAYGROUD_IMAGE_W + x0];
            sum += src[(uint16)y1 * PLAYGROUD_IMAGE_W + x1];
            sum += src[(uint16)y1 * PLAYGROUD_IMAGE_W + x2];
            sum += src[(uint16)y2 * PLAYGROUD_IMAGE_W + x0];
            sum += src[(uint16)y2 * PLAYGROUD_IMAGE_W + x1];
            sum += src[(uint16)y2 * PLAYGROUD_IMAGE_W + x2];

            dst[(uint16)y * PLAYGROUD_IMAGE_W + x] = (uint8)((sum + 4U) / 9U);
        }
    }
}

static uint8 playgroud_percentile_from_hist(const uint16 *hist, uint16 total, uint8 p_x100)
{
    uint16 acc = 0U;
    uint16 target = 0U;

    if (total == 0U)
    {
        return 0U;
    }

    target = (uint16)(((uint32)(total - 1U) * (uint32)p_x100 + 50U) / 100U);
    for (uint16 i = 0U; i < 256U; i++)
    {
        acc = (uint16)(acc + hist[i]);
        if (acc > target)
        {
            return (uint8)i;
        }
    }
    return 255U;
}

static void playgroud_compute_integral_u8(const uint8 *gray, int *integral)
{
    const uint16 stride = PLAYGROUD_IMAGE_W + 1U;
    memset(integral, 0, sizeof(int) * (PLAYGROUD_IMAGE_H + 1U) * (PLAYGROUD_IMAGE_W + 1U));

    for (uint16 y = 1U; y <= PLAYGROUD_IMAGE_H; y++)
    {
        int row_sum = 0;
        for (uint16 x = 1U; x <= PLAYGROUD_IMAGE_W; x++)
        {
            row_sum += (int)gray[(y - 1U) * PLAYGROUD_IMAGE_W + (x - 1U)];
            integral[y * stride + x] = integral[(y - 1U) * stride + x] + row_sum;
        }
    }
}

static int playgroud_rect_sum(const int *integral, uint16 x0, uint16 y0, uint16 x1, uint16 y1)
{
    const uint16 stride = PLAYGROUD_IMAGE_W + 1U;
    const int A = integral[y0 * stride + x0];
    const int B = integral[y0 * stride + x1];
    const int C = integral[y1 * stride + x0];
    const int D = integral[y1 * stride + x1];
    return D - B - C + A;
}

/*
 * 构建亮核心掩码：
 * - 全局高分位阈值保证“足够亮”
 * - 与局部均值比较保证“相对突出”
 */
static void playgroud_build_bright_core_mask(const uint8 *gray)
{
    uint16 hist[256];
    uint16 q_hi;
    uint16 hi_thr;
    const uint8 radius = PLAYGROUD_LINE_MASK_LOCAL_MEAN_RADIUS;

    memset(hist, 0, sizeof(hist));
    playgroud_blur3x3_u8(gray, g_playgroud_scratch.gray_blur);

    for (uint16 i = 0U; i < PLAYGROUD_IMAGE_SIZE; i++)
    {
        hist[g_playgroud_scratch.gray_blur[i]]++;
    }

    q_hi = playgroud_percentile_from_hist(
        hist,
        (uint16)PLAYGROUD_IMAGE_SIZE,
        PLAYGROUD_LINE_MASK_QHI_PERCENT_X100);
    hi_thr = (q_hi > PLAYGROUD_LINE_MASK_HI_THR_MIN) ? q_hi : PLAYGROUD_LINE_MASK_HI_THR_MIN;

    playgroud_compute_integral_u8(g_playgroud_scratch.gray_blur, g_playgroud_scratch.integral);

    for (uint8 y = 0U; y < PLAYGROUD_IMAGE_H; y++)
    {
        for (uint8 x = 0U; x < PLAYGROUD_IMAGE_W; x++)
        {
            const uint8 x0 = (x > radius) ? (x - radius) : 0U;
            const uint8 y0 = (y > radius) ? (y - radius) : 0U;
            const uint8 x1 = (x + radius < (PLAYGROUD_IMAGE_W - 1U)) ? (x + radius) : (PLAYGROUD_IMAGE_W - 1U);
            const uint8 y1 = (y + radius < (PLAYGROUD_IMAGE_H - 1U)) ? (y + radius) : (PLAYGROUD_IMAGE_H - 1U);
            const uint16 sx1 = (uint16)x1 + 1U;
            const uint16 sy1 = (uint16)y1 + 1U;
            const uint16 area = (uint16)((x1 - x0 + 1U) * (y1 - y0 + 1U));
            const int local_mean = (playgroud_rect_sum(g_playgroud_scratch.integral, x0, y0, sx1, sy1) + (int)area / 2) / (int)area;
            const uint8 g = g_playgroud_scratch.gray_blur[(uint16)y * PLAYGROUD_IMAGE_W + x];
            const uint8 is_white = (uint8)((g >= hi_thr) &&
                                           (((int)g - local_mean) >= (int)PLAYGROUD_LINE_MASK_LOCAL_CONTRAST_MIN));
            g_playgroud_scratch.mask[(uint16)y * PLAYGROUD_IMAGE_W + x] = is_white ? 255U : 0U;
        }
    }
}

static void playgroud_flood_component(uint16 start_idx,
                                      uint16 label_id,
                                      playgroud_component_t *out)
{
    uint16 top = 0U;
    int area = 0;
    int xmin = (int)PLAYGROUD_IMAGE_W - 1;
    int ymin = (int)PLAYGROUD_IMAGE_H - 1;
    int xmax = 0;
    int ymax = 0;
    int32 sum_x = 0;
    int32 sum_y = 0;

    g_playgroud_scratch.stack[top++] = start_idx;
    g_playgroud_scratch.visited[start_idx] = 1U;
    g_playgroud_scratch.labels[start_idx] = label_id;

    while (top > 0U)
    {
        const uint16 idx = g_playgroud_scratch.stack[--top];
        const uint8 y = (uint8)(idx / PLAYGROUD_IMAGE_W);
        const uint8 x = (uint8)(idx - (uint16)y * PLAYGROUD_IMAGE_W);

        area++;
        sum_x += x;
        sum_y += y;

        if ((int)x < xmin) xmin = x;
        if ((int)x > xmax) xmax = x;
        if ((int)y < ymin) ymin = y;
        if ((int)y > ymax) ymax = y;

        if (y > 0U)
        {
            const uint16 n = (uint16)(idx - PLAYGROUD_IMAGE_W);
            if ((g_playgroud_scratch.visited[n] == 0U) && (g_playgroud_scratch.mask[n] != 0U))
            {
                g_playgroud_scratch.visited[n] = 1U;
                g_playgroud_scratch.labels[n] = label_id;
                g_playgroud_scratch.stack[top++] = n;
            }
        }
        if (y < (PLAYGROUD_IMAGE_H - 1U))
        {
            const uint16 n = (uint16)(idx + PLAYGROUD_IMAGE_W);
            if ((g_playgroud_scratch.visited[n] == 0U) && (g_playgroud_scratch.mask[n] != 0U))
            {
                g_playgroud_scratch.visited[n] = 1U;
                g_playgroud_scratch.labels[n] = label_id;
                g_playgroud_scratch.stack[top++] = n;
            }
        }
        if (x > 0U)
        {
            const uint16 n = (uint16)(idx - 1U);
            if ((g_playgroud_scratch.visited[n] == 0U) && (g_playgroud_scratch.mask[n] != 0U))
            {
                g_playgroud_scratch.visited[n] = 1U;
                g_playgroud_scratch.labels[n] = label_id;
                g_playgroud_scratch.stack[top++] = n;
            }
        }
        if (x < (PLAYGROUD_IMAGE_W - 1U))
        {
            const uint16 n = (uint16)(idx + 1U);
            if ((g_playgroud_scratch.visited[n] == 0U) && (g_playgroud_scratch.mask[n] != 0U))
            {
                g_playgroud_scratch.visited[n] = 1U;
                g_playgroud_scratch.labels[n] = label_id;
                g_playgroud_scratch.stack[top++] = n;
            }
        }
    }

    out->area = area;
    out->xmin = xmin;
    out->ymin = ymin;
    out->xmax = xmax;
    out->ymax = ymax;
    out->label_id = (int)label_id;
    out->centroid_x = (area > 0) ? ((float)sum_x / (float)area) : 0.0f;
    out->centroid_y = (area > 0) ? ((float)sum_y / (float)area) : 0.0f;
    out->score = 0.0f;
}

static uint8 playgroud_collect_components(void)
{
    uint8 count = 0U;

    memset(g_playgroud_scratch.visited, 0, sizeof(g_playgroud_scratch.visited));
    memset(g_playgroud_scratch.labels, 0, sizeof(g_playgroud_scratch.labels));

    for (uint16 i = 0U; i < PLAYGROUD_IMAGE_SIZE; i++)
    {
        if ((g_playgroud_scratch.mask[i] == 0U) || (g_playgroud_scratch.visited[i] != 0U))
        {
            continue;
        }
        if (count >= PLAYGROUD_LINE_MAX_COMPONENTS)
        {
            break;
        }
        playgroud_flood_component(i, (uint16)count + 1U, &g_playgroud_scratch.components[count]);
        count++;
    }
    return count;
}

static float playgroud_score_component(const playgroud_component_t *component)
{
    const float comp_w = (float)(component->xmax - component->xmin + 1);
    const float comp_h = (float)(component->ymax - component->ymin + 1);
    const float center = 1.0f -
        playgroud_abs_f(component->centroid_x - (float)PLAYGROUD_IMAGE_W * 0.5f) /
        playgroud_max_f(1.0f, (float)PLAYGROUD_IMAGE_W * 0.5f);
    const float top_touch = (component->ymin <= (int)((float)PLAYGROUD_IMAGE_H * 0.14f)) ? 1.0f : 0.0f;
    const float tall = comp_h / playgroud_max_f(1.0f, (float)PLAYGROUD_IMAGE_H);
    const float area = (float)component->area / playgroud_max_f(1.0f, (float)PLAYGROUD_IMAGE_SIZE);
    const float width_penalty = comp_w / playgroud_max_f(1.0f, (float)PLAYGROUD_IMAGE_W);

    return 1.35f * tall + 0.90f * center + 1.00f * top_touch + 0.25f * area - 0.50f * width_penalty;
}

static uint8 playgroud_filter_candidates(uint8 component_count)
{
    uint8 candidate_count = 0U;

    for (uint8 i = 0U; i < component_count; i++)
    {
        playgroud_component_t component = g_playgroud_scratch.components[i];
        const int comp_h = component.ymax - component.ymin + 1;
        const int min_area = (int)playgroud_max_f(
            (float)PLAYGROUD_LINE_MIN_AREA,
            (float)PLAYGROUD_IMAGE_SIZE * 0.012f);
        const int min_h = (int)playgroud_max_f(
            (float)PLAYGROUD_LINE_MIN_HEIGHT,
            (float)PLAYGROUD_IMAGE_H * 0.22f);

        if (component.area < min_area)
        {
            continue;
        }
        if (comp_h < min_h)
        {
            continue;
        }

        component.score = playgroud_score_component(&component);
        g_playgroud_scratch.candidates[candidate_count++] = component;
        if (candidate_count >= PLAYGROUD_LINE_MAX_COMPONENTS)
        {
            break;
        }
    }

    for (uint8 i = 1U; i < candidate_count; i++)
    {
        playgroud_component_t key = g_playgroud_scratch.candidates[i];
        int j = (int)i - 1;
        while ((j >= 0) && (g_playgroud_scratch.candidates[j].score < key.score))
        {
            g_playgroud_scratch.candidates[j + 1] = g_playgroud_scratch.candidates[j];
            j--;
        }
        g_playgroud_scratch.candidates[j + 1] = key;
    }

    return candidate_count;
}

static uint8 playgroud_fit_line_for_label(uint16 label_id,
                                          const playgroud_component_t *best,
                                          float *a,
                                          float *b,
                                          uint8 *rows_used)
{
    float sum_y = 0.0f;
    float sum_x = 0.0f;
    float sum_yy = 0.0f;
    float sum_yx = 0.0f;
    uint8 n = 0U;

    for (int y = best->ymin; y <= best->ymax; y++)
    {
        int cnt = 0;
        int sx = 0;
        const uint16 row = (uint16)y * PLAYGROUD_IMAGE_W;

        for (int x = best->xmin; x <= best->xmax; x++)
        {
            if (g_playgroud_scratch.labels[row + (uint8)x] == label_id)
            {
                cnt++;
                sx += x;
            }
        }
        if (cnt <= 0)
        {
            continue;
        }

        {
            const float cx = (float)sx / (float)cnt;
            sum_y += (float)y;
            sum_x += cx;
            sum_yy += (float)y * (float)y;
            sum_yx += (float)y * cx;
            n++;
        }
    }

    *rows_used = n;
    if (n < PLAYGROUD_LINE_MIN_FIT_ROWS)
    {
        return 0U;
    }

    {
        const float denom = (float)n * sum_yy - sum_y * sum_y;
        if (playgroud_abs_f(denom) < 1e-6f)
        {
            return 0U;
        }
        *a = ((float)n * sum_yx - sum_y * sum_x) / denom;
        *b = (sum_x - (*a) * sum_y) / (float)n;
    }

    return 1U;
}

static void playgroud_detect_frame(const uint8 *gray,
                                   playgroud_line_detector_frame_result_t *result)
{
    uint8 component_count;
    uint8 candidate_count;

    playgroud_clear_frame_result(result);

    playgroud_build_bright_core_mask(gray);
    component_count = playgroud_collect_components();
    candidate_count = playgroud_filter_candidates(component_count);

    result->component_count = component_count;
    result->candidate_count = candidate_count;

    if (candidate_count == 0U)
    {
        return;
    }

    {
        const playgroud_component_t *best = &g_playgroud_scratch.candidates[0];
        float a = 0.0f;
        float b = 0.0f;
        uint8 rows_used = 0U;

        if (playgroud_fit_line_for_label((uint16)best->label_id, best, &a, &b, &rows_used) == 0U)
        {
            return;
        }

        {
            const float y_bottom = (float)((uint16)((float)PLAYGROUD_IMAGE_H * 0.93f));
            const float y_lookahead = (float)((uint16)((float)PLAYGROUD_IMAGE_H * 0.72f));
            const float y2 = (float)((uint16)((float)PLAYGROUD_IMAGE_H * 0.62f));
            float x_bottom = a * y_bottom + b;
            float x_lookahead = a * y_lookahead + b;
            float x2 = a * y2 + b;
            float confidence;

            x_bottom = playgroud_min_f(playgroud_max_f(x_bottom, 0.0f), (float)(PLAYGROUD_IMAGE_W - 1U));
            x_lookahead = playgroud_min_f(playgroud_max_f(x_lookahead, 0.0f), (float)(PLAYGROUD_IMAGE_W - 1U));
            x2 = playgroud_min_f(playgroud_max_f(x2, 0.0f), (float)(PLAYGROUD_IMAGE_W - 1U));

            confidence = best->score / 2.6f;
            confidence = playgroud_min_f(playgroud_max_f(confidence, 0.0f), 1.0f);

            result->detected = (uint8)(rows_used >= PLAYGROUD_LINE_MIN_FIT_ROWS);
            if (confidence < PLAYGROUD_LINE_MIN_DECISION_SCORE)
            {
                result->detected = 0U;
            }

            result->confidence = confidence;
            result->bbox_xmin = (uint8)best->xmin;
            result->bbox_ymin = (uint8)best->ymin;
            result->bbox_xmax = (uint8)best->xmax;
            result->bbox_ymax = (uint8)best->ymax;
            result->centroid_x = best->centroid_x;
            result->centroid_y = best->centroid_y;
            result->line_x_bottom = x_bottom;
            result->line_x_lookahead = x_lookahead;
            result->line_point_rows = rows_used;
            result->lateral_error_px = x_lookahead - ((float)PLAYGROUD_IMAGE_W * 0.5f);
            result->line_yaw_deg = atanf((x2 - x_bottom) / (y_bottom - y2)) * 57.295779513f;
        }
    }
}

static playgroud_temporal_decision_t playgroud_temporal_update(
    const playgroud_line_detector_frame_result_t *raw,
    playgroud_line_detector_frame_result_t *stable)
{
    playgroud_temporal_decision_t decision;

    decision.mode = PLAYGROUD_LINE_MODE_LOST;
    decision.accepted = 0U;
    decision.temporal_score = -1.0f;
    *stable = *raw;

    if (raw->detected == 0U)
    {
        if (g_playgroud_temporal_state.active)
        {
            g_playgroud_temporal_state.lost_count++;
            if (g_playgroud_temporal_state.lost_count <= g_playgroud_temporal_state.max_lost)
            {
                decision.mode = PLAYGROUD_LINE_MODE_PREDICTED;
                stable->detected = 1U;
                stable->line_x_bottom = g_playgroud_temporal_state.bottom_x;
                stable->line_x_lookahead = g_playgroud_temporal_state.lookahead_x;
                stable->line_yaw_deg = g_playgroud_temporal_state.yaw_deg;
                stable->lateral_error_px = g_playgroud_temporal_state.lookahead_x -
                                           (float)PLAYGROUD_IMAGE_W * 0.5f;
                stable->confidence = g_playgroud_temporal_state.confidence;
                stable->temporal_score = -1.0f;
                return decision;
            }
            g_playgroud_temporal_state.active = 0U;
        }
        playgroud_clear_frame_result(stable);
        return decision;
    }

    if (g_playgroud_temporal_state.active == 0U)
    {
        g_playgroud_temporal_state.active = 1U;
        g_playgroud_temporal_state.lost_count = 0U;
        g_playgroud_temporal_state.bottom_x = raw->line_x_bottom;
        g_playgroud_temporal_state.lookahead_x = raw->line_x_lookahead;
        g_playgroud_temporal_state.yaw_deg = raw->line_yaw_deg;
        g_playgroud_temporal_state.confidence = raw->confidence;

        decision.mode = PLAYGROUD_LINE_MODE_DETECTED;
        decision.accepted = 1U;
        decision.temporal_score = raw->confidence;
        stable->temporal_score = decision.temporal_score;
        return decision;
    }

    {
        const float dx_bottom = playgroud_abs_f(raw->line_x_bottom - g_playgroud_temporal_state.bottom_x);
        const float dx_lookahead = playgroud_abs_f(raw->line_x_lookahead - g_playgroud_temporal_state.lookahead_x);
        const float dyaw = playgroud_abs_f(raw->line_yaw_deg - g_playgroud_temporal_state.yaw_deg);
        const float denom = playgroud_max_f(1.0f, (float)PLAYGROUD_IMAGE_W * 0.22f);
        const float pos_penalty = playgroud_min_f(1.0f, (0.65f * dx_lookahead + 0.35f * dx_bottom) / denom);
        const float yaw_penalty = playgroud_min_f(1.0f, dyaw / 28.0f);
        const float temporal_score = raw->confidence - 0.42f * pos_penalty - 0.18f * yaw_penalty;

        decision.temporal_score = temporal_score;
        stable->temporal_score = temporal_score;

        if (temporal_score >= g_playgroud_temporal_state.min_temporal_score)
        {
            const float alpha = g_playgroud_temporal_state.smooth_alpha;

            g_playgroud_temporal_state.bottom_x =
                alpha * raw->line_x_bottom + (1.0f - alpha) * g_playgroud_temporal_state.bottom_x;
            g_playgroud_temporal_state.lookahead_x =
                alpha * raw->line_x_lookahead + (1.0f - alpha) * g_playgroud_temporal_state.lookahead_x;
            g_playgroud_temporal_state.yaw_deg =
                alpha * raw->line_yaw_deg + (1.0f - alpha) * g_playgroud_temporal_state.yaw_deg;
            g_playgroud_temporal_state.confidence =
                alpha * raw->confidence + (1.0f - alpha) * g_playgroud_temporal_state.confidence;
            g_playgroud_temporal_state.lost_count = 0U;

            stable->detected = 1U;
            stable->line_x_bottom = g_playgroud_temporal_state.bottom_x;
            stable->line_x_lookahead = g_playgroud_temporal_state.lookahead_x;
            stable->line_yaw_deg = g_playgroud_temporal_state.yaw_deg;
            stable->lateral_error_px = g_playgroud_temporal_state.lookahead_x -
                                       (float)PLAYGROUD_IMAGE_W * 0.5f;
            stable->confidence = g_playgroud_temporal_state.confidence;

            decision.mode = PLAYGROUD_LINE_MODE_DETECTED;
            decision.accepted = 1U;
            return decision;
        }
    }

    g_playgroud_temporal_state.lost_count++;
    if (g_playgroud_temporal_state.lost_count <= g_playgroud_temporal_state.max_lost)
    {
        stable->detected = 1U;
        stable->line_x_bottom = g_playgroud_temporal_state.bottom_x;
        stable->line_x_lookahead = g_playgroud_temporal_state.lookahead_x;
        stable->line_yaw_deg = g_playgroud_temporal_state.yaw_deg;
        stable->lateral_error_px = g_playgroud_temporal_state.lookahead_x -
                                   (float)PLAYGROUD_IMAGE_W * 0.5f;
        stable->confidence = g_playgroud_temporal_state.confidence;
        stable->temporal_score = decision.temporal_score;
        decision.mode = PLAYGROUD_LINE_MODE_PREDICTED;
        return decision;
    }

    g_playgroud_temporal_state.active = 0U;
    playgroud_clear_frame_result(stable);
    return decision;
}

void playgroud_line_detector_init(void)
{
    playgroud_line_detector_reset_filter();

#if PLAYGROUD_LINE_DETECTOR_PROFILE_ENABLE
    timer_init(PLAYGROUD_LINE_DETECTOR_PROFILE_TIMER, TIMER_US);
    timer_start(PLAYGROUD_LINE_DETECTOR_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_playgroud_line_detector_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_playgroud_line_detector_frame_profiler);
    g_playgroud_last_frame_time_us = timer_get(PLAYGROUD_LINE_DETECTOR_PROFILE_TIMER);
#endif
}

void playgroud_line_detector_reset_filter(void)
{
    playgroud_line_detector_output_t empty;
    memset(&empty, 0, sizeof(empty));
    playgroud_clear_frame_result(&empty.raw);
    playgroud_clear_frame_result(&empty.stable);

    g_playgroud_temporal_state.active = 0U;
    g_playgroud_temporal_state.lost_count = 0U;
    g_playgroud_temporal_state.max_lost = g_playgroud_temporal_config.max_lost;
    g_playgroud_temporal_state.smooth_alpha = g_playgroud_temporal_config.smooth_alpha;
    g_playgroud_temporal_state.min_temporal_score = g_playgroud_temporal_config.min_temporal_score;
    g_playgroud_temporal_state.bottom_x = 0.0f;
    g_playgroud_temporal_state.lookahead_x = 0.0f;
    g_playgroud_temporal_state.yaw_deg = 0.0f;
    g_playgroud_temporal_state.confidence = 0.0f;

    g_playgroud_output_shadow = empty;
    g_playgroud_line_detector_output_write_busy = 1U;
    g_playgroud_line_detector_output = empty;
    g_playgroud_line_detector_output_write_busy = 0U;
}

void playgroud_line_detector_set_temporal_params(uint8 max_lost, float smooth_alpha, float min_temporal_score)
{
    /* 平滑系数限制在 [0.01, 1.00]，避免 alpha=0 导致状态完全不更新。 */
    g_playgroud_temporal_config.max_lost = max_lost;
    g_playgroud_temporal_config.smooth_alpha = playgroud_clamp_f(smooth_alpha, 0.01f, 1.0f);
    g_playgroud_temporal_config.min_temporal_score = playgroud_clamp_f(min_temporal_score, 0.0f, 1.0f);

    g_playgroud_temporal_state.max_lost = g_playgroud_temporal_config.max_lost;
    g_playgroud_temporal_state.smooth_alpha = g_playgroud_temporal_config.smooth_alpha;
    g_playgroud_temporal_state.min_temporal_score = g_playgroud_temporal_config.min_temporal_score;
}

const volatile playgroud_line_detector_output_t *playgroud_line_detector_get_output(void)
{
    return &g_playgroud_line_detector_output;
}

void playgroud_line_detector_process_camera_frame(const uint8 *gray)
{
    playgroud_line_detector_frame_result_t raw;
    playgroud_line_detector_frame_result_t stable;
    playgroud_temporal_decision_t decision;
    playgroud_line_detector_output_t next;

    if (gray == NULL)
    {
        return;
    }

#if PLAYGROUD_LINE_DETECTOR_PROFILE_ENABLE
    {
        const uint32 now_us = timer_get(PLAYGROUD_LINE_DETECTOR_PROFILE_TIMER);
        runtime_profiler_update(
            &g_playgroud_line_detector_frame_profiler,
            (uint32)(now_us - g_playgroud_last_frame_time_us));
        g_playgroud_last_frame_time_us = now_us;
    }
    RUNTIME_PROFILE_BEGIN(g_playgroud_line_detector_cost_profiler, PLAYGROUD_LINE_DETECTOR_PROFILE_TIMER);
#endif

    playgroud_detect_frame(gray, &raw);
    decision = playgroud_temporal_update(&raw, &stable);

    next = g_playgroud_output_shadow;
    next.frame_id++;
    next.raw_detected = raw.detected;
    next.stable_detected = stable.detected;
    next.mode = decision.mode;
    next.accepted = decision.accepted;
    next.lost_count = g_playgroud_temporal_state.lost_count;
    next.raw = raw;
    next.stable = stable;

    g_playgroud_output_shadow = next;
    g_playgroud_line_detector_output_write_busy = 1U;
    g_playgroud_line_detector_output = next;
    g_playgroud_line_detector_output_write_busy = 0U;

#if PLAYGROUD_LINE_DETECTOR_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_playgroud_line_detector_cost_profiler, PLAYGROUD_LINE_DETECTOR_PROFILE_TIMER);
#endif

#if (PLAYGROUD_LINE_DETECTOR_DEBUG_PRINT_EVERY > 0U)
    if ((g_playgroud_line_detector_output.frame_id % PLAYGROUD_LINE_DETECTOR_DEBUG_PRINT_EVERY) == 0U)
    {
        printf("[PLAYGROUD] frame=%lu mode=%u raw=%u stable=%u conf=%d cost=%lu us frame_dt=%lu us\r\n",
               (unsigned long)g_playgroud_line_detector_output.frame_id,
               g_playgroud_line_detector_output.mode,
               g_playgroud_line_detector_output.raw_detected,
               g_playgroud_line_detector_output.stable_detected,
               (int)(g_playgroud_line_detector_output.raw.confidence * 1000.0f),
               (unsigned long)g_playgroud_line_detector_cost_profiler.last_us,
               (unsigned long)g_playgroud_line_detector_frame_profiler.last_us);
    }
#endif
}

#else

volatile runtime_profiler_t g_playgroud_line_detector_cost_profiler = {0};
volatile runtime_profiler_t g_playgroud_line_detector_frame_profiler = {0};
volatile playgroud_line_detector_output_t g_playgroud_line_detector_output = {0};
volatile uint8 g_playgroud_line_detector_output_write_busy = 0U;

void playgroud_line_detector_init(void) {}
void playgroud_line_detector_reset_filter(void) {}
void playgroud_line_detector_set_temporal_params(uint8 max_lost, float smooth_alpha, float min_temporal_score)
{
    (void)max_lost;
    (void)smooth_alpha;
    (void)min_temporal_score;
}
const volatile playgroud_line_detector_output_t *playgroud_line_detector_get_output(void)
{
    return &g_playgroud_line_detector_output;
}
void playgroud_line_detector_process_camera_frame(const uint8 *gray)
{
    (void)gray;
}

#endif
