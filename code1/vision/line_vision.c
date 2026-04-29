#include "line_vision.h"

#if LINE_VISION_ENABLE

typedef struct
{
    uint16 x;
    uint16 y;
} line_point_t;

typedef struct
{
    uint16 area;
    uint8 xmin;
    uint8 ymin;
    uint8 xmax;
    uint8 ymax;
    float mean_gray;
    float fill_ratio;
    float score;
} line_bridge_component_t;

typedef struct
{
    uint8 white_mask[LINE_IMAGE_SIZE];
    uint8 visited[LINE_IMAGE_SIZE];
    uint16 stack[LINE_IMAGE_SIZE];
    uint16 hist[256];
    line_point_t points[LINE_IMAGE_H];
    uint8 widths[LINE_IMAGE_H];
} line_scratch_t;

volatile runtime_profiler_t g_line_vision_cost_profiler = {0};
volatile runtime_profiler_t g_line_vision_frame_profiler = {0};
volatile line_vision_output_t g_line_vision_output = {0};
volatile uint8 g_line_vision_output_write_busy = 0U;

static line_scratch_t g_line_scratch;
static line_vision_output_t g_line_output_shadow;
static uint32 g_line_last_frame_time_us = 0U;

static float line_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

static float line_min_f(float a, float b)
{
    return (a < b) ? a : b;
}

static float line_max_f(float a, float b)
{
    return (a > b) ? a : b;
}

static float line_constrain_f(float value, float min_value, float max_value)
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

static void line_clear_frame_result(line_vision_frame_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->bridge_bbox_xmin = 0xFFU;
    result->bridge_bbox_ymin = 0xFFU;
    result->bridge_bbox_xmax = 0xFFU;
    result->bridge_bbox_ymax = 0xFFU;
}

static uint8 line_percentile_from_hist(const uint16 *hist, uint16 total, uint16 percent)
{
    uint16 target = (uint16)((uint32)total * (uint32)percent / 100U);
    uint16 acc = 0U;

    if (target >= total)
    {
        target = (uint16)(total - 1U);
    }

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

static void line_build_near_white_mask(const uint8 *gray, uint8 y_min, uint8 y_max, float *white_ratio)
{
    uint32 white_count = 0U;
    uint32 roi_count = (uint32)(y_max - y_min + 1U) * (uint32)LINE_IMAGE_W;

    memset(g_line_scratch.white_mask, 0, sizeof(g_line_scratch.white_mask));

    for (uint8 y = y_min; y <= y_max; y++)
    {
        uint32 sum = 0U;
        uint8 p70;
        uint8 p92;
        float threshold_f;
        uint8 threshold;
        const uint16 row_base = (uint16)y * LINE_IMAGE_W;

        memset(g_line_scratch.hist, 0, sizeof(g_line_scratch.hist));

        for (uint8 x = 0U; x < LINE_IMAGE_W; x++)
        {
            const uint8 pixel = gray[row_base + x];
            sum += pixel;
            g_line_scratch.hist[pixel]++;
        }

        p70 = line_percentile_from_hist(g_line_scratch.hist, LINE_IMAGE_W, 70U);
        p92 = line_percentile_from_hist(g_line_scratch.hist, LINE_IMAGE_W, 92U);
        threshold_f = line_max_f((float)sum / (float)LINE_IMAGE_W + 8.0f, (float)p70 + 4.0f);
        threshold_f = line_max_f(threshold_f, (float)p92 - 22.0f);
        threshold_f = line_constrain_f(threshold_f, 145.0f, 245.0f);
        threshold = (uint8)(threshold_f + 0.5f);

        for (uint8 x = 0U; x < LINE_IMAGE_W; x++)
        {
            if (gray[row_base + x] >= threshold)
            {
                g_line_scratch.white_mask[row_base + x] = 255U;
                white_count++;
            }
        }
    }

    *white_ratio = (roi_count > 0U) ? ((float)white_count / (float)roi_count) : 0.0f;
}

static uint8 line_extract_center_points(uint8 y_min, uint8 y_max, uint8 *out_y_span, float *out_mean_width)
{
    uint8 point_count = 0U;
    uint16 width_sum = 0U;
    uint8 min_y = 0xFFU;
    uint8 max_y = 0U;
    uint8 lost_rows = 0U;
    float prev_center = ((float)LINE_IMAGE_W - 1.0f) * 0.5f;

    for (int y = (int)y_max; y >= (int)y_min; y--)
    {
        uint8 found = 0U;
        uint8 best_x0 = 0U;
        uint8 best_x1 = 0U;
        float best_center = 0.0f;
        float best_diff = 100000.0f;
        uint8 x = 0U;

        while (x < LINE_IMAGE_W)
        {
            uint8 x0;
            uint8 x1;
            uint8 run_width;
            float center;
            float diff;

            while ((x < LINE_IMAGE_W) &&
                   (g_line_scratch.white_mask[(uint16)y * LINE_IMAGE_W + x] == 0U))
            {
                x++;
            }
            if (x >= LINE_IMAGE_W)
            {
                break;
            }

            x0 = x;
            while ((x < LINE_IMAGE_W) &&
                   (g_line_scratch.white_mask[(uint16)y * LINE_IMAGE_W + x] != 0U))
            {
                x++;
            }
            x1 = (uint8)(x - 1U);
            run_width = (uint8)(x1 - x0 + 1U);
            if (run_width < LINE_VISION_MIN_WIDTH)
            {
                continue;
            }

            center = ((float)x0 + (float)x1) * 0.5f;
            diff = line_abs_f(center - prev_center);
            if (diff < best_diff)
            {
                best_diff = diff;
                best_center = center;
                best_x0 = x0;
                best_x1 = x1;
                found = 1U;
            }
        }

        if (found == 0U)
        {
            lost_rows++;
            if ((lost_rows >= 4U) && (point_count > 0U))
            {
                break;
            }
            continue;
        }

        if ((point_count > 0U) &&
            (line_abs_f(best_center - prev_center) > ((float)LINE_IMAGE_W * 0.30f)))
        {
            lost_rows++;
            continue;
        }

        lost_rows = 0U;
        prev_center = 0.70f * prev_center + 0.30f * best_center;

        if (point_count < LINE_IMAGE_H)
        {
            const uint8 run_width = (uint8)(best_x1 - best_x0 + 1U);
            g_line_scratch.points[point_count].x = (uint16)(best_center * 100.0f + 0.5f);
            g_line_scratch.points[point_count].y = (uint16)y;
            g_line_scratch.widths[point_count] = run_width;
            width_sum = (uint16)(width_sum + run_width);
            if ((uint8)y < min_y) { min_y = (uint8)y; }
            if ((uint8)y > max_y) { max_y = (uint8)y; }
            point_count++;
        }
    }

    *out_y_span = (point_count >= 2U) ? (uint8)(max_y - min_y) : 0U;
    *out_mean_width = (point_count > 0U) ? ((float)width_sum / (float)point_count) : 0.0f;
    return point_count;
}

static uint8 line_fit_points(uint8 point_count,
                             float *out_k,
                             float *out_b,
                             float *out_rmse)
{
    float sum_x = 0.0f;
    float sum_y = 0.0f;
    float sum_yy = 0.0f;
    float sum_xy = 0.0f;
    float denom;
    float err_sum = 0.0f;

    if (point_count < 2U)
    {
        return 0U;
    }

    for (uint8 i = 0U; i < point_count; i++)
    {
        const float x = (float)g_line_scratch.points[i].x * 0.01f;
        const float y = (float)g_line_scratch.points[i].y;
        sum_x += x;
        sum_y += y;
        sum_yy += y * y;
        sum_xy += x * y;
    }

    denom = (float)point_count * sum_yy - sum_y * sum_y;
    if (line_abs_f(denom) < 0.001f)
    {
        return 0U;
    }

    *out_k = ((float)point_count * sum_xy - sum_y * sum_x) / denom;
    *out_b = (sum_x - (*out_k) * sum_y) / (float)point_count;

    for (uint8 i = 0U; i < point_count; i++)
    {
        const float x = (float)g_line_scratch.points[i].x * 0.01f;
        const float y = (float)g_line_scratch.points[i].y;
        const float pred = (*out_k) * y + (*out_b);
        const float e = x - pred;
        err_sum += e * e;
    }

    *out_rmse = sqrtf(err_sum / (float)point_count);
    return 1U;
}

static void line_flood_dark_component(const uint8 *gray,
                                      uint16 start_index,
                                      uint8 y_min,
                                      uint8 y_max,
                                      line_bridge_component_t *out)
{
    uint16 stack_top = 0U;
    uint16 area = 0U;
    uint8 xmin = (uint8)(LINE_IMAGE_W - 1U);
    uint8 ymin = (uint8)(LINE_IMAGE_H - 1U);
    uint8 xmax = 0U;
    uint8 ymax = 0U;
    uint32 sum_gray = 0U;

    g_line_scratch.stack[stack_top++] = start_index;
    g_line_scratch.visited[start_index] = 1U;

    while (stack_top > 0U)
    {
        const uint16 index = g_line_scratch.stack[--stack_top];
        const uint8 y = (uint8)(index / LINE_IMAGE_W);
        const uint8 x = (uint8)(index - (uint16)y * LINE_IMAGE_W);

        area++;
        sum_gray += gray[index];

        if (x < xmin) { xmin = x; }
        if (x > xmax) { xmax = x; }
        if (y < ymin) { ymin = y; }
        if (y > ymax) { ymax = y; }

        if (y > y_min)
        {
            const uint16 ni = (uint16)(index - LINE_IMAGE_W);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
        if (y < y_max)
        {
            const uint16 ni = (uint16)(index + LINE_IMAGE_W);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
        if (x > 0U)
        {
            const uint16 ni = (uint16)(index - 1U);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
        if (x < (LINE_IMAGE_W - 1U))
        {
            const uint16 ni = (uint16)(index + 1U);
            if ((g_line_scratch.visited[ni] == 0U) &&
                (gray[ni] < LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                g_line_scratch.visited[ni] = 1U;
                if (stack_top < LINE_IMAGE_SIZE)
                {
                    g_line_scratch.stack[stack_top++] = ni;
                }
            }
        }
    }

    {
        const uint16 bbox_area = (uint16)((xmax - xmin + 1U) * (ymax - ymin + 1U));
        out->area = area;
        out->xmin = xmin;
        out->ymin = ymin;
        out->xmax = xmax;
        out->ymax = ymax;
        out->mean_gray = (area > 0U) ? ((float)sum_gray / (float)area) : 255.0f;
        out->fill_ratio = (bbox_area > 0U) ? ((float)area / (float)bbox_area) : 0.0f;
        out->score = 0.0f;
    }
}

static float line_score_bridge_component(const line_bridge_component_t *component)
{
    const uint8 comp_w = (uint8)(component->xmax - component->xmin + 1U);
    const uint8 comp_h = (uint8)(component->ymax - component->ymin + 1U);
    const float area_score = line_min_f((float)component->area / 260.0f, 1.0f);
    const float size_score = 0.5f * line_min_f((float)comp_w / 28.0f, 1.0f) +
                             0.5f * line_min_f((float)comp_h / 12.0f, 1.0f);
    const float dark_score = line_constrain_f((190.0f - component->mean_gray) / 70.0f, 0.0f, 1.0f);
    const float top_score = line_max_f(0.0f, 1.0f - (float)component->ymin / ((float)LINE_IMAGE_H * 0.45f));

    return 0.38f * area_score + 0.28f * size_score + 0.22f * dark_score + 0.12f * top_score;
}

static uint8 line_detect_dark_bridge(const uint8 *gray, line_vision_frame_result_t *result)
{
    const uint8 y_min = (uint8)((uint32)LINE_IMAGE_H * 3U / 100U);
    const uint8 y_max = (uint8)(LINE_IMAGE_H - 2U);
    line_bridge_component_t best;
    uint8 component_count = 0U;

    memset(&best, 0, sizeof(best));
    best.xmin = 0xFFU;
    best.ymin = 0xFFU;
    best.xmax = 0xFFU;
    best.ymax = 0xFFU;

    memset(g_line_scratch.visited, 0, sizeof(g_line_scratch.visited));

    for (uint8 y = y_min; y <= y_max; y++)
    {
        for (uint8 x = 0U; x < LINE_IMAGE_W; x++)
        {
            const uint16 index = (uint16)y * LINE_IMAGE_W + x;
            line_bridge_component_t component;
            uint8 comp_w;
            uint8 comp_h;

            if ((g_line_scratch.visited[index] != 0U) ||
                (gray[index] >= LINE_VISION_BRIDGE_DARK_THRESHOLD))
            {
                continue;
            }

            line_flood_dark_component(gray, index, y_min, y_max, &component);
            component_count++;

            comp_w = (uint8)(component.xmax - component.xmin + 1U);
            comp_h = (uint8)(component.ymax - component.ymin + 1U);
            if ((component.area < LINE_VISION_BRIDGE_MIN_AREA) ||
                (comp_w < LINE_VISION_BRIDGE_MIN_WIDTH) ||
                (comp_h < LINE_VISION_BRIDGE_MIN_HEIGHT) ||
                (component.fill_ratio < LINE_VISION_BRIDGE_MIN_FILL_RATIO))
            {
                continue;
            }

            component.score = line_score_bridge_component(&component);
            if (component.score > best.score)
            {
                best = component;
            }
        }
    }

    result->bridge_component_count = component_count;
    if (best.score >= LINE_VISION_BRIDGE_MIN_CONFIDENCE)
    {
        result->bridge_detected = 1U;
        result->bridge_confidence = best.score;
        result->bridge_bbox_xmin = best.xmin;
        result->bridge_bbox_ymin = best.ymin;
        result->bridge_bbox_xmax = best.xmax;
        result->bridge_bbox_ymax = best.ymax;
        result->target_speed_hint = LINE_VISION_BRIDGE_SPEED_HINT;
        return 1U;
    }

    result->bridge_confidence = best.score;
    result->bridge_bbox_xmin = best.xmin;
    result->bridge_bbox_ymin = best.ymin;
    result->bridge_bbox_xmax = best.xmax;
    result->bridge_bbox_ymax = best.ymax;
    return 0U;
}

static void line_detect_frame(const uint8 *gray, line_vision_frame_result_t *result)
{
    const uint8 y_min = (uint8)((uint32)LINE_IMAGE_H * LINE_VISION_ROI_TOP_RATIO_X100 / 100U);
    const uint8 y_max = (uint8)(LINE_IMAGE_H - 2U);
    const uint8 lookahead_y = (uint8)((uint32)LINE_IMAGE_H * 62U / 100U);
    const uint8 bottom_y = y_max;
    uint8 point_count;
    uint8 y_span;
    float mean_width;
    float k = 0.0f;
    float b = 0.0f;
    float rmse = 0.0f;

    line_clear_frame_result(result);

    line_detect_dark_bridge(gray, result);
    line_build_near_white_mask(gray, y_min, y_max, &result->roi_white_ratio);
    point_count = line_extract_center_points(y_min, y_max, &y_span, &mean_width);

    result->points_used = point_count;
    result->y_span = y_span;
    result->mean_track_width = mean_width;

    if ((point_count >= LINE_VISION_MIN_ROWS) &&
        (line_fit_points(point_count, &k, &b, &rmse) != 0U))
    {
        const float line_x_bottom = k * (float)bottom_y + b;
        const float line_x_lookahead = k * (float)lookahead_y + b;
        const float lateral_error_px = line_x_bottom - ((float)LINE_IMAGE_W - 1.0f) * 0.5f;
        const float yaw_error_deg = atanf(k) * 57.29578f;
        const float row_score = line_min_f((float)point_count /
                                           ((float)(y_max - y_min + 1U) * 0.70f), 1.0f);
        const float span_score = line_min_f((float)y_span /
                                            ((float)(y_max - y_min) * 0.75f), 1.0f);
        const float rmse_score = line_max_f(0.0f, 1.0f - rmse / 5.5f);
        const float width_score = line_min_f(mean_width / ((float)LINE_IMAGE_W * 0.45f), 1.0f);
        const float center_score = line_max_f(0.0f,
                                              1.0f - line_abs_f(lateral_error_px) /
                                              ((float)LINE_IMAGE_W * 0.55f));

        result->fit_rmse = rmse;
        result->line_x_bottom = line_x_bottom;
        result->line_x_lookahead = line_x_lookahead;
        result->lateral_error_px = lateral_error_px;
        result->yaw_error_deg = yaw_error_deg;
        result->confidence = 0.30f * row_score +
                             0.22f * span_score +
                             0.24f * rmse_score +
                             0.14f * width_score +
                             0.10f * center_score;

        if ((result->confidence >= LINE_VISION_MIN_CONFIDENCE) &&
            (y_span >= LINE_VISION_MIN_Y_SPAN) &&
            (line_abs_f(yaw_error_deg) <= LINE_VISION_MAX_ABS_YAW_DEG))
        {
            result->detected = 1U;
        }
    }

    if (result->bridge_detected)
    {
        result->detected = 0U;
        result->confidence = 0.0f;
        result->lateral_error_px = 0.0f;
        result->yaw_error_deg = 0.0f;
        result->line_x_bottom = 0.0f;
        result->line_x_lookahead = 0.0f;
        result->target_speed_hint = LINE_VISION_BRIDGE_SPEED_HINT;
    }
}

static void line_update_filter(const line_vision_frame_result_t *raw)
{
    line_vision_output_t next = g_line_output_shadow;

    next.frame_id++;
    next.raw = *raw;
    next.raw_detected = raw->detected;
    next.bridge_raw_detected = raw->bridge_detected;

    if (raw->bridge_detected)
    {
        if (next.bridge_detected_streak < 255U)
        {
            next.bridge_detected_streak++;
        }
        next.bridge_lost_streak = 0U;
        if (next.bridge_detected_streak >= LINE_VISION_BRIDGE_CONFIRM_FRAMES)
        {
            next.bridge_stable_detected = 1U;
        }
    }
    else
    {
        next.bridge_detected_streak = 0U;
        if (next.bridge_lost_streak < 255U)
        {
            next.bridge_lost_streak++;
        }
        if (next.bridge_lost_streak >= LINE_VISION_BRIDGE_LOST_HOLD_FRAMES)
        {
            next.bridge_stable_detected = 0U;
        }
    }

    if ((raw->detected != 0U) && (next.bridge_stable_detected == 0U))
    {
        if (next.detected_streak < 255U)
        {
            next.detected_streak++;
        }
        next.lost_streak = 0U;
        if (next.detected_streak >= LINE_VISION_CONFIRM_FRAMES)
        {
            next.stable_detected = 1U;
        }
    }
    else
    {
        next.detected_streak = 0U;
        if (next.lost_streak < 255U)
        {
            next.lost_streak++;
        }
        if (next.lost_streak >= LINE_VISION_LOST_HOLD_FRAMES)
        {
            next.stable_detected = 0U;
        }
    }

    if (next.bridge_stable_detected)
    {
        if (raw->bridge_detected)
        {
            next.stable = *raw;
        }
        next.stable.bridge_detected = 1U;
        next.stable.detected = 0U;
        next.stable.confidence = 0.0f;
        next.stable.lateral_error_px = 0.0f;
        next.stable.yaw_error_deg = 0.0f;
        next.stable.target_speed_hint = LINE_VISION_BRIDGE_SPEED_HINT;
        next.stable_detected = 0U;
    }
    else if (next.stable_detected)
    {
        if (raw->detected)
        {
            next.stable = *raw;
        }
        next.stable.detected = 1U;
        next.stable.bridge_detected = 0U;
    }
    else
    {
        line_clear_frame_result(&next.stable);
    }

    g_line_output_shadow = next;
    g_line_vision_output_write_busy = 1U;
    g_line_vision_output = next;
    g_line_vision_output_write_busy = 0U;
}

void line_vision_init(void)
{
    line_vision_reset_filter();
#if LINE_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_RESET(&g_line_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_line_vision_frame_profiler);
    g_line_last_frame_time_us = timer_get(LINE_VISION_PROFILE_TIMER);
#endif
}

void line_vision_reset_filter(void)
{
    line_vision_output_t empty;

    memset(&empty, 0, sizeof(empty));
    line_clear_frame_result(&empty.raw);
    line_clear_frame_result(&empty.stable);
    g_line_output_shadow = empty;
    g_line_vision_output_write_busy = 1U;
    g_line_vision_output = empty;
    g_line_vision_output_write_busy = 0U;
}

const volatile line_vision_output_t *line_vision_get_output(void)
{
    return &g_line_vision_output;
}

void line_vision_process_camera_frame(const uint8 *gray)
{
    line_vision_frame_result_t raw;

    if (gray == NULL)
    {
        return;
    }

#if LINE_VISION_PROFILE_ENABLE
    {
        const uint32 now_us = timer_get(LINE_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_line_vision_frame_profiler, (uint32)(now_us - g_line_last_frame_time_us));
        g_line_last_frame_time_us = now_us;
    }
    RUNTIME_PROFILE_BEGIN(g_line_vision_cost_profiler, LINE_VISION_PROFILE_TIMER);
#endif

    line_detect_frame(gray, &raw);
    line_update_filter(&raw);

#if LINE_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_line_vision_cost_profiler, LINE_VISION_PROFILE_TIMER);
#endif

#if (LINE_VISION_DEBUG_PRINT_EVERY > 0U)
    if ((g_line_vision_output.frame_id % LINE_VISION_DEBUG_PRINT_EVERY) == 0U)
    {
        printf("[LINE] frame=%lu line=%u bridge=%u conf=%d bconf=%d cost=%lu us\r\n",
               (unsigned long)g_line_vision_output.frame_id,
               g_line_vision_output.stable_detected,
               g_line_vision_output.bridge_stable_detected,
               (int)(g_line_vision_output.raw.confidence * 1000.0f),
               (int)(g_line_vision_output.raw.bridge_confidence * 1000.0f),
               (unsigned long)g_line_vision_cost_profiler.last_us);
    }
#endif
}

#endif
