#include "line_detector.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <time.h>
#endif

#ifndef LINE_PC_ENABLE_TIMING
#define LINE_PC_ENABLE_TIMING 1
#endif

#ifndef LINE_PC_WRITE_CSV
#define LINE_PC_WRITE_CSV 1
#endif

#define LINE_MAX_FRAMES 20000
#define LINE_PATH_MAX 1024

typedef struct { char name[256]; } FrameName;

typedef struct {
    char pgm_dir[LINE_PATH_MAX];
    char output_json[LINE_PATH_MAX];
    char output_csv[LINE_PATH_MAX];
    int max_frames;
    int debug_every;
} CliOptions;

typedef struct {
    int frame_index;
    char frame_name[256];
    LineDetectResult detection;
    double elapsed_us;
} FrameResult;

static int cmp_frame_name(const void *a, const void *b)
{
    const FrameName *fa = (const FrameName *)a;
    const FrameName *fb = (const FrameName *)b;
    return strcmp(fa->name, fb->name);
}

static void path_join(char *out, size_t out_size, const char *dir, const char *name)
{
    size_t n = strlen(dir);
    int sep = (n > 0 && dir[n - 1] != '/' && dir[n - 1] != '\\');
    snprintf(out, out_size, "%s%s%s", dir, sep ? "/" : "", name);
}

static int list_pgm_frames(const char *dir, FrameName *frames, int max_frames)
{
    int count = 0;
#if defined(_WIN32)
    char pattern[LINE_PATH_MAX];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    path_join(pattern, sizeof(pattern), dir, "*.pgm");
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            if (count < max_frames) {
                snprintf(frames[count].name, sizeof(frames[count].name), "%s", fd.cFileName);
                count++;
            }
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    DIR *dp = opendir(dir);
    struct dirent *entry;
    if (dp == NULL) return 0;
    while ((entry = readdir(dp)) != NULL) {
        size_t len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".pgm") == 0) {
            if (count < max_frames) {
                snprintf(frames[count].name, sizeof(frames[count].name), "%s", entry->d_name);
                count++;
            }
        }
    }
    closedir(dp);
#endif
    qsort(frames, (size_t)count, sizeof(FrameName), cmp_frame_name);
    return count;
}

static int read_next_token(FILE *fp, char *out, size_t out_size)
{
    int c;
    size_t n = 0;
    do {
        c = fgetc(fp);
        if (c == '#') {
            while (c != '\n' && c != EOF) c = fgetc(fp);
        }
    } while (c != EOF && isspace(c));

    if (c == EOF) return 0;
    while (c != EOF && !isspace(c)) {
        if (n + 1 < out_size) out[n++] = (char)c;
        c = fgetc(fp);
    }
    out[n] = '\0';
    return 1;
}

static int read_pgm_p5(const char *path, uint8_t *gray, int *width, int *height)
{
    char tok[64];
    int max_value;
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    if (!read_next_token(fp, tok, sizeof(tok)) || strcmp(tok, "P5") != 0) { fclose(fp); return -2; }
    if (!read_next_token(fp, tok, sizeof(tok))) { fclose(fp); return -3; }
    *width = atoi(tok);
    if (!read_next_token(fp, tok, sizeof(tok))) { fclose(fp); return -4; }
    *height = atoi(tok);
    if (!read_next_token(fp, tok, sizeof(tok))) { fclose(fp); return -5; }
    max_value = atoi(tok);
    if (*width <= 0 || *height <= 0 || *width > LINE_MAX_WIDTH || *height > LINE_MAX_HEIGHT || max_value != 255) {
        fclose(fp);
        return -6;
    }
    if (fread(gray, 1, (size_t)(*width * *height), fp) != (size_t)(*width * *height)) {
        fclose(fp);
        return -7;
    }
    fclose(fp);
    return 0;
}

static double now_us(void)
{
#if defined(_WIN32)
    static LARGE_INTEGER freq;
    LARGE_INTEGER c;
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000000.0 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
#endif
}

static void options_init(CliOptions *opt)
{
    memset(opt, 0, sizeof(*opt));
    snprintf(opt->output_json, sizeof(opt->output_json), "line_c_summary.json");
    snprintf(opt->output_csv, sizeof(opt->output_csv), "line_c_summary.csv");
}

static int parse_args(int argc, char **argv, CliOptions *opt)
{
    options_init(opt);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--pgm-dir") == 0 && i + 1 < argc) snprintf(opt->pgm_dir, sizeof(opt->pgm_dir), "%s", argv[++i]);
        else if (strcmp(argv[i], "--output-json") == 0 && i + 1 < argc) snprintf(opt->output_json, sizeof(opt->output_json), "%s", argv[++i]);
        else if (strcmp(argv[i], "--output-csv") == 0 && i + 1 < argc) snprintf(opt->output_csv, sizeof(opt->output_csv), "%s", argv[++i]);
        else if (strcmp(argv[i], "--max-frames") == 0 && i + 1 < argc) opt->max_frames = atoi(argv[++i]);
        else if (strcmp(argv[i], "--debug-every") == 0 && i + 1 < argc) opt->debug_every = atoi(argv[++i]);
        else return -1;
    }
    if (opt->pgm_dir[0] == '\0') return -1;
    return 0;
}

static void json_write_string(FILE *fp, const char *text)
{
    fputc('"', fp);
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) {
        if (*p == '\\') fputs("\\\\", fp);
        else if (*p == '"') fputs("\\\"", fp);
        else fputc(*p, fp);
    }
    fputc('"', fp);
}

static int write_json(
    const char *path,
    const CliOptions *opt,
    const FrameResult *rows,
    int n_rows,
    int n_detected,
    double total_us,
    double min_us,
    double max_us)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;
    fprintf(fp, "{\n  \"summary\": {\n");
    fprintf(fp, "    \"frame_dir\": ");
    json_write_string(fp, opt->pgm_dir);
    fprintf(fp, ",\n");
    fprintf(fp, "    \"frame_count\": %d,\n", n_rows);
    fprintf(fp, "    \"detected_count\": %d,\n", n_detected);
    fprintf(fp, "    \"timing\": {\n");
    fprintf(fp, "      \"total_us\": %.2f,\n", total_us);
    fprintf(fp, "      \"avg_us\": %.2f,\n", n_rows > 0 ? total_us / (double)n_rows : 0.0);
    fprintf(fp, "      \"min_us\": %.2f,\n", min_us);
    fprintf(fp, "      \"max_us\": %.2f,\n", max_us);
    fprintf(fp, "      \"fps\": %.2f\n", total_us > 0.0 ? (double)n_rows * 1000000.0 / total_us : 0.0);
    fprintf(fp, "    }\n");
    fprintf(fp, "  },\n  \"timeline\": [\n");
    for (int i = 0; i < n_rows; i++) {
        const LineDetectResult *r = &rows[i].detection;
        fprintf(fp, "    {\"frame\": %d, \"frame_name\": ", rows[i].frame_index);
        json_write_string(fp, rows[i].frame_name);
        fprintf(fp, ", \"detected\": %s, \"score\": %.5f, ", r->detected ? "true" : "false", r->confidence);
        fprintf(fp, "\"line_x_bottom\": %.5f, \"line_x_lookahead\": %.5f, ", r->line_x_bottom, r->line_x_lookahead);
        fprintf(fp, "\"line_yaw_deg\": %.5f, \"lateral_error_px\": %.5f, ", r->line_yaw_deg, r->lateral_error_px);
        fprintf(fp, "\"component_count\": %d, \"candidate_count\": %d, \"elapsed_us\": %.3f}", r->component_count, r->candidate_count, rows[i].elapsed_us);
        fprintf(fp, "%s\n", (i + 1 < n_rows) ? "," : "");
    }
    fprintf(fp, "  ]\n}\n");
    fclose(fp);
    return 0;
}

static int write_csv(const char *path, const FrameResult *rows, int n_rows)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;
    fprintf(fp, "frame_index,frame_name,detected,score,line_x_bottom,line_x_lookahead,line_yaw_deg,lateral_error_px,component_count,candidate_count,elapsed_us\n");
    for (int i = 0; i < n_rows; i++) {
        const LineDetectResult *r = &rows[i].detection;
        fprintf(fp, "%d,%s,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%d,%.3f\n",
            rows[i].frame_index, rows[i].frame_name, r->detected ? 1 : 0, r->confidence,
            r->line_x_bottom, r->line_x_lookahead, r->line_yaw_deg, r->lateral_error_px,
            r->component_count, r->candidate_count, rows[i].elapsed_us);
    }
    fclose(fp);
    return 0;
}

int main(int argc, char **argv)
{
    CliOptions opt;
    FrameName *names = NULL;
    FrameResult *rows = NULL;
    uint8_t *gray = NULL;
    LineDetectScratch *scratch = NULL;
    int frame_count;
    int width = 0, height = 0;
    int n_detected = 0;
    double total_us = 0.0, min_us = 1e30, max_us = 0.0;

    if (parse_args(argc, argv, &opt) != 0) {
        fprintf(stderr, "usage: %s --pgm-dir DIR [--output-json FILE] [--output-csv FILE] [--max-frames N] [--debug-every N]\n", argv[0]);
        return 2;
    }

    names = (FrameName *)calloc((size_t)LINE_MAX_FRAMES, sizeof(FrameName));
    if (!names) {
        return 10;
    }
    gray = (uint8_t *)malloc((size_t)LINE_MAX_PIXELS);
    if (!gray) {
        free(names);
        return 11;
    }

    frame_count = list_pgm_frames(opt.pgm_dir, names, LINE_MAX_FRAMES);
    if (frame_count <= 0) {
        fprintf(stderr, "no pgm frames found: %s\n", opt.pgm_dir);
        free(gray);
        free(names);
        return 3;
    }
    if (opt.max_frames > 0 && frame_count > opt.max_frames) frame_count = opt.max_frames;

    rows = (FrameResult *)calloc((size_t)frame_count, sizeof(FrameResult));
    if (!rows) {
        free(gray);
        free(names);
        return 4;
    }
    scratch = (LineDetectScratch *)calloc(1, sizeof(LineDetectScratch));
    if (!scratch) {
        free(rows);
        free(gray);
        free(names);
        return 9;
    }

    for (int i = 0; i < frame_count; i++) {
        char path[LINE_PATH_MAX];
        int ret;
        double t0 = 0.0, t1 = 0.0, dt = 0.0;
        path_join(path, sizeof(path), opt.pgm_dir, names[i].name);
        ret = read_pgm_p5(path, gray, &width, &height);
        if (ret != 0) {
            fprintf(stderr, "read_pgm failed %s (ret=%d)\n", path, ret);
            free(scratch);
            free(rows);
            free(gray);
            free(names);
            return 5;
        }
#if LINE_PC_ENABLE_TIMING
        t0 = now_us();
#endif
        ret = line_detect_frame_gray(gray, width, height, scratch, &rows[i].detection);
#if LINE_PC_ENABLE_TIMING
        t1 = now_us();
        dt = t1 - t0;
#endif
        if (ret != 0) {
            fprintf(stderr, "line_detect_frame_gray failed frame=%d ret=%d\n", i + 1, ret);
            free(scratch);
            free(rows);
            free(gray);
            free(names);
            return 6;
        }
        rows[i].frame_index = i + 1;
        snprintf(rows[i].frame_name, sizeof(rows[i].frame_name), "%s", names[i].name);
        rows[i].elapsed_us = dt;
        if (rows[i].detection.detected) n_detected++;
        total_us += dt;
        if (dt < min_us) min_us = dt;
        if (dt > max_us) max_us = dt;
        if (opt.debug_every > 0 && ((i + 1) % opt.debug_every) == 0) {
            printf("processed %d/%d detected=%d score=%.3f\n", i + 1, frame_count, rows[i].detection.detected ? 1 : 0, rows[i].detection.confidence);
        }
    }

    if (write_json(opt.output_json, &opt, rows, frame_count, n_detected, total_us, min_us, max_us) != 0) {
        free(scratch);
        free(rows);
        free(gray);
        free(names);
        return 7;
    }
#if LINE_PC_WRITE_CSV
    if (write_csv(opt.output_csv, rows, frame_count) != 0) {
        free(scratch);
        free(rows);
        free(gray);
        free(names);
        return 8;
    }
#endif
    printf("frames=%d detected=%d avg_us=%.3f fps=%.2f\n",
        frame_count,
        n_detected,
        frame_count > 0 ? total_us / (double)frame_count : 0.0,
        total_us > 0.0 ? (double)frame_count * 1000000.0 / total_us : 0.0);
    free(scratch);
    free(rows);
    free(gray);
    free(names);
    return 0;
}
