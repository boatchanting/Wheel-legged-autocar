/**
 * @file    vision_edge_detect.c
 * @brief   边缘方向检测 — 颠簸路段视觉导航 (极简封装实现)
 * @details 完整边缘检测流水线，全部内部状态自包含。
 *          对外仅暴露 EdgeDetect_Init() 和 EdgeDetect_Process()。
 *
 * @date    2026-07-18
 */

#include "vision_edge_detect.h"
#include "tcm.h"
#include "edge_conv_asm.h"
#include <math.h>
#include <string.h>

/* ==========================================================================
 * 内部常量 (模块自包含, 不暴露到头文件)
 * ========================================================================== */
#define EDGE_IMAGE_W        (188U)
#define EDGE_IMAGE_H        (120U)
#define EDGE_OUT_W          (185U)   /* W-3 */
#define EDGE_RING_DEPTH     (4U)

#define EDGE_FIXED_THR      (1500)   /* 固定阈值: |Gx|+|Gy| >= thr */
#define EDGE_R_SQ_BUMPY     (0.81f)  /* R² > 0.81 (对应 R > 0.9)     */
#define EDGE_MIN_STRONG_N   (128U)   /* 强边缘数 > 128 才判定        */

/* ==========================================================================
 * DTCM 环形缓冲 (模块私有, 0 等待数据访问)
 * ========================================================================== */
DTCM_BSS static int16_t  edge_gx_ring[EDGE_RING_DEPTH][EDGE_OUT_W];
DTCM_BSS static int16_t  edge_gy_ring[EDGE_RING_DEPTH][EDGE_OUT_W];
DTCM_BSS static int16_t  edge_row_buf[EDGE_IMAGE_W];


/* ==========================================================================
 * 内部辅助
 * ========================================================================== */
static float edge_sqrtf(float x)
{
    return sqrtf(x);
}


/* ==========================================================================
 * API 实现
 * ========================================================================== */

void EdgeDetect_Init(void)
{
    memset((void *)edge_gx_ring, 0, sizeof(edge_gx_ring));
    memset((void *)edge_gy_ring, 0, sizeof(edge_gy_ring));
    memset((void *)edge_row_buf,  0, sizeof(edge_row_buf));
}

void EdgeDetect_Process(const uint8_t *gray, edge_detect_output_t *out)
{
    edge_dir_result_t edge_accum;
    int ring_idx;
    int row;
    float r_sq, norm;

    /* ---- 初始化累加器 ---- */
    edge_accum.sum_gx       = 0;
    edge_accum.sum_gy       = 0;
    edge_accum.strong_count = 0;
    edge_accum.total_pixels = 0;
    ring_idx = 0;

    /* ---- 逐行流水线 ---- */
    for (row = 0; row < EDGE_IMAGE_H; row++)
    {
        const uint8_t *src_row = &gray[row * EDGE_IMAGE_W];
        int x;

        /* 1) uint8 → int16 展开 */
        for (x = 0; x < EDGE_IMAGE_W; x++)
        {
            edge_row_buf[x] = (int16_t)src_row[x];
        }

        /* 2) 水平 pass */
        conv1d_horiz_gxgy(
            edge_row_buf,
            edge_gx_ring[ring_idx],
            edge_gy_ring[ring_idx],
            EDGE_OUT_W);

        /* 3) 积累 4 行后触发垂直 pass + 方向累加 */
        if (row >= 3)
        {
            int i0 = ring_idx;
            int i1 = (ring_idx + 1) & 3;
            int i2 = (ring_idx + 2) & 3;
            int i3 = (ring_idx + 3) & 3;

            conv1d_vert_gxgy_row(
                edge_gx_ring[i0], edge_gx_ring[i1],
                edge_gx_ring[i2], edge_gx_ring[i3],
                edge_gy_ring[i0], edge_gy_ring[i1],
                edge_gy_ring[i2], edge_gy_ring[i3],
                edge_gx_ring[i0],
                edge_gy_ring[i0],
                EDGE_OUT_W);

            gradient_mag_dir_fixed(
                edge_gx_ring[i0],
                edge_gy_ring[i0],
                &edge_accum,
                EDGE_FIXED_THR,
                EDGE_OUT_W);

            edge_accum.total_pixels += EDGE_OUT_W;
        }

        ring_idx = (ring_idx + 1) & 3;
    }

    /* ---- 计算 R² 和方向向量 ---- */
    if (edge_accum.strong_count > 0U)
    {
        r_sq = (float)((int64_t)edge_accum.sum_gx * edge_accum.sum_gx
                     + (int64_t)edge_accum.sum_gy * edge_accum.sum_gy)
             / ((float)edge_accum.strong_count * (float)edge_accum.strong_count);

        /* 方向向量归一化 */
        norm = edge_sqrtf((float)((int64_t)edge_accum.sum_gx * edge_accum.sum_gx
                                + (int64_t)edge_accum.sum_gy * edge_accum.sum_gy));
        if (norm > 0.0f)
        {
            out->dir_x = (float)edge_accum.sum_gx / norm;
            out->dir_y = (float)edge_accum.sum_gy / norm;
        }
        else
        {
            out->dir_x = 1.0f;
            out->dir_y = 0.0f;
        }
    }
    else
    {
        r_sq = 0.0f;
        out->dir_x = 1.0f;
        out->dir_y = 0.0f;
    }

    /* ---- 判定颠簸路段: N > 128 且 R² > 0.81 ---- */
    out->is_bumpy = (edge_accum.strong_count > EDGE_MIN_STRONG_N
                     && r_sq > EDGE_R_SQ_BUMPY) ? 1U : 0U;
}
