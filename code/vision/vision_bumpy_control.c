#include "vision/vision_bumpy_control.h"
#include "vision/vision_ipc_core0.h"

volatile vision_bumpy_control_status_t g_vision_bumpy_control_status = {0};
volatile runtime_profiler_t g_vision_bumpy_control_profiler = {0};
volatile uint8 g_bumpy_control_enable = VISION_BUMPY_CONTROL_DEFAULT_ACTIVE;

static vision_bumpy_control_status_t g_bumpy_ctrl_shadow;

static float vision_bumpy_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

static float vision_bumpy_constrain_f(float value, float min_value, float max_value)
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

static float vision_bumpy_calc_err_degree(const volatile vision_ipc_packet_t *packet)
{
    const float steer_px = (float)packet->bumpy_steer_error_px_x100 * 0.01f;
    float err = steer_px * VISION_BUMPY_K_STEER_DEG_PER_PX;

    /* 方向控制仅基于视觉，不叠加惯导角度闭环；按图像误差直接映射。 */
    err = vision_bumpy_constrain_f(err, -VISION_BUMPY_MAX_ERR_DEG, VISION_BUMPY_MAX_ERR_DEG);

    if (vision_bumpy_abs_f(err) < VISION_BUMPY_DEADBAND_DEG)
    {
        err = 0.0f;
    }
    return err;
}

static void vision_bumpy_apply_idle_outputs(void)
{
    g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_IDLE;
    g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;
    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;
}

void VisionBumpyControl_Init(void)
{
    memset(&g_bumpy_ctrl_shadow, 0, sizeof(g_bumpy_ctrl_shadow));
    g_bumpy_control_enable = VISION_BUMPY_CONTROL_DEFAULT_ACTIVE;
    g_bumpy_ctrl_shadow.enabled = g_bumpy_control_enable;
    g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_IDLE;
    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;

#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_RESET(&g_vision_bumpy_control_profiler);
#endif
}

void VisionBumpyControl_SetEnable(uint8 enable)
{
    g_bumpy_control_enable = enable ? 1U : 0U;
    g_bumpy_ctrl_shadow.enabled = g_bumpy_control_enable;

    if (g_bumpy_control_enable == 0U)
    {
        vision_bumpy_apply_idle_outputs();
    }
}

uint8 VisionBumpyControl_IsEnabled(void)
{
    return g_bumpy_control_enable;
}

void VisionBumpyControl_Update_2ms(void)
{
#if VISION_BUMPY_CONTROL_ENABLE
    const volatile vision_ipc_packet_t *packet;
    uint8 packet_is_bumpy;
    uint8 packet_new;

#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_BEGIN(g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);
#endif

    g_bumpy_ctrl_shadow.enabled = g_bumpy_control_enable ? 1U : 0U;
    if (g_bumpy_ctrl_shadow.enabled == 0U)
    {
        vision_bumpy_apply_idle_outputs();
#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    packet = VisionIpc_Core0_GetLatest();
    packet_new = (uint8)(packet->seq != g_bumpy_ctrl_shadow.last_seq);
    packet_is_bumpy = (uint8)((packet->valid_mask & VISION_VALID_BUMPY) != 0U);

    g_bumpy_ctrl_shadow.has_new_packet = packet_new;
    if (packet_new)
    {
        g_bumpy_ctrl_shadow.last_seq = packet->seq;
        g_bumpy_ctrl_shadow.stale_ticks = 0U;
    }
    else if (g_bumpy_ctrl_shadow.stale_ticks < 0xFFFFU)
    {
        g_bumpy_ctrl_shadow.stale_ticks++;
    }

    if ((packet->seq == 0U) ||
        (packet_is_bumpy == 0U) ||
        (g_bumpy_ctrl_shadow.stale_ticks > VISION_BUMPY_STALE_TIMEOUT_TICKS))
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_STALE;
        g_bumpy_ctrl_shadow.stable_detected = 0U;
        g_bumpy_ctrl_shadow.raw_detected = 0U;
        g_bumpy_ctrl_shadow.phase = 0U;
        g_bumpy_ctrl_shadow.mode = 0U;
        g_bumpy_ctrl_shadow.confidence_u16 = 0U;
        g_bumpy_ctrl_shadow.steer_error_px_x100 = 0;
        g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;
        g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;
#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    g_bumpy_ctrl_shadow.stable_detected = packet->bumpy_stable_detected;
    g_bumpy_ctrl_shadow.raw_detected = packet->bumpy_detected;
    g_bumpy_ctrl_shadow.phase = packet->bumpy_phase;
    g_bumpy_ctrl_shadow.mode = packet->bumpy_mode;
    g_bumpy_ctrl_shadow.confidence_u16 = packet->bumpy_confidence_u16;
    g_bumpy_ctrl_shadow.steer_error_px_x100 = packet->bumpy_steer_error_px_x100;

    if (packet->bumpy_stable_detected)
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_TRACK;
        g_bumpy_ctrl_shadow.err_degree_cmd = vision_bumpy_calc_err_degree(packet);
    }
    else if (packet->bumpy_detected)
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_SEARCH;
        g_bumpy_ctrl_shadow.err_degree_cmd = vision_bumpy_calc_err_degree(packet) * 0.6f;
    }
    else
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_SEARCH;
        g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;
    }

    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;

#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);
#endif
#endif
}

float VisionBumpyControl_GetErrDegreeCmd(void)
{
    return g_vision_bumpy_control_status.err_degree_cmd;
}
