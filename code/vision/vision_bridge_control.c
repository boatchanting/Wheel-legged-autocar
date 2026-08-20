/*
 * =================================================================================
 * 文件: vision_bridge_control.c
 * 作用: 0 核 (Core 0) 视觉桥梁任务（单边桥）的状态机与控制执行。
 * 说明: 这个文件就像是车子过桥时的“大脑”。它不断读取 1 核发来的直线和黑块位置，
 *       然后决定车子现在该干嘛（比如：接近入口 -> 桥头对齐 -> 上桥盲跑 -> 
 *       找线微调 -> 发现出口 -> 下桥缓冲 -> 恢复正常循迹）。
 * =================================================================================
 */
#include "vision/vision_bridge_control.h"
#include "vision/vision_ipc_core0.h"
#include "../../code1/vision/ipm_transform.h"
#include "plan/bridge.h"
#include "tools/sbus.h"
#include "../config/sys_options.h"   /* DEBUG_LOG_ENABLE (zf_common_headfile 不含 config) */

#if VISION_BRIDGE_TASK_ENABLE

/* --- 外部变量引用 --- */
/* 这些变量通常在其他文件里定义，用来控制底盘电机和舵机 */
extern volatile float err_degree;           /* 方向盘打多少度 */
extern volatile float target_speed_set;     /* 目标速度（负数代表前进） */
extern int g_motor_enable;                  /* 电机使能开关 */
extern uint8 g_special_action_trigger;      /* 特殊动作触发标志（比如过桥） */
extern uint8_t roll_balance_enable;         /* 滚转平衡（过单边桥防翻车）开关 */
extern int32 acc_limit;                     /* 加速度限制 */
extern int32 dec_limit;                     /* 减速度限制 */
extern float servo_height;                  /* 舵机高度（比如过桥时可能要抬高底盘） */
extern volatile uint8 exit_beep_request;    /* 出口蜂鸣请求(视觉确认2声): 脱出判定处置位 (定义于 nav_replay/plan4, 主循环消费) */

/* --- 全局变量 --- */
volatile uint8 g_bridge_vision_task_enable = 0U; /* 任务总开关，别人可以把它设为 1 来启动任务 */
volatile vision_bridge_task_status_t g_bridge_vision_task_status = {0}; /* 记录当前任务的详细状态供外人看 */
volatile vision_bridge_exit_reason_e g_bridge_vision_task_exit_reason = VISION_BRIDGE_EXIT_NONE;
volatile uint8 g_bridge_exit_timeout_beep_request = 0U; /* 兜底退出(AUTO_TIMEOUT)蜂鸣请求: 主循环消费响1声 (2026-08-08) */

/* --- 内部数据结构 --- */
/**
 * @brief 桥梁任务内部的“记事本”
 * @note  记录了任务进行到了哪一步、起点的坐标是多少、上桥前方向是多少等。
 */
typedef struct
{
    vision_bridge_task_state_e state; /* 当前处于哪个阶段 */
    uint32 state_ticks;               /* 在这个阶段待了多久了（每个 tick 是 2ms） */
    uint16 bridge_hold_ticks;         /* 看见桥面(gate)后的“闭眼盲跑”倒计时 */
    uint16 align_ok_ticks;            /* 桥头对齐：连续多少次对准了 */
    uint32 last_seq;                  /* 上次滤波处理的 IPC 序号 */
    uint8 center_filter_valid;
    uint8 center_filter_pending_jump;
    uint8 center_filter_lost_frames;
    uint8 center_filter_recover_frames; /* 恢复计数: 连续有效帧 (C09) */
    float filtered_lookahead_x;
    float filtered_heading_deg;
    float pending_lookahead_x;
    float pending_heading_deg;
    float start_x_mm;                 /* 上桥那一刻的 X 坐标（惯导） */
    float start_y_mm;                 /* 上桥那一刻的 Y 坐标（惯导） */
    float exit_start_x_mm;            /* 开始下桥那一刻的 X 坐标 */
    float exit_start_y_mm;            /* 开始下桥那一刻的 Y 坐标 */
    float locked_yaw_deg;             /* 锁角盲跑的目标航向（视觉失效时改回 entry_yaw_deg） */
    float entry_yaw_deg;              /* 进入任务那一刻的 yaw (视觉失效/锁角盲跑时改回此角, 2026-08-15) */
    uint8 run_yaw_locked;             /* 跑过视觉控制距离后，是否已锁定航向 */
    uint8 err_source;                 /* 当前 err 来源: 0=视觉 1=锁角 (C10 换源 ramp) */
    float last_err_ramp;              /* ramp 输出的上一帧 err (C10) */
    float exit_line_y;                /* 退出线在 x=47 处的图像行 (调试, 无效为 -1) */
    int32 saved_acc_limit;            /* 备份原来的加速度限制，下桥后恢复 */
    int32 saved_dec_limit;            /* 备份原来的减速度限制，下桥后恢复 */
    float saved_servo_height;         /* 备份进入任务时的腿高 (servo_height)，退出后恢复到它而非写死值 (同雷区做法, 2026-08-14) */
    uint8 saved_limits_valid;         /* 标记备份数据是否有效 */
    float filtered_lateral_m;         /* 前视横向误差 e (m, 低通, 控制 P 项用) */
    float edot_mps;                   /* 横向误差导数 ė (m/s, 低通+限幅, D 项用) */
    float last_lateral_m;             /* 上一视觉包的原始 e, 用于 ė 帧差 */
    uint8 edot_has_history;           /* 首次/重捕获后不计算 ė (防微分冲击) */
} vision_bridge_task_ctx_t;

/* 这个就是真正的“记事本”本尊，只有这个文件能用 */
static vision_bridge_task_ctx_t s_bridge_task;

/* 方向控制可调参数面板默认值 (参照 trials/track.html 与 trials/index.html) */
const vision_bridge_tune_t g_vision_bridge_tune_defaults =
{
    .lat_kp               = 6.0f,
    .lat_ki               = 0.0f,
    .lat_kd               = 6.0f,
    .lat_int_max          = 3.0f,
    .lat_adaptive_enable  = 1U,
    .edot_alpha           = 0.25f,
    .edot_clamp_mps       = 3.0f,
    .edot_fps             = 30.0f,
    .lookahead_m          = 1.0f,
    .yaw_hold_kp          = 1.8f,
    .yaw_hold_src_sel     = VISION_BRIDGE_YAWHOLD_SRC_ENTRY,
    .out_max_deg          = 22.9f,
    .ramp_step_deg_per_2ms = 0.5f,
    .lat_sign             = 1.0f,
    .edot_sign            = 1.0f,
    .yaw_hold_sign        = 1.0f,
};

/* --- 基础数学工具函数 --- */

/**
 * @brief 取浮点数的绝对值
 */
static float vision_bridge_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

/**
 * @brief 把数值限制在最小值和最大值之间
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

/**
 * @brief 把角度规范到 -180 到 180 度之间
 * @note  比如 370 度其实就是 10 度。
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

/**
 * @brief 算一下车子离某个坐标点（比如桥头）有多远
 * @param x_mm 那个点的 X 坐标
 * @param y_mm 那个点的 Y 坐标
 * @return float 距离（毫米）
 */
static float vision_bridge_distance_from(float x_mm, float y_mm)
{
    const float dx = inertial_nav.x - x_mm; /* inertial_nav 是外部的惯导结构体 */
    const float dy = inertial_nav.y - y_mm;
    /* 勾股定理求直线距离 */
    return sqrtf(dx * dx + dy * dy);
}

static uint8 vision_bridge_get_control_measurement(const volatile vision_ipc_packet_t *packet,
                                                   float *lookahead_x,
                                                   float *heading_deg,
                                                   float *lateral_m)
{
    float x_at_lookahead;
    uint8_t lookahead_img_x;
    IPM_Point_t target_point;
    IPM_Point_t reference_point;
    const uint8_t lookahead_img_y = (uint8_t)VISION_BRIDGE_TASK_LOOKAHEAD_Y;

    if ((packet == NULL) || (packet->b2_valid == 0U))
    {
        return 0U;
    }

    /* 第①级 (C07): 支撑校验 — 前视行必须落在控制线支撑范围 [u_lo, u_hi] 内 */
    if ((lookahead_img_y < packet->b2_line_u_lo) ||
        (lookahead_img_y > packet->b2_line_u_hi))
    {
        return 0U;
    }

    /* 第①级 (C07): 系数直接代入 x = a*y + b (a×1000, b×100), 不再用两点插值 */
    x_at_lookahead = ((float)packet->b2_line_a_x1000 * (float)lookahead_img_y) / 1000.0f +
                     ((float)packet->b2_line_b_x100) / 100.0f;
    *lookahead_x = x_at_lookahead;
    if ((*lookahead_x < 0.0f) || (*lookahead_x > (float)(IPM_IMG_WIDTH - 1U)))
    {
        return 0U;
    }

    lookahead_img_x = (uint8_t)(*lookahead_x + 0.5f);
    target_point = IPM_GetPhysicalCoord(lookahead_img_x, lookahead_img_y);
    reference_point = IPM_GetPhysicalCoord((uint8_t)VISION_BRIDGE_TASK_IMAGE_CENTER_X,
                                           lookahead_img_y);
    if ((!target_point.is_valid) || (!reference_point.is_valid) ||
        (target_point.y_mm <= 0) || (reference_point.y_mm <= 0))
    {
        return 0U;
    }

    /* 前视横向误差 e (m): IPM 物理 x 差 (target - 中心列同行参考点), 向右为正 */
    *lateral_m = (float)(target_point.x_mm - reference_point.x_mm) / 1000.0f;

    /* The LUT gives physical X (right) / Y (forward).  Subtract the calibrated
     * straight-ahead ray so a line at IMAGE_CENTER_X produces exactly 0 deg. */
    *heading_deg = vision_bridge_normalize_angle(
        (atan2f((float)target_point.x_mm, (float)target_point.y_mm) -
         atan2f((float)reference_point.x_mm, (float)reference_point.y_mm)) * 57.2957795f);
    return 1U;
}

static uint8 vision_bridge_center_jump_is_confirmed(float lookahead_x,
                                                     float heading_deg)
{
    return (uint8)((vision_bridge_abs_f(lookahead_x - s_bridge_task.pending_lookahead_x) <=
                    VISION_BRIDGE_TASK_CENTER_JUMP_CONFIRM_PX) &&
                   (vision_bridge_abs_f(heading_deg - s_bridge_task.pending_heading_deg) <=
                    VISION_BRIDGE_TASK_CENTER_JUMP_CONFIRM_DEG));
}

static void vision_bridge_update_center_filter(const volatile vision_ipc_packet_t *packet)
{
    const vision_bridge_tune_t *t = &g_vision_bridge_tune_defaults;
    float lookahead_x;
    float heading_deg;
    float lateral_m;
    uint8 is_jump;

    /* The control loop is 2 ms while vision packets arrive much slower.  A
     * filter update must therefore be tied to packet sequence, not loop rate. */
    if ((packet == NULL) || (packet->seq == 0U) || (packet->seq == s_bridge_task.last_seq))
    {
        return;
    }
    s_bridge_task.last_seq = packet->seq;

    /* C08: 原始可信来自 b2_valid (1核仲裁层输出); C09: 失能连续 N 帧才回锁角 */
    if ((packet->b2_valid == 0U) ||
        (vision_bridge_get_control_measurement(packet, &lookahead_x, &heading_deg, &lateral_m) == 0U))
    {
        s_bridge_task.center_filter_pending_jump = 0U;
        s_bridge_task.center_filter_recover_frames = 0U;
        if (s_bridge_task.center_filter_lost_frames < 255U)
        {
            s_bridge_task.center_filter_lost_frames++;
        }
        if (s_bridge_task.center_filter_lost_frames >= VISION_BRIDGE_TASK_VALID_LOST_FRAMES)
        {
            s_bridge_task.center_filter_valid = 0U;
        }
        return;
    }

    s_bridge_task.center_filter_lost_frames = 0U;
    if (s_bridge_task.center_filter_valid == 0U)
    {
        /* C09: 恢复需连续 M 帧有效才回视觉 */
        if (s_bridge_task.center_filter_recover_frames < 255U)
        {
            s_bridge_task.center_filter_recover_frames++;
        }
        if (s_bridge_task.center_filter_recover_frames < VISION_BRIDGE_TASK_VALID_RECOVER_FRAMES)
        {
            return;
        }
        s_bridge_task.center_filter_recover_frames = 0U;
        s_bridge_task.filtered_lookahead_x = lookahead_x;
        s_bridge_task.filtered_heading_deg = heading_deg;
        s_bridge_task.filtered_lateral_m = lateral_m;
        s_bridge_task.last_lateral_m = lateral_m;
        s_bridge_task.edot_mps = 0.0f;
        s_bridge_task.edot_has_history = 0U; /* 重捕获防微分冲击 */
        s_bridge_task.center_filter_valid = 1U;
        s_bridge_task.center_filter_pending_jump = 0U;
        return;
    }

    is_jump = (uint8)((vision_bridge_abs_f(lookahead_x - s_bridge_task.filtered_lookahead_x) >
                       VISION_BRIDGE_TASK_CENTER_JUMP_REJECT_PX) ||
                      (vision_bridge_abs_f(heading_deg - s_bridge_task.filtered_heading_deg) >
                       VISION_BRIDGE_TASK_CENTER_JUMP_REJECT_DEG));
    if (is_jump)
    {
        if ((s_bridge_task.center_filter_pending_jump == 0U) ||
            (vision_bridge_center_jump_is_confirmed(lookahead_x, heading_deg) == 0U))
        {
            s_bridge_task.pending_lookahead_x = lookahead_x;
            s_bridge_task.pending_heading_deg = heading_deg;
            s_bridge_task.center_filter_pending_jump = 1U;
            return;
        }
    }

    s_bridge_task.center_filter_pending_jump = 0U;
    s_bridge_task.filtered_lookahead_x += VISION_BRIDGE_TASK_CENTER_FILTER_ALPHA *
                                           (lookahead_x - s_bridge_task.filtered_lookahead_x);
    s_bridge_task.filtered_heading_deg += VISION_BRIDGE_TASK_CENTER_FILTER_ALPHA *
                                           (heading_deg - s_bridge_task.filtered_heading_deg);
    s_bridge_task.filtered_lateral_m += VISION_BRIDGE_TASK_CENTER_FILTER_ALPHA *
                                         (lateral_m - s_bridge_task.filtered_lateral_m);

    /* ė: 原始 e 帧差 + 强低通 + 限幅 (复刻 track.html, 仅在新视觉包时更新) */
    if (s_bridge_task.edot_has_history != 0U)
    {
        const float d_raw = vision_bridge_constrain_f(
            (lateral_m - s_bridge_task.last_lateral_m) * t->edot_fps,
            -t->edot_clamp_mps,
            t->edot_clamp_mps);
        s_bridge_task.edot_mps += t->edot_alpha * (d_raw - s_bridge_task.edot_mps);
    }
    else
    {
        s_bridge_task.edot_mps = 0.0f;
    }
    s_bridge_task.last_lateral_m = lateral_m;
    s_bridge_task.edot_has_history = 1U;
}

/* --- 控制核心辅助函数 --- */

/**
 * @brief stage1 (v8 循迹) 横向乘性 PID, 输出 err_degree(deg)
 *
 * 落地公式 (保持 err_degree → 底层转向角环):
 *   e = filtered_lateral_m (m),  ė = edot_mps (m/s),  v = |vx_body|/1000 (m/s)
 *   ω_radps = LAT_SIGN·Kp·e·v + EDOT_SIGN·Kd·ė   (乘性自适应; Ki 默认 0 未启用)
 *   err_degree = ω_radps·(180/π) / TURN_ANG_KP
 * @return float 给底层转向角环的 err_degree
 */
static float vision_bridge_calc_visual_err_degree(void)
{
    const vision_bridge_tune_t *t = &g_vision_bridge_tune_defaults;
    const float v_mps = vision_bridge_abs_f(inertial_nav.vx_body) / 1000.0f;
    float omega_radps;
    float err_deg;

    if (s_bridge_task.center_filter_valid == 0U)
    {
        return 0.0f;
    }

    if (t->lat_adaptive_enable != 0U)
    {
        /* 乘性速度自适应: 横向 P 通道随车速缩放 */
        omega_radps = t->lat_sign * t->lat_kp * s_bridge_task.filtered_lateral_m * v_mps;
    }
    else
    {
        /* 固定增益: 力度与车速无关 (对应仿真非自适应模式) */
        omega_radps = t->lat_sign * t->lat_kp * s_bridge_task.filtered_lateral_m;
    }
    /* D 通道 (Ki 默认 0, 未启用积分) */
    omega_radps += t->edot_sign * t->lat_kd * s_bridge_task.edot_mps;

    err_deg = omega_radps * 57.2957795f / VISION_BRIDGE_TURN_ANG_KP_REF;
    return vision_bridge_constrain_f(err_deg, -t->out_max_deg, t->out_max_deg);
}

/**
 * @brief 锁角目标航向 (stage0/stage2/丢线共用)
 *
 * @note  yaw_hold_src_sel: 0=进入任务时刻 entry_yaw; 1=路表当前点 target_yaw。
 *        路表 target_yaw 的具体访问接口移植时需确认 (见实现文档风险项),
 *        当前未接入时回退到 entry_yaw。
 */
static float vision_bridge_yaw_hold_target_deg(void)
{
    if (g_vision_bridge_tune_defaults.yaw_hold_src_sel == VISION_BRIDGE_YAWHOLD_SRC_ROUTE)
    {
        /* TODO(落地确认): 接入路表当前点 nav_ram_data.points[?].target_yaw_deg。
         * 桥任务期间 nav_replay 暂停, 无统一 current index 接口, 暂回退 locked_yaw_deg。 */
        return s_bridge_task.locked_yaw_deg;
    }
    /* ENTRY 模式: 状态机维护的锁角目标 (= 进入任务时刻 yaw) */
    return s_bridge_task.locked_yaw_deg;
}

/**
 * @brief 在桥上盲跑/进场/脱出时，根据惯导保持车头方向 (返回 ψ_err, deg)
 *
 * @note 目标航向按 yaw_hold_src_sel 选择; 偏了就靠惯导纠正。
 */
static float vision_bridge_calc_yaw_hold_err(void)
{
    const float target = vision_bridge_yaw_hold_target_deg();
    /* 误差 = 目标方向 - 当前惯导测出的方向 */
    const float err = vision_bridge_normalize_angle(target - inertial_nav.relative_yaw);
    /* 限制在盲跑允许的最大范围内 */
    return vision_bridge_constrain_f(err,
                                     -VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG,
                                     VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG);
}

/**
 * @brief 锁角纯 P, 输出 err_degree(deg)
 *
 * 落地公式: err_degree = YAWHOLD_SIGN · Kψ_lock · ψ_err,
 *           Kψ_lock = yaw_hold_kp / |TURN_ANG_KP| = 1.8/8 = 0.225
 */
static float vision_bridge_calc_yaw_hold_err_degree(void)
{
    const vision_bridge_tune_t *t = &g_vision_bridge_tune_defaults;
    const float psi_err = vision_bridge_calc_yaw_hold_err();
    const float k_lock = t->yaw_hold_kp / vision_bridge_abs_f(VISION_BRIDGE_TURN_ANG_KP_REF);
    const float err = t->yaw_hold_sign * k_lock * psi_err;
    return vision_bridge_constrain_f(err, -t->out_max_deg, t->out_max_deg);
}

/**
 * @brief 判断视觉侧状态机是否已切到"准备脱出"(寻找脱出线)阶段
 * @note  b2_mode 低 3 位 = 融合阶段 (B2M_*, 见 vision_ipc.h);
 *        阶段 2 = 1 核已锁存 gate_top 并切回 ref 检测器找脱出线。
 */
static uint8 vision_bridge_packet_in_exit_stage(const volatile vision_ipc_packet_t *packet)
{
    return (uint8)((packet != NULL) &&
                   ((packet->b2_mode & B2M_STAGE_MASK) == B2M_STAGE_PREPARE_EXIT));
}

/**
 * @brief 换源 ramp (C10): err_degree 变化率限 ≤ RAMP_STEP/2ms
 * @note  视觉 err 与锁角 err 切换时的跳变被限速; 源切换靠 source 记录 (诊断)。
 */
static float vision_bridge_apply_err_ramp(float target, uint8 source)
{
    float diff;

    (void)source; /* 统一限速, 源切换跳变天然被钳住 */
    diff = target - s_bridge_task.last_err_ramp;
    if (diff > g_vision_bridge_tune_defaults.ramp_step_deg_per_2ms)
    {
        s_bridge_task.last_err_ramp += g_vision_bridge_tune_defaults.ramp_step_deg_per_2ms;
    }
    else if (diff < -g_vision_bridge_tune_defaults.ramp_step_deg_per_2ms)
    {
        s_bridge_task.last_err_ramp -= g_vision_bridge_tune_defaults.ramp_step_deg_per_2ms;
    }
    else
    {
        s_bridge_task.last_err_ramp = target;
    }
    return s_bridge_task.last_err_ramp;
}

/**
 * @brief 退出线图像行测量: b2_top 横线 y=a*x+b 在图像中心列 x=47 处的行坐标
 * @note  结果同时写入 s_bridge_task.exit_line_y 供状态发布/调试。
 * @return 行坐标 (0~59); 无效返回 -1
 */
static float vision_bridge_exit_line_measure_y(const volatile vision_ipc_packet_t *packet)
{
    const uint8_t x = (uint8_t)VISION_BRIDGE_TASK_IMAGE_CENTER_X;
    float y;

    if ((packet == NULL) || (packet->b2_gate == 0U) || (packet->b2_has_top == 0U))
    {
        s_bridge_task.exit_line_y = -1.0f;
        return -1.0f;
    }
    y = ((float)packet->b2_top_a_x1000 * (float)x) / 1000.0f +
        ((float)packet->b2_top_b_x100) / 100.0f;
    if ((y < 0.0f) || (y > (float)(IPM_IMG_HEIGHT - 1U)))
    {
        s_bridge_task.exit_line_y = -1.0f;
        return -1.0f;
    }
    s_bridge_task.exit_line_y = y;
    return y;
}

/**
 * @brief 把上桥前的加减速限制存起来
 * @note  因为上桥可能要慢慢开，需要改限制；下桥后得把这些参数还给系统。
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

/**
 * @brief 切换到“上桥姿态”
 * @note  限制加速度、开启滚转平衡（防翻车）、把底盘升高。
 */
static void vision_bridge_apply_high_posture(void)
{
    vision_bridge_save_servo_limits_once();
    // acc_limit = bridge_params.servo_acc_bridge;
    // dec_limit = bridge_params.servo_dec_bridge;
    // roll_balance_enable = 1U; /* 开启滚转平衡 */
    /* 抬高底盘，并且规定抬高的速度（步长） */
    Bridge_Apply_Height_Control(bridge_params.height_bridge,
                                bridge_params.height_step_rise * VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE);
}

/**
 * @brief 退出后要恢复到的腿高: 进入任务时的备份值, 无备份兜底 height_normal
 */
static float vision_bridge_restore_height_target(void)
{
    return (s_bridge_task.saved_limits_valid) ? s_bridge_task.saved_servo_height
                                              : bridge_params.height_normal;
}

/**
 * @brief 切换回“正常姿态”
 * @note  把底盘降回进入任务时的高度（不在此处关 Rolling，由脱出时刻统一管理）。
 */
static void vision_bridge_apply_normal_posture(void)
{
    /* 降下底盘 (恢复到进入时的腿高, 不写死 3.0f) */
    Bridge_Apply_Height_Control(vision_bridge_restore_height_target(),
                                bridge_params.height_step_drop * VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE);
}

/**
 * @brief 切换到下一个任务阶段
 * 
 * @param next_state 下一个阶段是什么
 * 
 * @note 切换时把计时器清零；下桥完成由视觉确认或时间兜底决定，不依赖惯导距离。
 */
static void vision_bridge_set_state(vision_bridge_task_state_e next_state)
{
    s_bridge_task.state = next_state;
    s_bridge_task.state_ticks = 0U;
    s_bridge_task.align_ok_ticks = 0U;

    if (next_state == VISION_BRIDGE_TASK_RUN)
    {
        /* RUN 距离从真正上桥的时刻开始计；到 1.2m 时再锁定当时的实际航向。 */
        s_bridge_task.start_x_mm = inertial_nav.x;
        s_bridge_task.start_y_mm = inertial_nav.y;
        s_bridge_task.run_yaw_locked = 0U;
    }

}

/**
 * @brief 把当前的内部状态打包公开，给外面的模块（或者屏幕）看
 */
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
    status.bridge_hold_ticks = s_bridge_task.bridge_hold_ticks;

    if (packet != NULL)
    {
        status.b2_valid   = packet->b2_valid;
        status.b2_source  = packet->b2_source;
        status.b2_mode    = packet->b2_mode;
        status.b2_gate    = packet->b2_gate;
        status.b2_has_top = packet->b2_has_top;
        status.exit_line_y = s_bridge_task.exit_line_y;
    }
    status.center_filter_valid = s_bridge_task.center_filter_valid;
    status.center_filter_pending_jump = s_bridge_task.center_filter_pending_jump;
    status.filtered_lookahead_x = s_bridge_task.filtered_lookahead_x;
    status.filtered_heading_deg = s_bridge_task.filtered_heading_deg;
    status.filtered_lateral_m = s_bridge_task.filtered_lateral_m;
    status.edot_mps = s_bridge_task.edot_mps;
    g_bridge_vision_task_status = status;
}

#if VISION_BRIDGE_TASK_NAV_CORRECT_ENABLE
/**
 * @brief 下桥后，用视觉任务的已知信息去纠正惯导的坐标
 * 
 * @note 惯导跑久了会有误差，如果知道桥的固定长度，
 *       可以在下桥时强行把惯导的坐标“拉回”正确的位置。
 */
static void vision_bridge_apply_nav_correction(void)
{
    /* 算出上桥时的角度（弧度） */
    const float yaw_rad = s_bridge_task.locked_yaw_deg * 0.0174532925f;
    /* 当前坐标 = 起点坐标 + 桥长 * 角度方向 */
    inertial_nav.x = s_bridge_task.start_x_mm -
                     cosf(yaw_rad) * VISION_BRIDGE_TASK_NAV_CORRECT_DISTANCE_MM;
    inertial_nav.y = s_bridge_task.start_y_mm +
                     sinf(yaw_rad) * VISION_BRIDGE_TASK_NAV_CORRECT_DISTANCE_MM;
    inertial_nav.relative_yaw = s_bridge_task.locked_yaw_deg;
    /* 速度清零，重新开始算 */
    inertial_nav.vx_body = 0.0f;
    inertial_nav.vy_body = 0.0f;
}
#endif

/**
 * @brief 任务结束或中断时的清理工作
 * 
 * @param stop_car 结束时要不要把车刹停？（1=刹停，0=不管它继续开）
 * 
 * @note 关掉视觉检测、恢复车身姿态、清空状态机。
 */
static void vision_bridge_cleanup(uint8 stop_car)
{
    /* 告诉 1 核停止单边桥检测。 */
    VisionIpc_Core0_SetBridgeEnable(0U);
    
    /* 任务彻底退出，确保关闭 Rolling 平衡并恢复巡航加速度限制 */
    roll_balance_enable = 0U;
    if (s_bridge_task.saved_limits_valid)
    {
        acc_limit = s_bridge_task.saved_acc_limit;
        dec_limit = s_bridge_task.saved_dec_limit;
    }

    /* 恢复正常姿态 (底盘降至进入时高度) */
    vision_bridge_apply_normal_posture();

    if (stop_car)
    {
        target_speed_set = 0.0f;
        err_degree = 0.0f;
    }

    /* 撤销特殊动作标记，告诉系统“我完事了” */
    g_special_action_trigger = 0U;
    g_bridge_vision_task_enable = 0U;
    
    /* 清空“记事本” */
    memset(&s_bridge_task, 0, sizeof(s_bridge_task));
    g_bridge_vision_task_status.enabled = 0U;
    g_bridge_vision_task_status.state = VISION_BRIDGE_TASK_IDLE;
}

/* --- 对外接口函数 --- */

/**
 * @brief 桥梁任务初始化
 * @note  开机时调用，把所有东西都清零。
 */
void VisionBridgeTask_Init(void)
{
    memset(&s_bridge_task, 0, sizeof(s_bridge_task));
    memset((void *)&g_bridge_vision_task_status, 0, sizeof(g_bridge_vision_task_status));
    g_bridge_vision_task_enable = 0U;
    g_bridge_vision_task_exit_reason = VISION_BRIDGE_EXIT_NONE;
}

/**
 * @brief 启动桥梁任务
 * @note  当主程序觉得快到桥了，调用这个函数启动整个流程。
 */
void VisionBridgeTask_Start(void)
{
    g_bridge_vision_task_exit_reason = VISION_BRIDGE_EXIT_NONE;
    g_bridge_vision_task_enable = 1U;
}

/**
 * @brief 强制停止桥梁任务
 */
void VisionBridgeTask_Stop(void)
{
    vision_bridge_cleanup(1U);
}

/**
 * @brief 检查桥梁任务是否正在忙
 * @return 1: 正在忙; 0: 闲着
 */
uint8 VisionBridgeTask_IsActive(void)
{
    return (uint8)((g_bridge_vision_task_enable != 0U) ||
                   (s_bridge_task.state != VISION_BRIDGE_TASK_IDLE));
}

/**
 * @brief 进入任务的准备阶段
 * @note  把当前位置设为起点，并告诉 1 核开始单边桥检测。
 */
static void vision_bridge_enter_task(void)
{
    memset(&s_bridge_task, 0, sizeof(s_bridge_task));
    s_bridge_task.state = VISION_BRIDGE_TASK_ALIGN;
    s_bridge_task.start_x_mm = inertial_nav.x;
    s_bridge_task.start_y_mm = inertial_nav.y;
    s_bridge_task.locked_yaw_deg = inertial_nav.relative_yaw;
    s_bridge_task.entry_yaw_deg  = inertial_nav.relative_yaw; /* 进入时刻锁存的 yaw, 视觉失效时改回此角 */
    s_bridge_task.saved_acc_limit = acc_limit;
    s_bridge_task.saved_dec_limit = dec_limit;
    s_bridge_task.saved_servo_height = servo_height; /* 备份进入时腿高, 退出恢复用 */
    s_bridge_task.saved_limits_valid = 1U;

    g_special_action_trigger = 1U; /* 告诉系统我接管车子了 */
    
    vision_bridge_apply_high_posture(); /* 准备上桥阶段即开启高姿态与 Rolling 平衡 */
    VisionIpc_Core0_SetBridgeEnable(1U);
}

/* --- 核心状态机 --- */

/**
 * @brief 桥梁任务的心脏（每 2 毫秒执行一次）
 * 
 * @note 这是一个“状态机”。它根据当前处于哪一步（state），执行不同的逻辑。
 *       步骤 1: ENTER_PVC (跟着 PVC 入口走)
 *       步骤 2: ALIGN (在桥头对齐方向)
 *       步骤 3: RUN (在桥上跑)
 *       步骤 4: EXIT (下桥缓冲)
 *       步骤 5: FINISH (结束)
 */
void VisionBridgeTask_Update_2ms(void)
{
    /* 获取 1 核刚刚传过来的最新“眼睛看到的信息” */
    const volatile vision_ipc_packet_t *packet = VisionIpc_Core0_GetLatest();
    float traveled_mm = 0.0f; /* 跑了多远 */
    float err_cmd = 0.0f;     /* 打算给方向盘的指令 */
    float speed_cmd = 0.0f;   /* 打算给电机的指令 */

    /* 如果没开启任务，且现在是空闲状态，啥也不干 */
    if ((g_bridge_vision_task_enable == 0U) &&
        (s_bridge_task.state == VISION_BRIDGE_TASK_IDLE))
    {
        return;
    }

    /* 如果车子被紧急停止了，或者惯导还没准备好，赶紧退出任务 */
    #if REMOTE_CONTROL == 1
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U || robot_ctrl.brake_active == 1U))
    {
        vision_bridge_cleanup(1U);
        return;
    }
    #endif
    #if REMOTE_CONTROL == 0
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U))
    {
        vision_bridge_cleanup(1U);
        return;
    }
    #endif


    /* 如果刚刚被叫醒，准备开始任务 */
    if (s_bridge_task.state == VISION_BRIDGE_TASK_IDLE)
    {
        vision_bridge_enter_task();
    }

    vision_bridge_update_center_filter(packet);

    s_bridge_task.state_ticks++; /* 计时器滴答一下 */
    /* 算算从起点到现在跑了多远了 */
    traveled_mm = vision_bridge_distance_from(s_bridge_task.start_x_mm, s_bridge_task.start_y_mm);

    /* 看看现在走到哪一步了 */
    switch (s_bridge_task.state)
    {
        case VISION_BRIDGE_TASK_ALIGN:
            speed_cmd = VISION_BRIDGE_TASK_RUN_SPEED_SET;
            /* stage0 = PVC + IMU: 方向只来自 IMU 锁角 (PVC 只做入口检测, 不提供转向) */
            err_cmd = vision_bridge_calc_yaw_hold_err_degree();
            s_bridge_task.err_source = 1U;
            if (vision_bridge_abs_f(vision_bridge_calc_yaw_hold_err()) <= VISION_BRIDGE_TASK_ALIGN_YAW_TOL_DEG)
            {
                s_bridge_task.align_ok_ticks++;
            }
            else
            {
                s_bridge_task.align_ok_ticks = 0U;
            }

            err_cmd = vision_bridge_apply_err_ramp(err_cmd, s_bridge_task.err_source);
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            /* 视觉确认上桥 (沿用旧逻辑 ON_BRIDGE+stable 语义): 控制线可信且桥面
               已到底部(锁存) → 抬底盘进 RUN。只有这条路抬底盘。 */
            if ((packet->b2_valid != 0U) && (packet->b2_gate != 0U))
            {
                s_bridge_task.bridge_hold_ticks = VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS;
                vision_bridge_apply_high_posture();
                vision_bridge_set_state(VISION_BRIDGE_TASK_RUN);
                break;
            }

            /* 惯导门: 从交接点起 traveled ≥ 阈值进 RUN (不抬底盘, 与旧逻辑兜底路径一致) */
            if (traveled_mm >= VISION_BRIDGE_TASK_ON_BRIDGE_TRIGGER_MM)
            {
                s_bridge_task.locked_yaw_deg = s_bridge_task.entry_yaw_deg;
                vision_bridge_set_state(VISION_BRIDGE_TASK_RUN);
                break;
            }

            /* 兜底: 对齐达标或超时也上桥 (不抬底盘, 与旧逻辑一致) */
            if ((s_bridge_task.align_ok_ticks >= VISION_BRIDGE_TASK_ALIGN_OK_TICKS) ||
                (s_bridge_task.state_ticks >= VISION_BRIDGE_TASK_ALIGN_TIMEOUT_TICKS))
            {
                s_bridge_task.locked_yaw_deg = s_bridge_task.entry_yaw_deg;
                vision_bridge_set_state(VISION_BRIDGE_TASK_RUN); /* 冲！ */
            }
            break;

        /* --- 阶段 3：在桥上跑 --- */
        case VISION_BRIDGE_TASK_RUN:
            /* 桥面证据 (沿用旧逻辑 ON_BRIDGE 语义): b2_gate 锁存表示桥面曾到底部,
               b2_valid 每帧表示控制线仍在; 两者同时在场才刷新防抖倒计时。
               (0809 只看 b2_gate 锁存量 → 一旦锁存全程刷新 → 全程高腿, 2026-08-14 修复) */
            if ((packet->b2_valid != 0U) && (packet->b2_gate != 0U))
            {
                s_bridge_task.bridge_hold_ticks = VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS;
            }
            else if (s_bridge_task.bridge_hold_ticks > 0U)
            {
                s_bridge_task.bridge_hold_ticks--; /* 没看到，倒计时减 1 */
            }

            vision_bridge_exit_line_measure_y(packet); /* 每 tick 刷新退出线行坐标 (调试可观测) */

            /* 如果倒计时没归零，说明现在车还在桥上 */
            if (s_bridge_task.bridge_hold_ticks > 0U)
            {
                vision_bridge_apply_high_posture(); /* 保持高底盘 */
                speed_cmd = VISION_BRIDGE_TASK_BRIDGE_SPEED_SET; /* 桥上速度 */
            }
            else
            {
                /* 如果归零了，说明可能快下桥了或者在桥的平缓段 */
                vision_bridge_apply_normal_posture(); /* 降下底盘 (已移除内部误关 rolling 的 bug) */
                /* 如果能看到地上的线，就跟着线跑 */
                if (s_bridge_task.center_filter_valid)
                {
                    speed_cmd = VISION_BRIDGE_TASK_RUN_SPEED_SET;
                }
                else
                {
                    /* 线也看不见，只能盲跑了 */
                    speed_cmd = VISION_BRIDGE_TASK_BLIND_SPEED_SET;
                }
            }

            /* 前 1100 mm 保留当前 PD 视觉对中；超过该距离后锁住进入时 yaw，
               之后不再接受中心线转向输入。 */
            if (traveled_mm <= VISION_BRIDGE_TASK_VISUAL_CONTROL_DISTANCE_MM)
            {
                if (s_bridge_task.center_filter_valid)
                {
                    err_cmd = vision_bridge_calc_visual_err_degree();
                    s_bridge_task.err_source = 0U;
                }
                else
                {
                    err_cmd = vision_bridge_calc_yaw_hold_err_degree();
                    s_bridge_task.err_source = 1U;
                }
            }
            else
            {
                if (s_bridge_task.run_yaw_locked == 0U)
                {
                    s_bridge_task.locked_yaw_deg = s_bridge_task.entry_yaw_deg;
                    s_bridge_task.run_yaw_locked = 1U;
                }
                err_cmd = vision_bridge_calc_yaw_hold_err_degree();
                s_bridge_task.err_source = 1U;
            }

            err_cmd = vision_bridge_apply_err_ramp(err_cmd, s_bridge_task.err_source);
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            /* 仅融合状态机已进入准备脱出阶段，且 RUN 里程满 2300 mm 后，
               顶部线到图像顶部才允许退出。 */
            if ((traveled_mm >= VISION_BRIDGE_TASK_RUN_MIN_MM) &&
                vision_bridge_packet_in_exit_stage(packet) &&
                (s_bridge_task.exit_line_y >= 0.0f) &&
                (s_bridge_task.exit_line_y < VISION_BRIDGE_TASK_EXIT_LINE_TOP_Y_PX))
            {
                g_bridge_vision_task_exit_reason = VISION_BRIDGE_EXIT_VISUAL_CONFIRMED;
                exit_beep_request = 1U; /* 脱出时刻: 视觉确认响 2 声 (侧键/Plan4 驱动都响) */
                vision_bridge_set_state(VISION_BRIDGE_TASK_EXIT);
            }
            else if ((traveled_mm >= VISION_BRIDGE_TASK_RUN_MAX_MM) ||        /* 距离过大强制下桥 (恢复 12b6fe4 的历史上界, 2026-08-14) */
                     (s_bridge_task.state_ticks >= VISION_BRIDGE_TASK_RUN_AUTO_EXIT_TICKS))
            {
                // 视觉异常时自动继续；Plan3/Plan4 会知道这不是已确认的视觉出口，不会重定位。
                g_bridge_vision_task_exit_reason = VISION_BRIDGE_EXIT_AUTO_TIMEOUT;
                g_bridge_exit_timeout_beep_request = 1U; /* 兜底退出: 主循环响 1 声 (区别于视觉确认的 2 声) */
                vision_bridge_set_state(VISION_BRIDGE_TASK_EXIT);
            }
            break;

        /* --- 阶段 4：下桥缓冲 --- */
        case VISION_BRIDGE_TASK_EXIT:
            vision_bridge_apply_normal_posture(); /* 确保底盘降下来 */
            err_cmd = vision_bridge_apply_err_ramp(vision_bridge_calc_yaw_hold_err_degree(), 1U); /* 锁死方向冲出桥区 */
            speed_cmd = VISION_BRIDGE_TASK_EXIT_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            /* 底盘恢复后即可交还导航；此时下桥完成（包含视觉脱出与惯导脱出），正式关闭 Rolling 环并切入 FINISH */
            if ((vision_bridge_abs_f(servo_height - vision_bridge_restore_height_target()) < 0.2f) ||
                (s_bridge_task.state_ticks >= VISION_BRIDGE_TASK_EXIT_SETTLE_TICKS))
            {
                roll_balance_enable = 0U; /* 下桥脱出完成，正式关闭 Rolling 环 */
#if VISION_BRIDGE_TASK_NAV_CORRECT_ENABLE
                vision_bridge_apply_nav_correction(); /* 修正惯导 */
#endif
                vision_bridge_set_state(VISION_BRIDGE_TASK_FINISH);
            }
            break;

        /* --- 阶段 5：任务完成 --- */
        case VISION_BRIDGE_TASK_FINISH:
            vision_bridge_cleanup(0U); /* 保持最后的退出速度/航向，交给 Plan3 平滑接管 */
            break;

        /* --- 阶段 6：故障处理 --- */
        case VISION_BRIDGE_TASK_FAILSAFE:
        default:
            vision_bridge_cleanup(1U);
            break;
    }

    /* 把这一刻的状态广播出去 */
    vision_bridge_publish_status(packet, traveled_mm, err_cmd, speed_cmd);

#if DEBUG_LOG_ENABLE
    /* 0核 状态串口调试 (500ms 一条, 2ms tick): 状态机/滤波/退出线全链路可见 */
    {
        static uint32 ctrl_dbg_div = 0U;
        if ((ctrl_dbg_div++ % 250U) == 0U)
        {
            printf("[BridgeCtrl] st=%d tick=%lu trav=%.0f err=%.1f spd=%.0f e=%.3f ed=%.3f filt=%u/%u/%u b2v=%u src=%u m=%u gate=%u top=%u exit_y=%.1f\r\n",
                   (int)s_bridge_task.state,
                   (unsigned long)s_bridge_task.state_ticks,
                   (double)traveled_mm,
                   (double)err_cmd,
                   (double)speed_cmd,
                   (double)s_bridge_task.filtered_lateral_m,
                   (double)s_bridge_task.edot_mps,
                   (unsigned int)s_bridge_task.center_filter_valid,
                   (unsigned int)s_bridge_task.center_filter_lost_frames,
                   (unsigned int)s_bridge_task.center_filter_recover_frames,
                   (unsigned int)(packet ? packet->b2_valid : 0U),
                   (unsigned int)(packet ? packet->b2_source : 0U),
                   (unsigned int)(packet ? packet->b2_mode : 0U),
                   (unsigned int)(packet ? packet->b2_gate : 0U),
                   (unsigned int)(packet ? packet->b2_has_top : 0U),
                   (double)s_bridge_task.exit_line_y);
        }
    }
#endif
}

#endif
