/*
 * =================================================================================
 * 文件: pvc_vision.c
 * 作用: 1 核 (Core 1) PVC 入口视觉检测模块 (RLE 游程编码版)。
 * 说明: 使用游程编码替代 Flood Fill 连通域提取。
 *       参考: D:\WORKS\2026TUsmart-Gited\new-vision\project\code\cc_extract.c
 * =================================================================================
 */
#include "pvc_vision.h"
#include "ipm_transform.h"

#if PVC_VISION_ENABLE

/* --- 1. RLE 游程编码数据结构 --- */
typedef struct { uint8 x0; uint8 x1; uint8 blob_idx; uint8 skip_cnt; } pvc_run_t;
typedef struct { uint8 y; uint8 xmin; uint8 xmax; } pvc_bottom_row_t;
typedef struct {
    uint32 area, sum_x, sum_y, sum_gray;
    uint8  min_x, max_x, min_y, max_y, row_count, touches_border;
    uint16 bbox_area; float score; uint8 last_rows;
    pvc_bottom_row_t bottom_rows[16]; uint8 is_valid;
} pvc_blob_t;

/* --- 2. 全局变量 (对外公开) --- */
volatile runtime_profiler_t g_pvc_vision_cost_profiler = {0};
volatile runtime_profiler_t g_pvc_vision_frame_profiler = {0};
volatile pvc_vision_output_t g_pvc_vision_output = {0};
volatile uint8 g_pvc_vision_output_write_busy = 0U;

/* --- 3. 内部全局变量 (RLE 游程缓冲 + blob 池) --- */
static pvc_run_t  g_runs_buf0[60];
static pvc_run_t  g_runs_buf1[60];
static pvc_run_t  g_runs_buf2[60];
static pvc_blob_t g_blob_pool[PVC_VISION_MAX_COMPONENTS];
static pvc_vision_output_t g_pvc_output_shadow;
static uint8  g_pvc_smooth_inited = 0U;
static int16  g_pvc_smooth_forward_mm = -1;
static int16  g_pvc_smooth_lateral_mm = 0;
static uint32 g_pvc_last_frame_time_us = 0U;

/* --- 4. 基础数学工具函数 --- */
static float pvc_min_f(float a, float b) { return (a < b) ? a : b; }
static float pvc_max_f(float a, float b) { return (a > b) ? a : b; }

static int16 pvc_float_to_i16_x100(float value)
{
    if (value > 327.67f) return 32767;
    if (value < -327.68f) return -32768;
    return (int16)(value * 100.0f);
}

/* --- 5. 底部行目标提取 (从 RLE 环形缓冲读取, 替代二次扫描) --- */
static float pvc_extract_target_x_from_blob(const pvc_blob_t *blob)
{
    float weighted_sum = 0.0f, weight_total = 0.0f;
    uint8 count, y_min = 255, i;
    count = (blob->last_rows < PVC_VISION_BOTTOM_TARGET_ROWS)
          ? blob->last_rows : PVC_VISION_BOTTOM_TARGET_ROWS;
    if (count == 0U) return (float)(blob->sum_x / blob->area);
    for (i = 0U; i < count; i++) {
        uint8 idx = (uint8)((blob->last_rows - 1U - i) & 0x0Fu);
        uint8 y = blob->bottom_rows[idx].y;
        if (y < y_min) y_min = y;
    }
    for (i = 0U; i < count; i++) {
        uint8 idx = (uint8)((blob->last_rows - 1U - i) & 0x0Fu);
        uint8 y = blob->bottom_rows[idx].y;
        float row_center_x = ((float)blob->bottom_rows[idx].xmin
                            + (float)blob->bottom_rows[idx].xmax) * 0.5f;
        float weight = 1.0f + 0.08f * (float)(int)(y - y_min);
        weighted_sum += row_center_x * weight; weight_total += weight;
    }
    if (weight_total > 0.0f) return weighted_sum / weight_total;
    return (float)(blob->sum_x / blob->area);
}

/* --- 6. 距离估算 (不变) --- */
static int16 pvc_estimate_forward_mm_from_row(uint8 row)
{ return (int16)((PVC_IMAGE_H - 1U - row) * 20U); }
static int16 pvc_estimate_lateral_mm_from_x(float x)
{ return (int16)((x - ((float)PVC_IMAGE_W - 1.0f) * 0.5f) * 8.0f); }

/* --- 7. IPM 物理坐标 (适配新参数签名) --- */
static void pvc_fill_physical_coord_from_ipm(float centroid_x, uint8 ymax,
                                              pvc_vision_frame_result_t *result)
{
    uint8 img_x; IPM_Point_t ipm_point;
    if (centroid_x <= 0.0f) img_x = 0U;
    else if (centroid_x >= (float)(PVC_IMAGE_W - 1U)) img_x = (uint8)(PVC_IMAGE_W - 1U);
    else img_x = (uint8)(centroid_x + 0.5f);
    ipm_point = IPM_GetPhysicalCoord(img_x, ymax);
    if (ipm_point.is_valid) { result->phy_x_mm = ipm_point.x_mm; result->phy_y_mm = ipm_point.y_mm; }
    else { result->phy_x_mm = PVC_VISION_PHY_INVALID_MM; result->phy_y_mm = PVC_VISION_PHY_INVALID_MM; }
}

/* --- 8. 结果清理 + 打分 --- */
static void pvc_clear_frame_result(pvc_vision_frame_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->bbox_xmin = 0xFFU; result->bbox_ymin = 0xFFU;
    result->bbox_xmax = 0xFFU; result->bbox_ymax = 0xFFU;
    result->entry_bottom_y = 0xFFU; result->entry_top_y = 0xFFU;
    result->target_x_px_x100 = 0; result->steer_error_px_x100 = 0;
    result->phy_x_mm = PVC_VISION_PHY_INVALID_MM;
    result->phy_y_mm = PVC_VISION_PHY_INVALID_MM;
    result->forward_mm = -1;
}

static float pvc_score_blob(const pvc_blob_t *blob, float fill_ratio, float mean_gray)
{
    uint8 w = (uint8)(blob->max_x - blob->min_x + 1U);
    uint8 h = (uint8)(blob->max_y - blob->min_y + 1U);
    return 0.38f * pvc_min_f((float)blob->area / 600.0f, 1.0f)
         + 0.20f * pvc_min_f((float)w / 45.0f, 1.0f)
         + 0.16f * pvc_min_f((float)h / 18.0f, 1.0f)
         + 0.16f * pvc_min_f(fill_ratio / 0.55f, 1.0f)
         + 0.10f * pvc_min_f(pvc_max_f((mean_gray - 235.0f) / 20.0f, 0.0f), 1.0f);
}

/* --- 9. RLE 游程核心 --- */
#define PVC_RLE_SKIP_LINES_MAX  3U
#define PVC_RLE_MERGE_GAP_MAX  10U

static uint8 pvc_scan_row(const uint8 *gray_row, uint8 threshold,
                          pvc_run_t *out_runs, uint8 max_runs)
{
    uint8 run_count = 0U, x = 0U;
    while (x < PVC_IMAGE_W) {
        while (x < PVC_IMAGE_W && gray_row[x] < threshold) x++;
        if (x >= PVC_IMAGE_W) break;
        uint8 x0 = x;
        while (x < PVC_IMAGE_W && gray_row[x] >= threshold) x++;
        if (run_count < max_runs) {
            out_runs[run_count].x0 = x0; out_runs[run_count].x1 = (uint8)(x - 1U);
            out_runs[run_count].blob_idx = 0xFFU; out_runs[run_count].skip_cnt = 0U;
            run_count++;
        }
    }
    return run_count;
}

static void pvc_merge_rows(pvc_run_t *curr_runs, uint8 curr_count,
                           pvc_run_t *prev_runs, uint8 *prev_count,
                           pvc_run_t *next_runs, uint8 *next_count,
                           pvc_blob_t *blob_pool, uint8 max_blobs,
                           uint8 *next_blob_idx, uint8 y, const uint8 *gray_row)
{
    uint8 next_prev_count = 0U, i, j; (void)max_blobs;
    if (curr_count == 0U) {
        for (j = 0U; j < *prev_count; j++)
            if (prev_runs[j].skip_cnt < PVC_RLE_SKIP_LINES_MAX) {
                prev_runs[j].skip_cnt++;
                if (next_prev_count < 60U) next_runs[next_prev_count++] = prev_runs[j];
            }
        *prev_count = next_prev_count; *next_count = next_prev_count; return;
    }
    i = 0U; j = 0U;
    while (i < curr_count && j < *prev_count) {
        uint8 cs = curr_runs[i].x0, ce = curr_runs[i].x1;
        uint8 ps = prev_runs[j].x0, pe = prev_runs[j].x1;
        uint8 os = (cs > ps) ? cs : ps, oe = (ce < pe) ? ce : pe;
        uint8 ov = (oe >= os) ? (uint8)(oe - os + 1U) : 0U;
        uint8 cl = (uint8)(ce - cs + 1U);
        uint8 inh = (uint8)((ov > 0U) && (ov > (uint8)(cl >> 1)));
        uint8 pc;
        if (pe < cs) { pc = 1U; }
        else if (ce < ps) { i++; continue; }
        else if (inh) { if (curr_runs[i].blob_idx == 0xFFU) curr_runs[i].blob_idx = prev_runs[j].blob_idx; pc = (uint8)(ce >= pe); }
        else { pc = (uint8)(ce >= pe); }
        if (pc) {
            if (!inh && prev_runs[j].skip_cnt < PVC_RLE_SKIP_LINES_MAX)
                { prev_runs[j].skip_cnt++; if (next_prev_count < 60U) next_runs[next_prev_count++] = prev_runs[j]; }
            j++;
        } else i++;
    }
    while (j < *prev_count) {
        if (prev_runs[j].skip_cnt < PVC_RLE_SKIP_LINES_MAX) {
            prev_runs[j].skip_cnt++;
            if (next_prev_count < 60U) next_runs[next_prev_count++] = prev_runs[j];
        } j++;
    }
    for (i = 0U; i < curr_count; i++) {
        if (curr_runs[i].blob_idx != 0xFFU) continue;
        if (*next_blob_idx < PVC_VISION_MAX_COMPONENTS) {
            uint8 bidx = *next_blob_idx; curr_runs[i].blob_idx = bidx;
            pvc_blob_t *b = &blob_pool[bidx]; memset(b, 0, sizeof(*b));
            b->is_valid = 1U; b->min_x = curr_runs[i].x0; b->max_x = curr_runs[i].x1;
            b->min_y = y; b->max_y = y; (*next_blob_idx)++;
        }
    }
    { uint8 wi = 0U;
    for (i = 0U; i < curr_count; i++) {
        uint8 ds = 0U;
        if (wi > 0U) {
            int gap = (int)curr_runs[i].x0 - (int)curr_runs[wi - 1U].x1 - 1;
            ds = (uint8)((curr_runs[wi - 1U].blob_idx == curr_runs[i].blob_idx)
                  && (curr_runs[i].blob_idx != 0xFFU) && (gap <= (int)PVC_RLE_MERGE_GAP_MAX));
        }
        if (ds) curr_runs[wi - 1U].x1 = curr_runs[i].x1;
        else if (wi < 60U) curr_runs[wi++] = curr_runs[i];
    } curr_count = wi; }
    for (i = 0U; i < curr_count; i++) {
        uint8 bidx = curr_runs[i].blob_idx; if (bidx == 0xFFU) continue;
        pvc_blob_t *b = &blob_pool[bidx];
        uint8 rl = (uint8)(curr_runs[i].x1 - curr_runs[i].x0 + 1U);
        uint8 x0 = curr_runs[i].x0, x1 = curr_runs[i].x1, x;
        b->area += rl; b->sum_x += (uint32)(x0 + x1) * rl / 2U;
        b->sum_y += (uint32)y * rl;
        for (x = x0; x <= x1; x++) b->sum_gray += gray_row[x];
        if (x0 < b->min_x) b->min_x = x0; if (x1 > b->max_x) b->max_x = x1; b->max_y = y;
        if (!b->touches_border)
            if (x0 == 0U || x1 == (PVC_IMAGE_W - 1U) || y == 0U || y == (PVC_IMAGE_H - 1U))
                b->touches_border = 1U;
        { uint8 ri = (uint8)(b->last_rows & 0x0FU);
          b->bottom_rows[ri].y = y; b->bottom_rows[ri].xmin = x0; b->bottom_rows[ri].xmax = x1;
          b->last_rows++; } b->row_count++;
    }
    for (i = 0U; i < curr_count; i++) {
        if (curr_runs[i].blob_idx == 0xFFU) continue;
        curr_runs[i].skip_cnt = 0U;
        if (next_prev_count < 60U) next_runs[next_prev_count++] = curr_runs[i];
    }
    *prev_count = next_prev_count; *next_count = next_prev_count;
}

static void pvc_swap_buffers(pvc_run_t **c, uint8 *cc, pvc_run_t **p, uint8 *pc,
                             pvc_run_t **n, uint8 *nc)
{ pvc_run_t *t = *p; uint8 tc = *pc; *p = *c; *pc = *cc; *c = *n; *cc = 0U; *n = t; *nc = tc; }

static uint8 pvc_extract_level(const uint8 gray[PVC_IMAGE_H][PVC_IMAGE_W],
                               uint8 threshold, pvc_blob_t *pool, uint8 max_blobs)
{
    uint8 cc = 0U, pc = 0U, nc = 0U, nbi = 0U, y;
    pvc_run_t *cb = g_runs_buf0, *pb = g_runs_buf1, *nb = g_runs_buf2;
    memset(pool, 0, (size_t)max_blobs * sizeof(pvc_blob_t));
    for (y = 0U; y < PVC_IMAGE_H; y++) {
        cc = pvc_scan_row(gray[y], threshold, cb, 60U);
        pvc_merge_rows(cb, cc, pb, &pc, nb, &nc, pool, max_blobs, &nbi, y, gray[y]);
        pvc_swap_buffers(&cb, &cc, &pb, &pc, &nb, &nc); pc = nc; nc = 0U;
    }
    { uint8 oc = 0U, i;
    for (i = 0U; i < nbi; i++) {
        if (!pool[i].is_valid) continue;
        pvc_blob_t *b = &pool[i]; uint8 w = (uint8)(b->max_x - b->min_x + 1U);
        uint8 h = (uint8)(b->max_y - b->min_y + 1U);
        if (b->area < (uint32)PVC_VISION_MIN_AREA) continue;
        if (w < PVC_VISION_MIN_WIDTH || h < PVC_VISION_MIN_HEIGHT) continue;
        if (oc < i) pool[oc] = *b; oc++;
    } return oc; }
}

static void pvc_blob_finalize(pvc_blob_t *pool, uint8 count)
{
    uint8 i; for (i = 0U; i < count; i++) {
        if (!pool[i].is_valid || pool[i].area == 0U) continue;
        pool[i].bbox_area = (uint16)(pool[i].max_x - pool[i].min_x + 1U)
                          * (uint16)(pool[i].max_y - pool[i].min_y + 1U);
    }
}

/* --- 10. 主检测流程 (RLE 游程版) --- */
static void pvc_detect_frame(const uint8 *gray, pvc_vision_frame_result_t *result)
{
    uint8 blob_count, candidate_count = 0U, i;
    pvc_blob_t *best = NULL; float best_score = 0.0f;
    blob_count = pvc_extract_level((const uint8(*)[PVC_IMAGE_W])gray,
                                    PVC_VISION_WHITE_THRESHOLD,
                                    g_blob_pool, PVC_VISION_MAX_COMPONENTS);
    pvc_blob_finalize(g_blob_pool, blob_count);
    pvc_clear_frame_result(result);
    result->component_count = blob_count;
    for (i = 0U; i < blob_count; i++) {
        pvc_blob_t *b = &g_blob_pool[i]; float fr, mg, sc;
        uint8 w, h; if (!b->is_valid || b->area == 0U) continue;
        w = (uint8)(b->max_x - b->min_x + 1U); h = (uint8)(b->max_y - b->min_y + 1U);
        if (b->area < (uint32)PVC_VISION_MIN_AREA) continue;
        if (w < PVC_VISION_MIN_WIDTH || h < PVC_VISION_MIN_HEIGHT) continue;
        fr = (b->bbox_area > 0U) ? (float)b->area / (float)b->bbox_area : 0.0f;
        if (fr < PVC_VISION_MIN_FILL_RATIO) continue;
        mg = (float)b->sum_gray / (float)b->area;
        sc = pvc_score_blob(b, fr, mg); candidate_count++;
        if (sc > best_score) { best_score = sc; best = b; b->score = sc; }
    }
    result->candidate_count = candidate_count;
    if (best != NULL) {
        float cx = (float)best->sum_x / (float)best->area;
        float cy = (float)best->sum_y / (float)best->area;
        float fr = (best->bbox_area > 0U) ? (float)best->area / (float)best->bbox_area : 0.0f;
        float mg = (float)best->sum_gray / (float)best->area;
        float tx = pvc_extract_target_x_from_blob(best);
        float se = tx - (((float)PVC_IMAGE_W - 1.0f) * 0.5f);
        result->detected = (uint8)(best_score >= PVC_VISION_MIN_DECISION_SCORE);
        result->area = (uint16)best->area; result->bbox_xmin = best->min_x;
        result->bbox_ymin = best->min_y; result->bbox_xmax = best->max_x;
        result->bbox_ymax = best->max_y; result->entry_bottom_y = best->max_y;
        result->entry_top_y = best->min_y; result->confidence = best_score;
        result->centroid_x = cx; result->centroid_y = cy;
        result->fill_ratio = fr; result->mean_gray = mg;
        result->target_x_px_x100 = pvc_float_to_i16_x100(tx);
        result->steer_error_px_x100 = pvc_float_to_i16_x100(se);
        pvc_fill_physical_coord_from_ipm(cx, best->max_y, result);
        result->forward_mm = pvc_estimate_forward_mm_from_row(best->max_y);
        result->lateral_mm = pvc_estimate_lateral_mm_from_x(tx);
        result->yaw_error_deg_x100 = 0;
        if (result->detected == 0U) result->forward_mm = -1;
    }
}

/* --- 11. 防抖过滤器 (不变) --- */
static void pvc_update_filter(const pvc_vision_frame_result_t *raw)
{
    pvc_vision_output_t next = g_pvc_output_shadow;
    next.frame_id++; next.raw = *raw; next.raw_detected = raw->detected;
#if PVC_VISION_SMOOTH_ENABLE
    if (raw->detected) {
        if (next.detected_streak < 255U) next.detected_streak++;
        next.lost_streak = 0U;
        if (g_pvc_smooth_inited == 0U) {
            g_pvc_smooth_forward_mm = raw->forward_mm;
            g_pvc_smooth_lateral_mm = raw->lateral_mm; g_pvc_smooth_inited = 1U;
        } else {
            g_pvc_smooth_forward_mm = (int16)((3 * g_pvc_smooth_forward_mm + raw->forward_mm) / 4);
            g_pvc_smooth_lateral_mm = (int16)((3 * g_pvc_smooth_lateral_mm + raw->lateral_mm) / 4);
        }
        if (next.detected_streak >= PVC_VISION_CONFIRM_FRAMES) next.stable_detected = 1U;
    } else {
        next.detected_streak = 0U;
        if (next.lost_streak < 255U) next.lost_streak++;
        if (next.lost_streak >= PVC_VISION_LOST_HOLD_FRAMES)
        { next.stable_detected = 0U; g_pvc_smooth_inited = 0U; }
    }
    if (next.stable_detected) {
        if (raw->detected) next.stable = *raw;
        next.stable.detected = 1U;
        next.stable.forward_mm = g_pvc_smooth_forward_mm;
        next.stable.lateral_mm = g_pvc_smooth_lateral_mm;
    } else { pvc_clear_frame_result(&next.stable); }
#else
    next.detected_streak = raw->detected ? 1U : 0U;
    next.lost_streak = raw->detected ? 0U : 1U;
    next.stable_detected = raw->detected; next.stable = *raw;
#endif
    g_pvc_output_shadow = next;
    g_pvc_vision_output_write_busy = 1U;
    g_pvc_vision_output = next;
    g_pvc_vision_output_write_busy = 0U;
}

/* --- 12. 对外公开的函数接口 (不变) --- */
void pvc_vision_init(void)
{
    pvc_vision_reset_filter();
#if PVC_VISION_PROFILE_ENABLE
    timer_init(PVC_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(PVC_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_pvc_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_pvc_vision_frame_profiler);
    g_pvc_last_frame_time_us = timer_get(PVC_VISION_PROFILE_TIMER);
#endif
}

void pvc_vision_reset_filter(void)
{
    pvc_vision_output_t empty; memset(&empty, 0, sizeof(empty));
    pvc_clear_frame_result(&empty.raw); pvc_clear_frame_result(&empty.stable);
    g_pvc_output_shadow = empty;
    g_pvc_vision_output_write_busy = 1U;
    g_pvc_vision_output = empty;
    g_pvc_vision_output_write_busy = 0U;
    g_pvc_smooth_inited = 0U; g_pvc_smooth_forward_mm = -1; g_pvc_smooth_lateral_mm = 0;
}

const volatile pvc_vision_output_t *pvc_vision_get_output(void)
{ return &g_pvc_vision_output; }

void pvc_vision_process_camera_frame(const uint8 *gray)
{
    pvc_vision_frame_result_t raw;
    if (gray == NULL) return;
#if PVC_VISION_PROFILE_ENABLE
    { const uint32 now_us = timer_get(PVC_VISION_PROFILE_TIMER);
      runtime_profiler_update(&g_pvc_vision_frame_profiler, (uint32)(now_us - g_pvc_last_frame_time_us));
      g_pvc_last_frame_time_us = now_us; }
    RUNTIME_PROFILE_BEGIN(g_pvc_vision_cost_profiler, PVC_VISION_PROFILE_TIMER);
#endif
    pvc_detect_frame(gray, &raw);
    pvc_update_filter(&raw);
#if PVC_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_pvc_vision_cost_profiler, PVC_VISION_PROFILE_TIMER);
#endif
#if (PVC_VISION_DEBUG_PRINT_EVERY > 0U)
    if ((g_pvc_vision_output.frame_id % PVC_VISION_DEBUG_PRINT_EVERY) == 0U) {
        printf("[PVC] frame=%lu raw=%u stable=%u score=%d cost=%lu us frame_dt=%lu us\r\n",
               (unsigned long)g_pvc_vision_output.frame_id, g_pvc_vision_output.raw_detected,
               g_pvc_vision_output.stable_detected,
               (int)(g_pvc_vision_output.raw.confidence * 1000.0f),
               (unsigned long)g_pvc_vision_cost_profiler.last_us,
               (unsigned long)g_pvc_vision_frame_profiler.last_us);
    }
#endif
}

#endif
