#ifndef BRIDGE_VISION_H
#define BRIDGE_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "bridge_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BRIDGE_VISION_IMAGE_W                 (94U)
#define BRIDGE_VISION_IMAGE_H                 (60U)
#define BRIDGE_VISION_CONFIRM_FRAMES          (1U)
#define BRIDGE_VISION_LOST_HOLD_FRAMES        (5U)

/* 车辆部署设置：每帧仅使用一个阈值。设置为 -1 则恢复为检测器的自适应多阈值模式。自适应模式比较稳定，现阶段占用两倍算力，没有做多帧记忆优化 */
#define BRIDGE_VISION_FIXED_THRESHOLD          (200)
#define BRIDGE_VISION_PROFILE_TIMER           (TC_TIME2_CH1)

/* 检测器评分校准基准约为 350。IPC 置信度仍保持 0.0 .. 1.0 的归一化值，
 * 以便 Core 0 维持其现有的通信数据格式。 */
#define BRIDGE_VISION_SCORE_FULL_SCALE         (500.0f)

typedef struct
{
    uint8 detected;                 /* 有效的桥中心线几何信息 */
    uint8 bridge_detected;          /* 桥候选目标通过了检测器门限 */
    uint8 state;                    /* 桥检测状态 (BridgeDetectionState) */
    uint8 geometry_valid;           /* 几何信息有效 */
    uint8 left_line_visible;        /* 左侧线可见 */
    uint8 right_line_visible;       /* 右侧线可见 */
    uint8 top_line_visible;         /* 顶部线可见 */
    uint8 entry_line_visible;       /* 入口线可见 */
    uint8 bbox_xmin;                /* 边界框左上角 X 坐标 */
    uint8 bbox_ymin;                /* 边界框左上角 Y 坐标 */
    uint8 bbox_xmax;                /* 边界框右下角 X 坐标 */
    uint8 bbox_ymax;                /* 边界框右下角 Y 坐标 */
    uint16 area;                    /* 面积 */
    float confidence;               /* 置信度 */
    float bridge_confidence;        /* 桥置信度 */
    float lateral_error_px;         /* 横向误差（像素） */
    float yaw_error_deg;            /* 偏航角误差（度） */
    float center_x;                 /* 中心 X 坐标 */
    uint8 center_x0;                /* 中心线起点 X 坐标 */
    uint8 center_y0;                /* 中心线起点 Y 坐标 */
    uint8 center_x1;                /* 中心线终点 X 坐标 */
    uint8 center_y1;                /* 中心线终点 Y 坐标 */
    float line_x_bottom;            /* 底部线 X 坐标 */
    float line_x_lookahead;         /* 前瞻线 X 坐标 */
    float candidate_score;          /* 候选目标评分 */
    float edge_contrast;            /* 边缘对比度 */
} bridge_vision_frame_result_t;

typedef struct
{
    uint32 frame_id;                /* 帧ID */
    uint8 raw_detected;             /* 原始检测结果 */
    uint8 stable_detected;          /* 稳定检测结果 */
    uint8 bridge_raw_detected;      /* 桥原始检测结果 */
    uint8 bridge_stable_detected;   /* 桥稳定检测结果 */
    uint8 detected_streak;          /* 连续检测成功帧数 */
    uint8 lost_streak;              /* 连续丢失帧数 */
    uint8 bridge_detected_streak;   /* 桥连续检测成功帧数 */
    uint8 bridge_lost_streak;       /* 桥连续丢失帧数 */
    bridge_vision_frame_result_t raw;    /* 原始帧结果 */
    bridge_vision_frame_result_t stable; /* 稳定帧结果 */
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
