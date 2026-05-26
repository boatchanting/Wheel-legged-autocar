/*
 * =================================================================================
 * File: vision_slope_control.c
 * Role: Core 0 visual slope task state machine implementation.
 * Notes:
 *   1) Do not modify the common PVC control module for the slope requirement.
 *   2) Reuse PVC control output only as steering reference during entrance stage.
 *   3) Keep the whole slope task at fixed high speed, without any leg up/down logic.
 * =================================================================================
 */
#include "vision/vision_slope_control.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_pvc_control.h"
#include "tools/sbus.h"

/* --- External control variables --- */
extern volatile float err_degree;
extern volatile float target_speed_set;
extern int g_motor_enable;
extern uint8 g_special_action_trigger;

/* --- Global status --- */
volatile uint8 g_slope_vision_task_enable = 0U;
volatile uint8_t vision_detected_slope_point = 0U;
volatile vision_slope_task_status_t g_slope_vision_task_status = {0};

typedef struct
{
    vision_slope_task_state_e state;
    uint32 state_ticks;
    float start_x_mm;
    float start_y_mm;
    float locked_yaw_deg;
} vision_slope_task_ctx_t;

static vision_slope_task_ctx_t s_slope_task =
{
    VISION_SLOPE_TASK_IDLE,
    0U,
    0.0f,
    0.0f,
    0.0f
};

static float vision_slope_constrain_f(float value, float min_value, float max_value)
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

static float vision_slope_normalize_angle(float angle_deg)
{
    while (angle_deg > 180.0f)
    {
        angle_deg -= 360.0f;
    }
    while (angle_deg < -180.0f)
    {
        angle_deg += 360.0f;
    }
    return angle_deg;
}

static float vision_slope_distance_from(float x_mm, float y_mm)
{
    const float dx = inertial_nav.x - x_mm;
    const float dy = inertial_nav.y - y_mm;
    return sqrtf(dx * dx + dy * dy);
}

static float vision_slope_calc_yaw_hold_err(void)
{
    const float yaw_err = vision_slope_normalize_angle(s_slope_task.locked_yaw_deg -
                                                       inertial_nav.relative_yaw);
    return vision_slope_constrain_f(yaw_err,
                                    -VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG,
                                    VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG);
}

static void vision_slope_set_state(vision_slope_task_state_e next_state)
{
    s_slope_task.state = next_state;
    s_slope_task.state_ticks = 0U;

    if (next_state == VISION_SLOPE_TASK_LOCK_FAST)
    {
        /* Use the yaw-lock moment as the mileage start point for the 2000mm exit rule. */
        s_slope_task.start_x_mm = inertial_nav.x;
        s_slope_task.start_y_mm = inertial_nav.y;
        s_slope_task.locked_yaw_deg = inertial_nav.relative_yaw;
    }
}

static void vision_slope_publish_status(const volatile vision_ipc_packet_t *packet,
                                        float traveled_mm,
                                        float err_cmd,
                                        float speed_cmd)
{
    vision_slope_task_status_t status;

    memset(&status, 0, sizeof(status));
    status.enabled = g_slope_vision_task_enable;
    status.state = s_slope_task.state;
    status.state_ticks = s_slope_task.state_ticks;
    status.last_seq = (packet != NULL) ? packet->seq : 0U;
    status.traveled_mm = traveled_mm;
    status.locked_yaw_deg = s_slope_task.locked_yaw_deg;
    status.err_degree_cmd = err_cmd;
    status.speed_cmd = speed_cmd;
    status.pvc_stable = g_vision_pvc_control_status.stable_detected;
    status.pvc_ratio_u16 = g_vision_pvc_control_status.bbox_area_ratio_u16;
    status.pvc_steer_error_px_x100 = g_vision_pvc_control_status.steer_error_px_x100;

    g_slope_vision_task_status = status;
}

static void vision_slope_cleanup(uint8 stop_car)
{
    /* Only release PVC assist. No leg up/down logic exists in the slope task. */
    VisionPvcControl_SetEnable(0U);
    VisionIpc_Core0_SetPvcEnable(VISION_PVC_DETECT_DEFAULT_ACTIVE);

    err_degree = 0.0f;
    if (stop_car != 0U)
    {
        target_speed_set = 0.0f;
    }

    g_special_action_trigger = 0U;
    g_slope_vision_task_enable = 0U;

    memset(&s_slope_task, 0, sizeof(s_slope_task));
    g_slope_vision_task_status.enabled = 0U;
    g_slope_vision_task_status.state = VISION_SLOPE_TASK_IDLE;
}

static void vision_slope_enter_task(void)
{
    memset(&s_slope_task, 0, sizeof(s_slope_task));
    s_slope_task.state = VISION_SLOPE_TASK_ENTER_PVC;
    s_slope_task.locked_yaw_deg = inertial_nav.relative_yaw;

    g_special_action_trigger = 1U;

    /* Enable PVC vision only for entrance alignment. Speed is always forced locally. */
    VisionIpc_Core0_SetPvcEnable(1U);
    VisionPvcControl_SetEnable(1U);
}

void VisionSlopeTask_Init(void)
{
    memset(&s_slope_task, 0, sizeof(s_slope_task));
    memset((void *)&g_slope_vision_task_status, 0, sizeof(g_slope_vision_task_status));
    g_slope_vision_task_enable = 0U;
    vision_detected_slope_point = 0U;
}

void VisionSlopeTask_Start(void)
{
    g_slope_vision_task_enable = 1U;
}

void VisionSlopeTask_Stop(void)
{
    vision_slope_cleanup(1U);
}

uint8 VisionSlopeTask_IsActive(void)
{
    return (uint8)((g_slope_vision_task_enable != 0U) ||
                   (s_slope_task.state != VISION_SLOPE_TASK_IDLE));
}

void VisionSlopeTask_Update_2ms(void)
{
    const volatile vision_ipc_packet_t *packet = VisionIpc_Core0_GetLatest();
    float traveled_mm = 0.0f;
    float err_cmd = 0.0f;
    float speed_cmd = 0.0f;

    if ((g_slope_vision_task_enable == 0U) &&
        (s_slope_task.state == VISION_SLOPE_TASK_IDLE))
    {
        return;
    }

    #if REMOTE_CONTROL == 1
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U) || (robot_ctrl.brake_active == 1U))
    {
        vision_slope_cleanup(1U);
        return;
    }
    #endif
    #if REMOTE_CONTROL == 0
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U))
    {
        vision_slope_cleanup(1U);
        return;
    }
    #endif

    if (s_slope_task.state == VISION_SLOPE_TASK_IDLE)
    {
        vision_slope_enter_task();
    }

    if (s_slope_task.state_ticks < 0xFFFFFFFFU)
    {
        s_slope_task.state_ticks++;
    }

    if ((s_slope_task.state == VISION_SLOPE_TASK_LOCK_FAST) ||
        (s_slope_task.state == VISION_SLOPE_TASK_LOCK_CRUISE) ||
        (s_slope_task.state == VISION_SLOPE_TASK_FINISH))
    {
        traveled_mm = vision_slope_distance_from(s_slope_task.start_x_mm, s_slope_task.start_y_mm);
    }

    switch (s_slope_task.state)
    {
        case VISION_SLOPE_TASK_ENTER_PVC:
            /*
             * Reuse PVC steering only. The common PVC controller speed logic is ignored here,
             * so the slope task can hold fixed high speed from trigger to finish.
             */
            err_cmd = g_vision_pvc_control_status.err_degree_cmd;
            speed_cmd = VISION_SLOPE_TASK_ENTER_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if ((g_vision_pvc_control_status.stable_detected != 0U) &&
                (g_vision_pvc_control_status.bbox_area_ratio_u16 >= VISION_SLOPE_TASK_PVC_FULL_RATIO_U16))
            {
                VisionPvcControl_SetEnable(0U);
                VisionIpc_Core0_SetPvcEnable(VISION_PVC_DETECT_DEFAULT_ACTIVE);
                vision_slope_set_state(VISION_SLOPE_TASK_LOCK_FAST);
            }
            else if (s_slope_task.state_ticks >= VISION_SLOPE_TASK_ENTER_TIMEOUT_TICKS)
            {
                vision_slope_set_state(VISION_SLOPE_TASK_FAILSAFE);
            }
            break;

        case VISION_SLOPE_TASK_LOCK_FAST:
            err_cmd = vision_slope_calc_yaw_hold_err();
            speed_cmd = VISION_SLOPE_TASK_ENTER_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if (s_slope_task.state_ticks >= VISION_SLOPE_TASK_FAST_HOLD_TICKS)
            {
                vision_slope_set_state(VISION_SLOPE_TASK_LOCK_CRUISE);
            }
            break;

        case VISION_SLOPE_TASK_LOCK_CRUISE:
            err_cmd = vision_slope_calc_yaw_hold_err();
            speed_cmd = VISION_SLOPE_TASK_CRUISE_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if (traveled_mm >= VISION_SLOPE_TASK_EXIT_DISTANCE_MM)
            {
                vision_slope_set_state(VISION_SLOPE_TASK_FINISH);
            }
            break;

        case VISION_SLOPE_TASK_FINISH:
            /* Normal finish only releases control ownership, no leg action and no forced brake. */
            vision_slope_cleanup(0U);
            break;

        case VISION_SLOPE_TASK_FAILSAFE:
        default:
            vision_slope_cleanup(1U);
            break;
    }

    vision_slope_publish_status(packet, traveled_mm, err_cmd, speed_cmd);
}
