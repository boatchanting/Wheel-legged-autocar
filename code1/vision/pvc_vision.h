/*
 * =================================================================================
 * 文件: pvc_vision.h
 * 作用: 1 核 (Core 1) PVC 入口检测模块的对外接口与配置。
 * 说明: 这个文件就像是 PVC 视觉模块的“说明书”和“遥控器”。
 *       里面定义了怎么调参、输出什么结果，以及供其他模块调用的函数。
 * =================================================================================
 */
#ifndef PVC_VISION_H
#define PVC_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * =================================================================================
 * 【新手必读：PVC 入口视觉检测模块说明】
 * =================================================================================
 *
 * 模块边界（这个模块做什么，不做什么）：
 * - 本文件只负责“看”：看见白色的 PVC 入口区域，并估算入口距离车有多远、偏左还是偏右。
 * - 本文件不负责“开”：不直接控制车轮转向或电机减速。
 * - 0 核的控制层会来读取这里算出的结果，然后再决定车该怎么开。
 *
 * 推荐使用流程：
 * 1. 惯导或路径状态机判断小车进入某个项目入口约 800mm 内（也就是快到 PVC 入口了）。
 * 2. 0 核告诉 1 核：“请开启 PVC 检测”。
 * 3. 1 核摄像头每拍到一帧画面：
 *    - 先把 188x120 的原图缩小（压缩）成 94x60。
 *    - 把 94x60 的小图交给本模块处理。
 * 4. 1 核定时把稳定的结果（比如偏差多少、距离多远）发送给 0 核。
 * 5. 0 核根据结果计算方向盘打多少度、车速该降到多少。
 *
 * 初始调参建议（车在赛道上跑不好的时候看这里）：
 * - 如果现场太暗，PVC 识别不到：把下面的 `PVC_VISION_WHITE_THRESHOLD` 从 245 降到 235~240（降低白色门槛）。
 * - 如果地上的反光点被误认成了 PVC：把 `PVC_VISION_MIN_AREA` 变大（要求面积更大才算），或者把连续确认帧数 `PVC_VISION_CONFIRM_FRAMES` 变大。
 * - 如果车反应太慢：把确认帧数 `PVC_VISION_CONFIRM_FRAMES` 从 3 降到 2。
 * - 如果图像轻微抖动导致识别断断续续：把允许丢失的帧数 `PVC_VISION_LOST_HOLD_FRAMES` 提高。
 * =================================================================================
 */

/* --- 1. 图像尺寸定义 --- */
/* 为了让计算更快，算法不处理原图，而是处理缩小后的图 */
#define PVC_IMAGE_W                       (94U)   /* 图像宽度，单位：像素 */
#define PVC_IMAGE_H                       (60U)   /* 图像高度，单位：像素 */
#define PVC_IMAGE_SIZE                    (PVC_IMAGE_W * PVC_IMAGE_H) /* 图像总像素数 */
#define PVC_VISION_PHY_INVALID_MM         (32767) /* 物理坐标无效标记值（IPM查表失败或越界） */

/* --- 2. 功能开关与硬件配置 --- */
#define PVC_VISION_ENABLE                 (1)     /* 模块总开关：1 为开启编译，0 为关闭。遇到问题可以关掉排查 */
#define PVC_VISION_PROFILE_ENABLE         (0)     /* 性能统计开关：1 为开启。开启后可以看算一帧要多久 */
#define PVC_VISION_SMOOTH_ENABLE          (1)     /* 平滑开关：1 为开启。开启后可以过滤掉突然闪烁的反光，让结果更稳定 */
#define PVC_VISION_PROFILE_TIMER          (TC_TIME2_CH1) /* 测时间用的硬件定时器。别和遥控器冲突 */
#define PVC_VISION_DEBUG_PRINT_EVERY      (0U)    /* 串口打印周期。0 是不打印；50 表示每 50 帧在电脑上打印一次信息 */

/* --- 3. 核心算法参数（调车时最常改的地方） --- */
/*
 * 亮度阈值：灰度值（0-255）大于等于这个数，才被认为是白色的 PVC
 * 室内灯光亮就用 245，阴天或暗处用 235
 */
#define PVC_VISION_WHITE_THRESHOLD        (200U)

/*
 * 尺寸门槛：过滤掉太小、太窄、太稀疏的“假 PVC”
 */
#define PVC_VISION_MIN_AREA               (120)   /* 面积：白点少于 120 个不要 */
#define PVC_VISION_MIN_WIDTH              (12)    /* 宽度：不够宽不要 */
#define PVC_VISION_MIN_HEIGHT             (4)     /* 高度：不够高不要 */
#define PVC_VISION_MIN_FILL_RATIO         (0.25f) /* 填充率：如果是零零散散的白点（不到 25%）也不要 */

/*
 * 最终打分门槛：算法会给候选的 PVC 打分，分数 >= 0.58 才算真正看到了 PVC
 */
#define PVC_VISION_MIN_DECISION_SCORE     (0.58f)
#define PVC_VISION_MAX_COMPONENTS         (32)   /* 内存限制：画面里最多允许找 32 块白斑 */
#define PVC_VISION_BOTTOM_TARGET_ROWS     (12U)  /* 方向控制参考底部若干行，学习颠簸路段的目标点提取方式 */

/*
 * 稳定策略参数：防抖动
 */
#define PVC_VISION_CONFIRM_FRAMES         (3U)    /* 连续 3 帧都看到，才向 0 核汇报“看到了” */
#define PVC_VISION_LOST_HOLD_FRAMES       (2U)    /* 偶尔 1、2 帧没看到，可以假装还看着，防止短时间闪烁导致停车 */

/* --- 4. 数据结构定义 --- */

/**
 * @brief 单帧的检测结果（记录了一张照片里的 PVC 长啥样）
 */
typedef struct
{
    uint8 detected;              /* 本帧有没有看到 PVC（1=看到，0=没看到） */
    uint8 component_count;       /* 画面里一共有多少块白斑（用来看看噪声多不多） */
    uint8 candidate_count;       /* 有多少块白斑通过了尺寸门槛（有潜力成为 PVC） */
    uint16 area;                 /* 最像 PVC 的那一块的面积 */
    uint8 bbox_xmin;             /* 包围框的左边位置 */
    uint8 bbox_ymin;             /* 包围框的上边位置 */
    uint8 bbox_xmax;             /* 包围框的右边位置 */
    uint8 bbox_ymax;             /* 包围框的下边位置 */
    uint8 entry_bottom_y;        /* PVC 最底下的行号（靠车越近行号越大） */
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
} pvc_vision_frame_result_t;

/**
 * @brief 整个模块最终对外的输出（包括原始结果和防抖处理后的结果）
 */
typedef struct
{
    uint32 frame_id;                 /* 处理了多少帧了（序号） */
    uint8 raw_detected;              /* 这一瞬间看没看到（容易抖） */
    uint8 stable_detected;           /* 经过防抖处理后，算不算稳定看到（0 核用这个） */
    uint8 detected_streak;           /* 连续看到的帧数 */
    uint8 lost_streak;               /* 连续没看到的帧数 */
    pvc_vision_frame_result_t raw;   /* 这一瞬间的具体数据 */
    pvc_vision_frame_result_t stable;/* 防抖处理后的具体数据（0 核用这个） */
} pvc_vision_output_t;

/* --- 5. 全局变量声明 --- */
extern volatile runtime_profiler_t g_pvc_vision_cost_profiler;     /* 测算算法花了多少微秒 */
extern volatile runtime_profiler_t g_pvc_vision_frame_profiler;    /* 测算两帧之间隔了多久 */
extern volatile pvc_vision_output_t g_pvc_vision_output;           /* 最终要交出去的作业（数据） */
extern volatile uint8 g_pvc_vision_output_write_busy;              /* 防冲突锁：正在写数据时变成 1，别人别来读 */

/* --- 6. 对外函数接口 --- */

/**
 * @brief 初始化 PVC 视觉模块
 */
void pvc_vision_init(void);

/**
 * @brief 重置防抖状态（每次重新进入项目时调用，忘掉过去的记忆）
 */
void pvc_vision_reset_filter(void);

/**
 * @brief 获取最终结果的指针
 * @return const volatile pvc_vision_output_t* 指向结果的指针
 */
const volatile pvc_vision_output_t *pvc_vision_get_output(void);

/**
 * @brief 处理摄像头送来的一张新照片
 * @param gray 压缩好的 94x60 的黑白照片数据
 */
void pvc_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
