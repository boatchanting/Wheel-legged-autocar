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
 * @brief   根据视觉方向向量计算原始转向误差角度
 * @param   packet 视觉数据包指针
 * @return  原始转向误差角度(度), 仅做 atan2 转换, 不做滤波/限幅/死区
 * @details 方向向量以图像下方为前向（+Y），X 为横向；其相对前向的夹角
 *          作为后续滤波链的原始输入。
 *          滤波、死区、限幅在 Update_2ms 的滤波链中统一处理。
 */
static float vision_bumpy_calc_err_degree(const volatile vision_ipc_packet_t *packet)
{
    const float rad_to_deg = 57.2957795f;
    return -atan2f(packet->bumpy_direction_x, packet->bumpy_direction_y) * rad_to_deg;
}

/**
 * @brief   3点中值滤波
 * @param   a, b, c 三个候选值
 * @return  排序后的中值
 * @details 用于剔除单帧脉冲离群噪声（反光、遮挡、误检等瞬时干扰）。
 *          相比均值滤波，中值滤波对脉冲噪声是"完全剔除法"而非"稀释法"。
 */
static float vision_bumpy_median3(float a, float b, float c)
{
    /* 3元素排序网络: 最多3次比较+交换即可得到中值 */
    if (a > b) { float t = a; a = b; b = t; }
    if (a > c) { float t = a; a = c; c = t; }
    if (b > c) { float t = b; b = c; c = t; }
    return b;
}

/**
 * @brief   一阶IIR低通滤波器步进
 * @param   state 滤波器状态指针(保存上次输出值, 就地更新)
 * @param   input 当前输入值
 * @param   alpha 滤波系数(0~1, 越小越平滑, 0.15 → fc≈12Hz @2ms周期)
 * @return  滤波后输出值
 * @details 差分方程: y[n] = α·x[n] + (1-α)·y[n-1]
 *          传递函数: H(z) = α / (1 - (1-α)z⁻¹)
 */
static float vision_bumpy_lpf_step(float *state, float input, float alpha)
{
    *state = alpha * input + (1.0f - alpha) * (*state);
    return *state;
}

/**
 * @brief   滤波器状态初始化(首次进入TRACK或状态转入时调用)
 * @param   e_raw 首帧原始角度误差(度)
 * @details 用首帧值填充中值历史缓冲区和LPF状态, 实现零延迟启动。
 *          避免滤波器从0平滑到目标值的启动斜坡（否则首帧仅输出 α*e_raw）。
 */
static void vision_bumpy_filter_init(float e_raw)
{
    g_bumpy_ctrl_shadow.raw_err_history[0] = e_raw;
    g_bumpy_ctrl_shadow.raw_err_history[1] = e_raw;
    g_bumpy_ctrl_shadow.raw_err_history[2] = e_raw;
    g_bumpy_ctrl_shadow.history_idx = 0;
    g_bumpy_ctrl_shadow.filter_ready = 1U;
    g_bumpy_ctrl_shadow.lpf_state = e_raw;
}

/**
 * @brief   滤波器状态复位(退出TRACK时调用)
 * @details 清零中值历史、LPF状态和就绪标志, 确保下次进入TRACK时干净启动
 */
static void vision_bumpy_filter_reset(void)
{
    g_bumpy_ctrl_shadow.raw_err_history[0] = 0.0f;
    g_bumpy_ctrl_shadow.raw_err_history[1] = 0.0f;
    g_bumpy_ctrl_shadow.raw_err_history[2] = 0.0f;
    g_bumpy_ctrl_shadow.history_idx = 0;
    g_bumpy_ctrl_shadow.filter_ready = 0U;
    g_bumpy_ctrl_shadow.lpf_state = 0.0f;
}

/**
 * @brief   应用空闲输出状态
 * @details 将控制状态设置为空闲，并清零误差指令
 */
static void vision_bumpy_apply_idle_outputs(void)
{
    g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_IDLE;  // 设置状态为空闲
    g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;           // 清零误差指令
    vision_bumpy_filter_reset();
    g_vision_bumpy_control_status = g_bumpy_ctrl_shadow; // 更新全局状态
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
    vision_bumpy_filter_reset();
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
        vision_bumpy_filter_reset();
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
        float e_raw;
        float e_med;
        float e_filt;

        /* 计算原始角度误差(仅 atan2 转换, 不做滤波/死区/限幅) */
        e_raw = vision_bumpy_calc_err_degree(packet);

        /* 首次进入 TRACK 或从其他状态转入: 初始化滤波器, 实现零延迟启动 */
        if (g_bumpy_ctrl_shadow.state != VISION_BUMPY_CTRL_TRACK)
        {
            vision_bumpy_filter_init(e_raw);
        }
        else if (packet_new)
        {
            /* 仅在新帧到达时更新中值滤波历史(避免帧间重复填充相同值导致滤波失效) */
            g_bumpy_ctrl_shadow.raw_err_history[g_bumpy_ctrl_shadow.history_idx] = e_raw;
            g_bumpy_ctrl_shadow.history_idx = (uint8)((g_bumpy_ctrl_shadow.history_idx + 1U) % VISION_BUMPY_MEDIAN_WINDOW);
        }

        /* 第1级: 3点中值滤波 — 剔除单帧脉冲离群噪声 */
        e_med = vision_bumpy_median3(
            g_bumpy_ctrl_shadow.raw_err_history[0],
            g_bumpy_ctrl_shadow.raw_err_history[1],
            g_bumpy_ctrl_shadow.raw_err_history[2]);

        /* 第2级: 一阶IIR低通滤波 — 抑制高频抖动, 平滑方向信号 */
        e_filt = vision_bumpy_lpf_step(&g_bumpy_ctrl_shadow.lpf_state, e_med, VISION_BUMPY_LPF_ALPHA);

        /* 第3级: 死区处理 — 消除微小角度振荡对下游转向PID的干扰 */
        if (vision_bumpy_abs_f(e_filt) < VISION_BUMPY_DEADBAND_DEG)
        {
            e_filt = 0.0f;
        }

        /* 第4级: 输出增益 — 调节转向指令的整体强度 */
        e_filt *= VISION_BUMPY_OUTPUT_GAIN;

        /* 第5级: 输出限幅 — 防止异常大角度导致车体失控 */
        g_bumpy_ctrl_shadow.err_degree_cmd = vision_bumpy_constrain_f(e_filt, -VISION_BUMPY_MAX_ERR_DEG, VISION_BUMPY_MAX_ERR_DEG);
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_TRACK;
    }
    else
    {
        g_bumpy_ctrl_shadow.state = VISION_BUMPY_CTRL_SEARCH;         // 设置状态为搜索
        g_bumpy_ctrl_shadow.err_degree_cmd = 0.0f;                     // 清零误差指令
        vision_bumpy_filter_reset();
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
