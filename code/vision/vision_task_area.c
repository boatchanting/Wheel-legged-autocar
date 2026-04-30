/*
 * 文件: vision_task_area.c
 * 作用: 视觉桥任务（单边桥相关）的状态机与控制执行。
 * 说明: 使用 1 核直线/桥面检测结果驱动过桥阶段控制。
 */
#include "vision/vision_task_area.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_pvc_control.h"
#include "plan/bridge.h"

#if VISION_BRIDGE_TASK_ENABLE

extern volatile float err_degree;
extern volatile float target_speed_set;
extern int g_motor_enable;
extern uint8 g_special_action_trigger;
extern uint8_t roll_balance_enable;
extern int32 acc_limit;
extern int32 dec_limit;
extern float servo_height;

volatile uint8 g_bridge_vision_task_enable = 0U;
volatile vision_bridge_task_status_t g_bridge_vision_task_status = {0};

typedef struct
{
    vision_bridge_task_state_e state;
    uint32 state_ticks;
    uint16 exit_lost_ticks;
    uint16 bridge_hold_ticks;
    uint16 align_ok_ticks;
    uint32 last_seq;
    float start_x_mm;
    float start_y_mm;
    float exit_start_x_mm;
    float exit_start_y_mm;
    float locked_yaw_deg;
    int32 saved_acc_limit;
    int32 saved_dec_limit;
    uint8 saved_limits_valid;
} vision_bridge_task_ctx_t;

static vision_bridge_task_ctx_t s_bridge_task;

/*
 * 函数: vision_bridge_abs_f
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static float vision_bridge_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

/*
 * 函数: vision_bridge_constrain_f
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static float vision_bridge_constrain_f(float value, float min_value, float max_value)
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

/*
 * 函数: vision_bridge_normalize_angle
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static float vision_bridge_normalize_angle(float angle_deg)
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

/*
 * 函数: vision_bridge_distance_from
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static float vision_bridge_distance_from(float x_mm, float y_mm)
{
    const float dx = inertial_nav.x - x_mm;
    const float dy = inertial_nav.y - y_mm;
    return sqrtf(dx * dx + dy * dy);
}

/*
 * 函数: vision_bridge_calc_line_err_degree
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static float vision_bridge_calc_line_err_degree(const volatile vision_ipc_packet_t *packet)
{
    const float lateral_px = (float)packet->line_lateral_px_x100 * 0.01f;
    const float yaw_deg = (float)packet->line_yaw_error_deg_x100 * 0.01f;
    const float err = VISION_BRIDGE_TASK_LINE_SIGN *
                      (lateral_px * VISION_BRIDGE_TASK_K_LAT_DEG_PER_PX +
                       yaw_deg * VISION_BRIDGE_TASK_K_YAW_DEG_PER_DEG);

    return vision_bridge_constrain_f(err,
                                     -VISION_BRIDGE_TASK_MAX_ERR_DEG,
                                     VISION_BRIDGE_TASK_MAX_ERR_DEG);
}

/*
 * 函数: vision_bridge_calc_yaw_hold_err
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static float vision_bridge_calc_yaw_hold_err(void)
{
    const float err = vision_bridge_normalize_angle(s_bridge_task.locked_yaw_deg -
                                                   inertial_nav.relative_yaw);
    return vision_bridge_constrain_f(err,
                                     -VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG,
                                     VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG);
}

/*
 * 函数: vision_bridge_save_servo_limits_once
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static void vision_bridge_save_servo_limits_once(void)
{
    if (s_bridge_task.saved_limits_valid == 0U)
    {
        s_bridge_task.saved_acc_limit = acc_limit;
        s_bridge_task.saved_dec_limit = dec_limit;
        s_bridge_task.saved_limits_valid = 1U;
    }
}

/*
 * 函数: vision_bridge_apply_high_posture
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static void vision_bridge_apply_high_posture(void)
{
    vision_bridge_save_servo_limits_once();
    acc_limit = bridge_params.servo_acc_bridge;
    dec_limit = bridge_params.servo_dec_bridge;
    roll_balance_enable = 1U;
    Bridge_Apply_Height_Control(bridge_params.height_bridge,
                                bridge_params.height_step_rise * VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE);
}

/*
 * 函数: vision_bridge_apply_normal_posture
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static void vision_bridge_apply_normal_posture(void)
{
    roll_balance_enable = 0U;
    if (s_bridge_task.saved_limits_valid)
    {
        acc_limit = s_bridge_task.saved_acc_limit;
        dec_limit = s_bridge_task.saved_dec_limit;
    }
    Bridge_Apply_Height_Control(bridge_params.height_normal,
                                bridge_params.height_step_drop * VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE);
}

/*
 * 函数: vision_bridge_set_state
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static void vision_bridge_set_state(vision_bridge_task_state_e next_state)
{
    s_bridge_task.state = next_state;
    s_bridge_task.state_ticks = 0U;
    s_bridge_task.align_ok_ticks = 0U;

    if (next_state == VISION_BRIDGE_TASK_EXIT)
    {
        s_bridge_task.exit_start_x_mm = inertial_nav.x;
        s_bridge_task.exit_start_y_mm = inertial_nav.y;
    }
}

static void vision_bridge_publish_status(const volatile vision_ipc_packet_t *packet,
                                         float traveled_mm,
                                         float err_cmd,
                                         float speed_cmd)
{
    vision_bridge_task_status_t status;

    memset(&status, 0, sizeof(status));
    status.enabled = g_bridge_vision_task_enable;
    status.state = s_bridge_task.state;
    status.state_ticks = s_bridge_task.state_ticks;
    status.last_seq = packet ? packet->seq : 0U;
    status.traveled_mm = traveled_mm;
    status.err_degree_cmd = err_cmd;
    status.speed_cmd = speed_cmd;
    status.exit_lost_ticks = s_bridge_task.exit_lost_ticks;
    status.bridge_hold_ticks = s_bridge_task.bridge_hold_ticks;

    if (packet != NULL)
    {
        status.line_stable = packet->line_stable_detected;
        status.bridge_stable = packet->line_bridge_stable_detected;
        status.line_confidence_u16 = packet->line_confidence_u16;
        status.bridge_confidence_u16 = packet->line_bridge_confidence_u16;
        status.line_roi_white_ratio_u16 = packet->line_roi_white_ratio_u16;
        status.line_lateral_px_x100 = packet->line_lateral_px_x100;
        status.line_yaw_error_deg_x100 = packet->line_yaw_error_deg_x100;
    }

    g_bridge_vision_task_status = status;
}

#if VISION_BRIDGE_TASK_NAV_CORRECT_ENABLE
/*
 * 函数: vision_bridge_apply_nav_correction
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static void vision_bridge_apply_nav_correction(void)
{
    const float yaw_rad = s_bridge_task.locked_yaw_deg * 0.0174532925f;
    inertial_nav.x = s_bridge_task.start_x_mm -
                     cosf(yaw_rad) * VISION_BRIDGE_TASK_NAV_CORRECT_DISTANCE_MM;
    inertial_nav.y = s_bridge_task.start_y_mm +
                     sinf(yaw_rad) * VISION_BRIDGE_TASK_NAV_CORRECT_DISTANCE_MM;
    inertial_nav.relative_yaw = s_bridge_task.locked_yaw_deg;
    inertial_nav.vx_body = 0.0f;
    inertial_nav.vy_body = 0.0f;
}
#endif

/*
 * 函数: vision_bridge_cleanup
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static void vision_bridge_cleanup(uint8 stop_car)
{
    VisionPvcControl_SetEnable(0U);
    VisionIpc_Core0_SetBridgeLineEnable(0U);
    vision_bridge_apply_normal_posture();

    if (stop_car)
    {
        target_speed_set = 0.0f;
        err_degree = 0.0f;
    }

    g_special_action_trigger = 0U;
    g_bridge_vision_task_enable = 0U;
    memset(&s_bridge_task, 0, sizeof(s_bridge_task));
    g_bridge_vision_task_status.enabled = 0U;
    g_bridge_vision_task_status.state = VISION_BRIDGE_TASK_IDLE;
}

/*
 * 函数: VisionBridgeTask_Init
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
void VisionBridgeTask_Init(void)
{
    memset(&s_bridge_task, 0, sizeof(s_bridge_task));
    memset((void *)&g_bridge_vision_task_status, 0, sizeof(g_bridge_vision_task_status));
    g_bridge_vision_task_enable = 0U;
}

/*
 * 函数: VisionBridgeTask_Start
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
void VisionBridgeTask_Start(void)
{
    g_bridge_vision_task_enable = 1U;
}

/*
 * 函数: VisionBridgeTask_Stop
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
void VisionBridgeTask_Stop(void)
{
    vision_bridge_cleanup(1U);
}

/*
 * 函数: VisionBridgeTask_IsActive
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
uint8 VisionBridgeTask_IsActive(void)
{
    return (uint8)((g_bridge_vision_task_enable != 0U) ||
                   (s_bridge_task.state != VISION_BRIDGE_TASK_IDLE));
}

/*
 * 函数: vision_bridge_enter_task
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
static void vision_bridge_enter_task(void)
{
    memset(&s_bridge_task, 0, sizeof(s_bridge_task));
    s_bridge_task.state = VISION_BRIDGE_TASK_ENTER_PVC;
    s_bridge_task.start_x_mm = inertial_nav.x;
    s_bridge_task.start_y_mm = inertial_nav.y;
    s_bridge_task.locked_yaw_deg = inertial_nav.relative_yaw;
    s_bridge_task.saved_acc_limit = acc_limit;
    s_bridge_task.saved_dec_limit = dec_limit;
    s_bridge_task.saved_limits_valid = 1U;

    g_special_action_trigger = 1U;
    VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);
    VisionPvcControl_SetEnable(1U);
}

/*
 * 函数: VisionBridgeTask_Update_2ms
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
void VisionBridgeTask_Update_2ms(void)
{
    const volatile vision_ipc_packet_t *packet = VisionIpc_Core0_GetLatest();
    float traveled_mm = 0.0f;
    float err_cmd = 0.0f;
    float speed_cmd = 0.0f;

    if ((g_bridge_vision_task_enable == 0U) &&
        (s_bridge_task.state == VISION_BRIDGE_TASK_IDLE))
    {
        return;
    }

    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U))
    {
        vision_bridge_cleanup(1U);
        return;
    }

    if (s_bridge_task.state == VISION_BRIDGE_TASK_IDLE)
    {
        vision_bridge_enter_task();
    }

    s_bridge_task.state_ticks++;
    traveled_mm = vision_bridge_distance_from(s_bridge_task.start_x_mm, s_bridge_task.start_y_mm);

    switch (s_bridge_task.state)
    {
        case VISION_BRIDGE_TASK_ENTER_PVC:
            err_cmd = g_vision_pvc_control_status.err_degree_cmd;
            speed_cmd = g_vision_pvc_control_status.speed_cmd;

            if (g_vision_pvc_control_status.state == VISION_PVC_CTRL_ARRIVED)
            {
                s_bridge_task.align_ok_ticks++;
            }
            else
            {
                s_bridge_task.align_ok_ticks = 0U;
            }

            if ((s_bridge_task.align_ok_ticks >= 50U) ||
                (s_bridge_task.state_ticks >= VISION_BRIDGE_TASK_ENTER_TIMEOUT_TICKS))
            {
                VisionPvcControl_SetEnable(0U);
                VisionIpc_Core0_SetBridgeLineEnable(1U);
                target_speed_set = 0.0f;
                err_degree = 0.0f;
                s_bridge_task.start_x_mm = inertial_nav.x;
                s_bridge_task.start_y_mm = inertial_nav.y;
                s_bridge_task.locked_yaw_deg = inertial_nav.relative_yaw;
                s_bridge_task.exit_lost_ticks = 0U;
                s_bridge_task.bridge_hold_ticks = 0U;
                vision_bridge_set_state(VISION_BRIDGE_TASK_ALIGN);
            }
            break;

        case VISION_BRIDGE_TASK_ALIGN:
            speed_cmd = VISION_BRIDGE_TASK_ALIGN_SPEED_SET;
            target_speed_set = speed_cmd;

            if (packet->line_bridge_stable_detected)
            {
                s_bridge_task.bridge_hold_ticks = VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS;
                err_cmd = vision_bridge_calc_yaw_hold_err();
                err_degree = err_cmd;
                vision_bridge_apply_high_posture();
                vision_bridge_set_state(VISION_BRIDGE_TASK_RUN);
                break;
            }

            if (packet->line_stable_detected)
            {
                err_cmd = vision_bridge_calc_line_err_degree(packet);
                err_degree = err_cmd;
                if ((vision_bridge_abs_f(err_cmd) <= VISION_BRIDGE_TASK_ALIGN_ERR_TOL_DEG) &&
                    (vision_bridge_abs_f((float)packet->line_yaw_error_deg_x100 * 0.01f) <=
                     VISION_BRIDGE_TASK_ALIGN_YAW_TOL_DEG))
                {
                    s_bridge_task.align_ok_ticks++;
                }
                else
                {
                    s_bridge_task.align_ok_ticks = 0U;
                }
            }
            else
            {
                err_cmd = vision_bridge_calc_yaw_hold_err();
                err_degree = err_cmd;
                s_bridge_task.align_ok_ticks = 0U;
            }

            if ((s_bridge_task.align_ok_ticks >= VISION_BRIDGE_TASK_ALIGN_OK_TICKS) ||
                (s_bridge_task.state_ticks >= VISION_BRIDGE_TASK_ALIGN_TIMEOUT_TICKS))
            {
                s_bridge_task.start_x_mm = inertial_nav.x;
                s_bridge_task.start_y_mm = inertial_nav.y;
                s_bridge_task.locked_yaw_deg = inertial_nav.relative_yaw;
                s_bridge_task.exit_lost_ticks = 0U;
                vision_bridge_set_state(VISION_BRIDGE_TASK_RUN);
            }
            break;

        case VISION_BRIDGE_TASK_RUN:
            if (packet->line_bridge_stable_detected)
            {
                s_bridge_task.bridge_hold_ticks = VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS;
            }
/*
 * 函数: if
 * 说明: 该函数属于视觉模块内部流程，负责当前步骤的数据处理与状态更新。
 * 注意: 仅补充注释，不改变原有算法与控制逻辑。
 */
            else if (s_bridge_task.bridge_hold_ticks > 0U)
            {
                s_bridge_task.bridge_hold_ticks--;
            }

            if (s_bridge_task.bridge_hold_ticks > 0U)
            {
                vision_bridge_apply_high_posture();
                err_cmd = vision_bridge_calc_yaw_hold_err();
                speed_cmd = VISION_BRIDGE_TASK_BRIDGE_SPEED_SET;
            }
            else
            {
                vision_bridge_apply_normal_posture();
                if (packet->line_stable_detected)
                {
                    err_cmd = vision_bridge_calc_line_err_degree(packet);
                    speed_cmd = VISION_BRIDGE_TASK_RUN_SPEED_SET;
                }
                else
                {
                    err_cmd = vision_bridge_calc_yaw_hold_err();
                    speed_cmd = VISION_BRIDGE_TASK_BLIND_SPEED_SET;
                }
            }

            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if (traveled_mm >= VISION_BRIDGE_TASK_RUN_MIN_MM)
            {
                const uint8 no_visual = (uint8)((packet->line_stable_detected == 0U) &&
                                                (packet->line_bridge_stable_detected == 0U) &&
                                                (s_bridge_task.bridge_hold_ticks == 0U) &&
                                                (packet->line_roi_white_ratio_u16 <=
                                                 VISION_BRIDGE_TASK_EXIT_WHITE_RATIO_U16));
                if (no_visual)
                {
                    s_bridge_task.exit_lost_ticks++;
                }
                else
                {
                    s_bridge_task.exit_lost_ticks = 0U;
                }
            }

            if ((s_bridge_task.exit_lost_ticks >= VISION_BRIDGE_TASK_EXIT_LOST_TICKS) ||
                (traveled_mm >= VISION_BRIDGE_TASK_RUN_MAX_MM))
            {
                vision_bridge_set_state(VISION_BRIDGE_TASK_EXIT);
            }
            break;

        case VISION_BRIDGE_TASK_EXIT:
            vision_bridge_apply_normal_posture();
            err_cmd = vision_bridge_calc_yaw_hold_err();
            speed_cmd = VISION_BRIDGE_TASK_EXIT_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if ((vision_bridge_distance_from(s_bridge_task.exit_start_x_mm,
                                             s_bridge_task.exit_start_y_mm) >=
                 VISION_BRIDGE_TASK_EXIT_BUFFER_MM) &&
                (vision_bridge_abs_f(servo_height - bridge_params.height_normal) < 0.2f))
            {
#if VISION_BRIDGE_TASK_NAV_CORRECT_ENABLE
                vision_bridge_apply_nav_correction();
#endif
                vision_bridge_set_state(VISION_BRIDGE_TASK_FINISH);
            }
            break;

        case VISION_BRIDGE_TASK_FINISH:
            vision_bridge_cleanup(1U);
            break;

        case VISION_BRIDGE_TASK_FAILSAFE:
        default:
            vision_bridge_cleanup(1U);
            break;
    }

    vision_bridge_publish_status(packet, traveled_mm, err_cmd, speed_cmd);
}

#endif
