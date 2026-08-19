/**
 * ============================================================================
 * bumpy_conv.h  ——  7-tap 可分离卷积接口 (bumpy 颠簸路方向场)
 * ============================================================================
 * 核: D=[1,14,115,0,-115,-14,-1] (7-tap 差分, FS7 最优整数核)
 *     P=[2,13,49,75,49,13,2]     (7-tap 平滑, FS7 最优整数核)
 * gx = 水平 D ⊗ 垂直 P ;  gy = 水平 P ⊗ 垂直 D
 *
 * 输入 94x60 uint8 灰度 → 输出 94x60 int32 Gx/Gy (reflect 边界, 同 Python 参考)
 *
 * MCU 侧由 edge_conv7_asm.s 汇编实现 (SMLAD), PC 侧由 bumpy_conv_ref.c 提供
 * 语义等价 C 实现; int32 全精度 (max |G|≈255*260=66300 << 2^31).
 *
 * RAM 紧缩 (2026-08-17): 水平 pass 全帧中间结果 gxh/gyh (2×BUMPY_PIX int32)
 * 不再静态分配, 由调用方经 scratch 传入 (MCU 侧复用 bumpy_pipeline 的 mag2 区,
 * 卷积期间该区空闲; 见 bumpy_pipeline.c 顶部缓冲时分复用说明).
 * ============================================================================
 */
#ifndef _BUMPY_CONV_H_
#define _BUMPY_CONV_H_

#include <stdint.h>

#define BUMPY_W   94
#define BUMPY_H   60
#define BUMPY_PIX (BUMPY_W * BUMPY_H)

#ifdef __cplusplus
extern "C" {
#endif

/* img: BUMPY_W*BUMPY_H uint8 行优先; gy: BUMPY_PIX int32 行优先输出 (仅垂直梯度, v3 gy-only);
   scratch: 1*BUMPY_PIX int32 工作区 (水平 pass 全帧中间结果 gyh) */
void bumpy_conv7_gy(const uint8_t *img, int32_t *gy, int32_t *scratch);

#ifdef __cplusplus
}
#endif

#endif /* _BUMPY_CONV_H_ */
