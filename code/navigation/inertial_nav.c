#include "inertial_nav.h"
#include "../config/sys_options.h"
#include "../plan/minefield.h"

// --- 宏定义 ---
#ifndef M_PI
#define M_PI                3.1415926535f
#endif
#define DEG2RAD(x)          ((x) * M_PI / 180.0f)

// --- 雷区自转抗干扰参数 ---
// 1. 加速度死区阈值 (mm/s^2)：用于过滤原地自转时的机械震动和离心力引起的虚假加速度。
//    实车调试时，如果发现原地转时纵向速度仍然漂移增加，可适当调大该值（如 600, 800）。
//    如果发现碰撞等真实物理位移被漏检，可适当调小该值（如 300）。
#define MINEFIELD_ACC_DEADZONE  500.0f 

// 2. 速度阻尼衰减系数 (ZUPT)：用于强迫残余的假速度迅速收敛到 0。
//    实车调试时，值越小 (如 0.90) 衰减越快，定位越稳，但对真实碰撞位移的记录会缩水。
//    值越大 (如 0.99) 保留的真实位移越足，但可能会漏掉一些漂移。0.95 表示每 10ms 衰减 5%。
#define MINEFIELD_VEL_DAMPING   0.95f


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
    inertial_nav.v_pred = 0.0f;
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
    (void)acc_lat_left; // 横向加速度暂不使用

    // 缓存调试信息供 WiFi 发送
    inertial_nav.current_speed_L = v_L_mm;
    inertial_nav.current_speed_R = v_R_mm;
    inertial_nav.theoretical_yaw_rate = (v_R_mm - v_L_mm) / WHEEL_BASE_MM;
    inertial_nav.actual_yaw_rate = actual_yaw_rate;

    // --- 自转状态判断（直接读取雷区旋转标志位） ---
    if (Minefield_Is_Active()) {
        inertial_nav.slip_flag = 3; // 雷区自转中
        // acc_lon_forward = 0;        // 自转时纵向加速度归零
        // inertial_nav.vy_body = 0;   // 自转时横向速度归零
        inertial_nav.slip_timer_ms = 0;
    } else {
        inertial_nav.slip_flag = 0;
    }

    // --- 3. 横向速度观测器 ---
    // slip_flag 已在上方由 Minefield_Is_Active() 设定
    if (inertial_nav.slip_flag == 3) {
        // inertial_nav.vy_body = 0.0f;
        inertial_nav.vy_body *= 0.8f; // 测试完整版惯导，不强制清零
    } else {
        inertial_nav.vy_body *= 0.8f; // 正常抓地时快速衰减
    }
    
    // 计算侧滑角 (度)
    float min_vx = 200.0f; // 低于此速度时侧滑角分母过小失去意义
    float abs_vx = fabsf(inertial_nav.vx_body);
    if (abs_vx < min_vx) abs_vx = min_vx;
    inertial_nav.slip_angle = atan2f(inertial_nav.vy_body, abs_vx) * 180.0f / M_PI;

    // --- 4. 纵向速度融合 ---
    // 自转时减小轮速权重，信任 IMU 加速度积分；正常行驶时完全信任轮速
    float alpha = (inertial_nav.slip_flag == 3) ? 0.0f : NAV_ALPHA_VEL;
    
    // +++ 自转专属：加速度死区过滤 (抗震动与离心力串扰) +++
    if (inertial_nav.slip_flag == 3) {
        if (fabsf(acc_lon_forward) < MINEFIELD_ACC_DEADZONE) {
            acc_lon_forward = 0.0f;
        }
    }
    
    float v_pred = inertial_nav.vx_body + acc_lon_forward * NAV_DT;
    
    // +++ 自转专属：速度阻尼衰减 (Leaky Integrator) +++
    if (inertial_nav.slip_flag == 3) {
        v_pred *= MINEFIELD_VEL_DAMPING; 
    }

    // 缓存融合前的 IMU 积分速度，供 WiFi 回传调试
    inertial_nav.v_pred = v_pred;

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