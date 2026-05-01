/*
 * =================================================================================
 * 文件: playgroud_line_detector.h
 * 作用: 1 核 (Core 1) 红色操场直线检测模块对外接口与配置定义
 * 说明: 本模块输入 94x60 灰度图，输出操场中线的检测/预测结果，并带时序稳定处理。
 * =================================================================================
 */
#ifndef PLAYGROUD_LINE_DETECTOR_H
#define PLAYGROUD_LINE_DETECTOR_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1) 模块总开关与基础尺寸 --- */
#define PLAYGROUD_LINE_DETECTOR_ENABLE                (1)
#define PLAYGROUD_LINE_DETECTOR_PROFILE_ENABLE        (1)
#define PLAYGROUD_LINE_DETECTOR_PROFILE_TIMER         (TC_TIME2_CH1)
#define PLAYGROUD_LINE_DETECTOR_DEBUG_PRINT_EVERY     (0U)

#define PLAYGROUD_IMAGE_W                             (94U)
#define PLAYGROUD_IMAGE_H                             (60U)
#define PLAYGROUD_IMAGE_SIZE                          (PLAYGROUD_IMAGE_W * PLAYGROUD_IMAGE_H)

/* --- 2) 算法参数（默认值与用户要求一致） --- */
#define PLAYGROUD_LINE_MAX_COMPONENTS                 (128U)
#define PLAYGROUD_LINE_MIN_DECISION_SCORE             (0.35f)
#define PLAYGROUD_LINE_MASK_QHI_PERCENT_X100          (88U)
#define PLAYGROUD_LINE_MASK_HI_THR_MIN                (148U)
#define PLAYGROUD_LINE_MASK_LOCAL_MEAN_RADIUS         (4U)
#define PLAYGROUD_LINE_MASK_LOCAL_CONTRAST_MIN        (2U)
#define PLAYGROUD_LINE_MIN_AREA                       (90U)
#define PLAYGROUD_LINE_MIN_HEIGHT                     (14U)
#define PLAYGROUD_LINE_MIN_FIT_ROWS                   (6U)

/* 时序参数（用户指定默认值） */
#define PLAYGROUD_LINE_DEFAULT_MAX_LOST               (30U)
#define PLAYGROUD_LINE_DEFAULT_SMOOTH_ALPHA           (0.45f)
#define PLAYGROUD_LINE_DEFAULT_MIN_TEMPORAL_SCORE     (0.20f)

/* --- 3) 时序模式定义 --- */
#define PLAYGROUD_LINE_MODE_LOST                      (0U) /* mode=0: lost */
#define PLAYGROUD_LINE_MODE_DETECTED                  (1U) /* mode=1: detected */
#define PLAYGROUD_LINE_MODE_PREDICTED                 (2U) /* mode=2: predicted */

/**
 * @brief 单帧检测结果（原始检测或时序融合后的结果都使用该结构）
 */
typedef struct
{
    uint8 detected;                 /* 当前结果是否可用: 1=可用, 0=不可用 */
    uint8 component_count;          /* 连通域总数 */
    uint8 candidate_count;          /* 通过尺寸筛选的候选数 */
    uint8 line_point_rows;          /* 用于拟合直线的有效行数 */
    uint8 bbox_xmin;                /* 包围框左边界（无效时为 0xFF） */
    uint8 bbox_ymin;                /* 包围框上边界（无效时为 0xFF） */
    uint8 bbox_xmax;                /* 包围框右边界（无效时为 0xFF） */
    uint8 bbox_ymax;                /* 包围框下边界（无效时为 0xFF） */
    float confidence;               /* 检测置信度 [0,1] */
    float centroid_x;               /* 连通域中心 x */
    float centroid_y;               /* 连通域中心 y */
    float line_x_bottom;            /* 拟合线在近处 y_bottom 的 x */
    float line_x_lookahead;         /* 拟合线在前瞻 y_lookahead 的 x */
    float line_yaw_deg;             /* 线方向角（度） */
    float lateral_error_px;         /* 相对图像中心横向误差（像素） */
    float temporal_score;           /* 本帧时序评分 */
} playgroud_line_detector_frame_result_t;

/**
 * @brief 对外输出（含原始结果、时序融合结果、模式与计数）
 */
typedef struct
{
    uint32 frame_id;                                    /* 已处理帧序号 */
    uint8 raw_detected;                                 /* 原始检测是否有效 */
    uint8 stable_detected;                              /* 时序融合后是否有效 */
    uint8 mode;                                         /* PLAYGROUD_LINE_MODE_* */
    uint8 accepted;                                     /* 本帧是否通过时序门控 */
    uint8 lost_count;                                   /* 当前连续丢失计数 */
    playgroud_line_detector_frame_result_t raw;         /* 原始检测结果 */
    playgroud_line_detector_frame_result_t stable;      /* 时序融合结果 */
} playgroud_line_detector_output_t;

/* --- 4) 全局输出与性能统计 --- */
extern volatile runtime_profiler_t g_playgroud_line_detector_cost_profiler;
extern volatile runtime_profiler_t g_playgroud_line_detector_frame_profiler;
extern volatile playgroud_line_detector_output_t g_playgroud_line_detector_output;
extern volatile uint8 g_playgroud_line_detector_output_write_busy;

/* --- 5) 对外接口 --- */
void playgroud_line_detector_init(void);
void playgroud_line_detector_reset_filter(void);
void playgroud_line_detector_set_temporal_params(uint8 max_lost, float smooth_alpha, float min_temporal_score);
const volatile playgroud_line_detector_output_t *playgroud_line_detector_get_output(void);
void playgroud_line_detector_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
