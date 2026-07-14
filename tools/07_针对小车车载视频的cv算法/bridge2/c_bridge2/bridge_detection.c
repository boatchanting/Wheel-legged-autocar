#include "bridge_detection.h"

#include <stddef.h>
#include <string.h>

#define BD_MIN_COMPONENT_AREA 24
#define BD_MAX_LINE_POINTS BRIDGE_DETECTION_MAX_WIDTH
#define BD_MAX_HULL_POINTS (BRIDGE_DETECTION_MAX_WIDTH * 4)
#define BD_LINE_SAMPLE_COUNT 24
#define BD_CACHE_MAGIC 0x42444745u
#define BD_TEMPORAL_MAX_STREAK 3
#define BD_QUEUE_X_BITS 7
#define BD_QUEUE_X_MASK ((1u << BD_QUEUE_X_BITS) - 1u)

typedef struct {
    int threshold;
    float score;
    int top_row;
    int start_row;
    int bottom_row;
    int max_width;
    int bottom_width;
    int area;
    float area_ratio;
    float center_x;
    float edge_contrast;
    float left_clip_ratio;
    float right_clip_ratio;
    float dual_clip_ratio;
    float border_monotonic;
} Candidate;

typedef struct {
    int valid;
    float slope;
    float intercept;
    float support_min;
    float support_max;
    int inlier_count;
    float span;
    float residual;
    float border_touch_ratio;
    float mean_value;
} LineFit;

typedef struct { int x; int y; } PointI;

enum { PREF_TOP, PREF_BOTTOM, PREF_LEFT, PREF_RIGHT };

static float absf_fast(float value) { return value < 0.0f ? -value : value; }

static int clamp_int(int value, int lo, int hi)
{
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

static int round_positive(float value)
{
    return (int)(value + 0.5f);
}

static int ctz32(uint32_t value)
{
    static const uint8_t table[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    return table[((value & (0u - value)) * 0x077CB531u) >> 27];
}

static int msb32(uint32_t value)
{
    static const uint8_t table[32] = {
        0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return table[(value * 0x07C4ACDDu) >> 27];
}

static uint32_t valid_last_word_mask(int width)
{
    int bits = width & 31;
    return bits ? ((1u << bits) - 1u) : 0xFFFFFFFFu;
}

static int words_for_width(int width) { return (width + 31) >> 5; }

static int cache_matches(const BridgeDetectionScratch *scratch, const uint8_t *gray,
                         int width, int height, int stride, const BridgeDetectionConfig *config)
{
    int y;
    if (scratch->cache_magic != BD_CACHE_MAGIC || scratch->cache_width != width || scratch->cache_height != height ||
        scratch->cache_min_valid_score != config->min_valid_score ||
        scratch->cache_min_edge_contrast != config->min_edge_contrast) return 0;
    for (y = 0; y < height; ++y) {
        if (memcmp(scratch->previous_gray + y * width, gray + y * stride, (size_t)width) != 0) return 0;
    }
    return 1;
}

static void cache_store(BridgeDetectionScratch *scratch, const uint8_t *gray,
                        int width, int height, int stride, const BridgeDetectionConfig *config,
                        const BridgeDetectionResult *result, int status)
{
    int y;
    for (y = 0; y < height; ++y) memcpy(scratch->previous_gray + y * width, gray + y * stride, (size_t)width);
    scratch->cached_result = *result;
    scratch->cache_width = (uint16_t)width;
    scratch->cache_height = (uint16_t)height;
    scratch->cache_min_valid_score = config->min_valid_score;
    scratch->cache_min_edge_contrast = config->min_edge_contrast;
    scratch->cache_status = status;
    scratch->cache_magic = BD_CACHE_MAGIC;
}

static int temporal_input_is_small_change(const BridgeDetectionScratch *scratch,
                                          const uint8_t *gray, int width, int height, int stride)
{
    int changed = 0, x, y;
    for (y = 0; y < height; ++y) {
        const uint8_t *current = gray + y * stride;
        const uint8_t *previous = scratch->previous_gray + y * width;
        for (x = 0; x < width; ++x) {
            int delta = (int)current[x] - (int)previous[x];
            if (delta < 0) delta = -delta;
            if (delta > 8 && ++changed > 96) return 0;
        }
    }
    return 1;
}

static void bitmap_clear(BridgeDetectionBitmap *bitmap)
{
    memset(bitmap, 0, sizeof(*bitmap));
}

static void bitmap_copy(BridgeDetectionBitmap *dst, const BridgeDetectionBitmap *src)
{
    memcpy(dst, src, sizeof(*dst));
}

static int bitmap_get(const BridgeDetectionBitmap *bitmap, int x, int y)
{
    return (int)((bitmap->row[y][x >> 5] >> (x & 31)) & 1u);
}

static void bitmap_set_range(BridgeDetectionBitmap *bitmap, int y, int left, int right)
{
    int first_word = left >> 5;
    int last_word = right >> 5;
    uint32_t first_mask = 0xFFFFFFFFu << (left & 31);
    uint32_t last_mask = (right & 31) == 31 ? 0xFFFFFFFFu : ((1u << ((right & 31) + 1)) - 1u);
    if (first_word == last_word) {
        bitmap->row[y][first_word] |= first_mask & last_mask;
    } else {
        int word;
        bitmap->row[y][first_word] |= first_mask;
        for (word = first_word + 1; word < last_word; ++word) bitmap->row[y][word] = 0xFFFFFFFFu;
        bitmap->row[y][last_word] |= last_mask;
    }
}

static void bitmap_clear_range(BridgeDetectionBitmap *bitmap, int y, int left, int right)
{
    int first_word = left >> 5;
    int last_word = right >> 5;
    uint32_t first_mask = 0xFFFFFFFFu << (left & 31);
    uint32_t last_mask = (right & 31) == 31 ? 0xFFFFFFFFu : ((1u << ((right & 31) + 1)) - 1u);
    if (first_word == last_word) {
        bitmap->row[y][first_word] &= ~(first_mask & last_mask);
    } else {
        int word;
        bitmap->row[y][first_word] &= ~first_mask;
        for (word = first_word + 1; word < last_word; ++word) bitmap->row[y][word] = 0u;
        bitmap->row[y][last_word] &= ~last_mask;
    }
}

static void row_shift_left(const uint32_t *src, uint32_t *dst, int words)
{
    int word;
    for (word = 0; word < words; ++word) {
        dst[word] = src[word] << 1;
        if (word > 0) dst[word] |= src[word - 1] >> 31;
    }
}

static void row_shift_right(const uint32_t *src, uint32_t *dst, int words)
{
    int word;
    for (word = 0; word < words; ++word) {
        dst[word] = src[word] >> 1;
        if (word + 1 < words) dst[word] |= src[word + 1] << 31;
    }
}

static void dilate3(const BridgeDetectionBitmap *src, BridgeDetectionBitmap *dst, int width, int height)
{
    int y, word, words = words_for_width(width);
    uint32_t last_mask = valid_last_word_mask(width);
    bitmap_clear(dst);
    for (y = 0; y < height; ++y) {
        uint32_t merged[BRIDGE_DETECTION_WORDS_PER_ROW] = {0};
        uint32_t left[BRIDGE_DETECTION_WORDS_PER_ROW], right[BRIDGE_DETECTION_WORDS_PER_ROW];
        for (word = 0; word < words; ++word) {
            uint32_t value = src->row[y][word];
            if (y > 0) value |= src->row[y - 1][word];
            if (y + 1 < height) value |= src->row[y + 1][word];
            merged[word] = value;
        }
        row_shift_left(merged, left, words);
        row_shift_right(merged, right, words);
        for (word = 0; word < words; ++word) dst->row[y][word] = merged[word] | left[word] | right[word];
        dst->row[y][words - 1] &= last_mask;
    }
}

static void erode3(const BridgeDetectionBitmap *src, BridgeDetectionBitmap *dst, int width, int height)
{
    int y, word, words = words_for_width(width);
    uint32_t last_mask = valid_last_word_mask(width);
    bitmap_clear(dst);
    for (y = 1; y + 1 < height; ++y) {
        uint32_t merged[BRIDGE_DETECTION_WORDS_PER_ROW];
        uint32_t left[BRIDGE_DETECTION_WORDS_PER_ROW], right[BRIDGE_DETECTION_WORDS_PER_ROW];
        for (word = 0; word < words; ++word) merged[word] = src->row[y - 1][word] & src->row[y][word] & src->row[y + 1][word];
        row_shift_left(merged, left, words);
        row_shift_right(merged, right, words);
        for (word = 0; word < words; ++word) dst->row[y][word] = merged[word] & left[word] & right[word];
        dst->row[y][words - 1] &= last_mask;
    }
}

static void erode2(const BridgeDetectionBitmap *src, BridgeDetectionBitmap *dst, int width, int height)
{
    int y, word, words = words_for_width(width);
    uint32_t last_mask = valid_last_word_mask(width);
    bitmap_clear(dst);
    for (y = 1; y < height; ++y) {
        uint32_t merged[BRIDGE_DETECTION_WORDS_PER_ROW];
        uint32_t left[BRIDGE_DETECTION_WORDS_PER_ROW];
        for (word = 0; word < words; ++word) merged[word] = src->row[y - 1][word] & src->row[y][word];
        row_shift_left(merged, left, words);
        for (word = 0; word < words; ++word) dst->row[y][word] = merged[word] & left[word];
        dst->row[y][words - 1] &= last_mask;
    }
}

static void dilate2(const BridgeDetectionBitmap *src, BridgeDetectionBitmap *dst, int width, int height)
{
    int y, word, words = words_for_width(width);
    uint32_t last_mask = valid_last_word_mask(width);
    bitmap_clear(dst);
    for (y = 0; y < height; ++y) {
        uint32_t merged[BRIDGE_DETECTION_WORDS_PER_ROW] = {0};
        uint32_t right[BRIDGE_DETECTION_WORDS_PER_ROW];
        for (word = 0; word < words; ++word) {
            merged[word] = src->row[y][word];
            if (y + 1 < height) merged[word] |= src->row[y + 1][word];
        }
        row_shift_right(merged, right, words);
        for (word = 0; word < words; ++word) dst->row[y][word] = merged[word] | right[word];
        dst->row[y][words - 1] &= last_mask;
    }
}

static void close3_open2(BridgeDetectionBitmap *mask, BridgeDetectionBitmap *temp, int width, int height)
{
    dilate3(mask, temp, width, height);
    erode3(temp, mask, width, height);
    erode2(mask, temp, width, height);
    dilate2(temp, mask, width, height);
}

static uint32_t isqrt64(uint64_t value)
{
    uint64_t bit = (uint64_t)1 << 62;
    uint64_t result = 0;
    while (bit > value) bit >>= 2;
    while (bit != 0) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return (uint32_t)result;
}

static int otsu_threshold_limited(const int hist[256], int pixels)
{
    int threshold, best = 0;
    int cumulative_count = 0;
    int cumulative_sum = 0;
    int limited_sum = 0;
    float best_score = -1.0f;
    for (threshold = 0; threshold <= 180; ++threshold) limited_sum += hist[threshold] * threshold;
    for (threshold = 0; threshold < 180; ++threshold) {
        int other_count;
        float w0, w1, mean0, mean1, delta, score;
        cumulative_count += hist[threshold];
        cumulative_sum += hist[threshold] * threshold;
        other_count = pixels - cumulative_count;
        if (cumulative_count == 0 || other_count == 0) continue;
        w0 = (float)cumulative_count / (float)pixels;
        w1 = 1.0f - w0;
        mean0 = (float)cumulative_sum / (float)cumulative_count;
        mean1 = (float)(limited_sum - cumulative_sum) / (float)other_count;
        delta = mean0 - mean1;
        score = w0 * w1 * delta * delta;
        if (score > best_score) { best_score = score; best = threshold; }
    }
    return best < 70 ? 70 : best;
}

static int percentile_hist(const int hist[256], int pixels, int percent)
{
    int numerator = (pixels - 1) * percent;
    int lo = numerator / 100;
    int rem = numerator % 100;
    int hi = lo + (rem != 0);
    int cumulative = 0, lo_value = -1, hi_value = 0, value;
    for (value = 0; value < 256; ++value) {
        cumulative += hist[value];
        if (lo_value < 0 && cumulative > lo) lo_value = value;
        if (cumulative > hi) { hi_value = value; break; }
    }
    if (!rem) return lo_value;
    return lo_value + ((hi_value - lo_value) * rem) / 100;
}

static void sort_small_int(int *values, int count)
{
    int i;
    for (i = 1; i < count; ++i) {
        int value = values[i], j = i - 1;
        while (j >= 0 && values[j] > value) { values[j + 1] = values[j]; --j; }
        values[j + 1] = value;
    }
}

static int build_threshold_candidates(const uint8_t *gray, int width, int height, int stride, int out[11])
{
    int hist[256] = {0};
    int values[11], unique = 0, x, y, i;
    uint32_t sum = 0;
    uint64_t sum_sq = 0;
    int pixels = width * height;
    int base;
    uint64_t variance_numerator;
    uint32_t std_times_pixels;
    for (y = 0; y < height; ++y) {
        const uint8_t *row = gray + y * stride;
        for (x = 0; x < width; ++x) {
            uint32_t value = row[x];
            ++hist[value]; sum += value; sum_sq += value * value;
        }
    }
    base = otsu_threshold_limited(hist, pixels);
    variance_numerator = sum_sq * (uint64_t)pixels - (uint64_t)sum * sum;
    std_times_pixels = isqrt64(variance_numerator);
    values[0] = base - 25;
    values[1] = base - 15;
    values[2] = base - 8;
    values[3] = base;
    values[4] = percentile_hist(hist, pixels, 82);
    values[5] = percentile_hist(hist, pixels, 86);
    values[6] = percentile_hist(hist, pixels, 90);
    values[7] = percentile_hist(hist, pixels, 92);
    values[8] = (int)(((uint64_t)sum * 100u + (uint64_t)std_times_pixels * 45u) / ((uint64_t)pixels * 100u));
    values[9] = (int)(((uint64_t)sum * 100u + (uint64_t)std_times_pixels * 75u) / ((uint64_t)pixels * 100u));
    values[10] = (int)(((uint64_t)sum * 100u + (uint64_t)std_times_pixels * 95u) / ((uint64_t)pixels * 100u));
    for (i = 0; i < 11; ++i) values[i] = clamp_int(values[i], 90, 225);
    sort_small_int(values, 11);
    for (i = 0; i < 11; ++i) if (unique == 0 || values[i] != out[unique - 1]) out[unique++] = values[i];
    return unique;
}

static void threshold_mask(const uint8_t *gray, BridgeDetectionBitmap *mask,
                           int width, int height, int stride, int threshold)
{
    int x, y;
    bitmap_clear(mask);
    for (y = 0; y < height; ++y) {
        const uint8_t *row = gray + y * stride;
#define BD_THRESHOLD_RANGE(begin_, end_, value_) do { \
    int bd_end = (end_) < width ? (end_) : width; \
    for (x = (begin_); x < bd_end; ++x) if (row[x] > (value_)) mask->row[y][x >> 5] |= 1u << (x & 31); \
} while (0)
        BD_THRESHOLD_RANGE(0, 19, threshold - 10);
        if (width > 19) BD_THRESHOLD_RANGE(19, 76, threshold);
        if (width > 76) BD_THRESHOLD_RANGE(76, 83, threshold - 10);
        if (width > 83) BD_THRESHOLD_RANGE(83, 89, threshold - 20);
        if (width > 89) BD_THRESHOLD_RANGE(89, width, threshold - 10);
#undef BD_THRESHOLD_RANGE
    }
}

static int component_seed_span(BridgeDetectionBitmap *global_mask, BridgeDetectionBitmap *component,
                               uint16_t *queue, int *tail, int x, int y, int width, int *span_length)
{
    int left = x, right = x;
    while (left > 0 && bitmap_get(global_mask, left - 1, y)) --left;
    while (right + 1 < width && bitmap_get(global_mask, right + 1, y)) ++right;
    bitmap_clear_range(global_mask, y, left, right);
    bitmap_set_range(component, y, left, right);
    /* Queue entries are span starts.  Packing x/y removes the expensive
     * divide/modulo pair from every breadth-first span dequeue.  X needs
     * seven bits for the fixed <=96-pixel input width. */
    queue[(*tail)++] = (uint16_t)(((uint16_t)y << BD_QUEUE_X_BITS) | (uint16_t)left);
    *span_length = right - left + 1;
    return right;
}

static int extract_component(BridgeDetectionBitmap *global_mask, BridgeDetectionBitmap *component,
                             uint16_t *queue, int start, int width, int height)
{
    int head = 0, tail = 0, area = 0;
    int sx = start % width, sy = start / width;
    int span_length;
    bitmap_clear(component);
    component_seed_span(global_mask, component, queue, &tail, sx, sy, width, &span_length);
    area += span_length;
    while (head < tail) {
        uint16_t packed = queue[head++];
        int left = (int)(packed & BD_QUEUE_X_MASK);
        int y = (int)(packed >> BD_QUEUE_X_BITS);
        int right = left;
        int adjacent_index;
        while (right + 1 < width && bitmap_get(component, right + 1, y)) ++right;
        for (adjacent_index = 0; adjacent_index < 2; ++adjacent_index) {
            int adjacent_y = adjacent_index == 0 ? y - 1 : y + 1;
            int x = left;
            if (adjacent_y < 0 || adjacent_y >= height) continue;
            while (x <= right) {
                if (bitmap_get(global_mask, x, adjacent_y)) {
                    int span_right = component_seed_span(global_mask, component, queue, &tail, x, adjacent_y, width, &span_length);
                    area += span_length;
                    x = span_right + 1;
                } else {
                    ++x;
                }
            }
        }
    }
    return area;
}

static long cross_point(PointI o, PointI a, PointI b)
{
    return (long)(a.x - o.x) * (long)(b.y - o.y) - (long)(a.y - o.y) * (long)(b.x - o.x);
}

static int extract_row_borders(const BridgeDetectionBitmap *mask, int width, int height,
                               int *left, int *right, int *widths)
{
    int y, word, words = words_for_width(width), count = 0;
    for (y = 0; y < height; ++y) {
        int first = -1, last = -1;
        for (word = 0; word < words; ++word) {
            uint32_t bits = mask->row[y][word];
            if (!bits) continue;
            if (first < 0) first = (word << 5) + ctz32(bits);
            last = (word << 5) + msb32(bits);
        }
        if (last >= width) last = width - 1;
        left[y] = first; right[y] = last;
        widths[y] = first >= 0 ? last - first + 1 : 0;
        if (first >= 0) ++count;
    }
    return count;
}

static int convex_hull_mask(const BridgeDetectionBitmap *src, BridgeDetectionBitmap *dst,
                            int width, int height, int *left, int *right, int *widths, int *area_out)
{
    PointI points[BRIDGE_DETECTION_MAX_WIDTH * 2];
    PointI hull[BD_MAX_HULL_POINTS];
    int column_top[BRIDGE_DETECTION_MAX_WIDTH];
    int column_bottom[BRIDGE_DETECTION_MAX_WIDTH];
    int point_count = 0, hull_count = 0, lower_count, x, y, i, reverse_first = 1;
    int area = 0;

    /* The old implementation searched every column from both ends, i.e.
     * 2*width*height bitmap tests.  A single forward row scan obtains the
     * exact same per-column extrema and is considerably cheaper on M7. */
    for (x = 0; x < width; ++x) {
        column_top[x] = height;
        column_bottom[x] = -1;
    }
    for (y = 0; y < height; ++y) {
        int word;
        int words = words_for_width(width);
        for (word = 0; word < words; ++word) {
            uint32_t bits = src->row[y][word];
            while (bits != 0u) {
                int bit_x = (word << 5) + ctz32(bits);
                if (bit_x < width) {
                    if (column_top[bit_x] == height) column_top[bit_x] = y;
                    column_bottom[bit_x] = y;
                }
                bits &= bits - 1u;
            }
        }
    }
    for (x = 0; x < width; ++x) {
        int top = column_top[x];
        int bottom = column_bottom[x];
        if (bottom < 0) continue;
        points[point_count].x = x; points[point_count++].y = top;
        if (bottom != top) { points[point_count].x = x; points[point_count++].y = bottom; }
    }
    bitmap_clear(dst);
    if (point_count < 3) return 0;
    for (i = 0; i < point_count; ++i) {
        while (hull_count >= 2 && cross_point(hull[hull_count - 2], hull[hull_count - 1], points[i]) <= 0) --hull_count;
        hull[hull_count++] = points[i];
    }
    lower_count = hull_count;
    for (i = point_count - 1; i >= 0; --i) {
        if (reverse_first) { reverse_first = 0; continue; }
        while (hull_count > lower_count && cross_point(hull[hull_count - 2], hull[hull_count - 1], points[i]) <= 0) --hull_count;
        hull[hull_count++] = points[i];
    }
    if (hull_count > 1) --hull_count;
    for (y = 0; y < height; ++y) {
        int32_t min_q = 0x7FFFFFFF, max_q = -1;
        for (i = 0; i < hull_count; ++i) {
            PointI a = hull[i], b = hull[(i + 1) % hull_count];
            if (a.y == b.y) {
                if (y == a.y) {
                    int32_t aq = a.x << 16, bq = b.x << 16;
                    if (aq < min_q) min_q = aq; if (aq > max_q) max_q = aq;
                    if (bq < min_q) min_q = bq; if (bq > max_q) max_q = bq;
                }
            } else if (y >= (a.y < b.y ? a.y : b.y) && y <= (a.y > b.y ? a.y : b.y)) {
                int32_t hit_q = (a.x << 16) + (int32_t)(((int64_t)(b.x - a.x) * (y - a.y) << 16) / (b.y - a.y));
                if (hit_q < min_q) min_q = hit_q;
                if (hit_q > max_q) max_q = hit_q;
            }
        }
        if (min_q <= max_q) {
            int row_left = clamp_int((min_q + 0x8000) >> 16, 0, width - 1);
            int row_right = clamp_int((max_q + 0x7FFF) >> 16, 0, width - 1);
            left[y] = row_left; right[y] = row_right; widths[y] = row_right - row_left + 1;
            area += widths[y]; bitmap_set_range(dst, y, row_left, row_right);
        } else {
            left[y] = -1; right[y] = -1; widths[y] = 0;
        }
    }
    *area_out = area;
    return 1;
}

static int evaluate_component(const uint8_t *gray, int stride, const BridgeDetectionBitmap *component,
                              int threshold, int width, int height, BridgeDetectionScratch *scratch, Candidate *out)
{
    int left[BRIDGE_DETECTION_MAX_HEIGHT], right[BRIDGE_DETECTION_MAX_HEIGHT], widths[BRIDGE_DETECTION_MAX_HEIGHT];
    int valid_rows[BRIDGE_DETECTION_MAX_HEIGHT];
    int valid_count = 0, stable_count, top_row, bottom_row, max_width = 0, start_row, min_width;
    int area = 0, bottom_width, start_width, y, i;
    int left_clip = 0, right_clip = 0, dual_clip = 0, monotonic = 0, contrast_count = 0;
    float center_sum = 0.0f, weight_sum = 0.0f, inside_sum = 0.0f, outside_sum = 0.0f;
    float center_x, edge_contrast, score;
    BridgeDetectionBitmap *visible = &scratch->work4;
    BridgeDetectionBitmap *outer = &scratch->work2;
    bitmap_copy(visible, component);
    close3_open2(visible, &scratch->work1, width, height);
    /* Filling interior holes is redundant before a convex hull: it cannot
     * add an extreme point or change the hull boundary. */
    if (!convex_hull_mask(visible, outer, width, height, left, right, widths, &area)) {
        bitmap_copy(outer, visible);
        extract_row_borders(outer, width, height, left, right, widths);
        area = 0;
        for (y = 0; y < height; ++y) area += widths[y];
    }
    for (y = 0; y < height; ++y) if (widths[y] > 0) {
        valid_rows[valid_count++] = y;
        if (widths[y] > max_width) max_width = widths[y];
    }
    if (valid_count < 10) return 0;
    top_row = valid_rows[0]; bottom_row = valid_rows[valid_count - 1];
    min_width = (max_width * 12 + 50) / 100; if (min_width < 6) min_width = 6;
    start_row = valid_rows[0];
    for (i = 0; i < valid_count; ++i) {
        int next_count = valid_count - i, ok = 1, j, limit = min_width - 2;
        if (widths[valid_rows[i]] < min_width) continue;
        if (next_count > 3) next_count = 3;
        if (next_count < 2) continue;
        if (limit < 4) limit = 4;
        for (j = 0; j < next_count; ++j) if (widths[valid_rows[i + j]] < limit) { ok = 0; break; }
        if (ok) { start_row = valid_rows[i]; break; }
    }
    stable_count = 0;
    for (i = 0; i < valid_count; ++i) if (valid_rows[i] >= start_row) ++stable_count;
    if (stable_count < 10) return 0;
    bottom_width = widths[bottom_row]; start_width = widths[start_row];
    for (y = start_row; y <= bottom_row; ++y) if (widths[y] > 0) {
        float outside = 0.0f; int outside_n = 0;
        center_sum += (left[y] + right[y]) * 0.5f * widths[y]; weight_sum += (float)widths[y];
        if (left[y] >= 2) { outside += gray[y * stride + left[y] - 2]; ++outside_n; }
        else if (left[y] >= 1) { outside += gray[y * stride + left[y] - 1]; ++outside_n; }
        if (right[y] <= width - 3) { outside += gray[y * stride + right[y] + 2]; ++outside_n; }
        else if (right[y] <= width - 2) { outside += gray[y * stride + right[y] + 1]; ++outside_n; }
        if (outside_n) {
            inside_sum += (gray[y * stride + left[y]] + gray[y * stride + right[y]]) * 0.5f;
            outside_sum += outside / outside_n; ++contrast_count;
        }
        if (left[y] <= 1) ++left_clip;
        if (right[y] >= width - 2) ++right_clip;
        if (left[y] <= 1 && right[y] >= width - 2) ++dual_clip;
    }
    for (y = start_row; y < bottom_row; ++y) if (widths[y] > 0 && widths[y + 1] > 0 && widths[y + 1] - widths[y] >= -2) ++monotonic;
    center_x = center_sum / weight_sum;
    edge_contrast = contrast_count ? inside_sum / contrast_count - outside_sum / contrast_count : 0.0f;
    out->left_clip_ratio = (float)left_clip / stable_count;
    out->right_clip_ratio = (float)right_clip / stable_count;
    out->dual_clip_ratio = (float)dual_clip / stable_count;
    out->border_monotonic = stable_count > 1 ? (float)monotonic / (stable_count - 1) : 1.0f;
    score = stable_count * 9.0f + edge_contrast * 3.5f + max_width * 0.8f;
    score += (bottom_row - start_row > 0 ? bottom_row - start_row : 0) * 1.2f;
    score += (max_width - start_width > 0 ? max_width - start_width : 0) * 0.4f;
    score += out->border_monotonic * 60.0f + threshold * 0.25f;
    score -= absf_fast(center_x - (width - 1) * 0.5f) * 1.8f;
    score -= out->left_clip_ratio * 25.0f + out->right_clip_ratio * 25.0f + out->dual_clip_ratio * 120.0f;
    if (edge_contrast < 15.0f) score -= 1500.0f;
    if (max_width >= width - 4 && out->dual_clip_ratio > 0.55f) score -= 2200.0f;
    if (top_row <= 4 && out->dual_clip_ratio > 0.45f) score -= 1400.0f;
    if ((float)area / (width * height) > 0.72f && edge_contrast < 25.0f) score -= 1200.0f;
    if (max_width < 12) score -= 600.0f;
    out->threshold = threshold; out->score = score; out->top_row = top_row; out->start_row = start_row;
    out->bottom_row = bottom_row; out->max_width = max_width; out->bottom_width = bottom_width;
    out->area = area; out->area_ratio = (float)area / (width * height); out->center_x = center_x; out->edge_contrast = edge_contrast;
    return 1;
}

static void insertion_sort_float(float *values, int count)
{
    int i;
    for (i = 1; i < count; ++i) {
        float value = values[i]; int j = i - 1;
        while (j >= 0 && values[j] > value) { values[j + 1] = values[j]; --j; }
        values[j + 1] = value;
    }
}

static float percentile_float(float *values, int count, int percent)
{
    int numerator, lo, rem, hi;
    insertion_sort_float(values, count);
    numerator = (count - 1) * percent; lo = numerator / 100; rem = numerator % 100; hi = lo + (rem != 0);
    return values[lo] + (values[hi] - values[lo]) * (rem * 0.01f);
}

static void linear_fit(const float *independent, const float *dependent, const uint8_t *inliers,
                       int count, float *slope, float *intercept)
{
    float sx = 0.0f, sy = 0.0f, sxx = 0.0f, sxy = 0.0f;
    int n = 0, i;
    for (i = 0; i < count; ++i) if (inliers[i]) {
        sx += independent[i]; sy += dependent[i]; sxx += independent[i] * independent[i]; sxy += independent[i] * dependent[i]; ++n;
    }
    if (n <= 1 || absf_fast(n * sxx - sx * sx) < 1e-5f) { *slope = 0.0f; *intercept = n ? sy / n : 0.0f; return; }
    *slope = (n * sxy - sx * sy) / (n * sxx - sx * sx);
    *intercept = (sy - *slope * sx) / n;
}

static int build_sample_indices(int count, int *sample)
{
    int i, sample_count;
    if (count <= BD_LINE_SAMPLE_COUNT) { for (i = 0; i < count; ++i) sample[i] = i; return count; }
    sample_count = BD_LINE_SAMPLE_COUNT;
    for (i = 0; i < sample_count; ++i) sample[i] = (i * (count - 1) + (sample_count - 1) / 2) / (sample_count - 1);
    return sample_count;
}

static LineFit fit_line_fast(const float *independent, const float *dependent, int count,
                             float slope_min, float slope_max, float residual_threshold,
                             int min_inliers, float min_span, float border_limit, int prefer)
{
    LineFit result;
    uint8_t best_inliers[BD_MAX_LINE_POINTS] = {0}, inliers[BD_MAX_LINE_POINTS] = {0};
    float residuals[BD_MAX_LINE_POINTS], subset[BD_MAX_LINE_POINTS];
    int sample[BD_LINE_SAMPLE_COUNT], sample_count, ai, aj, i, have_best = 0;
    float best_score = -1.0e30f, slope = 0.0f, intercept = 0.0f;
    memset(&result, 0, sizeof(result));
    if (count < min_inliers) return result;
    sample_count = build_sample_indices(count, sample);
    for (ai = 0; ai < sample_count - 1; ++ai) for (aj = ai + 1; aj < sample_count; ++aj) {
        int p0 = sample[ai], p1 = sample[aj], n = 0;
        float delta = independent[p1] - independent[p0], test_slope, test_intercept;
        float min_value = 1.0e30f, max_value = -1.0e30f, sum_dep = 0.0f, sum_res = 0.0f, score;
        if (absf_fast(delta) < 3.0f) continue;
        test_slope = (dependent[p1] - dependent[p0]) / delta;
        if (test_slope < slope_min || test_slope > slope_max) continue;
        test_intercept = dependent[p0] - test_slope * independent[p0];
        for (i = 0; i < count; ++i) {
            float residual = absf_fast(dependent[i] - (test_slope * independent[i] + test_intercept));
            inliers[i] = (uint8_t)(residual <= residual_threshold);
            if (inliers[i]) { ++n; if (independent[i] < min_value) min_value = independent[i]; if (independent[i] > max_value) max_value = independent[i]; sum_dep += dependent[i]; sum_res += residual; }
        }
        if (n < min_inliers || max_value - min_value < min_span) continue;
        score = n * 12.0f + (max_value - min_value) * 2.0f - (sum_res / n) * 6.0f;
        if (prefer == PREF_TOP) score -= (sum_dep / n) * 0.7f;
        else if (prefer == PREF_BOTTOM) score += (sum_dep / n) * 0.7f;
        else if (prefer == PREF_LEFT) score -= (sum_dep / n) * 0.25f;
        else score += (sum_dep / n) * 0.25f;
        if (score > best_score) { best_score = score; memcpy(best_inliers, inliers, (size_t)count); have_best = 1; }
    }
    if (!have_best) return result;
    linear_fit(independent, dependent, best_inliers, count, &slope, &intercept);
    {
        int subset_count = 0, n = 0; float cutoff;
        for (i = 0; i < count; ++i) { residuals[i] = absf_fast(dependent[i] - (slope * independent[i] + intercept)); if (best_inliers[i]) subset[subset_count++] = residuals[i]; }
        cutoff = percentile_float(subset, subset_count, 80) * 1.3f; if (cutoff < residual_threshold) cutoff = residual_threshold;
        for (i = 0; i < count; ++i) { inliers[i] = (uint8_t)(residuals[i] <= cutoff); n += inliers[i]; }
        if (n < min_inliers) memcpy(inliers, best_inliers, (size_t)count);
    }
    linear_fit(independent, dependent, inliers, count, &slope, &intercept);
    {
        uint8_t refined[BD_MAX_LINE_POINTS]; int subset_count = 0, n = 0; float cutoff;
        for (i = 0; i < count; ++i) { residuals[i] = absf_fast(dependent[i] - (slope * independent[i] + intercept)); if (inliers[i]) subset[subset_count++] = residuals[i]; }
        cutoff = percentile_float(subset, subset_count, 80) * 1.2f; if (cutoff < residual_threshold) cutoff = residual_threshold;
        for (i = 0; i < count; ++i) { refined[i] = (uint8_t)(residuals[i] <= cutoff); n += refined[i]; }
        if (n < min_inliers) memcpy(inliers, best_inliers, (size_t)count); else memcpy(inliers, refined, (size_t)count);
    }
    {
        float support_min = 1.0e30f, support_max = -1.0e30f, sum_dep = 0.0f, sum_res = 0.0f;
        int n = 0, border_count = 0;
        for (i = 0; i < count; ++i) if (inliers[i]) {
            ++n; if (independent[i] < support_min) support_min = independent[i]; if (independent[i] > support_max) support_max = independent[i];
            sum_dep += dependent[i]; sum_res += residuals[i];
            if ((prefer == PREF_LEFT || prefer == PREF_TOP) ? dependent[i] <= border_limit : dependent[i] >= border_limit) ++border_count;
        }
        result.valid = n >= min_inliers; result.slope = slope; result.intercept = intercept;
        result.support_min = support_min; result.support_max = support_max; result.inlier_count = n; result.span = support_max - support_min;
        result.residual = n ? sum_res / n : 0.0f; result.border_touch_ratio = n ? (float)border_count / n : 0.0f; result.mean_value = n ? sum_dep / n : 0.0f;
    }
    return result;
}

static LineFit fit_one_side(const BridgeDetectionBitmap *mask, int width, int height, int side_right)
{
    int left[BRIDGE_DETECTION_MAX_HEIGHT], right[BRIDGE_DETECTION_MAX_HEIGHT], widths[BRIDGE_DETECTION_MAX_HEIGHT];
    int rows[BRIDGE_DETECTION_MAX_HEIGHT], unclipped[BRIDGE_DETECTION_MAX_HEIGHT];
    float independent[BRIDGE_DETECTION_MAX_HEIGHT], dependent[BRIDGE_DETECTION_MAX_HEIGHT];
    int row_count = 0, unclipped_count = 0, y, count, use_unclipped;
    LineFit none;
    extract_row_borders(mask, width, height, left, right, widths);
    for (y = 0; y < height; ++y) if (widths[y] > 0) {
        rows[row_count++] = y;
        if ((!side_right && left[y] > 1) || (side_right && right[y] < width - 2)) unclipped[unclipped_count++] = y;
    }
    if (row_count < 8) { memset(&none, 0, sizeof(none)); return none; }
    use_unclipped = unclipped_count >= 6 && unclipped[unclipped_count - 1] - unclipped[0] >= 10;
    count = use_unclipped ? unclipped_count : row_count;
    for (y = 0; y < count; ++y) { int row = use_unclipped ? unclipped[y] : rows[y]; independent[y] = (float)row; dependent[y] = (float)(side_right ? right[row] : left[row]); }
    return fit_line_fast(independent, dependent, count, side_right ? 0.15f : -2.5f, side_right ? 2.5f : 0.25f,
                         1.35f, 6, 10.0f, side_right ? width - 2.5f : 1.5f, side_right ? PREF_RIGHT : PREF_LEFT);
}

static float score_side_fit(LineFit line)
{
    if (!line.valid) return -1.0e9f;
    return line.inlier_count * 5.0f + line.span * 1.8f - line.residual * 10.0f - line.border_touch_ratio * 30.0f +
           absf_fast(line.slope) * 10.0f + absf_fast(line.slope) * line.span * 3.0f;
}

static LineFit fit_horizontal(const BridgeDetectionBitmap *mask, int width, int height, int bottom)
{
    float independent[BRIDGE_DETECTION_MAX_WIDTH], dependent[BRIDGE_DETECTION_MAX_WIDTH];
    int x, count = 0;
    for (x = 0; x < width; ++x) {
        int y, found = -1;
        if (!bottom) { for (y = 0; y < height; ++y) if (bitmap_get(mask, x, y)) { found = y; break; } }
        else { for (y = height - 1; y >= 0; --y) if (bitmap_get(mask, x, y)) { found = y; break; } }
        if (found >= 0) { independent[count] = (float)x; dependent[count] = (float)found; ++count; }
    }
    return fit_line_fast(independent, dependent, count, -0.32f, 0.32f, 1.2f, 6, 8.0f,
                         bottom ? height - 2.5f : 1.5f, bottom ? PREF_BOTTOM : PREF_TOP);
}

static LineFit fit_top_plateau(const BridgeDetectionBitmap *mask, int width, int height)
{
    int top[BRIDGE_DETECTION_MAX_WIDTH], cols[BRIDGE_DETECTION_MAX_WIDTH];
    int col_count = 0, min_top = height, x, y, tolerance;
    LineFit none;
    memset(&none, 0, sizeof(none));
    for (x = 0; x < width; ++x) {
        top[x] = -1;
        for (y = 0; y < height; ++y) if (bitmap_get(mask, x, y)) { top[x] = y; break; }
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
            float independent[BRIDGE_DETECTION_MAX_WIDTH], dependent[BRIDGE_DETECTION_MAX_WIDTH];
            int count = best_end - best_start + 1, j;
            for (j = 0; j < count; ++j) { int col = cols[best_start + j]; independent[j] = (float)col; dependent[j] = (float)top[col]; }
            {
                LineFit fit = fit_line_fast(independent, dependent, count, -0.32f, 0.32f, 0.9f, 4, 6.0f, 1.5f, PREF_TOP);
                if (fit.valid) return fit;
            }
        }
    }
    return none;
}

static int should_show_left(LineFit line, const Candidate *candidate)
{
    if (!line.valid || line.inlier_count < 6 || line.span < 10.0f) return 0;
    if (line.border_touch_ratio >= 0.75f && absf_fast(line.slope) <= 0.25f && line.mean_value <= 1.6f) return 0;
    if (candidate->left_clip_ratio >= 0.75f && absf_fast(line.slope) <= 0.12f && line.mean_value <= 6.0f) return 0;
    return 1;
}

static int should_show_right(LineFit line, int width, const Candidate *candidate)
{
    if (!line.valid || line.inlier_count < 6 || line.span < 10.0f) return 0;
    if (line.border_touch_ratio >= 0.75f && absf_fast(line.slope) <= 0.25f && line.mean_value >= width - 2.6f) return 0;
    if (candidate->max_width <= 36 && candidate->area_ratio <= 0.19f && line.mean_value >= width - 18.0f) return 0;
    return 1;
}

static int should_show_top(LineFit line, const Candidate *candidate)
{
    if (!line.valid || line.inlier_count < 6 || line.span < 8.0f) return 0;
    if (line.border_touch_ratio >= 0.75f && line.mean_value <= 1.6f) return 0;
    return candidate->top_row > 1;
}

static int should_show_entry(LineFit line, int height, const Candidate *candidate)
{
    int min_bottom_width, narrow_width;
    if (!line.valid || line.inlier_count < 6 || line.span < 8.0f) return 0;
    if (line.border_touch_ratio >= 0.55f && line.mean_value >= height - 2.6f) return 0;
    narrow_width = (candidate->max_width * 14 + 50) / 100; if (narrow_width < 8) narrow_width = 8;
    if (line.slope <= -0.08f && line.border_touch_ratio <= 0.2f &&
        (candidate->top_row <= 5 || candidate->max_width <= 52 || candidate->top_row >= 10 ||
         (candidate->top_row <= 8 && candidate->bottom_width <= narrow_width))) return 1;
    min_bottom_width = (candidate->max_width * 28 + 50) / 100; if (min_bottom_width < 16) min_bottom_width = 16;
    return candidate->bottom_width >= min_bottom_width && line.slope <= -0.02f && line.border_touch_ratio <= 0.4f;
}

static void export_side_line(LineFit line, BridgeDetectionSideLine *out)
{
    memset(out, 0, sizeof(*out));
    if (!line.valid) return;
    out->valid = 1; out->slope = line.slope; out->intercept = line.intercept; out->support_min_y = line.support_min;
    out->support_max_y = line.support_max; out->inlier_count = line.inlier_count; out->span = line.span;
    out->residual = line.residual; out->border_touch_ratio = line.border_touch_ratio; out->mean_x = line.mean_value;
}

static BridgeDetectionSegment side_segment(LineFit line, int visible, int side_right,
                                           int bottom_row, int width, int height)
{
    BridgeDetectionSegment segment;
    float y0, y1, x0, x1;
    memset(&segment, 0, sizeof(segment));
    if (!visible || !line.valid) return segment;
    y0 = line.support_min; y1 = line.support_max;
    if (!side_right && bottom_row - y1 >= 8.0f && line.slope * bottom_row + line.intercept > 2.5f) y1 = (float)bottom_row;
    x0 = line.slope * y0 + line.intercept; x1 = line.slope * y1 + line.intercept;
    segment.valid = 1; segment.x0 = clamp_int(round_positive(x0), 0, width - 1); segment.y0 = clamp_int(round_positive(y0), 0, height - 1);
    segment.x1 = clamp_int(round_positive(x1), 0, width - 1); segment.y1 = clamp_int(round_positive(y1), 0, height - 1);
    return segment;
}

static BridgeDetectionSegment center_segment(BridgeDetectionSegment left, BridgeDetectionSegment right,
                                             int width, int height)
{
    BridgeDetectionSegment out;
    float ls, li, rs, ri, ms, mi, top_y, bottom_y;
    memset(&out, 0, sizeof(out));
    if (!left.valid || !right.valid || left.y1 == left.y0 || right.y1 == right.y0) return out;
    ls = (float)(left.x1 - left.x0) / (left.y1 - left.y0); li = left.x0 - ls * left.y0;
    rs = (float)(right.x1 - right.x0) / (right.y1 - right.y0); ri = right.x0 - rs * right.y0;
    ms = (ls + rs) * 0.5f; mi = (li + ri) * 0.5f; top_y = (float)(left.y0 < right.y0 ? left.y0 : right.y0);
    bottom_y = (float)(left.y1 > right.y1 ? left.y1 : right.y1);
    out.valid = 1; out.x0 = clamp_int(round_positive(ms * top_y + mi), 0, width - 1); out.y0 = clamp_int(round_positive(top_y), 0, height - 1);
    out.x1 = clamp_int(round_positive(ms * bottom_y + mi), 0, width - 1); out.y1 = clamp_int(round_positive(bottom_y), 0, height - 1);
    if (out.x0 == out.x1 && out.y0 == out.y1) out.valid = 0;
    return out;
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
    result->threshold = -1; result->top_row = -1; result->start_row = -1; result->bottom_row = -1;
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

static int find_best_candidate(const uint8_t *gray, int width, int height, int stride,
                               const int *thresholds, int threshold_count,
                               BridgeDetectionScratch *scratch, Candidate *best_out)
{
    int t, y, word, words = words_for_width(width), have_best = 0;
    Candidate best;
    memset(&best, 0, sizeof(best));
    for (t = 0; t < threshold_count; ++t) {
        threshold_mask(gray, &scratch->work0, width, height, stride, thresholds[t]);
        close3_open2(&scratch->work0, &scratch->work1, width, height);
        for (y = 0; y < height; ++y) for (word = 0; word < words; ++word) {
            uint32_t bits;
            while ((bits = scratch->work0.row[y][word]) != 0u) {
                int x = (word << 5) + ctz32(bits);
                int component_area, start;
                Candidate candidate;
                if (x >= width) { scratch->work0.row[y][word] = 0; break; }
                start = y * width + x;
                component_area = extract_component(&scratch->work0, &scratch->work2, scratch->queue, start, width, height);
                if (component_area < BD_MIN_COMPONENT_AREA) continue;
                if (!evaluate_component(gray, stride, &scratch->work2, thresholds[t], width, height, scratch, &candidate)) continue;
                if (!have_best || candidate.score > best.score) {
                    best = candidate; have_best = 1;
                    bitmap_copy(&scratch->best_visible, &scratch->work4);
                    bitmap_copy(&scratch->best_outer, &scratch->work2);
                }
            }
        }
    }
    if (have_best) *best_out = best;
    return have_best;
}

static int temporal_candidate_is_stable(const Candidate *candidate,
                                        const BridgeDetectionResult *previous,
                                        const BridgeDetectionConfig *config)
{
    int area_limit;
    if (!previous->candidate_found || previous->state != BRIDGE_DETECTION_STATE_ON_BRIDGE) return 0;
    if (candidate->score < config->min_valid_score || candidate->edge_contrast < config->min_edge_contrast) return 0;
    if (absf_fast(candidate->center_x - previous->center_x) > 1.0f) return 0;
    area_limit = previous->area / 100 + 1;
    if (candidate->area > previous->area + area_limit || candidate->area + area_limit < previous->area) return 0;
    if (candidate->top_row > previous->top_row + 1 || candidate->top_row + 1 < previous->top_row ||
        candidate->bottom_row > previous->bottom_row + 1 || candidate->bottom_row + 1 < previous->bottom_row ||
        candidate->max_width > previous->max_width + 2 || candidate->max_width + 2 < previous->max_width ||
        candidate->bottom_width > previous->bottom_width + 2 || candidate->bottom_width + 2 < previous->bottom_width) return 0;
    if (candidate->left_clip_ratio > 0.55f || candidate->right_clip_ratio > 0.55f) return 0;
    return 1;
}

static int build_temporal_thresholds(const int *all_thresholds, int count, int previous_threshold, int *out)
{
    int lower = -1, upper = -1, i, out_count = 0;
    out[out_count++] = previous_threshold;
    for (i = 0; i < count; ++i) {
        if (all_thresholds[i] < previous_threshold) lower = all_thresholds[i];
        if (all_thresholds[i] > previous_threshold) { upper = all_thresholds[i]; break; }
    }
    if (lower >= 0) out[out_count++] = lower;
    if (upper >= 0) out[out_count++] = upper;
    return out_count;
}

int bridge_detection_detect_gray(const uint8_t *gray, int width, int height, int stride,
                                 const BridgeDetectionConfig *config_in, BridgeDetectionScratch *scratch,
                                 BridgeDetectionResult *result)
{
    BridgeDetectionConfig default_config;
    const BridgeDetectionConfig *config = config_in;
    Candidate best;
    int thresholds[11], threshold_count, have_best = 0;
    int temporal_used = 0;
    LineFit visible_left, visible_right, outer_left, outer_right, left, right, top, plateau, entry;
    if (result == NULL) return -1;
    bridge_detection_result_clear(result);
    memset(&best, 0, sizeof(best));
    if (gray == NULL || scratch == NULL || width <= 0 || height <= 0 || stride < width ||
        width > BRIDGE_DETECTION_MAX_WIDTH || height > BRIDGE_DETECTION_MAX_HEIGHT || width * height > 65535) return -2;
    if (scratch->cache_magic != BD_CACHE_MAGIC) {
        scratch->exact_cache_hits = 0;
        scratch->temporal_fast_hits = 0;
        scratch->full_detection_calls = 0;
        scratch->temporal_streak = 0;
    }
    if (config == NULL) { bridge_detection_default_config(&default_config); config = &default_config; }
    if (cache_matches(scratch, gray, width, height, stride, config)) {
        ++scratch->exact_cache_hits;
        *result = scratch->cached_result;
        return scratch->cache_status;
    }
    threshold_count = build_threshold_candidates(gray, width, height, stride, thresholds);
    if (scratch->cache_magic == BD_CACHE_MAGIC && scratch->cached_result.candidate_found &&
        scratch->temporal_streak < BD_TEMPORAL_MAX_STREAK &&
        temporal_input_is_small_change(scratch, gray, width, height, stride)) {
        int previous_threshold = scratch->cached_result.threshold;
        int temporal_thresholds[3];
        int temporal_threshold_count = build_temporal_thresholds(thresholds, threshold_count,
                                                                  previous_threshold, temporal_thresholds);
        Candidate temporal_candidate;
        if (find_best_candidate(gray, width, height, stride, temporal_thresholds, temporal_threshold_count, scratch, &temporal_candidate) &&
            temporal_candidate_is_stable(&temporal_candidate, &scratch->cached_result, config)) {
            best = temporal_candidate;
            have_best = 1;
            temporal_used = 1;
        }
    }
    if (!have_best) {
        ++scratch->full_detection_calls;
        have_best = find_best_candidate(gray, width, height, stride, thresholds, threshold_count, scratch, &best);
    }
    if (!have_best) {
        scratch->temporal_streak = 0;
        cache_store(scratch, gray, width, height, stride, config, result, 0);
        return 0;
    }
    result->candidate_found = 1; result->threshold = best.threshold; result->candidate_score = best.score;
    result->area = best.area; result->area_ratio = best.area_ratio; result->top_row = best.top_row; result->start_row = best.start_row;
    result->bottom_row = best.bottom_row; result->max_width = best.max_width; result->bottom_width = best.bottom_width;
    result->center_x = best.center_x; result->edge_contrast = best.edge_contrast; result->left_clip_ratio = best.left_clip_ratio;
    result->right_clip_ratio = best.right_clip_ratio; result->dual_clip_ratio = best.dual_clip_ratio; result->border_monotonic = best.border_monotonic;
    visible_left = fit_one_side(&scratch->best_visible, width, height, 0);
    visible_right = fit_one_side(&scratch->best_visible, width, height, 1);
    outer_left = fit_one_side(&scratch->best_outer, width, height, 0);
    outer_right = fit_one_side(&scratch->best_outer, width, height, 1);
    left = score_side_fit(visible_left) >= score_side_fit(outer_left) ? visible_left : outer_left;
    right = score_side_fit(visible_right) >= score_side_fit(outer_right) ? visible_right : outer_right;
    top = fit_horizontal(&scratch->best_outer, width, height, 0);
    plateau = fit_top_plateau(&scratch->best_outer, width, height);
    entry = fit_horizontal(&scratch->best_outer, width, height, 1);
    export_side_line(left, &result->left_line); export_side_line(right, &result->right_line);
    result->left_line_visible = (uint8_t)should_show_left(left, &best);
    result->right_line_visible = (uint8_t)should_show_right(right, width, &best);
    result->top_line_visible = (uint8_t)(((result->left_line_visible && result->right_line_visible && plateau.valid) || should_show_top(top, &best)) && best.top_row > 1);
    result->entry_line_visible = (uint8_t)should_show_entry(entry, height, &best);
    if (result->top_line_visible && result->entry_line_visible && best.top_row <= 5 && entry.slope <= -0.08f) result->top_line_visible = 0;
    if (result->top_line_visible && !result->entry_line_visible && best.top_row <= 4 && best.left_clip_ratio >= 0.4f && best.max_width >= 85 && best.bottom_width >= 70) result->top_line_visible = 0;
    if (best.center_x >= width * 0.62f && best.max_width <= 50 && best.bottom_width <= 6 &&
        best.left_clip_ratio < 0.1f && best.right_clip_ratio < 0.1f && best.top_row >= 10 && best.top_row <= 24 &&
        right.valid && absf_fast(right.slope) <= 0.08f) result->entry_line_visible = 0;
    if (best.score >= config->min_valid_score && best.edge_contrast >= config->min_edge_contrast) {
        float exit_clip = best.left_clip_ratio > best.right_clip_ratio ? best.left_clip_ratio : best.right_clip_ratio;
        result->bridge_found = 1;
        if (result->entry_line_visible || best.bottom_row <= height - 10) result->state = BRIDGE_DETECTION_STATE_PREPARE_ENTER;
        else if (exit_clip >= 0.82f || (exit_clip >= 0.68f && best.start_row >= 18)) result->state = BRIDGE_DETECTION_STATE_PREPARE_EXIT;
        else result->state = BRIDGE_DETECTION_STATE_ON_BRIDGE;
    }
    if (!result->bridge_found) {
        result->left_line_visible = 0; result->right_line_visible = 0; result->top_line_visible = 0; result->entry_line_visible = 0;
    }
    if (result->bridge_found && best.top_row >= 30 && best.center_x >= width * 0.70f && best.max_width <= 40 && best.bottom_width >= 28) result->right_line_visible = 0;
    if (result->bridge_found) {
        result->left_segment = side_segment(left, result->left_line_visible, 0, best.bottom_row, width, height);
        result->right_segment = side_segment(right, result->right_line_visible, 1, best.bottom_row, width, height);
        result->center_segment = center_segment(result->left_segment, result->right_segment, width, height);
        if (result->center_segment.valid) {
            int dy = result->center_segment.y1 - result->center_segment.y0;
            result->control_center_x = (float)result->center_segment.x1;
            result->lateral_error_px = result->control_center_x - (width - 1) * 0.5f;
            result->heading_dx_per_dy = dy ? (float)(result->center_segment.x1 - result->center_segment.x0) / dy : 0.0f;
        }
    }
    if (temporal_used) {
        ++scratch->temporal_fast_hits;
        ++scratch->temporal_streak;
    } else {
        scratch->temporal_streak = 0;
    }
    cache_store(scratch, gray, width, height, stride, config, result, 1);
    return 1;
}
