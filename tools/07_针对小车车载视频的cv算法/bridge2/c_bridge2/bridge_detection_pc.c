#define _CRT_SECURE_NO_WARNINGS

#include "bridge_detection.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/time.h>
#endif

#define B2_PC_MAX_FRAMES 10000
#define B2_PC_PATH_MAX 1024

typedef struct { char name[260]; } FrameName;
typedef struct {
    char name[260];
    BridgeDetectionResult result;
    double elapsed_us;
} FrameResult;

typedef struct {
    const char *pgm_dir;
    const char *output_csv;
    const char *timing_json;
    int max_frames;
    int debug_every;
} Options;

static int ends_with(const char *text, const char *suffix)
{
    size_t a = strlen(text), b = strlen(suffix);
    return a >= b && strcmp(text + a - b, suffix) == 0;
}

static int compare_name(const void *a, const void *b)
{
    return strcmp(((const FrameName *)a)->name, ((const FrameName *)b)->name);
}

static int compare_double_pc(const void *a, const void *b)
{
    double da = *(const double *)a, db = *(const double *)b;
    return da < db ? -1 : da > db ? 1 : 0;
}

static void path_join(char *out, size_t size, const char *dir, const char *name)
{
    size_t len = strlen(dir);
    snprintf(out, size, "%s%s%s", dir, len && (dir[len - 1] == '/' || dir[len - 1] == '\\') ? "" : "\\", name);
}

static int list_frames(const char *dir, FrameName *frames, int limit)
{
    int count = 0;
#ifdef _WIN32
    char pattern[B2_PC_PATH_MAX];
    struct _finddata_t item;
    intptr_t handle;
    path_join(pattern, sizeof(pattern), dir, "*.pgm");
    handle = _findfirst(pattern, &item);
    if (handle == -1) return 0;
    do {
        if (!(item.attrib & _A_SUBDIR) && ends_with(item.name, ".pgm") && count < limit) {
            strncpy(frames[count].name, item.name, sizeof(frames[count].name) - 1);
            frames[count].name[sizeof(frames[count].name) - 1] = '\0';
            ++count;
        }
    } while (_findnext(handle, &item) == 0);
    _findclose(handle);
#else
    DIR *dp = opendir(dir);
    struct dirent *item;
    if (!dp) return 0;
    while ((item = readdir(dp)) != NULL && count < limit) {
        if (ends_with(item->d_name, ".pgm")) {
            strncpy(frames[count].name, item->d_name, sizeof(frames[count].name) - 1);
            frames[count].name[sizeof(frames[count].name) - 1] = '\0';
            ++count;
        }
    }
    closedir(dp);
#endif
    qsort(frames, (size_t)count, sizeof(frames[0]), compare_name);
    return count;
}

static int next_token(FILE *fp, char *out, size_t size)
{
    int c;
    size_t n = 0;
    do {
        c = fgetc(fp);
        if (c == '#') while (c != '\n' && c != EOF) c = fgetc(fp);
    } while (c != EOF && isspace((unsigned char)c));
    if (c == EOF) return 0;
    do {
        if (n + 1 < size) out[n++] = (char)c;
        c = fgetc(fp);
    } while (c != EOF && !isspace((unsigned char)c));
    out[n] = '\0';
    return n > 0;
}

static int read_pgm(const char *path, uint8_t *gray, int *width, int *height)
{
    FILE *fp = fopen(path, "rb");
    char token[64];
    int max_value;
    size_t pixels;
    if (!fp) return 0;
    if (!next_token(fp, token, sizeof(token)) || strcmp(token, "P5") != 0 ||
        !next_token(fp, token, sizeof(token))) { fclose(fp); return 0; }
    *width = atoi(token);
    if (!next_token(fp, token, sizeof(token))) { fclose(fp); return 0; }
    *height = atoi(token);
    if (!next_token(fp, token, sizeof(token))) { fclose(fp); return 0; }
    max_value = atoi(token);
    if (*width <= 0 || *height <= 0 || *width > BRIDGE_DETECTION_MAX_WIDTH ||
        *height > BRIDGE_DETECTION_MAX_HEIGHT || max_value != 255) { fclose(fp); return 0; }
    pixels = (size_t)(*width * *height);
    if (fread(gray, 1, pixels, fp) != pixels) { fclose(fp); return 0; }
    fclose(fp);
    return 1;
}

static double now_us(void)
{
#ifdef _WIN32
    static LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    if (frequency.QuadPart == 0) QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000000.0 / (double)frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
#endif
}

static void stem_to_png(const char *pgm, char *out, size_t size)
{
    size_t len = strlen(pgm);
    if (len >= 4 && strcmp(pgm + len - 4, ".pgm") == 0) len -= 4;
    snprintf(out, size, "%.*s.png", (int)len, pgm);
}

static void write_segment(FILE *fp, BridgeDetectionSegment segment)
{
    if (segment.valid) fprintf(fp, "%d;%d;%d;%d", segment.x0, segment.y0, segment.x1, segment.y1);
}

static int write_csv(const char *path, const FrameResult *rows, int count)
{
    FILE *fp = fopen(path, "wb");
    int i;
    if (!fp) return 0;
    fprintf(fp, "frame,threshold,bridge_found,bridge_state_code,bridge_state,bridge_area,bridge_area_ratio,bridge_top_row,bridge_start_row,bridge_bottom_row,bridge_max_width,bridge_bottom_width,bridge_center_x,edge_contrast,left_clip_ratio,right_clip_ratio,dual_clip_ratio,border_monotonic,candidate_score,left_line_visible,right_line_visible,top_line_visible,entry_line_visible,left_line_segment,right_line_segment,center_line_segment,control_center_x,lateral_error_px,heading_dx_per_dy,elapsed_us\n");
    for (i = 0; i < count; ++i) {
        const BridgeDetectionResult *r = &rows[i].result;
        char png[280];
        stem_to_png(rows[i].name, png, sizeof(png));
        fprintf(fp, "%s,%d,%s,%d,%s,%d,%.6f,%d,%d,%d,%d,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%s,%s,%s,%s,",
            png, r->threshold, r->bridge_found ? "True" : "False", (int)r->state, bridge_detection_state_name(r->state),
            r->area, r->area_ratio, r->top_row, r->start_row, r->bottom_row, r->max_width, r->bottom_width,
            r->center_x, r->edge_contrast, r->left_clip_ratio, r->right_clip_ratio, r->dual_clip_ratio,
            r->border_monotonic, r->candidate_score, r->left_line_visible ? "True" : "False",
            r->right_line_visible ? "True" : "False", r->top_line_visible ? "True" : "False",
            r->entry_line_visible ? "True" : "False");
        write_segment(fp, r->left_segment); fputc(',', fp);
        write_segment(fp, r->right_segment); fputc(',', fp);
        write_segment(fp, r->center_segment);
        fprintf(fp, ",%.6f,%.6f,%.6f,%.3f\n", r->control_center_x, r->lateral_error_px, r->heading_dx_per_dy, rows[i].elapsed_us);
    }
    fclose(fp);
    return 1;
}

static double percentile_sorted(const double *values, int count, double q)
{
    double pos = (count - 1) * q;
    int lo = (int)floor(pos), hi = (int)ceil(pos);
    return values[lo] + (values[hi] - values[lo]) * (pos - lo);
}

static int write_timing_json(const char *path, const FrameResult *rows, int count)
{
    FILE *fp;
    double *values, sum = 0.0, sum_sq = 0.0, mean, variance;
    int i;
    if (count <= 0) return 0;
    values = (double *)malloc((size_t)count * sizeof(double));
    if (!values) return 0;
    for (i = 0; i < count; ++i) { values[i] = rows[i].elapsed_us; sum += values[i]; sum_sq += values[i] * values[i]; }
    qsort(values, (size_t)count, sizeof(values[0]), compare_double_pc);
    mean = sum / count; variance = sum_sq / count - mean * mean; if (variance < 0.0) variance = 0.0;
    fp = fopen(path, "wb");
    if (!fp) { free(values); return 0; }
    fprintf(fp,
        "{\n  \"frame_count\": %d,\n  \"total_ms\": %.6f,\n  \"mean_us\": %.6f,\n  \"min_us\": %.6f,\n  \"max_us\": %.6f,\n  \"stddev_us\": %.6f,\n  \"p50_us\": %.6f,\n  \"p95_us\": %.6f,\n  \"p99_us\": %.6f\n}\n",
        count, sum / 1000.0, mean, values[0], values[count - 1], sqrt(variance),
        percentile_sorted(values, count, 0.50), percentile_sorted(values, count, 0.95), percentile_sorted(values, count, 0.99));
    fclose(fp);
    printf("timing detector-only: mean %.3f us, min %.3f us, max %.3f us, p95 %.3f us, total %.3f ms\n",
           mean, values[0], values[count - 1], percentile_sorted(values, count, 0.95), sum / 1000.0);
    free(values);
    return 1;
}

static void usage(const char *exe)
{
    fprintf(stderr, "usage: %s --pgm-dir DIR --output-csv FILE --timing-json FILE [--max-frames N] [--debug-every N]\n", exe);
}

static int parse_options(int argc, char **argv, Options *o)
{
    int i;
    memset(o, 0, sizeof(*o)); o->max_frames = B2_PC_MAX_FRAMES; o->debug_every = 100;
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--pgm-dir") == 0 && i + 1 < argc) o->pgm_dir = argv[++i];
        else if (strcmp(argv[i], "--output-csv") == 0 && i + 1 < argc) o->output_csv = argv[++i];
        else if (strcmp(argv[i], "--timing-json") == 0 && i + 1 < argc) o->timing_json = argv[++i];
        else if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) o->max_frames = atoi(argv[++i]);
        else if (strcmp(argv[i], "--debug-every") == 0 && i + 1 < argc) o->debug_every = atoi(argv[++i]);
        else return 0;
    }
    if (o->max_frames <= 0 || o->max_frames > B2_PC_MAX_FRAMES) o->max_frames = B2_PC_MAX_FRAMES;
    return o->pgm_dir && o->output_csv && o->timing_json;
}

int main(int argc, char **argv)
{
    Options options;
    FrameName *names;
    FrameResult *rows;
    BridgeDetectionScratch *scratch;
    BridgeDetectionConfig config;
    uint8_t gray[BRIDGE_DETECTION_MAX_PIXELS];
    int frame_count, processed = 0, i;
    if (!parse_options(argc, argv, &options)) { usage(argv[0]); return 2; }
    names = (FrameName *)calloc((size_t)options.max_frames, sizeof(*names));
    rows = (FrameResult *)calloc((size_t)options.max_frames, sizeof(*rows));
    scratch = (BridgeDetectionScratch *)calloc(1, sizeof(*scratch));
    if (!names || !rows || !scratch) { fprintf(stderr, "allocation failed\n"); return 3; }
    frame_count = list_frames(options.pgm_dir, names, options.max_frames);
    if (!frame_count) { fprintf(stderr, "no PGM frames in %s\n", options.pgm_dir); return 4; }
    bridge_detection_default_config(&config);
    printf("core memory: scratch=%zu bytes, result=%zu bytes\n", sizeof(*scratch), sizeof(BridgeDetectionResult));
    for (i = 0; i < frame_count; ++i) {
        char path[B2_PC_PATH_MAX];
        int width, height, status;
        double begin, end;
        path_join(path, sizeof(path), options.pgm_dir, names[i].name);
        if (!read_pgm(path, gray, &width, &height)) { fprintf(stderr, "bad PGM: %s\n", path); continue; }
        begin = now_us();
        status = bridge_detection_detect_gray(gray, width, height, width, &config, scratch, &rows[processed].result);
        end = now_us();
        if (status < 0) { fprintf(stderr, "detector error %d: %s\n", status, path); continue; }
        strncpy(rows[processed].name, names[i].name, sizeof(rows[processed].name) - 1);
        rows[processed].elapsed_us = end - begin;
        ++processed;
        if (options.debug_every > 0 && (processed == 1 || processed % options.debug_every == 0)) {
            const BridgeDetectionResult *r = &rows[processed - 1].result;
            printf("%d/%d %s found=%d state=%s threshold=%d score=%.3f\n", processed, frame_count,
                   names[i].name, r->bridge_found, bridge_detection_state_name(r->state), r->threshold, r->candidate_score);
        }
    }
    if (!write_csv(options.output_csv, rows, processed)) { fprintf(stderr, "cannot write %s\n", options.output_csv); return 5; }
    if (!write_timing_json(options.timing_json, rows, processed)) { fprintf(stderr, "cannot write %s\n", options.timing_json); return 6; }
    printf("temporal stats: exact_cache=%lu temporal_fast=%lu full=%lu\n",
           (unsigned long)scratch->exact_cache_hits,
           (unsigned long)scratch->temporal_fast_hits,
           (unsigned long)scratch->full_detection_calls);
    printf("frames processed: %d\noutput: %s\n", processed, options.output_csv);
    free(scratch); free(rows); free(names);
    return 0;
}
