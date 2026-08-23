#ifndef _MINEFIELD_H_
#define _MINEFIELD_H_

#include "zf_common_headfile.h"

// 导出触发标志位，外部逻辑（如图像处理/gps信号）将此置1触发动作
extern volatile uint8_t minefield_flag;
// 雷区转圈规则：720 度是基础圈数，额外 5 度作为硬性安全余量。
#define MINEFIELD_SPIN_BASE_CIRCLE_ANGLE          720.0f
#define MINEFIELD_SPIN_RESERVE_ANGLE              5.0f
#define MINEFIELD_SPIN_MIN_TOTAL_ANGLE            (MINEFIELD_SPIN_BASE_CIRCLE_ANGLE + MINEFIELD_SPIN_RESERVE_ANGLE)
// 满足最小转圈角后，出口航向在该误差内即可释放，剩余偏差由导航边跑边修。
#define MINEFIELD_SPIN_EXIT_RELEASE_YAW_TOLERANCE 35.0f
#define MINEFIELD_SPIN_ABORT_NONE                 0U
#define MINEFIELD_SPIN_ABORT_TIMEOUT              1U
#define MINEFIELD_SPIN_ABORT_STALLED              2U

// 雷区转圈及交接参数：转向输出最多占用 5500 PWM，按 8000 PWM 电机上限为平衡环保留 2500 PWM。
#define MINEFIELD_SPIN_HEIGHT_TARGET              3.0f
#define MINEFIELD_BALANCE_PWM_RESERVE              2500.0f
#define MINEFIELD_TURN_PWM_MAX_ALLOWED            5500.0f
#define MINEFIELD_SPIN_HANDOFF_DURATION_MS        500U
#define MINEFIELD_SPIN_HANDOFF_RATIO_STEP         (1.0f / (float)MINEFIELD_SPIN_HANDOFF_DURATION_MS)

typedef enum
{
    MINEFIELD_SPIN_PHASE_IDLE = 0,
    MINEFIELD_SPIN_PHASE_DRIVE,
    MINEFIELD_SPIN_PHASE_COAST,
    MINEFIELD_SPIN_PHASE_CAPTURE
} MinefieldSpinPhase_e;
extern uint8 vision_detected_marker;//雷区调用,测试用
extern volatile uint8_t g_minefield_spin_abort_reason;
extern volatile uint8_t g_minefield_beep_request; // 自转结束蜂鸣器请求标志

// Telemetry debug variables for autorotation
extern volatile float g_minefield_debug_accumulated_angle;
extern volatile float g_minefield_debug_angle_cmd;
extern volatile float g_minefield_debug_feedforward_speed;
extern volatile float g_minefield_debug_current_speed_cmd;
extern volatile float g_minefield_debug_stall_elapsed_s;
extern volatile uint8_t g_minefield_debug_phase;
extern volatile float g_minefield_debug_exit_yaw_error;

/**
 * @brief 初始化/复位旋转控制的相关变量
 */
void Minefield_Init(void);
void Minefield_SetSpinPlan(float total_spin_deg, float exit_yaw_deg, float spin_speed_sign);
/**
 * @brief 设置精确旋转角度（用于调试PD控制器）
 * @param total_spin_deg 精确旋转角度（deg），不会钳到最小725度
 * @param spin_speed_sign 旋转方向：1.0=CW, -1.0=CCW
 * @param enable_exit_release 1=启用航向提前释放，0=禁用（精确转到指定角度）
 */
void Minefield_SetSpinPlanExact(float total_spin_deg, float spin_speed_sign, uint8_t enable_exit_release);

/**
 * @brief 判断当前是否处于旋转动作执行中
 * @return 1: 正在旋转, 0: 空闲/正常行驶
 */
uint8_t Minefield_Is_Active(void);
uint8_t Minefield_IsCoasting(void);
MinefieldSpinPhase_e Minefield_GetSpinPhase(void);

/**
 * @brief 旋转动作核心控制函数 (需在2ms Gyro环中调用，当前Core0调度1ms)
 * 
 * @param gyro_z_deg      当前Z轴角速度 (单位: °/s)
 * @param dt_s            调用周期时间 (单位: 秒，通常为 0.002f，当前Core0转向角速度环为0.001f)
 * @param current_yaw_deg 当前偏航角；保留参数用于兼容旧调用，新逻辑不再用它慢速补角
 * @param target_yaw_ptr  目标偏航角指针；保留参数用于兼容旧调用，新逻辑不再修改 g_initial_yaw
 * 
 * @return float          计算出的目标旋转角速度 (单位: °/s)，若未激活则返回 0.0f
 * 
 * @note 此函数内部实现梯形速度规划；满足 725 度后会用出口航向判断是否提前释放。
 */
float Minefield_Spin_Controller(float gyro_z_deg, float dt_s, float current_yaw_deg,volatile float* target_yaw_ptr);

#endif
