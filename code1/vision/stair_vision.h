#ifndef STAIR_VISION_H
#define STAIR_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "v9_stair_conv_asm.h"
#include "v9_stair_postprocess.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STAIR_VISION_IMAGE_W          (94U)
#define STAIR_VISION_IMAGE_H          (60U)
#define STAIR_VISION_CONFIRM_FRAMES   (2U)
#define STAIR_VISION_LOST_HOLD_FRAMES (3U)
#define STAIR_VISION_PROFILE_TIMER    (TC_TIME2_CH2)

typedef struct
{
    uint8  detected;            /* has_stairs, 1 表示检测到台阶 */
    float  joint_score;         /* 联合判别分数 */
    float  left_rho;            /* 左边界 ρ (px) */
    float  left_theta;          /* 左边界 θ (rad) */
    float  right_rho;           /* 右边界 ρ (px) */
    float  right_theta;         /* 右边界 θ (rad) */
    float  center_a;            /* 中线 a (ax+by+c=0) */
    float  center_b;            /* 中线 b */
    float  center_c;            /* 中线 c */
    int16  crease_y;            /* 上方尖峰行号 (0=图像顶部), -1=无效 */
    int16  crease_span;         /* 双峰间距 (px) */
} stair_vision_frame_result_t;

typedef struct
{
    uint32 frame_id;                /* 帧号 */
    uint8  raw_detected;            /* 原始检测结果 */
    uint8  stable_detected;         /* 连续帧滤波后的稳定结果 */
    uint8  detected_streak;         /* 连续检测到台阶的帧数 */
    uint8  lost_streak;             /* 连续丢失的帧数 */
    stair_vision_frame_result_t raw;
    stair_vision_frame_result_t stable;
} stair_vision_output_t;

extern volatile runtime_profiler_t g_stair_vision_cost_profiler;
extern volatile runtime_profiler_t g_stair_vision_frame_profiler;
extern volatile stair_vision_output_t g_stair_vision_output;
extern volatile uint8 g_stair_vision_output_write_busy;

void stair_vision_init(void);
void stair_vision_reset_filter(void);
const volatile stair_vision_output_t *stair_vision_get_output(void);
void stair_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
