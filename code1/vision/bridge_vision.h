#ifndef BRIDGE_VISION_H
#define BRIDGE_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "bridge_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BRIDGE_VISION_IMAGE_W          (94U)
#define BRIDGE_VISION_IMAGE_H          (60U)
#define BRIDGE_VISION_CONFIRM_FRAMES   (1U)
#define BRIDGE_VISION_LOST_HOLD_FRAMES (5U)
#define BRIDGE_VISION_FIXED_THRESHOLD  (200)/* 车辆部署设置：每帧仅使用一个阈值。设置为 -1 则恢复为检测器的自适应多阈值模式。自适应模式比较稳定，现阶段占用两倍算力，没有做多帧记忆优化 */
#define BRIDGE_VISION_PROFILE_TIMER    (TC_TIME2_CH1)
#define BRIDGE_VISION_COORD_INVALID    ((int16)-1)

typedef struct
{
    uint8 detected;                 /* 是否拿到了可直接用于控制的桥几何。1 表示中线坐标有效，0 表示中线无效。 */
    uint8 bridge_detected;          /* 是否检测到桥候选目标并通过门限。1 表示当前帧认为桥存在，0 表示桥候选不成立。 */
    uint8 state;                    /* 桥检测状态机输出，取值见 BridgeDetectionState。只有 bridge_detected=1 时才有实际意义。 */
    uint8 geometry_valid;           /* 几何信息是否完整可用。1 表示至少中线有效，0 表示下面所有坐标都应按无效值处理。 */

    int16 left_line_x0;             /* 左线起点 X 坐标。无效时固定为 -1。 */
    int16 left_line_y0;             /* 左线起点 Y 坐标。无效时固定为 -1。 */
    int16 left_line_x1;             /* 左线终点 X 坐标。无效时固定为 -1。 */
    int16 left_line_y1;             /* 左线终点 Y 坐标。无效时固定为 -1。 */

    int16 right_line_x0;            /* 右线起点 X 坐标。无效时固定为 -1。 */
    int16 right_line_y0;            /* 右线起点 Y 坐标。无效时固定为 -1。 */
    int16 right_line_x1;            /* 右线终点 X 坐标。无效时固定为 -1。 */
    int16 right_line_y1;            /* 右线终点 Y 坐标。无效时固定为 -1。 */

    int16 down_line_x0;             /* 下线起点 X 坐标。这里的“下线”对应靠近车头一侧的横线，来自检测器的 entry line。无效时固定为 -1。 */
    int16 down_line_y0;             /* 下线起点 Y 坐标。无效时固定为 -1。 */
    int16 down_line_x1;             /* 下线终点 X 坐标。无效时固定为 -1。 */
    int16 down_line_y1;             /* 下线终点 Y 坐标。无效时固定为 -1。 */

    int16 up_line_x0;               /* 上线起点 X 坐标。这里的“上线”对应桥远端上沿线，来自检测器的 top line。无效时固定为 -1。 */
    int16 up_line_y0;               /* 上线起点 Y 坐标。无效时固定为 -1。 */
    int16 up_line_x1;               /* 上线终点 X 坐标。无效时固定为 -1。 */
    int16 up_line_y1;               /* 上线终点 Y 坐标。无效时固定为 -1。 */

    int16 center_line_x0;           /* 中线起点 X 坐标。中线由左右边线共同推导，只有 detected=1 时才可信；无效时固定为 -1。 */
    int16 center_line_y0;           /* 中线起点 Y 坐标。无效时固定为 -1。 */
    int16 center_line_x1;           /* 中线终点 X 坐标。通常更靠近图像下方，是控制更关心的端点；无效时固定为 -1。 */
    int16 center_line_y1;           /* 中线终点 Y 坐标。无效时固定为 -1。 */
} bridge_vision_frame_result_t;

typedef struct
{
    uint32 frame_id;                /* 帧号，每处理一帧加一。 */
    uint8 raw_detected;             /* 原始几何检测结果，直接等于 raw.detected。 */
    uint8 stable_detected;          /* 经过简单连续帧滤波后的几何检测结果，供控制优先使用。 */
    uint8 bridge_raw_detected;      /* 原始桥存在判定，直接等于 raw.bridge_detected。 */
    uint8 bridge_stable_detected;   /* 经过简单连续帧滤波后的桥存在判定。 */
    uint8 detected_streak;          /* 连续检测到有效中线的帧数。 */
    uint8 lost_streak;              /* 连续丢失有效中线的帧数。 */
    uint8 bridge_detected_streak;   /* 连续检测到桥候选成立的帧数。 */
    uint8 bridge_lost_streak;       /* 连续桥候选不成立的帧数。 */
    bridge_vision_frame_result_t raw;    /* 当前帧的原始输出。 */
    bridge_vision_frame_result_t stable; /* 经过连续帧滤波后保留下来的稳定输出。 */
} bridge_vision_output_t;

extern volatile runtime_profiler_t g_bridge_vision_cost_profiler;
extern volatile runtime_profiler_t g_bridge_vision_frame_profiler;
extern volatile bridge_vision_output_t g_bridge_vision_output;
extern volatile uint8 g_bridge_vision_output_write_busy;

void bridge_vision_init(void);
void bridge_vision_reset_filter(void);
const volatile bridge_vision_output_t *bridge_vision_get_output(void);
void bridge_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
