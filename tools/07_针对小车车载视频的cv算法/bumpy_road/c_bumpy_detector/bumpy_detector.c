/*
Timing note:
- benchmark command:
  powershell -ExecutionPolicy Bypass -File tools\07_针对小车车载视频的cv算法\bumpy_road\c_bumpy_detector\run_bumpy_pc_pipeline.ps1
- compiler: Visual Studio 2022 cl /O2
- dataset: 631 frames from data\frames\颠簸路段
- total_us: 107431.00
- avg_us: 170.26
- min_us: 55.00
- max_us: 1541.00
- fps: 5873.54
- macros: BUMPY_PC_ENABLE_TIMING / BUMPY_PC_WRITE_CSV
*/

#include "bumpy_detector.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static float bumpy_min_f(float a, float b)
{
    return a < b ? a : b;
}

static float bumpy_max_f(float a, float b)
{
    return a > b ? a : b;
}

static float bumpy_clamp_f(float value, float low, float high)
{
    return bumpy_max_f(low, bumpy_min_f(value, high));
}

static int bumpy_component_width(const BumpyComponent *component)
{
    return component->xmax - component->xmin + 1;
}

static int bumpy_component_height(const BumpyComponent *component)
{
    return component->ymax - component->ymin + 1;
}

static int bumpy_run_width(const BumpyWhiteRun *run)
{
    return run->xmax - run->xmin + 1;
}

static float bumpy_run_center_x(const BumpyWhiteRun *run)
{
    return 0.5f * (float)(run->xmin + run->xmax);
}

static int bumpy_rib_width(const BumpyRibBand *band)
{
    return band->xmax - band->xmin + 1;
}

static int bumpy_rib_height(const BumpyRibBand *band)
{
    return band->ymax - band->ymin + 1;
}

static float bumpy_rib_center_y(const BumpyRibBand *band)
{
    return 0.5f * (float)(band->ymin + band->ymax);
}

void bumpy_detect_result_clear(BumpyDetectResult *result)
{
    if (result == NULL) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->phase = BUMPY_PHASE_UNCERTAIN;
    result->mode = BUMPY_MODE_FALLBACK_SEARCH;
    result->centerline.top_y = -1;
    result->centerline.bottom_y = -1;
}

const char *bumpy_phase_name(BumpyPhase phase)
{
    switch (phase) {
    case BUMPY_PHASE_APPROACH:
        return "approach_bumpy";
    case BUMPY_PHASE_INSIDE:
        return "inside_bumpy";
    case BUMPY_PHASE_EXIT:
        return "exit_bumpy";
    case BUMPY_PHASE_WHITE_SURFACE_ONLY:
        return "white_surface_only";
    case BUMPY_PHASE_UNCERTAIN:
    default:
        return "uncertain";
    }
}

const char *bumpy_mode_name(BumpyControllerMode mode)
{
    switch (mode) {
    case BUMPY_MODE_SEEK_ENTRANCE:
        return "seek_bumpy_entrance";
    case BUMPY_MODE_FOLLOW_CENTERLINE:
        return "follow_bumpy_centerline";
    case BUMPY_MODE_HOLD_EXIT_LINE:
        return "hold_exit_line";
    case BUMPY_MODE_HOLD_WHITE_SURFACE:
        return "hold_white_surface";
    case BUMPY_MODE_FALLBACK_SEARCH:
    default:
        return "fallback_search";
    }
}

static void bumpy_hist_reset(int *hist)
{
    memset(hist, 0, 256 * sizeof(hist[0]));
}

static float bumpy_hist_value_at_rank(const int *hist, int total, float rank)
{
    int cumulative = 0;
    int target = (int)rank;
    if (target < 0) {
        target = 0;
    }
    if (target >= total) {
        target = total - 1;
    }

    for (int value = 0; value < 256; value++) {
        cumulative += hist[value];
        if (cumulative > target) {
            return (float)value;
        }
    }
    return 255.0f;
}

static float bumpy_percentile_from_hist(const int *hist, int total, float p)
{
    float index;
    float low_value;
    float high_value;
    int low_rank;
    int high_rank;
    float weight;

    if (total <= 0) {
        return 0.0f;
    }

    if (p <= 0.0f) {
        return bumpy_hist_value_at_rank(hist, total, 0.0f);
    }
    if (p >= 100.0f) {
        return bumpy_hist_value_at_rank(hist, total, (float)(total - 1));
    }

    index = (p / 100.0f) * (float)(total - 1);
    low_rank = (int)floorf(index);
    high_rank = (int)ceilf(index);
    weight = index - (float)low_rank;

    low_value = bumpy_hist_value_at_rank(hist, total, (float)low_rank);
    high_value = bumpy_hist_value_at_rank(hist, total, (float)high_rank);
    return low_value + weight * (high_value - low_value);
}

static void bumpy_build_roi_histogram(
    const uint8_t *gray,
    int width,
    int height,
    int x0,
    int x1,
    int y0,
    int y1,
    int *hist,
    int *count,
    double *sum,
    double *sum_sq)
{
    int local_count = 0;
    double local_sum = 0.0;
    double local_sum_sq = 0.0;

    bumpy_hist_reset(hist);
    for (int y = y0; y <= y1 && y < height; y++) {
        for (int x = x0; x <= x1 && x < width; x++) {
            const uint8_t value = gray[y * width + x];
            hist[value]++;
            local_count++;
            local_sum += (double)value;
            local_sum_sq += (double)value * (double)value;
        }
    }

    *count = local_count;
    *sum = local_sum;
    *sum_sq = local_sum_sq;
}

static float bumpy_score_white_component(const BumpyComponent *component)
{
    const float area_score = bumpy_min_f((float)component->area / 2600.0f, 1.0f);
    const float width_score = bumpy_min_f((float)bumpy_component_width(component) / 82.0f, 1.0f);
    const float height_score = bumpy_min_f((float)bumpy_component_height(component) / 34.0f, 1.0f);
    const float fill_score = bumpy_min_f(component->fill_ratio / 0.82f, 1.0f);
    const float brightness_score = bumpy_min_f(
        bumpy_max_f((component->mean_gray - 210.0f) / 45.0f, 0.0f),
        1.0f);
    const float border_score = component->touches_border ? 1.0f : 0.0f;

    return 0.30f * area_score
        + 0.18f * width_score
        + 0.15f * height_score
        + 0.14f * fill_score
        + 0.13f * brightness_score
        + 0.10f * border_score;
}

static void bumpy_sort_components_by_area(BumpyComponent *components, int count)
{
    for (int i = 1; i < count; i++) {
        BumpyComponent key = components[i];
        int j = i - 1;
        while (j >= 0 && components[j].area < key.area) {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void bumpy_sort_components_by_score(BumpyComponent *components, int count)
{
    for (int i = 1; i < count; i++) {
        BumpyComponent key = components[i];
        int j = i - 1;
        while (j >= 0 && components[j].score < key.score) {
            components[j + 1] = components[j];
            j--;
        }
        components[j + 1] = key;
    }
}

static void bumpy_flood_component(
    const uint8_t *gray,
    const uint8_t *mask,
    int width,
    int height,
    int start_index,
    BumpyDetectScratch *scratch,
    BumpyComponent *out)
{
    int stack_top = 0;
    int area = 0;
    int xmin = width - 1;
    int ymin = height - 1;
    int xmax = 0;
    int ymax = 0;
    int sum_x = 0;
    int sum_y = 0;
    int sum_gray = 0;

    scratch->stack[stack_top++] = start_index;
    scratch->visited[start_index] = 1;

    while (stack_top > 0) {
        const int index = scratch->stack[--stack_top];
        const int y = index / width;
        const int x = index - y * width;
        const int neighbors[4] = { index - width, index + width, index - 1, index + 1 };

        area++;
        sum_x += x;
        sum_y += y;
        sum_gray += gray[index];

        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;
        if (y < ymin) ymin = y;
        if (y > ymax) ymax = y;

        for (int n = 0; n < 4; n++) {
            const int ni = neighbors[n];
            int valid = 1;
            if (n == 0 && y == 0) valid = 0;
            if (n == 1 && y == height - 1) valid = 0;
            if (n == 2 && x == 0) valid = 0;
            if (n == 3 && x == width - 1) valid = 0;
            if (!valid) {
                continue;
            }
            if (!scratch->visited[ni] && mask[ni]) {
                scratch->visited[ni] = 1;
                scratch->stack[stack_top++] = ni;
            }
        }
    }

    out->area = area;
    out->xmin = xmin;
    out->ymin = ymin;
    out->xmax = xmax;
    out->ymax = ymax;
    out->centroid_x = (float)sum_x / (float)area;
    out->centroid_y = (float)sum_y / (float)area;
    out->fill_ratio = (float)area / (float)((xmax - xmin + 1) * (ymax - ymin + 1));
    out->touches_border = (uint8_t)(xmin == 0 || ymin == 0 || xmax == width - 1 || ymax == height - 1);
    out->mean_gray = (float)sum_gray / (float)area;
    out->score = 0.0f;
}

static int bumpy_collect_components(
    const uint8_t *gray,
    const uint8_t *mask,
    int width,
    int height,
    BumpyDetectScratch *scratch)
{
    const int pixels = width * height;
    int component_count = 0;

    memset(scratch->visited, 0, (size_t)pixels);
    for (int i = 0; i < pixels; i++) {
        if (scratch->visited[i] || !mask[i]) {
            continue;
        }
        if (component_count >= BUMPY_MAX_COMPONENTS) {
            scratch->visited[i] = 1;
            continue;
        }
        bumpy_flood_component(gray, mask, width, height, i, scratch, &scratch->components[component_count]);
        component_count++;
    }

    bumpy_sort_components_by_area(scratch->components, component_count);
    return component_count;
}

static int bumpy_filter_candidates(
    const BumpyComponent *components,
    int component_count,
    BumpyDetectScratch *scratch)
{
    int candidate_count = 0;
    for (int i = 0; i < component_count; i++) {
        BumpyComponent component = components[i];
        component.score = bumpy_score_white_component(&component);
        if (component.area < BUMPY_MIN_COMPONENT_AREA) {
            continue;
        }
        if (bumpy_component_width(&component) < BUMPY_MIN_COMPONENT_WIDTH ||
            bumpy_component_height(&component) < BUMPY_MIN_COMPONENT_HEIGHT) {
            continue;
        }
        if (component.fill_ratio < 0.22f) {
            continue;
        }
        if (candidate_count < BUMPY_MAX_COMPONENTS) {
            scratch->candidates[candidate_count++] = component;
        }
    }
    bumpy_sort_components_by_score(scratch->candidates, candidate_count);
    return candidate_count;
}

static void bumpy_estimate_white_threshold(
    const uint8_t *gray,
    int width,
    int height,
    float prev_threshold,
    int has_prev_threshold,
    float *smoothed_threshold,
    float *candidate_threshold)
{
    int hist[256];
    int count = 0;
    double sum = 0.0;
    double sum_sq = 0.0;
    float mean;
    float variance;
    float stddev;
    float p75;
    float p85;
    float p95;
    float p99;
    float tail_candidate;
    float shoulder_candidate;
    float sigma_candidate;
    float highlight_candidate;
    float candidate;

    bumpy_build_roi_histogram(
        gray,
        width,
        height,
        BUMPY_ROI_X0,
        BUMPY_ROI_X1,
        BUMPY_ROI_Y0,
        BUMPY_ROI_Y1,
        hist,
        &count,
        &sum,
        &sum_sq);
    mean = count > 0 ? (float)(sum / (double)count) : 0.0f;
    variance = count > 0 ? (float)(sum_sq / (double)count - (sum / (double)count) * (sum / (double)count)) : 0.0f;
    if (variance < 0.0f) {
        variance = 0.0f;
    }
    stddev = sqrtf(variance);

    p75 = bumpy_percentile_from_hist(hist, count, 75.0f);
    p85 = bumpy_percentile_from_hist(hist, count, 85.0f);
    p95 = bumpy_percentile_from_hist(hist, count, 95.0f);
    p99 = bumpy_percentile_from_hist(hist, count, 99.0f);

    tail_candidate = p75 + 0.32f * bumpy_max_f(0.0f, p99 - p75);
    shoulder_candidate = p85 - 6.0f;
    sigma_candidate = mean + 0.58f * stddev;
    highlight_candidate = p95 - 10.0f;
    candidate = bumpy_clamp_f(
        bumpy_max_f(
            bumpy_max_f(tail_candidate, shoulder_candidate),
            bumpy_max_f(sigma_candidate, highlight_candidate)),
        200.0f,
        248.0f);

    *candidate_threshold = candidate;
    if (!has_prev_threshold) {
        *smoothed_threshold = candidate;
    } else {
        *smoothed_threshold = bumpy_clamp_f(0.72f * prev_threshold + 0.28f * candidate, 200.0f, 248.0f);
    }
}

static int bumpy_estimate_row_white_threshold(const uint8_t *row, int length, float global_threshold)
{
    int hist[256];
    float p60;
    float p75;
    float p90;
    float candidate;
    bumpy_hist_reset(hist);
    for (int i = 0; i < length; i++) {
        hist[row[i]]++;
    }
    p60 = bumpy_percentile_from_hist(hist, length, 60.0f);
    p75 = bumpy_percentile_from_hist(hist, length, 75.0f);
    p90 = bumpy_percentile_from_hist(hist, length, 90.0f);
    candidate = bumpy_max_f(
        p75,
        bumpy_max_f(p60 + 0.35f * bumpy_max_f(0.0f, p90 - p60), global_threshold - 20.0f));
    return (int)lroundf(bumpy_clamp_f(candidate, 185.0f, bumpy_min_f(250.0f, global_threshold + 6.0f)));
}

static void bumpy_close_small_gaps(uint8_t *row_mask, int length)
{
    int x = 0;
    while (x < length) {
        int start;
        int end;
        int gap;
        int left_on;
        int right_on;

        if (row_mask[x]) {
            x++;
            continue;
        }
        start = x;
        while (x < length && !row_mask[x]) {
            x++;
        }
        end = x - 1;
        gap = end - start + 1;
        left_on = start > 0 && row_mask[start - 1];
        right_on = x < length && row_mask[x];
        if (left_on && right_on && gap <= BUMPY_MAX_ROW_GAP) {
            for (int i = start; i <= end; i++) {
                row_mask[i] = 1;
            }
        }
    }
}

static int bumpy_extract_row_runs(
    const uint8_t *row_mask,
    int row_length,
    int y,
    int threshold,
    BumpyWhiteRun *runs,
    int max_runs)
{
    int run_count = 0;
    int x = 0;
    while (x < row_length) {
        int start;
        int end;
        if (!row_mask[x]) {
            x++;
            continue;
        }
        start = x;
        while (x < row_length && row_mask[x]) {
            x++;
        }
        end = x - 1;
        if (end - start + 1 >= BUMPY_MIN_ROW_RUN_WIDTH && run_count < max_runs) {
            runs[run_count].y = y;
            runs[run_count].xmin = BUMPY_ROI_X0 + start;
            runs[run_count].xmax = BUMPY_ROI_X0 + end;
            runs[run_count].threshold = threshold;
            run_count++;
        }
    }
    return run_count;
}

static int bumpy_choose_best_run(const BumpyWhiteRun *runs, int run_count, float anchor_x)
{
    int best_index = -1;
    float best_score = 0.0f;
    for (int i = 0; i < run_count; i++) {
        const float width_bonus = (float)bumpy_run_width(&runs[i]);
        const float center_penalty = 1.35f * fabsf(bumpy_run_center_x(&runs[i]) - anchor_x);
        const float edge_bonus = (runs[i].xmin <= BUMPY_ROI_X0 + 1 || runs[i].xmax >= BUMPY_ROI_X1 - 1) ? 4.0f : 0.0f;
        const float score = width_bonus + edge_bonus - center_penalty;
        if (best_index < 0 || score > best_score) {
            best_index = i;
            best_score = score;
        }
    }
    return best_index;
}

static int bumpy_build_white_scan_mask(
    const uint8_t *gray,
    int width,
    int height,
    float global_threshold,
    BumpyDetectScratch *scratch)
{
    uint8_t row_mask[BUMPY_MAX_WIDTH];
    BumpyWhiteRun row_runs[BUMPY_MAX_WIDTH];
    float anchor_x = BUMPY_IMAGE_CENTER_X;
    int run_count = 0;
    (void)height;

    memset(scratch->scan_white_mask, 0, (size_t)(width * height));
    for (int y = BUMPY_ROI_Y1; y >= BUMPY_ROI_Y0; y--) {
        const uint8_t *row = &gray[y * width + BUMPY_ROI_X0];
        const int row_length = BUMPY_ROI_X1 - BUMPY_ROI_X0 + 1;
        const int row_threshold = bumpy_estimate_row_white_threshold(row, row_length, global_threshold);
        int local_run_count;
        int best_index;

        for (int x = 0; x < row_length; x++) {
            row_mask[x] = row[x] >= row_threshold ? 1u : 0u;
        }
        bumpy_close_small_gaps(row_mask, row_length);
        local_run_count = bumpy_extract_row_runs(row_mask, row_length, y, row_threshold, row_runs, BUMPY_MAX_WIDTH);
        best_index = bumpy_choose_best_run(row_runs, local_run_count, anchor_x);
        if (best_index < 0) {
            continue;
        }
        if (run_count < BUMPY_MAX_RUNS) {
            scratch->runs[run_count] = row_runs[best_index];
            run_count++;
        }
        for (int x = row_runs[best_index].xmin; x <= row_runs[best_index].xmax; x++) {
            scratch->scan_white_mask[y * width + x] = 1;
        }
        anchor_x = 0.72f * anchor_x + 0.28f * bumpy_run_center_x(&row_runs[best_index]);
    }

    for (int i = 0; i < run_count / 2; i++) {
        BumpyWhiteRun tmp = scratch->runs[i];
        scratch->runs[i] = scratch->runs[run_count - 1 - i];
        scratch->runs[run_count - 1 - i] = tmp;
    }
    return run_count;
}

static float bumpy_estimate_dark_threshold(const uint8_t *gray, int width, int height)
{
    int hist[256];
    int count = 0;
    double sum = 0.0;
    double sum_sq = 0.0;
    float p10;
    float p20;
    float p25;
    float candidate;

    bumpy_build_roi_histogram(
        gray,
        width,
        height,
        BUMPY_ROI_X0,
        BUMPY_ROI_X1,
        BUMPY_ROI_Y0,
        BUMPY_ROI_Y1,
        hist,
        &count,
        &sum,
        &sum_sq);
    (void)sum;
    (void)sum_sq;

    p10 = bumpy_percentile_from_hist(hist, count, 10.0f);
    p20 = bumpy_percentile_from_hist(hist, count, 20.0f);
    p25 = bumpy_percentile_from_hist(hist, count, 25.0f);
    candidate = bumpy_min_f(p25 - 6.0f, bumpy_min_f(p10 + 18.0f, p20 + 10.0f));
    return bumpy_clamp_f(candidate, 95.0f, 185.0f);
}

static void bumpy_build_supported_dark_mask(
    const uint8_t *gray,
    int width,
    int height,
    float dark_threshold,
    const uint8_t *white_mask,
    uint8_t *rib_mask)
{
    const int threshold = (int)lroundf(dark_threshold);
    memset(rib_mask, 0, (size_t)(width * height));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const int index = y * width + x;
            int supported = 0;

            if (gray[index] > threshold) {
                continue;
            }
            for (int offset = 2; offset < 7; offset++) {
                const int y_above = y - offset;
                const int y_below = y + offset;
                if (y_above >= 0 && y_below < height &&
                    white_mask[y_above * width + x] &&
                    white_mask[y_below * width + x]) {
                    supported = 1;
                    break;
                }
            }
            if (supported) {
                rib_mask[index] = 1;
            }
        }
    }
}

static int bumpy_find_rib_bands(
    const uint8_t *gray,
    int width,
    int height,
    const uint8_t *rib_mask,
    BumpyRibBand *bands,
    int max_bands)
{
    int row_pixels[BUMPY_MAX_HEIGHT];
    int row_flags[BUMPY_MAX_HEIGHT];
    int band_count = 0;
    int grouped_rows[BUMPY_MAX_HEIGHT];
    int grouped_count = 0;

    memset(row_pixels, 0, sizeof(row_pixels));
    memset(row_flags, 0, sizeof(row_flags));

    for (int y = BUMPY_ROI_Y0; y <= BUMPY_ROI_Y1 && y < height; y++) {
        int count = 0;
        for (int x = BUMPY_ROI_X0; x <= BUMPY_ROI_X1 && x < width; x++) {
            if (rib_mask[y * width + x]) {
                count++;
            }
        }
        row_pixels[y] = count;
        if (count >= BUMPY_MIN_RIB_ROW_PIXELS) {
            grouped_rows[grouped_count++] = y;
            row_flags[y] = 1;
        }
    }

    for (int i = 0; i < grouped_count; ) {
        int ymin = grouped_rows[i];
        int ymax = ymin;
        int xmin = width - 1;
        int xmax = 0;
        int area = 0;
        int max_row_pixels = 0;
        int gray_sum = 0;
        int gray_count = 0;

        while (i + 1 < grouped_count && grouped_rows[i + 1] <= ymax + 1) {
            i++;
            ymax = grouped_rows[i];
        }
        i++;

        if (ymax - ymin + 1 < BUMPY_MIN_RIB_HEIGHT) {
            continue;
        }

        for (int y = ymin; y <= ymax; y++) {
            if (row_pixels[y] > max_row_pixels) {
                max_row_pixels = row_pixels[y];
            }
            for (int x = BUMPY_ROI_X0; x <= BUMPY_ROI_X1 && x < width; x++) {
                if (!rib_mask[y * width + x]) {
                    continue;
                }
                if (x < xmin) xmin = x;
                if (x > xmax) xmax = x;
                area++;
                gray_sum += gray[y * width + x];
                gray_count++;
            }
        }

        if (area <= 0) {
            continue;
        }
        if (xmax - xmin + 1 < BUMPY_MIN_RIB_WIDTH) {
            continue;
        }
        if (band_count < max_bands) {
            bands[band_count].ymin = ymin;
            bands[band_count].ymax = ymax;
            bands[band_count].xmin = xmin;
            bands[band_count].xmax = xmax;
            bands[band_count].area = area;
            bands[band_count].max_row_pixels = max_row_pixels;
            bands[band_count].mean_gray = gray_count > 0 ? (float)gray_sum / (float)gray_count : 0.0f;
            band_count++;
        }
    }

    for (int a = 0; a < band_count - 1; a++) {
        for (int b = a + 1; b < band_count; b++) {
            if (bumpy_rib_center_y(&bands[a]) > bumpy_rib_center_y(&bands[b])) {
                BumpyRibBand tmp = bands[a];
                bands[a] = bands[b];
                bands[b] = tmp;
            }
        }
    }

    return band_count;
}

static void bumpy_summarize_centerline(
    const BumpyWhiteRun *runs,
    int run_count,
    float fallback_x,
    int has_fallback_x,
    BumpyCenterlineSummary *summary)
{
    float width_sum = 0.0f;
    if (run_count <= 0) {
        const float target_x = has_fallback_x ? fallback_x : BUMPY_IMAGE_CENTER_X;
        summary->target_x = target_x;
        summary->steer_error_px = target_x - BUMPY_IMAGE_CENTER_X;
        summary->row_count = 0;
        summary->bottom_row_count = 0;
        summary->top_y = -1;
        summary->bottom_y = -1;
        summary->mean_width = 0.0f;
        return;
    }

    summary->row_count = run_count;
    summary->bottom_row_count = 0;
    summary->top_y = runs[0].y;
    summary->bottom_y = runs[run_count - 1].y;

    for (int i = 0; i < run_count; i++) {
        if (runs[i].y >= BUMPY_ROI_Y1 - BUMPY_BOTTOM_TARGET_ROWS + 1) {
            summary->bottom_row_count++;
        }
        width_sum += (float)bumpy_run_width(&runs[i]);
    }
    summary->mean_width = width_sum / (float)run_count;

    {
        const int use_bottom = summary->bottom_row_count > 0;
        const int target_count = use_bottom ? summary->bottom_row_count : (run_count < 8 ? run_count : 8);
        int start_index = 0;
        float weighted_sum = 0.0f;
        float weight_total = 0.0f;

        if (!use_bottom) {
            start_index = run_count - target_count;
        }

        for (int i = 0; i < run_count; i++) {
            const int include = use_bottom
                ? (runs[i].y >= BUMPY_ROI_Y1 - BUMPY_BOTTOM_TARGET_ROWS + 1)
                : (i >= start_index);
            if (include) {
                const float weight = 1.0f + 0.08f * (float)bumpy_max_f(0, runs[i].y - BUMPY_ROI_Y0);
                weighted_sum += weight * bumpy_run_center_x(&runs[i]);
                weight_total += weight;
            }
        }
        summary->target_x = weight_total > 0.0f ? weighted_sum / weight_total : (has_fallback_x ? fallback_x : BUMPY_IMAGE_CENTER_X);
        summary->steer_error_px = summary->target_x - BUMPY_IMAGE_CENTER_X;
    }
}

static BumpyPhase bumpy_classify_phase(
    int component_found,
    const BumpyCenterlineSummary *centerline,
    int rib_count)
{
    if (rib_count > 0) {
        return BUMPY_PHASE_INSIDE;
    }
    if (!component_found && centerline->bottom_y >= 56 && centerline->bottom_row_count >= 3) {
        return BUMPY_PHASE_EXIT;
    }
    if (!component_found && centerline->row_count < 4) {
        return BUMPY_PHASE_UNCERTAIN;
    }
    if (centerline->top_y >= 0 && centerline->top_y <= 18 && centerline->bottom_row_count <= 5) {
        return BUMPY_PHASE_APPROACH;
    }
    if (centerline->bottom_y >= 54 && centerline->top_y >= 20) {
        return BUMPY_PHASE_EXIT;
    }
    if (centerline->row_count >= 8 && centerline->bottom_row_count >= 4) {
        return BUMPY_PHASE_WHITE_SURFACE_ONLY;
    }
    if (component_found) {
        return BUMPY_PHASE_APPROACH;
    }
    return BUMPY_PHASE_UNCERTAIN;
}

static BumpyControllerMode bumpy_mode_from_phase(BumpyPhase phase)
{
    switch (phase) {
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

int bumpy_detect_frame_gray(
    const uint8_t *gray,
    int width,
    int height,
    float prev_white_threshold,
    int has_prev_white_threshold,
    BumpyDetectScratch *scratch,
    BumpyDetectResult *result)
{
    int pixels;
    int component_count;
    int candidate_count;
    int run_count;
    int rib_count;
    float white_threshold;
    float white_candidate;
    int white_threshold_int;
    float dark_threshold;
    float fallback_x = 0.0f;
    int has_fallback_x = 0;

    if (gray == NULL || scratch == NULL || result == NULL) {
        return -1;
    }
    if (width <= 0 || height <= 0 || width > BUMPY_MAX_WIDTH || height > BUMPY_MAX_HEIGHT) {
        return -2;
    }

    pixels = width * height;
    bumpy_detect_result_clear(result);

    bumpy_estimate_white_threshold(
        gray,
        width,
        height,
        prev_white_threshold,
        has_prev_white_threshold,
        &white_threshold,
        &white_candidate);
    white_threshold_int = (int)lroundf(white_threshold);

    for (int i = 0; i < pixels; i++) {
        scratch->global_white_mask[i] = gray[i] >= white_threshold_int ? 1u : 0u;
    }
    component_count = bumpy_collect_components(gray, scratch->global_white_mask, width, height, scratch);
    candidate_count = bumpy_filter_candidates(scratch->components, component_count, scratch);

    run_count = bumpy_build_white_scan_mask(gray, width, height, white_threshold, scratch);
    for (int i = 0; i < pixels; i++) {
        scratch->white_mask[i] = (uint8_t)(scratch->global_white_mask[i] || scratch->scan_white_mask[i]);
    }

    if (candidate_count > 0) {
        result->best_component = scratch->candidates[0];
        result->best_component_found = 1;
        has_fallback_x = 1;
        fallback_x = scratch->candidates[0].centroid_x;
    }

    bumpy_summarize_centerline(scratch->runs, run_count, fallback_x, has_fallback_x, &result->centerline);

    dark_threshold = bumpy_estimate_dark_threshold(gray, width, height);
    bumpy_build_supported_dark_mask(gray, width, height, dark_threshold, scratch->white_mask, scratch->rib_mask);
    rib_count = bumpy_find_rib_bands(gray, width, height, scratch->rib_mask, scratch->rib_bands, BUMPY_MAX_RIB_BANDS);

    result->phase = bumpy_classify_phase(result->best_component_found, &result->centerline, rib_count);
    result->mode = bumpy_mode_from_phase(result->phase);
    result->detected = result->best_component_found || run_count > 0 ? 1u : 0u;
    result->white_threshold = white_threshold;
    result->white_threshold_candidate = white_candidate;
    result->white_threshold_int = white_threshold_int;
    result->dark_threshold = dark_threshold;
    result->component_count = component_count;
    result->candidate_count = candidate_count;
    result->run_count = run_count;
    result->rib_count = rib_count;

    return 0;
}
