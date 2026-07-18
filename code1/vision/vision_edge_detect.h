/**
 * @file    vision_edge_detect.h
 * @brief   边缘方向检测 — 颠簸路段视觉导航 (极简封装)
 * @details 基于 4x4 可分离卷积核的边缘方向检测。
 *          输入 96×60 uint8 灰度图像，输出是否在颠簸路段 + 方向向量。
 *
 *          对外接口仅两个:
 *            EdgeDetect_Init()     — 初始化
 *            EdgeDetect_Process()  — 输入图像 → 输出 {is_bumpy, dir_x, dir_y}
 *
 *          内核由 edge_conv_asm.s 提供 (ITCM 零等待取指)，
 *          环形缓冲由 DTCM_BSS 分配 (DTCM 零等待数据访问)。
 *
 * @date    2026-07-18
 */

#ifndef VISION_EDGE_DETECT_H
#define VISION_EDGE_DETECT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * 对外输出结构体
 * ========================================================================== */
typedef struct {
    uint8_t  is_bumpy;    /* 颠簸路段: 强边缘数>128 且 R²>0.81 (R>0.9)  */
    float  dir_x;       /* 方向向量 X 分量 (归一化, 范围 [-1, 1])       */
    float  dir_y;       /* 方向向量 Y 分量 (归一化, 范围 [-1, 1])       */
} edge_detect_output_t;


/* ==========================================================================
 * 对外 API (仅两个函数)
 * ========================================================================== */

/**
 * @brief 初始化边缘检测模块
 */
void EdgeDetect_Init(void);

/**
 * @brief 处理一帧图像，输出颠簸路段判定 + 方向向量
 * @param gray  输入灰度图像 (uint8[120][188], 行优先)
 * @param out   输出: is_bumpy + 方向向量
 * @note  耗时约 8.2ms @125MHz
 */
void EdgeDetect_Process(const uint8_t *gray, edge_detect_output_t *out);


#ifdef __cplusplus
}
#endif

#endif /* VISION_EDGE_DETECT_H */
