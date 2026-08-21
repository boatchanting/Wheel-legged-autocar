/*
 * =================================================================================
 * 文件: vision_three_stage_control.c
 * 作用: 三级跳视觉融合状态机（0 核）
 * 设计原则:
 *   - 只做方向与跳跃触发，不修改 target_speed_set；
 *   - 外部单标志位触发，内部完成三次跳跃时序；
 *   - 通过 PVC 行号阈值触发：下边界/上边界/下边界；
 *   - 通过“PVC 短暂丢失后再出现”过滤第二次与第三次之间的黑区阶段。
 * =================================================================================
 */
#include "vision/vision_three_stage_control.h"

#if VISION_THREE_STAGE_CONTROL_ENABLE

#include "../config/sys_options.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_three_stage_distance.h"
#include "servo/servo_jump.h"
#include "navigation/inertial_nav.h"

extern volatile uint8 exit_beep_request;

// /* ---------------- 外部控制量 ---------------- */
// extern volatile float err_degree; /* 底盘方向控制输入 */
// extern int g_motor_enable;        /* 电机使能 */
// extern bool g_yaw_initialized;   /* 姿态初始化状态 */
// extern uint8 g_special_action_trigger; /* 导航特殊动作占用标志 */

/* ---------------- 全局状态 ---------------- */
volatile vision_three_stage_control_status_t g_vision_three_stage_control_status = {0};
volatile uint8 g_vision_three_stage_control_enable = VISION_THREE_STAGE_CONTROL_DEFAULT_ACTIVE;

volatile uint8 g_vision_three_stage_jump1_bottom_y = VISION_THREE_STAGE_JUMP1_BOTTOM_Y_DEFAULT;
volatile uint8 g_vision_three_stage_jump2_top_y = VISION_THREE_STAGE_JUMP2_TOP_Y_DEFAULT;
volatile uint8 g_vision_three_stage_jump3_bottom_y = VISION_THREE_STAGE_JUMP3_BOTTOM_Y_DEFAULT;
volatile uint8 g_vision_three_stage_exit_top_y = VISION_THREE_STAGE_EXIT_TOP_Y_DEFAULT;
volatile uint8 g_vision_three_stage_jump1_correction_bottom_y = VISION_THREE_STAGE_JUMP1_CORRECTION_BOTTOM_Y_DEFAULT;

/* 第二跳触发策略：默认固定延时（宏默认值），可在线置 0 回退旧视觉 top_y 阈值策略 */
volatile uint8 g_vision_three_stage_jump2_delay_enable = VISION_THREE_STAGE_JUMP2_DELAY_ENABLE_DEFAULT;

volatile float g_vision_three_stage_speed_approach = -310.0f; /* 锁定目标靠近时的速度 */
volatile float g_vision_three_stage_speed_jump1    = -310.0f;/* 第一跳寻找速度 */
volatile float g_vision_three_stage_speed_jump2    = -310.0f;/* 第二跳寻找速度 */
volatile float g_vision_three_stage_speed_gap      = -310.0f; /* 短暂丢失过渡阶段速度 */
volatile float g_vision_three_stage_speed_jump3    = -310.0f;/* 第三跳寻找速度 */
volatile float g_vision_three_stage_speed_exit     = -310.0f; /* 最后一跳完成后的驶出减速 */

/* 影子变量：先算完，再一次性发布，减少并发读写中间态 */
static vision_three_stage_control_status_t s_ctrl_shadow;
static uint8 s_jump1_correction_filter_valid = 0U;
static float s_exit_anchor_x_mm = 0.0f;
static float s_exit_anchor_y_mm = 0.0f;
static float s_post_exit_start_x_mm = 0.0f;
static float s_post_exit_start_y_mm = 0.0f;
static float s_jump1_start_x_mm = 0.0f;
static float s_jump1_start_y_mm = 0.0f;
static float s_locked_relative_yaw_deg = 0.0f;
static uint8 s_exit_anchor_valid = 0U;

/* ---------------- 工具函数 ---------------- */
static float vision_three_stage_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

static float vision_three_stage_constrain_f(float value, float min_value, float max_value)
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

static float vision_three_stage_normalize_angle(float angle)
{
    while (angle > 180.0f)
    {
        angle -= 360.0f;
    }
    while (angle <= -180.0f)
    {
        angle += 360.0f;
    }
    return angle;
}

static void vision_three_stage_publish_status(void)
{
    g_vision_three_stage_control_status = s_ctrl_shadow;
}

static void vision_three_stage_set_state(vision_three_stage_ctrl_state_e next_state)
{
    s_ctrl_shadow.state = next_state;
    s_ctrl_shadow.state_ticks = 0U;
    s_ctrl_shadow.stable_count = 0U;
    s_ctrl_shadow.lost_count = 0U;
}

static void vision_three_stage_stop_internal(vision_three_stage_exit_reason_e reason)
{
    s_ctrl_shadow.active = 0U;
    s_ctrl_shadow.state = VISION_THREE_STAGE_CTRL_IDLE;
    s_ctrl_shadow.state_ticks = 0U;
    s_ctrl_shadow.stable_count = 0U;
    s_ctrl_shadow.lost_count = 0U;
    s_ctrl_shadow.black_gap_seen = 0U;
    s_ctrl_shadow.jump_cooldown_ticks = 0U;
    s_ctrl_shadow.exit_reason = reason;
    s_ctrl_shadow.pvc_lateral_filtered_mm = 0.0f;
    s_ctrl_shadow.err_degree_cmd = 0.0f;
    s_jump1_correction_filter_valid = 0U;
    s_exit_anchor_valid = 0U;

    /* 退出时释放方向控制，避免残留转向量 */
    err_degree = 0.0f;
    target_speed_set = 0.0f;

    /* 关闭本任务的视觉目标，归还上层调度权 */
    VisionIpc_Core0_SetTask(VISION_TARGET_NONE, 0U);
    g_special_action_trigger = 0U;
}

static uint8 vision_three_stage_jump1_correction_allowed(void)
{
    return (uint8)(
        ((s_ctrl_shadow.state == VISION_THREE_STAGE_CTRL_WAIT_PVC_LOCK) ||
         (s_ctrl_shadow.state == VISION_THREE_STAGE_CTRL_WAIT_JUMP1_BOTTOM)) &&
        (g_vision_three_stage_jump1_correction_bottom_y < g_vision_three_stage_jump1_bottom_y) &&
        (s_ctrl_shadow.pvc_stable_detected != 0U) &&
        (s_ctrl_shadow.pvc_entry_bottom_y < g_vision_three_stage_jump1_correction_bottom_y));
}

static void vision_three_stage_apply_err_from_pvc(uint8 packet_new)
{
    float err;

    if (vision_three_stage_jump1_correction_allowed() != 0U)
    {
        if (packet_new != 0U)
        {
            if (s_jump1_correction_filter_valid == 0U)
            {
                s_ctrl_shadow.pvc_lateral_filtered_mm = (float)s_ctrl_shadow.pvc_lateral_mm;
                s_jump1_correction_filter_valid = 1U;
            }
            else
            {
                s_ctrl_shadow.pvc_lateral_filtered_mm +=
                    VISION_THREE_STAGE_JUMP1_CORRECTION_LPF_ALPHA *
                    ((float)s_ctrl_shadow.pvc_lateral_mm - s_ctrl_shadow.pvc_lateral_filtered_mm);
            }
        }

        err = VISION_THREE_STAGE_JUMP1_CORRECTION_LATERAL_SIGN *
              s_ctrl_shadow.pvc_lateral_filtered_mm *
              VISION_THREE_STAGE_JUMP1_CORRECTION_K_LAT_DEG_PER_MM;
        err = vision_three_stage_constrain_f(
            err,
            -VISION_THREE_STAGE_JUMP1_CORRECTION_MAX_ERR_DEG,
            VISION_THREE_STAGE_JUMP1_CORRECTION_MAX_ERR_DEG);

        if (vision_three_stage_abs_f(err) < VISION_THREE_STAGE_DEADBAND_DEG)
        {
            err = 0.0f;
        }
        s_ctrl_shadow.err_degree_cmd = err;
    }
    else
    {
        /* 暂失目标时平滑衰减，避免方向瞬变 */
        s_ctrl_shadow.err_degree_cmd *= 0.90f;
        if (vision_three_stage_abs_f(s_ctrl_shadow.err_degree_cmd) < 0.05f)
        {
            s_ctrl_shadow.err_degree_cmd = 0.0f;
        }
    }

    err_degree = s_ctrl_shadow.err_degree_cmd;
}

static void vision_three_stage_apply_locked_heading(void)
{
    s_ctrl_shadow.err_degree_cmd = vision_three_stage_normalize_angle(
        s_locked_relative_yaw_deg - inertial_nav.relative_yaw);
    err_degree = s_ctrl_shadow.err_degree_cmd;
}

static uint8 vision_three_stage_try_trigger_step_jump(void)
{
    if ((jump_flag == 0U) &&
        (s_ctrl_shadow.jump_cooldown_ticks >= VISION_THREE_STAGE_JUMP_COOLDOWN_TICKS))
    {
        jump_trigger_with_type(JUMP_TYPE_STEP_UP);
        s_ctrl_shadow.jump_count++;
        s_ctrl_shadow.jump_cooldown_ticks = 0U;
        return 1U;
    }
    return 0U;
}

/* ---------------- 对外接口 ---------------- */
void VisionThreeStageControl_Init(void)
{
    memset(&s_ctrl_shadow, 0, sizeof(s_ctrl_shadow));
    s_ctrl_shadow.enabled = g_vision_three_stage_control_enable ? 1U : 0U;
    s_ctrl_shadow.state = VISION_THREE_STAGE_CTRL_IDLE;
    s_ctrl_shadow.exit_reason = VISION_THREE_STAGE_EXIT_NONE;
    vision_three_stage_publish_status();
}

void VisionThreeStageControl_SetEnable(uint8 enable)
{
    g_vision_three_stage_control_enable = enable ? 1U : 0U;
    s_ctrl_shadow.enabled = g_vision_three_stage_control_enable;

    if (g_vision_three_stage_control_enable == 0U)
    {
        vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_MANUAL_STOP);
    }

    vision_three_stage_publish_status();
}

uint8 VisionThreeStageControl_IsEnabled(void)
{
    return g_vision_three_stage_control_enable;
}

void VisionThreeStageControl_Start(void)
{
    if (g_vision_three_stage_control_enable == 0U)
    {
        return;
    }

    if (s_ctrl_shadow.active != 0U)
    {
        return;
    }

    memset(&s_ctrl_shadow, 0, sizeof(s_ctrl_shadow));
    s_ctrl_shadow.enabled = 1U;
    s_ctrl_shadow.active = 1U;
    s_ctrl_shadow.exit_reason = VISION_THREE_STAGE_EXIT_NONE;
    s_ctrl_shadow.jump_cooldown_ticks = VISION_THREE_STAGE_JUMP_COOLDOWN_TICKS;
    s_locked_relative_yaw_deg = inertial_nav.relative_yaw;
    s_jump1_start_x_mm = inertial_nav.x;
    s_jump1_start_y_mm = inertial_nav.y;

#if (THREE_STAGE_JUMP1_TRIGGER_MODE == 2U)
    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_JUMP1_DISTANCE);
#else
    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_PVC_LOCK);
#endif

    /* 切到 PVC 视觉任务，获取入口 top/bottom 行号 */
    VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);

    /* 告诉导航层：此时由特殊动作状态机占用控制权 */
    g_special_action_trigger = 1U;
    vision_three_stage_publish_status();
}

void VisionThreeStageControl_Stop(void)
{
    vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_MANUAL_STOP);
    vision_three_stage_publish_status();
}

uint8 VisionThreeStageControl_IsActive(void)
{
    return s_ctrl_shadow.active;
}

void VisionThreeStageControl_SetExitAnchor(float x_mm, float y_mm)
{
    if (s_ctrl_shadow.active == 0U)
    {
        s_exit_anchor_x_mm = x_mm;
        s_exit_anchor_y_mm = y_mm;
        s_exit_anchor_valid = 1U;
    }
}

void VisionThreeStageControl_Update_2ms(void)
{
    const volatile vision_ipc_packet_t *packet;
    uint8 packet_new;
    uint8 pvc_valid;

    s_ctrl_shadow.enabled = g_vision_three_stage_control_enable ? 1U : 0U;

    if (s_ctrl_shadow.enabled == 0U)
    {
        vision_three_stage_publish_status();
        return;
    }

    if (s_ctrl_shadow.active == 0U)
    {
        vision_three_stage_publish_status();
        return;
    }

    #if REMOTE_CONTROL == 1
    if (robot_ctrl.brake_active != 0U)//遥控器刹车生效时退出状态机
    {
        vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_MOTOR_OFF);
        vision_three_stage_publish_status();
        return;
    }
    #endif

    if (g_motor_enable == 0)
    {
        vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_MOTOR_OFF);
        vision_three_stage_publish_status();
        return;
    }

    if (g_yaw_initialized == 0U)
    {
        vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_YAW_INVALID);
        vision_three_stage_publish_status();
        return;
    }

    packet = VisionIpc_Core0_GetLatest();
    packet_new = (uint8)(packet->seq != s_ctrl_shadow.last_seq);

    if (packet_new != 0U)
    {
        s_ctrl_shadow.last_seq = packet->seq;
        s_ctrl_shadow.stale_ticks = 0U;
    }
    else if (s_ctrl_shadow.stale_ticks < 0xFFFFU)
    {
        s_ctrl_shadow.stale_ticks++;
    }

    if (s_ctrl_shadow.jump_cooldown_ticks < 0xFFFFU)
    {
        s_ctrl_shadow.jump_cooldown_ticks++;
    }

    if (s_ctrl_shadow.state_ticks < 0xFFFFU)
    {
        s_ctrl_shadow.state_ticks++;
    }

    if ((s_ctrl_shadow.state != VISION_THREE_STAGE_CTRL_POST_EXIT_RUNOUT) &&
        (s_ctrl_shadow.stale_ticks > VISION_THREE_STAGE_STALE_TIMEOUT_TICKS))
    {
        vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_STALE);
        vision_three_stage_publish_status();
        return;
    }

    if (s_ctrl_shadow.state_ticks > VISION_THREE_STAGE_STATE_TIMEOUT_TICKS)
    {
        vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_TIMEOUT);
        vision_three_stage_publish_status();
        return;
    }

    pvc_valid = (uint8)((packet->valid_mask & VISION_VALID_PVC) != 0U);
    if (pvc_valid != 0U)
    {
        s_ctrl_shadow.pvc_stable_detected = packet->pvc_stable_detected;
        s_ctrl_shadow.pvc_raw_detected = packet->pvc_detected;
        s_ctrl_shadow.pvc_entry_bottom_y = packet->pvc_entry_bottom_y;
        s_ctrl_shadow.pvc_entry_top_y = packet->pvc_entry_top_y;
        s_ctrl_shadow.pvc_lateral_mm = packet->pvc_lateral_mm;
        s_ctrl_shadow.pvc_yaw_error_deg_x100 = packet->pvc_yaw_error_deg_x100;
    }
    else
    {
        s_ctrl_shadow.pvc_stable_detected = 0U;
        s_ctrl_shadow.pvc_raw_detected = 0U;
    }

    /* 视觉只控制方向，不控制速度 */
    if (s_ctrl_shadow.state == VISION_THREE_STAGE_CTRL_POST_EXIT_RUNOUT)
    {
        s_ctrl_shadow.err_degree_cmd = 0.0f;
    }
    else
    {
        vision_three_stage_apply_err_from_pvc(packet_new);
    }
    vision_three_stage_apply_locked_heading();

    switch (s_ctrl_shadow.state)
    {
        case VISION_THREE_STAGE_CTRL_WAIT_PVC_LOCK:
        target_speed_set = g_vision_three_stage_speed_approach; /* 设置靠近阶段速度 */
            if (s_ctrl_shadow.pvc_stable_detected != 0U)
            {
                s_ctrl_shadow.stable_count++;
                if (s_ctrl_shadow.stable_count >= VISION_THREE_STAGE_LOCK_STABLE_FRAMES)
                {
                    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_JUMP1_BOTTOM);
                }
            }
            else
            {
                s_ctrl_shadow.stable_count = 0U;
            }
            break;

        case VISION_THREE_STAGE_CTRL_WAIT_JUMP1_DISTANCE:
            target_speed_set = g_vision_three_stage_speed_jump1; /* 设置第一跳速度 */
            if (VisionThreeStageJump1DistanceReached(
                    s_jump1_start_x_mm,
                    s_jump1_start_y_mm,
                    inertial_nav.x,
                    inertial_nav.y,
                    THREE_STAGE_JUMP1_INERTIAL_DISTANCE_MM) != 0U)
            {
                if (vision_three_stage_try_trigger_step_jump() != 0U)
                {
                    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_JUMP2_TOP);
                }
            }
            break;

        case VISION_THREE_STAGE_CTRL_WAIT_JUMP1_BOTTOM:
            target_speed_set = g_vision_three_stage_speed_jump1; /* 设置第一跳速度 */
            if ((s_ctrl_shadow.pvc_stable_detected != 0U) &&
                (s_ctrl_shadow.pvc_entry_bottom_y >= g_vision_three_stage_jump1_bottom_y))
            {
                if (vision_three_stage_try_trigger_step_jump() != 0U)
                {
                    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_JUMP2_TOP);
                }
            }
            break;

        case VISION_THREE_STAGE_CTRL_WAIT_JUMP2_TOP:
            target_speed_set = g_vision_three_stage_speed_jump2; /* 设置第二跳速度 */
            if (g_vision_three_stage_jump2_delay_enable != 0U)
            {
                /* 固定延时策略（默认）：第一跳触发后延时 VISION_THREE_STAGE_JUMP2_DELAY_AFTER_JUMP1_TICKS 触发第二跳，
                 * 与第三跳的固定延时写法一致。state_ticks 自进入本状态（即第一跳触发）时刻起算。
                 * 注意：受 jump_flag 门控，若延时未到但第一跳动作已结束，会等到延时到点再触发。 */
                if (s_ctrl_shadow.state_ticks >= VISION_THREE_STAGE_JUMP2_DELAY_AFTER_JUMP1_TICKS)
                {
                    if (vision_three_stage_try_trigger_step_jump() != 0U)
                    {
                        vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_SECOND_PVC);
                        s_ctrl_shadow.black_gap_seen = 0U;
                    }
                }
            }
            else
            {
                /* 旧视觉策略：PVC 上边界 top_y 达到阈值时触发第二跳 */
                if ((s_ctrl_shadow.pvc_stable_detected != 0U) &&
                    (s_ctrl_shadow.pvc_entry_top_y >= g_vision_three_stage_jump2_top_y))
                {
                    if (vision_three_stage_try_trigger_step_jump() != 0U)
                    {
                        vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_SECOND_PVC);
                        s_ctrl_shadow.black_gap_seen = 0U;
                    }
                }
            }
            break;

        case VISION_THREE_STAGE_CTRL_WAIT_SECOND_PVC:
            /* 第二跳触发后累计 180ms，直接触发第三跳。 */
            target_speed_set = g_vision_three_stage_speed_jump3;
            if (s_ctrl_shadow.state_ticks >= VISION_THREE_STAGE_JUMP3_DELAY_AFTER_JUMP2_TICKS)
            {
                if (vision_three_stage_try_trigger_step_jump() != 0U)
                {
                    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_EXIT_TOP);
                }
            }

            /* 原第三跳逻辑：PVC 丢失后重新识别，并按 bottom_y 阈值触发。
            if (s_ctrl_shadow.pvc_stable_detected == 0U)
            {
                s_ctrl_shadow.stable_count = 0U;
                if (s_ctrl_shadow.lost_count < 0xFFFFU)
                {
                    s_ctrl_shadow.lost_count++;
                }
                if (s_ctrl_shadow.lost_count >= VISION_THREE_STAGE_BLACK_GAP_LOST_FRAMES)
                {
                    s_ctrl_shadow.black_gap_seen = 1U;
                }
            }
            else
            {
                s_ctrl_shadow.lost_count = 0U;
                if (s_ctrl_shadow.black_gap_seen != 0U)
                {
                    if (s_ctrl_shadow.stable_count < 0xFFFFU)
                    {
                        s_ctrl_shadow.stable_count++;
                    }
                    if (s_ctrl_shadow.stable_count >= VISION_THREE_STAGE_REACQUIRE_STABLE_FRAMES)
                    {
                        vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_JUMP3_BOTTOM);
                    }
                }
            }
            */
            break;

        case VISION_THREE_STAGE_CTRL_WAIT_JUMP3_BOTTOM:
            /* 原第三跳逻辑保留，第三跳现由 WAIT_SECOND_PVC 的 180ms 定时触发。 */
            /*
            target_speed_set = g_vision_three_stage_speed_jump3;
            if ((s_ctrl_shadow.pvc_stable_detected != 0U) &&
                (s_ctrl_shadow.pvc_entry_bottom_y >= g_vision_three_stage_jump3_bottom_y))
            {
                if (vision_three_stage_try_trigger_step_jump() != 0U)
                {
                    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_WAIT_EXIT_TOP);
                }
            }
            */
            break;

        case VISION_THREE_STAGE_CTRL_WAIT_EXIT_TOP:
            target_speed_set = g_vision_three_stage_speed_exit; /* 设置退出阶段速度 */
            if ((s_ctrl_shadow.pvc_stable_detected != 0U) &&
                (s_ctrl_shadow.pvc_entry_top_y >= g_vision_three_stage_exit_top_y))
            {
                s_ctrl_shadow.stable_count++;
                if (s_ctrl_shadow.stable_count >= VISION_THREE_STAGE_EXIT_STABLE_FRAMES)
                {
                    if (s_exit_anchor_valid != 0U)
                    {
                        /* 视觉确认出口时只重定位导航融合坐标。 */
                        nav_vision_fusion_x = s_exit_anchor_x_mm;
                        nav_vision_fusion_y = s_exit_anchor_y_mm;
                    }
                    exit_beep_request = 1U;
                    /* 脱出距离从此刻的原始惯导坐标起算，不受上方融合重定位影响。 */
                    s_post_exit_start_x_mm = inertial_nav.x;
                    s_post_exit_start_y_mm = inertial_nav.y;
                    VisionIpc_Core0_SetTask(VISION_TARGET_NONE, 0U);
                    vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_POST_EXIT_RUNOUT);
                }
            }
            else
            {
                s_ctrl_shadow.stable_count = 0U;
            }
            break;

        case VISION_THREE_STAGE_CTRL_POST_EXIT_RUNOUT:
        {
            float dx;
            float dy;

            target_speed_set = VISION_THREE_STAGE_POST_EXIT_SPEED_SET;
            /* 视觉出口重定位完成后，保持使用原始惯导坐标跑满脱出距离。 */
            dx = inertial_nav.x - s_post_exit_start_x_mm;
            dy = inertial_nav.y - s_post_exit_start_y_mm;
            if ((dx * dx + dy * dy) >=
                (VISION_THREE_STAGE_POST_EXIT_DISTANCE_MM * VISION_THREE_STAGE_POST_EXIT_DISTANCE_MM))
            {
                exit_beep_request = 1U;
                vision_three_stage_set_state(VISION_THREE_STAGE_CTRL_FINISH);
            }
            break;
        }

        case VISION_THREE_STAGE_CTRL_FINISH:
            vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_SUCCESS);
            break;

        case VISION_THREE_STAGE_CTRL_FAILSAFE:
        default:
            vision_three_stage_stop_internal(VISION_THREE_STAGE_EXIT_TIMEOUT);
            break;
    }

    vision_three_stage_publish_status();
}

#else
/* 编译开关关闭时保留空实现，保证链接通过 */
void VisionThreeStageControl_Init(void) {}
void VisionThreeStageControl_SetEnable(uint8 enable) {(void)enable;}
uint8 VisionThreeStageControl_IsEnabled(void) { return 0U; }
void VisionThreeStageControl_Start(void) {}
void VisionThreeStageControl_Stop(void) {}
uint8 VisionThreeStageControl_IsActive(void) { return 0U; }
void VisionThreeStageControl_SetExitAnchor(float x_mm, float y_mm) {(void)x_mm; (void)y_mm;}
void VisionThreeStageControl_Update_2ms(void) {}
#endif
