/**
 * @file    bumpy_vision.h
 * @brief   边缘方向检测 — 颠簸路段视觉导航 (极简封装)
 * @details 基于 4x4 可分离卷积核的边缘方向检测。
 *          输入 96×60 uint8 灰度图像，输出是否在颠簸路段 + 方向向量。
 *
 *          对外接口仅两个:
 *            EdgeDetect_Init()     — 初始化
 *            EdgeDetect_Process()  — 输入图像 → 输出 {is_bumpy, dir_x, dir_y}
 *
 *          内核由 edge_conv_asm.c 提供 (ITCM 零等待取指)，
 *          环形缓冲由 DTCM_BSS 分配 (DTCM 零等待数据访问)。
 *
 * @date    2026-07-18
 */

#ifndef BUMPY_VISION_H
#define BUMPY_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "pvc_vision.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * 对外输出结构体
 * ========================================================================== */
typedef struct {
    uint8_t is_bumpy;
    float dir_x;
    float dir_y;
    float coherence_r;
    uint32_t strong_count;
    uint32_t total_pixels;
    uint16_t max_gradient_mag;
} bumpy_edge_detect_output_t;


/* ==========================================================================
 * 对外 API (仅两个函数)
 * ========================================================================== */

/**
 * @brief 初始化边缘检测模块
 */
void bumpy_vision_init(void);

/**
 * @brief 处理一帧图像，输出颠簸路段判定 + 方向向量
 * @param gray  输入灰度图像 (uint8[120][188], 行优先)
 * @param out   输出: is_bumpy + 方向向量
 * @note  耗时约 8.2ms @125MHz
 */
#define BUMPY_VISION_ENABLE                 (1)
#define BUMPY_VISION_PROFILE_ENABLE         (1)
#define BUMPY_VISION_PROFILE_TIMER          (TC_TIME2_CH0)
#define BUMPY_IMAGE_W                       (188)
#define BUMPY_IMAGE_H                       (120)
#define BUMPY_IMAGE_SIZE                    (BUMPY_IMAGE_W * BUMPY_IMAGE_H)
#define BUMPY_ROI_Y0                        (0U)
#define BUMPY_ROI_Y1                        (BUMPY_IMAGE_H - 1U)

typedef struct
{
    uint32 frame_id;
    uint8 bumpy_detected;
    float direction_x;
    float direction_y;
    float coherence_r;
    uint32_t strong_count;
    uint32_t total_pixels;
    uint16_t max_gradient_mag;
} bumpy_vision_output_t;

extern volatile runtime_profiler_t g_bumpy_vision_cost_profiler;
extern volatile runtime_profiler_t g_bumpy_vision_frame_profiler;
extern volatile bumpy_vision_output_t g_bumpy_vision_output;
extern volatile uint8 g_bumpy_vision_output_write_busy;

void bumpy_vision_reset_filter(void);
const volatile bumpy_vision_output_t *bumpy_vision_get_output(void);
void bumpy_vision_process_camera_frame(const uint8 *gray);


#ifdef __cplusplus
}
#endif

#endif /* BUMPY_VISION_H */
