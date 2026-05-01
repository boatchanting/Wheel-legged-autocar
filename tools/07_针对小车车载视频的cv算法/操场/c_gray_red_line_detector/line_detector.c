/*
 * 灰度红操场单线检测 - C 实现
 *
 * Timing Note (updated after benchmark on 2026-05-01):
 * - input: data/line_gray_red_c_pgm_frames (6561 frames, 188x120)
 * - total_us: 880159.6
 * - avg_us_per_frame: 134.15
 * - min_us: 113.8
 * - max_us: 1598.9
 * - fps: 7454.33
 */

#include "line_detector.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static float min_f(float a, float b) { return a < b ? a : b; }
static float max_f(float a, float b) { return a > b ? a : b; }

static void blur3x3_u8(const uint8_t *src, int w, int h, uint8_t *dst)
{
    for (int y = 0; y < h; y++) {
        const int y0 = y > 0 ? y - 1 : 0;
        const int y1 = y;
        const int y2 = y < h - 1 ? y + 1 : h - 1;
        for (int x = 0; x < w; x++) {
            const int x0 = x > 0 ? x - 1 : 0;
            const int x1 = x;
            const int x2 = x < w - 1 ? x + 1 : w - 1;
            int sum = 0;
            sum += src[y0 * w + x0] + src[y0 * w + x1] + src[y0 * w + x2];
            sum += src[y1 * w + x0] + src[y1 * w + x1] + src[y1 * w + x2];
            sum += src[y2 * w + x0] + src[y2 * w + x1] + src[y2 * w + x2];
            dst[y * w + x] = (uint8_t)((sum + 4) / 9);
        }
    }
}

static int percentile_from_hist(const int *hist, int hist_len, int total_count, float p)
{
    if (total_count <= 0) {
        return 0;
    }
    int target = (int)(p * (float)(total_count - 1) + 0.5f);
    int acc = 0;
    for (int v = 0; v < hist_len; v++) {
        acc += hist[v];
        if (acc > target) {
            return v;
        }
    }
    return hist_len - 1;
}

static void compute_integral_u8(const uint8_t *gray, int w, int h, int *integral)
{
    const int stride = w + 1;
    memset(integral, 0, (size_t)(h + 1) * (size_t)(w + 1) * sizeof(int));
    for (int y = 1; y <= h; y++) {
        int row_sum = 0;
        for (int x = 1; x <= w; x++) {
            row_sum += gray[(y - 1) * w + (x - 1)];
            integral[y * stride + x] = integral[(y - 1) * stride + x] + row_sum;
        }
    }
}

static int rect_sum(const int *integral, int w, int x0, int y0, int x1, int y1)
{
    const int stride = w + 1;
    const int A = integral[y0 * stride + x0];
    const int B = integral[y0 * stride + x1];
    const int C = integral[y1 * stride + x0];
    const int D = integral[y1 * stride + x1];
    return D - B - C + A;
}

static int build_bright_core_mask(const uint8_t *gray, int w, int h, LineDetectScratch *scratch)
{
    int hist[256];
    memset(hist, 0, sizeof(hist));
    blur3x3_u8(gray, w, h, scratch->gray_blur);
    for (int i = 0; i < w * h; i++) {
        hist[scratch->gray_blur[i]]++;
    }
    compute_integral_u8(scratch->gray_blur, w, h, scratch->integral);
    {
        const int q_hi = percentile_from_hist(hist, 256, w * h, 0.88f);
        const int hi_thr = q_hi > 148 ? q_hi : 148;
        const int radius = 4;
        int white = 0;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                const int x0 = x > radius ? x - radius : 0;
                const int y0 = y > radius ? y - radius : 0;
                const int x1 = x + radius < w - 1 ? x + radius : w - 1;
                const int y1 = y + radius < h - 1 ? y + radius : h - 1;
                const int sx1 = x1 + 1;
                const int sy1 = y1 + 1;
                const int area = (x1 - x0 + 1) * (y1 - y0 + 1);
                const int mean = (rect_sum(scratch->integral, w, x0, y0, sx1, sy1) + area / 2) / area;
                const int g = scratch->gray_blur[y * w + x];
                const uint8_t m = (uint8_t)((g >= hi_thr && (g - mean) >= 2) ? 255 : 0);
                scratch->mask[y * w + x] = m;
                white += (m > 0);
            }
        }
        return white;
    }
}

static void flood_component(
    int start_idx,
    int label,
    int w,
    int h,
    LineDetectScratch *scratch,
    LineComponent *out)
{
    int top = 0;
    int area = 0;
    int xmin = w - 1;
    int ymin = h - 1;
    int xmax = 0;
    int ymax = 0;
    int64_t sum_x = 0;
    int64_t sum_y = 0;

    scratch->stack[top++] = start_idx;
    scratch->visited[start_idx] = 1;
    scratch->labels[start_idx] = (uint16_t)label;

    while (top > 0) {
        const int idx = scratch->stack[--top];
        const int y = idx / w;
        const int x = idx - y * w;
        area++;
        sum_x += x;
        sum_y += y;
        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;
        if (y < ymin) ymin = y;
        if (y > ymax) ymax = y;

        if (y > 0) {
            const int n = idx - w;
            if (!scratch->visited[n] && scratch->mask[n] > 0) {
                scratch->visited[n] = 1;
                scratch->labels[n] = (uint16_t)label;
                scratch->stack[top++] = n;
            }
        }
        if (y + 1 < h) {
            const int n = idx + w;
            if (!scratch->visited[n] && scratch->mask[n] > 0) {
                scratch->visited[n] = 1;
                scratch->labels[n] = (uint16_t)label;
                scratch->stack[top++] = n;
            }
        }
        if (x > 0) {
            const int n = idx - 1;
            if (!scratch->visited[n] && scratch->mask[n] > 0) {
                scratch->visited[n] = 1;
                scratch->labels[n] = (uint16_t)label;
                scratch->stack[top++] = n;
            }
        }
        if (x + 1 < w) {
            const int n = idx + 1;
            if (!scratch->visited[n] && scratch->mask[n] > 0) {
                scratch->visited[n] = 1;
                scratch->labels[n] = (uint16_t)label;
                scratch->stack[top++] = n;
            }
        }
    }

    out->area = area;
    out->xmin = xmin;
    out->ymin = ymin;
    out->xmax = xmax;
    out->ymax = ymax;
    out->centroid_x = area > 0 ? (float)sum_x / (float)area : 0.0f;
    out->centroid_y = area > 0 ? (float)sum_y / (float)area : 0.0f;
    out->score = 0.0f;
}

static int collect_components(int w, int h, LineDetectScratch *scratch)
{
    int count = 0;
    const int pixels = w * h;
    memset(scratch->visited, 0, (size_t)pixels);
    memset(scratch->labels, 0, (size_t)pixels * sizeof(uint16_t));

    for (int i = 0; i < pixels; i++) {
        if (scratch->mask[i] == 0 || scratch->visited[i]) {
            continue;
        }
        if (count >= LINE_MAX_COMPONENTS) {
            break;
        }
        flood_component(i, count + 1, w, h, scratch, &scratch->components[count]);
        count++;
    }
    return count;
}

static float score_component(const LineComponent *c, int w, int h)
{
    const float comp_w = (float)(c->xmax - c->xmin + 1);
    const float comp_h = (float)(c->ymax - c->ymin + 1);
    const float center = 1.0f - fabsf(c->centroid_x - (float)w * 0.5f) / max_f(1.0f, (float)w * 0.5f);
    const float top_touch = c->ymin <= (int)((float)h * 0.14f) ? 1.0f : 0.0f;
    const float tall = comp_h / max_f(1.0f, (float)h);
    const float area = (float)c->area / max_f(1.0f, (float)(w * h));
    const float width_penalty = comp_w / max_f(1.0f, (float)w);
    return 1.35f * tall + 0.9f * center + 1.0f * top_touch + 0.25f * area - 0.5f * width_penalty;
}

static int filter_candidates(int comp_count, int w, int h, LineDetectScratch *scratch)
{
    int count = 0;
    for (int i = 0; i < comp_count; i++) {
        LineComponent c = scratch->components[i];
        const int ch = c.ymax - c.ymin + 1;
        if (c.area < max_f(90.0f, (float)(w * h) * 0.012f)) continue;
        if (ch < max_f(14.0f, (float)h * 0.22f)) continue;
        c.score = score_component(&c, w, h);
        scratch->candidates[count++] = c;
        if (count >= LINE_MAX_COMPONENTS) break;
    }
    for (int i = 1; i < count; i++) {
        LineComponent key = scratch->candidates[i];
        int j = i - 1;
        while (j >= 0 && scratch->candidates[j].score < key.score) {
            scratch->candidates[j + 1] = scratch->candidates[j];
            j--;
        }
        scratch->candidates[j + 1] = key;
    }
    return count;
}

static int fit_line_for_label(
    int label_id,
    int w,
    int h,
    const LineComponent *best,
    const LineDetectScratch *scratch,
    float *a,
    float *b,
    int *rows_used)
{
    double sum_y = 0.0, sum_x = 0.0, sum_yy = 0.0, sum_yx = 0.0;
    int n = 0;
    for (int y = best->ymin; y <= best->ymax; y++) {
        int cnt = 0;
        int sx = 0;
        const int row = y * w;
        for (int x = best->xmin; x <= best->xmax; x++) {
            if ((int)scratch->labels[row + x] == label_id) {
                cnt++;
                sx += x;
            }
        }
        if (cnt <= 0) continue;
        {
            const float cx = (float)sx / (float)cnt;
            sum_y += (double)y;
            sum_x += (double)cx;
            sum_yy += (double)y * (double)y;
            sum_yx += (double)y * (double)cx;
            n++;
        }
    }
    *rows_used = n;
    if (n < 6) return -1;
    {
        const double denom = (double)n * sum_yy - sum_y * sum_y;
        if (fabs(denom) < 1e-6) return -2;
        *a = (float)(((double)n * sum_yx - sum_y * sum_x) / denom);
        *b = (float)((sum_x - (double)(*a) * sum_y) / (double)n);
    }
    return 0;
}

void line_detect_result_clear(LineDetectResult *result)
{
    if (result == NULL) return;
    memset(result, 0, sizeof(*result));
    result->bbox_xmin = -1;
    result->bbox_ymin = -1;
    result->bbox_xmax = -1;
    result->bbox_ymax = -1;
    result->line_x_bottom = -1.0f;
    result->line_x_lookahead = -1.0f;
}

int line_detect_frame_gray(
    const uint8_t *gray,
    int width,
    int height,
    LineDetectScratch *scratch,
    LineDetectResult *result)
{
    int comp_count;
    int cand_count;
    float a = 0.0f, b = 0.0f;
    int rows_used = 0;

    if (gray == NULL || scratch == NULL || result == NULL) return -1;
    if (width <= 0 || height <= 0 || width > LINE_MAX_WIDTH || height > LINE_MAX_HEIGHT) return -2;

    line_detect_result_clear(result);
    (void)build_bright_core_mask(gray, width, height, scratch);
    comp_count = collect_components(width, height, scratch);
    cand_count = filter_candidates(comp_count, width, height, scratch);
    result->component_count = comp_count;
    result->candidate_count = cand_count;

    if (cand_count <= 0) return 0;

    {
        const LineComponent *best = &scratch->candidates[0];
        const int label_id = 1; /* re-map by bbox + centroid match below */
        int matched_label = -1;
        float best_label_dist = 1e9f;
        for (int i = 0; i < comp_count; i++) {
            const LineComponent *c = &scratch->components[i];
            const float dx = fabsf(c->centroid_x - best->centroid_x);
            const float dy = fabsf(c->centroid_y - best->centroid_y);
            const float d = dx + dy;
            if (d < best_label_dist) {
                best_label_dist = d;
                matched_label = i + 1;
            }
        }
        if (matched_label < 0) return 0;
        if (fit_line_for_label(matched_label, width, height, best, scratch, &a, &b, &rows_used) != 0) return 0;

        {
            const float y_bottom = (float)((int)((float)height * 0.93f));
            const float y_look = (float)((int)((float)height * 0.72f));
            const float y2 = (float)((int)((float)height * 0.62f));
            float xb = a * y_bottom + b;
            float xl = a * y_look + b;
            float x2 = a * y2 + b;
            xb = min_f(max_f(xb, 0.0f), (float)(width - 1));
            xl = min_f(max_f(xl, 0.0f), (float)(width - 1));
            x2 = min_f(max_f(x2, 0.0f), (float)(width - 1));

            result->detected = (uint8_t)(rows_used >= 6);
            result->confidence = min_f(max_f(best->score / 2.6f, 0.0f), 1.0f);
            if (result->confidence < LINE_MIN_DECISION_SCORE) {
                result->detected = 0;
            }

            result->best_label = matched_label;
            result->bbox_xmin = best->xmin;
            result->bbox_ymin = best->ymin;
            result->bbox_xmax = best->xmax;
            result->bbox_ymax = best->ymax;
            result->centroid_x = best->centroid_x;
            result->centroid_y = best->centroid_y;
            result->line_x_bottom = xb;
            result->line_x_lookahead = xl;
            result->line_point_rows = rows_used;
            result->lateral_error_px = xl - ((float)width * 0.5f);
            result->line_yaw_deg = (float)(atan2((double)(x2 - xb), (double)(y_bottom - y2)) * 57.295779513);
        }
    }
    return 0;
}

void line_temporal_state_init(LineTemporalState *state, int max_lost, float smooth_alpha, float min_temporal_score)
{
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    state->active = 0;
    state->lost_count = 0;
    state->max_lost = max_lost > 0 ? max_lost : 30;
    state->smooth_alpha = (smooth_alpha > 0.0f && smooth_alpha <= 1.0f) ? smooth_alpha : 0.45f;
    state->min_temporal_score = min_temporal_score;
}

LineTemporalDecision line_temporal_update(
    LineTemporalState *state,
    int image_width,
    const LineDetectResult *raw,
    LineDetectResult *out)
{
    LineTemporalDecision d;
    d.mode = LINE_TEMPORAL_MODE_LOST;
    d.accepted = 0;
    d.temporal_score = -1.0f;

    if (state == NULL || raw == NULL || out == NULL) {
        line_detect_result_clear(out);
        return d;
    }

    *out = *raw;

    if (!raw->detected) {
        if (state->active) {
            state->lost_count++;
            if (state->lost_count <= state->max_lost) {
                d.mode = LINE_TEMPORAL_MODE_PREDICTED;
                out->detected = 1;
                out->line_x_bottom = state->bottom_x;
                out->line_x_lookahead = state->lookahead_x;
                out->line_yaw_deg = state->yaw_deg;
                out->lateral_error_px = state->lookahead_x - (float)image_width * 0.5f;
                out->confidence = state->confidence;
                return d;
            }
            state->active = 0;
        }
        line_detect_result_clear(out);
        return d;
    }

    if (!state->active) {
        state->active = 1;
        state->lost_count = 0;
        state->bottom_x = raw->line_x_bottom;
        state->lookahead_x = raw->line_x_lookahead;
        state->yaw_deg = raw->line_yaw_deg;
        state->confidence = raw->confidence;
        d.accepted = 1;
        d.mode = LINE_TEMPORAL_MODE_DETECTED;
        d.temporal_score = raw->confidence;
        return d;
    }

    {
        const float dx_bottom = fabsf(raw->line_x_bottom - state->bottom_x);
        const float dx_look = fabsf(raw->line_x_lookahead - state->lookahead_x);
        const float dyaw = fabsf(raw->line_yaw_deg - state->yaw_deg);
        const float denom = max_f(1.0f, (float)image_width * 0.22f);
        const float pos_penalty = min_f(1.0f, (0.65f * dx_look + 0.35f * dx_bottom) / denom);
        const float yaw_penalty = min_f(1.0f, dyaw / 28.0f);
        const float temporal_score = raw->confidence - 0.42f * pos_penalty - 0.18f * yaw_penalty;
        d.temporal_score = temporal_score;

        if (temporal_score >= state->min_temporal_score) {
            const float a = state->smooth_alpha;
            state->bottom_x = a * raw->line_x_bottom + (1.0f - a) * state->bottom_x;
            state->lookahead_x = a * raw->line_x_lookahead + (1.0f - a) * state->lookahead_x;
            state->yaw_deg = a * raw->line_yaw_deg + (1.0f - a) * state->yaw_deg;
            state->confidence = a * raw->confidence + (1.0f - a) * state->confidence;
            state->lost_count = 0;

            out->detected = 1;
            out->line_x_bottom = state->bottom_x;
            out->line_x_lookahead = state->lookahead_x;
            out->line_yaw_deg = state->yaw_deg;
            out->lateral_error_px = state->lookahead_x - (float)image_width * 0.5f;
            out->confidence = state->confidence;

            d.accepted = 1;
            d.mode = LINE_TEMPORAL_MODE_DETECTED;
            return d;
        }
    }

    state->lost_count++;
    if (state->lost_count <= state->max_lost) {
        out->detected = 1;
        out->line_x_bottom = state->bottom_x;
        out->line_x_lookahead = state->lookahead_x;
        out->line_yaw_deg = state->yaw_deg;
        out->lateral_error_px = state->lookahead_x - (float)image_width * 0.5f;
        out->confidence = state->confidence;
        d.mode = LINE_TEMPORAL_MODE_PREDICTED;
        return d;
    }

    state->active = 0;
    line_detect_result_clear(out);
    return d;
}
