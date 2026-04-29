#include "bumpy_detector.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <time.h>
#endif

#ifndef BUMPY_PC_ENABLE_TIMING
#define BUMPY_PC_ENABLE_TIMING 1
#endif

#ifndef BUMPY_PC_WRITE_CSV
#define BUMPY_PC_WRITE_CSV 1
#endif

#define BUMPY_MAX_FRAMES 10000
#define BUMPY_PATH_MAX 1024

typedef struct {
    char name[256];
} FrameName;

typedef struct {
    char pgm_dir[BUMPY_PATH_MAX];
    char output_json[BUMPY_PATH_MAX];
    char output_csv[BUMPY_PATH_MAX];
    int max_frames;
    int debug_every;
} CliOptions;

typedef struct {
    int frame_index;
    char frame_name[256];
    BumpyDetectResult detection;
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
    char pattern[BUMPY_PATH_MAX];
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
    if (*width <= 0 || *height <= 0 || *width > BUMPY_MAX_WIDTH || *height > BUMPY_MAX_HEIGHT) {
        fclose(fp);
        return -6;
    }
    if (max_value != 255) {
        fclose(fp);
        return -7;
    }
    if (fread(gray, 1, (size_t)(*width * *height), fp) != (size_t)(*width * *height)) {
        fclose(fp);
        return -8;
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

static void options_init(CliOptions *options)
{
    memset(options, 0, sizeof(*options));
    snprintf(options->output_json, sizeof(options->output_json), "bumpy_c_summary.json");
    snprintf(options->output_csv, sizeof(options->output_csv), "bumpy_c_summary.csv");
}

static void print_usage(const char *exe)
{
    printf("Usage: %s --pgm-dir DIR --output-json FILE [--output-csv FILE] [--max-frames N] [--debug-every N]\n", exe);
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

static void json_write_string(FILE *fp, const char *text)
{
    fputc('"', fp);
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        switch (*p) {
        case '\\': fputs("\\\\", fp); break;
        case '"': fputs("\\\"", fp); break;
        case '\b': fputs("\\b", fp); break;
        case '\f': fputs("\\f", fp); break;
        case '\n': fputs("\\n", fp); break;
        case '\r': fputs("\\r", fp); break;
        case '\t': fputs("\\t", fp); break;
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

static void json_write_frame(FILE *fp, const FrameResult *frame)
{
    const BumpyDetectResult *r = &frame->detection;
    fprintf(fp, "    {\n");
    fprintf(fp, "      \"frame\": %d,\n", frame->frame_index);
    fprintf(fp, "      \"frame_name\": ");
    json_write_string(fp, frame->frame_name);
    fprintf(fp, ",\n");
    fprintf(fp, "      \"phase\": ");
    json_write_string(fp, bumpy_phase_name(r->phase));
    fprintf(fp, ",\n");
    fprintf(fp, "      \"mode\": ");
    json_write_string(fp, bumpy_mode_name(r->mode));
    fprintf(fp, ",\n");
    fprintf(fp, "      \"white_threshold\": %.2f,\n", r->white_threshold);
    fprintf(fp, "      \"white_threshold_candidate\": %.2f,\n", r->white_threshold_candidate);
    fprintf(fp, "      \"dark_threshold\": %.2f,\n", r->dark_threshold);
    fprintf(fp, "      \"target_x\": %.2f,\n", r->centerline.target_x);
    fprintf(fp, "      \"steer_error_px\": %.2f,\n", r->centerline.steer_error_px);
    fprintf(fp, "      \"centerline_row_count\": %d,\n", r->centerline.row_count);
    fprintf(fp, "      \"centerline_bottom_row_count\": %d,\n", r->centerline.bottom_row_count);
    if (r->centerline.top_y >= 0) {
        fprintf(fp, "      \"centerline_top_y\": %d,\n", r->centerline.top_y);
    } else {
        fprintf(fp, "      \"centerline_top_y\": null,\n");
    }
    if (r->centerline.bottom_y >= 0) {
        fprintf(fp, "      \"centerline_bottom_y\": %d,\n", r->centerline.bottom_y);
    } else {
        fprintf(fp, "      \"centerline_bottom_y\": null,\n");
    }
    fprintf(fp, "      \"centerline_mean_width\": %.2f,\n", r->centerline.mean_width);
    fprintf(fp, "      \"rib_count\": %d,\n", r->rib_count);
    if (r->best_component_found) {
        fprintf(
            fp,
            "      \"best_component_bbox\": [%d, %d, %d, %d],\n",
            r->best_component.xmin,
            r->best_component.ymin,
            r->best_component.xmax,
            r->best_component.ymax);
    } else {
        fprintf(fp, "      \"best_component_bbox\": null,\n");
    }
    fprintf(fp, "      \"elapsed_us\": %.2f\n", frame->elapsed_us);
    fprintf(fp, "    }");
}

static int write_json(
    const char *path,
    const CliOptions *options,
    const FrameResult *results,
    int frame_count,
    double total_us,
    double min_us,
    double max_us)
{
    int phase_counts[5] = { 0, 0, 0, 0, 0 };
    int first_approach = 0;
    int first_inside = 0;
    int first_exit = 0;
    int last_inside = 0;
    int last_exit = 0;
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }

    for (int i = 0; i < frame_count; i++) {
        const BumpyPhase phase = results[i].detection.phase;
        if ((int)phase >= 0 && (int)phase < 5) {
            phase_counts[(int)phase]++;
        }
        if (phase == BUMPY_PHASE_APPROACH && first_approach == 0) {
            first_approach = results[i].frame_index;
        }
        if (phase == BUMPY_PHASE_INSIDE) {
            if (first_inside == 0) first_inside = results[i].frame_index;
            last_inside = results[i].frame_index;
        }
        if (phase == BUMPY_PHASE_EXIT) {
            if (first_exit == 0) first_exit = results[i].frame_index;
            last_exit = results[i].frame_index;
        }
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"summary\": {\n");
    fprintf(fp, "    \"frame_dir\": ");
    json_write_string(fp, options->pgm_dir);
    fprintf(fp, ",\n");
    fprintf(fp, "    \"frame_count\": %d,\n", frame_count);
    fprintf(fp, "    \"first_approach_frame\": %d,\n", first_approach > 0 ? first_approach : 0);
    fprintf(fp, "    \"first_inside_frame\": %d,\n", first_inside > 0 ? first_inside : 0);
    fprintf(fp, "    \"first_exit_frame\": %d,\n", first_exit > 0 ? first_exit : 0);
    fprintf(fp, "    \"last_inside_frame\": %d,\n", last_inside > 0 ? last_inside : 0);
    fprintf(fp, "    \"last_exit_frame\": %d,\n", last_exit > 0 ? last_exit : 0);
    fprintf(fp, "    \"phase_counts\": {\n");
    fprintf(fp, "      \"approach_bumpy\": %d,\n", phase_counts[BUMPY_PHASE_APPROACH]);
    fprintf(fp, "      \"white_surface_only\": %d,\n", phase_counts[BUMPY_PHASE_WHITE_SURFACE_ONLY]);
    fprintf(fp, "      \"inside_bumpy\": %d,\n", phase_counts[BUMPY_PHASE_INSIDE]);
    fprintf(fp, "      \"exit_bumpy\": %d,\n", phase_counts[BUMPY_PHASE_EXIT]);
    fprintf(fp, "      \"uncertain\": %d\n", phase_counts[BUMPY_PHASE_UNCERTAIN]);
    fprintf(fp, "    },\n");
    fprintf(fp, "    \"timing\": {\n");
    fprintf(fp, "      \"total_us\": %.2f,\n", total_us);
    fprintf(fp, "      \"avg_us\": %.2f,\n", frame_count > 0 ? total_us / (double)frame_count : 0.0);
    fprintf(fp, "      \"min_us\": %.2f,\n", min_us);
    fprintf(fp, "      \"max_us\": %.2f,\n", max_us);
    fprintf(fp, "      \"fps\": %.2f\n", total_us > 0.0 ? 1000000.0 / (total_us / (double)frame_count) : 0.0);
    fprintf(fp, "    }\n");
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"timeline\": [\n");
    for (int i = 0; i < frame_count; i++) {
        json_write_frame(fp, &results[i]);
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
    fprintf(fp, "frame,frame_name,phase,mode,white_threshold,dark_threshold,target_x,steer_error_px,centerline_row_count,centerline_bottom_row_count,rib_count,best_component_found,best_xmin,best_ymin,best_xmax,best_ymax,elapsed_us\n");
    for (int i = 0; i < frame_count; i++) {
        const BumpyDetectResult *r = &results[i].detection;
        fprintf(
            fp,
            "%d,%s,%s,%s,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%d,%d,%d,%d,%d,%.2f\n",
            results[i].frame_index,
            results[i].frame_name,
            bumpy_phase_name(r->phase),
            bumpy_mode_name(r->mode),
            r->white_threshold,
            r->dark_threshold,
            r->centerline.target_x,
            r->centerline.steer_error_px,
            r->centerline.row_count,
            r->centerline.bottom_row_count,
            r->rib_count,
            r->best_component_found,
            r->best_component_found ? r->best_component.xmin : -1,
            r->best_component_found ? r->best_component.ymin : -1,
            r->best_component_found ? r->best_component.xmax : -1,
            r->best_component_found ? r->best_component.ymax : -1,
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
    BumpyDetectScratch scratch;
    uint8_t gray[BUMPY_MAX_PIXELS];
    int frame_count;
    double total_us = 0.0;
    double min_us = 0.0;
    double max_us = 0.0;
    float prev_white_threshold = 0.0f;
    int has_prev_white_threshold = 0;
    int parse_ret = parse_args(argc, argv, &options);

    if (parse_ret != 0) {
        print_usage(argv[0]);
        return parse_ret > 0 ? 0 : 2;
    }

    frames = (FrameName *)calloc(BUMPY_MAX_FRAMES, sizeof(FrameName));
    if (frames == NULL) {
        fprintf(stderr, "failed to allocate frame list\n");
        return 4;
    }

    frame_count = list_pgm_frames(options.pgm_dir, frames, BUMPY_MAX_FRAMES);
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
        char path[BUMPY_PATH_MAX];
        int width = 0;
        int height = 0;
        double t0 = 0.0;
        double t1 = 0.0;
        int ret;

        path_join(path, sizeof(path), options.pgm_dir, frames[i].name);
        ret = read_pgm_p5(path, gray, &width, &height);
        if (ret != 0) {
            fprintf(stderr, "failed to read PGM %s, err=%d\n", path, ret);
            free(frames);
            free(results);
            return 5;
        }

#if BUMPY_PC_ENABLE_TIMING
        t0 = now_us();
#endif
        ret = bumpy_detect_frame_gray(
            gray,
            width,
            height,
            prev_white_threshold,
            has_prev_white_threshold,
            &scratch,
            &results[i].detection);
#if BUMPY_PC_ENABLE_TIMING
        t1 = now_us();
#endif
        if (ret != 0) {
            fprintf(stderr, "detect failed for %s, err=%d\n", path, ret);
            free(frames);
            free(results);
            return 6;
        }

        prev_white_threshold = results[i].detection.white_threshold;
        has_prev_white_threshold = 1;

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

        if (options.debug_every > 0 && ((i + 1) % options.debug_every == 0)) {
            printf(
                "frame=%d phase=%s target_x=%.2f err=%.2f ribs=%d elapsed=%.2fus\n",
                i + 1,
                bumpy_phase_name(results[i].detection.phase),
                results[i].detection.centerline.target_x,
                results[i].detection.centerline.steer_error_px,
                results[i].detection.rib_count,
                results[i].elapsed_us);
        }
    }

    if (write_json(options.output_json, &options, results, frame_count, total_us, min_us, max_us) != 0) {
        fprintf(stderr, "failed to write %s: %s\n", options.output_json, strerror(errno));
        free(frames);
        free(results);
        return 7;
    }

#if BUMPY_PC_WRITE_CSV
    if (write_csv(options.output_csv, results, frame_count) != 0) {
        fprintf(stderr, "failed to write %s: %s\n", options.output_csv, strerror(errno));
        free(frames);
        free(results);
        return 8;
    }
#endif

    printf("frames: %d\n", frame_count);
    printf("timing_us: avg=%.2f min=%.2f max=%.2f fps=%.2f\n",
        frame_count > 0 ? total_us / (double)frame_count : 0.0,
        min_us,
        max_us,
        total_us > 0.0 ? 1000000.0 / (total_us / (double)frame_count) : 0.0);
    printf("output_json: %s\n", options.output_json);

    free(frames);
    free(results);
    return 0;
}
