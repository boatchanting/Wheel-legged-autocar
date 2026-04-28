#ifndef VISION_PVC_CONTROL_H
#define VISION_PVC_CONTROL_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "vision/vision_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 0 核 PVC 入口控制模块。
 *
 * 输入：
 * - VisionIpc_Core0_Update_2ms() 从共享内存复制出的 g_vision_ipc_latest。
 * - 主要使用 pvc_stable_detected、pvc_forward_mm、pvc_lateral_mm、pvc_bbox_*。
 *
 * 输出：
 * - err_degree：转向角度环输入，单位度。
 * - target_speed_set：速度环目标，负数代表向前。
 *
 * 第一版到达逻辑：
 * - stable 检测到 PVC 后，按 lateral_mm 修正方向并向前走。
 * - forward_mm 小于 CLOSE_FORWARD_MM 后减速。
 * - forward_mm 小于 ARRIVE_FORWARD_MM，或者 PVC 包围框面积超过整幅图像 90%，停车。
 *
 * 使用方式：
 * - 惯导接近项目入口约 800mm 内时调用 VisionPvcControl_SetEnable(1)。
 * - 离开入口控制，切入具体项目状态机时调用 VisionPvcControl_SetEnable(0)，避免覆盖项目控制。
 */

#define VISION_PVC_CONTROL_ENABLE                 (1)
#define VISION_PVC_CONTROL_DEFAULT_ACTIVE         (1)
#define VISION_PVC_CONTROL_PROFILE_ENABLE         (1)
#define VISION_PVC_CONTROL_PROFILE_TIMER          (TC_TIME2_CH0)

/* 如果第一次实车测试发现车朝远离 PVC 中心的方向修正，把 1.0f 改成 -1.0f。 */
#define VISION_PVC_CONTROL_LATERAL_SIGN           (1.0f)

/* 画面尺寸必须和 1 核 pvc_vision.h 的 PVC_IMAGE_W/H 保持一致。 */
#define VISION_PVC_CONTROL_IMAGE_W                (94U)
#define VISION_PVC_CONTROL_IMAGE_H                (60U)
#define VISION_PVC_CONTROL_IMAGE_AREA             (VISION_PVC_CONTROL_IMAGE_W * VISION_PVC_CONTROL_IMAGE_H)

/*
 * 速度初始参数。
 * - SEARCH：还没有稳定识别，慢速向前搜索入口。
 * - TRACK：稳定识别且距离还远，正常视觉引导前进。
 * - CLOSE：入口较近，减速防止冲过。
 * - ARRIVE：到达/压到 PVC 区域后停车。
 */
#define VISION_PVC_CONTROL_SEARCH_SPEED_SET       (-35.0f)
#define VISION_PVC_CONTROL_TRACK_SPEED_SET        (-55.0f)
#define VISION_PVC_CONTROL_CLOSE_SPEED_SET        (-25.0f)
#define VISION_PVC_CONTROL_ARRIVE_SPEED_SET       (0.0f)

/*
 * 距离初始参数，单位 mm。
 * 当前 forward_mm 还是 1 核线性估计值，实车标定前不要把阈值设得太激进。
 */
#define VISION_PVC_CONTROL_CLOSE_FORWARD_MM       (320)
#define VISION_PVC_CONTROL_ARRIVE_FORWARD_MM      (140)
#define VISION_PVC_CONTROL_STALE_TIMEOUT_TICKS    (100U)  /* 100 * 2ms = 200ms */

/*
 * 包围框面积停车阈值。
 * - 900 表示 PVC 最佳候选包围框面积 >= 整幅 94x60 图像的 90.0%。
 * - 这个条件比 forward_mm 更直接，适合“看到一大片 PVC 已经进入区域”时停车。
 * - 如果停车太早，改成 950；如果冲过区域，改成 800~850。
 */
#define VISION_PVC_CONTROL_STOP_BBOX_RATIO_U16    (900U)

/*
 * 横向偏差转角初始参数。
 * err_degree = lateral_mm * K_LAT + yaw_error * K_YAW，最后限幅到 MAX_ERR_DEG。
 * 第一版 PVC 入口 yaw_error 通常为 0，主要靠 lateral_mm 控制。
 */
#define VISION_PVC_CONTROL_K_LAT_DEG_PER_MM       (0.035f)
#define VISION_PVC_CONTROL_K_YAW_DEG_PER_DEG      (0.50f)
#define VISION_PVC_CONTROL_MAX_ERR_DEG            (18.0f)

typedef enum
{
    VISION_PVC_CTRL_IDLE = 0,
    VISION_PVC_CTRL_SEARCH,
    VISION_PVC_CTRL_TRACK,
    VISION_PVC_CTRL_ARRIVED,
    VISION_PVC_CTRL_STALE,
} vision_pvc_control_state_e;

typedef struct
{
    uint8 enabled;
    uint8 has_new_packet;
    uint8 stable_detected;
    uint8 raw_detected;
    uint32 last_seq;
    uint16 stale_ticks;
    vision_pvc_control_state_e state;
    int16 forward_mm;
    int16 lateral_mm;
    int16 yaw_error_deg_x100;
    uint16 bbox_area_ratio_u16;
    float err_degree_cmd;
    float speed_cmd;
} vision_pvc_control_status_t;

extern volatile vision_pvc_control_status_t g_vision_pvc_control_status;
extern volatile runtime_profiler_t g_vision_pvc_control_profiler;

void VisionPvcControl_Init(void);
void VisionPvcControl_SetEnable(uint8 enable);
uint8 VisionPvcControl_IsEnabled(void);
void VisionPvcControl_Update_2ms(void);

#ifdef __cplusplus
}
#endif

#endif
