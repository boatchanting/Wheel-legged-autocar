/*
 * =================================================================================
 * 文件: vision_slope_control.c
 * 作用: 0 核(Core 0)斜坡路段视觉识别与控制状态机。
 * 说明: 白色 PVC 斜坡入口阶段复用现有 PVC 控制模块修正方向；检测稳定后，
 *       立即锁定当前惯导航向。蓝色坡面不做视觉识别，前 300ms 后方向误差清零，
 *       依靠定速和惯导里程完成上坡及下坡并自动退出。
 * =================================================================================
 */
#include "vision/vision_slope_control.h"
#include "vision/vision_ipc_core0.h"
#include "vision/vision_pvc_control.h"
#include "navigation/inertial_nav.h"
#include "tools/sbus.h"

#if VISION_SLOPE_TASK_ENABLE

/* --- 外部变量引用 --- */
/* 这些变量由底层控制主循环维护；本状态机只在激活期间写入目标指令。 */
extern volatile float err_degree;
extern volatile float target_speed_set;
extern int g_motor_enable;
extern uint8 g_special_action_trigger;

/* --- 全局状态 --- */
volatile uint8 g_slope_vision_task_enable = 0U;
volatile vision_slope_task_status_t g_slope_vision_task_status = {0};

/* --- 内部状态 --- */
typedef struct
{
    vision_slope_task_state_e state;     /* 当前状态 */
    uint32 state_ticks;                   /* 当前状态的 2ms 计时 */
    float start_x_mm;                     /* 锁角进入斜坡瞬间的惯导 X 坐标 */
    float start_y_mm;                     /* 锁角进入斜坡瞬间的惯导 Y 坐标 */
    float locked_yaw_deg;                 /* PVC 校准完成后锁定的惯导航向 */
    uint16 pvc_align_ok_ticks;            /* PVC 入口确认条件连续满足的 2ms tick 数 */
    float exit_anchor_x_mm;
    float exit_anchor_y_mm;
    uint8_t exit_anchor_valid;
} vision_slope_task_ctx_t;

static vision_slope_task_ctx_t s_slope_task;

/* --- 基础数学工具函数 --- */

/**
 * @brief 将角度归一化到 -180 至 180 度，保证跨越正负 180 度时不会反向猛打方向。
 */
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

/**
 * @brief 将数值限制在给定范围，防止锁角误差过大时方向输出突变。
 */
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

/**
 * @brief 计算车辆距锁角起点的惯导平面距离，单位为 mm。
 */
static float vision_slope_distance_from(float x_mm, float y_mm)
{
    const float dx = inertial_nav.x - x_mm;
    const float dy = inertial_nav.y - y_mm;
    return sqrtf(dx * dx + dy * dy);
}

/**
 * @brief 计算锁定航向所需的方向误差，并限制到安全输出范围。
 */
static float vision_slope_calc_yaw_hold_err(void)
{
    const float yaw_error = vision_slope_normalize_angle(
        s_slope_task.locked_yaw_deg - inertial_nav.relative_yaw);
    return vision_slope_constrain_f(yaw_error,
                                    -VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG,
                                    VISION_SLOPE_TASK_YAW_HOLD_MAX_ERR_DEG);
}

/**
 * @brief 切换状态并从零开始计时，避免不同阶段复用旧计时值。
 */
static void vision_slope_set_state(vision_slope_task_state_e next_state)
{
    s_slope_task.state = next_state;
    s_slope_task.state_ticks = 0U;

    /* PVC 入口确认完成的瞬间记录航向和位置，后续里程从这里开始累计。 */
    if (next_state == VISION_SLOPE_TASK_ENTRY_HOLD)
    {
        s_slope_task.start_x_mm = inertial_nav.x;
        s_slope_task.start_y_mm = inertial_nav.y;
        s_slope_task.locked_yaw_deg = inertial_nav.relative_yaw;
    }
}

/**
 * @brief 将内部状态一次性同步到全局状态，便于调试读取。
 */
static void vision_slope_publish_status(const volatile vision_ipc_packet_t *packet,
                                        float traveled_mm,
                                        float err_cmd,
                                        float speed_cmd)
{
    g_slope_vision_task_status.enabled = g_slope_vision_task_enable;
    g_slope_vision_task_status.state = s_slope_task.state;
    g_slope_vision_task_status.state_ticks = s_slope_task.state_ticks;
    g_slope_vision_task_status.last_seq = (packet != NULL) ? packet->seq : 0U;
    g_slope_vision_task_status.traveled_mm = traveled_mm;
    g_slope_vision_task_status.locked_yaw_deg = s_slope_task.locked_yaw_deg;
    g_slope_vision_task_status.err_degree_cmd = err_cmd;
    g_slope_vision_task_status.speed_cmd = speed_cmd;
    g_slope_vision_task_status.pvc_stable_detected = g_vision_pvc_control_status.stable_detected;
    g_slope_vision_task_status.pvc_ratio_u16 = g_vision_pvc_control_status.bbox_area_ratio_u16;
    g_slope_vision_task_status.pvc_steer_error_px_x100 = g_vision_pvc_control_status.steer_error_px_x100;
}

/**
 * @brief 释放 PVC 视觉控制与特殊任务标记；stop_car 为 1 时同时输出停车指令。
 */
static void vision_slope_cleanup(uint8 stop_car)
{
    /* 释放 PVC 控制，并恢复 PVC 检测的项目默认调度状态，保证下次任务可重新触发。 */
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
    memset((void *)&g_slope_vision_task_status, 0, sizeof(g_slope_vision_task_status));
    g_slope_vision_task_status.state = VISION_SLOPE_TASK_IDLE;
}

/**
 * @brief 进入 PVC 校准阶段：先启动既有 PVC 检测/方向控制，再由斜坡任务覆盖速度。
 */
static void vision_slope_enter_task(void)
{
    memset(&s_slope_task, 0, sizeof(s_slope_task));
    s_slope_task.state = VISION_SLOPE_TASK_PVC_ALIGN;
    s_slope_task.locked_yaw_deg = inertial_nav.relative_yaw;
    g_special_action_trigger = 1U;

    /* PVC 控制模块会提供方向误差；本状态机在本周期末统一强制入口速度。 */
    VisionIpc_Core0_SetPvcEnable(1U);
    VisionPvcControl_SetEnable(1U);
}

void VisionSlopeTask_SetExitAnchor(float x_mm, float y_mm)
{
    if (s_slope_task.state == VISION_SLOPE_TASK_IDLE)
    {
        s_slope_task.exit_anchor_x_mm = x_mm;
        s_slope_task.exit_anchor_y_mm = y_mm;
        s_slope_task.exit_anchor_valid = 1U;
    }
}

/* --- 对外接口函数 --- */

void VisionSlopeTask_Init(void)
{
    memset(&s_slope_task, 0, sizeof(s_slope_task));
    memset((void *)&g_slope_vision_task_status, 0, sizeof(g_slope_vision_task_status));
    g_slope_vision_task_enable = 0U;
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

/**
 * @brief 斜坡任务的核心状态机，每 2ms 执行一次。
 *
 * 流程：PVC 白色入口校准 -> 锁定当前航向 -> -600 行驶 300ms -> -400 锁角行驶 -> 2m 自动退出。
 */
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

    /* 急停、未使能电机或惯导尚未完成航向初始化时，立即安全退出。 */
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

    if ((s_slope_task.state == VISION_SLOPE_TASK_ENTRY_HOLD) ||
        (s_slope_task.state == VISION_SLOPE_TASK_RUN))
    {
        traveled_mm = vision_slope_distance_from(s_slope_task.start_x_mm,
                                                 s_slope_task.start_y_mm);
    }

    switch (s_slope_task.state)
    {
        case VISION_SLOPE_TASK_PVC_ALIGN:
            /* 复用 PVC 控制模块给出的方向修正；搜索/校准期间以低速行驶，给转向收敛留出距离。 */
            err_cmd = g_vision_pvc_control_status.err_degree_cmd;
            speed_cmd = VISION_SLOPE_TASK_PVC_ALIGN_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            /*
             * 沿用参考提交的入口确认逻辑：稳定看到 PVC 且白色区域占满画面才表示已压上入口。
             * 入口条件连续保持 50ms，可滤除临界画面抖动，避免过早锁住尚未进入斜坡的航向。
             */
            if ((g_vision_pvc_control_status.stable_detected != 0U) &&
                (g_vision_pvc_control_status.bbox_area_ratio_u16 >= VISION_SLOPE_TASK_PVC_FULL_RATIO_U16))
            {
                if (s_slope_task.pvc_align_ok_ticks < 0xFFFFU)
                {
                    s_slope_task.pvc_align_ok_ticks++;
                }
            }
            else
            {
                s_slope_task.pvc_align_ok_ticks = 0U;
            }

            /* PVC 入口确认稳定 50ms 后，保存此刻航向；后续蓝色坡面不再依赖视觉。 */
            if (s_slope_task.pvc_align_ok_ticks >= VISION_SLOPE_TASK_PVC_ALIGN_OK_TICKS)
            {
                VisionPvcControl_SetEnable(0U);
                VisionIpc_Core0_SetPvcEnable(VISION_PVC_DETECT_DEFAULT_ACTIVE);
                vision_slope_set_state(VISION_SLOPE_TASK_ENTRY_HOLD);
            }
            else if (s_slope_task.state_ticks >= VISION_SLOPE_TASK_PVC_ALIGN_TIMEOUT_TICKS)
            {
                /* 5 秒内仍未确认进入 PVC，认为视觉入口异常并安全退出。 */
                vision_slope_set_state(VISION_SLOPE_TASK_FAILSAFE);
            }
            break;

        case VISION_SLOPE_TASK_ENTRY_HOLD:
            /* 刚压上斜坡的 300ms 保持较高入口速度，并只根据惯导航向锁角。 */
            err_cmd = vision_slope_calc_yaw_hold_err();
            speed_cmd = VISION_SLOPE_TASK_ENTRY_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if (s_slope_task.state_ticks >= VISION_SLOPE_TASK_ENTRY_HOLD_TICKS)
            {
                vision_slope_set_state(VISION_SLOPE_TASK_RUN);
            }
            break;

        case VISION_SLOPE_TASK_RUN:
            /* 蓝色斜坡路面不使用视觉或惯导方向修正，直接保持方向误差为零。 */
            err_cmd = 0.0f;
            speed_cmd = VISION_SLOPE_TASK_RUN_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            if (traveled_mm >= VISION_SLOPE_TASK_EXIT_DISTANCE_MM)
            {
                vision_slope_set_state(VISION_SLOPE_TASK_FINISH);
            }
            break;

        case VISION_SLOPE_TASK_FINISH:
            /* 退出前如果有锚点信息，直接修正融合坐标 */
            if (s_slope_task.exit_anchor_valid != 0U)
            {
                extern float nav_vision_fusion_x;
                extern float nav_vision_fusion_y;
                extern volatile uint8 exit_beep_request;
                nav_vision_fusion_x = s_slope_task.exit_anchor_x_mm;
                nav_vision_fusion_y = s_slope_task.exit_anchor_y_mm;
                exit_beep_request = 1U;
            }
            /* 保留上一周期的锁角和速度输出，让主控逻辑在下一周期平滑接管。 */
            vision_slope_cleanup(0U);
            return;

        case VISION_SLOPE_TASK_FAILSAFE:
        default:
            vision_slope_cleanup(1U);
            return;
    }

    vision_slope_publish_status(packet, traveled_mm, err_cmd, speed_cmd);
}

#endif
