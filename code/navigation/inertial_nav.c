#include "inertial_nav.h"
#include "../config/sys_options.h"

// --- 宏定义 ---
#ifndef M_PI
#define M_PI                3.1415926535f
#endif
#define DEG2RAD(x)          ((x) * M_PI / 180.0f)


// --- 全局变量定义 ---
// 定义在 .h 文件中声明的全局导航状态实例
InertialNav_t inertial_nav;

/**
 * @brief 初始化惯性导航系统
 */
void InertialNav_Init(void) {
    inertial_nav.x = 0.0f;//小车向前为负数,向后为x正方向
    inertial_nav.y = 0.0f;//小车向右为正数,向右为y正方向
    inertial_nav.relative_yaw = 0; // 初始化相对偏航角
    inertial_nav.init_yaw = 0.0f;
    inertial_nav.vx_body = 0.0f;
    inertial_nav.vy_body = 0.0f;
    inertial_nav.slip_flag = 0;// 初始化打滑标志位
    inertial_nav.slip_timer_ms = 0;
}

/**
 * @brief 惯性导航更新函数 (10ms 调用一次)
 */
void InertialNav_Update(float curr_yaw,
                        float acc_lat_left, float acc_lon_forward, 
                        float speed_L, float speed_R, float gyro_z_rad_s) 
{
    if (inertial_nav.init_yaw == 0.0f) {
        inertial_nav.init_yaw = euler_angle.yaw;//初始化记录开始导航时的角度
    }

    // --- 1. 物理单位标准化 ---
    // 参考 Guandao_Plus: 将原始脉冲/数值转换为 mm/s
    // 假设 speed_R 为正代表前进，speed_L 为负代表前进（需根据你实际极性调整）
    float v_L_mm = -speed_L * SPEED_TO_MM_S; 
    float v_R_mm = speed_R * SPEED_TO_MM_S;
    float v_wheel_avg = (v_L_mm + v_R_mm) * 0.5f;

    // --- 2. 打滑检测逻辑 ---
    float rel_yaw_deg = curr_yaw - inertial_nav.init_yaw;
    // 角度归一化到 [-180, 180]
    rel_yaw_deg = fmodf(rel_yaw_deg + 180.0f, 360.0f);
    if (rel_yaw_deg < 0) rel_yaw_deg += 360.0f;
    rel_yaw_deg -= 180.0f;
    
    float curr_yaw_rad = DEG2RAD(rel_yaw_deg);
    
    // 废弃偏航角差分，改用传入的车体系陀螺仪角速度
    float actual_yaw_rate = gyro_z_rad_s; 
    
    // 计算角加速度，用于动态阈值放宽
    static float last_actual_yaw_rate = 0.0f;
    float dot_omega = (actual_yaw_rate - last_actual_yaw_rate) / NAV_DT;
    last_actual_yaw_rate = actual_yaw_rate;

    // 理论角速度 (基于轮速差): ω = (Vr - Vl) / L
    float theoretical_yaw_rate = (v_R_mm - v_L_mm) / WHEEL_BASE_MM;

    // 缓存调试信息供 WiFi 发送
    inertial_nav.current_speed_L = v_L_mm;
    inertial_nav.current_speed_R = v_R_mm;
    inertial_nav.theoretical_yaw_rate = theoretical_yaw_rate;
    inertial_nav.actual_yaw_rate = actual_yaw_rate;

    // 1. 理论向心加速度 (单位: mm/s^2)
    // 根据运动学，向心加速度 a_c = v_x * omega
    float centripetal_accel = v_wheel_avg * actual_yaw_rate;

    // 2. 异常的侧向滑动加速度
    float anomaly_lat_accel = acc_lat_left - centripetal_accel;

    // 3. 动态侧滑阈值 (多维度动态容错)
    // 基础容错 500 mm/s^2 + 向心加速度比例容错 + 角加速度动态延迟容错
    float lat_slip_thres_enter = 500.0f + 0.15f * fabsf(centripetal_accel) + 20.0f * fabsf(dot_omega);
    float lat_slip_thres_exit  = 300.0f + 0.10f * fabsf(centripetal_accel);

    // --- 4. 横向速度与侧滑角观测器 ---
    if (inertial_nav.slip_flag == 2 || inertial_nav.slip_flag == 3) {
        inertial_nav.vy_body = 0.0f;
    } else if (inertial_nav.slip_flag == 1 || fabsf(anomaly_lat_accel) > lat_slip_thres_enter) {
        // 侧滑期间使用纯积分。废弃原本的泄漏积分器（0.95），因为它会在减速段过度扣减速度导致反向速度（轨迹回抽）。
        inertial_nav.vy_body += anomaly_lat_accel * NAV_DT;
    } else {
        // 恢复抓地时，物理侧向速度必须为0，快速衰减以消除纯积分带来的漂移
        inertial_nav.vy_body *= 0.8f;
    }
    
    // 计算侧滑角 (度)
    float min_vx = 200.0f; // 低于此速度时侧滑角分母过小失去意义
    float abs_vx = fabsf(inertial_nav.vx_body);
    if (abs_vx < min_vx) abs_vx = min_vx;
    inertial_nav.slip_angle = atan2f(inertial_nav.vy_body, abs_vx) * 180.0f / M_PI;

    // --- 综合打滑判断 ---
#if SLIP_DETECTION_ENABLE
    // 1. 静止判断 (物理速度极小，无异常横移，无异常旋转)
    if (fabsf(v_wheel_avg) < 5.0f && fabsf(anomaly_lat_accel) < 300.0f && fabsf(actual_yaw_rate) < 0.1f) {
        inertial_nav.slip_flag = 2; // 真实静止
        inertial_nav.vx_body = 0;
        inertial_nav.vy_body = 0;
        inertial_nav.slip_timer_ms = 0;
    } 
    // 2. 原地自转判断 (左右轮差速极大，平均轮速相对于差速很小，且有较大角速度)
    else if (fabsf(actual_yaw_rate) > 0.3f && fabsf(v_R_mm - v_L_mm) > 200.0f && fabsf(v_wheel_avg) < fabsf(v_R_mm - v_L_mm) * 0.5f) {
        inertial_nav.slip_flag = 3; // 真实原地自转
        acc_lon_forward = 0; // 自转时不应该有明显的纵向加速度
        inertial_nav.vy_body = 0;
        inertial_nav.slip_timer_ms = 0;
    }
    // 3. 动态阈值打滑判断 (基于纯径向侧滑，带状态机)
    else {
        if (inertial_nav.slip_flag == 1) {
            // 当前处于侧滑状态，判断是否退出 (加速度残差减小 且 侧滑角回落)
            if (fabsf(anomaly_lat_accel) < lat_slip_thres_exit && fabsf(inertial_nav.slip_angle) < 3.0f) {
                inertial_nav.slip_timer_ms += (uint16_t)(NAV_DT * 1000);
                if (inertial_nav.slip_timer_ms >= 200) { // 连续 200ms 低于退出阈值
                    inertial_nav.slip_flag = 0;
                    inertial_nav.slip_timer_ms = 0;
                }
            } else {
                inertial_nav.slip_timer_ms = 0; // 只要有一次超限，重新计时
            }
        } else {
            // 当前非侧滑状态（可能是正常或从静止/自转刚切换出来），先确认标志位
            if (inertial_nav.slip_flag != 0) {
                inertial_nav.slip_flag = 0;
                inertial_nav.slip_timer_ms = 0;
            }
            
            // 判断是否进入侧滑 (加速度残差过大 OR 侧滑角过大)
            if (fabsf(anomaly_lat_accel) > lat_slip_thres_enter || fabsf(inertial_nav.slip_angle) > 5.0f) {
                inertial_nav.slip_timer_ms += (uint16_t)(NAV_DT * 1000);
                if (inertial_nav.slip_timer_ms >= 80) { // 连续 80ms 超限
                    inertial_nav.slip_flag = 1;
                    inertial_nav.slip_timer_ms = 0;
                }
            } else {
                inertial_nav.slip_timer_ms = 0;
            }
        }
    }
#else
    inertial_nav.slip_flag = 0; // 禁用打滑检测时，默认正常抓地
#endif

    // --- 3. 纵向速度融合 ---
    // 如果没有明显打滑，信任轮速多一点；如果打滑，减小轮速权重
    float alpha = (inertial_nav.slip_flag == 1) ? 0.3f : NAV_ALPHA_VEL;
    float v_pred = inertial_nav.vx_body + acc_lon_forward * NAV_DT;
    inertial_nav.vx_body = alpha * v_wheel_avg + (1.0f - alpha) * v_pred;

    // 旧的横向速度修正已在上方作为观测器实现，这里保留空行以对齐代码格式

    

    // --- 5. 坐标变换与积分 ---
    inertial_nav.relative_yaw = rel_yaw_deg;
    float cos_theta = cosf(curr_yaw_rad);
    float sin_theta = sinf(curr_yaw_rad);

    // 车身坐标系 -> 世界坐标系
    // 车身x前进，车身y向左。根据坐标系定义：
    // vx_world = vx*cos - vy*sin
    // vy_world = vx*sin + vy*cos
    float vx_world = inertial_nav.vx_body * cos_theta - inertial_nav.vy_body * sin_theta;
    float vy_world = inertial_nav.vx_body * sin_theta + inertial_nav.vy_body * cos_theta;

    // 积分得到位置 (单位: mm)
    // 注意：这里的方向需对应你代码注释的“前进为负，向右为正”
    // 如果 vx_world 是正（前进），则 dx 应为负
    inertial_nav.x += vx_world * NAV_DT; 
    inertial_nav.y += vy_world * NAV_DT; 
}