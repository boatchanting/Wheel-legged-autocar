/**
 * @file    vision_bumpy_control.c
 * @brief   视觉凹凸路面控制模块实现
 * @details 该模块负责处理视觉系统检测到的凹凸路面信息，并生成相应的转向控制指令
 */
#include "vision/vision_bumpy_control.h"
#include "vision/vision_ipc_core0.h"
#include <math.h>

/* 全局变量定义区 */
volatile vision_bumpy_control_status_t g_vision_bumpy_control_status = {0};  ///< 凹凸路面控制状态全局变量(供外部访问)
volatile runtime_profiler_t g_vision_bumpy_control_profiler = {0};          ///< 运行时性能分析器
volatile uint8 g_bumpy_control_enable = VISION_BUMPY_CONTROL_DEFAULT_ACTIVE; ///< 凹凸路面控制使能标志

/* 内部变量定义区 */
static vision_bumpy_control_status_t g_bumpy_ctrl_shadow;  ///< 凹凸路面控制状态影子变量(内部使用)

/**
 * @brief   计算浮点数的绝对值
 * @param   value 输入值
 * @return  输入值的绝对值
 */
static float vision_bumpy_abs_f(float value)
{
    return (value < 0.0f) ? -value : value;
}

/**
 * @brief   将浮点数限制在指定范围内
 * @param   value 输入值
 * @param   min_value 最小值
 * @param   max_value 最大值
 * @return  限制在[min_value, max_value]范围内的值
 */
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

/**
 * @brief   根据视觉方向向量计算转向误差角度
 * @param   packet 视觉数据包指针
 * @return  计算得到的转向误差角度(度)
 * @details 方向向量以图像下方为前向（+Y），X 为横向；其相对前向的夹角
 *          直接作为方向 PID 的输入。
 */
static float vision_bumpy_calc_err_degree(const volatile vision_ipc_packet_t *packet)
{
    const float rad_to_deg = 57.2957795f;
    float err = -atan2f(packet->bumpy_direction_x, packet->bumpy_direction_y) * rad_to_deg;

    /* 方向控制仅基于视觉，不叠加惯导角度闭环；按图像误差直接映射。 */
    err = vision_bumpy_constrain_f(err, -VISION_BUMPY_MAX_ERR_DEG, VISION_BUMPY_MAX_ERR_DEG);  // 限制最大误差角度

    if (vision_bumpy_abs_f(err) < VISION_BUMPY_DEADBAND_DEG)  // 死区处理
    {
        err = 0.0f;
    }
    return err;
}

static void vision_bumpy_pid_reset(vision_bumpy_pid_t *pid)
{
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

static float vision_bumpy_pid_calc(vision_bumpy_pid_t *pid, float error)
{
    float derivative;
    pid->error = error;
    pid->integral += error;
    pid->integral = vision_bumpy_constrain_f(pid->integral, -VISION_BUMPY_PID_I_LIMIT, VISION_BUMPY_PID_I_LIMIT);
    derivative = pid->error - pid->last_error;
    pid->output = pid->Kp * pid->error + pid->Ki * pid->integral + pid->Kd * derivative;
    pid->last_error = pid->error;
    pid->output = vision_bumpy_constrain_f(pid->output, -VISION_BUMPY_MAX_ERR_DEG, VISION_BUMPY_MAX_ERR_DEG);
    return pid->output;
}

/**
 * @brief   应用空闲输出状态
 * @details 将控制状态设置为空闲，并清零误差指令
 */
static void vision_bumpy_apply_idle_outputs(void)
{
    g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_IDLE;  // 设置状态为空闲
    g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;           // 清零误差指令
    vision_bumpy_pid_reset(&g_bumpy_ctrl_shadow.pid);
    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow; // 更新全局状态
}

static void vision_bumpy_reset_exit_detection(void)
{
    g_bumpy_ctrl_shadow.exit_confirmed = 0U;
    g_bumpy_ctrl_shadow.miss_frame_count = 0U;
}

static void vision_bumpy_reset_entry_detection(void)
{
    g_bumpy_ctrl_shadow.entry_confirmed = 0U;
    g_bumpy_ctrl_shadow.detect_frame_count = 0U;
    g_bumpy_ctrl_shadow.last_frame_id = 0U;
}

static void vision_bumpy_reset_all_detection(void)
{
    vision_bumpy_reset_entry_detection();
    vision_bumpy_reset_exit_detection();
}

/**
 * @brief   初始化凹凸路面控制模块
 * @details 1. 清零影子变量
 *          2. 设置默认使能状态
 *          3. 重置性能分析器(如果启用)
 */
void VisionBumpyControl_Init(void)
{
    memset(&g_bumpy_ctrl_shadow, 0, sizeof(g_bumpy_ctrl_shadow));  // 清零影子变量
    g_bumpy_control_enable = VISION_BUMPY_CONTROL_DEFAULT_ACTIVE;   // 设置默认使能状态
    g_bumpy_ctrl_shadow.enabled = g_bumpy_control_enable;           // 更新影子变量中的使能状态
    g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_IDLE;              // 设置初始状态为空闲
    g_bumpy_ctrl_shadow.pid.Kp = VISION_BUMPY_PID_KP;
    g_bumpy_ctrl_shadow.pid.Ki = VISION_BUMPY_PID_KI;
    g_bumpy_ctrl_shadow.pid.Kd = VISION_BUMPY_PID_KD;
    vision_bumpy_pid_reset(&g_bumpy_ctrl_shadow.pid);
    vision_bumpy_reset_all_detection();
    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;             // 更新全局状态

#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_RESET(&g_vision_bumpy_control_profiler);         // 重置性能分析器
#endif
}

/**
 * @brief   设置凹凸路面控制使能状态
 * @param   enable 使能标志(1:使能, 0:禁用)
 * @details 如果禁用控制，将应用空闲输出状态
 */
void VisionBumpyControl_SetEnable(uint8 enable)
{
    g_bumpy_control_enable = enable ? 1U : 0U;            // 设置使能标志
    g_bumpy_ctrl_shadow.enabled = g_bumpy_control_enable; // 更新影子变量中的使能状态

    if (g_bumpy_control_enable == 0U)                     // 如果禁用控制
    {
        vision_bumpy_apply_idle_outputs();                // 应用空闲输出状态
    }
}

void VisionBumpyControl_ResetExitDetection(void)
{
    vision_bumpy_reset_all_detection();
    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;
}

void VisionBumpyControl_RearmExitDetection(void)
{
    vision_bumpy_reset_exit_detection();
    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;
}

uint8 VisionBumpyControl_IsEntryConfirmed(void)
{
    return g_vision_bumpy_control_status.entry_confirmed;
}

uint8 VisionBumpyControl_IsExitConfirmed(void)
{
    return g_vision_bumpy_control_status.exit_confirmed;
}

/**
 * @brief   获取凹凸路面控制使能状态
 * @return  当前使能状态(1:使能, 0:禁用)
 */
uint8 VisionBumpyControl_IsEnabled(void)
{
    return g_bumpy_control_enable;
}

/**
 * @brief   2ms周期调用的控制更新函数
 * @details 1. 检查使能状态
 *          2. 获取最新视觉数据包
 *          3. 处理数据包有效性
 *          4. 根据检测状态计算控制指令
 */
void VisionBumpyControl_Update_2ms(void)
{
#if VISION_BUMPY_CONTROL_ENABLE
    const volatile vision_ipc_packet_t *packet;  // 视觉数据包指针
    uint8 packet_is_bumpy;                        // 数据包是否包含凹凸路面信息
    uint8 packet_new;                             // 数据包是否为新数据

#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_BEGIN(g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);  // 开始性能分析
#endif

    g_bumpy_ctrl_shadow.enabled = g_bumpy_control_enable ? 1U : 0U;  // 更新使能状态
    if (g_bumpy_ctrl_shadow.enabled == 0U)                           // 如果未使能
    {
        vision_bumpy_apply_idle_outputs();                          // 应用空闲输出状态
#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);  // 结束性能分析
#endif
        return;
    }

    packet = VisionIpc_Core0_GetLatest();                           // 获取最新视觉数据包
    packet_new = (uint8)(packet->seq != g_bumpy_ctrl_shadow.last_seq);  // 检查是否为新数据包
    packet_is_bumpy = (uint8)((packet->valid_mask & VISION_VALID_BUMPY) != 0U);  // 检查数据包是否包含凹凸路面信息

    g_bumpy_ctrl_shadow.has_new_packet = packet_new;                // 更新新数据包标志
    if (packet_new)                                                 // 如果是新数据包
    {
        g_bumpy_ctrl_shadow.last_seq = packet->seq;                 // 更新序列号
        g_bumpy_ctrl_shadow.stale_ticks = 0U;                       // 清零数据包过期计数器
    }
    else if (g_bumpy_ctrl_shadow.stale_ticks < 0xFFFFU)             // 如果数据包未过期
    {
        g_bumpy_ctrl_shadow.stale_ticks++;                          // 增加数据包过期计数器
    }

    /* 检查数据包有效性 */
    if ((packet->seq == 0U) ||                                      // 数据包序列号为0(无效)
        (packet_is_bumpy == 0U) ||                                  // 数据包不包含凹凸路面信息
        (g_bumpy_ctrl_shadow.stale_ticks > VISION_BUMPY_STALE_TIMEOUT_TICKS))  // 数据包已过期
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_STALE;       // 设置状态为数据过期
        g_bumpy_ctrl_shadow.bumpy_detected = 0U;
        g_bumpy_ctrl_shadow.direction_x = 0.0f;
        g_bumpy_ctrl_shadow.direction_y = 0.0f;
        g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;                  // 清零误差指令
        vision_bumpy_pid_reset(&g_bumpy_ctrl_shadow.pid);
        // 视觉数据暂时无效不抹除已确认的“进入”事实，但出口统计必须重新开始。
        vision_bumpy_reset_exit_detection();
        g_bumpy_ctrl_shadow.detect_frame_count = 0U;
        g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;        // 更新全局状态
#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
        RUNTIME_PROFILE_END(&g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);  // 结束性能分析
#endif
        return;
    }

    /* 更新影子变量中的检测信息 */
    g_bumpy_ctrl_shadow.bumpy_detected = packet->bumpy_detected;
    g_bumpy_ctrl_shadow.direction_x = packet->bumpy_direction_x;
    g_bumpy_ctrl_shadow.direction_y = packet->bumpy_direction_y;

    /* 根据检测状态计算控制指令 */
    if (packet->bumpy_detected)
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_TRACK;          // 设置状态为跟踪
        g_bumpy_ctrl_shadow.err_degree_cmd = vision_bumpy_pid_calc(&g_bumpy_ctrl_shadow.pid, vision_bumpy_calc_err_degree(packet));  // 计算误差指令
    }
    else
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_SEARCH;         // 设置状态为搜索
        g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;                     // 清零误差指令
        vision_bumpy_pid_reset(&g_bumpy_ctrl_shadow.pid);
    }

    // 仅在新的视觉帧上计数，避免 2ms 控制周期重复统计同一帧。
    if (packet->frame_id != g_bumpy_ctrl_shadow.last_frame_id)
    {
        g_bumpy_ctrl_shadow.last_frame_id = packet->frame_id;
        if (packet->bumpy_detected)
        {
            if (g_bumpy_ctrl_shadow.entry_confirmed == 0U)
            {
                if (g_bumpy_ctrl_shadow.detect_frame_count < VISION_BUMPY_ENTRY_DETECT_FRAMES)
                {
                    g_bumpy_ctrl_shadow.detect_frame_count++;
                }
                if (g_bumpy_ctrl_shadow.detect_frame_count >= VISION_BUMPY_ENTRY_DETECT_FRAMES)
                {
                    g_bumpy_ctrl_shadow.entry_confirmed = 1U;
                }
            }
            g_bumpy_ctrl_shadow.miss_frame_count = 0U;
            g_bumpy_ctrl_shadow.exit_confirmed = 0U;
        }
        else if (g_bumpy_ctrl_shadow.miss_frame_count < VISION_BUMPY_EXIT_MISS_FRAMES)
        {
            if (g_bumpy_ctrl_shadow.entry_confirmed == 0U)
            {
                g_bumpy_ctrl_shadow.detect_frame_count = 0U;
            }
            g_bumpy_ctrl_shadow.miss_frame_count++;
            if (g_bumpy_ctrl_shadow.miss_frame_count >= VISION_BUMPY_EXIT_MISS_FRAMES)
            {
                g_bumpy_ctrl_shadow.exit_confirmed = 1U;
            }
        }
    }

    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow;              // 更新全局状态

#if VISION_BUMPY_CONTROL_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_vision_bumpy_control_profiler, VISION_BUMPY_CONTROL_PROFILE_TIMER);  // 结束性能分析
#endif
#endif
}

/**
 * @brief   获取转向误差指令
 * @return  当前转向误差指令(度)
 */
float VisionBumpyControl_GetErrDegreeCmd(void)
{
    return g_vision_bumpy_control_status.err_degree_cmd;
}
