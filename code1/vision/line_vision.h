/*
 * =================================================================================
 * 文件: line_vision.h
 * 作用: 1 核 (Core 1) 桥梁/直线视觉检测模块的配置与接口。
 * 说明: 这个模块负责在桥梁任务中寻找地上的直线（赛道边缘或引导线），
 *       同时也会寻找桥上的黑色区域（也就是桥梁的实体特征）。
 *       这里定义了找直线和找桥梁用的所有门槛（阈值）和输出的数据结构。
 * =================================================================================
 */
#ifndef LINE_VISION_H
#define LINE_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "pvc_vision.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1. 图像尺寸定义 --- */
/* 和 PVC 模块一样，这里处理的也是压缩后的 94x60 小图，速度快 */
#define LINE_IMAGE_W                         (PVC_IMAGE_W)
#define LINE_IMAGE_H                         (PVC_IMAGE_H)
#define LINE_IMAGE_SIZE                      (LINE_IMAGE_W * LINE_IMAGE_H)

/* --- 2. 功能开关与硬件配置 --- */
#define LINE_VISION_ENABLE                   (1)     /* 模块总开关：1 为开启，0 为关闭 */
#define LINE_VISION_PROFILE_ENABLE           (1)     /* 性能统计开关：用来测算代码跑了多久 */
#define LINE_VISION_PROFILE_TIMER            (PVC_VISION_PROFILE_TIMER) /* 复用同一个定时器 */
#define LINE_VISION_DEBUG_PRINT_EVERY        (0U)    /* 串口打印周期，0 为不打印 */

/* --- 3. 找直线的核心参数（调直线循迹时看这里） --- */
#define LINE_VISION_ROI_TOP_RATIO_X100       (25U)   /* 感兴趣区域（ROI）顶部：从照片上往下 25% 开始看，上面太远的不看 */
#define LINE_VISION_MIN_ROWS                 (15U)   /* 最少要找到 15 行白线，少于这个就不算找到直线 */
#define LINE_VISION_MIN_WIDTH                (8U)    /* 线的最窄像素要求，太细的可能是反光点 */
#define LINE_VISION_MIN_Y_SPAN               (22U)   /* 找到的线在竖直方向至少要跨越 22 行像素 */
#define LINE_VISION_MAX_ABS_YAW_DEG          (35.0f) /* 直线偏角太大（比如 > 35度）就认为找错成横线了，不要 */
#define LINE_VISION_MIN_CONFIDENCE           (0.72f) /* 直线打分门槛，大于 0.72 算真正找到 */

/* --- 4. 找桥梁的核心参数（调找桥时看这里） --- */
/* 注意：桥梁一般是深色/黑色的，所以找桥其实是在找“黑块” */
#define LINE_VISION_BRIDGE_DARK_THRESHOLD    (180U)  /* 黑块的亮度门槛：灰度值小于 180 算作桥梁上的黑块 */
#define LINE_VISION_BRIDGE_MIN_AREA          (35U)   /* 黑块最少要包含 35 个像素点 */
#define LINE_VISION_BRIDGE_MIN_WIDTH         (18U)   /* 黑块的最小宽度 */
#define LINE_VISION_BRIDGE_MIN_HEIGHT        (4U)    /* 黑块的最小高度 */
#define LINE_VISION_BRIDGE_MIN_FILL_RATIO    (0.22f) /* 黑块不能太碎，填充率至少 22% */
#define LINE_VISION_BRIDGE_MIN_CONFIDENCE    (0.56f) /* 桥梁打分门槛，大于 0.56 算看到桥 */
#define LINE_VISION_BRIDGE_SPEED_HINT        (-90.0f)/* 看到桥的时候，建议给 0 核发送什么速度指示？(-90 可能是减速标记) */

/* --- 5. 稳定策略参数（防抖动） --- */
#define LINE_VISION_CONFIRM_FRAMES           (2U)    /* 直线：连续看清 2 帧才算数 */
#define LINE_VISION_LOST_HOLD_FRAMES         (3U)    /* 直线：偶尔 3 帧看不清也先假装看着 */
#define LINE_VISION_BRIDGE_CONFIRM_FRAMES    (1U)    /* 桥梁：只要看清 1 帧就算看到了桥（因为上桥很关键，不能漏） */
#define LINE_VISION_BRIDGE_LOST_HOLD_FRAMES  (5U)    /* 桥梁：5 帧看不清才算彻底离开桥 */

/* --- 6. 数据结构定义 --- */

/**
 * @brief 单帧照片里，找直线和找桥梁的结果
 */
typedef struct
{
    uint8 detected;              /* 看到直线了吗？(1=看到，0=没看到) */
    uint8 bridge_detected;       /* 看到桥梁了吗？(1=看到，0=没看到) */
    uint8 bridge_component_count;/* 找到了几个像桥的黑块 */
    uint8 points_used;           /* 拟合这条直线用到了多少个点 */
    uint8 y_span;                /* 这条直线在照片里上下跨越了多少行 */
    uint8 bridge_bbox_xmin;      /* 桥梁黑块包围框的左边界 */
    uint8 bridge_bbox_ymin;      /* 桥梁黑块包围框的上边界 */
    uint8 bridge_bbox_xmax;      /* 桥梁黑块包围框的右边界 */
    uint8 bridge_bbox_ymax;      /* 桥梁黑块包围框的下边界 */
    float confidence;            /* 直线的综合打分 */
    float bridge_confidence;     /* 桥梁的综合打分 */
    float lateral_error_px;      /* 车偏离直线的横向偏差（单位：像素。正=偏右，负=偏左） */
    float yaw_error_deg;         /* 车头偏离直线的角度（单位：度） */
    float line_x_bottom;         /* 直线在照片最底下的位置（代表车头前的偏差） */
    float line_x_lookahead;      /* 直线在照片中间靠上的位置（代表远处的偏差） */
    float fit_rmse;              /* 直线拟合的误差（越小说明线越直） */
    float mean_track_width;      /* 赛道的平均宽度 */
    float roi_white_ratio;       /* 关注区域里的白色比例（判断是不是大片反光） */
    float target_speed_hint;     /* 视觉给控制层的建议速度 */
} line_vision_frame_result_t;

/**
 * @brief 最终对外的输出（包括原始结果和防抖处理后的稳定结果）
 */
typedef struct
{
    uint32 frame_id;                 /* 处理了多少帧照片了 */
    uint8 raw_detected;              /* 这一瞬间看没看到直线 */
    uint8 stable_detected;           /* 经过防抖后，稳定看到直线了吗（0核用这个） */
    uint8 bridge_raw_detected;       /* 这一瞬间看没看到桥梁 */
    uint8 bridge_stable_detected;    /* 经过防抖后，稳定看到桥了吗（0核用这个） */
    uint8 detected_streak;           /* 连续看到直线的帧数 */
    uint8 lost_streak;               /* 连续没看到直线的帧数 */
    uint8 bridge_detected_streak;    /* 连续看到桥的帧数 */
    uint8 bridge_lost_streak;        /* 连续没看到桥的帧数 */
    line_vision_frame_result_t raw;  /* 单帧的原始数据 */
    line_vision_frame_result_t stable;/* 防抖后的稳定数据 */
} line_vision_output_t;

/* --- 7. 对外公开的全局变量与函数 --- */

extern volatile runtime_profiler_t g_line_vision_cost_profiler;  /* 算一帧要花多久 */
extern volatile runtime_profiler_t g_line_vision_frame_profiler; /* 照片来的有多快 */
extern volatile line_vision_output_t g_line_vision_output;       /* 最终的输出结果 */
extern volatile uint8 g_line_vision_output_write_busy;           /* 写数据时的防冲突锁 */

/**
 * @brief 初始化直线/桥梁视觉模块
 */
void line_vision_init(void);

/**
 * @brief 重置防抖状态（比如退出桥梁任务后调用，忘掉以前的记录）
 */
void line_vision_reset_filter(void);

/**
 * @brief 获取最终的检测结果指针
 */
const volatile line_vision_output_t *line_vision_get_output(void);

/**
 * @brief 处理一张新的灰度照片
 * @param gray 压缩后的 94x60 小图数据
 */
void line_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
