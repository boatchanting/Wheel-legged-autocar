/*
 * 文件: pvc_vision.h
 * 作用: 1 核 PVC 入口检测模块的对外接口与配置。
 */
#ifndef PVC_VISION_H
#define PVC_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PVC 入口视觉检测模块
 *
 * 模块边界：
 * - 本文件只做“看见 PVC 白色入口区域，并估计入口相对车的前向距离和横向偏差”。
 * - 本文件不直接写 err_degree、target_speed_set，也不直接让车停车。
 * - 0 核控制层读取 IPC 回传结果后，再决定转向、前进、减速、停车。
 *
 * 推荐使用流程：
 * 1. 惯导或路径状态机判断小车进入某个项目入口约 800mm 内。
 * 2. 0 核调用 VisionPvcControl_SetEnable(1)，通过 2ms IPC 命令让 1 核开启 PVC 检测。
 * 3. 1 核摄像头每来一帧：
 *    - 复制 mt9v03x_image，避免图像撕裂。
 *    - 调用 compress_image_to_target() 压缩到 94x60。
 *    - 调用 pvc_vision_process_camera_frame(compressed_image_copy[0])。
 * 4. 1 核 2ms 中断把 g_pvc_vision_output 中的 stable 摘要发布到共享内存。
 * 5. 0 核 2ms 中断读取共享内存，用 VisionPvcControl_Update_2ms() 生成：
 *    - err_degree：横向偏差换算出的转向角误差。
 *    - target_speed_set：搜索、跟踪、接近、到达时的目标速度。
 *
 * 到达区域的第一版判断：
 * - 当 PVC 稳定检测到，且白色区域的包围框面积超过整幅 94x60 图像的 90% 时，
 *   0 核控制层认为车已经进入/压到 PVC 区域，target_speed_set 置 0。
 * - 这个 90% 阈值在 code/vision/vision_pvc_control.h 里调：
 *   VISION_PVC_CONTROL_STOP_BBOX_RATIO_U16，900 表示 90.0%。
 *
 * 当前算法：
 * - 灰度阈值提取高亮白色 PVC 区域。
 * - 4 邻域连通域搜索，把相邻白色像素聚成连通域。
 * - 按面积、宽高、填充率、是否触边、平均亮度打分。
 * - 连续多帧确认后输出 stable_detected，避免单帧反光误触发。
 *
 * 初始调参建议：
 * - 如果现场曝光偏暗，PVC 经常识别不到：先把 PVC_VISION_WHITE_THRESHOLD 从 245 降到 235~240。
 * - 如果反光点误识别：提高 PVC_VISION_MIN_AREA，或把 PVC_VISION_CONFIRM_FRAMES 从 3 提到 4。
 * - 如果响应太慢：把 PVC_VISION_CONFIRM_FRAMES 从 3 降到 2。
 * - 如果图像轻微抖动导致 stable 断断续续：把 PVC_VISION_LOST_HOLD_FRAMES 从 2 提到 3。
 */

/* ====================================================================
 * 【新增宏定义】 解耦摄像头原始分辨率，定义算法独立运行分辨率
 * 原图为 188x120，传入算法前经过 2x2 压缩变为 94x60
 * ==================================================================== */
#define PVC_IMAGE_W                       (94U)
#define PVC_IMAGE_H                       (60U)
#define PVC_IMAGE_SIZE                    (PVC_IMAGE_W * PVC_IMAGE_H)

/* 总开关：0 时不编译本模块主体，便于快速排除视觉代码对工程的影响。 */
#define PVC_VISION_ENABLE                 (1)
/* 性能统计开关：开启后使用 PVC_VISION_PROFILE_TIMER 统计单帧耗时和帧间隔。 */
#define PVC_VISION_PROFILE_ENABLE         (1)
/* 多帧平滑开关：开启后 raw 结果会经过连续帧确认和短时间丢帧保持。 */
#define PVC_VISION_SMOOTH_ENABLE          (1)
/* 1 核视觉默认使用 TC_TIME2_CH1。TC_TIME2_CH2 已被遥控接收占用，不建议改到 CH2。 */
#define PVC_VISION_PROFILE_TIMER          (TC_TIME2_CH1)
/* 串口调试打印周期。0 表示关闭；例如设为 50 表示每 50 帧打印一次。 */
#define PVC_VISION_DEBUG_PRINT_EVERY      (0U)

/*
 * 白色阈值，与 PC Python/C 版本保持一致：gray >= 245 认为是白色候选像素。
 *
 * 现场初始值：
 * - 室外强光、PVC 很亮：245。
 * - 阴天、曝光偏暗：235~240。
 * - 如果黑色单边桥/阴影被误判为白色，说明阈值过低。
 */
#define PVC_VISION_WHITE_THRESHOLD        (245U)
/*
 * 连通域基础过滤阈值，用于去掉反光小点和噪声。
 *
 * 初始值含义：
 * - MIN_AREA=120：94x60 图像里小于 120 像素的白块直接当噪声。
 * - MIN_WIDTH=12、MIN_HEIGHT=4：候选至少要有一段明显宽度和高度。
 * - MIN_FILL_RATIO=0.25：包围框内部至少 25% 是白色，太稀疏一般是噪声。
 */
#define PVC_VISION_MIN_AREA               (120)
#define PVC_VISION_MIN_WIDTH              (12)
#define PVC_VISION_MIN_HEIGHT             (4)
#define PVC_VISION_MIN_FILL_RATIO         (0.25f)
/*
 * 最终决策阈值，与 PC 版本保持一致。
 * 最佳候选 score >= 0.58 才认为 raw detected。
 * 如果候选框稳定画出来但 detected=0，可以先降到 0.52~0.55。
 */
#define PVC_VISION_MIN_DECISION_SCORE     (0.58f)
/* 单帧最多保留的连通域数量。94x60 图像下 128 已足够，且内存占用可控。 */
#define PVC_VISION_MAX_COMPONENTS         (128)

/*
 * stable_detected 需要连续检测到的帧数。
 * 3 帧是默认安全值；比赛提速可试 2 帧，但误触发概率会增加。
 */
#define PVC_VISION_CONFIRM_FRAMES         (3U)
/*
 * stable_detected 允许短时间丢失的帧数。
 * 2 帧可以抗轻微曝光波动；如果车在颠簸入口抖动明显，可试 3 帧。
 */
#define PVC_VISION_LOST_HOLD_FRAMES       (2U)

typedef struct
{
    uint8 detected;              /* 本帧是否检测到 PVC。raw/stable 都使用这个字段表达有效性。 */
    uint8 component_count;       /* 本帧白色连通域总数，用于调试阈值和噪声情况。 */
    uint8 candidate_count;       /* 通过基础过滤的候选数量，用于判断是否过严/过松。 */
    uint16 area;                 /* 最佳候选的像素面积。 */
    uint8 bbox_xmin;             /* 最佳候选包围框左边界。无效时为 0xFF。 */
    uint8 bbox_ymin;             /* 最佳候选包围框上边界。无效时为 0xFF。 */
    uint8 bbox_xmax;             /* 最佳候选包围框右边界。无效时为 0xFF。 */
    uint8 bbox_ymax;             /* 最佳候选包围框下边界。无效时为 0xFF。 */
    uint8 entry_bottom_y;        /* 入口白边的近端行号，后续应查表换算 forward_mm。 */
    uint8 entry_top_y;           /* 入口白边的远端行号，可用于判断白边展开程度。 */
    float confidence;            /* 0~1 左右的评分，当前与 PC 版本 score 对齐。 */
    float centroid_x;            /* 白色区域质心 x，后续可转成 lateral_mm。 */
    float centroid_y;            /* 白色区域质心 y，主要用于调试。 */
    float fill_ratio;            /* 连通域面积 / 包围框面积，低填充率一般是噪声或破碎区域。 */
    float mean_gray;             /* 连通域平均灰度，便于现场看曝光是否合适。 */
    int16 forward_mm;            /* 入口距离估计。当前是占位线性表，车机实测后应替换为标定表。 */
    int16 lateral_mm;            /* 横向偏差估计。当前是占位线性表，后续应替换为标定表。 */
    int16 yaw_error_deg_x100;    /* 航向误差，单位 0.01 度。PVC 入口第一版暂不计算，固定为 0。 */
} pvc_vision_frame_result_t;

typedef struct
{
    uint32 frame_id;                 /* 视觉模块处理过的帧序号，从 1 开始递增。 */
    uint8 raw_detected;              /* 当前单帧原始检测结果，响应快但可能抖动。 */
    uint8 stable_detected;           /* 多帧确认后的稳定结果，控制层优先使用这个字段。 */
    uint8 detected_streak;           /* 连续 raw_detected=1 的帧数。 */
    uint8 lost_streak;               /* 连续 raw_detected=0 的帧数。 */
    pvc_vision_frame_result_t raw;   /* 单帧直接检测结果，适合调试和观察算法响应。 */
    pvc_vision_frame_result_t stable;/* 平滑后的结果，适合控制层和 0/1 核通信回传。 */
} pvc_vision_output_t;

/* 单帧检测耗时统计，单位 us：last/min/max/avg/count。 */
extern volatile runtime_profiler_t g_pvc_vision_cost_profiler;
/* 帧间隔统计，单位 us：可换算实际摄像头帧率。 */
extern volatile runtime_profiler_t g_pvc_vision_frame_profiler;
/* 模块统一输出。后续 0/1 核通信建议只搬运这个结构中的 stable 摘要字段。 */
extern volatile pvc_vision_output_t g_pvc_vision_output;
/* 输出结构写入保护标志。1 核 2ms ISR 发布 IPC 时看到该标志会跳过本次发布。 */
extern volatile uint8 g_pvc_vision_output_write_busy;

/* 初始化视觉模块、清空滤波状态、启动运行时间统计定时器。 */
void pvc_vision_init(void);
/* 清空连续帧确认和平滑状态。状态机切换项目时建议调用一次。 */
void pvc_vision_reset_filter(void);
/* 获取输出指针。当前 1 核内使用可直接读；跨核读取需要配合 DCache 同步。 */
const volatile pvc_vision_output_t *pvc_vision_get_output(void);
/* 处理一帧 MT9V03X 灰度图。gray 应指向压缩后的 94x60 连续灰度数组。 */
void pvc_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
