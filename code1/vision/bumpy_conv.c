/**
 * ============================================================================
 * bumpy_conv.c  ——  7-tap 可分离卷积骨架 (MCU 工程版)
 * ============================================================================
 * reflect 边界 (半采样对称) pad 铺陈 + 调度汇编内核:
 *   水平: 每行 uint8 → int16 展开 → row_pad[100] → conv7_horiz_row → Gx_h/Gy_h
 *   垂直: 每列 int32 → col_pad[66] → conv7_vert_col → Gx/Gy
 * 内核实现在 edge_conv7_asm.s (SMLAD/SMLAL), 语义与 bumpy_conv_ref.c 逐位一致.
 * ============================================================================
 */
#include "bumpy_conv.h"
#include "edge_conv7_asm.h"

/* numpy/scipy reflect (半采样): i<0 -> -i-1 ; i>=n -> 2n-1-i */
static int ref_idx(int i, int n)
{
    if (i < 0) return -i - 1;
    if (i >= n) return 2 * n - 1 - i;
    return i;
}

/* 工作缓冲 (常规 SRAM; 汇编内循环 LDR 需 4 字节对齐) */
#if defined(__ICCARM__)
    #define ALIGN4 _Pragma("data_alignment=4")
#else
    #define ALIGN4 __attribute__((aligned(4)))
#endif

static ALIGN4 int16_t row_pad[BUMPY_W + 6];                       /* 100 */
static int32_t gxh_row[BUMPY_W];                                  /* 94 */
static int32_t gyh_row[BUMPY_W];
static int32_t col_pad[BUMPY_H + 6];                              /* 66 */
static int32_t vout[BUMPY_H];                                     /* 垂直单列输出 */

void bumpy_conv7(const uint8_t *img, int32_t *gx, int32_t *gy, int32_t *scratch)
{
    int32_t *gxh = scratch;                 /* 水平中间结果 (调用方提供, 2×BUMPY_PIX) */
    int32_t *gyh = scratch + BUMPY_PIX;
    int y, x, j;

    /* ---- 水平 pass ---- */
    for (y = 0; y < BUMPY_H; y++) {
        const uint8_t *src = img + y * BUMPY_W;
        for (j = 0; j < BUMPY_W + 6; j++) {
            int idx = j - 3;
            idx = ref_idx(idx, BUMPY_W);
            row_pad[j] = (int16_t)src[idx];
        }
        conv7_horiz_row(row_pad, gxh_row, gyh_row, BUMPY_W);
        for (x = 0; x < BUMPY_W; x++) {
            gxh[y * BUMPY_W + x] = gxh_row[x];
            gyh[y * BUMPY_W + x] = gyh_row[x];
        }
    }

    /* ---- 垂直 pass ---- */
    for (x = 0; x < BUMPY_W; x++) {
        for (j = 0; j < BUMPY_H + 6; j++) {
            int idx = j - 3;
            idx = ref_idx(idx, BUMPY_H);
            col_pad[j] = gxh[idx * BUMPY_W + x];   /* Gx_h 列 */
        }
        conv7_vert_col(col_pad, vout, BUMPY_H, 0);       /* P 核 → Gx */
        for (y = 0; y < BUMPY_H; y++) gx[y * BUMPY_W + x] = vout[y];
        for (j = 0; j < BUMPY_H + 6; j++) {
            int idx = j - 3;
            idx = ref_idx(idx, BUMPY_H);
            col_pad[j] = gyh[idx * BUMPY_W + x];   /* Gy_h 列 */
        }
        conv7_vert_col(col_pad, vout, BUMPY_H, 1);       /* D 核 → Gy */
        for (y = 0; y < BUMPY_H; y++) gy[y * BUMPY_W + x] = vout[y];
    }
}
