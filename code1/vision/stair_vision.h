/**
 * @file    stair_vision.h
 * @brief   台阶检测视觉模块 — Core 1 算法封装层 (V10)
 * @details 基于 V10 台阶检测算法 (Gx/Gy 边缘卷积 + 三阶段后处理)。
 *          输入 120×188 uint8 灰度图像，输出台阶几何信息。
 *
 *          【重要】本模块是纯算法层，不涉及跨核 IPC 通信。
 *
 *          === 输出字段说明 ===
 *          result.mid_x/y    上峰横线中点 (y = peak_y)
 *          result.mid2_x/y   下峰横线中点 (y = peak2_y)
 *          result.edge_span  横线水平跨度
 *          result.peak_y     上峰行号 (小y, 远离机器人)
 *          result.peak2_y    下峰行号 (大y, 靠近机器人)
 *          result.crease_y   折痕行号
 *          result.crease_span 双峰间距
 *          result.gy_max_x/y/val  |Gy|全局最大
 *
 * @date    2026-07-24
 */

#ifndef STAIR_VISION_H
#define STAIR_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "v10_stair_postprocess.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ==========================================================================
 * 编译开关与配置
 * ========================================================================== */
#define STAIR_VISION_ENABLE                 (1)
#define STAIR_VISION_PROFILE_ENABLE         (1)
#define STAIR_VISION_PROFILE_TIMER          (TC_TIME2_CH2)
#define STAIR_VISION_CONFIRM_FRAMES         (2U)
#define STAIR_VISION_LOST_HOLD_FRAMES       (3U)

#define STAIR_IMAGE_W                       (V10_IMG_COLS)  /* 188 */
#define STAIR_IMAGE_H                       (V10_IMG_ROWS)  /* 120 */


/* ==========================================================================
 * 对外输出结构体
 * ========================================================================== */
typedef struct
{
    uint32 frame_id;
    uint8  detected;
    uint8  raw_detected;
    uint8  detected_streak;
    uint8  lost_streak;
    v10_stair_result_t result;
} stair_vision_output_t;


/* ==========================================================================
 * 全局状态
 * ========================================================================== */
extern volatile runtime_profiler_t g_stair_vision_cost_profiler;
extern volatile runtime_profiler_t g_stair_vision_frame_profiler;
extern volatile stair_vision_output_t g_stair_vision_output;
extern volatile uint8 g_stair_vision_output_write_busy;


/* ==========================================================================
 * 对外 API
 * ========================================================================== */
void stair_vision_init(void);
void stair_vision_reset_filter(void);
const volatile stair_vision_output_t *stair_vision_get_output(void);
void stair_vision_process_camera_frame(const uint8 *gray);


#ifdef __cplusplus
}
#endif

#endif /* STAIR_VISION_H */
