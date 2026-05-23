/*
 * =================================================================================
 * 文件: vision_pvc_control.c
 * 作用: 0 核 (Core 0) PVC 入口控制的“大脑”。
 * 说明: 这个文件负责根据 1 核找出的 PVC 位置，计算出方向盘该怎么打、电机该怎么转。
 * =================================================================================
 */
#include "vision/vision_pvc_control.h"
#include "vision/vision_ipc_core0.h"

/* --- 外部变量引用 --- */
/* 这些是底盘控制的核心变量，我们要去修改它们来控制车子 */
extern volatile float err_degree;           /* 方向盘打角指令 */
extern volatile float target_speed_set;     /* 电机速度指令 */
extern int g_motor_enable;                  /* 电机有没有通电 */

/* --- 全局变量 --- */
volatile vision_pvc_control_status_t g_vision_pvc_control_status = {0}; /* 状态仪表盘 */
volatile runtime_profiler_t g_vision_pvc_control_profiler = {0};        /* 算力测速表 */
volatile uint8 g_pvc_control_enable = VISION_PVC_CONTROL_DEFAULT_ACTIVE;/* 控制开关 */

/* 内部用的影子变量，算完再一次性更新给外面看，防止数据看到一半变了 */
static vision_pvc_control_status_t g_pvc_ctrl_shadow;

/* --- 基础数学工具函数 --- */

/**
 * @brief 求浮点数的绝对值
 */
static float vision_pvc_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

/**
 * @brief 把数值限制在最小值和最大值之间（限幅）
 */
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

static void vision_pvc_pid_reset(vision_pvc_pid_t *pid)
{
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

static float vision_pvc_pid_calc(vision_pvc_pid_t *pid, float error)
{
    const float derivative = error - pid->last_error;

    pid->error = error;
    pid->integral += error;
    pid->integral = vision_pvc_constrain_f(pid->integral,
                                           -VISION_PVC_CONTROL_PID_I_LIMIT,
                                           VISION_PVC_CONTROL_PID_I_LIMIT);
    pid->output = pid->Kp * pid->error +
                  pid->Ki * pid->integral +
                  pid->Kd * derivative;
    pid->last_error = pid->error;
    pid->output = vision_pvc_constrain_f(pid->output,
                                         -VISION_PVC_CONTROL_MAX_ERR_DEG,
                                         VISION_PVC_CONTROL_MAX_ERR_DEG);
    return pid->output;
}

/* --- 核心计算函数 --- */

/**
 * @brief 根据 PVC 的位置，计算方向盘该打多少度
 * 
 * @param packet 1 核发过来的数据包
 * @return float 算出的方向盘角度（有最大值限制）
 * 
 * @note 这里的逻辑是：你偏离中心越多，方向盘就打得越死。
 */
static float vision_pvc_calc_err_degree(const volatile vision_ipc_packet_t *packet)
{
    const float steer_error_px =
        (float)packet->pvc_steer_error_px_x100 * 0.01f + VISION_PVC_CONTROL_STEER_OFFSET_PX;
    /* 横向偏差算出来的打角 */
    const float lateral_deg =
        steer_error_px * VISION_PVC_CONTROL_K_STEER_DEG_PER_PX;
    /* 车头偏角算出来的打角（目前 1 核没算这个，传过来的是 0） */
    const float yaw_deg =
        ((float)packet->pvc_yaw_error_deg_x100 * 0.01f) * VISION_PVC_CONTROL_K_YAW_DEG_PER_DEG;
    /* 两个加起来，像素误差定义已经包含左右方向 */
    const float err = lateral_deg + yaw_deg;

    /* 限幅，别把舵机打坏了 */
    return vision_pvc_constrain_f(err,
                                  -VISION_PVC_CONTROL_MAX_ERR_DEG,
                                  VISION_PVC_CONTROL_MAX_ERR_DEG);
}

/**
 * @brief 计算画面里 PVC 的面积占了整张照片的千分之几
 * 
 * @param packet 1 核发过来的数据包
 * @return uint16 比例（0 ~ 1000。1000 就是 100%）
 * 
 * @note 为什么算这个？因为有时候测距离不准，但只要整个屏幕白花花的都是 PVC，
 *       那就说明车子已经开进去了，可以停车了。
 */
static uint16 vision_pvc_calc_bbox_ratio_u16(const volatile vision_ipc_packet_t *packet)
{
    uint16 width;
    uint16 height;
    uint32 bbox_area;

    /* 如果连个包围框都没有，或者数据是错的，那就当 0 处理 */
    if ((packet->pvc_bbox_xmin == 0xFFU) ||
        (packet->pvc_bbox_ymin == 0xFFU) ||
        (packet->pvc_bbox_xmax < packet->pvc_bbox_xmin) ||
        (packet->pvc_bbox_ymax < packet->pvc_bbox_ymin))
    {
        return 0U;
    }

    /* 算出包围框的宽和高 */
    width = (uint16)(packet->pvc_bbox_xmax - packet->pvc_bbox_xmin + 1U);
    height = (uint16)(packet->pvc_bbox_ymax - packet->pvc_bbox_ymin + 1U);
    bbox_area = (uint32)width * (uint32)height;

    /* 如果算出来的面积比整张照片还大（基本不可能），就当 100% */
    if (bbox_area >= VISION_PVC_CONTROL_IMAGE_AREA)
    {
        return 1000U;
    }

    /* 返回千分比 */
    return (uint16)((bbox_area * 1000U) / VISION_PVC_CONTROL_IMAGE_AREA);
}

/**
 * @brief 把状态机恢复到“空闲”状态
 * @note  没在控制车子时，速度和打角都归零。
 */
static void vision_pvc_apply_idle_outputs(void)
{
    g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_IDLE;
    g_pvc_ctrl_shadow.speed_cmd = 0.0f;
    g_pvc_ctrl_shadow.err_degree_cmd = 0.0f;
    vision_pvc_pid_reset(&g_pvc_ctrl_shadow.pid);
    g_vision_pvc_control_status = g_pvc_ctrl_shadow;
}

/* --- 对外接口函数 --- */

/**
 * @brief 模块初始化
 * @note  开机时调用。把仪表盘清零，并告诉 1 核：“开始检测 PVC！”
 */
void VisionPvcControl_Init(void)
{
    memset(&g_pvc_ctrl_shadow, 0, sizeof(g_pvc_ctrl_shadow));
    g_pvc_control_enable = VISION_PVC_CONTROL_DEFAULT_ACTIVE;
    g_pvc_ctrl_shadow.enabled = g_pvc_control_enable;
    g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_IDLE;
    g_pvc_ctrl_shadow.pid.Kp = VISION_PVC_CONTROL_PID_KP;
    g_pvc_ctrl_shadow.pid.Ki = VISION_PVC_CONTROL_PID_KI;
    g_pvc_ctrl_shadow.pid.Kd = VISION_PVC_CONTROL_PID_KD;
    vision_pvc_pid_reset(&g_pvc_ctrl_shadow.pid);
    g_vision_pvc_control_status = g_pvc_ctrl_shadow;

#if VISION_PVC_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_RESET(&g_vision_pvc_control_profiler);
#endif

    /*
     * 告诉 1 核：请运行 PVC 检测。
     * （即使 0 核还没开始用这些数据控车，也先让 1 核跑起来，这样看屏幕调试比较方便）
     */
    VisionIpc_Core0_SetPvcEnable(VISION_PVC_DETECT_DEFAULT_ACTIVE);
}

/**
 * @brief 开启或关闭 PVC 控制
 * 
 * @param enable 1: 0核开始接管方向和油门; 0: 0核放手
 */
void VisionPvcControl_SetEnable(uint8 enable)
{
    g_pvc_control_enable = enable ? 1U : 0U;
    g_pvc_ctrl_shadow.enabled = g_pvc_control_enable;
    
    /* 如果关掉了，就把发给底盘的指令清零 */
    if (g_pvc_control_enable == 0U)
    {
        vision_pvc_apply_idle_outputs();
    }
}

/**
 * @brief 看看现在是不是在接管车子
 */
uint8 VisionPvcControl_IsEnabled(void)
{
    return g_pvc_control_enable;
}

/* --- 核心控制状态机 --- */

/**
 * @brief PVC 控制模块的心脏（每 2 毫秒调用一次）
 * 
 * @note 逻辑流程：
 *       1. 看 1 核有没有发新照片数据来。
 *       2. 如果 1 核说没看到 PVC：进入搜索模式，龟速往前挪。
 *       3. 如果 1 核说看到了：
 *          - 如果离得很远：进入跟踪模式，快速跑过去。
 *          - 如果离得近了：进入接近模式，踩刹车减速。
 *          - 如果已经压在上面了：进入到达模式，停车。
 */
void VisionPvcControl_Update_2ms(void)
{
#if VISION_PVC_CONTROL_ENABLE
    const volatile vision_ipc_packet_t *packet;
    uint8 packet_is_pvc;
    uint8 packet_new;

#if VISION_PVC_CONTROL_PROFILE_ENABLE
    /* 掐表开始 */
    RUNTIME_PROFILE_BEGIN(g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif

    /* 更新仪表盘上的使能状态 */
    g_pvc_ctrl_shadow.enabled = g_pvc_control_enable ? 1U : 0U;

    /* 如果没开启接管，就老老实实发空指令，然后退出 */
    if (g_pvc_ctrl_shadow.enabled == 0U)
    {
        vision_pvc_apply_idle_outputs();
#if VISION_PVC_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    /* 1. 拿 1 核最新送来的数据 */
    packet = VisionIpc_Core0_GetLatest();
    /* 判断是不是新照片 */
    packet_new = (uint8)(packet->seq != g_pvc_ctrl_shadow.last_seq);
    /* 判断这包数据里有没有 PVC 信息 */
    packet_is_pvc = (uint8)((packet->valid_mask & VISION_VALID_PVC) != 0U);

    g_pvc_ctrl_shadow.has_new_packet = packet_new;
    if (packet_new)
    {
        g_pvc_ctrl_shadow.last_seq = packet->seq; /* 记下照片编号 */
        g_pvc_ctrl_shadow.stale_ticks = 0U;       /* 没收到照片的时间清零 */
    }
    else if (g_pvc_ctrl_shadow.stale_ticks < 0xFFFFU)
    {
        g_pvc_ctrl_shadow.stale_ticks++;          /* 没收到新照片，计时器加 1 */
    }

    /* 如果电机没开，或者惯导还没初始化好，也不能控车 */
    if ((g_motor_enable == 0) || (g_yaw_initialized == 0U))
    {
        vision_pvc_apply_idle_outputs();
#if VISION_PVC_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    /* 2. 如果数据无效，或者太久没收到新数据（比如 1 核死机了），进入故障模式 */
    if ((packet->seq == 0U) ||
        (packet_is_pvc == 0U) ||
        (g_pvc_ctrl_shadow.stale_ticks > VISION_PVC_CONTROL_STALE_TIMEOUT_TICKS))
    {
        g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_STALE; /* 数据过期了 */
        g_pvc_ctrl_shadow.stable_detected = 0U;
        g_pvc_ctrl_shadow.raw_detected = 0U;
        g_pvc_ctrl_shadow.target_x_px_x100 = 0;
        g_pvc_ctrl_shadow.steer_error_px_x100 = 0;
        g_pvc_ctrl_shadow.forward_mm = -1;
        g_pvc_ctrl_shadow.lateral_mm = 0;
        g_pvc_ctrl_shadow.yaw_error_deg_x100 = 0;
        g_pvc_ctrl_shadow.bbox_area_ratio_u16 = 0U;
        
        g_pvc_ctrl_shadow.err_degree_cmd = 0.0f;
        g_pvc_ctrl_shadow.speed_cmd = 0.0f;
        vision_pvc_pid_reset(&g_pvc_ctrl_shadow.pid);
        err_degree = 0.0f; /* 停车并回正方向盘 */
        target_speed_set = 0.0f;
        
        g_vision_pvc_control_status = g_pvc_ctrl_shadow;
#if VISION_PVC_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
        return;
    }

    /* 把 1 核的数据抄到仪表盘上 */
    g_pvc_ctrl_shadow.stable_detected = packet->pvc_stable_detected;
    g_pvc_ctrl_shadow.raw_detected = packet->pvc_detected;
    g_pvc_ctrl_shadow.target_x_px_x100 = packet->pvc_target_x_px_x100;
    g_pvc_ctrl_shadow.steer_error_px_x100 = packet->pvc_steer_error_px_x100;
    g_pvc_ctrl_shadow.forward_mm = packet->pvc_forward_mm;
    g_pvc_ctrl_shadow.lateral_mm = packet->pvc_lateral_mm;
    g_pvc_ctrl_shadow.yaw_error_deg_x100 = packet->pvc_yaw_error_deg_x100;
    g_pvc_ctrl_shadow.bbox_area_ratio_u16 = vision_pvc_calc_bbox_ratio_u16(packet);

    /* 3. 如果 1 核确认看到了 PVC */
    if (packet->pvc_stable_detected)
    {
        const int16 forward_mm = packet->pvc_forward_mm;
        /* 算算方向盘该打多少 */
        const float turn_err = vision_pvc_pid_calc(&g_pvc_ctrl_shadow.pid,
                                                   vision_pvc_calc_err_degree(packet));
        /* 看看画面是不是被 PVC 占满了 */
        const uint8 bbox_stop = (uint8)(g_pvc_ctrl_shadow.bbox_area_ratio_u16 >=
                                        VISION_PVC_CONTROL_STOP_BBOX_RATIO_U16);

        g_pvc_ctrl_shadow.err_degree_cmd = turn_err;
        err_degree = turn_err; /* 转方向盘 */

        /* 阶段 A：如果画面满了，或者距离小于“到达门槛” */
        if (bbox_stop ||
            ((forward_mm >= 0) && (forward_mm <= VISION_PVC_CONTROL_ARRIVE_FORWARD_MM)))
        {
            g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_ARRIVED; /* 到了！ */
            g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_ARRIVE_SPEED_SET; /* 停车 */
        }
        /* 阶段 B：如果距离小于“接近门槛” */
        else if ((forward_mm >= 0) && (forward_mm <= VISION_PVC_CONTROL_CLOSE_FORWARD_MM))
        {
            g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_TRACK; /* 还在跑 */
            g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_CLOSE_SPEED_SET; /* 但要减速了 */
        }
        /* 阶段 C：离得还远 */
        else
        {
            g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_TRACK; /* 正常跑 */
            g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_TRACK_SPEED_SET; /* 冲！ */
        }

        target_speed_set = g_pvc_ctrl_shadow.speed_cmd;
    }
    /* 4. 如果没确认，但这一瞬间仿佛看到了（不太稳定） */
    else if (packet->pvc_detected)
    {
        const float turn_err = vision_pvc_pid_calc(&g_pvc_ctrl_shadow.pid,
                                                   vision_pvc_calc_err_degree(packet) * 0.5f);

        g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_SEARCH; /* 搜索模式 */
        g_pvc_ctrl_shadow.err_degree_cmd = turn_err; /* 既然不确定，方向盘就打轻一点（减半） */
        g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_SEARCH_SPEED_SET; /* 慢点开 */
        
        err_degree = g_pvc_ctrl_shadow.err_degree_cmd;
        target_speed_set = g_pvc_ctrl_shadow.speed_cmd;
    }
    /* 5. 啥也没看到 */
    else
    {
        g_pvc_ctrl_shadow.state = VISION_PVC_CTRL_SEARCH; /* 搜索模式 */
        g_pvc_ctrl_shadow.err_degree_cmd = 0.0f; /* 找不到？那就直着往前开 */
        g_pvc_ctrl_shadow.speed_cmd = VISION_PVC_CONTROL_SEARCH_SPEED_SET; /* 慢点开 */
        vision_pvc_pid_reset(&g_pvc_ctrl_shadow.pid);
        
        err_degree = 0.0f;
        target_speed_set = g_pvc_ctrl_shadow.speed_cmd;
    }

    /* 死区设置：如果方向盘偏角非常小（小于 0.3 度），干脆就不打了，防止车子在直道上画龙 */
    if (vision_pvc_abs_f(g_pvc_ctrl_shadow.err_degree_cmd) < VISION_PVC_CONTROL_DEADBAND_DEG)
    {
        g_pvc_ctrl_shadow.err_degree_cmd = 0.0f;
        err_degree = 0.0f;
    }

    /* 把这一刻的计算结果挂在仪表盘上，给别人看 */
    g_vision_pvc_control_status = g_pvc_ctrl_shadow;

#if VISION_PVC_CONTROL_PROFILE_ENABLE
    /* 掐表结束 */
    RUNTIME_PROFILE_END(&g_vision_pvc_control_profiler, VISION_PVC_CONTROL_PROFILE_TIMER);
#endif
#endif
}
