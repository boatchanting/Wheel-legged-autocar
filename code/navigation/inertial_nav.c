#include "inertial_nav.h"

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
    inertial_nav.x = 0.0f;//小车向前为负数
    inertial_nav.y = 0.0f;//小车向右为正数
    inertial_nav.relative_yaw = 0.0f; // 初始化相对偏航角
    inertial_nav.vx_body = 0.0f;
    inertial_nav.vy_body = 0.0f;
}

/**
 * @brief 惯性导航更新函数 (10ms 调用一次)
 */
void InertialNav_Update(float curr_yaw, float init_yaw, 
                        float acc_lat_left, float acc_lon_forward, 
                        float speed_L, float speed_R) 
{
    // --- 1. 预处理输入数据 ---
    // 将轮速统一为前进方向为正 (单位: mm/s)
    float v_wheel_left = -speed_L; 
    float v_wheel_right = speed_R;
    
    // 计算车身中心点的平均轮速
    float v_wheel_avg = (v_wheel_left + v_wheel_right) * 0.5f;

    // --- 2. 纵向速度融合 ---
    // 预测值: 上一时刻速度 + 加速度积分
    float v_pred = inertial_nav.vx_body + acc_lon_forward * NAV_DT;
    // 融合: 使用互补滤波
    if (fabsf(v_wheel_avg) < 1.0f) { // 如果小车接近静止，则速度清零以防漂移
        inertial_nav.vx_body = 0.0f;
    }
    else if (fabsf(acc_lon_forward) < NAV_LON_ACC_ZERO_THRESHOLD) {
        // A. 如果加速度极小，说明小车实际没有在加速或减速。
        //    此时，无论轮速计多快，实际位移都应该为零。
        //    我们应优先强制速度为零，以防止漂移。
        inertial_nav.vx_body = 0.0f;
        // printf("acc_lon_forward: %f\n", acc_lon_forward);
    }
    else {
        inertial_nav.vx_body = NAV_ALPHA_VEL * v_wheel_avg + (1.0f - NAV_ALPHA_VEL) * v_pred;
    }

    // --- 3. 横向速度估算 (侧滑) ---
    // 衰减上一时刻的侧滑速度, 并累加当前加速度产生的侧滑增量
    inertial_nav.vy_body = inertial_nav.vy_body * NAV_DECAY_LAT + acc_lat_left * NAV_DT;

    // 死区处理: 如果横向加速度过小, 认为是噪声, 强制将侧滑速度清零
    // 这是为了防止在直线行驶时, 因为传感器噪声导致位置逐渐偏移
    if (fabsf(acc_lat_left) < NAV_LAT_ACC_DEADZONE) {
        inertial_nav.vy_body = 0.0f;
        // printf("acc_lat_left: %f\n", acc_lat_left);
    }

    // --- 4. 坐标系转换 ---
    // 计算以初始偏航角为 0 度的当前航向角
    float relative_yaw_deg = curr_yaw - init_yaw;
     relative_yaw_deg = fmodf(relative_yaw_deg + 180.0f, 360.0f) - 180.0f;

    // 将计算结果存入全局结构体
    inertial_nav.relative_yaw = relative_yaw_deg;

    // 将相对角度转换为弧度用于三角函数计算
    float relative_yaw_rad = DEG2RAD(inertial_nav.relative_yaw);

    float cos_theta = cosf(relative_yaw_rad);
    float sin_theta = sinf(relative_yaw_rad);

    // 将车身坐标系下的速度旋转到世界坐标系
    float vx_world = inertial_nav.vx_body * cos_theta - inertial_nav.vy_body * sin_theta;
    float vy_world = inertial_nav.vx_body * sin_theta + inertial_nav.vy_body * cos_theta;

    // --- 5. 位置积分 ---
    // 在最终的位移上乘以校准系数
    float dx = vx_world * NAV_DT * NAV_DISTANCE_SCALE_FACTOR;
    float dy = vy_world * NAV_DT * NAV_DISTANCE_SCALE_FACTOR;

    inertial_nav.x += dx;
    inertial_nav.y += dy;
}