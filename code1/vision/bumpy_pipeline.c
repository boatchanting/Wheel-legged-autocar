/**
 * ============================================================================
 * bumpy_pipeline.c  ——  颠簸路三段式边线提取 C 实现 (v3 gy-only)
 * ============================================================================
 * 与 heatmap/pipeline_extract_v3.py 同语义 (阈值宏在 bumpy_pipeline.h):
 *   ① 卷积 (gy-only, bumpy_conv7_gy) → 双阈值带符号二值化 → 横向条纹
 *   ② 8 邻域连通域 (等值邻接拆符号) → 每域 PCA 方向角 + 线性度
 *   ③ 每域 x 极值 3 点外点 + 同符号倾角外扩跨域剔除 → RANSAC(穷举对) 直线
 *   ④ 帧航向角 (合规域加权圆均值) + 个数/方差双门限
 *   ⑤ 时间验证: 连续帧稳定 + 跳变滤除 (状态按视频隔离)
 *
 * RANSAC 采用确定性穷举点对 (C host 与 MCU 共用同一实现, 对拍可复现),
 * 与 Python 的随机 RANSAC 在小型簇上收敛到同一主导假设 (容差对拍).
 *
 * RAM 紧缩 (2026-08-17 纯缓冲时分复用 + 2026-08-19 v3 gy-only):
 *   ① 卷积 (gy-only): 水平中间仅 gyh (1×PIX int32) 借用 gyh 区 (bumpy_conv7_gy scratch);
 *   ② 阈值: 亮度归一 T = BP_NORM_K·mean, 双阈值带符号判定, 无 mag²/无分位/无平方;
 *   ③ CCL : labels 借用 gyh 区前段, relab 借用同区后段 (uint16 别名),
 *            并查集 uf 借用 gy 区 (②后 gy 不再读);
 *   labels 值上界: 新标号像素可单射到非 horiz 像素 (左邻/上邻必有一个非 horiz),
 *   故 ≤ PIX/2+1 = 2821 < 65536, uint16 安全; uf 用量同样 ≤ PIX/2+1 项, 远小于 gy 区.
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
static unsigned int st_conv, st_strong, st_ccl, st_domain, st_outer;
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
static uint8_t ccsign[MAX_CC + 1];        /* 域符号 (horiz 掩膜值 1/2; 2026-08-19 拆符号 is_inner 用,
                                             旧规则掩膜全 1 → 判定与逐位原版一致) */
static int16_t cc_minx[MAX_CC + 1][3], cc_miny[MAX_CC + 1][3];  /* x 最小 3 点 */
static int16_t cc_maxx[MAX_CC + 1][3], cc_maxy[MAX_CC + 1][3];  /* x 最大 3 点 */
static int16_t g_lp_x[6 * MAX_CC], g_lp_y[6 * MAX_CC];
static int16_t g_rp_x[6 * MAX_CC], g_rp_y[6 * MAX_CC];
/* (v3: inl[] RANSAC 内点索引已删除) */

/* 圆角度差 [0,180) —— (v3: cd_deg 随 RANSAC/时间验证删除) */
/* (v3: ang_mod180 随边线时间验证删除) */

/* 并查集 */
static int uf_find(int x)
{
    while (uf[x] != x) { uf[x] = uf[uf[x]]; x = uf[x]; }
    return x;
}

/* ---------------------------------------------------------------------------
 * 8 邻域两遍法连通域 (numpy label, structure=ones(3,3)):
 *   relab 数组调用前需清零.
 *   2026-08-19: 邻接判定改为"掩膜值相等"(==horiz[p]) —— 支持 horiz 按 gy 符号编码 1/2
 *   (拆符号 CCL, 防条纹正负沿粘团); 旧规则掩膜全为 1, 等值判定与原"非零"判定逐位等价.
 * ------------------------------------------------------------------------- */
static int ccl8(const uint8_t *horiz, uint16_t *labels, uint16_t *relab)
{
    int x, y, i;
    int32_t next = 1;
    int ncc;

    for (y = 0; y < BUMPY_H; y++) {
        for (x = 0; x < BUMPY_W; x++) {
            int p = y * BUMPY_W + x;
            int root = 0;
            if (!horiz[p]) { labels[p] = 0; continue; }
            if (x > 0 && horiz[p - 1] == horiz[p]) root = labels[p - 1];
            if (y > 0) {
                if (x > 0 && horiz[p - BUMPY_W - 1] == horiz[p] &&
                    (!root || labels[p - BUMPY_W - 1] < root)) root = labels[p - BUMPY_W - 1];
                if (horiz[p - BUMPY_W] == horiz[p] &&
                    (!root || labels[p - BUMPY_W] < root)) root = labels[p - BUMPY_W];
                if (x < BUMPY_W - 1 && horiz[p - BUMPY_W + 1] == horiz[p] &&
                    (!root || labels[p - BUMPY_W + 1] < root)) root = labels[p - BUMPY_W + 1];
            }
            if (!root) {
                uf[next] = next;
                labels[p] = (uint16_t)next++;
            } else {
                int r = uf_find(root);
                labels[p] = (uint16_t)r;
                if (x > 0 && horiz[p - 1] == horiz[p]) uf[uf_find(labels[p - 1])] = r;
                if (y > 0) {
                    if (x > 0 && horiz[p - BUMPY_W - 1] == horiz[p]) uf[uf_find(labels[p - BUMPY_W - 1])] = r;
                    if (horiz[p - BUMPY_W] == horiz[p]) uf[uf_find(labels[p - BUMPY_W])] = r;
                    if (x < BUMPY_W - 1 && horiz[p - BUMPY_W + 1] == horiz[p]) uf[uf_find(labels[p - BUMPY_W + 1])] = r;
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
                relab[r] = (uint16_t)++ncc;
                ccsign[ncc] = horiz[i];     /* 记录域符号 (拆符号 is_inner 用) */
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

static void domain_accum(const uint16_t *labels, int ncc)
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

static void domain_finish(const uint16_t *labels, int ncc)
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
                    const uint16_t *labels)
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
        const uint16_t *row = labels + yy * BUMPY_W;
        for (xx = x0; (step > 0) ? (xx <= x1) : (xx >= x1); xx += step) {
            uint16_t lb = row[xx];
            if (lb == 0 || lb == self_ci || !cclin[lb]) continue;
            if (ccsign[lb] != ccsign[self_ci]) continue;   /* 拆符号: 只认同号域延伸 (2026-08-19) */
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
static void extract_outer(int ncc, const uint16_t *labels, int *nl, int *nr)
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
 * (v3, 2026-08-19) 边线拟合 fit_outer (穷举点对 RANSAC + PCA) 已删除:
 *   边线位置改由 bumpy_vision 对外点逐点 IPM → 物理 x 主带 → 主带内点均值直接得出,
 *   不再需要拟合直线/内点集/显著性门槛/夹角门控/时间验证 (见规划文档 §2 目标架构).
 * --------------------------------------------------------------------------- */

/* 帧航向角: 合规线性 CC 方向角加权圆均值 (权重=像素数), 双门限 (v3, 2026-08-19):
   ① 个数 ≥ BP_MIN_HDG_LINES(2) —— 拒绝单线 (1 个 CC 时圆 std≡0 恒有效, 防单条噪声误报);
   ② 帧内散布方差门: 加权圆 std > BP_HDG_STD_MAX(2°) → 该帧角度不可信, 判无效.
   口径: 合规线性域 (cccomp: n≥30 且 宽≥30) —— 旧口径(n≥10)在全库无条纹帧上常开 96~100%,
   作"在颠簸段"信号失效 (见 lat_study/卷积核评估与阈值锚定证明报告.md §6). */
static int frame_heading(int ncc, float *hdg_out)
{
    float ys = 0, xs = 0;
    int ci, n_line = 0;
    int wsum = 0;
    for (ci = 1; ci <= ncc; ci++) {
        if (!cclin[ci]) continue;
        if (!cccomp[ci]) continue;
        n_line++;
        wsum += ccn[ci];
        {
            float a2 = 2.0f * ccang[ci] * 0.01745329251f;
            ys += ccn[ci] * sinf(a2);
            xs += ccn[ci] * cosf(a2);
        }
    }
    /* 门限①: 至少检出 BP_MIN_HDG_LINES 条合规横向线才算"有颠簸条纹" (拒绝单线) */
    if (n_line < (int)BP_MIN_HDG_LINES) return 0;
    if (fabsf(xs) < 1e-9f && fabsf(ys) < 1e-9f) return 0;
    /* 门限②: 帧内散布门 R = |Σw·e^{i2a}|/Σw → σ = ½·√(−2·lnR) */
    {
        float R = sqrtf(xs * xs + ys * ys) / (float)wsum;
        float lnR = logf(R < 1e-9f ? 1e-9f : R);
        float std_deg = 0.5f * sqrtf(-2.0f * lnR) * 57.295779513f;
        if (std_deg > BP_HDG_STD_MAX) return 0;
    }
    *hdg_out = 0.5f * atan2f(ys, xs) * 57.295779513f;
    return 1;
}

/* (v3: hist_update 边线时间验证已删除 —— 边线质量由下游 IPM 主带门 + 采信窗滤波负责) */

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
    uint16_t *labels;           /* ③④ CCL 标号复用 gyh 区前段 (垂直 pass 后 gyh 不再读) */
    uint16_t *relab;            /* ③④ 根→新域映射 (gyh 区第二段, uint16 别名) */
#if BP_DEBUG_FRAME
    int dbg_cnt_strong = 0;
#endif
    float hdg;
    uint32_t img_sum = 0;       /* 帧灰度和 (亮度归一阈值用) */

    memset(out, 0, sizeof(*out));

    /* ① 卷积 (gy-only): 只算 Gy (垂直 D 核); 无 gx/无 mag²/无分位; 同循环累计帧灰度和 */
    {
#if BP_STAGE_TIMER
        unsigned int s0 = STAGE_T0();
#endif
        bumpy_conv7_gy(img, s->gy, s->gyh);   /* 水平中间 gyh 借用 gyh 区 (1×PIX) */
        for (i = 0; i < PIX; i++) {
            img_sum += img[i];
        }
#if BP_STAGE_TIMER
        STAGE_ACC(st_conv, s0);
#endif
    }

    /* ② 双阈值带符号二值化 (v3, 2026-08-19):
       亮度归一 T = BP_NORM_K·mean(gray) = BP_NORM_K·img_sum/PIX (整数);
       判定: gy ≥ +T → horiz=1 (正沿); gy ≤ −T → horiz=2 (负沿); 其余 0.
       取消绝对值: 两个阈值正好对应两种符号连通域, 天然拆符号 (ccl8 等值邻接);
       无平方/无 uint64 (|gy|≤66,300, T≤637,500, int32 安全);
       与测试工程 |gy|≥T / gy²≥T² 非负等价 → 逐位一致. */
    {
#if BP_STAGE_TIMER
        unsigned int s0 = STAGE_T0();
#endif
        const int32_t tnorm = (int32_t)(((uint32_t)BP_NORM_K * img_sum) / PIX);
        for (i = 0; i < PIX; i++) {
            int32_t gyv = s->gy[i];
            uint8_t st = 0;
            if (gyv >= tnorm) st = 1;
            else if (gyv <= -tnorm) st = 2;
            s->horiz[i] = st;
#if BP_DEBUG_FRAME
            dbg_cnt_strong += (st != 0);
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
    labels = (uint16_t *)s->gyh;                                /* CCL 标号复用 gyh 区前段 */
    relab  = (uint16_t *)s->gyh + PIX;                          /* 根→新域映射 (同区第二段) */
    memset((void *)relab, 0, PIX * sizeof(uint16_t));
    ncc = ccl8(s->horiz, labels, relab);
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

    /* ④ 外点提取 + 透出 (v3: 不再 RANSAC 拟合; 边线位置由 bumpy_vision 逐点 IPM → 物理 x 主带提取) */
    {
#if BP_STAGE_TIMER
        unsigned int s0 = STAGE_T0();
#endif
    extract_outer(ncc, labels, &nl, &nr);
    out->lp_n = (uint8_t)(nl < BP_OUT_MAX ? nl : BP_OUT_MAX);
    for (i = 0; i < out->lp_n; i++) { out->lp_x[i] = g_lp_x[i]; out->lp_y[i] = g_lp_y[i]; }
    out->rp_n = (uint8_t)(nr < BP_OUT_MAX ? nr : BP_OUT_MAX);
    for (i = 0; i < out->rp_n; i++) { out->rp_x[i] = g_rp_x[i]; out->rp_y[i] = g_rp_y[i]; }
#if BP_STAGE_TIMER
        STAGE_ACC(st_outer, s0);
#endif
    }

    /* ⑤ 帧航向角 (个数≥2 + 方差双门限) + 横向线性连通域透出 (渲染用) */
    {
        int ci;
        out->lin_n = 0;
        for (ci = 1; ci <= ncc && out->lin_n < BP_LIN_MAX; ci++) {
            uint8_t k;
            if (!cclin[ci]) continue;
            k = out->lin_n++;
            out->lin_cx[k] = cc_cx[ci];
            out->lin_cy[k] = cc_cy[ci];
            out->lin_ang[k] = ccang[ci];
            out->lin_pix[k] = ccn[ci];
        }
    }
    if (frame_heading(ncc, &hdg)) {
        out->hdg_valid = 1;
        out->hdg = hdg;
    }
    /* ⑥ (v3 删除) 边线时间验证: 边线质量由下游 IPM 物理 x 主带门 + 采信窗滤波负责 */

#if BP_DEBUG_FRAME
    {
        /* v3: 无 p85/无 mag²; T = BP_NORM_K·mean 直接打印; strong 计数在 ②循环内同步累计 */
        int cnt_horiz = 0;
        for (i = 0; i < PIX; i++) { cnt_horiz += (s->horiz[i] != 0); }
        printf("DBG T=%d strong=%d horiz=%d ncc=%d nl=%d nr=%d hdg=%.1f\r\n",
               (int)(((uint32_t)BP_NORM_K * img_sum) / PIX), dbg_cnt_strong, cnt_horiz, ncc, nl, nr, hdg);
    }
#endif
#if BP_STAGE_TIMER
    printf("STAGE conv=%lu strong=%lu ccl=%lu domain=%lu outer=%lu (cyc)\r\n",
           (unsigned long)st_conv, (unsigned long)st_strong, (unsigned long)st_ccl,
           (unsigned long)st_domain, (unsigned long)st_outer);
    st_conv = st_strong = st_ccl = st_domain = st_outer = 0;
#endif
}
