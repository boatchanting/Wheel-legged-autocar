#include "bridge_detection.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define B2_MIN_COMPONENT_AREA 24
#define B2_LINE_MAX_POINTS BRIDGE_DETECTION_MAX_WIDTH

typedef struct {
    int threshold;
    double score;
    int top_row;
    int start_row;
    int bottom_row;
    int max_width;
    int bottom_width;
    int area;
    double area_ratio;
    double center_x;
    double edge_contrast;
    double left_clip_ratio;
    double right_clip_ratio;
    double dual_clip_ratio;
    double border_monotonic;
} Candidate;

typedef struct {
    int valid;
    double slope;
    double intercept;
    double support_min;
    double support_max;
    int inlier_count;
    double span;
    double residual;
    double border_touch_ratio;
    double mean_value;
} LineFit;

static int clamp_int(int value, int lo, int hi)
{
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

static double clamp_double(double value, double lo, double hi)
{
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

static int round_nearest(double value)
{
    return value >= 0.0 ? (int)floor(value + 0.5) : (int)ceil(value - 0.5);
}

void bridge_detection_default_config(BridgeDetectionConfig *config)
{
    if (config == NULL) return;
    config->min_valid_score = 350.0f;
    config->min_edge_contrast = 20.0f;
}

void bridge_detection_result_clear(BridgeDetectionResult *result)
{
    if (result == NULL) return;
    memset(result, 0, sizeof(*result));
    result->threshold = -1;
    result->top_row = -1;
    result->start_row = -1;
    result->bottom_row = -1;
}

const char *bridge_detection_state_name(BridgeDetectionState state)
{
    switch (state) {
        case BRIDGE_DETECTION_STATE_PREPARE_ENTER: return "prepare_enter";
        case BRIDGE_DETECTION_STATE_ON_BRIDGE: return "on_bridge";
        case BRIDGE_DETECTION_STATE_PREPARE_EXIT: return "prepare_exit";
        default: return "none";
    }
}

static void copy_gray_contiguous(const uint8_t *gray, int width, int height, int stride, uint8_t *out)
{
    int y;
    for (y = 0; y < height; ++y) {
        memcpy(out + y * width, gray + y * stride, (size_t)width);
    }
}

static void binary_dilate(const uint8_t *src, uint8_t *dst, int width, int height, int size)
{
    int x, y, kx, ky;
    int low = size == 2 ? 0 : -1;
    int high = 1;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            int on = 0;
            for (ky = low; ky <= high && !on; ++ky) {
                int sy = y + ky;
                if (sy < 0 || sy >= height) continue;
                for (kx = low; kx <= high; ++kx) {
                    int sx = x + kx;
                    if (sx >= 0 && sx < width && src[sy * width + sx]) {
                        on = 1;
                        break;
                    }
                }
            }
            dst[y * width + x] = (uint8_t)on;
        }
    }
}

static void binary_erode(const uint8_t *src, uint8_t *dst, int width, int height, int size)
{
    int x, y, kx, ky;
    int low = -1;
    int high = size == 2 ? 0 : 1;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            int on = 1;
            for (ky = low; ky <= high && on; ++ky) {
                int sy = y + ky;
                for (kx = low; kx <= high; ++kx) {
                    int sx = x + kx;
                    if (sx < 0 || sx >= width || sy < 0 || sy >= height || !src[sy * width + sx]) {
                        on = 0;
                        break;
                    }
                }
            }
            dst[y * width + x] = (uint8_t)on;
        }
    }
}

static void close3_open2(uint8_t *mask, uint8_t *temp, int width, int height)
{
    binary_dilate(mask, temp, width, height, 3);
    binary_erode(temp, mask, width, height, 3);
    binary_erode(mask, temp, width, height, 2);
    binary_dilate(temp, mask, width, height, 2);
}

static void fill_holes_4(const uint8_t *src, uint8_t *dst, uint8_t *visited,
                         uint16_t *queue, int width, int height)
{
    int pixels = width * height;
    int head = 0, tail = 0;
    int x, y;
    memset(visited, 0, (size_t)pixels);

#define B2_PUSH_BORDER(index_) do { \
    int b2_i = (index_); \
    if (!src[b2_i] && !visited[b2_i]) { \
        visited[b2_i] = 1; queue[tail++] = (uint16_t)b2_i; \
    } \
} while (0)

    for (x = 0; x < width; ++x) {
        B2_PUSH_BORDER(x);
        B2_PUSH_BORDER((height - 1) * width + x);
    }
    for (y = 1; y < height - 1; ++y) {
        B2_PUSH_BORDER(y * width);
        B2_PUSH_BORDER(y * width + width - 1);
    }
    while (head < tail) {
        int index = queue[head++];
        int qx = index % width;
        int qy = index / width;
        int ni;
        if (qx > 0) {
            ni = index - 1;
            if (!src[ni] && !visited[ni]) { visited[ni] = 1; queue[tail++] = (uint16_t)ni; }
        }
        if (qx + 1 < width) {
            ni = index + 1;
            if (!src[ni] && !visited[ni]) { visited[ni] = 1; queue[tail++] = (uint16_t)ni; }
        }
        if (qy > 0) {
            ni = index - width;
            if (!src[ni] && !visited[ni]) { visited[ni] = 1; queue[tail++] = (uint16_t)ni; }
        }
        if (qy + 1 < height) {
            ni = index + width;
            if (!src[ni] && !visited[ni]) { visited[ni] = 1; queue[tail++] = (uint16_t)ni; }
        }
    }
    for (x = 0; x < pixels; ++x) dst[x] = (uint8_t)(src[x] || !visited[x]);
#undef B2_PUSH_BORDER
}

typedef struct { int x; int y; } PointI;

static long cross_point(PointI o, PointI a, PointI b)
{
    return (long)(a.x - o.x) * (long)(b.y - o.y) - (long)(a.y - o.y) * (long)(b.x - o.x);
}

static int convex_hull_mask(const uint8_t *src, uint8_t *dst, int width, int height)
{
    PointI hull[2 * (BRIDGE_DETECTION_MAX_WIDTH + BRIDGE_DETECTION_MAX_HEIGHT) + 8];
    int count = 0, k = 0, x, y, lower, reverse_first = 1;
    memset(dst, 0, (size_t)(width * height));
    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            PointI point;
            if (!src[y * width + x]) continue;
            point.x = x; point.y = y; ++count;
            while (k >= 2 && cross_point(hull[k - 2], hull[k - 1], point) <= 0) --k;
            hull[k++] = point;
        }
    }
    if (count < 3) return 0;
    lower = k;
    for (x = width - 1; x >= 0; --x) {
        for (y = height - 1; y >= 0; --y) {
            PointI point;
            if (!src[y * width + x]) continue;
            point.x = x; point.y = y;
            if (reverse_first) { reverse_first = 0; continue; }
            while (k > lower && cross_point(hull[k - 2], hull[k - 1], point) <= 0) --k;
            hull[k++] = point;
        }
    }
    if (k > 1) --k;
    if (k < 3) return 0;
    /* PIL ImageDraw.polygon, used by the Python prototype, fills a convex
     * scanline between nearest rounded edge intersections with inward ties. */
    for (y = 0; y < height; ++y) {
        double min_x = 1e18, max_x = -1e18;
        int edge;
        for (edge = 0; edge < k; ++edge) {
            PointI a = hull[edge], b = hull[(edge + 1) % k];
            if (a.y == b.y) {
                if (y == a.y) {
                    if (a.x < min_x) min_x = a.x; if (a.x > max_x) max_x = a.x;
                    if (b.x < min_x) min_x = b.x; if (b.x > max_x) max_x = b.x;
                }
            } else if (y >= (a.y < b.y ? a.y : b.y) && y <= (a.y > b.y ? a.y : b.y)) {
                double hit = a.x + (double)(b.x - a.x) * (y - a.y) / (double)(b.y - a.y);
                if (hit < min_x) min_x = hit;
                if (hit > max_x) max_x = hit;
            }
        }
        if (min_x <= max_x) {
            int left = clamp_int((int)floor(min_x + 0.5), 0, width - 1);
            int right = clamp_int((int)ceil(max_x - 0.5), 0, width - 1);
            for (x = left; x <= right; ++x) dst[y * width + x] = 1;
        }
    }
    return 1;
}

static int otsu_threshold_limited(const uint8_t *gray, int pixels)
{
    int hist[256] = {0};
    double cumulative_prob = 0.0, cumulative_mean = 0.0, global_mean = 0.0;
    double best_score = -1.0;
    int best = 0, i;
    for (i = 0; i < pixels; ++i) ++hist[gray[i]];
    for (i = 0; i <= 180; ++i) global_mean += ((double)hist[i] / pixels) * i;
    for (i = 0; i < 180; ++i) {
        double w0, w1, mean0, mean1, score;
        cumulative_prob += (double)hist[i] / pixels;
        cumulative_mean += ((double)hist[i] / pixels) * i;
        w0 = cumulative_prob;
        w1 = 1.0 - w0;
        if (w0 <= 1e-6 || w1 <= 1e-6) continue;
        mean0 = cumulative_mean / w0;
        mean1 = (global_mean - cumulative_mean) / w1;
        score = w0 * w1 * (mean0 - mean1) * (mean0 - mean1);
        if (score > best_score) { best_score = score; best = i; }
    }
    return best < 70 ? 70 : best;
}

static double percentile_u8(const int hist[256], int pixels, double q)
{
    double position = (pixels - 1) * q;
    int lo = (int)floor(position);
    int hi = (int)ceil(position);
    int lo_value = 0, hi_value = 0, cumulative = 0, i;
    for (i = 0; i < 256; ++i) {
        cumulative += hist[i];
        if (cumulative > lo && lo_value == 0) lo_value = i;
        if (cumulative > hi) { hi_value = i; break; }
    }
    if (hi == lo) return lo_value;
    return lo_value + (hi_value - lo_value) * (position - lo);
}

static int int_compare(const void *a, const void *b)
{
    int ia = *(const int *)a, ib = *(const int *)b;
    return ia - ib;
}

static int build_threshold_candidates(const uint8_t *gray, int pixels, int out[11])
{
    int hist[256] = {0};
    int values[11], unique = 0, i;
    double sum = 0.0, sum_sq = 0.0, mean, variance, std;
    int base;
    for (i = 0; i < pixels; ++i) {
        double v = gray[i];
        ++hist[gray[i]];
        sum += v;
        sum_sq += v * v;
    }
    mean = sum / pixels;
    variance = sum_sq / pixels - mean * mean;
    if (variance < 0.0) variance = 0.0;
    std = sqrt(variance);
    base = otsu_threshold_limited(gray, pixels);
    values[0] = base - 25;
    values[1] = base - 15;
    values[2] = base - 8;
    values[3] = base;
    values[4] = (int)percentile_u8(hist, pixels, 0.82);
    values[5] = (int)percentile_u8(hist, pixels, 0.86);
    values[6] = (int)percentile_u8(hist, pixels, 0.90);
    values[7] = (int)percentile_u8(hist, pixels, 0.92);
    values[8] = (int)(mean + 0.45 * std);
    values[9] = (int)(mean + 0.75 * std);
    values[10] = (int)(mean + 0.95 * std);
    for (i = 0; i < 11; ++i) values[i] = clamp_int(values[i], 90, 225);
    qsort(values, 11, sizeof(values[0]), int_compare);
    for (i = 0; i < 11; ++i) {
        if (unique == 0 || values[i] != out[unique - 1]) out[unique++] = values[i];
    }
    return unique;
}

static void threshold_mask(const uint8_t *gray, uint8_t *mask, int width, int height, int threshold)
{
    int x, y;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            int local = threshold;
            if (x < 19) local -= 10;
            if (x >= 76) local -= 10;
            if (x >= 83 && x < 89) local -= 10;
            mask[y * width + x] = (uint8_t)(gray[y * width + x] > local);
        }
    }
}

static int extract_component(uint8_t *global_mask, uint8_t *component, uint16_t *queue,
                             int start, int width, int height)
{
    int head = 0, tail = 0;
    memset(component, 0, (size_t)(width * height));
    global_mask[start] = 0;
    component[start] = 1;
    queue[tail++] = (uint16_t)start;
    while (head < tail) {
        int index = queue[head++];
        int x = index % width, y = index / width, ni;
#define B2_VISIT(ni_) do { ni = (ni_); if (global_mask[ni]) { global_mask[ni] = 0; component[ni] = 1; queue[tail++] = (uint16_t)ni; } } while (0)
        if (x > 0) B2_VISIT(index - 1);
        if (x + 1 < width) B2_VISIT(index + 1);
        if (y > 0) B2_VISIT(index - width);
        if (y + 1 < height) B2_VISIT(index + width);
#undef B2_VISIT
    }
    return tail;
}

static int evaluate_component(const uint8_t *gray, const uint8_t *component, int threshold,
                              int width, int height, BridgeDetectionScratch *scratch, Candidate *out)
{
    int left[BRIDGE_DETECTION_MAX_HEIGHT], right[BRIDGE_DETECTION_MAX_HEIGHT];
    int widths[BRIDGE_DETECTION_MAX_HEIGHT], valid_rows[BRIDGE_DETECTION_MAX_HEIGHT];
    int valid_count = 0, stable_count = 0;
    int top_row, bottom_row, max_width = 0, start_row, min_width;
    int area = 0, bottom_width, start_width, x, y, i;
    double weighted_center_sum = 0.0, weight_sum = 0.0;
    double inside_sum = 0.0, outside_sum = 0.0;
    int contrast_count = 0, left_clip_count = 0, right_clip_count = 0, dual_clip_count = 0;
    int monotonic_count = 0;
    double edge_contrast, center_x, score;
    uint8_t *visible = scratch->work4;
    uint8_t *filled = scratch->work3;
    uint8_t *outer = scratch->work2;

    memcpy(visible, component, (size_t)(width * height));
    close3_open2(visible, scratch->work1, width, height);
    fill_holes_4(visible, filled, scratch->work1, scratch->queue, width, height);
    if (!convex_hull_mask(filled, outer, width, height)) memcpy(outer, filled, (size_t)(width * height));

    for (y = 0; y < height; ++y) {
        left[y] = -1; right[y] = -1; widths[y] = 0;
        for (x = 0; x < width; ++x) {
            if (!outer[y * width + x]) continue;
            if (left[y] < 0) left[y] = x;
            right[y] = x;
            ++area;
        }
        if (left[y] >= 0) {
            widths[y] = right[y] - left[y] + 1;
            valid_rows[valid_count++] = y;
            if (widths[y] > max_width) max_width = widths[y];
        }
    }
    if (valid_count < 10) return 0;
    top_row = valid_rows[0];
    bottom_row = valid_rows[valid_count - 1];
    min_width = (int)floor(max_width * 0.12 + 0.5);
    if (min_width < 6) min_width = 6;
    start_row = valid_rows[0];
    for (i = 0; i < valid_count; ++i) {
        int next_count = valid_count - i;
        int ok = 1, j;
        if (widths[valid_rows[i]] < min_width) continue;
        if (next_count > 3) next_count = 3;
        if (next_count < 2) continue;
        for (j = 0; j < next_count; ++j) {
            int limit = min_width - 2;
            if (limit < 4) limit = 4;
            if (widths[valid_rows[i + j]] < limit) { ok = 0; break; }
        }
        if (ok) { start_row = valid_rows[i]; break; }
    }
    for (i = 0; i < valid_count; ++i) if (valid_rows[i] >= start_row) ++stable_count;
    if (stable_count < 10) return 0;
    bottom_width = widths[bottom_row];
    start_width = widths[start_row];
    for (i = 0; i < valid_count; ++i) {
        int row = valid_rows[i];
        double outside_values[2];
        int outside_n = 0;
        if (row < start_row) continue;
        weighted_center_sum += ((left[row] + right[row]) * 0.5) * widths[row];
        weight_sum += widths[row];
        if (left[row] >= 2) outside_values[outside_n++] = gray[row * width + left[row] - 2];
        else if (left[row] >= 1) outside_values[outside_n++] = gray[row * width + left[row] - 1];
        if (right[row] <= width - 3) outside_values[outside_n++] = gray[row * width + right[row] + 2];
        else if (right[row] <= width - 2) outside_values[outside_n++] = gray[row * width + right[row] + 1];
        if (outside_n) {
            inside_sum += (gray[row * width + left[row]] + gray[row * width + right[row]]) * 0.5;
            outside_sum += outside_n == 2 ? (outside_values[0] + outside_values[1]) * 0.5 : outside_values[0];
            ++contrast_count;
        }
        if (left[row] <= 1) ++left_clip_count;
        if (right[row] >= width - 2) ++right_clip_count;
        if (left[row] <= 1 && right[row] >= width - 2) ++dual_clip_count;
    }
    for (y = start_row; y < bottom_row; ++y) {
        if (widths[y] > 0 && widths[y + 1] > 0) {
            if (widths[y + 1] - widths[y] >= -2) ++monotonic_count;
        }
    }
    center_x = weight_sum > 0.0 ? weighted_center_sum / weight_sum : 0.0;
    edge_contrast = contrast_count ? inside_sum / contrast_count - outside_sum / contrast_count : 0.0;
    out->left_clip_ratio = (double)left_clip_count / stable_count;
    out->right_clip_ratio = (double)right_clip_count / stable_count;
    out->dual_clip_ratio = (double)dual_clip_count / stable_count;
    out->border_monotonic = stable_count > 1 ? (double)monotonic_count / (stable_count - 1) : 1.0;
    score = stable_count * 9.0 + edge_contrast * 3.5 + max_width * 0.8;
    score += (bottom_row - start_row > 0 ? bottom_row - start_row : 0) * 1.2;
    score += (max_width - start_width > 0 ? max_width - start_width : 0) * 0.4;
    score += out->border_monotonic * 60.0 + threshold * 0.25;
    score -= fabs(center_x - (width - 1) * 0.5) * 1.8;
    score -= out->left_clip_ratio * 25.0 + out->right_clip_ratio * 25.0 + out->dual_clip_ratio * 120.0;
    if (edge_contrast < 15.0) score -= 1500.0;
    if (max_width >= width - 4 && out->dual_clip_ratio > 0.55) score -= 2200.0;
    if (top_row <= 4 && out->dual_clip_ratio > 0.45) score -= 1400.0;
    if ((double)area / (width * height) > 0.72 && edge_contrast < 25.0) score -= 1200.0;
    if (max_width < 12) score -= 600.0;

    out->threshold = threshold;
    out->score = score;
    out->top_row = top_row;
    out->start_row = start_row;
    out->bottom_row = bottom_row;
    out->max_width = max_width;
    out->bottom_width = bottom_width;
    out->area = area;
    out->area_ratio = (double)area / (width * height);
    out->center_x = center_x;
    out->edge_contrast = edge_contrast;
    return 1;
}

static int compare_double(const void *a, const void *b)
{
    double da = *(const double *)a, db = *(const double *)b;
    return da < db ? -1 : da > db ? 1 : 0;
}

static double percentile_double(double *values, int count, double q)
{
    double pos, fraction;
    int lo, hi;
    qsort(values, (size_t)count, sizeof(values[0]), compare_double);
    pos = (count - 1) * q;
    lo = (int)floor(pos); hi = (int)ceil(pos); fraction = pos - lo;
    return values[lo] + (values[hi] - values[lo]) * fraction;
}

static void linear_fit(const double *independent, const double *dependent, const uint8_t *inliers,
                       int count, double *slope, double *intercept)
{
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    int n = 0, i;
    for (i = 0; i < count; ++i) if (inliers[i]) {
        sx += independent[i]; sy += dependent[i];
        sxx += independent[i] * independent[i]; sxy += independent[i] * dependent[i]; ++n;
    }
    if (n <= 1 || fabs(n * sxx - sx * sx) < 1e-12) { *slope = 0.0; *intercept = n ? sy / n : 0.0; return; }
    *slope = (n * sxy - sx * sy) / (n * sxx - sx * sx);
    *intercept = (sy - *slope * sx) / n;
}

enum { PREF_TOP, PREF_BOTTOM, PREF_LEFT, PREF_RIGHT };

static LineFit fit_line_exhaustive(const double *independent, const double *dependent, int count,
                                   double slope_min, double slope_max, double residual_threshold,
                                   int min_inliers, double min_span, double border_limit, int prefer)
{
    LineFit result;
    uint8_t best_inliers[B2_LINE_MAX_POINTS] = {0}, inliers[B2_LINE_MAX_POINTS] = {0};
    double residuals[B2_LINE_MAX_POINTS], subset[B2_LINE_MAX_POINTS];
    double best_score = -1e18, slope = 0.0, intercept = 0.0;
    int i, j, have_best = 0;
    memset(&result, 0, sizeof(result));
    if (count < min_inliers) return result;
    for (i = 0; i < count - 1; ++i) for (j = i + 1; j < count; ++j) {
        double delta = independent[j] - independent[i], test_slope, test_intercept;
        double min_v = 1e18, max_v = -1e18, sum_dep = 0.0, sum_res = 0.0, score;
        int n = 0, k;
        if (fabs(delta) < 3.0) continue;
        test_slope = (dependent[j] - dependent[i]) / delta;
        if (test_slope < slope_min || test_slope > slope_max) continue;
        test_intercept = dependent[i] - test_slope * independent[i];
        for (k = 0; k < count; ++k) {
            double r = fabs(dependent[k] - (test_slope * independent[k] + test_intercept));
            inliers[k] = (uint8_t)(r <= residual_threshold);
            if (inliers[k]) { ++n; if (independent[k] < min_v) min_v = independent[k]; if (independent[k] > max_v) max_v = independent[k]; sum_dep += dependent[k]; sum_res += r; }
        }
        if (n < min_inliers || max_v - min_v < min_span) continue;
        score = n * 12.0 + (max_v - min_v) * 2.0 - (sum_res / n) * 6.0;
        if (prefer == PREF_TOP) score -= (sum_dep / n) * 0.7;
        else if (prefer == PREF_BOTTOM) score += (sum_dep / n) * 0.7;
        else if (prefer == PREF_LEFT) score -= (sum_dep / n) * 0.25;
        else score += (sum_dep / n) * 0.25;
        if (score > best_score) { best_score = score; memcpy(best_inliers, inliers, (size_t)count); have_best = 1; }
    }
    if (!have_best) return result;
    linear_fit(independent, dependent, best_inliers, count, &slope, &intercept);
    {
        int subset_n = 0, n = 0;
        double cutoff;
        for (i = 0; i < count; ++i) {
            residuals[i] = fabs(dependent[i] - (slope * independent[i] + intercept));
            if (best_inliers[i]) subset[subset_n++] = residuals[i];
        }
        cutoff = percentile_double(subset, subset_n, 0.80) * 1.3;
        if (cutoff < residual_threshold) cutoff = residual_threshold;
        for (i = 0; i < count; ++i) { inliers[i] = (uint8_t)(residuals[i] <= cutoff); n += inliers[i]; }
        if (n < min_inliers) memcpy(inliers, best_inliers, (size_t)count);
    }
    linear_fit(independent, dependent, inliers, count, &slope, &intercept);
    {
        uint8_t second[B2_LINE_MAX_POINTS];
        int subset_n = 0, n = 0;
        double cutoff;
        for (i = 0; i < count; ++i) {
            residuals[i] = fabs(dependent[i] - (slope * independent[i] + intercept));
            if (inliers[i]) subset[subset_n++] = residuals[i];
        }
        cutoff = percentile_double(subset, subset_n, 0.80) * 1.2;
        if (cutoff < residual_threshold) cutoff = residual_threshold;
        for (i = 0; i < count; ++i) { second[i] = (uint8_t)(residuals[i] <= cutoff); n += second[i]; }
        if (n < min_inliers) memcpy(inliers, best_inliers, (size_t)count);
        else memcpy(inliers, second, (size_t)count);
    }
    {
        double support_min = 1e18, support_max = -1e18, sum_dep = 0.0, sum_res = 0.0;
        int n = 0, border_n = 0;
        for (i = 0; i < count; ++i) if (inliers[i]) {
            ++n; if (independent[i] < support_min) support_min = independent[i]; if (independent[i] > support_max) support_max = independent[i];
            sum_dep += dependent[i]; sum_res += residuals[i];
            if ((prefer == PREF_LEFT || prefer == PREF_TOP) ? dependent[i] <= border_limit : dependent[i] >= border_limit) ++border_n;
        }
        result.valid = n >= min_inliers;
        result.slope = slope; result.intercept = intercept; result.support_min = support_min; result.support_max = support_max;
        result.inlier_count = n; result.span = support_max - support_min; result.residual = n ? sum_res / n : 0.0;
        result.border_touch_ratio = n ? (double)border_n / n : 0.0; result.mean_value = n ? sum_dep / n : 0.0;
    }
    return result;
}

static void extract_row_borders(const uint8_t *mask, int width, int height, int *left, int *right, int *widths)
{
    int x, y;
    for (y = 0; y < height; ++y) {
        left[y] = -1; right[y] = -1; widths[y] = 0;
        for (x = 0; x < width; ++x) if (mask[y * width + x]) { if (left[y] < 0) left[y] = x; right[y] = x; }
        if (left[y] >= 0) widths[y] = right[y] - left[y] + 1;
    }
}

static LineFit fit_one_side(const uint8_t *mask, int width, int height, int side_right)
{
    int left[BRIDGE_DETECTION_MAX_HEIGHT], right[BRIDGE_DETECTION_MAX_HEIGHT], widths[BRIDGE_DETECTION_MAX_HEIGHT];
    double independent[BRIDGE_DETECTION_MAX_HEIGHT], dependent[BRIDGE_DETECTION_MAX_HEIGHT];
    int rows[BRIDGE_DETECTION_MAX_HEIGHT], unclipped[BRIDGE_DETECTION_MAX_HEIGHT];
    int row_count = 0, unclipped_count = 0, y, use_unclipped = 0, count = 0;
    extract_row_borders(mask, width, height, left, right, widths);
    for (y = 0; y < height; ++y) if (widths[y] > 0) {
        rows[row_count++] = y;
        if ((!side_right && left[y] > 1) || (side_right && right[y] < width - 2)) unclipped[unclipped_count++] = y;
    }
    if (row_count < 8) { LineFit none; memset(&none, 0, sizeof(none)); return none; }
    if (unclipped_count >= 6 && unclipped[unclipped_count - 1] - unclipped[0] >= 10) use_unclipped = 1;
    count = use_unclipped ? unclipped_count : row_count;
    for (y = 0; y < count; ++y) {
        int row = use_unclipped ? unclipped[y] : rows[y];
        independent[y] = row; dependent[y] = side_right ? right[row] : left[row];
    }
    return fit_line_exhaustive(independent, dependent, count,
        side_right ? 0.15 : -2.5, side_right ? 2.5 : 0.25,
        1.35, 6, 10.0, side_right ? width - 2.5 : 1.5, side_right ? PREF_RIGHT : PREF_LEFT);
}

static double score_side_fit(LineFit line)
{
    if (!line.valid) return -1e9;
    return line.inlier_count * 5.0 + line.span * 1.8 - line.residual * 10.0 - line.border_touch_ratio * 30.0 +
           fabs(line.slope) * 10.0 + fabs(line.slope) * line.span * 3.0;
}

static LineFit fit_horizontal(const uint8_t *mask, int width, int height, int bottom)
{
    double independent[BRIDGE_DETECTION_MAX_WIDTH], dependent[BRIDGE_DETECTION_MAX_WIDTH];
    int x, count = 0;
    for (x = 0; x < width; ++x) {
        int y, found = -1;
        if (!bottom) { for (y = 0; y < height; ++y) if (mask[y * width + x]) { found = y; break; } }
        else { for (y = height - 1; y >= 0; --y) if (mask[y * width + x]) { found = y; break; } }
        if (found >= 0) { independent[count] = x; dependent[count] = found; ++count; }
    }
    return fit_line_exhaustive(independent, dependent, count, -0.32, 0.32, 1.2, 6, 8.0,
                               bottom ? height - 2.5 : 1.5, bottom ? PREF_BOTTOM : PREF_TOP);
}

static LineFit fit_top_plateau(const uint8_t *mask, int width, int height)
{
    int top[BRIDGE_DETECTION_MAX_WIDTH], cols[BRIDGE_DETECTION_MAX_WIDTH];
    int col_count = 0, min_top = height, x, y, tolerance;
    LineFit none;
    memset(&none, 0, sizeof(none));
    for (x = 0; x < width; ++x) {
        top[x] = -1;
        for (y = 0; y < height; ++y) if (mask[y * width + x]) { top[x] = y; break; }
        if (top[x] >= 0) { cols[col_count++] = x; if (top[x] < min_top) min_top = top[x]; }
    }
    if (col_count < 6) return none;
    for (tolerance = 0; tolerance <= 3; ++tolerance) {
        int best_start = -1, best_end = -1, run_start = -1, i;
        for (i = 0; i <= col_count; ++i) {
            int eligible = i < col_count && top[cols[i]] <= min_top + tolerance;
            if (eligible && run_start < 0) run_start = i;
            if ((!eligible || i == col_count) && run_start >= 0) {
                int run_end = i - 1;
                if (best_start < 0 || run_end - run_start > best_end - best_start) { best_start = run_start; best_end = run_end; }
                run_start = -1;
            }
        }
        if (best_start >= 0 && best_end - best_start + 1 >= 6) {
            double independent[BRIDGE_DETECTION_MAX_WIDTH], dependent[BRIDGE_DETECTION_MAX_WIDTH];
            int count = best_end - best_start + 1, j;
            for (j = 0; j < count; ++j) { int col = cols[best_start + j]; independent[j] = col; dependent[j] = top[col]; }
            {
                LineFit fit = fit_line_exhaustive(independent, dependent, count, -0.32, 0.32, 0.9, 4, 6.0, 1.5, PREF_TOP);
                if (fit.valid) return fit;
            }
        }
    }
    return none;
}

static int should_show_left(LineFit line, const Candidate *candidate)
{
    if (!line.valid || line.inlier_count < 6 || line.span < 10.0) return 0;
    if (line.border_touch_ratio >= 0.75 && fabs(line.slope) <= 0.25 && line.mean_value <= 1.6) return 0;
    if (candidate->left_clip_ratio >= 0.75 && fabs(line.slope) <= 0.12 && line.mean_value <= 6.0) return 0;
    return 1;
}

static int should_show_right(LineFit line, int width, const Candidate *candidate)
{
    if (!line.valid || line.inlier_count < 6 || line.span < 10.0) return 0;
    if (line.border_touch_ratio >= 0.75 && fabs(line.slope) <= 0.25 && line.mean_value >= width - 2.6) return 0;
    if (candidate->max_width <= 36 && candidate->area_ratio <= 0.19 && line.mean_value >= width - 18.0) return 0;
    return 1;
}

static int should_show_top(LineFit line, const Candidate *candidate)
{
    if (!line.valid || line.inlier_count < 6 || line.span < 8.0) return 0;
    if (line.border_touch_ratio >= 0.75 && line.mean_value <= 1.6) return 0;
    if (candidate->top_row <= 1) return 0;
    return 1;
}

static int should_show_entry(LineFit line, int height, const Candidate *candidate)
{
    int min_bottom_width;
    if (!line.valid || line.inlier_count < 6 || line.span < 8.0) return 0;
    if (line.border_touch_ratio >= 0.55 && line.mean_value >= height - 2.6) return 0;
    if (line.slope <= -0.08 && line.border_touch_ratio <= 0.2 &&
        (candidate->top_row <= 5 || candidate->max_width <= 52 || candidate->top_row >= 10 ||
         (candidate->top_row <= 8 && candidate->bottom_width <= (candidate->max_width * 0.14 + 0.5 > 8 ? (int)(candidate->max_width * 0.14 + 0.5) : 8)))) return 1;
    min_bottom_width = (int)floor(candidate->max_width * 0.28 + 0.5); if (min_bottom_width < 16) min_bottom_width = 16;
    return candidate->bottom_width >= min_bottom_width && line.slope <= -0.02 && line.border_touch_ratio <= 0.4;
}

static void export_side_line(LineFit line, BridgeDetectionSideLine *out)
{
    memset(out, 0, sizeof(*out));
    if (!line.valid) return;
    out->valid = 1; out->slope = (float)line.slope; out->intercept = (float)line.intercept;
    out->support_min_y = (float)line.support_min; out->support_max_y = (float)line.support_max;
    out->inlier_count = line.inlier_count; out->span = (float)line.span; out->residual = (float)line.residual;
    out->border_touch_ratio = (float)line.border_touch_ratio; out->mean_x = (float)line.mean_value;
}

static BridgeDetectionSegment side_segment(LineFit line, int visible, int side_right, int bottom_row, int width, int height)
{
    BridgeDetectionSegment segment;
    double y0, y1, x0, x1;
    memset(&segment, 0, sizeof(segment));
    if (!visible || !line.valid) return segment;
    y0 = line.support_min; y1 = line.support_max;
    if (!side_right && bottom_row - y1 >= 8 && line.slope * bottom_row + line.intercept > 2.5) y1 = bottom_row;
    x0 = line.slope * y0 + line.intercept; x1 = line.slope * y1 + line.intercept;
    segment.valid = 1;
    segment.x0 = clamp_int(round_nearest(x0), 0, width - 1); segment.y0 = clamp_int(round_nearest(y0), 0, height - 1);
    segment.x1 = clamp_int(round_nearest(x1), 0, width - 1); segment.y1 = clamp_int(round_nearest(y1), 0, height - 1);
    return segment;
}

static BridgeDetectionSegment center_segment(BridgeDetectionSegment left, BridgeDetectionSegment right, int width, int height)
{
    BridgeDetectionSegment out;
    double ls, li, rs, ri, ms, mi, top_y, bottom_y;
    memset(&out, 0, sizeof(out));
    if (!left.valid || !right.valid || left.y1 == left.y0 || right.y1 == right.y0) return out;
    ls = (double)(left.x1 - left.x0) / (left.y1 - left.y0); li = left.x0 - ls * left.y0;
    rs = (double)(right.x1 - right.x0) / (right.y1 - right.y0); ri = right.x0 - rs * right.y0;
    ms = (ls + rs) * 0.5; mi = (li + ri) * 0.5;
    top_y = left.y0 < right.y0 ? left.y0 : right.y0; bottom_y = left.y1 > right.y1 ? left.y1 : right.y1;
    out.valid = 1;
    out.x0 = clamp_int(round_nearest(ms * top_y + mi), 0, width - 1); out.y0 = clamp_int(round_nearest(top_y), 0, height - 1);
    out.x1 = clamp_int(round_nearest(ms * bottom_y + mi), 0, width - 1); out.y1 = clamp_int(round_nearest(bottom_y), 0, height - 1);
    if (out.x0 == out.x1 && out.y0 == out.y1) out.valid = 0;
    return out;
}

int bridge_detection_detect_gray(const uint8_t *gray, int width, int height, int stride,
                            const BridgeDetectionConfig *config_in, BridgeDetectionScratch *scratch,
                            BridgeDetectionResult *result)
{
    BridgeDetectionConfig default_config;
    const BridgeDetectionConfig *config = config_in;
    Candidate best;
    int thresholds[11], threshold_count, t, pixels, index, have_best = 0;
    LineFit visible_left, visible_right, outer_left, outer_right, left, right, top, plateau, entry;
    if (result == NULL) return -1;
    bridge_detection_result_clear(result);
    if (gray == NULL || scratch == NULL || width <= 0 || height <= 0 || stride < width ||
        width > BRIDGE_DETECTION_MAX_WIDTH || height > BRIDGE_DETECTION_MAX_HEIGHT || width * height > 65535) return -2;
    if (config == NULL) { bridge_detection_default_config(&default_config); config = &default_config; }
    pixels = width * height;
    copy_gray_contiguous(gray, width, height, stride, scratch->gray);
    threshold_count = build_threshold_candidates(scratch->gray, pixels, thresholds);
    memset(&best, 0, sizeof(best));
    for (t = 0; t < threshold_count; ++t) {
        threshold_mask(scratch->gray, scratch->work0, width, height, thresholds[t]);
        close3_open2(scratch->work0, scratch->work1, width, height);
        for (index = 0; index < pixels; ++index) {
            int component_area;
            Candidate candidate;
            if (!scratch->work0[index]) continue;
            component_area = extract_component(scratch->work0, scratch->work2, scratch->queue, index, width, height);
            if (component_area < B2_MIN_COMPONENT_AREA) continue;
            if (!evaluate_component(scratch->gray, scratch->work2, thresholds[t], width, height, scratch, &candidate)) continue;
            if (!have_best || candidate.score > best.score) {
                best = candidate; have_best = 1;
                memcpy(scratch->best_visible, scratch->work4, (size_t)pixels);
                memcpy(scratch->best_outer, scratch->work2, (size_t)pixels);
            }
        }
    }
    if (!have_best) return 0;
    result->candidate_found = 1;
    result->threshold = best.threshold; result->candidate_score = (float)best.score; result->area = best.area;
    result->area_ratio = (float)best.area_ratio; result->top_row = best.top_row; result->start_row = best.start_row;
    result->bottom_row = best.bottom_row; result->max_width = best.max_width; result->bottom_width = best.bottom_width;
    result->center_x = (float)best.center_x; result->edge_contrast = (float)best.edge_contrast;
    result->left_clip_ratio = (float)best.left_clip_ratio; result->right_clip_ratio = (float)best.right_clip_ratio;
    result->dual_clip_ratio = (float)best.dual_clip_ratio; result->border_monotonic = (float)best.border_monotonic;

    visible_left = fit_one_side(scratch->best_visible, width, height, 0);
    visible_right = fit_one_side(scratch->best_visible, width, height, 1);
    outer_left = fit_one_side(scratch->best_outer, width, height, 0);
    outer_right = fit_one_side(scratch->best_outer, width, height, 1);
    left = score_side_fit(visible_left) >= score_side_fit(outer_left) ? visible_left : outer_left;
    right = score_side_fit(visible_right) >= score_side_fit(outer_right) ? visible_right : outer_right;
    top = fit_horizontal(scratch->best_outer, width, height, 0);
    plateau = fit_top_plateau(scratch->best_outer, width, height);
    entry = fit_horizontal(scratch->best_outer, width, height, 1);
    export_side_line(left, &result->left_line); export_side_line(right, &result->right_line);
    result->left_line_visible = (uint8_t)should_show_left(left, &best);
    result->right_line_visible = (uint8_t)should_show_right(right, width, &best);
    result->top_line_visible = (uint8_t)(((result->left_line_visible && result->right_line_visible && plateau.valid) || should_show_top(top, &best)) && best.top_row > 1);
    result->entry_line_visible = (uint8_t)should_show_entry(entry, height, &best);
    if (result->top_line_visible && result->entry_line_visible && best.top_row <= 5 && entry.slope <= -0.08) result->top_line_visible = 0;
    if (result->top_line_visible && !result->entry_line_visible && best.top_row <= 4 && best.left_clip_ratio >= 0.4 &&
        best.max_width >= 85 && best.bottom_width >= 70) result->top_line_visible = 0;

    /* The Python annotation stage suppresses the lower edge for a far-right,
     * short side stub. It also re-runs state inference afterwards, so retain
     * this non-rendering consequence in the C state output. */
    if (best.center_x >= width * 0.62 && best.max_width <= 50 &&
        best.bottom_width <= ((int)floor(best.max_width * 0.08 + 0.5) > 6 ? (int)floor(best.max_width * 0.08 + 0.5) : 6) &&
        best.left_clip_ratio < 0.1 && best.right_clip_ratio < 0.1 && best.top_row >= 10 && best.top_row <= 24 &&
        right.valid && fabs(right.slope) <= 0.08) {
        result->entry_line_visible = 0;
    }

    if (best.score >= config->min_valid_score && best.edge_contrast >= config->min_edge_contrast) {
        result->bridge_found = 1;
        if (result->entry_line_visible || best.bottom_row <= height - 10) result->state = BRIDGE_DETECTION_STATE_PREPARE_ENTER;
        else if ((best.left_clip_ratio > best.right_clip_ratio ? best.left_clip_ratio : best.right_clip_ratio) >= 0.82) result->state = BRIDGE_DETECTION_STATE_PREPARE_EXIT;
        else if ((best.left_clip_ratio > best.right_clip_ratio ? best.left_clip_ratio : best.right_clip_ratio) >= 0.68 && best.start_row >= 18) result->state = BRIDGE_DETECTION_STATE_PREPARE_EXIT;
        else result->state = BRIDGE_DETECTION_STATE_ON_BRIDGE;
    }
    if (!result->bridge_found) {
        result->left_line_visible = 0;
        result->right_line_visible = 0;
        result->top_line_visible = 0;
        result->entry_line_visible = 0;
    }
    if (result->bridge_found && best.top_row >= 30 && best.center_x >= width * 0.70 && best.max_width <= 40 &&
        best.bottom_width >= ((int)floor(best.max_width * 0.75 + 0.5) > 28 ? (int)floor(best.max_width * 0.75 + 0.5) : 28)) {
        result->right_line_visible = 0;
    }
    if (result->bridge_found) {
        result->left_segment = side_segment(left, result->left_line_visible, 0, best.bottom_row, width, height);
        result->right_segment = side_segment(right, result->right_line_visible, 1, best.bottom_row, width, height);
        result->center_segment = center_segment(result->left_segment, result->right_segment, width, height);
        if (result->center_segment.valid) {
            double dy = result->center_segment.y1 - result->center_segment.y0;
            result->control_center_x = (float)result->center_segment.x1;
            result->lateral_error_px = result->control_center_x - (float)((width - 1) * 0.5);
            result->heading_dx_per_dy = dy != 0.0 ? (float)((result->center_segment.x1 - result->center_segment.x0) / dy) : 0.0f;
        }
    }
    return 1;
}
