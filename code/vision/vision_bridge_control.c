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
#include "vision/vision_pvc_control.h"
#include "plan/bridge.h"

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

/* --- 全局变量 --- */
volatile uint8 g_bridge_vision_task_enable = 0U; /* 任务总开关，别人可以把它设为 1 来启动任务 */
volatile vision_bridge_task_status_t g_bridge_vision_task_status = {0}; /* 记录当前任务的详细状态供外人看 */

/* --- 内部数据结构 --- */
/**
 * @brief 桥梁任务内部的“记事本”
 * @note  记录了任务进行到了哪一步、起点的坐标是多少、上桥前方向是多少等。
 */
typedef struct
{
    vision_bridge_task_state_e state; /* 当前处于哪个阶段 */
    uint32 state_ticks;               /* 在这个阶段待了多久了（每个 tick 是 2ms） */
    uint16 exit_lost_ticks;           /* 下桥阶段：连续多少次没看到线了 */
    uint16 bridge_hold_ticks;         /* 看见桥面黑块后的“闭眼盲跑”倒计时 */
    uint16 align_ok_ticks;            /* 桥头对齐：连续多少次对准了 */
    uint32 last_seq;                  /* 上次处理的数据包序号（防止处理重复数据） */
    float start_x_mm;                 /* 上桥那一刻的 X 坐标（惯导） */
    float start_y_mm;                 /* 上桥那一刻的 Y 坐标（惯导） */
    float exit_start_x_mm;            /* 开始下桥那一刻的 X 坐标 */
    float exit_start_y_mm;            /* 开始下桥那一刻的 Y 坐标 */
    float locked_yaw_deg;             /* 上桥前锁定的车头朝向（如果桥上看不见线，就照着这个方向开） */
    int32 saved_acc_limit;            /* 备份原来的加速度限制，下桥后恢复 */
    int32 saved_dec_limit;            /* 备份原来的减速度限制，下桥后恢复 */
    uint8 saved_limits_valid;         /* 标记备份数据是否有效 */
} vision_bridge_task_ctx_t;

/* 这个就是真正的“记事本”本尊，只有这个文件能用 */
static vision_bridge_task_ctx_t s_bridge_task;

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

/* --- 控制核心辅助函数 --- */

/**
 * @brief 根据 1 核传来的直线数据，算出方向盘该打多少度
 * 
 * @param packet 1 核传来的数据包
 * @return float 算出的方向盘打角（有最大值限制）
 * 
 * @note 公式：误差 = 横向偏差 * 横向比例 + 角度偏差 * 角度比例
 */
static float vision_bridge_calc_line_err_degree(const volatile vision_ipc_packet_t *packet)
{
    /* 把放大了 100 倍的整数还原成浮点数 */
    const float lateral_px = (float)packet->line_lateral_px_x100 * 0.01f;
    const float yaw_deg = (float)packet->line_yaw_error_deg_x100 * 0.01f;
    /* 算综合误差 */
    const float err = VISION_BRIDGE_TASK_LINE_SIGN *
                      (lateral_px * VISION_BRIDGE_TASK_K_LAT_DEG_PER_PX +
                       yaw_deg * VISION_BRIDGE_TASK_K_YAW_DEG_PER_DEG);

    /* 限制在最大允许的范围内，防止车子突然猛打方向 */
    return vision_bridge_constrain_f(err,
                                     -VISION_BRIDGE_TASK_MAX_ERR_DEG,
                                     VISION_BRIDGE_TASK_MAX_ERR_DEG);
}

/**
 * @brief 在桥上盲跑时，根据惯导保持车头方向
 * 
 * @return float 算出的方向盘打角
 * 
 * @note 如果在桥上看不见线，就照着上桥前记下的方向开，偏了就用惯导纠正。
 */
static float vision_bridge_calc_yaw_hold_err(void)
{
    /* 误差 = 目标方向 - 当前惯导测出的方向 */
    const float err = vision_bridge_normalize_angle(s_bridge_task.locked_yaw_deg -
                                                   inertial_nav.relative_yaw);
    /* 限制在盲跑允许的最大范围内 */
    return vision_bridge_constrain_f(err,
                                     -VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG,
                                     VISION_BRIDGE_TASK_YAW_HOLD_MAX_ERR_DEG);
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
    acc_limit = bridge_params.servo_acc_bridge;
    dec_limit = bridge_params.servo_dec_bridge;
    roll_balance_enable = 1U;
    /* 抬高底盘，并且规定抬高的速度（步长） */
    Bridge_Apply_Height_Control(bridge_params.height_bridge,
                                bridge_params.height_step_rise * VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE);
}

/**
 * @brief 切换回“正常姿态”
 * @note  恢复加速度限制、关掉滚转平衡、把底盘降下来。
 */
static void vision_bridge_apply_normal_posture(void)
{
    roll_balance_enable = 0U;
    if (s_bridge_task.saved_limits_valid)
    {
        acc_limit = s_bridge_task.saved_acc_limit;
        dec_limit = s_bridge_task.saved_dec_limit;
    }
    /* 降下底盘 */
    Bridge_Apply_Height_Control(bridge_params.height_normal,
                                bridge_params.height_step_drop * VISION_BRIDGE_TASK_HEIGHT_STEP_SCALE);
}

/**
 * @brief 切换到下一个任务阶段
 * 
 * @param next_state 下一个阶段是什么
 * 
 * @note 切换时，把计时器清零。如果是切换到“下桥”，还要记下下桥那一刻的位置。
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
    /* 告诉 1 核：不用再找 PVC 和直线了 */
    VisionPvcControl_SetEnable(0U);
    VisionIpc_Core0_SetBridgeLineEnable(0U);
    
    /* 恢复正常姿态 */
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
}

/**
 * @brief 启动桥梁任务
 * @note  当主程序觉得快到桥了，调用这个函数启动整个流程。
 */
void VisionBridgeTask_Start(void)
{
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
 * @note  把当前位置设为起点，并告诉 1 核：“开始找 PVC 入口！”
 */
static void vision_bridge_enter_task(void)
{
    memset(&s_bridge_task, 0, sizeof(s_bridge_task));
    s_bridge_task.state = VISION_BRIDGE_TASK_ENTER_PVC; /* 第一步：进 PVC */
    s_bridge_task.start_x_mm = inertial_nav.x;
    s_bridge_task.start_y_mm = inertial_nav.y;
    s_bridge_task.locked_yaw_deg = inertial_nav.relative_yaw;
    s_bridge_task.saved_acc_limit = acc_limit;
    s_bridge_task.saved_dec_limit = dec_limit;
    s_bridge_task.saved_limits_valid = 1U;

    g_special_action_trigger = 1U; /* 告诉系统我接管车子了 */
    
    /* 告诉 1 核找 PVC */
    VisionIpc_Core0_SetTask(VISION_TARGET_PVC_ENTRY, VISION_MASK_PVC_ENTRY);
    VisionPvcControl_SetEnable(1U);
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
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U))
    {
        vision_bridge_cleanup(1U);
        return;
    }

    /* 如果刚刚被叫醒，准备开始任务 */
    if (s_bridge_task.state == VISION_BRIDGE_TASK_IDLE)
    {
        vision_bridge_enter_task();
    }

    s_bridge_task.state_ticks++; /* 计时器滴答一下 */
    /* 算算从起点到现在跑了多远了 */
    traveled_mm = vision_bridge_distance_from(s_bridge_task.start_x_mm, s_bridge_task.start_y_mm);

    /* 看看现在走到哪一步了 */
    switch (s_bridge_task.state)
    {
        /* --- 阶段 1：通过 PVC 入口接近桥梁 --- */
        case VISION_BRIDGE_TASK_ENTER_PVC:
            /* 直接抄 PVC 控制模块算好的指令 */
            err_cmd = g_vision_pvc_control_status.err_degree_cmd;
            speed_cmd = g_vision_pvc_control_status.speed_cmd;

            /* 如果 PVC 模块说“我已经走到头了（ARRIVED）” */
            if (g_vision_pvc_control_status.state == VISION_PVC_CTRL_ARRIVED)
            {
                s_bridge_task.align_ok_ticks++;
            }
            else
            {
                s_bridge_task.align_ok_ticks = 0U;
            }

            /* 如果连续确认到了头，或者时间太长超时了，赶紧进入下一步：对齐！ */
            if ((s_bridge_task.align_ok_ticks >= 50U) ||
                (s_bridge_task.state_ticks >= VISION_BRIDGE_TASK_ENTER_TIMEOUT_TICKS))
            {
                VisionPvcControl_SetEnable(0U); /* 不看 PVC 了 */
                VisionIpc_Core0_SetBridgeLineEnable(1U); /* 让 1 核改看直线和桥 */
                
                target_speed_set = 0.0f; /* 停车 */
                err_degree = 0.0f;       /* 方向盘回正 */
                
                /* 把现在的位置设为新的起点 */
                s_bridge_task.start_x_mm = inertial_nav.x;
                s_bridge_task.start_y_mm = inertial_nav.y;
                s_bridge_task.locked_yaw_deg = inertial_nav.relative_yaw;
                s_bridge_task.exit_lost_ticks = 0U;
                s_bridge_task.bridge_hold_ticks = 0U;
                vision_bridge_set_state(VISION_BRIDGE_TASK_ALIGN);
            }
            break;

        /* --- 阶段 2：在桥头对齐方向 --- */
        case VISION_BRIDGE_TASK_ALIGN:
            speed_cmd = VISION_BRIDGE_TASK_ALIGN_SPEED_SET; /* 停车对齐 */
            target_speed_set = speed_cmd;

            /* 惊喜！如果在对齐的时候直接看到桥了，那还等什么，直接上！ */
            if (packet->line_bridge_stable_detected)
            {
                s_bridge_task.bridge_hold_ticks = VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS;
                err_cmd = vision_bridge_calc_yaw_hold_err(); /* 锁死方向 */
                err_degree = err_cmd;
                vision_bridge_apply_high_posture(); /* 抬高底盘，防侧翻 */
                vision_bridge_set_state(VISION_BRIDGE_TASK_RUN); /* 冲！ */
                break;
            }

            /* 如果看到的是直线，就根据直线来修方向盘 */
            if (packet->line_stable_detected)
            {
                err_cmd = vision_bridge_calc_line_err_degree(packet);
                err_degree = err_cmd;
                /* 如果误差很小，说明对准了 */
                if ((vision_bridge_abs_f(err_cmd) <= VISION_BRIDGE_TASK_ALIGN_ERR_TOL_DEG) &&
                    (vision_bridge_abs_f((float)packet->line_yaw_error_deg_x100 * 0.01f) <=
                     VISION_BRIDGE_TASK_ALIGN_YAW_TOL_DEG))
                {
                    s_bridge_task.align_ok_ticks++; /* 对准次数 +1 */
                }
                else
                {
                    s_bridge_task.align_ok_ticks = 0U; /* 没对准，重新数 */
                }
            }
            else
            {
                /* 啥也看不见，那就保持原来的方向别动 */
                err_cmd = vision_bridge_calc_yaw_hold_err();
                err_degree = err_cmd;
                s_bridge_task.align_ok_ticks = 0U;
            }

            /* 如果对准了好几次，或者实在是对不准超时了，只能硬着头皮上了 */
            if ((s_bridge_task.align_ok_ticks >= VISION_BRIDGE_TASK_ALIGN_OK_TICKS) ||
                (s_bridge_task.state_ticks >= VISION_BRIDGE_TASK_ALIGN_TIMEOUT_TICKS))
            {
                s_bridge_task.start_x_mm = inertial_nav.x;
                s_bridge_task.start_y_mm = inertial_nav.y;
                s_bridge_task.locked_yaw_deg = inertial_nav.relative_yaw;
                s_bridge_task.exit_lost_ticks = 0U;
                vision_bridge_set_state(VISION_BRIDGE_TASK_RUN); /* 冲！ */
            }
            break;

        /* --- 阶段 3：在桥上跑 --- */
        case VISION_BRIDGE_TASK_RUN:
            /* 看到桥梁黑块了，刷新“防抖”倒计时 */
            if (packet->line_bridge_stable_detected)
            {
                s_bridge_task.bridge_hold_ticks = VISION_BRIDGE_TASK_BRIDGE_HOLD_TICKS;
            }
            else if (s_bridge_task.bridge_hold_ticks > 0U)
            {
                s_bridge_task.bridge_hold_ticks--; /* 没看到，倒计时减 1 */
            }

            /* 如果倒计时没归零，说明现在车还在桥上 */
            if (s_bridge_task.bridge_hold_ticks > 0U)
            {
                vision_bridge_apply_high_posture(); /* 保持高底盘 */
                err_cmd = vision_bridge_calc_yaw_hold_err(); /* 盲跑，锁死方向 */
                speed_cmd = VISION_BRIDGE_TASK_BRIDGE_SPEED_SET; /* 桥上速度 */
            }
            else
            {
                /* 如果归零了，说明可能快下桥了或者在桥的平缓段 */
                vision_bridge_apply_normal_posture(); /* 降下底盘 */
                /* 如果能看到地上的线，就跟着线跑 */
                if (packet->line_stable_detected)
                {
                    err_cmd = vision_bridge_calc_line_err_degree(packet);
                    speed_cmd = VISION_BRIDGE_TASK_RUN_SPEED_SET;
                }
                else
                {
                    /* 线也看不见，只能盲跑了 */
                    err_cmd = vision_bridge_calc_yaw_hold_err();
                    speed_cmd = VISION_BRIDGE_TASK_BLIND_SPEED_SET;
                }
            }

            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            /* 至少跑够一定距离（比如 1 米）才允许判断是不是下桥了 */
            if (traveled_mm >= VISION_BRIDGE_TASK_RUN_MIN_MM)
            {
                /* 怎么判断下桥？线看不见 + 桥看不见 + 倒计时归零 + 画面里的白赛道不多 */
                const uint8 no_visual = (uint8)((packet->line_stable_detected == 0U) &&
                                                (packet->line_bridge_stable_detected == 0U) &&
                                                (s_bridge_task.bridge_hold_ticks == 0U) &&
                                                (packet->line_roi_white_ratio_u16 <=
                                                 VISION_BRIDGE_TASK_EXIT_WHITE_RATIO_U16));
                if (no_visual)
                {
                    s_bridge_task.exit_lost_ticks++; /* 疑似下桥次数 +1 */
                }
                else
                {
                    s_bridge_task.exit_lost_ticks = 0U; /* 误报，重新数 */
                }
            }

            /* 如果连续很多次都疑似下桥，或者跑得太远了（超时保护），进入下桥阶段 */
            if ((s_bridge_task.exit_lost_ticks >= VISION_BRIDGE_TASK_EXIT_LOST_TICKS) ||
                (traveled_mm >= VISION_BRIDGE_TASK_RUN_MAX_MM))
            {
                vision_bridge_set_state(VISION_BRIDGE_TASK_EXIT);
            }
            break;

        /* --- 阶段 4：下桥缓冲 --- */
        case VISION_BRIDGE_TASK_EXIT:
            vision_bridge_apply_normal_posture(); /* 确保底盘降下来 */
            err_cmd = vision_bridge_calc_yaw_hold_err(); /* 锁死方向冲出桥区 */
            speed_cmd = VISION_BRIDGE_TASK_EXIT_SPEED_SET;
            err_degree = err_cmd;
            target_speed_set = speed_cmd;

            /* 再往前开一小段距离，且底盘已经降到位了，任务才算彻底结束 */
            if ((vision_bridge_distance_from(s_bridge_task.exit_start_x_mm,
                                             s_bridge_task.exit_start_y_mm) >=
                 VISION_BRIDGE_TASK_EXIT_BUFFER_MM) &&
                (vision_bridge_abs_f(servo_height - bridge_params.height_normal) < 0.2f))
            {
#if VISION_BRIDGE_TASK_NAV_CORRECT_ENABLE
                vision_bridge_apply_nav_correction(); /* 修正惯导 */
#endif
                vision_bridge_set_state(VISION_BRIDGE_TASK_FINISH);
            }
            break;

        /* --- 阶段 5：任务完成 --- */
        case VISION_BRIDGE_TASK_FINISH:
            vision_bridge_cleanup(1U); /* 打扫战场，交还控制权 */
            break;

        /* --- 阶段 6：故障处理 --- */
        case VISION_BRIDGE_TASK_FAILSAFE:
        default:
            vision_bridge_cleanup(1U);
            break;
    }

    /* 把这一刻的状态广播出去 */
    vision_bridge_publish_status(packet, traveled_mm, err_cmd, speed_cmd);
}

#endif
