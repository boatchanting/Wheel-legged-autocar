/**
 * ============================================================================
 * bumpy_pipeline.c  ——  颠簸路三段式边线提取 C 实现
 * ============================================================================
 * 与 heatmap/pipeline_extract_v3.py 同语义 (阈值宏在 bumpy_pipeline.h):
 *   ① 卷积+梯度 (bumpy_conv7) → 平面区域二值化 → 横向条纹
 *   ② 8 邻域连通域 → 每域 PCA 方向角 + 线性度
 *   ③ 每域 x 极值 3 点外点 + 倾角外扩跨域剔除 → RANSAC(穷举对) 直线
 *   ④ 帧航向角 (线性域加权圆均值) + 夹角门控
 *   ⑤ 时间验证: 连续帧稳定 + 跳变滤除 (状态按视频隔离)
 *
 * RANSAC 采用确定性穷举点对 (C host 与 MCU 共用同一实现, 对拍可复现),
 * 与 Python 的随机 RANSAC 在小型簇上收敛到同一主导假设 (容差对拍).
 *
 * RAM 紧缩 (2026-08-17, 纯缓冲时分复用, 算法语句零改动):
 *   各工作缓冲生命周期互不重叠, 故在 bumpy_pipeline_t 内按阶段复用同一块内存:
 *     ① 卷积: gxh/gyh 水平中间结果借用 mag2 区 (bumpy_conv7 scratch 参数)
 *              —— 卷积返回后 mag2 才逐像素写入, 无冲突;
 *     ② 分位: percentile_q64 就地 quickselect (不再另开 45KB 拷贝),
 *              mag2 此后不再需要 (strong/horiz 本就由 gx/gy 现算);
 *     ③ CCL : labels 借用 gx 区, 并查集 uf 借用 gy 区 (gx/gy ②后不再读),
 *              relab 仍用 mag2 区 (memset 后作根→新域映射);
 *   uf 用量上界: 新标号像素可单射到非 horiz 像素 (左邻/上邻必有一个非 horiz),
 *   故 ≤ PIX/2+1 项, 远小于 gy 区 PIX 项.
 * ============================================================================
 */
#include "bumpy_pipeline.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#if (BP_DEBUG_FRAME || BP_STAGE_TIMER)
#include <stdio.h>
#endif

#if BP_STAGE_TIMER
static unsigned int st_conv, st_strong, st_ccl, st_domain, st_outer, st_fit;
#define STAGE_T0()  (bp_stage_cyc())
#define STAGE_ACC(field_, t0_)  do { unsigned int t1_ = STAGE_T0(); field_ += t1_ - (t0_); } while (0)
#endif

#define PIX (BUMPY_W * BUMPY_H)

/* ---------------- 内部工作缓冲 (单实例, 常规 SRAM) ---------------- */
static int32_t *uf;                    /* 并查集: 指向 s->gy (帧首赋值, 用量 ≤ PIX/2+1) */
#define MAX_CC 64          /* 连通域数上限 (实测 ncc ≤ ~40) */
/* 每域在线统计 (索引 1..ncc, ncc ≤ MAX_CC; float 逐点累加, 与坐标数组版 domain_dir 同序) */
static float   cc_sx[MAX_CC + 1], cc_sy[MAX_CC + 1];          /* Σx, Σy */
static float   cc_cx[MAX_CC + 1], cc_cy[MAX_CC + 1];          /* 域质心 */
static float   cc_sxx[MAX_CC + 1], cc_syy[MAX_CC + 1], cc_sxy[MAX_CC + 1];  /* 中心化矩 */
static float   cc_vx[MAX_CC + 1], cc_vy[MAX_CC + 1];          /* 主轴单位向量 */
static float   rms_acc[MAX_CC + 1];                            /* 残差平方和 */
static int16_t ccn[MAX_CC + 1];           /* 每 CC 像素数 */
static int16_t cc_xmin[MAX_CC + 1], cc_xmax[MAX_CC + 1];
static float   ccang[MAX_CC + 1];         /* 每 CC 方向角 */
static uint8_t cclin[MAX_CC + 1];         /* 线性 (rms <= LINEAR_SIGMA) */
static uint8_t cccomp[MAX_CC + 1];        /* 合规 (>=MIN_CC_PIX && >=MIN_CC_W) */
static int16_t cc_minx[MAX_CC + 1][3], cc_miny[MAX_CC + 1][3];  /* x 最小 3 点 */
static int16_t cc_maxx[MAX_CC + 1][3], cc_maxy[MAX_CC + 1][3];  /* x 最大 3 点 */
static int16_t g_lp_x[6 * MAX_CC], g_lp_y[6 * MAX_CC];
static int16_t g_rp_x[6 * MAX_CC], g_rp_y[6 * MAX_CC];
static int16_t inl[6 * MAX_CC];           /* 内点索引 */

/* numpy percentile 'linear' (uint64 输入): v[lo] + frac*(v[hi]-v[lo]), pos=q*(n-1)
   用快速选择 (Hoare partition) 求第 k 小, O(n) 期望, 结果与全排序完全一致. */
static uint64_t nth_smallest(uint64_t *a, int n, int k)
{
    int lo = 0, hi = n - 1;
    while (lo < hi) {
        uint64_t piv = a[(lo + hi) >> 1];
        int i = lo, j = hi;
        while (i <= j) {
            while (a[i] < piv) i++;
            while (a[j] > piv) j--;
            if (i <= j) { uint64_t t = a[i]; a[i] = a[j]; a[j] = t; i++; j--; }
        }
        if (k <= j) hi = j;
        else if (k >= i) lo = i;
        else break;
    }
    return a[k];
}

/* numpy percentile 'linear' (uint64 输入): v[lo] + frac*(v[hi]-v[lo]), pos=q*(n-1)
   就地快速选择 (Hoare partition), 不另开 45KB 拷贝; 结果与全排序完全一致.
   注意: 返回后 v 内容被扰乱, 调用方不得再使用 (mag2 分位后本就不再需要). */
static uint64_t percentile_q64(uint64_t *v, int n, float q)
{
    double pos, frac;
    int lo, i;
    uint64_t vl, vh;
    pos = (double)q * (double)(n - 1);
    lo = (int)pos;
    frac = pos - (double)lo;
    vl = nth_smallest(v, n, lo);
    if (frac > 0.0) {
        vh = v[lo + 1];             /* quickselect 到 lo 后, lo+1..n-1 全 >= vl, 最小即第 lo+1 小 */
        for (i = lo + 1; i < n; i++)
            if (v[i] < vh) vh = v[i];
    } else {
        vh = vl;
    }
    return (uint64_t)((double)vl + frac * ((double)vh - (double)vl));
}

/* 圆角度差 [0,180) */
static float cd_deg(float a, float b)
{
    float d = fmodf(a - b, 180.0f);
    if (d < 0.0f) d += 180.0f;
    if (d > 90.0f) d = 180.0f - d;
    return d;
}

static float ang_mod180(float a)
{
    float r = fmodf(a, 180.0f);
    if (r < 0.0f) r += 180.0f;
    return r;
}

/* 并查集 */
static int uf_find(int x)
{
    while (uf[x] != x) { uf[x] = uf[uf[x]]; x = uf[x]; }
    return x;
}

/* ---------------------------------------------------------------------------
 * 8 邻域两遍法连通域 (numpy label, structure=ones(3,3)):
 *   relab 数组调用前需清零.
 * ------------------------------------------------------------------------- */
static int ccl8(const uint8_t *horiz, int32_t *labels, int32_t *relab)
{
    int x, y, i;
    int32_t next = 1;
    int ncc;

    for (y = 0; y < BUMPY_H; y++) {
        for (x = 0; x < BUMPY_W; x++) {
            int p = y * BUMPY_W + x;
            int root = 0;
            if (!horiz[p]) { labels[p] = 0; continue; }
            if (x > 0 && horiz[p - 1]) root = labels[p - 1];
            if (y > 0) {
                if (x > 0 && horiz[p - BUMPY_W - 1] &&
                    (!root || labels[p - BUMPY_W - 1] < root)) root = labels[p - BUMPY_W - 1];
                if (horiz[p - BUMPY_W] && (!root || labels[p - BUMPY_W] < root)) root = labels[p - BUMPY_W];
                if (x < BUMPY_W - 1 && horiz[p - BUMPY_W + 1] &&
                    (!root || labels[p - BUMPY_W + 1] < root)) root = labels[p - BUMPY_W + 1];
            }
            if (!root) {
                uf[next] = next;
                labels[p] = next++;
            } else {
                int r = uf_find(root);
                labels[p] = r;
                if (x > 0 && horiz[p - 1]) uf[uf_find(labels[p - 1])] = r;
                if (y > 0) {
                    if (x > 0 && horiz[p - BUMPY_W - 1]) uf[uf_find(labels[p - BUMPY_W - 1])] = r;
                    if (horiz[p - BUMPY_W]) uf[uf_find(labels[p - BUMPY_W])] = r;
                    if (x < BUMPY_W - 1 && horiz[p - BUMPY_W + 1]) uf[uf_find(labels[p - BUMPY_W + 1])] = r;
                }
            }
        }
    }

    ncc = 0;
    for (i = 0; i < PIX; i++) {
        if (labels[i]) {
            int r = uf_find(labels[i]);
            if (!relab[r]) {
                if (ncc >= MAX_CC) { labels[i] = 0; continue; }   /* 域数上限保护 */
                relab[r] = ++ncc;
            }
            labels[i] = relab[r];
        }
    }
    return ncc;
}

/* ---------------------------------------------------------------------------
 * 域统计 (三遍扫描, 与坐标数组版 domain_dir 的 float 累加逐位一致):
 *   遍1: count + Σx/Σy (float += int, 行优先序) + x 范围 + 极值 3 点
 *   遍2: 中心化矩 Σ(x-cx)² 等 (cx 用遍1均值, 与 domain_dir 同公式)
 *   遍3: PCA 主轴角 + 残差 Σ(dist²) → rms → 线性/合规标记
 * 每域点累加顺序 = 行优先全局扫描序 = 坐标收集序 (域内相对顺序不变), 故逐位一致.
 * ------------------------------------------------------------------------- */
static void push_min3(int ci, int x, int y)
{
    int i;
    for (i = 0; i < 3; i++) {
        if (cc_minx[ci][i] < 0 || x < cc_minx[ci][i]) {
            int j;
            for (j = 2; j > i; j--) { cc_minx[ci][j] = cc_minx[ci][j - 1]; cc_miny[ci][j] = cc_miny[ci][j - 1]; }
            cc_minx[ci][i] = (int16_t)x; cc_miny[ci][i] = (int16_t)y;
            break;
        }
    }
}

static void push_max3(int ci, int x, int y)
{
    int i;
    for (i = 0; i < 3; i++) {
        if (cc_maxx[ci][i] < 0 || x > cc_maxx[ci][i]) {
            int j;
            for (j = 2; j > i; j--) { cc_maxx[ci][j] = cc_maxx[ci][j - 1]; cc_maxy[ci][j] = cc_maxy[ci][j - 1]; }
            cc_maxx[ci][i] = (int16_t)x; cc_maxy[ci][i] = (int16_t)y;
            break;
        }
    }
}

static void domain_accum(const int32_t *labels, int ncc)
{
    int i;
    for (i = 0; i <= ncc; i++) {
        ccn[i] = 0;
        cc_sx[i] = cc_sy[i] = 0.0f;
        cc_xmin[i] = 32767; cc_xmax[i] = -1;
        cc_minx[i][0] = cc_minx[i][1] = cc_minx[i][2] = -1;
        cc_maxx[i][0] = cc_maxx[i][1] = cc_maxx[i][2] = -1;
    }
    for (i = 0; i < PIX; i++) {
        int ci = labels[i];
        if (ci) {
            int x = i % BUMPY_W, y = i / BUMPY_W;
            ccn[ci]++;
            cc_sx[ci] += (float)x;          /* float += int, 与 domain_dir 同 */
            cc_sy[ci] += (float)y;
            if (x < cc_xmin[ci]) cc_xmin[ci] = (int16_t)x;
            if (x > cc_xmax[ci]) cc_xmax[ci] = (int16_t)x;
            push_min3(ci, x, y);
            push_max3(ci, x, y);
        }
    }
    for (i = 1; i <= ncc; i++) {
        int n = ccn[i];
        cc_cx[i] = n ? (cc_sx[i] / n) : 0.0f;
        cc_cy[i] = n ? (cc_sy[i] / n) : 0.0f;
        cc_sxx[i] = cc_syy[i] = cc_sxy[i] = 0.0f;
        cc_vx[i] = 1.0f; cc_vy[i] = 0.0f;
    }
}

static void domain_finish(const int32_t *labels, int ncc)
{
    int ci, i;
    /* 遍2: 中心化矩 */
    for (i = 0; i < PIX; i++) {
        int ci2 = labels[i];
        if (ci2) {
            float dx = (i % BUMPY_W) - cc_cx[ci2];
            float dy = (i / BUMPY_W) - cc_cy[ci2];
            cc_sxx[ci2] += dx * dx;
            cc_syy[ci2] += dy * dy;
            cc_sxy[ci2] += dx * dy;
        }
    }
    /* 每域: cov → PCA 主轴角 + 单位向量 (与 domain_dir 同公式) */
    for (ci = 1; ci <= ncc; ci++) {
        int n = ccn[ci];
        float a, c, b, lam, vx, vy, norm, ang;
        if (n < 5) continue;
        a = cc_sxx[ci] / (n - 1);
        c = cc_syy[ci] / (n - 1);
        b = cc_sxy[ci] / (n - 1);
        lam = 0.5f * (a + c + sqrtf((a - c) * (a - c) + 4.0f * b * b));
        vx = b; vy = lam - a;
        norm = sqrtf(vx * vx + vy * vy);
        if (norm < 1e-12f) { vx = 1.0f; vy = 0.0f; }
        else { vx /= norm; vy /= norm; }
        ang = atan2f(vy, vx) * 57.295779513f;
        ccang[ci] = ang;
        cc_vx[ci] = vx; cc_vy[ci] = vy;
    }
    /* 遍3: 残差 Σ(dist²) */
    for (i = 0; i <= ncc; i++) rms_acc[i] = 0.0f;
    for (i = 0; i < PIX; i++) {
        int ci2 = labels[i];
        if (ci2 && ccn[ci2] >= 5) {
            float dx = (i % BUMPY_W) - cc_cx[ci2];
            float dy = (i / BUMPY_W) - cc_cy[ci2];
            float dist = dx * cc_vy[ci2] - dy * cc_vx[ci2];
            rms_acc[ci2] += dist * dist;
        }
    }
    for (ci = 1; ci <= ncc; ci++) {
        int n = ccn[ci];
        float rms;
        if (n < 10) { cclin[ci] = 0; cccomp[ci] = 0; continue; }
        rms = sqrtf(rms_acc[ci] / n);
        cclin[ci] = (rms <= BP_LINEAR_SIGMA);
        cccomp[ci] = (n >= BP_MIN_CC_PIX && (cc_xmax[ci] - cc_xmin[ci] + 1) >= BP_MIN_CC_W);
    }
}

/* ---------------------------------------------------------------------------
 * 跨连通域内点判定 (倾角外扩直线被覆盖):
 *   u=(cos ang, sin ang); side=L: 外扩向 x 减小, side=R: 向 x 增大
 *   候选 CC2 像素: t=v·u∈(0,ALONG_GAP] 且 d=|v·n|<=CROSS_TOL → 内点
 * 加速: 只扫描外扩方向 40px × 法向 ±4px 的矩形邻域 (labels 网格), 而非全域像素.
 * ------------------------------------------------------------------------- */
static int is_inner(int x, int y, float ang, int self_ci, int side_is_left,
                    const int32_t *labels)
{
    float ux = cosf(ang * 0.01745329251f);
    float uy = sinf(ang * 0.01745329251f);
    float nx, ny;
    int yy, xx, x0, x1, y0, y1, step;
    if ((side_is_left && ux > 0.0f) || (!side_is_left && ux < 0.0f)) { ux = -ux; uy = -uy; }
    nx = -uy; ny = ux;
    /* 外扩方向: x 前进 ALONG_GAP, 法向 ±(CROSS_TOL + 斜偏) */
    y0 = y - 6; y1 = y + 6;
    if (y0 < 0) y0 = 0;
    if (y1 >= BUMPY_H) y1 = BUMPY_H - 1;
    if (ux > 0.0f) { x0 = x; x1 = x + (int)BP_ALONG_GAP + 1; step = 1; }
    else { x0 = x; x1 = x - (int)BP_ALONG_GAP - 1; step = -1; }
    if (x1 < 0) x1 = 0;
    if (x1 >= BUMPY_W) x1 = BUMPY_W - 1;
    for (yy = y0; yy <= y1; yy++) {
        const int32_t *row = labels + yy * BUMPY_W;
        for (xx = x0; (step > 0) ? (xx <= x1) : (xx >= x1); xx += step) {
            int32_t lb = row[xx];
            if (lb == 0 || lb == self_ci || !cclin[lb]) continue;
            {
                float dx = (float)(xx - x), dy = (float)(yy - y);
                float t = dx * ux + dy * uy;
                float d = fabsf(dx * nx + dy * ny);
                if (t > 0.0f && t <= BP_ALONG_GAP && d <= BP_CROSS_TOL)
                    return 1;
            }
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * 外点提取: 每合规线性 CC 取 x 最小 3 点(左) / 最大 3 点(右)
 * 硬边界 + 跨域剔除; 返回左右点数。
 * ------------------------------------------------------------------------- */
static void extract_outer(int ncc, const int32_t *labels, int *nl, int *nr)
{
    int ci, i, nl_ = 0, nr_ = 0;
    for (ci = 1; ci <= ncc; ci++) {
        if (!cccomp[ci] || !cclin[ci]) continue;
        for (i = 0; i < 3; i++) {
            if (cc_minx[ci][i] < 0) continue;
            {
                int x = cc_minx[ci][i], y = cc_miny[ci][i];
                if (x > BP_EDGE_M && x < BUMPY_W - BP_EDGE_M &&
                    !is_inner(x, y, ccang[ci], ci, 1, labels)) {
                    g_lp_x[nl_] = (int16_t)x; g_lp_y[nl_] = (int16_t)y; nl_++;
                }
            }
            if (cc_maxx[ci][i] < 0) continue;
            {
                int x = cc_maxx[ci][i], y = cc_maxy[ci][i];
                if (x > BP_EDGE_M && x < BUMPY_W - BP_EDGE_M &&
                    !is_inner(x, y, ccang[ci], ci, 0, labels)) {
                    g_rp_x[nr_] = (int16_t)x; g_rp_y[nr_] = (int16_t)y; nr_++;
                }
            }
        }
    }
    *nl = nl_; *nr = nr_;
}

/* ---------------------------------------------------------------------------
 * 边线拟合 (穷举点对 RANSAC + PCA 精化 + 显著性门槛):
 *   tol=3px, 内点数最大 (与 Python fit_outer 同准则);
 *   nin>=MIN_OUT_N 且 (x 跨度>=MIN_OUT_SPAN 或 y 跨度>=MIN_OUT_SPAN).
 * ------------------------------------------------------------------------- */
static int fit_outer(const int16_t *xs, const int16_t *ys, int n, bumpy_line_t *out)
{
    int i, j, k;
    int best_i = -1, best_j = -1, best_nin = -1;
    if (n < 3) return 0;
    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            float x1 = xs[i], y1 = ys[i], x2 = xs[j], y2 = ys[j];
            float d = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
            float nx, ny;
            int nin = 0;
            if (d < 1e-6f) continue;
            nx = -(y2 - y1) / d; ny = (x2 - x1) / d;
            for (k = 0; k < n; k++) {
                float dist = fabsf((xs[k] - x1) * nx + (ys[k] - y1) * ny);
                if (dist < BP_RANSAC_TOL) nin++;
            }
            if (nin >= 3 && nin > best_nin) { best_nin = nin; best_i = i; best_j = j; }
        }
    }
    if (best_i < 0) return 0;
        {
            float x1 = xs[best_i], y1 = ys[best_i], x2 = xs[best_j], y2 = ys[best_j];
            float d = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
            float nx = -(y2 - y1) / d, ny = (x2 - x1) / d;
            int cnt = 0, xmin = 9999, xmax = -9999, ymin = 9999, ymax = -9999;
            float cx = 0, cy = 0, sxx = 0, syy = 0, sxy = 0;
            for (k = 0; k < n; k++) {
                float dist = fabsf((xs[k] - x1) * nx + (ys[k] - y1) * ny);
                if (dist < BP_RANSAC_TOL) {
                    inl[cnt++] = (int16_t)k;
                    if (xs[k] < xmin) xmin = xs[k];
                    if (xs[k] > xmax) xmax = xs[k];
                    if (ys[k] < ymin) ymin = ys[k];
                    if (ys[k] > ymax) ymax = ys[k];
                }
            }
            if (cnt < BP_MIN_OUT_N) return 0;
            if ((float)(xmax - xmin) < BP_MIN_OUT_SPAN &&
                (float)(ymax - ymin) < BP_MIN_OUT_SPAN) return 0;
            for (k = 0; k < cnt; k++) { cx += xs[inl[k]]; cy += ys[inl[k]]; }
            cx /= cnt; cy /= cnt;
            for (k = 0; k < cnt; k++) {
                float dx = xs[inl[k]] - cx, dy = ys[inl[k]] - cy;
                sxx += dx * dx; syy += dy * dy; sxy += dx * dy;
            }
            {
                float a = sxx / (cnt - 1), c = syy / (cnt - 1), b = sxy / (cnt - 1);
                float lam = 0.5f * (a + c + sqrtf((a - c) * (a - c) + 4.0f * b * b));
                float vx = b, vy = lam - a, norm = sqrtf(vx * vx + vy * vy), ang;
                if (norm < 1e-12f) { vx = 1.0f; vy = 0.0f; }
                else { vx /= norm; vy /= norm; }
                ang = atan2f(vy, vx) * 57.295779513f;
                out->ang = ang;
            }
            out->cx = cx; out->cy = cy;
            out->n = cnt;
        }
    return 1;
}

/* 帧航向角: 线性 CC 方向角加权圆均值 (权重=像素数),
   返回 0 = 线性域不足 BP_MIN_HDG_LINES 条 或 无有效方向（2026-08-18 增加最少条数判定） */
static int frame_heading(int ncc, float *hdg_out)
{
    float ys = 0, xs = 0;
    int ci, n_line = 0;
    for (ci = 1; ci <= ncc; ci++) {
        if (!cclin[ci]) continue;
        n_line++;
        {
            float a2 = 2.0f * ccang[ci] * 0.01745329251f;
            ys += ccn[ci] * sinf(a2);
            xs += ccn[ci] * cosf(a2);
        }
    }
    /* 至少检出 BP_MIN_HDG_LINES 条横向线才算"有颠簸条纹"，防单条噪声误判 */
    if (n_line < (int)BP_MIN_HDG_LINES) return 0;
    if (fabsf(xs) < 1e-9f && fabsf(ys) < 1e-9f) return 0;
    *hdg_out = 0.5f * atan2f(ys, xs) * 57.295779513f;
    return 1;
}

/* 时间验证: 连续 MIN_STABLE 帧稳定才显示; 跳变超限清空重新积累 */
static int hist_update(int *h_n, float *h_a, float *h_x, float *h_y,
                       const bumpy_line_t *fit)
{
    if (!fit) { *h_n = 0; return 0; }
    if (*h_n > 0) {
        float pa = h_a[*h_n - 1], px0 = h_x[*h_n - 1], py0 = h_y[*h_n - 1];
        float da = cd_deg(fit->ang, pa);
        float dp = sqrtf((fit->cx - px0) * (fit->cx - px0) +
                         (fit->cy - py0) * (fit->cy - py0));
        if (da > BP_MAX_ANG_JMP || dp > BP_MAX_POS_JMP) { *h_n = 0; return 0; }
    }
    if (*h_n == BP_MIN_STABLE) {
        memmove(h_a, h_a + 1, (BP_MIN_STABLE - 1) * sizeof(float));
        memmove(h_x, h_x + 1, (BP_MIN_STABLE - 1) * sizeof(float));
        memmove(h_y, h_y + 1, (BP_MIN_STABLE - 1) * sizeof(float));
        *h_n = BP_MIN_STABLE - 1;
    }
    h_a[*h_n] = fit->ang; h_x[*h_n] = fit->cx; h_y[*h_n] = fit->cy;
    (*h_n)++;
    return (*h_n >= BP_MIN_STABLE);
}

void bumpy_pipeline_init(bumpy_pipeline_t *s)
{
    memset(s, 0, sizeof(*s));
}

/* ---------------------------------------------------------------------------
 * 主入口
 * ------------------------------------------------------------------------- */

void bumpy_pipeline_frame(bumpy_pipeline_t *s, const uint8_t *img, bumpy_frame_result_t *out)
{
    int i, ncc, nl, nr;
    int32_t *labels = (int32_t *)s->gx;     /* ③④ CCL 标号复用 gx 区 (②后 gx 不再读) */
#if BP_DEBUG_FRAME
    int dbg_cnt_strong = 0;
#endif
    float hdg;
    bumpy_line_t lfit, rfit;
    bumpy_line_t *lptr = NULL, *rptr = NULL;

    memset(out, 0, sizeof(*out));

    /* ① 卷积+梯度: mag² (uint64 精确) 供分位/强点判定 */
    {
#if BP_STAGE_TIMER
        unsigned int s0 = STAGE_T0();
#endif
        bumpy_conv7(img, s->gx, s->gy, (int32_t *)s->mag2);   /* 水平中间结果暂借 mag2 区 */
        for (i = 0; i < PIX; i++) {
            int32_t gxv = s->gx[i], gyv = s->gy[i];
            uint32_t ax = (gxv < 0) ? (uint32_t)(-(int64_t)gxv) : (uint32_t)gxv;
            uint32_t ay = (gyv < 0) ? (uint32_t)(-(int64_t)gyv) : (uint32_t)gyv;
            s->mag2[i] = (uint64_t)ax * (uint64_t)ax + (uint64_t)ay * (uint64_t)ay;  /* 2×UMULL */
        }
#if BP_STAGE_TIMER
        STAGE_ACC(st_conv, s0);
#endif
    }

    /* ② 平面区域二值化 (平方域单调等价) + 横向判定 (整数, 免 atan2):
       strong = |Gy|²>=thr_v² 且 |Gx|²<=thr_x²
       horiz  = strong 且 |θs|<DIR_TOL ⟺ gx²·K < gy²·tan²(DIR_TOL)  (K=100000, tan²20°=13247)
       与 Python atan2 判定等价 (72 帧仅 2 个 |θs| 恰为 ±20° 的边界点差异).
       分位就地 quickselect, 返回后 mag2 内容失效 (③ memset 复用为 relab). */
    {
#if BP_STAGE_TIMER
        unsigned int s0 = STAGE_T0();
#endif
        uint64_t p85q = percentile_q64(s->mag2, PIX, BP_MAG_PCT / 100.0f);
        double rr = (double)BP_VERT_RELAX * BP_VERT_RELAX;
        uint64_t thr_v2 = (uint64_t)((double)p85q * rr + 0.5);
        uint64_t thr_x2 = (uint64_t)((double)p85q * rr * BP_HORIZ_CAP * BP_HORIZ_CAP + 0.5);
        for (i = 0; i < PIX; i++) {
            int32_t gxv = s->gx[i], gyv = s->gy[i];
            uint64_t gx2, gy2;
            uint8_t st;
            if (gxv < 0) gxv = -gxv; if (gyv < 0) gyv = -gyv;
            gx2 = (uint64_t)gxv * (uint64_t)gxv;
            gy2 = (uint64_t)gyv * (uint64_t)gyv;
            st = (gy2 >= thr_v2) && (gx2 <= thr_x2);
            s->horiz[i] = st && (gx2 * 100000u < gy2 * 13247u);
#if BP_DEBUG_FRAME
            dbg_cnt_strong += st;
#endif
        }
#if BP_STAGE_TIMER
        STAGE_ACC(st_strong, s0);
#endif
    }

    /* ③ 连通域 + 每域方向角 */
    {
#if BP_STAGE_TIMER
        unsigned int s0 = STAGE_T0();
#endif
    uf = (int32_t *)s->gy;                                      /* 并查集复用 gy 区 (②后 gy 不再读) */
    memset((int32_t *)s->mag2, 0, (PIX + 1) * sizeof(int32_t));
    ncc = ccl8(s->horiz, labels, (int32_t *)s->mag2);
#if BP_STAGE_TIMER
        STAGE_ACC(st_ccl, s0);
        s0 = STAGE_T0();
#endif

    domain_accum(labels, ncc);
    domain_finish(labels, ncc);
#if BP_STAGE_TIMER
        STAGE_ACC(st_domain, s0);
#endif
    }

    /* ④ 外点 + 拟合 */
    {
#if BP_STAGE_TIMER
        unsigned int s0 = STAGE_T0();
#endif
    extract_outer(ncc, labels, &nl, &nr);
#if BP_STAGE_TIMER
        STAGE_ACC(st_outer, s0);
        s0 = STAGE_T0();
#endif
    if (fit_outer(g_lp_x, g_lp_y, nl, &lfit)) lptr = &lfit;
    if (fit_outer(g_rp_x, g_rp_y, nr, &rfit)) rptr = &rfit;
#if BP_STAGE_TIMER
        STAGE_ACC(st_fit, s0);
#endif
    }

    /* ④.5 原始单帧边线透出（未时间验证，渲染与横向观测用，2026-08-18） */
    if (lptr) { out->raw_L = *lptr; out->raw_L.valid = 1; }
    if (rptr) { out->raw_R = *rptr; out->raw_R.valid = 1; }

    /* ⑤ 帧航向角 + 夹角门控 (hdg 直出到 out: 条纹倾斜角与边线成败无关, 2026-08-17 引出) */
    if (!frame_heading(ncc, &hdg)) { lptr = NULL; rptr = NULL; }
    else {
        out->hdg_valid = 1;
        out->hdg = hdg;
        if (lptr && cd_deg(ang_mod180(lptr->ang), hdg) > BP_MAX_HDG_DIFF) lptr = NULL;
        if (rptr && cd_deg(ang_mod180(rptr->ang), hdg) > BP_MAX_HDG_DIFF) rptr = NULL;
    }

    /* ⑥ 时间验证 */
    if (BP_TEMPORAL) {
        if (hist_update(&s->hL_n, s->hL_a, s->hL_x, s->hL_y, lptr)) {
            out->L.valid = 1;
            out->L.ang = ang_mod180(lptr->ang);
            out->L.cx = lptr->cx; out->L.cy = lptr->cy; out->L.n = lptr->n;
        }
        if (hist_update(&s->hR_n, s->hR_a, s->hR_x, s->hR_y, rptr)) {
            out->R.valid = 1;
            out->R.ang = ang_mod180(rptr->ang);
            out->R.cx = rptr->cx; out->R.cy = rptr->cy; out->R.n = rptr->n;
        }
    } else {
        if (lptr) {
            out->L.valid = 1;
            out->L.ang = ang_mod180(lptr->ang);
            out->L.cx = lptr->cx; out->L.cy = lptr->cy; out->L.n = lptr->n;
        }
        if (rptr) {
            out->R.valid = 1;
            out->R.ang = ang_mod180(rptr->ang);
            out->R.cx = rptr->cx; out->R.cy = rptr->cy; out->R.n = rptr->n;
        }
    }

#if BP_DEBUG_FRAME
    {
        /* 注: strong 数组已取消 (RAM 紧缩), strong 计数在 ②循环内同步累计;
           此处 p85q 重算读到的 mag2 已被 ③ 复用为 relab, thr 打印值不可信 (沿用原调试行为). */
        int cnt_horiz = 0;
        uint64_t p85q = percentile_q64(s->mag2, PIX, BP_MAG_PCT / 100.0f);
        for (i = 0; i < PIX; i++) { cnt_horiz += s->horiz[i]; }
        printf("DBG thr=%.0f strong=%d horiz=%d ncc=%d nl=%d nr=%d hdg=%.1f "
               "L=%d(%.1f) R=%d(%.1f)\r\n",
               sqrtf((float)p85q) * BP_VERT_RELAX, dbg_cnt_strong, cnt_horiz, ncc, nl, nr, hdg,
               lptr != NULL, lptr ? lptr->ang : 0.0f,
               rptr != NULL, rptr ? rptr->ang : 0.0f);
    }
#endif
#if BP_STAGE_TIMER
    printf("STAGE conv=%lu strong=%lu ccl=%lu domain=%lu outer=%lu fit=%lu (cyc)\r\n",
           (unsigned long)st_conv, (unsigned long)st_strong, (unsigned long)st_ccl,
           (unsigned long)st_domain, (unsigned long)st_outer, (unsigned long)st_fit);
    st_conv = st_strong = st_ccl = st_domain = st_outer = st_fit = 0;
#endif
}
