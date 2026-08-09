/**
 * bridge_host_file.c —— bridge_detect.c 的 PC 宿主验证程序 (磁盘帧输入版)
 * 直接编译工程源码 code1/vision/bridge_detect.c,
 * 从帧目录逐帧读 94x60 raw (5640 字节 uint8 行优先) 跑 bridge_detect_frame,
 * 打印与 MCU (main_cm7_1.c) 相同的格式行 (含 v= valid 门控输出)。
 *
 * Windows 版: 用 FindFirstFileA/FindNextFileA 替代 POSIX dirent。
 * 卷积算子用 bridge_conv_ref_b2.c 的 C 参考实现 (b2_ 前缀)。
 *
 * 用法: bridge_host_file <帧目录> [标签] [--repeat N] [--init-each]
 *   --init-each: 每帧前 bridge_detect_init (匹配 PC 每帧独立状态)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "bridge_detect.h"

static bridge_state_t  st;
static bridge_result_t res;
static int init_each = 0;   /* --init-each: 每图前 bridge_detect_init (匹配 PC 每帧独立状态) */

static int cmp_str(const void *a, const void *b)
{
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

int main(int argc, char **argv)
{
    const char *dir, *tag;
    char pattern[1024];
    WIN32_FIND_DATAA fd;
    HANDLE hFind;
    char **names = NULL;
    int cap = 0, n = 0, i, rep, repeat = 1;
    char path[1024];

    if (argc < 2) {
        fprintf(stderr, "用法: bridge_host_file <帧目录> [标签] [--repeat N] [--init-each]\n");
        return 2;
    }
    dir = argv[1];
    tag = argc > 2 ? argv[2] : dir;
    if (argc > 3 && !strcmp(argv[3], "--repeat") && argc > 4)
        repeat = atoi(argv[4]);
    if (repeat < 1) repeat = 1;
    for (i = 1; i < argc; i++)
        if (!strcmp(argv[i], "--init-each")) init_each = 1;

    snprintf(pattern, sizeof(pattern), "%s/*.bin", dir);
    hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "no .bin in %s\n", dir);
        return 1;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        if (n == cap) {
            cap = cap ? cap * 2 : 64;
            names = realloc(names, cap * sizeof(char *));
            if (!names) { perror("realloc"); return 1; }
        }
        names[n++] = strdup(fd.cFileName);
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);

    qsort(names, n, sizeof(char *), cmp_str);

    bridge_detect_init(&st);
    for (i = 0; i < n; i++) {
        char *name = names[i];
        char *ext = strrchr(name, '.');
        uint8_t img[BRIDGE_W * BRIDGE_H];
        FILE *fp;
        size_t rd;

        snprintf(path, sizeof(path), "%s/%s", dir, name);
        fp = fopen(path, "rb");
        if (!fp) {
            perror(path);
            continue;
        }
        rd = fread(img, 1, sizeof(img), fp);
        fclose(fp);
        if (rd != sizeof(img)) {
            fprintf(stderr, "skip %s (%zu bytes)\n", path, rd);
            continue;
        }

        bridge_detect_frame(img, &st, &res);

        if (ext) *ext = 0;
        if (init_each)
            bridge_detect_init(&st);   /* 匹配 PC: 每帧独立状态 */
        for (rep = 0; rep < repeat; rep++) {
            if (rep)
                bridge_detect_frame(img, &st, &res);
            printf("bridge %s__%s: 0 us mode=%d nl=%u nr=%u R%u G%u B%u T%u "
                   "ra=%d rb=%d ga=%d gb=%d ba=%d bb=%d ta=%d tb=%d tn=%d trms=%d v=%u\r\n",
                   tag, name,
                   (int)res.mode,
                   (unsigned)res.n_lines,
                   (unsigned)res.n_rows_ok,
                   (unsigned)res.has_red, (unsigned)res.has_green,
                   (unsigned)res.has_blue, (unsigned)res.has_top,
                   (int)(res.red.a * 100.0f), (int)(res.red.b * 100.0f),
                   (int)(res.green.a * 100.0f), (int)(res.green.b * 100.0f),
                   (int)(res.blue.a * 100.0f), (int)(res.blue.b * 100.0f),
                   (int)(res.top.a * 100.0f), (int)(res.top.b * 100.0f),
                   (int)res.top.n, (int)res.top.rms,
                   (unsigned)res.valid);
        }
        free(name);
    }
    free(names);
    return 0;
}
