#include "vision/vision_pvc_control.h"
#include "vision/vision_ipc_core0.h"

extern volatile float err_degree;
extern volatile float target_speed_set;
extern int g_motor_enable;

volatile vision_pvc_control_status_t g_vision_pvc_control_status = {0};
volatile runtime_profiler_t g_vision_pvc_control_profiler = {0};
volatile uint8 g_pvc_control_enable = VISION_PVC_CONTROL_DEFAULT_ACTIVE;

static vision_pvc_control_status_t g_pvc_ctrl_shadow;

static float vision_pvc_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

static float vision_pvc_constrain_f(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static float vision_pvc_calc_err_degree(const volatile vision_ipc_packet_t *packet)
{
    const float lateral_deg =
        (float)packet->pvc_lateral_mm * VISION_PVC_CONTROL_K_LAT_DEG_PER_MM;
    const float yaw_deg =
        ((float)packet->pvc_yaw_error_deg_x100 * 0.01f) * VISION_PVC_CONTROL_K_YAW_DEG_PER_DEG;
    const float err = VISION_PVC_CONTROL_LATERAL_SIGN * (lateral_deg + yaw_deg);

    return vision_pvc_constrain_f(err,
                                  -VISION_PVC_CONTROL_MAX_ERR_DEG,
                                  VISION_PVC_CONTROL_MAX_ERR_DEG);
}

static uint16 vision_pvc_calc_bbox_ratio_u16(const volatile vision_ipc_packet_t *packet)
{
    /*
     * 计算 PVC 候选包围框占整幅图像的比例，单位 0.1%。
     *
     * 例：
     * - 返回 100 表示 10.0%。
     * - 返回 900 表示 90.0%。
     *
     * 为什么用包围框面积而不是 pvc_area：
     * - pvc_area 是真实白色像素数，受曝光、反光颗粒、阴影影响较大。
     * - bbox_area 更适合判断“PVC 区域已经铺满视野”，用于到达停车更稳定。
     */
    uint16 width;
    uint16 height;
    uint32 bbox_area;

    if ((packet->pvc_bbox_xmin == 0xFFU) ||
        (packet->pvc_bbox_ymin == 0xFFU) ||
        (packet->pvc_bbox_xmax < packet->pvc_bbox_xmin) ||
        (packet->pvc_bbox_ymax < packet->pvc_bbox_ymin))
    {
        return 0U;
    }

    width = (uint16)(packet->pvc_bbox_xmax - packet->pvc_bbox_xmin + 1U);
    height = (uint16)(packet->pvc_bbox_ymax - packet->pvc_bbox_ymin + 1U);
    bbox_area = (uint32)width * (uint32)height;

    if (bbox_area >= VISION_PVC_CONTROL_IMAGE_AREA)
    {
        return 1000U;
    }

    return (uint16)((bbox_area * 1000U) / VISION_PVC_CONTROL_IMAGE_AREA);
}

static void vision_pvc_apply_idle_outputs(void)
{
    g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_IDLE;
    g_pvc_ctrl_shadow.speed_cmd = 0.0f;
    g_pvc_ctrl_shadow.err_degree_cmd = 0.0f;
    g_vision_pvc_control_status = g_pvc_ctrl_shadow;
}

void VisionPvcControl_Init(void)
{
    memset(&g_pvc_ctrl_shadow, 0, sizeof(g_pvc_ctrl_shadow));
    g_pvc_control_enable = VISION_PVC_CONTROL_DEFAULT_ACTIVE;
    g_pvc_ctrl_shadow.enabled = g_pvc_control_enable;
    g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_IDLE;
    g_vision_pvc_control_status = g_pvc_ctrl_shadow;

#if VISION_PVC_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_RESET(&g_vision_pvc_control_profiler);
#endif

    /*
     * 检测任务选择和控制开关分离：
     * - VisionIpc_Core0_SetPvcEnable(1) 只是在 IPC 命令里选择 1 核运行 PVC 检测。
     * - g_pvc_control_enable 才决定 0 核是否接管 err_degree/target_speed_set。
     */
    VisionIpc_Core0_SetPvcEnable(VISION_PVC_DETECT_DEFAULT_ACTIVE);
}

void VisionPvcControl_SetEnable(uint8 enable)
{
    g_pvc_control_enable = enable ? 1U : 0U;
    g_pvc_ctrl_shadow.enabled = g_pvc_control_enable;
    if (g_pvc_control_enable == 0U)
    {
        vision_pvc_apply_idle_outputs();
    }
}

uint8 VisionPvcControl_IsEnabled(void)
{
    return g_pvc_control_enable;
}

void VisionPvcControl_Update_2ms(void)
{
#if VISION_PVC_CONTROL_ENABLE
    const volatile vision_ipc_packet_t *packet;
    uint8 packet_is_pvc;
    uint8 packet_new;

#if VISION_PVC_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_BEGIN(g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif

    /*
     * 允许调试器/菜单直接改 g_pvc_control_enable，不强制必须调用
     * VisionPvcControl_SetEnable()。
     */
    g_pvc_ctrl_shadow.enabled = g_pvc_control_enable ? 1U : 0U;

    if (g_pvc_ctrl_shadow.enabled == 0U)
    {
        vision_pvc_apply_idle_outputs();
#if VISION_PVC_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    packet = VisionIpc_Core0_GetLatest();
    packet_new = (uint8)(packet->seq != g_pvc_ctrl_shadow.last_seq);
    packet_is_pvc = (uint8)((packet->valid_mask & VISION_VALID_PVC) != 0U);

    g_pvc_ctrl_shadow.has_new_packet = packet_new;
    if (packet_new)
    {
        g_pvc_ctrl_shadow.last_seq = packet->seq;
        g_pvc_ctrl_shadow.stale_ticks = 0U;
    }
    else if (g_pvc_ctrl_shadow.stale_ticks < 0xFFFFU)
    {
        g_pvc_ctrl_shadow.stale_ticks++;
    }

    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U))
    {
        vision_pvc_apply_idle_outputs();
#if VISION_PVC_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    if ((packet->seq == 0U) ||
        (packet_is_pvc == 0U) ||
        (g_pvc_ctrl_shadow.stale_ticks > VISION_PVC_CONTROL_STALE_TIMEOUT_TICKS))
    {
        g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_STALE;
        g_pvc_ctrl_shadow.stable_detected = 0U;
        g_pvc_ctrl_shadow.raw_detected = 0U;
        g_pvc_ctrl_shadow.forward_mm = -1;
        g_pvc_ctrl_shadow.lateral_mm = 0;
        g_pvc_ctrl_shadow.yaw_error_deg_x100 = 0;
        g_pvc_ctrl_shadow.bbox_area_ratio_u16 = 0U;
        g_pvc_ctrl_shadow.err_degree_cmd = 0.0f;
        g_pvc_ctrl_shadow.speed_cmd = 0.0f;
        err_degree = 0.0f;
        target_speed_set = 0.0f;
        g_vision_pvc_control_status = g_pvc_ctrl_shadow;
#if VISION_PVC_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    g_pvc_ctrl_shadow.stable_detected = packet->pvc_stable_detected;
    g_pvc_ctrl_shadow.raw_detected = packet->pvc_detected;
    g_pvc_ctrl_shadow.forward_mm = packet->pvc_forward_mm;
    g_pvc_ctrl_shadow.lateral_mm = packet->pvc_lateral_mm;
    g_pvc_ctrl_shadow.yaw_error_deg_x100 = packet->pvc_yaw_error_deg_x100;
    g_pvc_ctrl_shadow.bbox_area_ratio_u16 = vision_pvc_calc_bbox_ratio_u16(packet);

    if (packet->pvc_stable_detected)
    {
        const int16 forward_mm = packet->pvc_forward_mm;
        const float turn_err = vision_pvc_calc_err_degree(packet);
        const uint8 bbox_stop = (uint8)(g_pvc_ctrl_shadow.bbox_area_ratio_u16 >=
                                        VISION_PVC_CONTROL_STOP_BBOX_RATIO_U16);

        g_pvc_ctrl_shadow.err_degree_cmd = turn_err;
        err_degree = turn_err;

        if (bbox_stop ||
            ((forward_mm >= 0) && (forward_mm <= VISION_PVC_CONTROL_ARRIVE_FORWARD_MM)))
        {
            g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_ARRIVED;
            g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_ARRIVE_SPEED_SET;
        }
        else if ((forward_mm >= 0) && (forward_mm <= VISION_PVC_CONTROL_CLOSE_FORWARD_MM))
        {
            g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_TRACK;
            g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_CLOSE_SPEED_SET;
        }
        else
        {
            g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_TRACK;
            g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_TRACK_SPEED_SET;
        }

        target_speed_set = g_pvc_ctrl_shadow.speed_cmd;
    }
    else if (packet->pvc_detected)
    {
        const float turn_err = vision_pvc_calc_err_degree(packet);

        g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_SEARCH;
        g_pvc_ctrl_shadow.err_degree_cmd = turn_err * 0.5f;
        g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_SEARCH_SPEED_SET;
        err_degree = g_pvc_ctrl_shadow.err_degree_cmd;
        target_speed_set = g_pvc_ctrl_shadow.speed_cmd;
    }
    else
    {
        g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_SEARCH;
        g_pvc_ctrl_shadow.err_degree_cmd = 0.0f;
        g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_SEARCH_SPEED_SET;
        err_degree = 0.0f;
        target_speed_set = g_pvc_ctrl_shadow.speed_cmd;
    }

    if (vision_pvc_abs_f(g_pvc_ctrl_shadow.err_degree_cmd) < 0.3f)
    {
        g_pvc_ctrl_shadow.err_degree_cmd = 0.0f;
        err_degree = 0.0f;
    }

    g_vision_pvc_control_status = g_pvc_ctrl_shadow;

#if VISION_PVC_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
#endif
}
