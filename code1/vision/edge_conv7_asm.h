/**
 * ============================================================================
 * edge_conv7_asm.h  ——  7-tap 可分离卷积汇编内核接口
 * ============================================================================
 * 内核实现在 edge_conv7_asm.s (SMLAD/SMLAL, SELF_ITCM).
 * 输入 94x60 uint8 灰度 → Gx/Gy int32 (reflect 半采样边界由调用方铺 pad).
 * ============================================================================
 */
#ifndef _EDGE_CONV7_ASM_H_
#define _EDGE_CONV7_ASM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 水平 pass: row_pad[100] int16 (已 reflect pad, 4 字节对齐) → Gx_h[94]/Gy_h[94] int32 */
void conv7_horiz_row(const int16_t *p_row_pad, int32_t *p_gx_h, int32_t *p_gy_h,
                     uint32_t out_width);

/* 垂直 pass: col_pad[66] int32 (已 reflect pad) → out[60] int32
 * use_d: 0 → P 核 (Gx_h 列 → Gx); 非0 → D 核 (Gy_h 列 → Gy) */
void conv7_vert_col(const int32_t *p_col_pad, int32_t *p_out,
                    uint32_t out_height, uint32_t use_d);

#ifdef __cplusplus
}
#endif

#endif /* _EDGE_CONV7_ASM_H_ */
