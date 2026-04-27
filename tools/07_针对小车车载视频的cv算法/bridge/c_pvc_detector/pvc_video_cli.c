#include "pvc_detector.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#endif

#ifndef PVC_PC_ENABLE_TIMING
#define PVC_PC_ENABLE_TIMING 1
#endif

#ifndef PVC_PC_WRITE_CSV
#define PVC_PC_WRITE_CSV 1
#endif

#define PVC_MAX_FRAMES 10000
#define PVC_PATH_MAX 1024

typedef struct {
    char name[256];
} FrameName;

typedef struct {
    char pgm_dir[PVC_PATH_MAX];
    char output_json[PVC_PATH_MAX];
    char output_csv[PVC_PATH_MAX];
    int max_frames;
    int debug_every;
} CliOptions;

typedef struct {
    int frame_index;
    char frame_name[256];
    PvcDetectResult detection;
    double elapsed_us;
} FrameResult;

static int str_ends_with(const char *text, const char *suffix)
{
    const size_t text_len = strlen(text);
    const size_t suffix_len = strlen(suffix);
    if (suffix_len > text_len) {
        return 0;
    }
    return strcmp(text + text_len - suffix_len, suffix) == 0;
}

static int compare_frame_name(const void *a, const void *b)
{
    const FrameName *fa = (const FrameName *)a;
    const FrameName *fb = (const FrameName *)b;
    return strcmp(fa->name, fb->name);
}

static void path_join(char *out, size_t out_size, const char *dir, const char *name)
{
    const size_t len = strlen(dir);
    const int need_sep = len > 0 && dir[len - 1] != '/' && dir[len - 1] != '\\';
    snprintf(out, out_size, "%s%s%s", dir, need_sep ? "/" : "", name);
}

static int list_pgm_frames(const char *dir, FrameName *frames, int max_frames)
{
    int count = 0;
#if defined(_WIN32)
    char pattern[PVC_PATH_MAX];
    WIN32_FIND_DATAA find_data;
    HANDLE handle;

    path_join(pattern, sizeof(pattern), dir, "*.pgm");
    handle = FindFirstFileA(pattern, &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            if (count < max_frames) {
                snprintf(frames[count].name, sizeof(frames[count].name), "%s", find_data.cFileName);
                count++;
            }
        }
    } while (FindNextFileA(handle, &find_data));
    FindClose(handle);
#else
    DIR *dp = opendir(dir);
    struct dirent *entry;
    if (dp == NULL) {
        return 0;
    }
    while ((entry = readdir(dp)) != NULL) {
        if (str_ends_with(entry->d_name, ".pgm") && count < max_frames) {
            snprintf(frames[count].name, sizeof(frames[count].name), "%s", entry->d_name);
            count++;
        }
    }
    closedir(dp);
#endif

    qsort(frames, (size_t)count, sizeof(FrameName), compare_frame_name);
    return count;
}

static int read_next_token(FILE *fp, char *out, size_t out_size)
{
    int c;
    size_t n = 0;

    do {
        c = fgetc(fp);
        if (c == '#') {
            while (c != '\n' && c != EOF) {
                c = fgetc(fp);
            }
        }
    } while (c != EOF && isspace(c));

    if (c == EOF) {
        return 0;
    }

    while (c != EOF && !isspace(c)) {
        if (n + 1 < out_size) {
            out[n++] = (char)c;
        }
        c = fgetc(fp);
    }
    out[n] = '\0';
    return 1;
}

static int read_pgm_p5(const char *path, uint8_t *gray, int *width, int *height)
{
    char token[64];
    int max_value;
    const int pixels_limit = PVC_MAX_PIXELS;
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return -1;
    }

    if (!read_next_token(fp, token, sizeof(token)) || strcmp(token, "P5") != 0) {
        fclose(fp);
        return -2;
    }
    if (!read_next_token(fp, token, sizeof(token))) {
        fclose(fp);
        return -3;
    }
    *width = atoi(token);
    if (!read_next_token(fp, token, sizeof(token))) {
        fclose(fp);
        return -4;
    }
    *height = atoi(token);
    if (!read_next_token(fp, token, sizeof(token))) {
        fclose(fp);
        return -5;
    }
    max_value = atoi(token);

    if (*width <= 0 || *height <= 0 || *width > PVC_MAX_WIDTH || *height > PVC_MAX_HEIGHT) {
        fclose(fp);
        return -6;
    }
    if (max_value != 255) {
        fclose(fp);
        return -7;
    }
    if ((*width) * (*height) > pixels_limit) {
        fclose(fp);
        return -8;
    }

    if (fread(gray, 1, (size_t)(*width * *height), fp) != (size_t)(*width * *height)) {
        fclose(fp);
        return -9;
    }

    fclose(fp);
    return 0;
}

static double now_us(void)
{
#if defined(_WIN32)
    static LARGE_INTEGER freq;
    LARGE_INTEGER counter;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000000.0 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
#endif
}

static void print_usage(const char *exe)
{
    printf("Usage: %s --pgm-dir DIR --output-json FILE [--output-csv FILE] [--max-frames N] [--debug-every N]\n", exe);
}

static void json_write_string(FILE *fp, const char *text)
{
    fputc('"', fp);
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        switch (*p) {
        case '\\':
            fputs("\\\\", fp);
            break;
        case '"':
            fputs("\\\"", fp);
            break;
        case '\b':
            fputs("\\b", fp);
            break;
        case '\f':
            fputs("\\f", fp);
            break;
        case '\n':
            fputs("\\n", fp);
            break;
        case '\r':
            fputs("\\r", fp);
            break;
        case '\t':
            fputs("\\t", fp);
            break;
        default:
            if (*p < 0x20) {
                fprintf(fp, "\\u%04x", *p);
            } else {
                fputc(*p, fp);
            }
            break;
        }
    }
    fputc('"', fp);
}

static void options_init(CliOptions *options)
{
    memset(options, 0, sizeof(*options));
    snprintf(options->output_json, sizeof(options->output_json), "pvc_c_summary.json");
    snprintf(options->output_csv, sizeof(options->output_csv), "pvc_c_summary.csv");
    options->max_frames = 0;
    options->debug_every = 0;
}

static int parse_args(int argc, char **argv, CliOptions *options)
{
    options_init(options);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--pgm-dir") == 0 && i + 1 < argc) {
            snprintf(options->pgm_dir, sizeof(options->pgm_dir), "%s", argv[++i]);
        } else if (strcmp(argv[i], "--output-json") == 0 && i + 1 < argc) {
            snprintf(options->output_json, sizeof(options->output_json), "%s", argv[++i]);
        } else if (strcmp(argv[i], "--output-csv") == 0 && i + 1 < argc) {
            snprintf(options->output_csv, sizeof(options->output_csv), "%s", argv[++i]);
        } else if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) {
            options->max_frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--debug-every") == 0 && i + 1 < argc) {
            options->debug_every = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            return 1;
        } else {
            fprintf(stderr, "unknown or incomplete argument: %s\n", argv[i]);
            return -1;
        }
    }

    if (options->pgm_dir[0] == '\0') {
        return -1;
    }
    return 0;
}

static void json_write_detection(FILE *fp, const FrameResult *frame)
{
    const PvcDetectResult *r = &frame->detection;
    fprintf(fp, "    {\n");
    fprintf(fp, "      \"frame\": %d,\n", frame->frame_index);
    fprintf(fp, "      \"frame_name\": ");
    json_write_string(fp, frame->frame_name);
    fprintf(fp, ",\n");
    fprintf(fp, "      \"detected\": %s,\n", r->detected ? "true" : "false");
    fprintf(fp, "      \"score\": %.4f,\n", r->confidence);
    fprintf(fp, "      \"area\": %d,\n", r->detected ? r->area : (r->area > 0 ? r->area : 0));
    if (r->bbox_xmin >= 0) {
        fprintf(fp, "      \"bbox\": [%d, %d, %d, %d],\n", r->bbox_xmin, r->bbox_ymin, r->bbox_xmax, r->bbox_ymax);
    } else {
        fprintf(fp, "      \"bbox\": null,\n");
    }
    if (r->entry_bottom_y >= 0) {
        fprintf(fp, "      \"entry_bottom_y\": %d,\n", r->entry_bottom_y);
        fprintf(fp, "      \"entry_top_y\": %d,\n", r->entry_top_y);
    } else {
        fprintf(fp, "      \"entry_bottom_y\": null,\n");
        fprintf(fp, "      \"entry_top_y\": null,\n");
    }
    fprintf(fp, "      \"component_count\": %d,\n", r->component_count);
    fprintf(fp, "      \"candidate_count\": %d,\n", r->candidate_count);
    fprintf(fp, "      \"centroid\": [%.2f, %.2f],\n", r->centroid_x, r->centroid_y);
    fprintf(fp, "      \"forward_mm\": %.1f,\n", r->forward_mm);
    fprintf(fp, "      \"lateral_mm\": %.1f,\n", r->lateral_mm);
    fprintf(fp, "      \"elapsed_us\": %.2f\n", frame->elapsed_us);
    fprintf(fp, "    }");
}

static int write_json(
    const char *path,
    const CliOptions *options,
    const FrameResult *results,
    int frame_count,
    int detected_count,
    int first_detected,
    int last_detected,
    double total_us,
    double min_us,
    double max_us)
{
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"summary\": {\n");
    fprintf(fp, "    \"frame_dir\": ");
    json_write_string(fp, options->pgm_dir);
    fprintf(fp, ",\n");
    fprintf(fp, "    \"frame_count\": %d,\n", frame_count);
    fprintf(fp, "    \"detected_count\": %d,\n", detected_count);
    if (first_detected > 0) {
        fprintf(fp, "    \"first_detected_frame\": %d,\n", first_detected);
        fprintf(fp, "    \"last_detected_frame\": %d,\n", last_detected);
    } else {
        fprintf(fp, "    \"first_detected_frame\": null,\n");
        fprintf(fp, "    \"last_detected_frame\": null,\n");
    }
    fprintf(fp, "    \"threshold\": %d,\n", PVC_WHITE_THRESHOLD);
    fprintf(fp, "    \"decision_score\": %.2f,\n", PVC_MIN_DECISION_SCORE);
    fprintf(fp, "    \"timing\": {\n");
    fprintf(fp, "      \"total_us\": %.2f,\n", total_us);
    fprintf(fp, "      \"avg_us\": %.2f,\n", frame_count > 0 ? total_us / (double)frame_count : 0.0);
    fprintf(fp, "      \"min_us\": %.2f,\n", min_us);
    fprintf(fp, "      \"max_us\": %.2f\n", max_us);
    fprintf(fp, "    }\n");
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"output_contract\": {\n");
    fprintf(fp, "    \"detected\": \"PVC candidate score reaches decision threshold\",\n");
    fprintf(fp, "    \"confidence\": \"same value as score, 0..1\",\n");
    fprintf(fp, "    \"entry_bottom_y\": \"bottom row of selected PVC component; use row-distance table\",\n");
    fprintf(fp, "    \"forward_mm\": \"debug placeholder from row; replace with calibrated table\",\n");
    fprintf(fp, "    \"lateral_mm\": \"debug placeholder from component centroid; replace with calibrated table\"\n");
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"timeline\": [\n");
    for (int i = 0; i < frame_count; i++) {
        json_write_detection(fp, &results[i]);
        fprintf(fp, "%s\n", (i + 1 < frame_count) ? "," : "");
    }
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    fclose(fp);
    return 0;
}

static int write_csv(const char *path, const FrameResult *results, int frame_count)
{
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }
    fprintf(fp, "frame,frame_name,detected,score,area,bbox_xmin,bbox_ymin,bbox_xmax,bbox_ymax,entry_bottom_y,component_count,candidate_count,forward_mm,lateral_mm,elapsed_us\n");
    for (int i = 0; i < frame_count; i++) {
        const PvcDetectResult *r = &results[i].detection;
        fprintf(
            fp,
            "%d,%s,%d,%.4f,%d,%d,%d,%d,%d,%d,%d,%d,%.1f,%.1f,%.2f\n",
            results[i].frame_index,
            results[i].frame_name,
            r->detected,
            r->confidence,
            r->area,
            r->bbox_xmin,
            r->bbox_ymin,
            r->bbox_xmax,
            r->bbox_ymax,
            r->entry_bottom_y,
            r->component_count,
            r->candidate_count,
            r->forward_mm,
            r->lateral_mm,
            results[i].elapsed_us);
    }
    fclose(fp);
    return 0;
}

int main(int argc, char **argv)
{
    CliOptions options;
    FrameName *frames = NULL;
    FrameResult *results = NULL;
    PvcDetectScratch scratch;
    uint8_t gray[PVC_MAX_PIXELS];
    int frame_count;
    int detected_count = 0;
    int first_detected = 0;
    int last_detected = 0;
    double total_us = 0.0;
    double min_us = 0.0;
    double max_us = 0.0;
    int parse_ret = parse_args(argc, argv, &options);

    if (parse_ret != 0) {
        print_usage(argv[0]);
        return parse_ret > 0 ? 0 : 2;
    }

    frames = (FrameName *)calloc(PVC_MAX_FRAMES, sizeof(FrameName));
    if (frames == NULL) {
        fprintf(stderr, "failed to allocate frame list\n");
        return 4;
    }

    frame_count = list_pgm_frames(options.pgm_dir, frames, PVC_MAX_FRAMES);
    if (frame_count <= 0) {
        fprintf(stderr, "no .pgm frames found in %s\n", options.pgm_dir);
        free(frames);
        return 3;
    }
    if (options.max_frames > 0 && options.max_frames < frame_count) {
        frame_count = options.max_frames;
    }

    results = (FrameResult *)calloc((size_t)frame_count, sizeof(FrameResult));
    if (results == NULL) {
        fprintf(stderr, "failed to allocate results\n");
        free(frames);
        return 4;
    }

    for (int i = 0; i < frame_count; i++) {
        char path[PVC_PATH_MAX];
        int width = 0;
        int height = 0;
        int ret;
        double t0 = 0.0;
        double t1 = 0.0;

        path_join(path, sizeof(path), options.pgm_dir, frames[i].name);
        ret = read_pgm_p5(path, gray, &width, &height);
        if (ret != 0) {
            fprintf(stderr, "failed to read PGM %s, err=%d\n", path, ret);
            free(frames);
            free(results);
            return 5;
        }

#if PVC_PC_ENABLE_TIMING
        t0 = now_us();
#endif
        ret = pvc_detect_frame_gray(gray, width, height, &scratch, &results[i].detection);
#if PVC_PC_ENABLE_TIMING
        t1 = now_us();
#endif
        if (ret != 0) {
            fprintf(stderr, "detect failed for %s, err=%d\n", path, ret);
            free(frames);
            free(results);
            return 6;
        }

        results[i].frame_index = i + 1;
        snprintf(results[i].frame_name, sizeof(results[i].frame_name), "%s", frames[i].name);
        results[i].elapsed_us = t1 - t0;

        if (i == 0 || results[i].elapsed_us < min_us) {
            min_us = results[i].elapsed_us;
        }
        if (i == 0 || results[i].elapsed_us > max_us) {
            max_us = results[i].elapsed_us;
        }
        total_us += results[i].elapsed_us;

        if (results[i].detection.detected) {
            detected_count++;
            if (first_detected == 0) {
                first_detected = i + 1;
            }
            last_detected = i + 1;
        }

        if (options.debug_every > 0 && ((i + 1) % options.debug_every == 0)) {
            printf(
                "frame=%d detected=%d score=%.4f area=%d bottom_y=%d elapsed=%.2fus\n",
                i + 1,
                results[i].detection.detected,
                results[i].detection.confidence,
                results[i].detection.area,
                results[i].detection.entry_bottom_y,
                results[i].elapsed_us);
        }
    }

    if (write_json(
            options.output_json,
            &options,
            results,
            frame_count,
            detected_count,
            first_detected,
            last_detected,
            total_us,
            min_us,
            max_us) != 0) {
        fprintf(stderr, "failed to write %s: %s\n", options.output_json, strerror(errno));
        free(frames);
        free(results);
        return 7;
    }

#if PVC_PC_WRITE_CSV
    if (write_csv(options.output_csv, results, frame_count) != 0) {
        fprintf(stderr, "failed to write %s: %s\n", options.output_csv, strerror(errno));
        free(frames);
        free(results);
        return 8;
    }
#endif

    printf("frames: %d\n", frame_count);
    printf("detected: %d\n", detected_count);
    printf("first_detected: %d\n", first_detected);
    printf("last_detected: %d\n", last_detected);
    printf("timing_us: avg=%.2f min=%.2f max=%.2f\n", frame_count > 0 ? total_us / (double)frame_count : 0.0, min_us, max_us);
    printf("output_json: %s\n", options.output_json);

    free(frames);
    free(results);
    return 0;
}
