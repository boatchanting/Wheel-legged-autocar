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
extern volatile uint8 entry_beep_request;
extern volatile uint8 exit_beep_request;

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
    float entry_yaw_deg;                  /* 进入状态机时刻锁存的航向(进入段PID 基准) */
    uint16 pvc_align_ok_ticks;            /* PVC 入口确认条件连续满足的 2ms tick 数 */
} vision_slope_task_ctx_t;

static vision_slope_task_ctx_t s_slope_task;

/* 进入段物理域 PID 微分状态（PVC_ALIGN 期间使用） */
static float s_entry_pid_psi_prev_rad = 0.0f;   /* 上一拍航向误差(rad) */
static float s_entry_pid_psi_dot_f    = 0.0f;   /* 滤波后航向误差微分(rad/s) */
static uint8 s_entry_pid_first_valid  = 0U;     /* 首个有效帧标志 */

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
 * @brief 进入段物理域 PID 方向：ω = P·ψ_err + D·ψ_err' + K_E·e
 *        e = D·sin(β−ψ_err)（f87b18b 修复公式，物理域距离归一化）
 *        输入直接用 IPC 的 pvc_phy_x_mm/pvc_phy_y_mm（1核 fill_pvc 已填充）。
 * @param packet 1 核最新数据包
 * @return err_degree（无效帧返回 0，调用方直行回退）
 */
static float vision_slope_entry_pid_update(const volatile vision_ipc_packet_t *packet)
{
    const float deg2rad = 0.0174532925f;
    const float fx = (float)packet->pvc_phy_x_mm;
    const float fy = (float)packet->pvc_phy_y_mm;
    const float dist_m = sqrtf(fx * fx + fy * fy) / 1000.0f;
    float beta_rad;
    float psi_err_rad;
    float e_m;
    float omega;
    float err_deg;

    /* 航向误差只依赖 IMU，每拍都算并发布（诊断） */
    psi_err_rad = vision_slope_normalize_angle(
        s_slope_task.entry_yaw_deg - inertial_nav.relative_yaw) * deg2rad;
    g_slope_vision_task_status.entry_psi_err_deg = psi_err_rad * 57.29578f;

    /* 门控：phy 无效或超出检测距离 → 本拍无视觉，直行回退 */
    if ((packet->pvc_phy_x_mm == VISION_SLOPE_ENTRY_PID_PHY_INVALID_MM) ||
        (packet->pvc_phy_y_mm == VISION_SLOPE_ENTRY_PID_PHY_INVALID_MM) ||
        (dist_m > VISION_SLOPE_ENTRY_PID_DETECT_RANGE_M))
    {
        return 0.0f;
    }

    /* 物理横向偏差（投影到入口基准系：e = D·sin(β − ψ_err)） */
    beta_rad = atan2f(fx, fy);
    e_m = dist_m * sinf(beta_rad - psi_err_rad);
    g_slope_vision_task_status.entry_e_m = e_m;

    /* 航向微分（2ms 差分 + 一阶低通，防 IMU 噪声放大） */
    if (s_entry_pid_first_valid != 0U)
    {
        const float psi_err_dot = (psi_err_rad - s_entry_pid_psi_prev_rad) *
                                  VISION_SLOPE_ENTRY_PID_CTRL_HZ;
        const float alpha = (1.0f / VISION_SLOPE_ENTRY_PID_CTRL_HZ) /
                            (VISION_SLOPE_ENTRY_PID_TAU_S + 1.0f / VISION_SLOPE_ENTRY_PID_CTRL_HZ);
        s_entry_pid_psi_dot_f += alpha * (psi_err_dot - s_entry_pid_psi_dot_f);
    }
    s_entry_pid_psi_prev_rad = psi_err_rad;
    s_entry_pid_first_valid = 1U;

    /* 控制律：ω = P·ψ_err + D·ψ_err' + K_E·e */
    omega = VISION_SLOPE_ENTRY_PID_P_PSI * psi_err_rad +
            VISION_SLOPE_ENTRY_PID_D_PSI * s_entry_pid_psi_dot_f +
            VISION_SLOPE_ENTRY_PID_K_E * e_m;
    omega = vision_slope_constrain_f(omega, -VISION_SLOPE_ENTRY_PID_W_MAX_RADPS,
                                     VISION_SLOPE_ENTRY_PID_W_MAX_RADPS);

    /* ω → err_degree（TURN_ANG_KP 分子分母抵消 → 角度环输出恒 = ω·57.29578） */
    err_deg = omega * 57.29578f / TURN_ANG_KP;
    return vision_slope_constrain_f(err_deg,
                                    -VISION_SLOPE_ENTRY_PID_W_MAX_RADPS * 57.29578f / 8.0f,
                                    VISION_SLOPE_ENTRY_PID_W_MAX_RADPS * 57.29578f / 8.0f);
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
    s_slope_task.entry_yaw_deg = inertial_nav.relative_yaw; /* 进入段PID 航向基准 */
    g_special_action_trigger = 1U;
    entry_beep_request = 1U;

    /* 复位进入段 PID 微分状态，防止上次任务残留 */
    s_entry_pid_psi_prev_rad = 0.0f;
    s_entry_pid_psi_dot_f = 0.0f;
    s_entry_pid_first_valid = 0U;

    /* PVC 控制模块会提供方向误差；本状态机在本周期末统一强制入口速度。 */
    VisionIpc_Core0_SetPvcEnable(1U);
    VisionPvcControl_SetEnable(1U);
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
#if VISION_SLOPE_ENTRY_PID_ENABLE
            /* 物理域 PID：消费 IPC 的 pvc_phy_x_mm/y_mm（距离归一化 + 航向保持） */
            err_cmd = vision_slope_entry_pid_update(packet);
#else
            /* 原逻辑：复用 PVC 控制模块给出的像素域方向修正 */
            err_cmd = g_vision_pvc_control_status.err_degree_cmd;
#endif
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
            exit_beep_request = 1U;
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
