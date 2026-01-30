#include "zf_common_headfile.h"

// ================= 参数配置宏 =================
#define SPIN_TARGET_ANGLE     800.0f  // 目标旋转总角度 (2圈)由于地打滑的缘故，实际转动角度会变大，视实际情况调整
#define SPIN_MAX_SPEED        220.0f  // 最大旋转速度 (°/s)
#define SPIN_ACCEL_STEP       0.6f    // 加速度步长 (每次调用增加的速度) 建议 0.4~1.0
#define SPIN_DECEL_ANGLE      180.0f  // 距离结束剩多少度时开始减速
#define SPIN_MIN_SPEED        45.0f   // 最小收尾速度 (防止由于摩擦力停下)

// ================= 全局/静态变量 =================
volatile uint8_t minefield_flag = 0;   // 触发标志位
static uint8_t  s_is_spinning = 0;     // 内部运行状态
static float    s_accumulated_angle = 0.0f; // 已旋转角度积分
static float    s_current_speed_cmd = 0.0f; // 当前平滑后的速度指令
uint8_t vision_detected_marker = 0; // 雷区调用，测试用
// 内部辅助函数：斜坡限制器
static float _ramp_float(float current, float target, float step)
{
    if (current < target)
    {
        current += step;
        if (current > target) current = target;
    }
    else if (current > target)
    {
        current -= step;
        if (current < target) current = target;
    }
    return current;
}

void Minefield_Init(void)
{
    minefield_flag = 0;
    s_is_spinning = 0;
    s_accumulated_angle = 0.0f;
    s_current_speed_cmd = 0.0f;
}

uint8_t Minefield_Is_Active(void)
{
    return s_is_spinning;
}

float Minefield_Spin_Controller(float gyro_z_deg, float dt_s, float current_yaw_deg,volatile float* target_yaw_ptr)
{
    // 1. 检测触发信号 (上升沿)
    if (minefield_flag == 1)
    {
        minefield_flag = 0;         // 清除触发标志
        s_is_spinning = 1;          // 激活内部状态
        s_accumulated_angle = 0.0f; // 重置积分
        s_current_speed_cmd = 0.0f; // 重置速度
    }

    // 2. 如果未激活，直接返回 0
    if (s_is_spinning == 0)
    {
        return 0.0f;
    }

    // ================= 核心控制逻辑 =================

    // 3. 积分计算已转过的角度
    // 无论左转还是右转，我们关注转过的幅度，所以用角速度的绝对值累加
    // 如果你知道一定是向左转，可以直接 s_accumulated_angle += gyro_z_deg * dt_s;
    s_accumulated_angle += fabsf(gyro_z_deg * dt_s); 

    float remaining = SPIN_TARGET_ANGLE - s_accumulated_angle;
    float target_speed = 0.0f;

    // 4. 判断是否完成
    if (s_accumulated_angle >= SPIN_TARGET_ANGLE)
    {
        // --- 动作结束 ---
        s_is_spinning = 0;
        s_current_speed_cmd = 0.0f;
        
        // [关键] 重置系统的航向目标为当前朝向，防止PID回弹
        if (target_yaw_ptr != 0) 
        {
            *target_yaw_ptr = current_yaw_deg;
        }
        
        return 0.0f;
    }

    // 5. 梯形速度规划
    if (remaining < SPIN_DECEL_ANGLE)
    {
        // [减速区] 线性减速
        target_speed = SPIN_MAX_SPEED * (remaining / SPIN_DECEL_ANGLE);
        if (target_speed < SPIN_MIN_SPEED) target_speed = SPIN_MIN_SPEED;
    }
    else
    {
        // [巡航区] 全速
        target_speed = SPIN_MAX_SPEED;
    }

    // 6. 斜坡平滑处理 (防止速度突变导致倒车)
    s_current_speed_cmd = _ramp_float(s_current_speed_cmd, target_speed, SPIN_ACCEL_STEP);

    // 7. 返回最终速度指令
    // 注意：如果是向右旋转(顺时针)，这里需要返回负值: return -s_current_speed_cmd;
    return s_current_speed_cmd; 
}