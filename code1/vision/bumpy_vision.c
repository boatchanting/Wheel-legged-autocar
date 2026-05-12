#include "bumpy_vision.h"
#include "ipm_transform.h"

#if BUMPY_VISION_ENABLE

#include <math.h>
#include <string.h>

typedef struct
{
    uint16 area;
    uint8 xmin;
    uint8 ymin;
    uint8 xmax;
    uint8 ymax;
    float centroid_x;
    float centroid_y;
    float fill_ratio;
    uint8 touches_border;
    float mean_gray;
    float score;
} bumpy_component_t;

typedef struct
{
    uint8 y;
    uint8 xmin;
    uint8 xmax;
    uint8 threshold;
} bumpy_white_run_t;

typedef struct
{
    uint8 ymin;
    uint8 ymax;
    uint8 xmin;
    uint8 xmax;
    uint16 area;
    uint8 max_row_pixels;
    float mean_gray;
} bumpy_rib_band_t;

typedef struct
{
    float target_x;
    float steer_error_px;
    uint8 row_count;
    uint8 bottom_row_count;
    int8 top_y;
    int8 bottom_y;
    float mean_width;
} bumpy_centerline_summary_t;

typedef struct
{
    uint8 visited[BUMPY_IMAGE_SIZE];
    uint16 stack[BUMPY_IMAGE_SIZE];
    bumpy_component_t components[BUMPY_MAX_COMPONENTS];
    bumpy_component_t candidates[BUMPY_MAX_COMPONENTS];
    uint8 global_white_mask[BUMPY_IMAGE_SIZE];
    uint8 scan_white_mask[BUMPY_IMAGE_SIZE];
    uint8 white_mask[BUMPY_IMAGE_SIZE];
    uint8 rib_mask[BUMPY_IMAGE_SIZE];
    bumpy_white_run_t runs[BUMPY_MAX_RUNS];
    bumpy_rib_band_t rib_bands[BUMPY_MAX_RIB_BANDS];
} bumpy_scratch_t;

typedef struct
{
    uint8 detected;
    bumpy_phase_e phase;
    bumpy_mode_e mode;

    float white_threshold;
    float dark_threshold;

    uint8 component_count;
    uint8 candidate_count;
    uint8 run_count;
    uint8 rib_count;

    uint8 best_component_found;
    bumpy_component_t best_component;
    bumpy_centerline_summary_t centerline;
} bumpy_detect_result_t;

volatile runtime_profiler_t g_bumpy_vision_cost_profiler = {0};
volatile runtime_profiler_t g_bumpy_vision_frame_profiler = {0};
volatile bumpy_vision_output_t g_bumpy_vision_output = {0};
volatile uint8 g_bumpy_vision_output_write_busy = 0U;

static bumpy_scratch_t g_bumpy_scratch;
static bumpy_vision_output_t g_bumpy_output_shadow;
static float g_bumpy_prev_white_threshold = 0.0f;
static uint8 g_bumpy_has_prev_white_threshold = 0U;
static uint32 g_bumpy_last_frame_time_us = 0U;

static float bumpy_min_f(float a, float b)
{
    return (a < b) ? a : b;
}

static float bumpy_max_f(float a, float b)
{
    return (a > b) ? a : b;
}

static float bumpy_clamp_f(float value, float low, float high)
{
    return bumpy_max_f(low, bumpy_min_f(value, high));
}

static uint8 bumpy_component_width(const bumpy_component_t *component)
{
    return (uint8)(component->xmax - component->xmin + 1U);
}

static uint8 bumpy_component_height(const bumpy_component_t *component)
{
    return (uint8)(component->ymax - component->ymin + 1U);
}

static uint8 bumpy_run_width(const bumpy_white_run_t *run)
{
    return (uint8)(run->xmax - run->xmin + 1U);
}

static float bumpy_run_center_x(const bumpy_white_run_t *run)
{
    return 0.5f * (float)(run->xmin + run->xmax);
}

static float bumpy_rib_center_y(const bumpy_rib_band_t *band)
{
    return 0.5f * (float)(band->ymin + band->ymax);
}

static uint16 bumpy_confidence_to_u16(float confidence)
{
    if (confidence <= 0.0f)
    {
        return 0U;
    }
    if (confidence >= 1.0f)
    {
        return 1000U;
    }
    return (uint16)(confidence * 1000.0f);
}

static int16 bumpy_float_to_i16_x100(float value)
{
    if (value > 327.67f)
    {
        return 32767;
    }
    if (value < -327.68f)
    {
        return -32768;
    }
    return (int16)(value * 100.0f);
}

static void bumpy_hist_reset(uint16 *hist)
{
    memset(hist, 0, 256U * sizeof(hist[0]));
}

static float bumpy_hist_value_at_rank(const uint16 *hist, uint16 total, uint16 rank)
{
    uint16 cumulative = 0U;

    if (total == 0U)
    {
        return 0.0f;
    }
    if (rank >= total)
    {
        rank = (uint16)(total - 1U);
    }

    for (uint16 value = 0U; value < 256U; value++)
    {
        cumulative = (uint16)(cumulative + hist[value]);
        if (cumulative > rank)
        {
            return (float)value;
        }
    }
    return 255.0f;
}

static float bumpy_percentile_from_hist(const uint16 *hist, uint16 total, float p)
{
    float index;
    int low_rank;
    int high_rank;
    float weight;
    float low_value;
    float high_value;

    if (total == 0U)
    {
        return 0.0f;
    }
    if (p <= 0.0f)
    {
        return bumpy_hist_value_at_rank(hist, total, 0U);
    }
    if (p >= 100.0f)
    {
        return bumpy_hist_value_at_rank(hist, total, (uint16)(total - 1U));
    }

    index = (p / 100.0f) * (float)(total - 1U);
    low_rank = (int)floorf(index);
    high_rank = (int)ceilf(index);
    weight = index - (float)low_rank;

    low_value = bumpy_hist_value_at_rank(hist, total, (uint16)low_rank);
    high_value = bumpy_hist_value_at_rank(hist, total, (uint16)high_rank);
    return low_value + weight * (high_value - low_value);
}

static float bumpy_score_white_component(const bumpy_component_t *component)
{
    const float area_score = bumpy_min_f((float)component->area / 2200.0f, 1.0f);
    const float width_score = bumpy_min_f((float)bumpy_component_width(component) / 80.0f, 1.0f);
    const float height_score = bumpy_min_f((float)bumpy_component_height(component) / 30.0f, 1.0f);
    const float fill_score = bumpy_min_f(component->fill_ratio / 0.80f, 1.0f);
    const float bright_score = bumpy_clamp_f((component->mean_gray - 200.0f) / 55.0f, 0.0f, 1.0f);
    const float border_score = component->touches_border ? 1.0f : 0.0f;

    return 0.30f * area_score +
           0.20f * width_score +
           0.16f * height_score +
           0.14f * fill_score +
           0.10f * bright_score +
           0.10f * border_score;
}

static void bumpy_sort_components_by_area(bumpy_component_t *components, uint8 count)
{
    for (uint8 i = 1U; i < count; i++)
    {
        bumpy_component_t key = components[i];
        int j = (int)i - 1;
        while ((j >= 0) && (components[j].area < key.area))
        {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void bumpy_sort_components_by_score(bumpy_component_t *components, uint8 count)
{
    for (uint8 i = 1U; i < count; i++)
    {
        bumpy_component_t key = components[i];
        int j = (int)i - 1;
        while ((j >= 0) && (components[j].score < key.score))
        {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void bumpy_flood_component(const uint8 *gray,
                                  const uint8 *mask,
                                  uint16 start_index,
                                  bumpy_component_t *out)
{
    uint16 stack_top = 0U;
    uint16 area = 0U;
    uint8 xmin = (uint8)(BUMPY_IMAGE_W - 1U);
    uint8 ymin = (uint8)(BUMPY_IMAGE_H - 1U);
    uint8 xmax = 0U;
    uint8 ymax = 0U;
    uint32 sum_x = 0U;
    uint32 sum_y = 0U;
    uint32 sum_gray = 0U;

    g_bumpy_scratch.stack[stack_top++] = start_index;
    g_bumpy_scratch.visited[start_index] = 1U;

    while (stack_top > 0U)
    {
        const uint16 index = g_bumpy_scratch.stack[--stack_top];
        const uint8 y = (uint8)(index / BUMPY_IMAGE_W);
        const uint8 x = (uint8)(index - (uint16)y * BUMPY_IMAGE_W);

        area++;
        sum_x += x;
        sum_y += y;
        sum_gray += gray[index];

        if (x < xmin) { xmin = x; }
        if (x > xmax) { xmax = x; }
        if (y < ymin) { ymin = y; }
        if (y > ymax) { ymax = y; }

        if (y > 0U)
        {
            const uint16 ni = (uint16)(index - BUMPY_IMAGE_W);
            if ((g_bumpy_scratch.visited[ni] == 0U) && (mask[ni] != 0U))
            {
                g_bumpy_scratch.visited[ni] = 1U;
                g_bumpy_scratch.stack[stack_top++] = ni;
            }
        }
        if (y < (BUMPY_IMAGE_H - 1U))
        {
            const uint16 ni = (uint16)(index + BUMPY_IMAGE_W);
            if ((g_bumpy_scratch.visited[ni] == 0U) && (mask[ni] != 0U))
            {
                g_bumpy_scratch.visited[ni] = 1U;
                g_bumpy_scratch.stack[stack_top++] = ni;
            }
        }
        if (x > 0U)
        {
            const uint16 ni = (uint16)(index - 1U);
            if ((g_bumpy_scratch.visited[ni] == 0U) && (mask[ni] != 0U))
            {
                g_bumpy_scratch.visited[ni] = 1U;
                g_bumpy_scratch.stack[stack_top++] = ni;
            }
        }
        if (x < (BUMPY_IMAGE_W - 1U))
        {
            const uint16 ni = (uint16)(index + 1U);
            if ((g_bumpy_scratch.visited[ni] == 0U) && (mask[ni] != 0U))
            {
                g_bumpy_scratch.visited[ni] = 1U;
                g_bumpy_scratch.stack[stack_top++] = ni;
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
        out->centroid_x = (float)sum_x / (float)area;
        out->centroid_y = (float)sum_y / (float)area;
        out->fill_ratio = (float)area / (float)bbox_area;
        out->touches_border = (uint8)((xmin == 0U) || (ymin == 0U) ||
                                      (xmax == (BUMPY_IMAGE_W - 1U)) ||
                                      (ymax == (BUMPY_IMAGE_H - 1U)));
        out->mean_gray = (float)sum_gray / (float)area;
        out->score = 0.0f;
    }
}

static uint8 bumpy_collect_components(const uint8 *gray, const uint8 *mask)
{
    uint8 component_count = 0U;

    memset(g_bumpy_scratch.visited, 0, sizeof(g_bumpy_scratch.visited));

    for (uint16 i = 0U; i < BUMPY_IMAGE_SIZE; i++)
    {
        if ((g_bumpy_scratch.visited[i] != 0U) || (mask[i] == 0U))
        {
            continue;
        }
        if (component_count >= BUMPY_MAX_COMPONENTS)
        {
            g_bumpy_scratch.visited[i] = 1U;
            continue;
        }
        bumpy_flood_component(gray, mask, i, &g_bumpy_scratch.components[component_count]);
        component_count++;
    }

    bumpy_sort_components_by_area(g_bumpy_scratch.components, component_count);
    return component_count;
}

static uint8 bumpy_filter_candidates(uint8 component_count)
{
    uint8 candidate_count = 0U;

    for (uint8 i = 0U; i < component_count; i++)
    {
        bumpy_component_t component = g_bumpy_scratch.components[i];

        component.score = bumpy_score_white_component(&component);
        if (component.area < BUMPY_MIN_COMPONENT_AREA)
        {
            continue;
        }
        if ((bumpy_component_width(&component) < BUMPY_MIN_COMPONENT_WIDTH) ||
            (bumpy_component_height(&component) < BUMPY_MIN_COMPONENT_HEIGHT))
        {
            continue;
        }
        if (component.fill_ratio < 0.20f)
        {
            continue;
        }

        if (candidate_count < BUMPY_MAX_COMPONENTS)
        {
            g_bumpy_scratch.candidates[candidate_count++] = component;
        }
    }

    bumpy_sort_components_by_score(g_bumpy_scratch.candidates, candidate_count);
    return candidate_count;
}

static void bumpy_estimate_white_threshold(const uint8 *gray,
                                           float *smoothed_threshold,
                                           float *candidate_threshold)
{
    uint16 hist[256];
    uint16 count = 0U;
    double sum = 0.0;
    double sum_sq = 0.0;
    float mean;
    float variance;
    float stddev;
    float p75;
    float p85;
    float p95;
    float p99;
    float candidate;

    bumpy_hist_reset(hist);

    for (uint8 y = BUMPY_ROI_Y0; y <= BUMPY_ROI_Y1; y++)
    {
        for (uint8 x = BUMPY_ROI_X0; x <= BUMPY_ROI_X1; x++)
        {
            const uint8 value = gray[(uint16)y * BUMPY_IMAGE_W + x];
            hist[value]++;
            count++;
            sum += value;
            sum_sq += (double)value * (double)value;
        }
    }

    mean = (count > 0U) ? (float)(sum / (double)count) : 0.0f;
    variance = (count > 0U) ?
        (float)(sum_sq / (double)count - (sum / (double)count) * (sum / (double)count)) : 0.0f;
    if (variance < 0.0f)
    {
        variance = 0.0f;
    }
    stddev = sqrtf(variance);

    p75 = bumpy_percentile_from_hist(hist, count, 75.0f);
    p85 = bumpy_percentile_from_hist(hist, count, 85.0f);
    p95 = bumpy_percentile_from_hist(hist, count, 95.0f);
    p99 = bumpy_percentile_from_hist(hist, count, 99.0f);

    candidate = bumpy_max_f(
        bumpy_max_f(p75 + 0.32f * bumpy_max_f(0.0f, p99 - p75), p85 - 6.0f),
        bumpy_max_f(mean + 0.58f * stddev, p95 - 10.0f));
    candidate = bumpy_clamp_f(candidate, 190.0f, 248.0f);

    *candidate_threshold = candidate;
    if (g_bumpy_has_prev_white_threshold == 0U)
    {
        *smoothed_threshold = candidate;
    }
    else
    {
        *smoothed_threshold = bumpy_clamp_f(0.72f * g_bumpy_prev_white_threshold +
                                            0.28f * candidate,
                                            190.0f,
                                            248.0f);
    }
}

static uint8 bumpy_estimate_row_white_threshold(const uint8 *row,
                                                uint8 length,
                                                float global_threshold)
{
    uint16 hist[256];
    float p60;
    float p75;
    float p90;
    float candidate;

    bumpy_hist_reset(hist);
    for (uint8 i = 0U; i < length; i++)
    {
        hist[row[i]]++;
    }

    p60 = bumpy_percentile_from_hist(hist, length, 60.0f);
    p75 = bumpy_percentile_from_hist(hist, length, 75.0f);
    p90 = bumpy_percentile_from_hist(hist, length, 90.0f);

    candidate = bumpy_max_f(p75,
                            bumpy_max_f(p60 + 0.35f * bumpy_max_f(0.0f, p90 - p60),
                                        global_threshold - 20.0f));
    candidate = bumpy_clamp_f(candidate, 175.0f, bumpy_min_f(250.0f, global_threshold + 6.0f));

    return (uint8)(candidate + 0.5f);
}

static void bumpy_close_small_gaps(uint8 *row_mask, uint8 length)
{
    uint8 x = 0U;
    while (x < length)
    {
        uint8 start;
        uint8 end;
        uint8 gap;
        uint8 left_on;
        uint8 right_on;

        if (row_mask[x] != 0U)
        {
            x++;
            continue;
        }

        start = x;
        while ((x < length) && (row_mask[x] == 0U))
        {
            x++;
        }
        end = (uint8)(x - 1U);
        gap = (uint8)(end - start + 1U);
        left_on = (start > 0U) ? row_mask[start - 1U] : 0U;
        right_on = (x < length) ? row_mask[x] : 0U;

        if ((left_on != 0U) && (right_on != 0U) && (gap <= BUMPY_MAX_ROW_GAP))
        {
            for (uint8 i = start; i <= end; i++)
            {
                row_mask[i] = 1U;
            }
        }
    }
}

static uint8 bumpy_extract_row_runs(const uint8 *row_mask,
                                    uint8 row_length,
                                    uint8 y,
                                    uint8 threshold,
                                    bumpy_white_run_t *runs,
                                    uint8 max_runs)
{
    uint8 run_count = 0U;
    uint8 x = 0U;

    while (x < row_length)
    {
        uint8 start;
        uint8 end;

        if (row_mask[x] == 0U)
        {
            x++;
            continue;
        }

        start = x;
        while ((x < row_length) && (row_mask[x] != 0U))
        {
            x++;
        }
        end = (uint8)(x - 1U);

        if ((end - start + 1U >= BUMPY_MIN_ROW_RUN_WIDTH) && (run_count < max_runs))
        {
            runs[run_count].y = y;
            runs[run_count].xmin = (uint8)(BUMPY_ROI_X0 + start);
            runs[run_count].xmax = (uint8)(BUMPY_ROI_X0 + end);
            runs[run_count].threshold = threshold;
            run_count++;
        }
    }

    return run_count;
}

static int bumpy_choose_best_run(const bumpy_white_run_t *runs,
                                 uint8 run_count,
                                 float anchor_x)
{
    int best_index = -1;
    float best_score = -10000.0f;

    for (uint8 i = 0U; i < run_count; i++)
    {
        const float width_bonus = (float)bumpy_run_width(&runs[i]);
        const float center_penalty = 1.35f * fabsf(bumpy_run_center_x(&runs[i]) - anchor_x);
        const float edge_bonus = ((runs[i].xmin <= (BUMPY_ROI_X0 + 1U)) ||
                                  (runs[i].xmax >= (BUMPY_ROI_X1 - 1U))) ? 4.0f : 0.0f;
        const float score = width_bonus + edge_bonus - center_penalty;

        if ((best_index < 0) || (score > best_score))
        {
            best_index = (int)i;
            best_score = score;
        }
    }

    return best_index;
}

static uint8 bumpy_build_white_scan_mask(const uint8 *gray, float global_threshold)
{
    uint8 row_mask[BUMPY_IMAGE_W];
    bumpy_white_run_t row_runs[BUMPY_IMAGE_W];
    float anchor_x = ((float)BUMPY_IMAGE_W - 1.0f) * 0.5f;
    uint8 run_count = 0U;

    memset(g_bumpy_scratch.scan_white_mask, 0, sizeof(g_bumpy_scratch.scan_white_mask));

    for (int y = (int)BUMPY_ROI_Y1; y >= (int)BUMPY_ROI_Y0; y--)
    {
        const uint8 *row = &gray[(uint16)y * BUMPY_IMAGE_W + BUMPY_ROI_X0];
        const uint8 row_length = (uint8)(BUMPY_ROI_X1 - BUMPY_ROI_X0 + 1U);
        const uint8 row_threshold = bumpy_estimate_row_white_threshold(row, row_length, global_threshold);
        int best_index;
        uint8 local_run_count;

        for (uint8 x = 0U; x < row_length; x++)
        {
            row_mask[x] = (row[x] >= row_threshold) ? 1U : 0U;
        }
        bumpy_close_small_gaps(row_mask, row_length);
        local_run_count = bumpy_extract_row_runs(row_mask, row_length, (uint8)y, row_threshold, row_runs, BUMPY_IMAGE_W);

        best_index = bumpy_choose_best_run(row_runs, local_run_count, anchor_x);
        if (best_index < 0)
        {
            continue;
        }

        if (run_count < BUMPY_MAX_RUNS)
        {
            g_bumpy_scratch.runs[run_count] = row_runs[best_index];
            run_count++;
        }

        for (uint8 x = row_runs[best_index].xmin; x <= row_runs[best_index].xmax; x++)
        {
            g_bumpy_scratch.scan_white_mask[(uint16)y * BUMPY_IMAGE_W + x] = 1U;
        }

        anchor_x = 0.72f * anchor_x + 0.28f * bumpy_run_center_x(&row_runs[best_index]);
    }

    for (uint8 i = 0U; i < (uint8)(run_count / 2U); i++)
    {
        bumpy_white_run_t tmp = g_bumpy_scratch.runs[i];
        g_bumpy_scratch.runs[i] = g_bumpy_scratch.runs[(uint8)(run_count - 1U - i)];
        g_bumpy_scratch.runs[(uint8)(run_count - 1U - i)] = tmp;
    }

    return run_count;
}

static float bumpy_estimate_dark_threshold(const uint8 *gray)
{
    uint16 hist[256];
    uint16 count = 0U;
    float p10;
    float p20;
    float p25;
    float candidate;

    bumpy_hist_reset(hist);

    for (uint8 y = BUMPY_ROI_Y0; y <= BUMPY_ROI_Y1; y++)
    {
        for (uint8 x = BUMPY_ROI_X0; x <= BUMPY_ROI_X1; x++)
        {
            const uint8 value = gray[(uint16)y * BUMPY_IMAGE_W + x];
            hist[value]++;
            count++;
        }
    }

    p10 = bumpy_percentile_from_hist(hist, count, 10.0f);
    p20 = bumpy_percentile_from_hist(hist, count, 20.0f);
    p25 = bumpy_percentile_from_hist(hist, count, 25.0f);

    candidate = bumpy_min_f(p25 - 6.0f, bumpy_min_f(p10 + 18.0f, p20 + 10.0f));
    return bumpy_clamp_f(candidate, 85.0f, 185.0f);
}

static void bumpy_build_supported_dark_mask(const uint8 *gray,
                                            float dark_threshold,
                                            const uint8 *white_mask,
                                            uint8 *rib_mask)
{
    const uint8 threshold = (uint8)(dark_threshold + 0.5f);

    memset(rib_mask, 0, BUMPY_IMAGE_SIZE);

    for (uint8 y = 0U; y < BUMPY_IMAGE_H; y++)
    {
        for (uint8 x = 0U; x < BUMPY_IMAGE_W; x++)
        {
            const uint16 index = (uint16)y * BUMPY_IMAGE_W + x;
            uint8 supported = 0U;

            if (gray[index] > threshold)
            {
                continue;
            }

            for (uint8 offset = 2U; offset < 7U; offset++)
            {
                const int y_above = (int)y - offset;
                const int y_below = (int)y + offset;

                if ((y_above >= 0) && (y_below < (int)BUMPY_IMAGE_H) &&
                    (white_mask[(uint16)y_above * BUMPY_IMAGE_W + x] != 0U) &&
                    (white_mask[(uint16)y_below * BUMPY_IMAGE_W + x] != 0U))
                {
                    supported = 1U;
                    break;
                }
            }

            if (supported)
            {
                rib_mask[index] = 1U;
            }
        }
    }
}

static uint8 bumpy_find_rib_bands(const uint8 *gray,
                                  const uint8 *rib_mask,
                                  bumpy_rib_band_t *bands,
                                  uint8 max_bands)
{
    uint8 row_pixels[BUMPY_IMAGE_H] = {0};
    uint8 grouped_rows[BUMPY_IMAGE_H] = {0};
    uint8 grouped_count = 0U;
    uint8 band_count = 0U;

    for (uint8 y = BUMPY_ROI_Y0; y <= BUMPY_ROI_Y1; y++)
    {
        uint8 count = 0U;
        for (uint8 x = BUMPY_ROI_X0; x <= BUMPY_ROI_X1; x++)
        {
            if (rib_mask[(uint16)y * BUMPY_IMAGE_W + x] != 0U)
            {
                count++;
            }
        }
        row_pixels[y] = count;
        if (count >= BUMPY_MIN_RIB_ROW_PIXELS)
        {
            grouped_rows[grouped_count++] = y;
        }
    }

    for (uint8 i = 0U; i < grouped_count; )
    {
        uint8 ymin = grouped_rows[i];
        uint8 ymax = ymin;
        uint8 xmin = (uint8)(BUMPY_IMAGE_W - 1U);
        uint8 xmax = 0U;
        uint16 area = 0U;
        uint8 max_row_pixels = 0U;
        uint32 gray_sum = 0U;
        uint16 gray_count = 0U;

        while (((uint8)(i + 1U) < grouped_count) &&
               (grouped_rows[(uint8)(i + 1U)] <= (uint8)(ymax + 1U)))
        {
            i++;
            ymax = grouped_rows[i];
        }
        i++;

        if ((ymax - ymin + 1U) < BUMPY_MIN_RIB_HEIGHT)
        {
            continue;
        }

        for (uint8 y = ymin; y <= ymax; y++)
        {
            if (row_pixels[y] > max_row_pixels)
            {
                max_row_pixels = row_pixels[y];
            }
            for (uint8 x = BUMPY_ROI_X0; x <= BUMPY_ROI_X1; x++)
            {
                const uint16 index = (uint16)y * BUMPY_IMAGE_W + x;
                if (rib_mask[index] == 0U)
                {
                    continue;
                }
                if (x < xmin) { xmin = x; }
                if (x > xmax) { xmax = x; }
                area++;
                gray_sum += gray[index];
                gray_count++;
            }
        }

        if (area == 0U)
        {
            continue;
        }
        if ((xmax - xmin + 1U) < BUMPY_MIN_RIB_WIDTH)
        {
            continue;
        }

        if (band_count < max_bands)
        {
            bands[band_count].ymin = ymin;
            bands[band_count].ymax = ymax;
            bands[band_count].xmin = xmin;
            bands[band_count].xmax = xmax;
            bands[band_count].area = area;
            bands[band_count].max_row_pixels = max_row_pixels;
            bands[band_count].mean_gray = (gray_count > 0U) ? (float)gray_sum / (float)gray_count : 0.0f;
            band_count++;
        }
    }

    for (uint8 a = 0U; a + 1U < band_count; a++)
    {
        for (uint8 b = (uint8)(a + 1U); b < band_count; b++)
        {
            if (bumpy_rib_center_y(&bands[a]) > bumpy_rib_center_y(&bands[b]))
            {
                bumpy_rib_band_t tmp = bands[a];
                bands[a] = bands[b];
                bands[b] = tmp;
            }
        }
    }

    return band_count;
}

static void bumpy_summarize_centerline(const bumpy_white_run_t *runs,
                                       uint8 run_count,
                                       float fallback_x,
                                       uint8 has_fallback_x,
                                       bumpy_centerline_summary_t *summary)
{
    float width_sum = 0.0f;

    if (run_count == 0U)
    {
        const float target_x = has_fallback_x ? fallback_x : (((float)BUMPY_IMAGE_W - 1.0f) * 0.5f);
        summary->target_x = target_x;
        summary->steer_error_px = target_x - (((float)BUMPY_IMAGE_W - 1.0f) * 0.5f);
        summary->row_count = 0U;
        summary->bottom_row_count = 0U;
        summary->top_y = -1;
        summary->bottom_y = -1;
        summary->mean_width = 0.0f;
        return;
    }

    summary->row_count = run_count;
    summary->bottom_row_count = 0U;
    summary->top_y = (int8)runs[0].y;
    summary->bottom_y = (int8)runs[run_count - 1U].y;

    for (uint8 i = 0U; i < run_count; i++)
    {
        if (runs[i].y >= (uint8)(BUMPY_ROI_Y1 - BUMPY_BOTTOM_TARGET_ROWS + 1U))
        {
            summary->bottom_row_count++;
        }
        width_sum += bumpy_run_width(&runs[i]);
    }
    summary->mean_width = width_sum / (float)run_count;

    {
        const uint8 use_bottom = (summary->bottom_row_count > 0U) ? 1U : 0U;
        const uint8 target_count = use_bottom ?
            summary->bottom_row_count :
            ((run_count < 8U) ? run_count : 8U);
        const uint8 start_index = use_bottom ? 0U : (uint8)(run_count - target_count);
        float weighted_sum = 0.0f;
        float weight_total = 0.0f;

        for (uint8 i = 0U; i < run_count; i++)
        {
            const uint8 include = use_bottom ?
                (runs[i].y >= (uint8)(BUMPY_ROI_Y1 - BUMPY_BOTTOM_TARGET_ROWS + 1U)) :
                (i >= start_index);

            if (include)
            {
                const float weight = 1.0f + 0.08f * bumpy_max_f(0.0f, (float)runs[i].y - (float)BUMPY_ROI_Y0);
                weighted_sum += weight * bumpy_run_center_x(&runs[i]);
                weight_total += weight;
            }
        }

        summary->target_x = (weight_total > 0.0f) ?
            (weighted_sum / weight_total) :
            (has_fallback_x ? fallback_x : (((float)BUMPY_IMAGE_W - 1.0f) * 0.5f));
        summary->steer_error_px = summary->target_x - (((float)BUMPY_IMAGE_W - 1.0f) * 0.5f);
    }
}

static bumpy_phase_e bumpy_classify_phase(uint8 component_found,
                                           const bumpy_centerline_summary_t *centerline,
                                           uint8 rib_count)
{
    if (rib_count > 0U)
    {
        return BUMPY_PHASE_INSIDE;
    }
    if ((component_found == 0U) && (centerline->bottom_y >= 56) && (centerline->bottom_row_count >= 2U))
    {
        return BUMPY_PHASE_EXIT;
    }
    if ((component_found == 0U) && (centerline->row_count < 4U))
    {
        return BUMPY_PHASE_UNCERTAIN;
    }
    if ((centerline->top_y >= 0) && (centerline->top_y <= 18) && (centerline->bottom_row_count <= 5U))
    {
        return BUMPY_PHASE_APPROACH;
    }
    if ((centerline->bottom_y >= 54) && (centerline->top_y >= 20))
    {
        return BUMPY_PHASE_EXIT;
    }
    if ((centerline->row_count >= 8U) && (centerline->bottom_row_count >= 4U))
    {
        return BUMPY_PHASE_WHITE_SURFACE_ONLY;
    }
    if (component_found != 0U)
    {
        return BUMPY_PHASE_APPROACH;
    }
    return BUMPY_PHASE_UNCERTAIN;
}

static bumpy_mode_e bumpy_mode_from_phase(bumpy_phase_e phase)
{
    switch (phase)
    {
        case BUMPY_PHASE_APPROACH:
            return BUMPY_MODE_SEEK_ENTRANCE;
        case BUMPY_PHASE_INSIDE:
            return BUMPY_MODE_FOLLOW_CENTERLINE;
        case BUMPY_PHASE_EXIT:
            return BUMPY_MODE_HOLD_EXIT_LINE;
        case BUMPY_PHASE_WHITE_SURFACE_ONLY:
            return BUMPY_MODE_HOLD_WHITE_SURFACE;
        case BUMPY_PHASE_UNCERTAIN:
        default:
            return BUMPY_MODE_FALLBACK_SEARCH;
    }
}

static void bumpy_detect_frame(const uint8 *gray, bumpy_detect_result_t *result)
{
    uint8 component_count;
    uint8 candidate_count;
    uint8 run_count;
    uint8 rib_count;
    float white_threshold;
    float white_candidate;
    float dark_threshold;

    memset(result, 0, sizeof(*result));
    result->phase = BUMPY_PHASE_UNCERTAIN;
    result->mode = BUMPY_MODE_FALLBACK_SEARCH;
    result->centerline.top_y = -1;
    result->centerline.bottom_y = -1;

    bumpy_estimate_white_threshold(gray, &white_threshold, &white_candidate);
    g_bumpy_prev_white_threshold = white_threshold;
    g_bumpy_has_prev_white_threshold = 1U;

    for (uint16 i = 0U; i < BUMPY_IMAGE_SIZE; i++)
    {
        g_bumpy_scratch.global_white_mask[i] =
            (gray[i] >= (uint8)(white_threshold + 0.5f)) ? 1U : 0U;
    }

    component_count = bumpy_collect_components(gray, g_bumpy_scratch.global_white_mask);
    candidate_count = bumpy_filter_candidates(component_count);
    run_count = bumpy_build_white_scan_mask(gray, white_threshold);

    for (uint16 i = 0U; i < BUMPY_IMAGE_SIZE; i++)
    {
        g_bumpy_scratch.white_mask[i] =
            (uint8)((g_bumpy_scratch.global_white_mask[i] != 0U) ||
                    (g_bumpy_scratch.scan_white_mask[i] != 0U));
    }

    if (candidate_count > 0U)
    {
        result->best_component = g_bumpy_scratch.candidates[0];
        result->best_component_found = 1U;
    }

    bumpy_summarize_centerline(g_bumpy_scratch.runs,
                               run_count,
                               result->best_component.centroid_x,
                               result->best_component_found,
                               &result->centerline);

    dark_threshold = bumpy_estimate_dark_threshold(gray);
    bumpy_build_supported_dark_mask(gray,
                                    dark_threshold,
                                    g_bumpy_scratch.white_mask,
                                    g_bumpy_scratch.rib_mask);
    rib_count = bumpy_find_rib_bands(gray,
                                     g_bumpy_scratch.rib_mask,
                                     g_bumpy_scratch.rib_bands,
                                     BUMPY_MAX_RIB_BANDS);

    result->phase = bumpy_classify_phase(result->best_component_found,
                                         &result->centerline,
                                         rib_count);
    result->mode = bumpy_mode_from_phase(result->phase);
    result->detected = (uint8)((result->best_component_found != 0U) || (run_count > 0U));
    result->white_threshold = white_candidate;
    result->dark_threshold = dark_threshold;
    result->component_count = component_count;
    result->candidate_count = candidate_count;
    result->run_count = run_count;
    result->rib_count = rib_count;
}

static void bumpy_clear_frame_result(bumpy_vision_frame_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->phase = BUMPY_PHASE_UNCERTAIN;
    result->mode = BUMPY_MODE_FALLBACK_SEARCH;
    result->centerline_top_y = 0xFFU;
    result->centerline_bottom_y = 0xFFU;
    result->bbox_xmin = 0xFFU;
    result->bbox_ymin = 0xFFU;
    result->bbox_xmax = 0xFFU;
    result->bbox_ymax = 0xFFU;
}

static void bumpy_copy_detect_to_frame_result(const bumpy_detect_result_t *detect,
                                              bumpy_vision_frame_result_t *result)
{
    float confidence = 0.0f;
    IPM_Point_t ipm_point;
    int16 target_x_px_int;
    uint8 target_x_px_u8;
    uint8 target_y_px_u8;

    bumpy_clear_frame_result(result);

    result->detected = detect->detected;
    result->phase = (uint8)detect->phase;
    result->mode = (uint8)detect->mode;

    result->best_component_found = detect->best_component_found;
    result->component_count = detect->component_count;
    result->candidate_count = detect->candidate_count;
    result->run_count = detect->run_count;
    result->rib_count = detect->rib_count;

    result->centerline_rows = detect->centerline.row_count;
    result->centerline_bottom_rows = detect->centerline.bottom_row_count;
    result->centerline_top_y = (detect->centerline.top_y < 0) ? 0xFFU : (uint8)detect->centerline.top_y;
    result->centerline_bottom_y = (detect->centerline.bottom_y < 0) ? 0xFFU : (uint8)detect->centerline.bottom_y;

    result->target_x_px_x100 = bumpy_float_to_i16_x100(detect->centerline.target_x);
    result->steer_error_px_x100 = bumpy_float_to_i16_x100(detect->centerline.steer_error_px);
    target_x_px_int = (int16)(detect->centerline.target_x + 0.5f);
    if (target_x_px_int < 0)
    {
        target_x_px_int = 0;
    }
    if (target_x_px_int > (BUMPY_IMAGE_W - 1))
    {
        target_x_px_int = (BUMPY_IMAGE_W - 1);
    }
    target_x_px_u8 = (uint8)target_x_px_int;
    target_y_px_u8 = (result->centerline_bottom_y == 0xFFU) ? (BUMPY_IMAGE_H - 1U) : result->centerline_bottom_y;
    ipm_point = IPM_GetPhysicalCoord(target_x_px_u8, target_y_px_u8);
    if (ipm_point.is_valid)
    {
        result->target_x_ipm_mm = ipm_point.x_mm;
        result->target_y_ipm_mm = ipm_point.y_mm;
        result->steer_error_ipm_mm = ipm_point.x_mm;
    }
    else
    {
        result->target_x_ipm_mm = 0;
        result->target_y_ipm_mm = 0;
        result->steer_error_ipm_mm = 0;
    }

    result->white_threshold_x10 = (uint16)(detect->white_threshold * 10.0f);
    result->dark_threshold_x10 = (uint16)(detect->dark_threshold * 10.0f);

    if (detect->best_component_found)
    {
        const bumpy_component_t *best = &detect->best_component;
        result->bbox_xmin = best->xmin;
        result->bbox_ymin = best->ymin;
        result->bbox_xmax = best->xmax;
        result->bbox_ymax = best->ymax;
        result->bbox_area = (uint16)(bumpy_component_width(best) * bumpy_component_height(best));
        result->local_s_mm = (uint16)((BUMPY_IMAGE_H - 1U - best->ymax) * 30U);
        confidence += 0.55f * best->score;
    }
    else
    {
        result->bbox_area = 0U;
        result->local_s_mm = 0U;
    }

    confidence += 0.30f * bumpy_min_f((float)detect->centerline.row_count / 20.0f, 1.0f);
    confidence += 0.15f * bumpy_min_f((float)detect->rib_count / 2.0f, 1.0f);
    result->confidence_u16 = bumpy_confidence_to_u16(bumpy_clamp_f(confidence, 0.0f, 1.0f));
}

static void bumpy_update_filter(const bumpy_vision_frame_result_t *raw)
{
    bumpy_vision_output_t next = g_bumpy_output_shadow;
    const uint8 prev_stable = next.stable_detected;

    next.frame_id++;
    next.raw = *raw;
    next.raw_detected = raw->detected;
    next.start_seen = 0U;
    next.end_seen = 0U;

#if BUMPY_VISION_SMOOTH_ENABLE
    if (raw->detected)
    {
        if (next.detected_streak < 255U)
        {
            next.detected_streak++;
        }
        next.lost_streak = 0U;
        if (next.detected_streak >= BUMPY_CONFIRM_FRAMES)
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
        if (next.lost_streak >= BUMPY_LOST_HOLD_FRAMES)
        {
            next.stable_detected = 0U;
        }
    }

    if (next.stable_detected)
    {
        if (raw->detected)
        {
            next.stable = *raw;
        }
        next.stable.detected = 1U;
    }
    else
    {
        bumpy_clear_frame_result(&next.stable);
    }
#else
    next.detected_streak = raw->detected ? 1U : 0U;
    next.lost_streak = raw->detected ? 0U : 1U;
    next.stable_detected = raw->detected;
    next.stable = *raw;
#endif

    if ((prev_stable == 0U) && (next.stable_detected != 0U))
    {
        next.start_seen = 1U;
    }
    if ((prev_stable != 0U) && (next.stable_detected == 0U))
    {
        next.end_seen = 1U;
    }

    g_bumpy_output_shadow = next;
    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = next;
    g_bumpy_vision_output_write_busy = 0U;
}

void bumpy_vision_init(void)
{
    bumpy_vision_reset_filter();

#if BUMPY_VISION_PROFILE_ENABLE
    timer_init(BUMPY_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(BUMPY_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_bumpy_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_bumpy_vision_frame_profiler);
    g_bumpy_last_frame_time_us = timer_get(BUMPY_VISION_PROFILE_TIMER);
#endif
}

void bumpy_vision_reset_filter(void)
{
    bumpy_vision_output_t empty;

    memset(&empty, 0, sizeof(empty));
    bumpy_clear_frame_result(&empty.raw);
    bumpy_clear_frame_result(&empty.stable);

    g_bumpy_output_shadow = empty;
    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = empty;
    g_bumpy_vision_output_write_busy = 0U;

    g_bumpy_prev_white_threshold = 0.0f;
    g_bumpy_has_prev_white_threshold = 0U;
}

const volatile bumpy_vision_output_t *bumpy_vision_get_output(void)
{
    return &g_bumpy_vision_output;
}

void bumpy_vision_process_camera_frame(const uint8 *gray)
{
    bumpy_detect_result_t detect;
    bumpy_vision_frame_result_t raw;

    if (gray == NULL)
    {
        return;
    }

#if BUMPY_VISION_PROFILE_ENABLE
    {
        const uint32 now_us = timer_get(BUMPY_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_bumpy_vision_frame_profiler,
                                (uint32)(now_us - g_bumpy_last_frame_time_us));
        g_bumpy_last_frame_time_us = now_us;
    }
    RUNTIME_PROFILE_BEGIN(g_bumpy_vision_cost_profiler, BUMPY_VISION_PROFILE_TIMER);
#endif

    bumpy_detect_frame(gray, &detect);
    bumpy_copy_detect_to_frame_result(&detect, &raw);
    bumpy_update_filter(&raw);

#if BUMPY_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_bumpy_vision_cost_profiler, BUMPY_VISION_PROFILE_TIMER);
#endif
}

#endif
