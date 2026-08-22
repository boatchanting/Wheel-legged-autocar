/*
 * =================================================================================
 * 文件: bridge_pvc_vision.h
 * 作用: 1 核 (Core 1) 单边桥专用 PVC 入口检测模块的对外接口与配置。
 * 说明: 从 pvc_vision.h 复制的独立实例（独立调参，符号全部加 bridge_ 前缀），
 *       仅被 bridge_fusion 的"准备进入"阶段调用，识别单边桥入口的白色 PVC 区域。
 *       与通用 pvc_vision 使用场景参数不同，故独立成模块，互不影响。
 * =================================================================================
 */
#ifndef BRIDGE_PVC_VISION_H
#define BRIDGE_PVC_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "../../code/config/sys_options.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * =================================================================================
 * 【模块边界】
 * - 本文件只负责"看"：看见白色的单边桥入口区域，估算入口距离与横向位置。
 * - 不直接控制车轮转向或电机减速（由 0 核控制层消费结果后决策）。
 * - 检测执行与输出读取都在 bridge_fusion 状态机的"准备进入"分支内完成。
 * =================================================================================
 */

/* --- 1. 图像尺寸定义 --- */
#define BRIDGE_PVC_IMAGE_W               (94U)   /* 图像宽度，单位：像素 */
#define BRIDGE_PVC_IMAGE_H               (60U)   /* 图像高度，单位：像素 */
#define BRIDGE_PVC_IMAGE_SIZE            (BRIDGE_PVC_IMAGE_W * BRIDGE_PVC_IMAGE_H) /* 图像总像素数 */
#define BRIDGE_PVC_VISION_PHY_INVALID_MM (32767) /* 物理坐标无效标记值（IPM查表失败或越界） */

/* --- 2. 功能开关与硬件配置 --- */
#define BRIDGE_PVC_VISION_ENABLE                 (1)     /* 模块总开关：1 为开启编译，0 为关闭 */
#define BRIDGE_PVC_VISION_PROFILE_ENABLE         (0)     /* 性能统计开关：1 为开启 */
#define BRIDGE_PVC_VISION_SMOOTH_ENABLE          (1)     /* 平滑开关：1 为开启（过滤突然闪烁的反光） */
#define BRIDGE_PVC_VISION_PROFILE_TIMER          (TC_TIME2_CH1) /* 测时间用的硬件定时器 */
#define BRIDGE_PVC_VISION_DEBUG_PRINT_EVERY      (0U)    /* 串口打印周期。0 是不打印 */

/* --- 3. 核心算法参数（单边桥入口独立调参） --- */
/* 亮度阈值：灰度值（0-255）大于等于这个数，才被认为是白色 */
#define BRIDGE_PVC_VISION_WHITE_THRESHOLD        (VISION_WHITE_THRESHOLD)

/* 尺寸门槛：过滤掉太小、太窄、太稀疏的"假 PVC" */
#define BRIDGE_PVC_VISION_MIN_AREA               (120)   /* 面积：白点少于 120 个不要 */
#define BRIDGE_PVC_VISION_MIN_WIDTH              (20)    /* 宽度：不够宽不要 */
#define BRIDGE_PVC_VISION_MIN_HEIGHT             (4)     /* 高度：不够高不要 */
#define BRIDGE_PVC_VISION_MIN_FILL_RATIO         (0.25f) /* 填充率：低于 25% 不要 */

/* 最终打分门槛：分数 >= 0.58 才算真正看到了 PVC */
#define BRIDGE_PVC_VISION_MIN_DECISION_SCORE     (0.58f)
#define BRIDGE_PVC_VISION_MAX_COMPONENTS         (32)   /* 内存限制：画面里最多允许找 32 块白斑 */
#define BRIDGE_PVC_VISION_BOTTOM_TARGET_ROWS     (12U)  /* 方向控制参考底部若干行 */

/* 稳定策略参数：防抖动 */
#define BRIDGE_PVC_VISION_CONFIRM_FRAMES         (3U)    /* 连续 3 帧都看到，才认为"看到了" */
#define BRIDGE_PVC_VISION_LOST_HOLD_FRAMES       (2U)    /* 偶尔 1、2 帧没看到，可以假装还看着 */

/* --- 4. 数据结构定义 --- */

/**
 * @brief 单帧的检测结果
 */
typedef struct
{
    uint8 detected;              /* 本帧有没有看到 PVC（1=看到，0=没看到） */
    uint8 component_count;       /* 画面里一共有多少块白斑 */
    uint8 candidate_count;       /* 有多少块白斑通过了尺寸门槛 */
    uint16 area;                 /* 最像 PVC 的那一块的面积 */
    uint8 bbox_xmin;             /* 包围框的左边位置 */
    uint8 bbox_ymin;             /* 包围框的上边位置 */
    uint8 bbox_xmax;             /* 包围框的右边位置 */
    uint8 bbox_ymax;             /* 包围框的下边位置 */
    uint8 entry_bottom_y;        /* PVC 最底下的行号（靠车越近行号越大，"最后结束线"） */
    uint8 entry_top_y;           /* PVC 最上面的行号 */
    float confidence;            /* 算法给它打的分数（满分 1.0） */
    float centroid_x;            /* PVC 的中心横坐标 */
    float centroid_y;            /* PVC 的中心纵坐标 */
    float fill_ratio;            /* 填充率（面积/包围框面积） */
    float mean_gray;             /* 平均亮度（255 是纯白） */
    int16 target_x_px_x100;      /* 入口目标中心横坐标（像素，放大 100 倍） */
    int16 steer_error_px_x100;   /* 控制用像素误差（目标中心 - 图像中心，放大 100 倍） */
    int16 forward_mm;            /* 估算离车还有多远（单位：毫米，-1表示不知道） */
    int16 lateral_mm;            /* 估算车偏离了中心多少（单位：毫米，正数偏右，负数偏左） */
    int16 phy_x_mm;              /* 基于 IPM 查表得到的真实物理 X 坐标（毫米） */
    int16 phy_y_mm;              /* 基于 IPM 查表得到的真实物理 Y 坐标（毫米） */
    int16 yaw_error_deg_x100;    /* 角度偏差（当前先不用，填 0） */
} bridge_pvc_vision_frame_result_t;

/**
 * @brief 整个模块最终对外的输出（包括原始结果和防抖处理后的结果）
 */
typedef struct
{
    uint32 frame_id;                 /* 处理了多少帧了（序号） */
    uint8 raw_detected;              /* 这一瞬间看没看到（容易抖） */
    uint8 stable_detected;           /* 经过防抖处理后，算不算稳定看到 */
    uint8 detected_streak;           /* 连续看到的帧数 */
    uint8 lost_streak;               /* 连续没看到的帧数 */
    bridge_pvc_vision_frame_result_t raw;   /* 这一瞬间的具体数据 */
    bridge_pvc_vision_frame_result_t stable;/* 防抖处理后的具体数据 */
} bridge_pvc_vision_output_t;

/* --- 5. 全局变量声明 --- */
extern volatile runtime_profiler_t g_bridge_pvc_vision_cost_profiler;     /* 测算算法花了多少微秒 */
extern volatile runtime_profiler_t g_bridge_pvc_vision_frame_profiler;    /* 测算两帧之间隔了多久 */
extern volatile bridge_pvc_vision_output_t g_bridge_pvc_vision_output;    /* 最终输出数据 */
extern volatile uint8 g_bridge_pvc_vision_output_write_busy;              /* 防冲突锁：正在写数据时变成 1 */

/* --- 6. 对外函数接口 --- */

/**
 * @brief 初始化单边桥专用 PVC 视觉模块
 */
void bridge_pvc_vision_init(void);

/**
 * @brief 重置防抖状态（每次重新进入项目时调用，忘掉过去的记忆）
 */
void bridge_pvc_vision_reset_filter(void);

/**
 * @brief 获取最终结果的指针
 */
const volatile bridge_pvc_vision_output_t *bridge_pvc_vision_get_output(void);

/**
 * @brief 处理摄像头送来的一张新照片
 * @param gray 压缩好的 94x60 的灰度照片数据
 */
void bridge_pvc_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
